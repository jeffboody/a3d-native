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

#ifndef a3d_radiolist_H
#define a3d_radiolist_H

#include "a3d_widget.h"
#include "a3d_listbox.h"
#include "a3d_screen.h"
#include "../math/a3d_vec4f.h"

typedef struct
{
	a3d_listbox_t listbox;
	int*          pvalue;
	int           value;

	// text properties
	int         text_wrapx;
	int         border;
	int         text_size;
	a3d_vec4f_t color_fill;
	a3d_vec4f_t color_text;
} a3d_radiolist_t;

a3d_radiolist_t* a3d_radiolist_new(a3d_screen_t* screen,
                                   int wsize,
                                   int orientation,
                                   a3d_widgetLayout_t* layout,
                                   int border,
                                   a3d_vec4f_t* color_fill,
                                   int text_wrapx,
                                   int text_border,
                                   int text_size,
                                   a3d_vec4f_t* color_text,
                                   int* pvalue,
                                   int scroll_bar,
                                   a3d_vec4f_t* color_scroll0,
                                   a3d_vec4f_t* color_scroll1);
void            a3d_radiolist_delete(a3d_radiolist_t** _self);
void            a3d_radiolist_clear(a3d_radiolist_t* self);
void            a3d_radiolist_value(a3d_radiolist_t* self, int value);
void            a3d_radiolist_printf(a3d_radiolist_t* self,
                                     int value,
                                     const char* fmt, ...);

#endif
