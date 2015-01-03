/*
 * Copyright (c) 2015 Jeff Boody
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

#include "a3d_font.h"
#include "../a3d_shader.h"
#include <texgz/texgz_tex.h>
#include <stdlib.h>
#include <assert.h>

#define LOG_TAG "a3d"
#include "a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

typedef struct
{
	short row;
	short col;
} a3d_fontindex_t;

static const a3d_fontindex_t INDEX[128] =
{
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 25 },   // null
	{ 3, 16 },   // space
	{ 3,  0 },   // !
	{ 3,  8 },   // "
	{ 3,  2 },   // #
	{ 3,  3 },   // $
	{ 3,  4 },   // %
	{ 3,  6 },   // &
	{ 3,  9 },   // '
	{ 2, 10 },   // (
	{ 2, 11 },   // )
	{ 3,  7 },   // *
	{ 2, 19 },   // +
	{ 3, 12 },   // ,
	{ 2, 18 },   // -
	{ 3, 13 },   // .
	{ 2, 21 },   // /
	{ 2,  0 },   // 0
	{ 2,  1 },   // 1
	{ 2,  2 },   // 2
	{ 2,  3 },   // 3
	{ 2,  4 },   // 4
	{ 2,  5 },   // 5
	{ 2,  6 },   // 6
	{ 2,  7 },   // 7
	{ 2,  8 },   // 8
	{ 2,  9 },   // 9
	{ 2, 24 },   // :
	{ 2, 25 },   // ;
	{ 2, 16 },   // <
	{ 2, 20 },   // =
	{ 2, 17 },   // >
	{ 3, 15 },   // ?
	{ 3,  1 },   // @
	{ 1,  0 },   // A
	{ 1,  1 },   // B
	{ 1,  2 },   // C
	{ 1,  3 },   // D
	{ 1,  4 },   // E
	{ 1,  5 },   // F
	{ 1,  6 },   // G
	{ 1,  7 },   // H
	{ 1,  8 },   // I
	{ 1,  9 },   // J
	{ 1, 10 },   // K
	{ 1, 11 },   // L
	{ 1, 12 },   // M
	{ 1, 13 },   // N
	{ 1, 14 },   // O
	{ 1, 15 },   // P
	{ 1, 16 },   // Q
	{ 1, 17 },   // R
	{ 1, 18 },   // S
	{ 1, 19 },   // T
	{ 1, 20 },   // U
	{ 1, 21 },   // V
	{ 1, 22 },   // W
	{ 1, 23 },   // X
	{ 1, 24 },   // Y
	{ 1, 25 },   // Z
	{ 2, 12 },   // [
	{ 2, 22 },   // back-slash
	{ 2, 13 },   // ]
	{ 3,  5 },   // ^
	{ 3, 14 },   // _
	{ 3, 10 },   // `
	{ 0,  0 },   // a
	{ 0,  1 },   // b
	{ 0,  2 },   // c
	{ 0,  3 },   // d
	{ 0,  4 },   // e
	{ 0,  5 },   // f
	{ 0,  6 },   // g
	{ 0,  7 },   // h
	{ 0,  8 },   // i
	{ 0,  9 },   // j
	{ 0, 10 },   // k
	{ 0, 11 },   // l
	{ 0, 12 },   // m
	{ 0, 13 },   // n
	{ 0, 14 },   // o
	{ 0, 15 },   // p
	{ 0, 16 },   // q
	{ 0, 17 },   // r
	{ 0, 18 },   // s
	{ 0, 19 },   // t
	{ 0, 20 },   // u
	{ 0, 21 },   // v
	{ 0, 22 },   // w
	{ 0, 23 },   // x
	{ 0, 24 },   // y
	{ 0, 25 },   // z
	{ 2, 14 },   // {
	{ 2, 23 },   // |
	{ 2, 15 },   // }
	{ 3, 11 },   // ~
	{ 3, 25 },   // null
};

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
	"#ifdef GL_ES\n"
	"precision mediump float;\n"
	"precision mediump int;\n"
	"#endif\n"
	"\n"
	"uniform vec4      color;\n"
	"uniform sampler2D sampler;\n"
	"varying vec2      varying_coords;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	vec4 c = color*texture2D(sampler, varying_coords);\n"
	"	gl_FragColor = c;\n"
	"}\n";

static int a3d_font_loadShaders(a3d_font_t* self)
{
	assert(self);
	LOGD("debug");

	self->prog = a3d_shader_make_source(VSHADER, FSHADER);
	if(self->prog == 0)
	{
		return 0;
	}

	self->attr_vertex  = glGetAttribLocation(self->prog, "vertex");
	self->attr_coords  = glGetAttribLocation(self->prog, "coords");
	self->unif_color   = glGetUniformLocation(self->prog, "color");
	self->unif_mvp     = glGetUniformLocation(self->prog, "mvp");
	self->unif_sampler = glGetUniformLocation(self->prog, "sampler");

	GLenum e = A3D_GL_GETERROR();
	if(e != GL_NO_ERROR)
	{
		LOGE("GL error = 0x%X", e);
		goto fail_error;
	}

	// success
	return 1;

	// failure
	fail_error:
		glDeleteProgram(self->prog);
	return 0;
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_font_t* a3d_font_new(const char* fname)
{
	assert(fname);
	LOGD("debug");

	a3d_font_t* self = (a3d_font_t*) malloc(sizeof(a3d_font_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	if(a3d_font_loadShaders(self) == 0)
	{
		goto fail_shaders;
	}

	texgz_tex_t* tex = texgz_tex_import(fname);
	if(tex == NULL)
	{
		goto fail_tex;
	}

	glGenTextures(1, &self->id_tex);
	glBindTexture(GL_TEXTURE_2D, self->id_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, tex->format, tex->stride, tex->vstride,
	             0, tex->format, tex->type, tex->pixels);

	self->char_w = tex->width / 26;
	self->char_h = tex->height / 4;
	self->width  = tex->stride;
	self->height = tex->vstride;

	texgz_tex_delete(&tex);

	// success
	return self;

	// failure
	fail_tex:
		glDeleteProgram(self->prog);
	fail_shaders:
		free(self);
	return NULL;
}

void a3d_font_delete(a3d_font_t** _self)
{
	assert(_self);

	a3d_font_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		glUseProgram(0);
		glDeleteProgram(self->prog);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &self->id_tex);
		free(self);
		*_self = NULL;
	}
}

void a3d_font_request(a3d_font_t* self,
                      char c,
                      a3d_regionf_t* tc,
                      a3d_regionf_t* vc)
{
	assert(self);
	assert(tc);
	assert(vc);
	LOGD("debug");

	// clamp to the supported range
	if((c < 0) || (c > 127)) c = '\0';

	a3d_fontindex_t index = INDEX[(int) c];
	float width  = (float) self->width;
	float height = (float) self->height;
	float row    = (float) index.row;
	float col    = (float) index.col;
	float char_w = (float) self->char_w;
	float char_h = (float) self->char_h;

	// fill in tex coords
	tc->t = row     * char_h / height;
	tc->l = col     * char_w / width;
	tc->b = (row+1) * char_h / height;
	tc->r = (col+1) * char_w / width;

	// fill in vertex coords
	vc->t = 0.0f;
	vc->l = 0.0f;
	vc->b = 1.0f;
	vc->r = char_w / char_h;
}

float a3d_font_aspectRatio(a3d_font_t* self)
{
	assert(self);
	LOGD("debug");

	return (float) self->char_w / (float) self->char_h;
}
