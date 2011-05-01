/*
 * Copyright (c) 2009-2010 Jeff Boody
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "a3d_texstring.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define LOG_TAG "a3d"
#include "a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static const int A3D_TEXSTRING_TOP    = 0x10;
static const int A3D_TEXSTRING_BOTTOM = 0x20;
static const int A3D_TEXSTRING_LEFT   = 0x01;
static const int A3D_TEXSTRING_CENTER = 0x02;
static const int A3D_TEXSTRING_RIGHT  = 0x04;

#ifdef A3D_GLESv2
	#include "a3d_shader.h"

	static const char* VSHADER =
		"attribute vec3 vertex;\n"
		"attribute vec2 coords;\n"
		"uniform   mat4 mvp;\n"
		"varying   vec2 varying_coords;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	varying_coords = coords;\n"
		"	gl_Position = mvp * vec4(vertex, 1.0);\n"
		"}\n";

	static const char* FSHADER =
		"precision mediump float;\n"
		"precision mediump int;\n"
		"\n"
		"uniform vec4      color;\n"
		"uniform sampler2D sampler;\n"
		"varying vec2      varying_coords;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	gl_FragColor = color * texture2D(sampler, varying_coords);\n"
		"}\n";

	static int a3d_texstring_load_shaders(a3d_texstring_t* self)
	{
		assert(self);
		LOGD("debug");

		self->program = a3d_shader_make_source(VSHADER, FSHADER);
		if(self->program == 0)
			return 0;

		self->attribute_vertex = glGetAttribLocation(self->program, "vertex");
		self->attribute_coords = glGetAttribLocation(self->program, "coords");
		self->uniform_color    = glGetUniformLocation(self->program, "color");
		self->uniform_mvp      = glGetUniformLocation(self->program, "mvp");
		self->uniform_sampler  = glGetUniformLocation(self->program, "sampler");

		GLenum e = A3D_GL_GETERROR();
		if(e != GL_NO_ERROR)
			goto fail_error;

		// success
		return 1;

		// failure
		fail_error:
			glDeleteProgram(self->program);
		return 0;
	}
#endif

/***********************************************************
* public                                                   *
***********************************************************/

const int A3D_TEXSTRING_TOP_LEFT      = 0x11;
const int A3D_TEXSTRING_TOP_CENTER    = 0x12;
const int A3D_TEXSTRING_TOP_RIGHT     = 0x14;
const int A3D_TEXSTRING_BOTTOM_LEFT   = 0x21;
const int A3D_TEXSTRING_BOTTOM_CENTER = 0x22;
const int A3D_TEXSTRING_BOTTOM_RIGHT  = 0x24;

