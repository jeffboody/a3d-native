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

#include "a3d_widget.h"
#include "a3d_text.h"
#include "a3d_screen.h"
#include <stdlib.h>
#include <assert.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

/***********************************************************
* public                                                   *
***********************************************************/

a3d_widget_t* a3d_widget_new(struct a3d_screen_s* screen,
                             int wsize,
                             int anchor,
                             int wrapx, int wrapy,
                             int stretch_mode,
                             float stretch_factor,
                             int style_border,
                             int style_line,
                             a3d_vec4f_t* color_line,
                             a3d_vec4f_t* color_fill,
                             a3d_widget_reflow_fn reflow_fn,
                             a3d_widget_size_fn size_fn,
                             a3d_widget_click_fn click_fn,
                             a3d_widget_layout_fn layout_fn,
                             a3d_widget_drag_fn drag_fn,
                             a3d_widget_draw_fn draw_fn,
                             a3d_widget_fade_fn fade_fn,
                             a3d_widget_refresh_fn refresh_fn)
{
	// reflow_fn, size_fn, click_fn, layout_fn, refresh_fn and draw_fn may be NULL
	assert(screen);
	assert(color_line);
	assert(color_fill);
	LOGD("debug wsize=%i, anchor=%i, wrapx=%i, wrapy=%i",
	     wsize, anchor, wrapx, wrapy);
	LOGD("debug stretch_mode=%i, stretch_factor=%f, style_border=%i, style_line=%i",
	     stretch_mode, stretch_factor, style_border, style_line);
	LOGD("debug color_line: r=%f, g=%f, b=%f, a=%f",
	     color_line->r, color_line->g, color_line->b, color_line->a);
	LOGD("debug color_fill: r=%f, g=%f, b=%f, a=%f",
	     color_fill->r, color_fill->g, color_fill->b, color_fill->a);

	if(wsize == 0)
	{
		wsize = sizeof(a3d_widget_t);
	}

	a3d_widget_t* self = (a3d_widget_t*) malloc(wsize);
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->screen         = screen;
	self->priv           = NULL;
	self->drag_dx        = 0.0f;
	self->drag_dy        = 0.0f;
	self->anchor         = anchor;
	self->wrapx          = wrapx;
	self->wrapy          = wrapy;
	self->stretch_mode   = stretch_mode;
	self->stretch_factor = stretch_factor;
	self->style_border   = style_border;
	self->style_line     = style_line;
	self->reflow_fn      = reflow_fn;
	self->size_fn        = size_fn;
	self->click_fn       = click_fn;
	self->keyPress_fn    = NULL;
	self->layout_fn      = layout_fn;
	self->drag_fn        = drag_fn;
	self->refresh_fn     = refresh_fn;
	self->draw_fn        = draw_fn;
	self->fade_fn        = fade_fn;
	self->fade           = 0.0f;
	self->sound_fx       = 1;

	a3d_rect4f_init(&self->rect_draw, 0.0f, 0.0f, 0.0f, 0.0f);
	a3d_rect4f_init(&self->rect_clip, 0.0f, 0.0f, 0.0f, 0.0f);
	a3d_rect4f_init(&self->rect_border, 0.0f, 0.0f, 0.0f, 0.0f);
	a3d_vec4f_copy(color_line, &self->color_line);
	a3d_vec4f_copy(color_fill, &self->color_fill);

	return self;
}

void a3d_widget_delete(a3d_widget_t** _self)
{
	assert(_self);

	a3d_widget_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		// TODO - screen top

		if(a3d_widget_hasFocus(self))
		{
			a3d_screen_focus(self->screen, NULL);
		}

		free(self);
		*_self = NULL;
	}
}

void a3d_widget_priv(a3d_widget_t* self, void* priv)
{
	assert(self);
	LOGD("debug");

	self->priv = priv;
}

