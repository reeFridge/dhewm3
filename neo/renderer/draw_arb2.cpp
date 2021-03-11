/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "sys/platform.h"
#include "renderer/VertexCache.h"

#include "renderer/tr_local.h"

static	const int	MAX_GLPROGS = 200;

// GLSL

typedef struct shaderProgDef_t shaderProgDef_t;
struct shaderProgDef_t {
	char vs_name[64];
	char fs_name[64];
	program_t id;
	GLuint program;
};

static shaderProgDef_t shader_progs[MAX_GLPROGS] = {
	{ "interaction.vert", "interaction.frag", SPROG_INTERACTION, 0 },
	{ "fill_depth.vert", "fill_depth.frag", SPROG_FILL_DEPTH_BUFFER, 0 },
	{ "shadow.vert", "shadow.frag", SPROG_SHADOW, 0 },
	{ "light_scale.vert", "light_scale.frag", SPROG_LIGHT_SCALE, 0 },
	{ "shader_pass.vert", "shader_pass.frag", SPROG_SHADER_PASS, 0 },
	{ "cube_shader_pass.vert", "cube_shader_pass.frag", SPROG_CUBE_SHADER_PASS, 0 },
	{ "environment.vert", "environment.frag", SPROG_ENV, 0 },
	{ "bumpy_environment.vert", "bumpy_environment.frag", SPROG_BUMPY_ENV, 0 },
	{ "glasswarp.vert", "glasswarp.frag", SPROG_GLASSWARP, 0 },
	{ "heat_haze.vert", "heat_haze.frag", SPROG_HEAT_HAZE, 0 },
	{ "heat_haze_with_mask.vert", "heat_haze_with_mask.frag", SPROG_HEAT_HAZE_WITH_MASK, 0 },
	{ "heat_haze_with_mask_and_vertex.vert", "heat_haze_with_mask_and_vertex.frag", SPROG_HEAT_HAZE_WITH_MASK_AND_VERTEX, 0 },
	{ "color_process.vert", "color_process.frag", SPROG_COLOR_PROCESS, 0 },
	{ "fog_pass.vert", "fog_pass.frag", SPROG_FOG_PASS, 0 },
	{ "blend_light.vert", "blend_light.frag", SPROG_BLEND_LIGHT, 0 }
};

typedef struct arbToProgram_t arbToProgram_t;
struct arbToProgram_t {
	char program_name[64];
	program_t id;
};

static const int MAX_ARB_TO_PROGRAM = 4;

static arbToProgram_t arb_to_program[MAX_ARB_TO_PROGRAM] = {
	{ "heatHaze.vfp", SPROG_HEAT_HAZE },
	{ "heatHazeWithMask.vfp", SPROG_HEAT_HAZE_WITH_MASK },
	{ "heatHazeWithMaskAndVertex.vfp", SPROG_HEAT_HAZE_WITH_MASK_AND_VERTEX },
	{ "colorProcess.vfp", SPROG_COLOR_PROCESS }
};

GLuint R_LoadPartShader(idStr path, GLuint type) {
	char *fileBuffer = 0;
	common->Printf( "Load GLSL prog %s", path.c_str() );
	fileSystem->ReadFile( path.c_str(), (void **)&fileBuffer, NULL );
	if ( !fileBuffer ) {
		common->Printf( ": File not found\n" );
		return 0;
	}
	common->Printf( ": OK\n");

	const char *buffer = (char *)_alloca( strlen( fileBuffer ) + 1 );
	strcpy( (char *)buffer, fileBuffer );
	fileSystem->FreeFile( fileBuffer );

	if ( !glConfig.isInitialized ) {
		return 0;
	}

	GLuint ref = glCreateShader(type);
	glShaderSource(ref, 1, &buffer, NULL);
	glCompileShader(ref);
	int success;
	char log[512];
	glGetShaderiv(ref, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(ref, 512, NULL, log);
		common->Printf("Error: shader compilation failed\n%s\n", log);
		return 0;
	}

	return ref;
}

