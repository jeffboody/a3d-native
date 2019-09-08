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

static void
a3d_radiolist_refresh(a3d_widget_t* widget, void* priv)
{
	assert(widget);
	assert(priv == NULL);

	a3d_radiolist_t* self = (a3d_radiolist_t*) widget;
	if(self->value != *(self->pvalue))
	{
		self->value = *(self->pvalue);

		a3d_listbox_t*  listbox = (a3d_listbox_t*) self;
		a3d_list_t*     widgets = a3d_listbox_widgets(listbox);
		a3d_listitem_t* iter    = a3d_list_head(widgets);
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
                                   a3d_widgetLayout_t* layout,
                                   a3d_widgetScroll_t* scroll,
                                   a3d_textStyle_t* text_style,
                                   int* pvalue)
{
	assert(screen);
	assert(layout);
	assert(scroll);
	assert(text_style);
	assert(pvalue);

	if(wsize == 0)
	{
		wsize = sizeof(a3d_radiolist_t);
	}

	a3d_widgetFn_t fn =
	{
		.refresh_fn = a3d_radiolist_refresh,
	};

	a3d_radiolist_t* self;
	self = (a3d_radiolist_t*)
	       a3d_listbox_new(screen, wsize,
	                       layout, scroll, &fn,
	                       A3D_LISTBOX_ORIENTATION_VERTICAL);
	if(self == NULL)
	{
		return NULL;
	}

	self->pvalue = pvalue;
	self->value  = *pvalue;

	memcpy(&self->text_style, text_style,
	       sizeof(a3d_textStyle_t));

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
	a3d_list_t*     widgets = a3d_listbox_widgets(listbox);
	a3d_listitem_t* iter    = a3d_list_head(widgets);
	while(iter)
	{
		a3d_radiobox_t* rb;
		rb = (a3d_radiobox_t*) a3d_list_remove(widgets, &iter);
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

	// decode string
	char string[256];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(string, 256, fmt, argptr);
	va_end(argptr);

	a3d_widget_t* widget = (a3d_widget_t*) self;
	a3d_radiobox_t* rb;
	rb = a3d_radiobox_new(widget->screen, 0,
	                      &self->text_style, value, self);
	if(rb == NULL)
	{
		return;
	}

	a3d_listbox_t* listbox = (a3d_listbox_t*) self;
	a3d_list_t*    widgets = a3d_listbox_widgets(listbox);
	if(a3d_list_enqueue(widgets, (const void*) rb) == 0)
	{
		goto fail_enqueue;
	}

	a3d_radiobox_textPrintf(rb, "%s", string);

	// success
	return;

	// failure
	fail_enqueue:
		a3d_radiobox_delete(&rb);
}
