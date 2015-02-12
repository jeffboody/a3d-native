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
#include "a3d_listbox.h"
#include <stdlib.h>
#include <assert.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static void a3d_listbox_size(a3d_widget_t* widget,
                             float* w, float* h)
{
	assert(widget);
	assert(w);
	assert(h);
	LOGD("debug");

	a3d_listbox_t*  self  = (a3d_listbox_t*) widget;
	a3d_listitem_t* iter  = a3d_list_head(self->list);

	float cnt   = (float) a3d_list_size(self->list);
	float dw    = *w/cnt;
	float dh    = *h/cnt;
	if(self->orientation == A3D_LISTBOX_ORIENTATION_VERTICAL)
	{
		dw = *w;
	}
	else
	{
		dh = *h;
	}

	float wmax  = 0.0f;
	float hmax  = 0.0f;
	float wsum  = 0.0f;
	float hsum  = 0.0f;
	float tmp_w = 0.0f;
	float tmp_h = 0.0f;
	while(iter)
	{
		tmp_w  = dw;
		tmp_h  = dh;
		widget = (a3d_widget_t*) a3d_list_peekitem(iter);
		a3d_widget_layoutSize(widget, &tmp_w, &tmp_h);

		if(tmp_w > wmax)
		{
			wmax = tmp_w;
		}
		wsum += tmp_w;

		if(tmp_h > hmax)
		{
			hmax = tmp_h;
		}
		hsum += tmp_h;

		iter = a3d_list_next(iter);
	}

	if(self->orientation == A3D_LISTBOX_ORIENTATION_HORIZONTAL)
	{
		*w = wsum;
		*h = hmax;
	}
	else
	{
		*w = wmax;
		*h = hsum;
	}
}

static int a3d_listbox_click(a3d_widget_t* widget,
                             float x, float y)
{
	assert(widget);
	LOGD("debug x=%f, y=%f", x, y);

	a3d_listbox_t*  self = (a3d_listbox_t*) widget;
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

	// listboxes are always clicked
	return 1;
}

static void a3d_listbox_layoutVerticalShrink(a3d_listbox_t* self)
{
	assert(self);
	LOGD("debug");

	// initialize the layout
	float x = 0.0f;
	float y = 0.0f;
	float t = self->widget.rect_draw.t;
	float l = self->widget.rect_draw.l;
	float w = self->widget.rect_draw.w;
	a3d_rect4f_t rect_clip;
	a3d_rect4f_t rect_draw;

	a3d_listitem_t* iter = a3d_list_head(self->list);
	while(iter)
	{
		a3d_widget_t* child = (a3d_widget_t*) a3d_list_peekitem(iter);
		float h = child->rect_border.h;

		rect_draw.t = t;
		rect_draw.l = l;
		rect_draw.w = w;
		rect_draw.h = h;
		t += h;

		a3d_widget_anchorPt(&rect_draw, child->anchor, &x, &y);
		a3d_rect4f_intersect(&rect_draw,
		                     &self->widget.rect_clip,
		                     &rect_clip);
		a3d_widget_layoutXYClip(child, x, y, &rect_clip);

		iter = a3d_list_next(iter);
	}
}

static void a3d_listbox_layoutVerticalStretch(a3d_listbox_t* self)
{
	assert(self);
	LOGD("debug");

	// initialize the layout
	float x   = 0.0f;
	float y   = 0.0f;
	float t   = self->widget.rect_draw.t;
	float l   = self->widget.rect_draw.l;
	float w   = self->widget.rect_draw.w;
	float h   = self->widget.rect_draw.h;
	float cnt = (float) a3d_list_size(self->list);
	float dh  = h/cnt;
	a3d_rect4f_t rect_clip;
	a3d_rect4f_t rect_draw;

	a3d_listitem_t* iter = a3d_list_head(self->list);
	while(iter)
	{
		a3d_widget_t* child = (a3d_widget_t*) a3d_list_peekitem(iter);

		rect_draw.t = t;
		rect_draw.l = l;
		rect_draw.w = w;
		rect_draw.h = dh;
		t += dh;

		a3d_widget_anchorPt(&rect_draw, child->anchor, &x, &y);
		a3d_rect4f_intersect(&rect_draw,
		                     &self->widget.rect_clip,
		                     &rect_clip);
		a3d_widget_layoutXYClip(child, x, y, &rect_clip);

		iter = a3d_list_next(iter);
	}
}