void R_LoadShaderProgram(int progIdx) {
	idStr progsPath = "glprogs_mod/";
	idStr vsPath = progsPath + shader_progs[progIdx].vs_name;
	idStr fsPath = progsPath + shader_progs[progIdx].fs_name;

	// loadPartShader
	GLuint vs = R_LoadPartShader(vsPath, GL_VERTEX_SHADER);
	
	if (!vs) {
		return;
	}

	GLuint fs = R_LoadPartShader(fsPath, GL_FRAGMENT_SHADER);

	if (!fs) {
		return;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	int success;
	char log[512];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, 512, NULL, log);
		printf("Error: program compilation failed\n%s\n", log);
		return;
	}

	shader_progs[progIdx].program = program;
}

GLuint R_FindShaderProgram( program_t id ) {
	// see if it is already loaded
	int i;
	for (i = 0; shader_progs[i].vs_name[0]; i++) {
		if ( shader_progs[i].id != id ) {
			continue;
		}

		if (!shader_progs[i].program) {
			R_LoadShaderProgram(i);
		}

		return shader_progs[i].program;
	}

	if ( i == MAX_GLPROGS ) {
		common->Error( "R_FindShaderProgram: not found" );
	}

	return 0;
}

// ---

/*
=========================================================================================

GENERAL INTERACTION RENDERING

=========================================================================================
*/

/*
====================
GL_SelectTextureNoClient
====================
*/
static void GL_SelectTextureNoClient( int unit ) {
	backEnd.glState.currenttmu = unit;
	//glActiveTextureARB( GL_TEXTURE0_ARB + unit );
	glActiveTexture(GL_TEXTURE0 + unit);
}

