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

#ifndef a3d_text_H
#define a3d_text_H

#include "a3d_widget.h"
#include "a3d_screen.h"
#include "../a3d_GL.h"
#include "../math/a3d_vec4f.h"

#define A3D_TEXT_STYLE_SMALL  0
#define A3D_TEXT_STYLE_MEDIUM 1
#define A3D_TEXT_STYLE_LARGE  2

typedef struct
{
	a3d_widget_t widget;

	// text properties
	// max_len includes null terminator
	int         max_len;
	char*       string;
	int         style;
	a3d_vec4f_t color;

	// rendering properties
	GLfloat* vertex;
	GLfloat* coords;
	GLuint   id_vertex;
	GLuint   id_coords;
} a3d_text_t;

a3d_text_t* a3d_text_new(a3d_screen_t* screen,
                         int wsize,
                         int anchor,
                         int style_border,
                         int style_line,
                         int style_text,
                         a3d_vec4f_t* color_fill,
                         a3d_vec4f_t* color_line,
                         a3d_vec4f_t* color_text,
                         int max_len,
                         a3d_widget_click_fn click_fn);
void        a3d_text_delete(a3d_text_t** _self);
void        a3d_text_printf(a3d_text_t* self,
                            const char* fmt, ...);
void        a3d_text_color(a3d_text_t* self,
                           a3d_vec4f_t* color);

#endif
