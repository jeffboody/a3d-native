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
#include <stdarg.h>
#include <stdio.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static int a3d_radiobox_click(a3d_widget_t* widget,
                              void* priv, int state,
                              float x, float y)
{
	// priv may be NULL
	assert(widget);

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
                                 a3d_textStyle_t* text_style,
                                 int value,
                                 a3d_radiolist_t* parent)
{
	assert(screen);
	assert(text_style);
	assert(parent);

	if(wsize == 0)
	{
		wsize = sizeof(a3d_radiobox_t);
	}

	a3d_widgetFn_t widget_fn =
	{
		.click_fn = a3d_radiobox_click
	};

	a3d_radiobox_t* self;
	self = (a3d_radiobox_t*)
	       a3d_bulletbox_new(screen, wsize, &widget_fn,
	                         text_style, 2);
	if(self == NULL)
	{
		return NULL;
	}

	a3d_bulletbox_t* bullet = &(self->bullet);
	if(a3d_bulletbox_spriteLoad(bullet, 0,
	                            "$ic_radio_button_unchecked_white_24dp.texz") == 0)
	{
		goto fail_sprite;
	}

	if(a3d_bulletbox_spriteLoad(bullet, 1,
	                            "$ic_radio_button_checked_white_24dp.texz") == 0)
	{
		goto fail_sprite;
	}

	self->value  = value;
	self->parent = parent;

	a3d_radiobox_refresh(self);

	// success
	return self;

	// failure
	fail_sprite:
		a3d_bulletbox_delete((a3d_bulletbox_t**) &self);
	return NULL;
}

void a3d_radiobox_delete(a3d_radiobox_t** _self)
{
	assert(_self);

	a3d_radiobox_t* self = *_self;
	if(self)
	{
		a3d_bulletbox_delete((a3d_bulletbox_t**) _self);
		*_self = NULL;
	}
}

void a3d_radiobox_refresh(a3d_radiobox_t* self)
{
	assert(self);

	a3d_radiolist_t* parent = self->parent;
	a3d_bulletbox_t* bullet = &(self->bullet);
	if(self->value == parent->value)
	{
		a3d_bulletbox_spriteSelect(bullet, 1);
	}
	else
	{
		a3d_bulletbox_spriteSelect(bullet, 0);
	}
}

void a3d_radiobox_textPrintf(a3d_radiobox_t* self,
                             const char* fmt, ...)
{
	assert(self);
	assert(fmt);

	// decode string
	char    string[256];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(string, 256, fmt, argptr);
	va_end(argptr);

	a3d_bulletbox_textPrintf(&self->bullet, "%s", string);
}