/*
==================
RB_ARB2_DrawInteraction
==================
*/
void	RB_ARB2_DrawInteraction( const drawInteraction_t *din ) {
	GLuint shader = R_FindShaderProgram(SPROG_INTERACTION);

	// load all the vertex program parameters
	//glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_LIGHT_ORIGIN, din->localLightOrigin.ToFloatPtr() );
	glUniform4fv(glGetUniformLocation(shader, "light_origin"), 1, din->localLightOrigin.ToFloatPtr());
	//glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_VIEW_ORIGIN, din->localViewOrigin.ToFloatPtr() );
	glUniform4fv(glGetUniformLocation(shader, "view_origin"), 1, din->localViewOrigin.ToFloatPtr());

	//glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_LIGHT_PROJECT_S, din->lightProjection[0].ToFloatPtr() );
	glUniform4fv(glGetUniformLocation(shader, "light_project_s"), 1, din->lightProjection[0].ToFloatPtr());
	//glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_LIGHT_PROJECT_T, din->lightProjection[1].ToFloatPtr() );
	glUniform4fv(glGetUniformLocation(shader, "light_project_t"), 1, din->lightProjection[1].ToFloatPtr());
	//glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_LIGHT_PROJECT_Q, din->lightProjection[2].ToFloatPtr() );
	glUniform4fv(glGetUniformLocation(shader, "light_project_q"), 1, din->lightProjection[2].ToFloatPtr());

	//glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_LIGHT_FALLOFF_S, din->lightProjection[3].ToFloatPtr() );
	glUniform4fv(glGetUniformLocation(shader, "light_falloff_s"), 1, din->lightProjection[3].ToFloatPtr());

	//glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_BUMP_MATRIX_S, din->bumpMatrix[0].ToFloatPtr() );
	glUniform4fv(glGetUniformLocation(shader, "bump_matrix_s"), 1, din->bumpMatrix[0].ToFloatPtr());
	//glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_BUMP_MATRIX_T, din->bumpMatrix[1].ToFloatPtr() );
	glUniform4fv(glGetUniformLocation(shader, "bump_matrix_t"), 1, din->bumpMatrix[1].ToFloatPtr());

	//glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_DIFFUSE_MATRIX_S, din->diffuseMatrix[0].ToFloatPtr() );
	glUniform4fv(glGetUniformLocation(shader, "diffuse_matrix_s"), 1, din->diffuseMatrix[0].ToFloatPtr());
	//glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_DIFFUSE_MATRIX_T, din->diffuseMatrix[1].ToFloatPtr() );
	glUniform4fv(glGetUniformLocation(shader, "diffuse_matrix_t"), 1, din->diffuseMatrix[1].ToFloatPtr());

	//glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_SPECULAR_MATRIX_S, din->specularMatrix[0].ToFloatPtr() );
	glUniform4fv(glGetUniformLocation(shader, "specular_matrix_s"), 1, din->specularMatrix[0].ToFloatPtr());
	//glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_SPECULAR_MATRIX_T, din->specularMatrix[1].ToFloatPtr() );
	glUniform4fv(glGetUniformLocation(shader, "specular_matrix_t"), 1, din->specularMatrix[1].ToFloatPtr());

	// testing fragment based normal mapping
	if ( r_testARBProgram.GetBool() ) {
		/*
		glProgramEnvParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 2, din->localLightOrigin.ToFloatPtr() );
		glProgramEnvParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 3, din->localViewOrigin.ToFloatPtr() );
		*/
	}

	static const float zero[4] = { 0, 0, 0, 0 };
	static const float one[4] = { 1, 1, 1, 1 };
	static const float negOne[4] = { -1, -1, -1, -1 };

	switch ( din->vertexColor ) {
	case SVC_IGNORE:
		glUniform4fv(glGetUniformLocation(shader, "color_modulate"), 1, zero);
		glUniform4fv(glGetUniformLocation(shader, "color_add"), 1, one);
		//glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_COLOR_MODULATE, zero );
		//glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_COLOR_ADD, one );
		break;
	case SVC_MODULATE:
		glUniform4fv(glGetUniformLocation(shader, "color_modulate"), 1, one);
		glUniform4fv(glGetUniformLocation(shader, "color_add"), 1, zero);
		//glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_COLOR_MODULATE, one );
		//glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_COLOR_ADD, zero );
		break;
	case SVC_INVERSE_MODULATE:
		glUniform4fv(glGetUniformLocation(shader, "color_modulate"), 1, negOne);
		glUniform4fv(glGetUniformLocation(shader, "color_add"), 1, one);
		//glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_COLOR_MODULATE, negOne );
		//glProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_COLOR_ADD, one );
		break;
	}

	// set the constant colors
	//glProgramEnvParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 0, din->diffuseColor.ToFloatPtr() );
	//glProgramEnvParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 1, din->specularColor.ToFloatPtr() );
	glUniform4fv(glGetUniformLocation(shader, "diffuse_color_const"), 1, din->diffuseColor.ToFloatPtr());
	glUniform4fv(glGetUniformLocation(shader, "specular_color_const"), 1, din->specularColor.ToFloatPtr());

	// set the textures

	// texture 1 will be the per-surface bump map
	GL_SelectTextureNoClient( 1 );
	glUniform1i(glGetUniformLocation(shader, "t_bump_map"), 1);
	din->bumpImage->Bind();

	// texture 2 will be the light falloff texture
	GL_SelectTextureNoClient( 2 );
	glUniform1i(glGetUniformLocation(shader, "t_falloff"), 2);
	din->lightFalloffImage->Bind();

	// texture 3 will be the light projection texture
	GL_SelectTextureNoClient( 3 );
	glUniform1i(glGetUniformLocation(shader, "t_projection"), 3);
	din->lightImage->Bind();

	// texture 4 is the per-surface diffuse map
	GL_SelectTextureNoClient( 4 );
	glUniform1i(glGetUniformLocation(shader, "t_diffuse_map"), 4);
	din->diffuseImage->Bind();

	// texture 5 is the per-surface specular map
	GL_SelectTextureNoClient( 5 );
	glUniform1i(glGetUniformLocation(shader, "t_specular_map"), 5);
	din->specularImage->Bind();

	// draw it
	RB_DrawElementsWithCounters( din->surf->geo );
}


