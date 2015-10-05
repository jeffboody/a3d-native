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
#include <stdarg.h>
#include <stdio.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

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

	a3d_dropbox_t* self   = (a3d_dropbox_t*) widget;
	a3d_widget_t*  bullet = (a3d_widget_t*) self->bullet;
	a3d_widget_t*  body   = self->body;

	float wmax     = 0.0f;
	float hsum     = 0.0f;
	float bullet_w = *w;
	float bullet_h = *h;

	a3d_widget_layoutSize(bullet, &bullet_w, &bullet_h);
	wmax = bullet_w;
	hsum = bullet_h;

	if(self->drop)
	{
		float h2 = *h - bullet_h;
		if(h2 < 0.0f)
		{
			h2 = 0.0f;
		}

		float body_w = *w;
		float body_h = h2 ;
		a3d_widget_layoutSize(body, &body_w, &body_h);

		if(body_w > wmax)
		{
			wmax = body_w;
		}
		hsum += body_h;
	}

	*w = wmax;
	*h = hsum;
}

static int a3d_dropbox_click(a3d_widget_t* widget,
                             int state,
                             float x, float y)
{
	assert(widget);
	LOGD("debug state=%i, x=%f, y=%f", state, x, y);

	a3d_dropbox_t* self = (a3d_dropbox_t*) widget;
	if(a3d_widget_click((a3d_widget_t*) self->bullet,
	                    state, x, y) == 0)
	{
		if(self->drop)
		{
			return a3d_widget_click(self->body,
			                        state, x, y);
		}
		return 0;
	}

	return 1;
}

