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

#include "a3d_viewbox.h"
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static void a3d_viewbox_size(a3d_widget_t* widget,
                             float* w, float* h)
{
	assert(widget);
	assert(w);
	assert(h);

	a3d_widgetLayout_t* layout = &widget->layout;
	a3d_viewbox_t*      self   = (a3d_viewbox_t*) widget;
	a3d_widget_t*       bullet = (a3d_widget_t*) self->bullet;
	a3d_widget_t*       body   = self->body;
	a3d_widget_t*       footer = self->footer;

	float wmax     = 0.0f;
	float hsum     = 0.0f;
	float bullet_w = *w;
	float bullet_h = *h;

	// layout bullet
	a3d_widget_layoutSize(bullet, &bullet_w, &bullet_h);
	wmax = bullet_w;
	hsum = bullet_h;

	// layout separator(s)
	float h_bo = 0.0f;
	float v_bo = 0.0f;
	a3d_screen_layoutBorder(widget->screen, layout->border,
	                        &h_bo, &v_bo);
	if(footer)
	{
		hsum += 4.0f*v_bo;
	}
	else
	{
		hsum += 2.0f*v_bo;
	}

	// layout footer
	if(footer)
	{
		float footer_w = *w;
		float footer_h = *h - hsum;
		if(footer_h < 0.0f)
		{
			footer_h = 0.0f;
		}

		a3d_widget_layoutSize(footer, &footer_w, &footer_h);
		if(footer_w > wmax)
		{
			wmax = footer_w;
		}

		hsum += footer_h;
	}

	// layout body
	float body_w = *w;
	float body_h = *h - hsum ;
	if(body_h < 0.0f)
	{
		body_h = 0.0f;
	}
	a3d_widget_layoutSize(body, &body_w, &body_h);

	if(body_w > wmax)
	{
		wmax = body_w;
	}
	hsum += body_h;

	*w = wmax;
	*h = hsum;
}

static int a3d_viewbox_click(a3d_widget_t* widget,
                             void* priv, int state,
                             float x, float y)
{
	// priv may be NULL
	assert(widget);

	a3d_viewbox_t* self = (a3d_viewbox_t*) widget;
	a3d_widget_t*  bullet = (a3d_widget_t*) self->bullet;
	a3d_widget_t*  body   = (a3d_widget_t*) self->body;
	a3d_widget_t*  footer = (a3d_widget_t*) self->footer;
	if(a3d_widget_click(bullet, state, x, y) ||
	   a3d_widget_click(body, state, x, y)   ||
	   (footer && a3d_widget_click(footer, state, x, y)))
	{
		return 1;
	}

	// viewboxes are always clicked
	return 1;
}

static void a3d_viewbox_layout(a3d_widget_t* widget,
                               int dragx, int dragy)
{
	assert(widget);

	a3d_widgetLayout_t* layout = &widget->layout;
	a3d_viewbox_t*      self   = (a3d_viewbox_t*) widget;
	a3d_widget_t*       bullet = (a3d_widget_t*) self->bullet;
	a3d_widget_t*       body   = self->body;
	a3d_widget_t*       footer = self->footer;

	// initialize the layout
	float x  = 0.0f;
	float y  = 0.0f;
	float t  = self->widget.rect_draw.t;
	float l  = self->widget.rect_draw.l;
	float w  = self->widget.rect_draw.w;
	float h  = self->widget.rect_draw.h;
	float bullet_h = bullet->rect_border.h;
	float footer_h = 0.0f;
	a3d_rect4f_t rect_clip;
	a3d_rect4f_t rect_draw;

	// layout bullet
	rect_draw.t = t;
	rect_draw.l = l;
	rect_draw.w = w;
	rect_draw.h = bullet_h;
	a3d_widget_layoutAnchor(bullet, &rect_draw, &x, &y);
	a3d_rect4f_intersect(&rect_draw,
	                     &self->widget.rect_clip,
	                     &rect_clip);
	a3d_widget_layoutXYClip(bullet, x, y, &rect_clip,
	                        dragx, dragy);

	// get separator size
	float h_bo = 0.0f;
	float v_bo = 0.0f;
	a3d_screen_layoutBorder(widget->screen,
	                        layout->border,
	                        &h_bo, &v_bo);

	// layout body
	if(footer)
	{
		footer_h = footer->rect_border.h;
		rect_draw.t = t + bullet_h + 2.0f*v_bo;
		rect_draw.h = h - bullet_h - 4.0f*v_bo - footer_h;
	}
	else
	{
		rect_draw.t = t + bullet_h + 2.0f*v_bo;
		rect_draw.h = h - bullet_h - 2.0f*v_bo;
	}
	a3d_widget_layoutAnchor(body, &rect_draw, &x, &y);
	a3d_rect4f_intersect(&rect_draw,
	                     &self->widget.rect_clip,
	                     &rect_clip);
	a3d_widget_layoutXYClip(body, x, y, &rect_clip,
	                        dragx, dragy);

	// layout footer
	if(footer)
	{
		rect_draw.t = t + h - footer_h;
		rect_draw.l = l;
		rect_draw.w = w;
		rect_draw.h = footer_h;
		a3d_widget_layoutAnchor(footer, &rect_draw, &x, &y);
		a3d_rect4f_intersect(&rect_draw,
		                     &self->widget.rect_clip,
		                     &rect_clip);
		a3d_widget_layoutXYClip(footer, x, y, &rect_clip,
		                        dragx, dragy);
	}
}

