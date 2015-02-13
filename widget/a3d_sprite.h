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
#include "a3d_screen.h"
#include "../a3d_GL.h"
#include "../math/a3d_vec4f.h"

typedef struct
{
	a3d_widget_t widget;

	// GL data
	GLuint id_tex;
	GLuint id_vertex;
	GLuint id_coords;

	// shader state
	GLuint prog;
	GLint  attr_vertex;
	GLint  attr_coords;
	GLint  unif_mvp;
	GLint  unif_sampler;
} a3d_sprite_t;

a3d_sprite_t* a3d_sprite_new(a3d_screen_t* screen,
                             int wsize,
                             int anchor,
                             int wraph, int wrapv,
                             int stretch_mode,
                             float stretch_factor,
                             int style_border,
                             int style_line,
                             a3d_vec4f_t* color_fill,
                             a3d_vec4f_t* color_line,
                             a3d_widget_click_fn click_fn,
                             a3d_widget_refresh_fn refresh_fn,
                             const char* fname);
void          a3d_sprite_delete(a3d_sprite_t** _self);

#endif