/*
=============
RB_ARB2_CreateDrawInteractions

=============
*/
void RB_ARB2_CreateDrawInteractions( const drawSurf_t *surf ) {
	if ( !surf ) {
		return;
	}

	// perform setup here that will be constant for all interactions
	GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHMASK | backEnd.depthFunc );
	GLuint shader = R_FindShaderProgram(SPROG_INTERACTION);

	// bind the vertex program
	if ( r_testARBProgram.GetBool() ) {
		//glBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_TEST );
		//glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, FPROG_TEST );
	} else {
		//glBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_INTERACTION );
		//glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, FPROG_INTERACTION );
		glUseProgram(shader);
	}


	//glEnable(GL_VERTEX_PROGRAM_ARB);
	//glEnable(GL_FRAGMENT_PROGRAM_ARB);

	// enable the vertex arrays
	//glEnableVertexAttribArrayARB( 8 );
	//glEnableVertexAttribArrayARB( 9 );
	//glEnableVertexAttribArrayARB( 10 );
	//glEnableVertexAttribArrayARB( 11 );
	glEnableVertexAttribArray( 8 );
	glEnableVertexAttribArray( 9 );
	glEnableVertexAttribArray( 10 );
	glEnableVertexAttribArray( 11 );
	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );
	//glEnableClientState( GL_COLOR_ARRAY );

	// texture 0 is the normalization cube map for the vector towards the light
	GL_SelectTextureNoClient( 0 );
	glUniform1i(glGetUniformLocation(shader, "t_cube_map"), 0);
	if ( backEnd.vLight->lightShader->IsAmbientLight() ) {
		globalImages->ambientNormalMap->Bind();
	} else {
		globalImages->normalCubeMapImage->Bind();
	}

	// texture 6 is the specular lookup table
	GL_SelectTextureNoClient( 6 );
	glUniform1i(glGetUniformLocation(shader, "t_specular_lookup"), 6);
	if ( r_testARBProgram.GetBool() ) {
		globalImages->specular2DTableImage->Bind();	// variable specularity in alpha channel
	} else {
		globalImages->specularTableImage->Bind();
	}

	// default matrices
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, backEnd.viewDef->worldSpace.modelMatrix);
	glUniformMatrix4fv(glGetUniformLocation(shader, "modelView"), 1, GL_FALSE, backEnd.viewDef->worldSpace.modelViewMatrix);
	glUniformMatrix4fv(glGetUniformLocation(shader, "proj"), 1, GL_FALSE, backEnd.viewDef->projectionMatrix);

	for ( ; surf ; surf=surf->nextOnLight ) {
		// perform setup here that will not change over multiple interaction passes

		// set the vertex pointers
		idDrawVert	*ac = (idDrawVert *)vertexCache.Position( surf->geo->ambientCache );
		glVertexAttribPointer( 9, 3, GL_FLOAT, GL_FALSE, sizeof( idDrawVert ), ac->normal.ToFloatPtr() );
		glVertexAttribPointer( 10, 3, GL_FLOAT, GL_FALSE, sizeof( idDrawVert ), ac->tangents[0].ToFloatPtr() );
		glVertexAttribPointer( 11, 3, GL_FLOAT, GL_FALSE, sizeof( idDrawVert ), ac->tangents[1].ToFloatPtr() );
		glVertexAttribPointer(8, 2, GL_FLOAT, GL_FALSE, sizeof( idDrawVert ), ac->st.ToFloatPtr());
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof( idDrawVert ), ac->xyz.ToFloatPtr());
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof( idDrawVert ), ac->color);
		//glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( idDrawVert ), ac->color );
		//glVertexPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->xyz.ToFloatPtr() );

		// this may cause RB_ARB2_DrawInteraction to be exacuted multiple
		// times with different colors and images if the surface or light have multiple layers
		RB_CreateSingleDrawInteractions( surf, RB_ARB2_DrawInteraction );
	}

	glDisableVertexAttribArray( 8 );
	glDisableVertexAttribArray( 9 );
	glDisableVertexAttribArray( 10 );
	glDisableVertexAttribArray( 11 );
	glDisableVertexAttribArray( 0 );
	glDisableVertexAttribArray( 1 );
	//glDisableClientState( GL_COLOR_ARRAY );

	// disable features
	/*
	GL_SelectTextureNoClient( 6 );
	globalImages->BindNull();

	GL_SelectTextureNoClient( 5 );
	globalImages->BindNull();

	GL_SelectTextureNoClient( 4 );
	globalImages->BindNull();

	GL_SelectTextureNoClient( 3 );
	globalImages->BindNull();

	GL_SelectTextureNoClient( 2 );
	globalImages->BindNull();

	GL_SelectTextureNoClient( 1 );
	globalImages->BindNull();
	*/
	globalImages->BindNull();

	backEnd.glState.currenttmu = -1;
	GL_SelectTexture( 0 );

	//glDisable(GL_VERTEX_PROGRAM_ARB);
	//glDisable(GL_FRAGMENT_PROGRAM_ARB);
	glUseProgram(0);
}


