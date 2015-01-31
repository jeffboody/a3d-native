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

#include "a3d_dropbox.h"
#include <stdlib.h>
#include <assert.h>

#define LOG_TAG "a3d"
#include "a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static void a3d_dropbox_size(a3d_widget_t* widget,
                             float* w, float* h)
{
	assert(widget);
	assert(w);
	assert(h);
	LOGD("debug");

	a3d_dropbox_t* self        = (a3d_dropbox_t*) widget;
	a3d_widget_t*  text_widget = (a3d_widget_t*) self->drop_text;
	a3d_widget_t*  drop_widget = self->drop_widget;

	float wmax  = 0.0f;
	float hsum  = 0.0f;
	float tmp_w = *w;
	float tmp_h = *h;

	a3d_widget_layoutSize(text_widget, &tmp_w, &tmp_h);
	wmax = tmp_w;
	hsum = tmp_h;

	if(self->drop)
	{
		float h2 = *h - tmp_h;
		if(h2 < 0.0f)
		{
			h2 = 0.0f;
		}

		tmp_w = *w;
		tmp_h = h2 ;
		a3d_widget_layoutSize(drop_widget, &tmp_w, &tmp_h);

		if(tmp_w > wmax)
		{
			wmax = tmp_w;
		}
		hsum += tmp_h;
	}

	*w = wmax;
	*h = hsum;
}

static int a3d_dropbox_click(a3d_widget_t* widget,
                             float x, float y)
{
	assert(widget);
	LOGD("debug x=%f, y=%f", x, y);

	a3d_dropbox_t* self = (a3d_dropbox_t*) widget;
	if(a3d_widget_click((a3d_widget_t*) self->drop_text, x, y))
	{
		return 1;
	}
	if(self->drop)
	{
		if(a3d_widget_click(self->drop_widget, x, y))
		{
			return 1;
		}
	}
	return 0;
}

static void a3d_dropbox_layout(a3d_widget_t* widget)
{
	assert(widget);
	LOGD("debug");

	a3d_dropbox_t* self        = (a3d_dropbox_t*) widget;
	a3d_widget_t*  text_widget = (a3d_widget_t*) self->drop_text;
	a3d_widget_t*  drop_widget = self->drop_widget;

	// initialize the layout
	float x  = 0.0f;
	float y  = 0.0f;
	float t  = self->widget.rect_draw.t;
	float l  = self->widget.rect_draw.l;
	float w  = self->widget.rect_draw.w;
	float h  = self->widget.rect_draw.h;
	float th = text_widget->rect_border.h;
	a3d_rect4f_t rect_clip;
	a3d_rect4f_t rect_draw;

	rect_draw.t = t;
	rect_draw.l = l;
	rect_draw.w = w;
	rect_draw.h = th;

	a3d_widget_anchorPt(&rect_draw, text_widget->anchor, &x, &y);
	a3d_rect4f_intersect(&rect_draw,
	                     &self->widget.rect_clip,
	                     &rect_clip);
	a3d_widget_layoutXYClip(text_widget, x, y, &rect_clip);

	if(self->drop)
	{
		rect_draw.t = t + th;
		rect_draw.h = h - th;

		a3d_widget_anchorPt(&rect_draw, drop_widget->anchor, &x, &y);
		a3d_rect4f_intersect(&rect_draw,
		                     &self->widget.rect_clip,
		                     &rect_clip);
		a3d_widget_layoutXYClip(drop_widget, x, y, &rect_clip);
	}
}

static void a3d_dropbox_drag(a3d_widget_t* widget,
                             float x, float y,
                             float dx, float dy,
                             double dt)
{
	assert(widget);
	LOGD("debug");

	a3d_dropbox_t* self = (a3d_dropbox_t*) widget;
	a3d_widget_drag((a3d_widget_t*) self->drop_text, x, y, dx, dy, dt);
	if(self->drop)
	{
		a3d_widget_drag(self->drop_widget, x, y, dx, dy, dt);
	}
}

static void a3d_dropbox_draw(a3d_widget_t* widget)
{
	assert(widget);
	LOGD("debug");

	a3d_dropbox_t* self = (a3d_dropbox_t*) widget;
	a3d_widget_draw((a3d_widget_t*) self->drop_text);
	if(self->drop)
	{
		a3d_widget_draw(self->drop_widget);
	}
}

