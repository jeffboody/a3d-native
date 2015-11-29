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
	LOGD("debug");

	a3d_viewbox_t* self   = (a3d_viewbox_t*) widget;
	a3d_widget_t*  bullet = (a3d_widget_t*) self->bullet;
	a3d_widget_t*  body   = self->body;

	float wmax     = 0.0f;
	float hsum     = 0.0f;
	float bullet_w = *w;
	float bullet_h = *h;

	// layout bullet
	a3d_widget_layoutSize(bullet, &bullet_w, &bullet_h);
	wmax = bullet_w;
	hsum = bullet_h;

	// layout separator
	float h_bo = 0.0f;
	float v_bo = 0.0f;
	a3d_screen_layoutBorder(widget->screen,
	                        widget->style_border,
	                        &h_bo, &v_bo);
	hsum += 2.0f*v_bo;

	// layout body
	float h2 = *h - bullet_h - 2.0f*v_bo;
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

	*w = wmax;
	*h = hsum;
}

static int a3d_viewbox_click(a3d_widget_t* widget,
                             int state,
                             float x, float y)
{
	assert(widget);
	LOGD("debug state=%i, x=%f, y=%f", state, x, y);

	a3d_viewbox_t* self = (a3d_viewbox_t*) widget;
	if(a3d_widget_click((a3d_widget_t*) self->bullet,
	                    state, x, y) == 0)
	{
		return a3d_widget_click(self->body,
		                        state, x, y);
	}

	return 1;
}

static void a3d_viewbox_layout(a3d_widget_t* widget,
                               int dragx, int dragy)
{
	assert(widget);
	LOGD("debug dragx=%i, dragy=%i", dragx, dragy);

	a3d_viewbox_t* self   = (a3d_viewbox_t*) widget;
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

	// layout bullet
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

	// get separator size
	float h_bo = 0.0f;
	float v_bo = 0.0f;
	a3d_screen_layoutBorder(widget->screen,
	                        widget->style_border,
	                        &h_bo, &v_bo);

	// layout body
	rect_draw.t = t + th + 2.0f*v_bo;
	rect_draw.h = h - th - 2.0f*v_bo;
	a3d_widget_anchorPt(&rect_draw, body->anchor, &x, &y);
	a3d_rect4f_intersect(&rect_draw,
	                     &self->widget.rect_clip,
	                     &rect_clip);
	a3d_widget_layoutXYClip(body, x, y, &rect_clip,
	                        dragx, dragy);
}

static void a3d_viewbox_drag(a3d_widget_t* widget,
                             float x, float y,
                             float dx, float dy,
                             double dt)
{
	assert(widget);
	LOGD("debug");

	a3d_viewbox_t* self = (a3d_viewbox_t*) widget;
	a3d_widget_drag((a3d_widget_t*) self->bullet,
	                x, y, dx, dy, dt);
	a3d_widget_drag(self->body,
	                x, y, dx, dy, dt);
}

