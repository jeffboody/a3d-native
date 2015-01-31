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
#include "a3d_layer.h"
#include <stdlib.h>
#include <assert.h>

#define LOG_TAG "a3d"
#include "a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static void a3d_layer_size(a3d_widget_t* widget,
                           float* w, float* h)
{
	assert(widget);
	assert(w);
	assert(h);
	LOGD("debug");

	a3d_layer_t*    self  = (a3d_layer_t*) widget;
	a3d_listitem_t* iter  = a3d_list_head(self->list);

	float wmax  = 0.0f;
	float hmax  = 0.0f;
	float tmp_w = 0.0f;
	float tmp_h = 0.0f;
	while(iter)
	{
		tmp_w  = *w;
		tmp_h  = *h;
		widget = (a3d_widget_t*) a3d_list_peekitem(iter);
		a3d_widget_layoutSize(widget, &tmp_w, &tmp_h);

		if(tmp_w > wmax)
		{
			wmax = tmp_w;
		}

		if(tmp_h > hmax)
		{
			hmax = tmp_h;
		}

		iter = a3d_list_next(iter);
	}

	*w = wmax;
	*h = hmax;
}

static int a3d_layer_click(a3d_widget_t* widget,
                           float x, float y)
{
	assert(widget);
	LOGD("debug x=%f, y=%f", x, y);

	// send events front-to-back
	a3d_layer_t*    self = (a3d_layer_t*) widget;
	a3d_listitem_t* iter = a3d_list_head(self->list);
	while(iter)
	{
		widget = (a3d_widget_t*) a3d_list_peekitem(iter);
		if(a3d_widget_click(widget, x, y))
		{
			return 1;
		}

		iter = a3d_list_next(iter);
	}

	// layers are only clicked if a child is clicked
	return 0;
}

static void a3d_layer_layout(a3d_widget_t* widget)
{
	assert(widget);
	LOGD("debug");

	// the rect_clip is constant across all layers
	a3d_rect4f_t rect_clip;
	a3d_rect4f_intersect(&widget->rect_draw,
	                     &widget->rect_clip,
	                     &rect_clip);

	a3d_layer_t*    self = (a3d_layer_t*) widget;
	a3d_listitem_t* iter = a3d_list_head(self->list);
	while(iter)
	{
		a3d_widget_t* child = (a3d_widget_t*) a3d_list_peekitem(iter);

		// layout the layer
		float x = 0.0f;
		float y = 0.0f;
		a3d_widget_anchorPt(&widget->rect_draw,
		                    child->anchor,
		                    &x, &y);
		a3d_widget_layoutXYClip(child, x, y, &rect_clip);

		iter = a3d_list_next(iter);
	}
}

static void a3d_layer_drag(a3d_widget_t* widget,
                           float x, float y,
                           float dx, float dy,
                           double dt)
{
	assert(widget);
	LOGD("debug");

	a3d_layer_t*    self = (a3d_layer_t*) widget;
	a3d_listitem_t* iter = a3d_list_head(self->list);
	while(iter)
	{
		widget = (a3d_widget_t*) a3d_list_peekitem(iter);
		a3d_widget_drag(widget, x, y, dx, dy, dt);

		iter = a3d_list_next(iter);
	}
}

static void a3d_layer_draw(a3d_widget_t* widget)
{
	assert(widget);
	LOGD("debug");

	// draw back-to-front
	a3d_layer_t*    self = (a3d_layer_t*) widget;
	a3d_listitem_t* iter = a3d_list_tail(self->list);
	while(iter)
	{
		widget = (a3d_widget_t*) a3d_list_peekitem(iter);
		a3d_widget_draw(widget);
		iter = a3d_list_prev(iter);
	}
}

static void a3d_layer_refresh(a3d_widget_t* widget)
{
	assert(widget);
	LOGD("debug");

	a3d_layer_t*    self = (a3d_layer_t*) widget;
	a3d_listitem_t* iter = a3d_list_head(self->list);
	while(iter)
	{
		widget = (a3d_widget_t*) a3d_list_peekitem(iter);
		a3d_widget_refresh(widget);
		iter = a3d_list_next(iter);
	}
}

static void a3d_layer_notify(void* owner, a3d_listitem_t* item)
{
	assert(owner);
	assert(item);
	LOGD("debug");

	a3d_widget_t* self = (a3d_widget_t*) owner;
	a3d_screen_dirty(self->screen);
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_layer_t* a3d_layer_new(a3d_screen_t* screen,
                           int wsize,
                           int anchor,
                           int wraph, int wrapv,
                           int style_border,
                           int style_line,
                           a3d_vec4f_t* color_fill,
                           a3d_vec4f_t* color_line)
{
	assert(screen);
	assert(color_fill);
	assert(color_line);
	LOGD("debug wsize=%i, anchor=%i, wraph=%i, wrapv=%i, style_border=%i, style_line=%i",
	     wsize, anchor, wraph, wrapv, style_border, style_line);
	LOGD("debug color_fill: r=%f, g=%f, b=%f, a=%f",
	     color_fill->r, color_fill->g, color_fill->b, color_fill->a);
	LOGD("debug color_line: r=%f, g=%f, b=%f, a=%f",
	     color_line->r, color_line->g, color_line->b, color_line->a);

	if(wsize == 0)
	{
		wsize = sizeof(a3d_layer_t);
	}

	a3d_layer_t* self = (a3d_layer_t*) a3d_widget_new(screen,
	                                                  wsize,
	                                                  anchor,
	                                                  wraph,
	                                                  wrapv,
	                                                  style_border,
	                                                  style_line,
	                                                  color_line,
	                                                  color_fill,
	                                                  a3d_layer_size,
	                                                  a3d_layer_click,
	                                                  a3d_layer_layout,
	                                                  a3d_layer_drag,
	                                                  a3d_layer_draw,
	                                                  a3d_layer_refresh);
	if(self == NULL)
	{
		return NULL;
	}

	self->list = a3d_list_new();
	if(self->list == NULL)
	{
		goto fail_list;
	}

	a3d_list_notify(self->list,
	                (void*) self,
	                a3d_layer_notify,
	                a3d_layer_notify,
	                a3d_layer_notify);

	// success
	return self;

	// failure
	fail_list:
		a3d_widget_delete((a3d_widget_t**) &self);
	return NULL;
}

void a3d_layer_delete(a3d_layer_t** _self)
{
	assert(_self);

	a3d_layer_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		a3d_list_delete(&self->list);
		a3d_widget_delete((a3d_widget_t**) _self);
	}
}