/*
==================
RB_ARB2_DrawInteractions
==================
*/
void RB_ARB2_DrawInteractions( void ) {
	viewLight_t		*vLight;

	GL_SelectTexture( 0 );
	//glDisableClientState( GL_TEXTURE_COORD_ARRAY );

	//
	// for each light, perform adding and shadowing
	//
	for ( vLight = backEnd.viewDef->viewLights ; vLight ; vLight = vLight->next ) {
		backEnd.vLight = vLight;

		// do fogging later
		if ( vLight->lightShader->IsFogLight() ) {
			continue;
		}
		if ( vLight->lightShader->IsBlendLight() ) {
			continue;
		}

		if ( !vLight->localInteractions && !vLight->globalInteractions
			&& !vLight->translucentInteractions ) {
			continue;
		}

		// clear the stencil buffer if needed
		if ( vLight->globalShadows || vLight->localShadows ) {
			backEnd.currentScissor = vLight->scissorRect;
			if ( r_useScissor.GetBool() ) {
				glScissor( backEnd.viewDef->viewport.x1 + backEnd.currentScissor.x1,
					backEnd.viewDef->viewport.y1 + backEnd.currentScissor.y1,
					backEnd.currentScissor.x2 + 1 - backEnd.currentScissor.x1,
					backEnd.currentScissor.y2 + 1 - backEnd.currentScissor.y1 );
			}
			glClear( GL_STENCIL_BUFFER_BIT );
		} else {
			// no shadows, so no need to read or write the stencil buffer
			// we might in theory want to use GL_ALWAYS instead of disabling
			// completely, to satisfy the invarience rules
			glStencilFunc( GL_ALWAYS, 128, 255 );
		}

		if ( r_useShadowVertexProgram.GetBool() ) {
			/*
			glEnable( GL_VERTEX_PROGRAM_ARB );
			glBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_STENCIL_SHADOW );
			*/
			RB_StencilShadowPass( vLight->globalShadows );
			RB_ARB2_CreateDrawInteractions( vLight->localInteractions );
			/*
			glEnable( GL_VERTEX_PROGRAM_ARB );
			glBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_STENCIL_SHADOW );
			*/
			RB_StencilShadowPass( vLight->localShadows );
			RB_ARB2_CreateDrawInteractions( vLight->globalInteractions );
			/*
			glDisable( GL_VERTEX_PROGRAM_ARB );	// if there weren't any globalInteractions, it would have stayed on
			*/
		}
/*
		} else {
			RB_StencilShadowPass( vLight->globalShadows );
			RB_ARB2_CreateDrawInteractions( vLight->localInteractions );
			RB_StencilShadowPass( vLight->localShadows );
			RB_ARB2_CreateDrawInteractions( vLight->globalInteractions );
		}
*/
		// translucent surfaces never get stencil shadowed
		if ( r_skipTranslucent.GetBool() ) {
			continue;
		}

		glStencilFunc( GL_ALWAYS, 128, 255 );

		backEnd.depthFunc = GLS_DEPTHFUNC_LESS;
		RB_ARB2_CreateDrawInteractions( vLight->translucentInteractions );

		backEnd.depthFunc = GLS_DEPTHFUNC_EQUAL;
	}

	// disable stencil shadow test
	glStencilFunc( GL_ALWAYS, 128, 255 );

	GL_SelectTexture( 0 );
	//glEnableClientState( GL_TEXTURE_COORD_ARRAY );
}

