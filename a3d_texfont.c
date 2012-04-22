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

#include "a3d_texfont.h"
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
} a3d_texfont_index_t;

static const a3d_texfont_index_t INDEX[128] =
{
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
	{ 3, 16 },   // null
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
	{ 3, 16 },   // null
};

/***********************************************************
* public                                                   *
***********************************************************/

a3d_texfont_t* a3d_texfont_new(const char* fname)
{
	assert(fname);
	LOGD("debug");

	a3d_texfont_t* self = (a3d_texfont_t*) malloc(sizeof(a3d_texfont_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		goto fail_malloc;
	}

	self->tex = a3d_tex_new(fname);
	if(self->tex == NULL)
	{
		// log message already output
		goto fail_tex;
	}

	if(a3d_tex_load(self->tex) == A3D_TEX_ERROR)
	{
		// log message already output
		goto fail_load;
	}

	self->char_w = self->tex->width / 26;
	self->char_h = self->tex->height / 4;

	glGenTextures(1, &self->tex->id);
	glBindTexture(GL_TEXTURE_2D, self->tex->id);
	#if defined(A3D_GLESv1_CM)
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	#else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	#endif
	glTexImage2D(GL_TEXTURE_2D, 0, self->tex->format, self->tex->stride, self->tex->vstride,
	             0, self->tex->format, self->tex->type, self->tex->pixels);
	a3d_tex_reclaim(self->tex);

	// success
	return self;

	// failure
	fail_load:
		a3d_tex_delete(&self->tex);
	fail_tex:
		free(self);
	fail_malloc:
		return NULL;
}

void a3d_texfont_delete(a3d_texfont_t** _self)
{
	// *_self can be null
	assert(_self);

	a3d_texfont_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &self->tex->id);
		self->tex->id = 0;

		a3d_tex_delete(&self->tex);
		free(self);
		*_self = NULL;
	}
}

void a3d_texfont_request(a3d_texfont_t* self, char c,
                         a3d_regionf_t* tc, a3d_regionf_t* vc)
{
	assert(self);
	assert(tc);
	assert(vc);
	LOGD("debug");

	// clamp to the supported range
	if((c < 0) || (c > 127)) c = '\0';

	a3d_texfont_index_t index = INDEX[(int) c];
	float stride  = (float) self->tex->stride;
	float vstride = (float) self->tex->vstride;
	float row     = (float) index.row;
	float col     = (float) index.col;
	float char_w  = (float) self->char_w;
	float char_h  = (float) self->char_h;

	// fill in tex coords
	tc->t = row     * char_h / vstride;
	tc->l = col     * char_w / stride;
	tc->b = (row+1) * char_h / vstride;
	tc->r = (col+1) * char_w / stride;

	// fill in vertex coords
	vc->t = 0.0f;
	vc->l = 0.0f;
	vc->b = 1.0f;
	vc->r = char_w / char_h;
}

float a3d_texfont_aspect_ratio(a3d_texfont_t* self)
{
	assert(self);
	LOGD("debug");

	return (float) self->char_w / (float) self->char_h;
}