static void a3d_dropbox_layout(a3d_widget_t* widget,
                               int dragx, int dragy)
{
	assert(widget);
	LOGD("debug dragx=%i, dragy=%i", dragx, dragy);

	a3d_dropbox_t* self   = (a3d_dropbox_t*) widget;
	a3d_widget_t*  bullet = (a3d_widget_t*) self->bullet;
	a3d_widget_t*  body   = self->body;

	// initialize the layout
	float x  = 0.0f;
	float y  = 0.0f;
	float t  = self->widget.rect_draw.t;
	float l  = self->widget.rect_draw.l;
	float w  = self->widget.rect_draw.w;
	float h  = self->widget.rect_draw.h;
	float th = bullet->rect_border.h;
	a3d_rect4f_t rect_clip;
	a3d_rect4f_t rect_draw;

	rect_draw.t = t;
	rect_draw.l = l;
	rect_draw.w = w;
	rect_draw.h = th;

	a3d_widget_anchorPt(&rect_draw, bullet->anchor, &x, &y);
	a3d_rect4f_intersect(&rect_draw,
	                     &self->widget.rect_clip,
	                     &rect_clip);
	a3d_widget_layoutXYClip(bullet, x, y, &rect_clip,
	                        dragx, dragy);

	if(self->drop)
	{
		rect_draw.t = t + th;
		rect_draw.h = h - th;

		a3d_widget_anchorPt(&rect_draw, body->anchor, &x, &y);
		a3d_rect4f_intersect(&rect_draw,
		                     &self->widget.rect_clip,
		                     &rect_clip);
		a3d_widget_layoutXYClip(body, x, y, &rect_clip,
	                            dragx, dragy);
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
	a3d_widget_drag((a3d_widget_t*) self->bullet,
	                x, y, dx, dy, dt);
	if(self->drop)
	{
		a3d_widget_drag(self->body,
		                x, y, dx, dy, dt);
	}
}

static void a3d_dropbox_draw(a3d_widget_t* widget)
{
	assert(widget);
	LOGD("debug");

	a3d_dropbox_t* self = (a3d_dropbox_t*) widget;
	a3d_widget_draw((a3d_widget_t*) self->bullet);
	if(self->drop)
	{
		a3d_widget_draw(self->body);
	}
}

static void a3d_dropbox_refresh(a3d_widget_t* widget)
{
	assert(widget);
	LOGD("debug");

	a3d_dropbox_t* self = (a3d_dropbox_t*) widget;
	a3d_widget_refresh((a3d_widget_t*) self->bullet);
	a3d_widget_refresh(self->body);
}

static int a3d_dropbox_clickBullet(a3d_widget_t* widget,
                                   int state,
                                   float x, float y)
{
	assert(widget);
	LOGD("debug state=%i, x=%f, y=%f", state, x, y);

	a3d_dropbox_t* self = (a3d_dropbox_t*) widget->priv;
	if(state == A3D_WIDGET_POINTER_UP)
	{
		self->drop = 1 - self->drop;
	}
	return 1;
}

static void a3d_dropbox_refreshBullet(a3d_widget_t* widget)
{
	assert(widget);
	LOGD("debug x=%f, y=%f", x, y);

	a3d_dropbox_t* self = (a3d_dropbox_t*) widget->priv;
	a3d_bulletbox_spriteSelect(self->bullet, self->drop);
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_dropbox_t* a3d_dropbox_new(a3d_screen_t* screen,
                               int wsize,
                               int anchor,
                               int wrapx, int wrapy,
                               int stretch_mode,
                               float stretch_factor,
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
                               a3d_widget_t* body)
{
	assert(screen);
	assert(color_fill);
	assert(color_line);
	assert(text_color_fill);
	assert(text_color_line);
	assert(text_color_text);
	assert(body);
	LOGD("debug wsize=%i, anchor=%i, wrapx=%i, wrapy=%i",
	     wsize, anchor, wrapx, wrapy);
	LOGD("debug stretch_mode=%i, stretch_factor=%f, style_border=%i, style_line=%i",
	     stretch_mode, stretch_factor, style_border, style_line);
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
	LOGD("debug text_max_len=%i", text_max_len);

	if(wsize == 0)
	{
		wsize = sizeof(a3d_dropbox_t);
	}

	a3d_dropbox_t* self = (a3d_dropbox_t*) a3d_widget_new(screen,
	                                                      wsize,
	                                                      anchor,
	                                                      wrapx,
	                                                      wrapy,
	                                                      stretch_mode,
	                                                      stretch_factor,
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

	a3d_vec4f_t clear =
	{
		.r = 0.0f,
		.g = 0.0f,
		.b = 0.0f,
		.a = 0.0f
	};

	self->bullet = a3d_bulletbox_new(screen,
	                                 0,
	                                 text_anchor,
	                                 text_style_border,
	                                 text_style_line,
	                                 text_style_text,
	                                 &clear, &clear,
	                                 text_color_text, text_color_text,
	                                 text_max_len, 2,
	                                 a3d_dropbox_clickBullet,
	                                 a3d_dropbox_refreshBullet);
	if(self->bullet == NULL)
	{
		goto fail_bullet;
	}
	a3d_widget_priv((a3d_widget_t*) self->bullet, (void*) self);

	if(a3d_bulletbox_spriteLoad(self->bullet, 0,
	                            "$ic_arrow_drop_down_white_24dp.texz") == 0)
	{
		goto fail_sprite;
	}

	if(a3d_bulletbox_spriteLoad(self->bullet, 1,
	                            "$ic_arrow_drop_up_white_24dp.texz") == 0)
	{
		goto fail_sprite;
	}

	self->drop = 0;
	self->body = body;

	// success
	return self;

	// failure
	fail_sprite:
		a3d_bulletbox_delete(&self->bullet);
	fail_bullet:
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

		a3d_bulletbox_delete(&self->bullet);
		a3d_widget_delete((a3d_widget_t**) _self);
	}
}

void a3d_dropbox_raise(a3d_dropbox_t* self)
{
	assert(self);
	LOGD("debug");

	self->drop = 0;
}

void a3d_dropbox_textPrintf(a3d_dropbox_t* self,
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

	LOGD("debug %s", string);

	a3d_bulletbox_textPrintf(self->bullet, "%s", string);
}