static void a3d_viewbox_draw(a3d_widget_t* widget)
{
	assert(widget);
	LOGD("debug");

	a3d_viewbox_t* self = (a3d_viewbox_t*) widget;
	a3d_widget_draw((a3d_widget_t*) self->bullet);
	a3d_widget_draw(self->body);

	// clip separator to border
	a3d_rect4f_t rect_border_clip;
	if(a3d_rect4f_intersect(&widget->rect_border,
	                        &widget->rect_clip,
	                        &rect_border_clip) == 0)
	{
		return;
	}

	float h_bo = 0.0f;
	float v_bo = 0.0f;
	a3d_screen_layoutBorder(widget->screen,
	                        widget->style_border,
	                        &h_bo, &v_bo);

	// clip separator
	a3d_widget_t* bw  = &(self->bullet->widget);
	float         y   = bw->rect_border.t + bw->rect_border.h + v_bo;
	float         top = widget->rect_clip.t;
	float         bot = widget->rect_clip.t + widget->rect_clip.h;
	if((y < top) || (y > bot))
	{
		return;
	}

	a3d_rect4f_t line =
	{
		.t = y,
		.l = rect_border_clip.l,
		.w = rect_border_clip.w,
		.h = 0.0f
	};

	// draw the separator
	a3d_screen_t* screen = widget->screen;
	a3d_vec4f_t*  c      = &widget->color_line;
	float         alpha  = widget->fade*c->a;
	if((alpha > 0.0f) && (widget->style_line != A3D_WIDGET_LINE_NONE))
	{
		glDisable(GL_SCISSOR_TEST);
		if(alpha < 1.0f)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		glUseProgram(screen->prog);
		glEnableVertexAttribArray(screen->attr_coords);

		float lw = a3d_screen_layoutLine(screen, widget->style_line);
		glLineWidth(lw);

		a3d_rect4f_t* r = &line;
		glBindBuffer(GL_ARRAY_BUFFER, screen->id_coords2);
		glVertexAttribPointer(screen->attr_coords, 2, GL_FLOAT, GL_FALSE, 0, 0);
		a3d_mat4f_t mvp;
		a3d_mat4f_ortho(&mvp, 1, 0.0f, screen->w, screen->h, 0.0f, 0.0f, 2.0f);
		glUniformMatrix4fv(screen->unif_mvp, 1, GL_FALSE, (GLfloat*) &mvp);
		glUniform4f(screen->unif_rect, r->t, r->l, r->w, r->h);
		glUniform4f(screen->unif_color, c->r, c->g, c->b, alpha);
		glDrawArrays(GL_LINES, 0, 2);

		glLineWidth(1.0f);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableVertexAttribArray(screen->attr_coords);
		glUseProgram(0);
		if(alpha < 1.0f)
		{
			glDisable(GL_BLEND);
		}
		glEnable(GL_SCISSOR_TEST);
	}
}

static int a3d_viewbox_fade(a3d_widget_t* widget,
                            float fade, float dt)
{
	assert(widget);
	LOGD("debug");

	int            animate = 0;
	a3d_viewbox_t* self    = (a3d_viewbox_t*) widget;
	a3d_widget_t*  bullet  = (a3d_widget_t*) self->bullet;
	animate |= a3d_widget_fade(bullet, fade, dt);
	animate |= a3d_widget_fade(self->body, fade, dt);
	return animate;
}

static void a3d_viewbox_refresh(a3d_widget_t* widget)
{
	assert(widget);
	LOGD("debug");

	a3d_viewbox_t* self = (a3d_viewbox_t*) widget;
	a3d_widget_refresh((a3d_widget_t*) self->bullet);
	a3d_widget_refresh(self->body);
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_viewbox_t* a3d_viewbox_new(a3d_screen_t* screen,
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
                               const char* sprite,
                               a3d_widget_click_fn click_fn,
                               void* click_priv,
                               a3d_widget_t* body)
{
	assert(screen);
	assert(color_fill);
	assert(color_line);
	assert(text_color_fill);
	assert(text_color_line);
	assert(text_color_text);
	assert(sprite);
	assert(body);

	if(wsize == 0)
	{
		wsize = sizeof(a3d_viewbox_t);
	}

	a3d_viewbox_t* self;
	self = (a3d_viewbox_t*) a3d_widget_new(screen,
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
	                                       NULL,
	                                       a3d_viewbox_size,
	                                       a3d_viewbox_click,
	                                       a3d_viewbox_layout,
	                                       a3d_viewbox_drag,
	                                       a3d_viewbox_draw,
	                                       a3d_viewbox_fade,
	                                       a3d_viewbox_refresh);
	if(self == NULL)
	{
		return NULL;
	}
	a3d_widget_soundFx((a3d_widget_t*) self, 0);

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
	                                 click_fn,
	                                 NULL);
	if(self->bullet == NULL)
	{
		goto fail_bullet;
	}
	a3d_widget_priv((a3d_widget_t*) self->bullet, click_priv);

	if(a3d_bulletbox_spriteLoad(self->bullet, 0, sprite) == 0)
	{
		goto fail_sprite;
	}

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

void a3d_viewbox_delete(a3d_viewbox_t** _self)
{
	assert(_self);

	a3d_viewbox_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		a3d_bulletbox_delete(&self->bullet);
		a3d_widget_delete((a3d_widget_t**) _self);
	}
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

	LOGD("debug %s", string);

	a3d_bulletbox_textPrintf(self->bullet, "%s", string);
}
