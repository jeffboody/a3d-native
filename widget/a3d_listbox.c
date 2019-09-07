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

	a3d_listbox_t*      self   = (a3d_listbox_t*) widget;
	a3d_listitem_t*     iter   = a3d_list_head(self->list);
	a3d_widgetLayout_t* layout = &widget->layout;

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

	// vertical lists that are shrink wrapped in x may have
	// widgets that are stretch parent (e.g. hlines) so
	// special handling is required to prevent the whole
	// list from being stretched to parent
	float               wmax  = 0.0f;
	float               hmax  = 0.0f;
	float               wsum  = 0.0f;
	float               hsum  = 0.0f;
	float               tmp_w = 0.0f;
	float               tmp_h = 0.0f;
	a3d_widget_t*       tmp_widget;
	a3d_widgetLayout_t* tmp_layout;
	if((layout->wrapx == A3D_WIDGET_WRAP_SHRINK) &&
	   (self->orientation == A3D_LISTBOX_ORIENTATION_VERTICAL))
	{
		// first pass computes size based on widgets
		// that are not stretch parent
		while(iter)
		{
			tmp_w  = dw;
			tmp_h  = dh;
			tmp_widget = (a3d_widget_t*) a3d_list_peekitem(iter);
			tmp_layout = &tmp_widget->layout;
			if(tmp_layout->wrapx == A3D_WIDGET_WRAP_STRETCH_PARENT)
			{
				iter = a3d_list_next(iter);
				continue;
			}

			a3d_widget_layoutSize(tmp_widget, &tmp_w, &tmp_h);

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

		// second pass computes size of widgets that are
		// stretch parent
		iter  = a3d_list_head(self->list);
		while(iter)
		{
			tmp_w  = wmax;
			tmp_h  = dh;
			tmp_widget = (a3d_widget_t*) a3d_list_peekitem(iter);
			tmp_layout = &tmp_widget->layout;
			if(tmp_layout->wrapx != A3D_WIDGET_WRAP_STRETCH_PARENT)
			{
				iter = a3d_list_next(iter);
				continue;
			}

			a3d_widget_layoutSize(tmp_widget, &tmp_w, &tmp_h);

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
	}
	else
	{
		while(iter)
		{
			tmp_w  = dw;
			tmp_h  = dh;
			tmp_widget = (a3d_widget_t*) a3d_list_peekitem(iter);
			a3d_widget_layoutSize(tmp_widget, &tmp_w, &tmp_h);

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
                             void* priv, int state,
                             float x, float y)
{
	// priv may be NULL
	assert(widget);

	a3d_listbox_t*  self = (a3d_listbox_t*) widget;
	a3d_listitem_t* iter = a3d_list_head(self->list);
	while(iter)
	{
		widget = (a3d_widget_t*) a3d_list_peekitem(iter);
		if(a3d_widget_click(widget, state, x, y))
		{
			return 1;
		}

		iter = a3d_list_next(iter);
	}

	// listboxes are always clicked
	return 1;
}

static void a3d_listbox_layoutVerticalShrink(a3d_listbox_t* self,
                                             int dragx, int dragy)
{
	assert(self);

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

		a3d_widget_layoutAnchor(child, &rect_draw, &x, &y);
		a3d_rect4f_intersect(&rect_draw,
		                     &self->widget.rect_clip,
		                     &rect_clip);
		a3d_widget_layoutXYClip(child, x, y, &rect_clip,
		                        dragx, dragy);

		iter = a3d_list_next(iter);
	}
}

static void a3d_listbox_layoutVerticalStretch(a3d_listbox_t* self,
                                              int dragx, int dragy)
{
	assert(self);

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

		a3d_widget_layoutAnchor(child, &rect_draw, &x, &y);
		a3d_rect4f_intersect(&rect_draw,
		                     &self->widget.rect_clip,
		                     &rect_clip);
		a3d_widget_layoutXYClip(child, x, y, &rect_clip,
		                        dragx, dragy);

		iter = a3d_list_next(iter);
	}
}

static void a3d_listbox_layoutHorizontalShrink(a3d_listbox_t* self,
                                               int dragx, int dragy)
{
	assert(self);

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

		a3d_widget_layoutAnchor(child, &rect_draw, &x, &y);
		a3d_rect4f_intersect(&rect_draw,
		                     &self->widget.rect_clip,
		                     &rect_clip);
		a3d_widget_layoutXYClip(child, x, y, &rect_clip,
		                        dragx, dragy);

		iter = a3d_list_next(iter);
	}
}

