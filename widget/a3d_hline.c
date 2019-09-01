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

	a3d_hline_t* self = (a3d_hline_t*) widget;
	a3d_font_t*  font = a3d_screen_font(widget->screen,
	                                    A3D_SCREEN_FONT_REGULAR);
	float       size = a3d_screen_layoutText(widget->screen, self->style);
	if(self->wrapx == A3D_HLINE_WRAP_STRETCH)
	{
		float aspect  = a3d_font_aspectRatioAvg(font);
		int   max_len = self->max_len;
		*w = size*aspect*max_len;
	}
	else if(self->wrapx == A3D_HLINE_WRAP_STRETCH_PARENT)
	{
		// ignore
	}
	else
	{
		float width  = (float) a3d_hline_width(self);
		float height = (float) a3d_hline_height(self);
		*w = size*(width/height);
	}
	*h = size;
}

static void a3d_hline_draw(a3d_widget_t* widget)
{
	assert(widget);

	a3d_hline_t* self = (a3d_hline_t*) widget;

	// clip separator to border
	a3d_rect4f_t rect_border_clip;
	if(a3d_rect4f_intersect(&widget->rect_border,
	                        &widget->rect_clip,
	                        &rect_border_clip) == 0)
	{
		return;
	}

	// clip separator
	float top = widget->rect_clip.t;
	float h2  = widget->rect_clip.h/2.0f;

	a3d_rect4f_t line =
	{
		.t = top + h2,
		.l = rect_border_clip.l,
		.w = rect_border_clip.w,
		.h = 0.0f
	};

	// draw the separator
	a3d_screen_t* screen = widget->screen;
	a3d_vec4f_t*  c      = &self->color;
	float         alpha  = c->a;
	if(alpha > 0.0f)
	{
		glDisable(GL_SCISSOR_TEST);
		if(alpha < 1.0f)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		glUseProgram(screen->prog);
		glEnableVertexAttribArray(screen->attr_coords);

		int line_style = A3D_WIDGET_LINE_MEDIUM;
		if(self->style == A3D_HLINE_STYLE_SMALL)
		{
			line_style = A3D_WIDGET_LINE_SMALL;
		}
		else if(self->style == A3D_HLINE_STYLE_LARGE)
		{
			line_style = A3D_WIDGET_LINE_LARGE;
		}

		float lw = a3d_screen_layoutLine(screen, line_style);
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

/***********************************************************
* public                                                   *
***********************************************************/

a3d_hline_t* a3d_hline_new(a3d_screen_t* screen,
                           int wsize,
                           int style_line,
                           a3d_vec4f_t* color_line,
                           int max_len)
{
	assert(screen);
	assert(color_line);

	a3d_vec4f_t clear =
	{
		.r = 0.0f,
		.g = 0.0f,
		.b = 0.0f,
		.a = 0.0f
	};

	if(wsize == 0)
	{
		wsize = sizeof(a3d_hline_t);
	}

	a3d_hline_t* self = (a3d_hline_t*) a3d_widget_new(screen,
	                                                  wsize,
	                                                  A3D_WIDGET_WRAP_SHRINK,
	                                                  A3D_WIDGET_WRAP_SHRINK,
	                                                  A3D_WIDGET_STRETCH_NA,
	                                                  1.0f,
	                                                  A3D_WIDGET_BORDER_NONE,
	                                                  &clear,
	                                                  NULL,
	                                                  a3d_hline_size,
	                                                  NULL,
	                                                  NULL,
	                                                  NULL,
	                                                  a3d_hline_draw,
	                                                  NULL);
	if(self == NULL)
	{
		return NULL;
	}

	self->wrapx   = A3D_HLINE_WRAP_SHRINK;
	self->max_len = max_len;
	self->style   = style_line;
	a3d_vec4f_copy(color_line, &self->color);

	return self;
}

void a3d_hline_delete(a3d_hline_t** _self)
{
	assert(_self);

	a3d_hline_t* self = *_self;
	if(self)
	{
		a3d_widget_delete((a3d_widget_t**) _self);
	}
}

int a3d_hline_width(a3d_hline_t* self)
{
	assert(self);

	return self->max_len - 1;
}

int a3d_hline_height(a3d_hline_t* self)
{
	assert(self);

	a3d_widget_t* widget = (a3d_widget_t*) self;
	a3d_font_t*   font   = a3d_screen_font(widget->screen,
	                                       A3D_SCREEN_FONT_REGULAR);
	return a3d_font_height(font);
}

void a3d_hline_wrapx(a3d_hline_t* self, int wrapx)
{
	assert(self);

	if((wrapx < 0) || (wrapx >= A3D_HLINE_WRAP_COUNT))
	{
		LOGW("invalid wrapx=%i", wrapx);
		return;
	}

	self->wrapx = wrapx;
}
