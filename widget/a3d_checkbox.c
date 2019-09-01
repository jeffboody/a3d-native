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

#include "a3d_checkbox.h"
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static int a3d_checkbox_click(a3d_widget_t* widget,
                              int state,
                              float x, float y)
{
	assert(widget);

	a3d_checkbox_t* self = (a3d_checkbox_t*) widget;
	if(state == A3D_WIDGET_POINTER_UP)
	{
		*(self->pvalue) = 1 - *(self->pvalue);
	}
	return 1;
}

static void a3d_checkbox_refresh(a3d_widget_t* widget)
{
	assert(widget);

	a3d_checkbox_t*  self   = (a3d_checkbox_t*) widget;
	a3d_bulletbox_t* bullet = &(self->bullet);
	a3d_bulletbox_spriteSelect(bullet, *(self->pvalue));
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_checkbox_t* a3d_checkbox_new(a3d_screen_t* screen,
                                 int wsize,
                                 int border,
                                 int style_text,
                                 a3d_vec4f_t* color_fill,
                                 a3d_vec4f_t* color_text,
                                 int max_len,
                                 int* pvalue)
{
	assert(screen);
	assert(color_fill);
	assert(color_text);
	assert(pvalue);

	if(wsize == 0)
	{
		wsize = sizeof(a3d_checkbox_t);
	}

	a3d_vec4f_t clear =
	{
		.r = 0.0f,
		.g = 0.0f,
		.b = 0.0f,
		.a = 0.0f
	};

	a3d_checkbox_t* self = (a3d_checkbox_t*) a3d_bulletbox_new(screen,
	                                                           wsize,
	                                                           border,
	                                                           style_text,
	                                                           &clear,
	                                                           color_text, color_text,
	                                                           max_len, 2,
	                                                           a3d_checkbox_click,
	                                                           a3d_checkbox_refresh);
	if(self == NULL)
	{
		return NULL;
	}

	a3d_bulletbox_t* bullet = &(self->bullet);
	if(a3d_bulletbox_spriteLoad(bullet, 0,
	                            "$ic_check_box_outline_blank_white_24dp.texz") == 0)
	{
		goto fail_sprite;
	}

	if(a3d_bulletbox_spriteLoad(bullet, 1,
	                            "$ic_check_box_white_24dp.texz") == 0)
	{
		goto fail_sprite;
	}

	self->pvalue = pvalue;

	// success
	return self;

	// failure
	fail_sprite:
		a3d_bulletbox_delete((a3d_bulletbox_t**) &self);
	return NULL;
}

void a3d_checkbox_delete(a3d_checkbox_t** _self)
{
	assert(_self);

	a3d_checkbox_t* self = *_self;
	if(self)
	{
		a3d_bulletbox_delete((a3d_bulletbox_t**) _self);
		*_self = NULL;
	}
}

void a3d_checkbox_textPrintf(a3d_checkbox_t* self,
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

void a3d_checkbox_textWrapx(a3d_checkbox_t* self, int wrapx)
{
	assert(self);

	a3d_bulletbox_textWrapx(&self->bullet, wrapx);
}
