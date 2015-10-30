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

#ifndef a3d_viewbox_H
#define a3d_viewbox_H

#include "a3d_widget.h"
#include "a3d_bulletbox.h"
#include "a3d_screen.h"
#include "../math/a3d_vec4f.h"

typedef struct
{
	a3d_widget_t  widget;

	a3d_bulletbox_t* bullet;
	a3d_widget_t*    body;
} a3d_viewbox_t;

a3d_viewbox_t* a3d_viewbox_new(a3d_screen_t* screen,
                               int wsize,
                               int anchor,
                               int wrapx, int wrapy,
                               int stretch_mode,
                               float stretch_factor,
                               int style_border,
                               int style_line,
                               a3d_vec4f_t* color_fill,
                               a3d_vec4f_t* color_line,
                               int text_anchor,
                               int text_style_border,
                               int text_style_line,
                               int text_style_text,
                               a3d_vec4f_t* text_color_fill,
                               a3d_vec4f_t* text_color_line,
                               a3d_vec4f_t* text_color_text,
                               int text_max_len,
                               const char* sprite,
                               a3d_widget_click_fn click_fn,
                               void* click_priv,
                               a3d_widget_t* body);
void            a3d_viewbox_delete(a3d_viewbox_t** _self);
void            a3d_viewbox_textPrintf(a3d_viewbox_t* self,
                                       const char* fmt, ...);

#endif