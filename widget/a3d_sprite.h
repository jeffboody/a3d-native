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

#ifndef a3d_sprite_H
#define a3d_sprite_H

#include "a3d_widget.h"
#include "../a3d_GL.h"
#include "../math/a3d_vec4f.h"

struct a3d_screen_s;

#define A3D_SPRITESHADER_ALPHA 0
#define A3D_SPRITESHADER_COLOR 1

typedef struct
{
	GLuint prog;
	GLint  attr_vertex;
	GLint  attr_coords;
	GLint  unif_color;
	GLint  unif_mvp;
	GLint  unif_sampler;
} a3d_spriteShader_t;

a3d_spriteShader_t* a3d_spriteShader_new(int format);
void                a3d_spriteShader_delete(a3d_spriteShader_t** _self);

typedef struct
{
	int    ref_count;
	char   fname[256];
	int    format;
	GLuint id_tex;
} a3d_spriteTex_t;

a3d_spriteTex_t* a3d_spriteTex_new(const char* fname,
                                   const char* resource);
void             a3d_spriteTex_delete(a3d_spriteTex_t** _self);
void             a3d_spriteTex_incRef(a3d_spriteTex_t* self);
int              a3d_spriteTex_decRef(a3d_spriteTex_t* self);

typedef struct
{
	a3d_widget_t widget;

	// texture(s)
	int sprite_count;
	int index;

	// GL properties
	a3d_vec4f_t color;
	float       theta;

	// GL data
	int*    format;
	GLuint* id_tex;
	GLuint  id_vertex;
	GLuint  id_coords;
} a3d_sprite_t;

a3d_sprite_t* a3d_sprite_new(struct a3d_screen_s* screen,
                             int wsize,
                             int border,
                             a3d_widgetLayout_t* layout,
                             a3d_vec4f_t* color,
                             int sprite_count,
                             void* priv,
                             a3d_widget_clickFn click_fn,
                             a3d_widget_refreshFn refresh_fn);
void          a3d_sprite_delete(a3d_sprite_t** _self);
int           a3d_sprite_load(a3d_sprite_t* self,
                              int index,
                              const char* fname);
void          a3d_sprite_select(a3d_sprite_t* self,
                                int index);
void          a3d_sprite_rotate(a3d_sprite_t* self,
                                float theta);

#endif
