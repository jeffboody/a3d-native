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

#include "a3d_screen.h"
#include "a3d_text.h"
#include "a3d_radiobox.h"
#include "a3d_radiolist.h"
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static void a3d_radiolist_refresh(a3d_widget_t* widget)
{
	assert(widget);
	LOGD("debug");

	a3d_radiolist_t* self = (a3d_radiolist_t*) widget;
	if(self->value != *(self->pvalue))
	{
		self->value = *(self->pvalue);

		a3d_list_t*     list = self->listbox.list;
		a3d_listitem_t* iter = a3d_list_head(list);
		while(iter)
		{
			a3d_radiobox_t* rb = (a3d_radiobox_t*) a3d_list_peekitem(iter);
			a3d_radiobox_refresh(rb);
			iter = a3d_list_next(iter);
		}
	}
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_radiolist_t* a3d_radiolist_new(a3d_screen_t* screen,
                                   int wsize,
                                   int orientation,
                                   int anchor,
                                   int wrapx, int wrapy,
                                   int stretch_mode,
                                   float stretch_factor,
                                   int style_border,
                                   int style_line,
                                   a3d_vec4f_t* color_fill,
                                   a3d_vec4f_t* color_line,
                                   int text_anchor,
                                   int text_wrapx,
                                   int text_style_border,
                                   int text_style_line,
                                   int text_style_text,
                                   a3d_vec4f_t* text_color_fill,
                                   a3d_vec4f_t* text_color_line,
                                   a3d_vec4f_t* text_color_text,
                                   int text_max_len,
                                   int* pvalue)
{
	assert(screen);
	assert(color_fill);
	assert(color_line);
	assert(text_color_fill);
	assert(text_color_line);
	assert(text_color_text);
	assert(pvalue);
	LOGD("debug wsize=%i, orientation=%i, anchor=%i, wrapx=%i, wrapy=%i",
	     wsize, orientation, anchor, wrapx, wrapy);
	LOGD("debug stretch_mode=%i, stretch_factor=%f, style_border=%i, style_line=%i",
	     stretch_mode, stretch_factor, style_border, style_line);
	LOGD("debug color_fill: r=%f, g=%f, b=%f, a=%f",
	     color_fill->r, color_fill->g, color_fill->b, color_fill->a);
	LOGD("debug color_line: r=%f, g=%f, b=%f, a=%f",
	     color_line->r, color_line->g, color_line->b, color_line->a);
	LOGD("text_anchor=%i, text_style_border=%i, text_style_line=%i, text_style_text=%i",
	     text_anchor, text_style_border, text_style_line, text_style_text);
	LOGD("debug color_fill: r=%f, g=%f, b=%f, a=%f",
	     color_fill->r, color_fill->g, color_fill->b, color_fill->a);
	LOGD("debug color_line: r=%f, g=%f, b=%f, a=%f",
	     color_line->r, color_line->g, color_line->b, color_line->a);
	LOGD("debug color_text: r=%f, g=%f, b=%f, a=%f",
	     color_text->r, color_text->g, color_text->b, color_text->a);
	LOGD("debug text_max_len=%i", text_max_len);

	if(wsize == 0)
	{
		wsize = sizeof(a3d_radiolist_t);
	}

	a3d_radiolist_t* self = (a3d_radiolist_t*) a3d_listbox_new(screen,
	                                                           wsize,
	                                                           orientation,
	                                                           anchor,
	                                                           wrapx, wrapy,
	                                                           stretch_mode,
	                                                           stretch_factor,
	                                                           style_border,
	                                                           style_line,
	                                                           color_fill,
	                                                           color_line,
	                                                           NULL,
	                                                           a3d_radiolist_refresh);
	if(self == NULL)
	{
		return NULL;
	}

	self->anchor       = text_anchor;
	self->text_wrapx   = text_wrapx;
	self->style_border = text_style_border;
	self->style_line   = text_style_line;
	self->style_text   = text_style_text;
	self->max_len      = text_max_len;
	self->pvalue       = pvalue;
	self->value        = *pvalue;

	a3d_vec4f_copy(text_color_fill, &self->color_fill);
	a3d_vec4f_copy(text_color_line, &self->color_line);
	a3d_vec4f_copy(text_color_text, &self->color_text);

	return self;
}

void a3d_radiolist_delete(a3d_radiolist_t** _self)
{
	assert(_self);

	a3d_radiolist_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		a3d_radiolist_clear(self);
		a3d_listbox_delete((a3d_listbox_t**) _self);
		*_self = NULL;
	}
}

void a3d_radiolist_clear(a3d_radiolist_t* self)
{
	assert(self);

	a3d_listbox_t*  listbox = (a3d_listbox_t*) self;
	a3d_listitem_t* iter    = a3d_list_head(listbox->list);
	while(iter)
	{
		a3d_radiobox_t* rb;
		rb = (a3d_radiobox_t*) a3d_list_remove(listbox->list, &iter);
		a3d_radiobox_delete(&rb);
	}
}

void a3d_radiolist_value(a3d_radiolist_t* self, int value)
{
	assert(self);
	LOGD("debug value=%i", value);

	*(self->pvalue) = value;
}

void a3d_radiolist_printf(a3d_radiolist_t* self,
                          int value,
                          const char* fmt, ...)
{
	assert(self);
	assert(fmt);

	char* string = (char*) calloc(self->max_len, sizeof(char));
	if(string == NULL)
	{
		LOGE("calloc failed");
		return;
	}

	// decode string
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(string, self->max_len, fmt, argptr);
	va_end(argptr);

	LOGD("debug %s", string);

	a3d_widget_t* widget = (a3d_widget_t*) self;
	a3d_radiobox_t* rb = a3d_radiobox_new(widget->screen,
	                                      0,
	                                      self->anchor,
	                                      self->style_border,
	                                      self->style_line,
	                                      self->style_text,
	                                      &(self->color_fill),
	                                      &(self->color_line),
	                                      &(self->color_text),
	                                      self->max_len,
	                                      value,
	                                      self);
	if(rb == NULL)
	{
		goto fail_rb;
	}
	a3d_radiobox_textWrapx(rb, self->text_wrapx);

	a3d_listbox_t* listbox = (a3d_listbox_t*) self;
	if(a3d_list_enqueue(listbox->list, (const void*) rb) == 0)
	{
		goto fail_enqueue;
	}

	a3d_radiobox_textPrintf(rb, "%s", string);
	free(string);

	// success
	return;

	// failure
	fail_enqueue:
		a3d_radiobox_delete(&rb);
	fail_rb:
		free(string);
}