static void a3d_listbox_layoutHorizontalStretch(a3d_listbox_t* self,
                                                int dragx, int dragy)
{
	assert(self);

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

		a3d_widget_layoutAnchor(child, &rect_draw, &x, &y);
		a3d_rect4f_intersect(&rect_draw,
		                     &self->widget.rect_clip,
		                     &rect_clip);
		a3d_widget_layoutXYClip(child, x, y, &rect_clip,
		                        dragx, dragy);

		iter = a3d_list_next(iter);
	}
}

static void a3d_listbox_layout(a3d_widget_t* widget,
                               int dragx, int dragy)
{
	assert(widget);

	a3d_widgetLayout_t* layout = &widget->layout;

	a3d_listbox_t* self = (a3d_listbox_t*) widget;
	if(a3d_list_empty(self->list))
	{
		return;
	}

	if(self->orientation == A3D_LISTBOX_ORIENTATION_VERTICAL)
	{
		if(layout->wrapy == A3D_WIDGET_WRAP_SHRINK)
		{
			a3d_listbox_layoutVerticalShrink(self, dragx, dragy);
		}
		else
		{
			a3d_listbox_layoutVerticalStretch(self, dragx, dragy);
		}
	}
	else
	{
		if(layout->wrapx == A3D_WIDGET_WRAP_SHRINK)
		{
			a3d_listbox_layoutHorizontalShrink(self, dragx, dragy);
		}
		else
		{
			a3d_listbox_layoutHorizontalStretch(self, dragx, dragy);
		}
	}
}

static void a3d_listbox_drag(a3d_widget_t* widget,
                             float x, float y,
                             float dx, float dy)
{
	assert(widget);

	a3d_listbox_t*  self = (a3d_listbox_t*) widget;
	a3d_listitem_t* iter = a3d_list_head(self->list);
	while(iter)
	{
		widget = (a3d_widget_t*) a3d_list_peekitem(iter);
		a3d_widget_drag(widget, x, y, dx, dy);
		iter = a3d_list_next(iter);
	}
}

static void a3d_listbox_draw(a3d_widget_t* widget)
{
	assert(widget);

	a3d_listbox_t*  self = (a3d_listbox_t*) widget;
	a3d_listitem_t* iter = a3d_list_head(self->list);
	while(iter)
	{
		widget = (a3d_widget_t*) a3d_list_peekitem(iter);
		a3d_widget_draw(widget);
		iter = a3d_list_next(iter);
	}
}

static void
a3d_listbox_refresh(a3d_widget_t* widget, void* priv)
{
	// priv may be NULL
	assert(widget);

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

	a3d_widget_t* self = (a3d_widget_t*) owner;
	a3d_screen_dirty(self->screen);
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_listbox_t* a3d_listbox_new(a3d_screen_t* screen,
                               int wsize,
                               a3d_widgetLayout_t* layout,
                               int orientation,
                               a3d_vec4f_t* color_scroll0,
                               a3d_vec4f_t* color_scroll1,
                               void* priv,
                               a3d_widget_clickFn click_fn,
                               a3d_widget_reflowFn reflow_fn,
                               a3d_widget_refreshFn refresh_fn)
{
	// priv, click_fn, reflow_fn, refresh_fn may be NULL
	assert(screen);
	assert(layout);
	assert(color_scroll0);
	assert(color_scroll1);

	if(wsize == 0)
	{
		wsize = sizeof(a3d_listbox_t);
	}

	a3d_widgetStyle_t style =
	{
		.color_scroll0 =
		{
			.r = color_scroll0->r,
			.g = color_scroll0->g,
			.b = color_scroll0->b,
			.a = color_scroll0->a,
		},
		.color_scroll1 =
		{
			.r = color_scroll1->r,
			.g = color_scroll1->g,
			.b = color_scroll1->b,
			.a = color_scroll1->a,
		}
	};

	a3d_widgetFn_t fn =
	{
		.priv       = priv,
		.reflow_fn  = reflow_fn,
		.size_fn    = a3d_listbox_size,
		.click_fn   = click_fn ? click_fn : a3d_listbox_click,
		.layout_fn  = a3d_listbox_layout,
		.drag_fn    = a3d_listbox_drag,
		.draw_fn    = a3d_listbox_draw,
		.refresh_fn = refresh_fn ? refresh_fn : a3d_listbox_refresh
	};

	a3d_listbox_t* self;
	self = (a3d_listbox_t*)
	       a3d_widget_new(screen, wsize, layout, &style, &fn);
	if(self == NULL)
	{
		return NULL;
	}
	a3d_widget_soundFx((a3d_widget_t*) self, 0);

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
		a3d_list_delete(&self->list);
		a3d_widget_delete((a3d_widget_t**) _self);
	}
}

void a3d_listbox_clear(a3d_listbox_t* self)
{
	assert(self);

	a3d_list_discard(self->list);
}
