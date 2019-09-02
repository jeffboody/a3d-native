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
                                   int wrapx, int wrapy,
                                   int stretch_mode,
                                   float stretch_factor,
                                   int border,
                                   a3d_vec4f_t* color_fill,
                                   int text_wrapx,
                                   int text_border,
                                   int text_size,
                                   a3d_vec4f_t* color_text,
                                   int text_max_len,
                                   int* pvalue)
{
	assert(screen);
	assert(color_fill);
	assert(color_text);
	assert(pvalue);

	if(wsize == 0)
	{
		wsize = sizeof(a3d_radiolist_t);
	}

	a3d_radiolist_t* self = (a3d_radiolist_t*) a3d_listbox_new(screen,
	                                                           wsize,
	                                                           orientation,
	                                                           wrapx, wrapy,
	                                                           stretch_mode,
	                                                           stretch_factor,
	                                                           border,
	                                                           color_fill,
	                                                           NULL,
	                                                           a3d_radiolist_refresh);
	if(self == NULL)
	{
		return NULL;
	}

	self->text_wrapx = text_wrapx;
	self->border     = text_border;
	self->text_size  = text_size;
	self->max_len    = text_max_len;
	self->pvalue     = pvalue;
	self->value      = *pvalue;

	a3d_vec4f_copy(color_text, &self->color_text);

	return self;
}

void a3d_radiolist_delete(a3d_radiolist_t** _self)
{
	assert(_self);

	a3d_radiolist_t* self = *_self;
	if(self)
	{
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

	a3d_widget_t* widget = (a3d_widget_t*) self;
	a3d_radiobox_t* rb;
	rb = a3d_radiobox_new(widget->screen, 0,
	                      self->border, self->text_size,
	                      &self->color_text, self->max_len,
	                      value, self);
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