void a3d_widget_layoutXYClip(a3d_widget_t* self,
                             float x, float y,
                             a3d_rect4f_t* clip,
                             int dragx, int dragy)
{
	assert(self);
	assert(clip);
	LOGD("debug x=%f, y=%f, dragx=%i, dragy=%i", x, y, dragx, dragy);

	float w  = self->rect_border.w;
	float h  = self->rect_border.h;
	float ct = clip->t;
	float cl = clip->l;
	float cw = clip->w;
	float ch = clip->h;
	float w2 = w/2.0f;
	float h2 = h/2.0f;
	float t  = y;
	float l  = x;

	a3d_rect4f_copy(clip, &self->rect_clip);

	// anchor the widget origin
	if(self->anchor == A3D_WIDGET_ANCHOR_TC)
	{
		l = x - w2;
	}
	else if(self->anchor == A3D_WIDGET_ANCHOR_TR)
	{
		l = x - w;
	}
	else if(self->anchor == A3D_WIDGET_ANCHOR_CL)
	{
		t = y - h2;
	}
	else if(self->anchor == A3D_WIDGET_ANCHOR_CC)
	{
		t = y - h2;
		l = x - w2;
	}
	else if(self->anchor == A3D_WIDGET_ANCHOR_CR)
	{
		t = y - h2;
		l = x - w;
	}
	else if(self->anchor == A3D_WIDGET_ANCHOR_BL)
	{
		t = y - h;
	}
	else if(self->anchor == A3D_WIDGET_ANCHOR_BC)
	{
		t = y - h;
		l = x - w2;
	}
	else if(self->anchor == A3D_WIDGET_ANCHOR_BR)
	{
		t = y - h;
		l = x - w;
	}

	// drag the widget (see dragable rules)
	if(dragx && (w > cw))
	{
		l += self->drag_dx;

		if(l > cl)
		{
			self->drag_dx -= l - cl;
			l = cl;
		}
		else if((l + w) < (cl + cw))
		{
			self->drag_dx += (cl + cw) - (l + w);
			l = cl + cw - w;
		}
		dragx = 0;
	}

	if(dragy && (h > ch))
	{
		t += self->drag_dy;

		if(t > ct)
		{
			self->drag_dy -= t - ct;
			t = ct;
		}
		else if((t + h) < (ct + ch))
		{
			self->drag_dy += (ct + ch) - (t + h);
			t = ct + ch - h;
		}
		dragy = 0;
	}

	// set the layout
	float h_bo = 0.0f;
	float v_bo = 0.0f;
	a3d_screen_layoutBorder(self->screen,
	                        self->style_border,
	                        &h_bo, &v_bo);
	self->rect_border.t = t;
	self->rect_border.l = l;
	self->rect_draw.t   = t + v_bo;
	self->rect_draw.l   = l + h_bo;

	// allow the widget to layout it's children
	a3d_widget_layout_fn layout_fn = self->layout_fn;
	if(layout_fn)
	{
		(*layout_fn)(self, dragx, dragy);
	}
}

