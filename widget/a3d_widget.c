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

#include "../math/a3d_vec2f.h"
#include "a3d_widget.h"
#include "a3d_text.h"
#include "a3d_screen.h"
#include "../a3d_shader.h"
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static const char* VSHADER =
	"attribute vec2 xy;\n"
	"uniform   mat4 mvp;\n"
	"\n"
	"varying vec2 varying_xy;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	varying_xy= xy;\n"
	"	gl_Position  = mvp*vec4(xy, 0.0, 1.0);\n"
	"}\n";

static const char* FSHADER =
	"#ifdef GL_ES\n"
	"precision mediump float;\n"
	"precision mediump int;\n"
	"#endif\n"
	"\n"
	"uniform vec4 color0;\n"
	"uniform vec4 color1;\n"
	"uniform vec2 ab;\n"
	"\n"
	"varying vec2 varying_xy;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	if((varying_xy.y < ab.x) ||\n"
	"	   (varying_xy.y > ab.y))\n"
	"	{\n"
	"		gl_FragColor = color0;\n"
	"	}\n"
	"	else\n"
	"	{\n"
	"		gl_FragColor = color1;\n"
	"	}\n"
	"}\n";

static int a3d_widget_shaders(a3d_widget_t* self)
{
	assert(self);

	self->prog = a3d_shader_make_source(VSHADER, FSHADER);
	if(self->prog == 0)
	{
		return 0;
	}

	self->attr_xy     = glGetAttribLocation(self->prog, "xy");
	self->unif_mvp    = glGetUniformLocation(self->prog, "mvp");
	self->unif_color0 = glGetUniformLocation(self->prog, "color0");
	self->unif_color1 = glGetUniformLocation(self->prog, "color1");
	self->unif_ab     = glGetUniformLocation(self->prog, "ab");

	return 1;
}