//===================================================================================


typedef struct {
	GLenum			target;
	GLuint			ident;
	char			name[64];
} progDef_t;

// a single file can have both a vertex program and a fragment program
static progDef_t	progs[MAX_GLPROGS] = {
	{ GL_VERTEX_PROGRAM_ARB, VPROG_TEST, "test.vfp" },
	{ GL_FRAGMENT_PROGRAM_ARB, FPROG_TEST, "test.vfp" },
	{ GL_VERTEX_PROGRAM_ARB, VPROG_INTERACTION, "interaction.vfp" },
	{ GL_FRAGMENT_PROGRAM_ARB, FPROG_INTERACTION, "interaction.vfp" },
	{ GL_VERTEX_PROGRAM_ARB, VPROG_BUMPY_ENVIRONMENT, "bumpyEnvironment.vfp" },
	{ GL_FRAGMENT_PROGRAM_ARB, FPROG_BUMPY_ENVIRONMENT, "bumpyEnvironment.vfp" },
	{ GL_VERTEX_PROGRAM_ARB, VPROG_AMBIENT, "ambientLight.vfp" },
	{ GL_FRAGMENT_PROGRAM_ARB, FPROG_AMBIENT, "ambientLight.vfp" },
	{ GL_VERTEX_PROGRAM_ARB, VPROG_STENCIL_SHADOW, "shadow.vp" },
	{ GL_VERTEX_PROGRAM_ARB, VPROG_ENVIRONMENT, "environment.vfp" },
	{ GL_FRAGMENT_PROGRAM_ARB, FPROG_ENVIRONMENT, "environment.vfp" },
	{ GL_VERTEX_PROGRAM_ARB, VPROG_GLASSWARP, "arbVP_glasswarp.txt" },
	{ GL_FRAGMENT_PROGRAM_ARB, FPROG_GLASSWARP, "arbFP_glasswarp.txt" },

	// additional programs can be dynamically specified in materials
};

/*
=================
R_LoadARBProgram
=================
*/
void R_LoadARBProgram( int progIndex ) {
	int		ofs;
	int		err;
	idStr	fullPath = "glprogs_mod/";
	fullPath += progs[progIndex].name;
	char	*fileBuffer;
	char	*buffer;
	char	*start = NULL, *end;

	common->Printf( "%s", fullPath.c_str() );

	// load the program even if we don't support it, so
	// fs_copyfiles can generate cross-platform data dumps
	fileSystem->ReadFile( fullPath.c_str(), (void **)&fileBuffer, NULL );
	if ( !fileBuffer ) {
		common->Printf( ": File not found\n" );
		return;
	}

	// copy to stack memory and free
	buffer = (char *)_alloca( strlen( fileBuffer ) + 1 );
	strcpy( buffer, fileBuffer );
	fileSystem->FreeFile( fileBuffer );

	if ( !glConfig.isInitialized ) {
		return;
	}

	//
	// submit the program string at start to GL
	//
	if ( progs[progIndex].ident == 0 ) {
		// allocate a new identifier for this program
		progs[progIndex].ident = PROG_USER + progIndex;
	}

	// vertex and fragment programs can both be present in a single file, so
	// scan for the proper header to be the start point, and stamp a 0 in after the end

	if ( progs[progIndex].target == GL_VERTEX_PROGRAM_ARB ) {
		if ( !glConfig.ARBVertexProgramAvailable ) {
			common->Printf( ": GL_VERTEX_PROGRAM_ARB not available\n" );
			return;
		}
		start = strstr( buffer, "!!ARBvp" );
	}
	if ( progs[progIndex].target == GL_FRAGMENT_PROGRAM_ARB ) {
		if ( !glConfig.ARBFragmentProgramAvailable ) {
			common->Printf( ": GL_FRAGMENT_PROGRAM_ARB not available\n" );
			return;
		}
		start = strstr( buffer, "!!ARBfp" );
	}
	if ( !start ) {
		common->Printf( ": !!ARB not found\n" );
		return;
	}
	end = strstr( start, "END" );

	if ( !end ) {
		common->Printf( ": END not found\n" );
		return;
	}
	end[3] = 0;

	glBindProgramARB( progs[progIndex].target, progs[progIndex].ident );
	glGetError();

	glProgramStringARB( progs[progIndex].target, GL_PROGRAM_FORMAT_ASCII_ARB,
		strlen( start ), start );

	err = glGetError();
	glGetIntegerv( GL_PROGRAM_ERROR_POSITION_ARB, (GLint *)&ofs );
	if ( err == GL_INVALID_OPERATION ) {
		const GLubyte *str = glGetString( GL_PROGRAM_ERROR_STRING_ARB );
		common->Printf( "\nGL_PROGRAM_ERROR_STRING_ARB: %s\n", str );
		if ( ofs < 0 ) {
			common->Printf( "GL_PROGRAM_ERROR_POSITION_ARB < 0 with error\n" );
		} else if ( ofs >= (int)strlen( start ) ) {
			common->Printf( "error at end of program\n" );
		} else {
			common->Printf( "error at %i:\n%s", ofs, start + ofs );
		}
		return;
	}
	if ( ofs != -1 ) {
		common->Printf( "\nGL_PROGRAM_ERROR_POSITION_ARB != -1 without error\n" );
		return;
	}

	common->Printf( "\n" );
}

