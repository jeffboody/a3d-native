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

#define A3D_TEXT_SIZE_SMALL  0
#define A3D_TEXT_SIZE_MEDIUM 1
#define A3D_TEXT_SIZE_LARGE  2

typedef void (*a3d_text_enterFn)(void* priv,
                                 const char* string);

typedef struct a3d_textFn_s
{
	// functions and priv may be NULL

	void*                priv;
	a3d_text_enterFn     enter_fn;
	a3d_widget_clickFn   click_fn;
	a3d_widget_refreshFn refresh_fn;
} a3d_textFn_t;

typedef struct a3d_textStyle_s
{
	int         font_type;
	int         size;
	a3d_vec4f_t color;
} a3d_textStyle_t;

typedef struct
{
	a3d_widget_t widget;

	// text properties
	a3d_text_enterFn enter_fn;
	a3d_textStyle_t  style;

	// string data
	size_t string_size;
	char*  string;

	// rendering properties
	GLfloat* vertex;
	GLfloat* coords;
	GLuint   id_vertex;
	GLuint   id_coords;
} a3d_text_t;

a3d_text_t* a3d_text_new(a3d_screen_t* screen,
                         int wsize,
                         int border,
                         a3d_textStyle_t* style,
                         a3d_textFn_t* fn);
void        a3d_text_delete(a3d_text_t** _self);
int         a3d_text_width(a3d_text_t* self, int cursor);
int         a3d_text_height(a3d_text_t* self);
void        a3d_text_printf(a3d_text_t* self,
                            const char* fmt, ...);

#endif