static void a3d_widget_makeRoundRect(a3d_vec2f_t* xy, int steps,
                                     float t, float l,
                                     float b, float r,
                                     float radius)
{
	assert(xy);

	// top-right
	int   i;
	int   idx = 0;
	float s   = (float) (steps - 1);
	for(i = 0; i < steps; ++i)
	{
		float ang = 0.0f + 90.0f*((float) i/s);
		xy[idx].x = r + radius*cosf(ang*M_PI/180.0f);
		xy[idx].y = t - radius*sinf(ang*M_PI/180.0f);
		idx++;
	}

	// top-left
	for(i = 0; i < steps; ++i)
	{
		float ang = 90.0f + 90.0f*((float) i/s);
		xy[idx].x = l + radius*cosf(ang*M_PI/180.0f);
		xy[idx].y = t - radius*sinf(ang*M_PI/180.0f);
		idx++;
	}

	// bottom-left
	for(i = 0; i < steps; ++i)
	{
		float ang = 180.0f + 90.0f*((float) i/s);
		xy[idx].x = l + radius*cosf(ang*M_PI/180.0f);
		xy[idx].y = b - radius*sinf(ang*M_PI/180.0f);
		idx++;
	}

	// bottom-right
	for(i = 0; i < steps; ++i)
	{
		float ang = 270.0f + 90.0f*((float) i/s);
		xy[idx].x = r + radius*cosf(ang*M_PI/180.0f);
		xy[idx].y = b - radius*sinf(ang*M_PI/180.0f);
		idx++;
	}
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_widget_t* a3d_widget_new(struct a3d_screen_s* screen,
                             int wsize,
                             a3d_widgetLayout_t* layout,
                             a3d_widgetStyle_t* style,
                             a3d_widgetFn_t* fn)
{
	assert(screen);
	assert(layout);
	assert(style);
	assert(fn);

	if(wsize == 0)
	{
		wsize = sizeof(a3d_widget_t);
	}

	a3d_widget_t* self = (a3d_widget_t*) calloc(1, wsize);
	if(self == NULL)
	{
		LOGE("calloc failed");
		return NULL;
	}

	self->screen   = screen;
	self->anchor   = A3D_WIDGET_ANCHOR_TL;
	self->sound_fx = 1;

	memcpy(&self->layout, layout, sizeof(a3d_widgetLayout_t));
	memcpy(&self->style, style, sizeof(a3d_widgetStyle_t));
	memcpy(&self->fn, fn, sizeof(a3d_widgetFn_t));

	// check for invalid layouts
	if(layout->wrapx == A3D_WIDGET_WRAP_SHRINK)
	{
		assert(layout->aspectx != A3D_WIDGET_ASPECT_SQUARE);
	}

	if(layout->wrapy == A3D_WIDGET_WRAP_SHRINK)
	{
		assert(layout->aspecty != A3D_WIDGET_ASPECT_SQUARE);
	}

	glGenBuffers(1, &self->id_xy_widget);
	glGenBuffers(1, &self->id_xy_scroll);

	if(a3d_widget_shaders(self) == 0)
	{
		goto fail_shaders;
	}

	// success
	return self;

	// failure
	fail_shaders:
		free(self);
	return NULL;
}

void a3d_widget_delete(a3d_widget_t** _self)
{
	assert(_self);

	a3d_widget_t* self = *_self;
	if(self)
	{
		// TODO - screen top

		if(a3d_widget_hasFocus(self))
		{
			a3d_screen_focus(self->screen, NULL);
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &self->id_xy_scroll);
		glDeleteBuffers(1, &self->id_xy_widget);
		glDeleteProgram(self->prog);
		free(self);
		*_self = NULL;
	}
}

void a3d_widget_layoutXYClip(a3d_widget_t* self,
                             float x, float y,
                             a3d_rect4f_t* clip,
                             int dragx, int dragy)
{
	assert(self);
	assert(clip);

	a3d_widgetLayout_t* layout = &self->layout;
	a3d_widgetFn_t*     fn     = &self->fn;

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
	a3d_screen_layoutBorder(self->screen, layout->border,
	                        &h_bo, &v_bo);
	self->rect_border.t = t;
	self->rect_border.l = l;
	self->rect_draw.t   = t + v_bo;
	self->rect_draw.l   = l + h_bo;

	// allow the widget to layout it's children
	a3d_widget_layoutFn layout_fn = fn->layout_fn;
	if(layout_fn)
	{
		(*layout_fn)(self, dragx, dragy);
	}

	// initialize rounded rectangle
	float b = self->rect_border.t + self->rect_border.h;
	float r = self->rect_border.l + self->rect_border.w;
	float radius = (h_bo == v_bo) ? h_bo : 0.0f;
	int steps    = A3D_WIDGET_BEZEL;
	int size_xy = 4*steps;   // corners*steps
	a3d_vec2f_t xy[size_xy];
	a3d_widget_makeRoundRect(xy, steps,
	                         t + v_bo, l + h_bo,
	                         b - v_bo, r - v_bo,
	                         radius);
	glBindBuffer(GL_ARRAY_BUFFER, self->id_xy_widget);
	glBufferData(GL_ARRAY_BUFFER, size_xy*sizeof(a3d_vec2f_t),
	             xy, GL_STATIC_DRAW);

	a3d_rect4f_t rect_border_clip;
	if(a3d_rect4f_intersect(&self->rect_border,
	                        &self->rect_clip,
	                        &rect_border_clip))
	{
		if(layout->scroll_bar)
		{
			float h_bo = 0.0f;
			float v_bo = 0.0f;
			a3d_screen_layoutBorder(self->screen,
			                        A3D_WIDGET_BORDER_MEDIUM,
			                        &h_bo, &v_bo);

			float w  = h_bo;
			float t  = rect_border_clip.t;
			float l  = rect_border_clip.l + rect_border_clip.w - w;
			float h  = rect_border_clip.h;
			float b  = t + h;
			float r  = l + w;
			int   sz = 8;   // 4*xy
			GLfloat xy[] =
			{
				l, t,   // top-left
				l, b,   // bottom-left
				r, t,   // top-right
				r, b,   // bottom-right
			};
			glBindBuffer(GL_ARRAY_BUFFER, self->id_xy_scroll);
			glBufferData(GL_ARRAY_BUFFER, sz*sizeof(GLfloat),
			             xy, GL_STATIC_DRAW);
		}
	}
}

void a3d_widget_layoutSize(a3d_widget_t* self,
                           float* w, float* h)
{
	assert(self);
	assert(w);
	assert(h);

	a3d_widgetLayout_t* layout = &self->layout;
	a3d_widgetFn_t*     fn     = &self->fn;

	float sw;
	float sh;
	a3d_screen_sizef(self->screen, &sw, &sh);

	a3d_font_t* font = a3d_screen_font(self->screen,
	                                   A3D_TEXT_FONTTYPE_REGULAR);
	float ar    = a3d_font_aspectRatioAvg(font);
	int   style = layout->wrapy - A3D_WIDGET_WRAP_STRETCH_TEXT_SMALL;
	float th    = a3d_screen_layoutText(self->screen, style);
	float tw    = ar*th;

	// screen/text/parent square
	float ssq = (sw > sh) ? sh : sw;
	float tsq = th;   // always use the height for square
	float psq = (*w > *h) ? *h : *w;
	int   sqw = (layout->aspectx == A3D_WIDGET_ASPECT_SQUARE);
	int   sqh = (layout->aspecty == A3D_WIDGET_ASPECT_SQUARE);

	// initialize size
	float h_bo = 0.0f;
	float v_bo = 0.0f;
	a3d_screen_layoutBorder(self->screen, layout->border,
	                        &h_bo, &v_bo);
	if(layout->wrapx == A3D_WIDGET_WRAP_SHRINK)
	{
		self->rect_draw.w   = *w - 2.0f*h_bo;
		self->rect_border.w = *w;
	}
	else
	{
		float rw = 0.0f;
		if(layout->wrapx == A3D_WIDGET_WRAP_STRETCH_SCREEN)
		{
			rw = sqw ? ssq : sw;
			rw *= layout->stretchx;
			self->rect_draw.w   = rw - 2.0f*h_bo;
			self->rect_border.w = rw;
		}
		else if((layout->wrapx >= A3D_WIDGET_WRAP_STRETCH_TEXT_SMALL) &&
		        (layout->wrapx <= A3D_WIDGET_WRAP_STRETCH_TEXT_LARGE))
		{
			rw = sqw ? tsq : tw;
			rw *= layout->stretchx;
			self->rect_draw.w   = rw;
			self->rect_border.w = rw + 2.0f*h_bo;
		}
		else
		{
			rw = sqw ? psq : *w;
			rw *= layout->stretchx;
			self->rect_draw.w   = rw - 2.0f*h_bo;
			self->rect_border.w = rw;
		}
	}

	// intersect draw with border interior
	if(self->rect_draw.w < 0.0f)
	{
		self->rect_draw.w = 0.0f;
	}

	if(layout->wrapy == A3D_WIDGET_WRAP_SHRINK)
	{
		self->rect_draw.h   = *h - 2.0f*v_bo;
		self->rect_border.h = *h;
	}
	else
	{
		float rh = 0.0f;
		if(layout->wrapy == A3D_WIDGET_WRAP_STRETCH_SCREEN)
		{
			rh = sqh ? ssq : sh;
			rh *= layout->stretchy;
			self->rect_draw.h   = rh - 2.0f*v_bo;
			self->rect_border.h = rh;
		}
		else if((layout->wrapy >= A3D_WIDGET_WRAP_STRETCH_TEXT_SMALL) &&
		        (layout->wrapy <= A3D_WIDGET_WRAP_STRETCH_TEXT_LARGE))
		{
			rh = sqh ? tsq : th;
			rh *= layout->stretchy;
			self->rect_draw.h   = rh;
			self->rect_border.h = rh + 2.0f*v_bo;
		}
		else
		{
			rh = sqh ? psq : *h;
			rh *= layout->stretchy;
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
	a3d_widget_reflowFn reflow_fn = fn->reflow_fn;
	if(reflow_fn)
	{
		(*reflow_fn)(self, draw_w, draw_h);
	}

	// compute draw size for shrink wrapped widgets and
	// recursively compute size of any children
	// the draw size of the widget also becomes the border
	// size of any children
	a3d_widget_sizeFn size_fn = fn->size_fn;
	if(size_fn)
	{
		(*size_fn)(self, &draw_w, &draw_h);
	}

	// wrap width
	if(layout->wrapx == A3D_WIDGET_WRAP_SHRINK)
	{
		self->rect_draw.w   = draw_w;
		self->rect_border.w = draw_w + 2.0f*h_bo;
	}

	// wrap height
	if(layout->wrapy == A3D_WIDGET_WRAP_SHRINK)
	{
		self->rect_draw.h   = draw_h;
		self->rect_border.h = draw_h + 2.0f*v_bo;
	}

	*w = self->rect_border.w;
	*h = self->rect_border.h;
}

void a3d_widget_layoutAnchor(a3d_widget_t* self,
                             a3d_rect4f_t* rect,
                             float* x, float * y)
{
	assert(self);
	assert(rect);
	assert(x);
	assert(y);

	// initialize to tl corner
	*x = rect->l;
	*y = rect->t;

	float w  = rect->w;
	float h  = rect->h;
	float w2 = w/2.0f;
	float h2 = h/2.0f;
	if(self->anchor == A3D_WIDGET_ANCHOR_TC)
	{
		*x += w2;
	}
	else if(self->anchor == A3D_WIDGET_ANCHOR_TR)
	{
		*x += w;
	}
	else if(self->anchor == A3D_WIDGET_ANCHOR_CL)
	{
		*y += h2;
	}
	else if(self->anchor == A3D_WIDGET_ANCHOR_CC)
	{
		*x += w2;
		*y += h2;
	}
	else if(self->anchor == A3D_WIDGET_ANCHOR_CR)
	{
		*x += w;
		*y += h2;
	}
	else if(self->anchor == A3D_WIDGET_ANCHOR_BL)
	{
		*y += h;
	}
	else if(self->anchor == A3D_WIDGET_ANCHOR_BC)
	{
		*x += w2;
		*y += h;
	}
	else if(self->anchor == A3D_WIDGET_ANCHOR_BR)
	{
		*x += w;
		*y += h;
	}
}

int a3d_widget_click(a3d_widget_t* self,
                     int state,
                     float x, float y)
{
	assert(self);

	a3d_widgetFn_t* fn = &self->fn;

	a3d_widget_clickFn click_fn = fn->click_fn;
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

	int clicked = (*click_fn)(self, self->fn.priv, state, x, y);
	if(clicked && self->sound_fx &&
	   (state == A3D_WIDGET_POINTER_UP))
	{
		a3d_screen_playClick(self->screen);
	}

	return clicked;
}

int a3d_widget_keyPress(a3d_widget_t* self,
                        int keycode, int meta)
{
	assert(self);

	a3d_widgetFn_t* fn = &self->fn;

	a3d_widget_keyPressFn keyPress_fn = fn->keyPress_fn;
	if(keyPress_fn == NULL)
	{
		return 0;
	}

	return (*keyPress_fn)(self, self->fn.priv, keycode, meta);
}

int a3d_widget_hasFocus(a3d_widget_t* self)
{
	assert(self);

	a3d_screen_t* screen = self->screen;
	return self == screen->focus_widget;
}

void a3d_widget_drag(a3d_widget_t* self,
                     float x, float y,
                     float dx, float dy)
{
	assert(self);

	a3d_widgetLayout_t* layout = &self->layout;
	a3d_widgetFn_t*     fn     = &self->fn;

	if((a3d_rect4f_contains(&self->rect_clip, x, y) == 0) ||
	   (a3d_rect4f_contains(&self->rect_border, x, y) == 0))
	{
		// don't drag if the pointer is outside the rect
		return;
	}

	if(layout->wrapx == A3D_WIDGET_WRAP_SHRINK)
	{
		self->drag_dx += dx;
		dx = 0.0f;
	}

	if(layout->wrapy == A3D_WIDGET_WRAP_SHRINK)
	{
		self->drag_dy += dy;
		dy = 0.0f;
	}

	// a shrink wrapped widget drags it's children by
	// changing it's own layout
	a3d_widget_dragFn drag_fn = fn->drag_fn;
	if(drag_fn &&
	   ((layout->wrapx > A3D_WIDGET_WRAP_SHRINK) ||
	    (layout->wrapy > A3D_WIDGET_WRAP_SHRINK)))
	{
		(*drag_fn)(self, x, y, dx, dy);
	}
}

void a3d_widget_draw(a3d_widget_t* self)
{
	assert(self);

	a3d_widgetLayout_t* layout = &self->layout;
	a3d_widgetStyle_t*  style  = &self->style;
	a3d_widgetFn_t*     fn     = &self->fn;

	a3d_rect4f_t rect_border_clip;
	if(a3d_rect4f_intersect(&self->rect_border,
	                        &self->rect_clip,
	                        &rect_border_clip) == 0)
	{
		return;
	}

	// fill the widget
	a3d_screen_t* screen       = self->screen;
	a3d_vec4f_t*  color_body   = &style->color_body;
	a3d_vec4f_t*  color_header = &style->color_header;
	float         alpha        = color_body->a;
	if(alpha > 0.0f)
	{
		a3d_screen_scissor(screen, &rect_border_clip);
		if(alpha < 1.0f)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		a3d_mat4f_t mvp;
		glBindBuffer(GL_ARRAY_BUFFER, self->id_xy_widget);
		a3d_mat4f_ortho(&mvp, 1, 0.0f, screen->w, screen->h, 0.0f, 0.0f, 2.0f);

		glEnableVertexAttribArray(self->attr_xy);
		glVertexAttribPointer(self->attr_xy, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glUseProgram(self->prog);
		glUniform4fv(self->unif_color0, 1, (const GLfloat*) color_header);
		glUniform4fv(self->unif_color1, 1, (const GLfloat*) color_body);
		glUniform2f(self->unif_ab, self->header_y,
		            rect_border_clip.t + rect_border_clip.h);
		glUniformMatrix4fv(self->unif_mvp, 1, GL_FALSE, (GLfloat*) &mvp);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4*A3D_WIDGET_BEZEL);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableVertexAttribArray(self->attr_xy);
		glUseProgram(0);
		if(alpha < 1.0f)
		{
			glDisable(GL_BLEND);
		}
	}

	// draw the contents
	a3d_rect4f_t rect_draw_clip;
	if(a3d_rect4f_intersect(&self->rect_draw,
	                        &self->rect_clip,
	                        &rect_draw_clip))
	{
		a3d_widget_drawFn draw_fn = fn->draw_fn;
		if(draw_fn)
		{
			a3d_screen_scissor(screen, &rect_draw_clip);
			(*draw_fn)(self);
		}

		// draw the scroll bar
		float s = rect_draw_clip.h/self->rect_draw.h;
		if(layout->scroll_bar && (s < 1.0f))
		{
			// clamp the start/end points
			float a = -self->drag_dy/self->rect_draw.h;
			float b = a + s;
			if(a < 0.0f)
			{
				a = 0.0f;
			}
			else if(a > 1.0f)
			{
				a = 1.0f;
			}

			if(b < 0.0f)
			{
				b = 0.0f;
			}
			else if(b > 1.0f)
			{
				b = 1.0f;
			}
			a = rect_border_clip.t + a*rect_border_clip.h;
			b = rect_border_clip.t + b*rect_border_clip.h;

			a3d_vec4f_t* c0 = &style->color_scroll0;
			a3d_vec4f_t* c1 = &style->color_scroll1;
			a3d_screen_scissor(screen, &rect_border_clip);
			if((c0->a < 1.0f) || (c1->a < 1.0f))
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}

			a3d_mat4f_t mvp;
			glBindBuffer(GL_ARRAY_BUFFER, self->id_xy_scroll);
			a3d_mat4f_ortho(&mvp, 1, 0.0f, screen->w, screen->h, 0.0f, 0.0f, 2.0f);

			glEnableVertexAttribArray(self->attr_xy);
			glVertexAttribPointer(self->attr_xy, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glUseProgram(self->prog);
			glUniform4f(self->unif_color0, c0->r, c0->g, c0->b, c0->a);
			glUniform4f(self->unif_color1, c1->r, c1->g, c1->b, c1->a);
			glUniform2f(self->unif_ab, a, b);
			glUniformMatrix4fv(self->unif_mvp, 1, GL_FALSE, (GLfloat*) &mvp);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDisableVertexAttribArray(self->attr_xy);
			glUseProgram(0);
			if((c0->a < 1.0f) || (c1->a < 1.0f))
			{
				glDisable(GL_BLEND);
			}
		}
	}
}

void a3d_widget_anchor(a3d_widget_t* self, int anchor)
{
	assert(self);

	self->anchor = anchor;
}

void a3d_widget_refresh(a3d_widget_t* self)
{
	assert(self);

	a3d_widgetFn_t* fn = &self->fn;

	a3d_widget_refreshFn refresh_fn = fn->refresh_fn;
	if(refresh_fn)
	{
		(*refresh_fn)(self, self->fn.priv);
	}
}

void a3d_widget_soundFx(a3d_widget_t* self,
                        int sound_fx)
{
	assert(self);

	self->sound_fx = sound_fx;
}

void a3d_widget_headerY(a3d_widget_t* self, float y)
{
	assert(self);

	self->header_y = y;
}

void a3d_widget_scrollTop(a3d_widget_t* self)
{
	assert(self);

	a3d_widgetFn_t* fn = &self->fn;

	self->drag_dx = 0.0f;
	self->drag_dy = 0.0f;

	a3d_widget_scrollTopFn scrollTop_fn = fn->scrollTop_fn;
	if(scrollTop_fn)
	{
		(*scrollTop_fn)(self);
	}
}