/*
==================
R_FindARBProgram

Returns a GL identifier that can be bound to the given target, parsing
a text file if it hasn't already been loaded.
==================
*/
int R_FindARBProgram( GLenum target, const char *program ) {
	common->Printf( "Search for ARB->GLSL program (%s): ", program );
	int i;
	for (i = 0; arb_to_program[i].program_name[0]; i++) {
		if (idStr::Icmp(arb_to_program[i].program_name, program) == 0) {
			common->Printf("OK\n");
			return arb_to_program[i].id;
		}
	}

	common->Printf("!!! NOT FOUND\n");

	if ( i == MAX_ARB_TO_PROGRAM ) {
		common->Error( "R_FindARBProgram: ARB->GLSL program not found" );
	}

	return 0;
	/*
	int		i;
	idStr	stripped = program;

	stripped.StripFileExtension();

	// see if it is already loaded
	for ( i = 0 ; progs[i].name[0] ; i++ ) {
		if ( progs[i].target != target ) {
			continue;
		}

		idStr	compare = progs[i].name;
		compare.StripFileExtension();

		if ( !idStr::Icmp( stripped.c_str(), compare.c_str() ) ) {
			return progs[i].ident;
		}
	}

	if ( i == MAX_GLPROGS ) {
		common->Error( "R_FindARBProgram: MAX_GLPROGS" );
	}

	// add it to the list and load it
	progs[i].ident = (program_t)0;	// will be gen'd by R_LoadARBProgram
	progs[i].target = target;
	strncpy( progs[i].name, program, sizeof( progs[i].name ) - 1 );

	R_LoadARBProgram( i );

	return progs[i].ident;
	*/
}

/*
==================
R_ReloadARBPrograms_f
==================
*/
void R_ReloadARBPrograms_f( const idCmdArgs &args ) {
	int		i;

	/*
	common->Printf( "----- R_ReloadARBPrograms -----\n" );
	for ( i = 0 ; progs[i].name[0] ; i++ ) {
		R_LoadARBProgram( i );
	}
	*/

	common->Printf("----- R_ReloadShaderPrograms -----\n");
	for (i = 0; shader_progs[i].vs_name[0] ; i++) {
		R_LoadShaderProgram(i);
	}
}

/*
==================
R_ARB2_Init

==================
*/
void R_ARB2_Init( void ) {
	glConfig.allowARB2Path = false;

	common->Printf( "ARB2 renderer: " );

	if ( !glConfig.ARBVertexProgramAvailable || !glConfig.ARBFragmentProgramAvailable ) {
		common->Printf( "Not available.\n" );
		return;
	}

	common->Printf( "Available.\n" );

	glConfig.allowARB2Path = true;
}