void a3d_widget_layoutSize(a3d_widget_t* self,
                           float* w, float* h)
{
	assert(self);
	assert(w);
	assert(h);
	LOGD("debug w=%f, h=%f", *w, *h);

	float sw;
	float sh;
	a3d_screen_sizef(self->screen, &sw, &sh);

	a3d_font_t* font = a3d_screen_font(self->screen);
	float ar = a3d_font_aspectRatio(font);
	float th = 0.0f;
	if((self->wrapy >= A3D_WIDGET_WRAP_STRETCH_TEXT_SMALL) &&
	   (self->wrapy <= A3D_WIDGET_WRAP_STRETCH_TEXT_LARGE))
	{
		int style = self->wrapy - A3D_WIDGET_WRAP_STRETCH_TEXT_SMALL;
		th = a3d_screen_layoutText(self->screen, style);
	}
	float tw = ar*th;

	// screen/text/parent square
	float ssq = (sw > sh) ? sh : sw;
	float tsq = th;   // always use the height for square
	float psq = (*w > *h) ? *h : *w;
	int   sq  = (self->stretch_mode == A3D_WIDGET_STRETCH_SQUARE);

	// initialize size
	float h_bo = 0.0f;
	float v_bo = 0.0f;
	a3d_screen_layoutBorder(self->screen,
	                        self->style_border,
	                        &h_bo, &v_bo);
	if(self->wrapx == A3D_WIDGET_WRAP_SHRINK)
	{
		self->rect_draw.w   = *w - 2.0f*h_bo;
		self->rect_border.w = *w;
	}
	else
	{
		float rw = 0.0f;
		if(self->wrapx == A3D_WIDGET_WRAP_STRETCH_SCREEN)
		{
			rw = sq ? ssq : sw;
			rw *= self->stretch_factor;
			self->rect_draw.w   = rw - 2.0f*h_bo;
			self->rect_border.w = rw;
		}
		else if((self->wrapx >= A3D_WIDGET_WRAP_STRETCH_TEXT_SMALL) &&
		        (self->wrapx <= A3D_WIDGET_WRAP_STRETCH_TEXT_LARGE))
		{
			rw = sq ? tsq : tw;
			rw *= self->stretch_factor;
			self->rect_draw.w   = rw;
			self->rect_border.w = rw + 2.0f*h_bo;
		}
		else
		{
			rw = sq ? psq : *w;
			rw *= self->stretch_factor;
			self->rect_draw.w   = rw - 2.0f*h_bo;
			self->rect_border.w = rw;
		}
	}

	// intersect draw with border interior
	if(self->rect_draw.w < 0.0f)
	{
		self->rect_draw.w = 0.0f;
	}

	if(self->wrapy == A3D_WIDGET_WRAP_SHRINK)
	{
		self->rect_draw.h   = *h - 2.0f*v_bo;
		self->rect_border.h = *h;
	}
	else
	{
		float rh = 0.0f;
		if(self->wrapy == A3D_WIDGET_WRAP_STRETCH_SCREEN)
		{
			rh = sq ? ssq : sh;
			rh *= self->stretch_factor;
			self->rect_draw.h   = rh - 2.0f*v_bo;
			self->rect_border.h = rh;
		}
		else if((self->wrapy >= A3D_WIDGET_WRAP_STRETCH_TEXT_SMALL) &&
		        (self->wrapy <= A3D_WIDGET_WRAP_STRETCH_TEXT_LARGE))
		{
			rh = sq ? tsq : th;
			rh *= self->stretch_factor;
			self->rect_draw.h   = rh;
			self->rect_border.h = rh + 2.0f*v_bo;
		}
		else
		{
			rh = sq ? psq : *h;
			rh *= self->stretch_factor;
			self->rect_draw.h   = rh - 2.0f*v_bo;
			self->rect_border.h = rh;
		}
	}

	// intersect draw with border interior
	if(self->rect_draw.h < 0.0f)
	{
		self->rect_draw.h = 0.0f;
	}

	// reflow dynamically sized widgets (e.g. textbox)
	// this makes the most sense for stretched widgets
	float draw_w = self->rect_draw.w;
	float draw_h = self->rect_draw.h;
	a3d_widget_reflow_fn reflow_fn = self->reflow_fn;
	if(reflow_fn)
	{
		(*reflow_fn)(self, draw_w, draw_h);
	}

	// compute draw size for shrink wrapped widgets and
	// recursively compute size of any children
	// the draw size of the widget also becomes the border
	// size of any children
	a3d_widget_size_fn size_fn = self->size_fn;
	if(size_fn)
	{
		(*size_fn)(self, &draw_w, &draw_h);
	}

	// wrap width
	if(self->wrapx == A3D_WIDGET_WRAP_SHRINK)
	{
		self->rect_draw.w   = draw_w;
		self->rect_border.w = draw_w + 2.0f*h_bo;
	}

	// wrap height
	if(self->wrapy == A3D_WIDGET_WRAP_SHRINK)
	{
		self->rect_draw.h   = draw_h;
		self->rect_border.h = draw_h + 2.0f*v_bo;
	}

	*w = self->rect_border.w;
	*h = self->rect_border.h;
}

int a3d_widget_click(a3d_widget_t* self,
                     int state,
                     float x, float y)
{
	assert(self);
	LOGD("debug state=%i, x=%f, y=%f", state, x, y);

	a3d_widget_click_fn click_fn = self->click_fn;
	if(click_fn == NULL)
	{
		return 0;
	}

	if((a3d_rect4f_contains(&self->rect_clip, x, y) == 0) ||
	   (a3d_rect4f_contains(&self->rect_border, x, y) == 0))
	{
		// x, y is outside intersection of rect_border and rect_clip
		return 0;
	}

	int clicked = (*click_fn)(self, state, x, y);
	if(clicked && self->sound_fx &&
	   (state == A3D_WIDGET_POINTER_UP))
	{
		a3d_screen_playClick(self->screen);
	}

	return clicked;
}