static void a3d_listbox_layoutHorizontalShrink(a3d_listbox_t* self)
{
	assert(self);
	LOGD("debug");

	// initialize the layout
	float x = 0.0f;
	float y = 0.0f;
	float t = self->widget.rect_draw.t;
	float l = self->widget.rect_draw.l;
	float h = self->widget.rect_draw.h;
	a3d_rect4f_t rect_clip;
	a3d_rect4f_t rect_draw;

	a3d_listitem_t* iter = a3d_list_head(self->list);
	while(iter)
	{
		a3d_widget_t* child = (a3d_widget_t*) a3d_list_peekitem(iter);
		float w = child->rect_border.w;

		rect_draw.t = t;
		rect_draw.l = l;
		rect_draw.w = w;
		rect_draw.h = h;
		l += w;

		a3d_widget_anchorPt(&rect_draw, child->anchor, &x, &y);
		a3d_rect4f_intersect(&rect_draw,
		                     &self->widget.rect_clip,
		                     &rect_clip);
		a3d_widget_layoutXYClip(child, x, y, &rect_clip);

		iter = a3d_list_next(iter);
	}
}

static void a3d_listbox_layoutHorizontalStretch(a3d_listbox_t* self)
{
	assert(self);
	LOGD("debug");

	// initialize the layout
	float x   = 0.0f;
	float y   = 0.0f;
	float t   = self->widget.rect_draw.t;
	float l   = self->widget.rect_draw.l;
	float w   = self->widget.rect_draw.w;
	float h   = self->widget.rect_draw.h;
	float cnt = (float) a3d_list_size(self->list);
	float dw  = w/cnt;
	a3d_rect4f_t rect_clip;
	a3d_rect4f_t rect_draw;

	a3d_listitem_t* iter = a3d_list_head(self->list);
	while(iter)
	{
		a3d_widget_t* child = (a3d_widget_t*) a3d_list_peekitem(iter);

		rect_draw.t = t;
		rect_draw.l = l;
		rect_draw.w = dw;
		rect_draw.h = h;
		l += dw;

		a3d_widget_anchorPt(&rect_draw, child->anchor, &x, &y);
		a3d_rect4f_intersect(&rect_draw,
		                     &self->widget.rect_clip,
		                     &rect_clip);
		a3d_widget_layoutXYClip(child, x, y, &rect_clip);

		iter = a3d_list_next(iter);
	}
}

static void a3d_listbox_layout(a3d_widget_t* widget)
{
	assert(widget);
	LOGD("debug");

	a3d_listbox_t* self = (a3d_listbox_t*) widget;
	if(a3d_list_empty(self->list))
	{
		return;
	}

	if(self->orientation == A3D_LISTBOX_ORIENTATION_VERTICAL)
	{
		if(widget->wrapv == A3D_WIDGET_WRAP_SHRINK)
		{
			a3d_listbox_layoutVerticalShrink(self);
		}
		else
		{
			a3d_listbox_layoutVerticalStretch(self);
		}
	}
	else
	{
		if(widget->wraph == A3D_WIDGET_WRAP_SHRINK)
		{
			a3d_listbox_layoutHorizontalShrink(self);
		}
		else
		{
			a3d_listbox_layoutHorizontalStretch(self);
		}
	}
}