a3d_texstring_t* a3d_texstring_new(a3d_texfont_t* font, int max_len,
                                   float size, int justify,
                                   float r, float g, float b, float a)
{
	assert(font);
	assert(max_len > 0);
	assert(size > 0.0f);
	assert((r >= 0.0f) && (r <= 1.0f));
	assert((g >= 0.0f) && (g <= 1.0f));
	assert((b >= 0.0f) && (b <= 1.0f));
	LOGD("debug max_len=%i, size=%f, r=%f, g=%f, b=%f", max_len, size, r, g, b);

	a3d_texstring_t* self = (a3d_texstring_t*) malloc(sizeof(a3d_texstring_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->string = (char*) malloc(sizeof(char) * max_len);
	if(self->string == NULL)
	{
		LOGE("malloc failed");
		goto fail_string;
	}
	memset(self->string, 0, max_len);

	int vertex_size = 18 * (max_len - 1);   // 2 * 3 * xyz
	self->vertex = (GLfloat*) malloc(sizeof(GLfloat) * vertex_size);
	if(self->vertex == NULL)
	{
		LOGE("malloc failed");
		goto fail_vertex;
	}

	int coords_size = 12 * (max_len - 1);   // 2 * 3 * uv
	self->coords = (GLfloat*) malloc(sizeof(GLfloat) * coords_size);
	if(self->coords == NULL)
	{
		LOGE("malloc failed");
		goto fail_coords;
	}

	#ifdef A3D_GLESv2
		if(a3d_texstring_load_shaders(self) == 0)
			goto fail_shader;

		a3d_mat4f_identity(&self->pm);
		a3d_mat4f_identity(&self->mvm);
	#endif

	self->size     = size;
	self->justify  = justify;
	a3d_vec4f_load(&self->color, r, g, b, a);
	self->max_len  = max_len;
	self->font     = font;

	// success
	return self;

	// failure
	#ifdef A3D_GLESv2
		fail_shader:
			free(self->coords);
	#endif
	fail_coords:
		free(self->vertex);
	fail_vertex:
		free(self->string);
	fail_string:
		free(self);
	return NULL;
}

void a3d_texstring_delete(a3d_texstring_t** _self)
{
	// *_self can be null
	assert(_self);

	a3d_texstring_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		#ifdef A3D_GLESv2
			glDeleteProgram(self->program);
		#endif

		free(self->coords);
		free(self->vertex);
		free(self->string);
		free(self);
		*_self = NULL;
	}
}

void a3d_texstring_printf(a3d_texstring_t* self, const char* fmt, ...)
{
	assert(self);
	assert(fmt);

	// decode string
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(self->string, self->max_len, fmt, argptr);
	va_end(argptr);

	LOGD("debug %s", self->string);

	// compute vertex/coords for glDrawArrays
	// quad vertex/coords order
	// bl.xyz/uv, tl.xyz/uv, br.xyz/uv, tr.xyz/uv
	int i;
	a3d_regionf_t coords;
	a3d_regionf_t vertex;
	int len = strlen(self->string);
	float offset = 0.0f;
	for(i = 0; i < len; ++i)
	{
		a3d_texfont_request(self->font, self->string[i], &coords, &vertex);

		self->vertex[18*i +  0] = vertex.l + offset;   // tl.xyz
		self->vertex[18*i +  1] = vertex.t;
		self->vertex[18*i +  2] = 0.0f;
		self->vertex[18*i +  3] = vertex.l + offset;   // bl.xyz
		self->vertex[18*i +  4] = vertex.b;
		self->vertex[18*i +  5] = 0.0f;
		self->vertex[18*i +  6] = vertex.r + offset;   // br.xyz
		self->vertex[18*i +  7] = vertex.b;
		self->vertex[18*i +  8] = 0.0f;

		self->vertex[18*i +  9] = vertex.l + offset;   // tl.xyz
		self->vertex[18*i + 10] = vertex.t;
		self->vertex[18*i + 11] = 0.0f;
		self->vertex[18*i + 12] = vertex.r + offset;   // br.xyz
		self->vertex[18*i + 13] = vertex.b;
		self->vertex[18*i + 14] = 0.0f;
		self->vertex[18*i + 15] = vertex.r + offset;   // tr.xyz
		self->vertex[18*i + 16] = vertex.t;
		self->vertex[18*i + 17] = 0.0f;

		self->coords[12*i + 0] = coords.l;   // tl.uv
		self->coords[12*i + 1] = coords.t;
		self->coords[12*i + 2] = coords.l;   // bl.uv
		self->coords[12*i + 3] = coords.b;
		self->coords[12*i + 4] = coords.r;   // br.uv
		self->coords[12*i + 5] = coords.b;

		self->coords[12*i +  6] = coords.l;   // tl.uv
		self->coords[12*i +  7] = coords.t;
		self->coords[12*i +  8] = coords.r;   // br.uv
		self->coords[12*i +  9] = coords.b;
		self->coords[12*i + 10] = coords.r;   // tr.uv
		self->coords[12*i + 11] = coords.t;

		// next character offset
		offset += vertex.r;
	}
}

void a3d_texstring_draw(a3d_texstring_t* self,
                        float x, float y,
                        float screen_w, float screen_h)
{
	assert(self);
	LOGD("debug x=%f, y=%f, string=%s", x, y, self->string);

	int len = strlen(self->string);
	if(len <= 0) return;

	// compute justify offsets
	float string_w   = (float) len * a3d_texfont_aspect_ratio(self->font) * self->size;
	float x_offset = 0;
	float y_offset = 0;
	if     (self->justify & A3D_TEXSTRING_CENTER) x_offset = -0.5f * string_w;
	else if(self->justify & A3D_TEXSTRING_RIGHT)  x_offset = -string_w;
	if     (self->justify & A3D_TEXSTRING_BOTTOM) y_offset = -self->size;

	// draw the string
	#ifdef A3D_GLESv1_CM
		glEnable(GL_TEXTURE_2D);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, self->font->tex->id);
		glVertexPointer(3, GL_FLOAT, 0, self->vertex);
		glTexCoordPointer(2, GL_FLOAT, 0, self->coords);
		glPushMatrix();
		glColor4f(self->color.x, self->color.y, self->color.z, 1.0f);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrthof(0.0f, screen_w, screen_h, 0.0f, 0.0f, 2.0f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(x + x_offset, y + y_offset, -1.0f);
		glScalef(self->size, self->size, 1.0f);
		glDrawArrays(GL_TRIANGLES, 0, 2 * 3 * len);
		glPopMatrix();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glDisable(GL_BLEND);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);
	#else
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, self->font->tex->id);
		glUseProgram(self->program);
		glEnableVertexAttribArray(self->attribute_vertex);
		glEnableVertexAttribArray(self->attribute_coords);
		glVertexAttribPointer(self->attribute_vertex, 3, GL_FLOAT, GL_FALSE, 0, self->vertex);
		glVertexAttribPointer(self->attribute_coords, 2, GL_FLOAT, GL_FALSE, 0, self->coords);
		glUniform4fv(self->uniform_color, 1, (GLfloat*) &self->color);
		glUniform1i(self->uniform_sampler, 0);
		a3d_mat4f_ortho(&self->pm, 1, 0.0f, screen_w, screen_h, 0.0f, 0.0f, 2.0f);
		a3d_mat4f_translate(&self->mvm, 1, x + x_offset, y + y_offset, -1.0f);
		a3d_mat4f_scale(&self->mvm, 0, self->size, self->size, 1.0f);
		a3d_mat4f_t mvp;
		a3d_mat4f_mulm_copy(&self->pm, &self->mvm, &mvp);
		glUniformMatrix4fv(self->uniform_mvp, 1, GL_FALSE, (GLfloat*) &mvp);
		glDrawArrays(GL_TRIANGLES, 0, 2 * 3 * len);
		glDisableVertexAttribArray(self->attribute_coords);
		glDisableVertexAttribArray(self->attribute_vertex);
		glUseProgram(0);
		glDisable(GL_BLEND);
	#endif

	A3D_GL_GETERROR();
}