void a3d_widget_keyPressFn(a3d_widget_t* self,
                           a3d_widget_keyPress_fn keyPress_fn)
{
	// keyPress_fn may be NULL
	assert(self);

	self->keyPress_fn = keyPress_fn;
}

int a3d_widget_keyPress(a3d_widget_t* self,
                        int keycode, int meta)
{
	assert(self);

	a3d_widget_keyPress_fn keyPress_fn = self->keyPress_fn;
	if(keyPress_fn == NULL)
	{
		return 0;
	}

	return (*keyPress_fn)(self, keycode, meta);
}

int a3d_widget_hasFocus(a3d_widget_t* self)
{
	assert(self);

	a3d_screen_t* screen = self->screen;
	return self == screen->focus_widget;
}

void a3d_widget_drag(a3d_widget_t* self,
                     float x, float y,
                     float dx, float dy,
                     double dt)
{
	assert(self);
	LOGD("debug dx=%f, dy=%f, dt=%lf", dx, dy, dt);

	if((a3d_rect4f_contains(&self->rect_clip, x, y) == 0) ||
	   (a3d_rect4f_contains(&self->rect_border, x, y) == 0))
	{
		// don't drag if the pointer is outside the rect
		return;
	}

	if(self->wrapx == A3D_WIDGET_WRAP_SHRINK)
	{
		self->drag_dx += dx;
		dx = 0.0f;
	}

	if(self->wrapy == A3D_WIDGET_WRAP_SHRINK)
	{
		self->drag_dy += dy;
		dy = 0.0f;
	}

	// a shrink wrapped widget drags it's children by
	// changing it's own layout
	a3d_widget_drag_fn drag_fn = self->drag_fn;
	if(drag_fn &&
	   ((self->wrapx > A3D_WIDGET_WRAP_SHRINK) ||
	    (self->wrapy > A3D_WIDGET_WRAP_SHRINK)))
	{
		(*drag_fn)(self, x, y, dx, dy, dt);
	}
}