static void a3d_dropbox_refresh(a3d_widget_t* widget)
{
	assert(widget);
	LOGD("debug");

	a3d_dropbox_t* self = (a3d_dropbox_t*) widget;
	a3d_widget_refresh((a3d_widget_t*) self->drop_text);
	a3d_widget_refresh(self->drop_widget);
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_dropbox_t* a3d_dropbox_new(a3d_screen_t* screen,
                               int wsize,
                               int anchor,
                               int wraph, int wrapv,
                               int style_border,
                               int style_line,
                               a3d_vec4f_t* color_fill,
                               a3d_vec4f_t* color_line,
                               int text_anchor,
                               int text_style_border,
                               int text_style_line,
                               int text_style_text,
                               a3d_vec4f_t* text_color_fill,
                               a3d_vec4f_t* text_color_line,
                               a3d_vec4f_t* text_color_text,
                               int text_max_len,
                               int text_indent,
                               a3d_widget_t* drop_widget)
{
	assert(screen);
	assert(color_fill);
	assert(color_line);
	assert(text_color_fill);
	assert(text_color_line);
	assert(text_color_text);
	assert(drop_widget);
	LOGD("debug wsize=%i, anchor=%i, wraph=%i, wrapv=%i, style_border=%i, style_line=%i",
	     wsize, anchor, wraph, wrapv, style_border, style_line);
	LOGD("debug color_fill: r=%f, g=%f, b=%f, a=%f",
	     color_fill->r, color_fill->g, color_fill->b, color_fill->a);
	LOGD("debug color_line: r=%f, g=%f, b=%f, a=%f",
	     color_line->r, color_line->g, color_line->b, color_line->a);
	LOGD("text_anchor=%i, text_style_border=%i, text_style_line=%i, text_style_text=%i",
	     text_anchor, text_style_border, text_style_line, text_style_text);
	LOGD("debug color_fill: r=%f, g=%f, b=%f, a=%f",
	     color_fill->r, color_fill->g, color_fill->b, color_fill->a);
	LOGD("debug color_line: r=%f, g=%f, b=%f, a=%f",
	     color_line->r, color_line->g, color_line->b, color_line->a);
	LOGD("debug color_text: r=%f, g=%f, b=%f, a=%f",
	     color_text->r, color_text->g, color_text->b, color_text->a);
	LOGD("debug text_max_len=%i, text_indent=%i", text_max_len, text_indent);

	if(wsize == 0)
	{
		wsize = sizeof(a3d_dropbox_t);
	}

	a3d_dropbox_t* self = (a3d_dropbox_t*) a3d_widget_new(screen,
	                                                      wsize,
	                                                      anchor,
	                                                      wraph,
	                                                      wrapv,
	                                                      style_border,
	                                                      style_line,
	                                                      color_line,
	                                                      color_fill,
	                                                      a3d_dropbox_size,
	                                                      a3d_dropbox_click,
	                                                      a3d_dropbox_layout,
	                                                      a3d_dropbox_drag,
	                                                      a3d_dropbox_draw,
	                                                      a3d_dropbox_refresh);
	if(self == NULL)
	{
		return NULL;
	}

	self->drop_text = a3d_droptext_new(screen,
	                                   0,
	                                   text_anchor,
	                                   text_style_border,
	                                   text_style_line,
	                                   text_style_text,
	                                   text_color_fill,
	                                   text_color_line,
	                                   text_color_text,
	                                   text_max_len,
	                                   text_indent,
	                                   &self->drop);
	if(self->drop_text == NULL)
	{
		goto fail_text;
	}

	self->drop        = 0;
	self->drop_widget = drop_widget;

	// success
	return self;

	// failure
	fail_text:
		a3d_widget_delete((a3d_widget_t**) &self);
	return NULL;
}

void a3d_dropbox_delete(a3d_dropbox_t** _self)
{
	assert(_self);

	a3d_dropbox_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		a3d_droptext_delete(&self->drop_text);
		a3d_widget_delete((a3d_widget_t**) _self);
	}
}