static int a3d_listbox_drag(a3d_widget_t* widget,
                            float x, float y,
                            float dx, float dy,
                            double dt)
{
	assert(widget);
	LOGD("debug");

	a3d_listbox_t*  self = (a3d_listbox_t*) widget;
	a3d_listitem_t* iter = a3d_list_head(self->list);
	while(iter)
	{
		widget = (a3d_widget_t*) a3d_list_peekitem(iter);
		a3d_widget_drag(widget, x, y, dx, dy, dt);

		iter = a3d_list_next(iter);
	}

	// always drag a listbox since its elements
	// may not always cover the entire box
	return 1;
}

static void a3d_listbox_draw(a3d_widget_t* widget)
{
	assert(widget);
	LOGD("debug");

	a3d_listbox_t*  self = (a3d_listbox_t*) widget;
	a3d_listitem_t* iter = a3d_list_head(self->list);
	while(iter)
	{
		widget = (a3d_widget_t*) a3d_list_peekitem(iter);
		a3d_widget_draw(widget);
		iter = a3d_list_next(iter);
	}
}

static void a3d_listbox_refresh(a3d_widget_t* widget)
{
	assert(widget);
	LOGD("debug");

	a3d_listbox_t*  self = (a3d_listbox_t*) widget;
	a3d_listitem_t* iter = a3d_list_head(self->list);
	while(iter)
	{
		widget = (a3d_widget_t*) a3d_list_peekitem(iter);
		a3d_widget_refresh(widget);
		iter = a3d_list_next(iter);
	}
}

static void a3d_listbox_notify(void* owner, a3d_listitem_t* item)
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

a3d_listbox_t* a3d_listbox_new(a3d_screen_t* screen,
                               int wsize,
                               int orientation,
                               int anchor,
                               int wraph, int wrapv,
                               int stretch_mode,
                               float stretch_factor,
                               int style_border,
                               int style_line,
                               a3d_vec4f_t* color_fill,
                               a3d_vec4f_t* color_line,
                               a3d_widget_refresh_fn refresh_fn)
{
	// refresh_fn may be NULL
	assert(screen);
	assert(color_fill);
	assert(color_line);
	LOGD("debug wsize=%i, orientation=%i, anchor=%i, wraph=%i, wrapv=%i",
	     wsize, orientation, anchor, wraph, wrapv);
	LOGD("debug stretch_mode=%i, stretch_factor=%f, style_border=%i, style_line=%i",
	     stretch_mode, stretch_factor, style_border, style_line);
	LOGD("debug color_fill: r=%f, g=%f, b=%f, a=%f",
	     color_fill->r, color_fill->g, color_fill->b, color_fill->a);
	LOGD("debug color_line: r=%f, g=%f, b=%f, a=%f",
	     color_line->r, color_line->g, color_line->b, color_line->a);

	if(wsize == 0)
	{
		wsize = sizeof(a3d_listbox_t);
	}

	// optionally overide refresh_fn
	if(refresh_fn == NULL)
	{
		refresh_fn = a3d_listbox_refresh;
	}

	a3d_listbox_t* self = (a3d_listbox_t*) a3d_widget_new(screen,
	                                                      wsize,
	                                                      anchor,
	                                                      wraph,
	                                                      wrapv,
	                                                      stretch_mode,
	                                                      stretch_factor,
	                                                      style_border,
	                                                      style_line,
	                                                      color_line,
	                                                      color_fill,
	                                                      a3d_listbox_size,
	                                                      a3d_listbox_click,
	                                                      a3d_listbox_layout,
	                                                      a3d_listbox_drag,
	                                                      a3d_listbox_draw,
	                                                      refresh_fn);
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
	                a3d_listbox_notify,
	                a3d_listbox_notify,
	                a3d_listbox_notify);

	self->orientation = orientation;

	// success
	return self;

	// failure
	fail_list:
		a3d_widget_delete((a3d_widget_t**) &self);
	return NULL;
}

void a3d_listbox_delete(a3d_listbox_t** _self)
{
	assert(_self);

	a3d_listbox_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		a3d_list_delete(&self->list);
		a3d_widget_delete((a3d_widget_t**) _self);
	}
}
