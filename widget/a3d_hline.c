/*
 * Copyright (c) 2018 Jeff Boody
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

#include "a3d_hline.h"
#include "a3d_screen.h"
#include "a3d_font.h"
#include "../math/a3d_regionf.h"
#include "../a3d_timestamp.h"
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static void a3d_hline_size(a3d_widget_t* widget,
                           float* w, float* h)
{
	assert(widget);
	assert(w);
	assert(h);

	a3d_hline_t * self = (a3d_hline_t*) widget;

	float hline_w = *w;
	float hline_h = *h;
	a3d_widget_layoutSize(self->line, &hline_w, &hline_h);
}

static void a3d_hline_layout(a3d_widget_t* widget,
                             int dragx, int dragy)
{
	assert(widget);

	a3d_hline_t*  self = (a3d_hline_t*) widget;
	a3d_widget_t* line = self->line;

	// initialize the layout
	float x = 0.0f;
	float y = 0.0f;
	float t = widget->rect_draw.t;
	float l = widget->rect_draw.l;
	float h = widget->rect_draw.h;
	float w = widget->rect_draw.w;

	// layout line
	a3d_rect4f_t rect_draw;
	rect_draw.t = t;
	rect_draw.l = l;
	rect_draw.w = w;
	rect_draw.h = h;
	a3d_rect4f_t rect_clip;
	a3d_widget_layoutAnchor(line, &rect_draw, &x, &y);
	a3d_rect4f_intersect(&rect_draw,
	                     &widget->rect_clip,
	                     &rect_clip);
	a3d_widget_layoutXYClip(line, x, y, &rect_clip,
	                        dragx, dragy);
}

static void a3d_hline_drag(a3d_widget_t* widget,
                           float x, float y,
                           float dx, float dy)
{
	assert(widget);

	a3d_hline_t* self = (a3d_hline_t*) widget;
	a3d_widget_drag(self->line, x, y, dx, dy);
}

static void a3d_hline_draw(struct a3d_widget_s* widget)
{
	assert(widget);

	a3d_hline_t* self = (a3d_hline_t*) widget;
	a3d_widget_draw(self->line);
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_hline_t* a3d_hline_new(a3d_screen_t* screen,
                           int wsize,
                           int size,
                           a3d_vec4f_t* color)
{
	assert(screen);
	assert(color);

	if(wsize == 0)
	{
		wsize = sizeof(a3d_hline_t);
	}

	int wrapy = A3D_WIDGET_WRAP_STRETCH_TEXT_SMALL +
	            size -
	            A3D_HLINE_SIZE_SMALL;
	a3d_widgetLayout_t layout =
	{
		.wrapx    = A3D_WIDGET_WRAP_STRETCH_PARENT,
		.wrapy    = wrapy,
		.aspectx  = A3D_WIDGET_ASPECT_DEFAULT,
		.aspecty  = A3D_WIDGET_ASPECT_DEFAULT,
		.stretchx = 1.0f,
		.stretchy = 1.0f
	};

	a3d_widgetStyle_t style;
	memset(&style, 0, sizeof(a3d_widgetStyle_t));

	a3d_widgetScroll_t scroll =
	{
		.scroll_bar = 0
	};

	a3d_widgetFn_t fn;
	memset(&fn, 0, sizeof(a3d_widgetFn_t));

	a3d_widgetPrivFn_t priv_fn =
	{
		.size_fn   = a3d_hline_size,
		.layout_fn = a3d_hline_layout,
		.drag_fn   = a3d_hline_drag,
		.draw_fn   = a3d_hline_draw,
	};

	a3d_hline_t* self;
	self = (a3d_hline_t*)
	       a3d_widget_new(screen, wsize, &layout, &style,
	                      &scroll, &fn, &priv_fn);
	if(self == NULL)
	{
		return NULL;
	}

	// override the line properties
	a3d_vec4f_copy(color, &style.color_body);
	memset(&priv_fn, 0, sizeof(a3d_widgetPrivFn_t));
	layout.stretchy = 0.15f;

	self->line = a3d_widget_new(screen, 0, &layout,
	                            &style, &scroll, &fn,
	                            &priv_fn);
	if(self->line == NULL)
	{
		goto fail_line;
	}

	a3d_widget_anchor(self->line, A3D_WIDGET_ANCHOR_CC);

	// success
	return self;

	// failure
	fail_line:
		a3d_widget_delete((a3d_widget_t**) &self);
	return NULL;
}

void a3d_hline_delete(a3d_hline_t** _self)
{
	assert(_self);

	a3d_hline_t* self = *_self;
	if(self)
	{
		a3d_widget_delete(&self->line);
		a3d_widget_delete((a3d_widget_t**) _self);
	}
}
