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

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include "a3d_bulletbox.h"

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

#define A3D_BULLETBOX_SPACE 1.25f

static void a3d_bulletbox_size(a3d_widget_t* widget,
                               float* w, float* h)
{
	assert(widget);
	assert(w);
	assert(h);

	a3d_bulletbox_t* self = (a3d_bulletbox_t*) widget;
	a3d_widget_t*    icon = (a3d_widget_t*) self->icon;
	a3d_widget_t*    text = (a3d_widget_t*) self->text;

	float icon_w = *w;
	float icon_h = *h;
	a3d_widget_layoutSize(icon, &icon_w, &icon_h);

	float text_w = *w;
	float text_h = *h;
	a3d_widget_layoutSize(text, &text_w, &text_h);

	*w = A3D_BULLETBOX_SPACE*icon_w + text_w;
	*h = (icon_h > text_h) ? icon_h : text_h;
}

static void a3d_bulletbox_layout(a3d_widget_t* widget,
                                 int dragx, int dragy)
{
	assert(widget);

	a3d_bulletbox_t* self = (a3d_bulletbox_t*) widget;
	a3d_widget_t*    icon = (a3d_widget_t*) self->icon;
	a3d_widget_t*    text = (a3d_widget_t*) self->text;

	// initialize the layout
	float x  = 0.0f;
	float y  = 0.0f;
	float t  = widget->rect_draw.t;
	float l  = widget->rect_draw.l;
	float h  = widget->rect_draw.h;
	float iw = icon->rect_border.w;
	float tw = text->rect_border.w;

	// layout icon
	a3d_rect4f_t rect_draw;
	rect_draw.t = t;
	rect_draw.l = l;
	rect_draw.w = iw;
	rect_draw.h = h;
	a3d_rect4f_t rect_clip;
	a3d_widget_layoutAnchor(icon, &rect_draw, &x, &y);
	a3d_rect4f_intersect(&rect_draw,
	                     &widget->rect_clip,
	                     &rect_clip);
	a3d_widget_layoutXYClip(icon, x, y, &rect_clip,
	                        dragx, dragy);

	// layout text
	rect_draw.l = l + A3D_BULLETBOX_SPACE*iw;
	rect_draw.w = tw;
	a3d_widget_layoutAnchor(text, &rect_draw, &x, &y);
	a3d_rect4f_intersect(&rect_draw,
	                     &widget->rect_clip,
	                     &rect_clip);
	a3d_widget_layoutXYClip(text, x, y, &rect_clip,
	                        dragx, dragy);
}

static void a3d_bulletbox_drag(a3d_widget_t* widget,
                               float x, float y,
                               float dx, float dy)
{

	a3d_bulletbox_t* self = (a3d_bulletbox_t*) widget;
	a3d_widget_drag((a3d_widget_t*) self->icon, x, y, dx, dy);
	a3d_widget_drag((a3d_widget_t*) self->text, x, y, dx, dy);
}

static void a3d_bulletbox_draw(a3d_widget_t* widget)
{
	assert(widget);

	a3d_bulletbox_t* self = (a3d_bulletbox_t*) widget;
	a3d_widget_draw((a3d_widget_t*) self->icon);
	a3d_widget_draw((a3d_widget_t*) self->text);
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_bulletbox_t* a3d_bulletbox_new(a3d_screen_t* screen,
                                   int wsize,
                                   a3d_widgetFn_t* fn,
                                   a3d_textStyle_t* text_style,
                                   int sprite_count)
{
	assert(screen);
	assert(fn);
	assert(text_style);

	if(wsize == 0)
	{
		wsize = sizeof(a3d_bulletbox_t);
	}

	a3d_widgetLayout_t widget_layout =
	{
		.wrapx      = A3D_WIDGET_WRAP_SHRINK,
		.wrapy      = A3D_WIDGET_WRAP_SHRINK,
		.aspectx    = A3D_WIDGET_ASPECT_DEFAULT,
		.aspecty    = A3D_WIDGET_ASPECT_DEFAULT,
		.stretchx   = 1.0f,
		.stretchy   = 1.0f
	};

	a3d_widgetStyle_t style;
	memset(&style, 0, sizeof(a3d_widgetStyle_t));

	a3d_widgetScroll_t scroll =
	{
		.scroll_bar = 0
	};

	a3d_widgetPrivFn_t priv_fn =
	{
		.size_fn   = a3d_bulletbox_size,
		.layout_fn = a3d_bulletbox_layout,
		.drag_fn   = a3d_bulletbox_drag,
		.draw_fn   = a3d_bulletbox_draw,
	};

	a3d_bulletbox_t* self;
	self = (a3d_bulletbox_t*)
	       a3d_widget_new(screen, wsize, &widget_layout,
	                      &style, &scroll, fn, &priv_fn);
	if(self == NULL)
	{
		return NULL;
	}

	int wrap = A3D_WIDGET_WRAP_STRETCH_TEXT_MEDIUM;
	if(text_style->size == A3D_TEXT_SIZE_LARGE)
	{
		wrap = A3D_WIDGET_WRAP_STRETCH_TEXT_LARGE;
	}
	else if(text_style->size == A3D_TEXT_SIZE_SMALL)
	{
		wrap = A3D_WIDGET_WRAP_STRETCH_TEXT_SMALL;
	}

	// convert spacing to border
	int spacing = text_style->spacing;
	int border  = spacing | (spacing >> 4);

	a3d_widgetLayout_t sprite_layout =
	{
		.border   = border,
		.wrapx    = wrap,
		.wrapy    = wrap,
		.aspectx  = A3D_WIDGET_ASPECT_SQUARE,
		.aspecty  = A3D_WIDGET_ASPECT_SQUARE,
		.stretchx = 1.0f,
		.stretchy = 1.0f
	};

	a3d_widgetFn_t sprite_fn;
	memset(&sprite_fn, 0, sizeof(a3d_widgetFn_t));

	self->icon = a3d_sprite_new(screen, 0,
	                            &sprite_layout, &sprite_fn,
	                            &text_style->color,
	                            sprite_count);
	if(self->icon == NULL)
	{
		goto fail_icon;
	}

	a3d_textFn_t text_fn;
	memset(&text_fn, 0, sizeof(a3d_textFn_t));

	self->text = a3d_text_new(screen, 0, text_style,
	                          &text_fn);
	if(self->text == NULL)
	{
		goto fail_text;
	}

	// success
	return self;

	// failure
	fail_text:
		a3d_sprite_delete(&self->icon);
	fail_icon:
		a3d_widget_delete((a3d_widget_t**) &self);
	return NULL;
}

void a3d_bulletbox_delete(a3d_bulletbox_t** _self)
{
	assert(_self);

	a3d_bulletbox_t* self = *_self;
	if(self)
	{
		a3d_text_delete(&self->text);
		a3d_sprite_delete(&self->icon);
		a3d_widget_delete((a3d_widget_t**) _self);
	}
}

int a3d_bulletbox_spriteLoad(a3d_bulletbox_t* self,
                             int index,
                             const char* fname)
{
	assert(self);

	return a3d_sprite_load(self->icon, index, fname);
}

void a3d_bulletbox_spriteSelect(a3d_bulletbox_t* self,
                                int index)
{
	assert(self);

	a3d_sprite_select(self->icon, index);
}

void a3d_bulletbox_textPrintf(a3d_bulletbox_t* self,
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

	a3d_text_printf(self->text, "%s", string);
}
