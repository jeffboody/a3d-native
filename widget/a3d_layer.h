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

#ifndef a3d_layer_H
#define a3d_layer_H

#include "a3d_widget.h"
#include "a3d_screen.h"
#include "../a3d_list.h"
#include "../math/a3d_vec4f.h"

typedef struct
{
	a3d_widget_t widget;
	a3d_list_t*  list;
} a3d_layer_t;

a3d_layer_t* a3d_layer_new(a3d_screen_t* screen,
                           int wsize,
                           int anchor,
                           int wraph, int wrapv,
                           int style_border,
                           int style_line,
                           a3d_vec4f_t* color_fill,
                           a3d_vec4f_t* color_line);
void         a3d_layer_delete(a3d_layer_t** _self);

#endif