static void a3d_viewbox_drag(a3d_widget_t* widget,
                             float x, float y,
                             float dx, float dy)
{
	assert(widget);

	a3d_viewbox_t* self = (a3d_viewbox_t*) widget;
	a3d_widget_drag((a3d_widget_t*) self->bullet,
	                x, y, dx, dy);
	a3d_widget_drag(self->body,
	                x, y, dx, dy);
	if(self->footer)
	{
		a3d_widget_drag(self->footer,
		                x, y, dx, dy);
	}
}

static void a3d_viewbox_scrollTop(a3d_widget_t* widget)
{
	assert(widget);

	a3d_viewbox_t* self = (a3d_viewbox_t*) widget;

	a3d_widget_scrollTop(self->body);
}

static void a3d_viewbox_draw(a3d_widget_t* widget)
{
	assert(widget);

	a3d_widgetLayout_t* layout = &widget->layout;

	// bullet separator y
	float h_bo = 0.0f;
	float v_bo = 0.0f;
	a3d_viewbox_t* self = (a3d_viewbox_t*) widget;
	a3d_screen_layoutBorder(widget->screen,
	                        layout->border,
	                        &h_bo, &v_bo);
	a3d_widget_t* w = &(self->bullet->widget);
	float         y = w->rect_border.t + w->rect_border.h + v_bo;

	a3d_widget_draw((a3d_widget_t*) self->bullet);
	a3d_widget_draw(self->body);
	if(self->footer)
	{
		a3d_widget_draw(self->footer);
	}
	a3d_widget_headerY(widget, y);
}

static void
a3d_viewbox_refresh(a3d_widget_t* widget, void* priv)
{
	// priv may be NULL
	assert(widget);

	a3d_viewbox_t* self = (a3d_viewbox_t*) widget;
	a3d_widget_refresh((a3d_widget_t*) self->bullet);
	a3d_widget_refresh(self->body);
	if(self->footer)
	{
		a3d_widget_refresh(self->footer);
	}
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_viewbox_t* a3d_viewbox_new(a3d_screen_t* screen,
                               int wsize,
                               a3d_widgetLayout_t* layout,
                               a3d_widgetFn_t* fn,
                               a3d_viewboxStyle_t* style,
                               a3d_widget_t* body,
                               a3d_widget_t* footer)
{
	// footer may be NULL
	assert(screen);
	assert(layout);
	assert(fn);
	assert(style);
	assert(body);

	if(wsize == 0)
	{
		wsize = sizeof(a3d_viewbox_t);
	}

	a3d_widgetScroll_t scroll =
	{
		.scroll_bar = 0
	};

	a3d_widgetFn_t viewbox_fn =
	{
		.click_fn   = a3d_viewbox_click,
		.refresh_fn = a3d_viewbox_refresh
	};

	a3d_widgetPrivFn_t priv_fn =
	{
		.size_fn      = a3d_viewbox_size,
		.layout_fn    = a3d_viewbox_layout,
		.drag_fn      = a3d_viewbox_drag,
		.scrollTop_fn = a3d_viewbox_scrollTop,
		.draw_fn      = a3d_viewbox_draw,
	};

	a3d_viewbox_t* self;
	self = (a3d_viewbox_t*)
	       a3d_widget_new(screen, wsize, layout,
	                      &style->widget_style, &scroll,
	                      &viewbox_fn, &priv_fn);
	if(self == NULL)
	{
		return NULL;
	}
	a3d_widget_soundFx((a3d_widget_t*) self, 0);

	self->bullet = a3d_bulletbox_new(screen, 0,
	                                 &style->text_style, 2,
	                                 fn);
	if(self->bullet == NULL)
	{
		goto fail_bullet;
	}

	self->body   = body;
	self->footer = footer;

	// success
	return self;

	// failure
	fail_bullet:
		a3d_widget_delete((a3d_widget_t**) &self);
	return NULL;
}

void a3d_viewbox_delete(a3d_viewbox_t** _self)
{
	assert(_self);

	a3d_viewbox_t* self = *_self;
	if(self)
	{
		a3d_bulletbox_delete(&self->bullet);
		a3d_widget_delete((a3d_widget_t**) _self);
	}
}

int a3d_viewbox_spriteLoad(a3d_viewbox_t* self, int index,
                           const char* fname)
{
	assert(self);
	assert(fname);

	return a3d_bulletbox_spriteLoad(self->bullet, index,
	                                fname);
}

void a3d_viewbox_spriteSelect(a3d_viewbox_t* self,
                              int index)
{
	assert(self);

	a3d_bulletbox_spriteSelect(self->bullet, index);
}

void a3d_viewbox_textPrintf(a3d_viewbox_t* self,
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

	a3d_bulletbox_textPrintf(self->bullet, "%s", string);
}
