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

#include "a3d_radiobox.h"
#include <stdlib.h>
#include <assert.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static int a3d_radiobox_click(a3d_widget_t* widget,
                              int state,
                              float x, float y)
{
	assert(widget);
	LOGD("debug state=%i, x=%f, y=%f", state, x, y);

	a3d_radiobox_t* self = (a3d_radiobox_t*) widget;
	if(state == A3D_WIDGET_POINTER_UP)
	{
		a3d_radiolist_value(self->parent, self->value);
	}

	return 1;
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_radiobox_t* a3d_radiobox_new(a3d_screen_t* screen,
                                 int wsize,
                                 int anchor,
                                 int style_border,
                                 int style_line,
                                 int style_text,
                                 a3d_vec4f_t* color_fill,
                                 a3d_vec4f_t* color_line,
                                 a3d_vec4f_t* color_text,
                                 int max_len,
                                 int indent,
                                 int value,
                                 a3d_radiolist_t* parent)
{
	assert(screen);
	assert(color_fill);
	assert(color_line);
	assert(color_text);
	assert(parent);
	LOGD("debug wsize=%i, anchor=%i, style_border=%i, style_line=%i, style_text=%i",
	     wsize, anchor, style_border, style_line, style_text);
	LOGD("debug color_fill: r=%f, g=%f, b=%f, a=%f",
	     color_fill->r, color_fill->g, color_fill->b, color_fill->a);
	LOGD("debug color_line: r=%f, g=%f, b=%f, a=%f",
	     color_line->r, color_line->g, color_line->b, color_line->a);
	LOGD("debug color_text: r=%f, g=%f, b=%f, a=%f",
	     color_text->r, color_text->g, color_text->b, color_text->a);
	LOGD("debug max_len=%i, indent=%i", max_len, indent);

	if(wsize == 0)
	{
		wsize = sizeof(a3d_radiobox_t);
	}

	a3d_radiobox_t* self = (a3d_radiobox_t*) a3d_text_new(screen,
	                                                      wsize,
	                                                      anchor,
	                                                      style_border,
	                                                      style_line,
	                                                      style_text,
	                                                      color_fill,
	                                                      color_line,
	                                                      color_text,
	                                                      max_len,
	                                                      indent,
	                                                      a3d_radiobox_click,
	                                                      NULL);
	if(self == NULL)
	{
		return NULL;
	}

	self->value  = value;
	self->parent = parent;

	a3d_text_prefix((a3d_text_t*) self, "(%s) ",
	                (self->value == *(parent->pvalue)) ? "*" : " ");

	return self;
}

void a3d_radiobox_delete(a3d_radiobox_t** _self)
{
	assert(_self);

	a3d_radiobox_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		a3d_text_delete((a3d_text_t**) _self);
		*_self = NULL;
	}
}

void a3d_radiobox_refresh(a3d_radiobox_t* self)
{
	assert(self);
	LOGD("debug");

	a3d_radiolist_t* parent = self->parent;
	a3d_text_prefix((a3d_text_t*) self, "(%s) ",
	                (self->value == *(parent->pvalue)) ? "*" : " ");
}