void a3d_widget_draw(a3d_widget_t* self)
{
	assert(self);
	LOGD("debug");

	if(self->fade == 0.0f)
	{
		return;
	}

	a3d_rect4f_t rect_border_clip;
	if(a3d_rect4f_intersect(&self->rect_border,
	                        &self->rect_clip,
	                        &rect_border_clip) == 0)
	{
		return;
	}

	// draw the fill
	a3d_screen_t* screen = self->screen;
	a3d_vec4f_t*  c      = &self->color_fill;
	float         alpha  = self->fade*c->a;
	if(alpha > 0.0f)
	{
		a3d_screen_scissor(screen, &rect_border_clip);
		if(alpha < 1.0f)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		glUseProgram(screen->prog);
		glEnableVertexAttribArray(screen->attr_coords);

		a3d_rect4f_t* r = &self->rect_border;
		glBindBuffer(GL_ARRAY_BUFFER, screen->id_coords4);
		glVertexAttribPointer(screen->attr_coords, 2, GL_FLOAT, GL_FALSE, 0, 0);
		a3d_mat4f_t mvp;
		a3d_mat4f_ortho(&mvp, 1, 0.0f, screen->w, screen->h, 0.0f, 0.0f, 2.0f);
		glUniformMatrix4fv(screen->unif_mvp, 1, GL_FALSE, (GLfloat*) &mvp);
		glUniform4f(screen->unif_rect, r->t, r->l, r->w, r->h);
		glUniform4f(screen->unif_color, c->r, c->g, c->b, alpha);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableVertexAttribArray(screen->attr_coords);
		glUseProgram(0);
		if(alpha < 1.0f)
		{
			glDisable(GL_BLEND);
		}
	}

	// draw the contents
	a3d_widget_draw_fn draw_fn = self->draw_fn;
	if(draw_fn)
	{
		a3d_rect4f_t rect_draw_clip;
		if(a3d_rect4f_intersect(&self->rect_draw,
		                        &self->rect_clip,
		                        &rect_draw_clip))
		{
			a3d_screen_scissor(screen, &rect_draw_clip);
			(*draw_fn)(self);
		}
	}

	// draw the border
	c     = &self->color_line;
	alpha = self->fade*c->a;
	if((alpha > 0.0f) && (self->style_line != A3D_WIDGET_LINE_NONE))
	{
		glDisable(GL_SCISSOR_TEST);
		if(alpha < 1.0f)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		glUseProgram(screen->prog);
		glEnableVertexAttribArray(screen->attr_coords);

		float lw = a3d_screen_layoutLine(screen, self->style_line);
		glLineWidth(lw);

		a3d_rect4f_t* r = &rect_border_clip;
		glBindBuffer(GL_ARRAY_BUFFER, screen->id_coords4);
		glVertexAttribPointer(screen->attr_coords, 2, GL_FLOAT, GL_FALSE, 0, 0);
		a3d_mat4f_t mvp;
		a3d_mat4f_ortho(&mvp, 1, 0.0f, screen->w, screen->h, 0.0f, 0.0f, 2.0f);
		glUniformMatrix4fv(screen->unif_mvp, 1, GL_FALSE, (GLfloat*) &mvp);
		glUniform4f(screen->unif_rect, r->t, r->l, r->w, r->h);
		glUniform4f(screen->unif_color, c->r, c->g, c->b, alpha);
		glDrawArrays(GL_LINE_LOOP, 0, 4);

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

int a3d_widget_fade(a3d_widget_t* self, float fade, float dt)
{
	assert(self);

	int animate = 0;
	if(self->fade != fade)
	{
		// animate and clamp fade
		float dfade = 3.0f*dt;
		if(self->fade > fade)
		{
			self->fade -= dfade;
			if(self->fade < fade)
			{
				self->fade = fade;
			}
			else
			{
				animate = 1;
			}
		}
		else
		{
			self->fade += dfade;
			if(self->fade > fade)
			{
				self->fade = fade;
			}
			else
			{
				animate = 1;
			}
		}
	}

	a3d_widget_fade_fn fade_fn = self->fade_fn;
	if(fade_fn)
	{
		animate |= (*fade_fn)(self, fade, dt);
	}

	return animate;
}

// helper function
void a3d_widget_anchorPt(a3d_rect4f_t* rect,
                         int anchor,
                         float* x, float * y)
{
	assert(rect);
	assert(x);
	assert(y);
	LOGD("debug anchor=%i", anchor);

	// initialize to tl corner
	*x = rect->l;
	*y = rect->t;

	float w  = rect->w;
	float h  = rect->h;
	float w2 = w/2.0f;
	float h2 = h/2.0f;
	if(anchor == A3D_WIDGET_ANCHOR_TC)
	{
		*x += w2;
	}
	else if(anchor == A3D_WIDGET_ANCHOR_TR)
	{
		*x += w;
	}
	else if(anchor == A3D_WIDGET_ANCHOR_CL)
	{
		*y += h2;
	}
	else if(anchor == A3D_WIDGET_ANCHOR_CC)
	{
		*x += w2;
		*y += h2;
	}
	else if(anchor == A3D_WIDGET_ANCHOR_CR)
	{
		*x += w;
		*y += h2;
	}
	else if(anchor == A3D_WIDGET_ANCHOR_BL)
	{
		*y += h;
	}
	else if(anchor == A3D_WIDGET_ANCHOR_BC)
	{
		*x += w2;
		*y += h;
	}
	else if(anchor == A3D_WIDGET_ANCHOR_BR)
	{
		*x += w;
		*y += h;
	}
}

void a3d_widget_refresh(a3d_widget_t* self)
{
	assert(self);
	LOGD("debug");

	a3d_widget_refresh_fn refresh_fn = self->refresh_fn;
	if(refresh_fn)
	{
		(*refresh_fn)(self);
	}
}

void a3d_widget_soundFx(a3d_widget_t* self,
                        int sound_fx)
{
	assert(self);

	self->sound_fx = sound_fx;
}
