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

#include "a3d_text.h"
#include "a3d_screen.h"
#include "a3d_font.h"
#include "../math/a3d_regionf.h"
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define LOG_TAG "a3d"
#include "a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static void a3d_text_size(a3d_widget_t* widget,
                          float* w, float* h)
{
	assert(widget);
	assert(w);
	assert(h);
	LOGD("debug");

	a3d_text_t* self = (a3d_text_t*) widget;
	a3d_font_t* font = a3d_screen_font(widget->screen);
	int         len  = strlen(self->string);
	float       size = a3d_screen_layoutText(widget->screen, self->style);
	float       r    = a3d_font_aspectRatio(font);
	*w = r*size*len;
	*h = size;
}

static void a3d_text_draw(a3d_widget_t* widget)
{
	assert(widget);
	LOGD("debug");

	a3d_text_t* self = (a3d_text_t*) widget;

	int len = strlen(self->string);
	if(len <= 0)
	{
		return;
	}

	a3d_mat4f_t mvp;
	float       w    = 0.0f;
	float       h    = 0.0f;
	float       x    = widget->rect_draw.l;
	float       y    = widget->rect_draw.t;
	float       size = widget->rect_draw.h;
	a3d_font_t* font = a3d_screen_font(widget->screen);
	a3d_screen_sizef(widget->screen, &w, &h);
	a3d_mat4f_ortho(&mvp, 1, 0.0f, w, h, 0.0f, 0.0f, 2.0f);
	a3d_mat4f_translate(&mvp, 0, x, y, -1.0f);
	a3d_mat4f_scale(&mvp, 0, size, size, 1.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, font->id_tex);
	glUseProgram(font->prog);
	glEnableVertexAttribArray(font->attr_vertex);
	glEnableVertexAttribArray(font->attr_coords);
	glBindBuffer(GL_ARRAY_BUFFER, self->id_vertex);
	glVertexAttribPointer(font->attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, self->id_coords);
	glVertexAttribPointer(font->attr_coords, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glUniform4fv(font->unif_color, 1, (GLfloat*) &self->color);
	glUniform1i(font->unif_sampler, 0);
	glUniformMatrix4fv(font->unif_mvp, 1, GL_FALSE, (GLfloat*) &mvp);
	glDrawArrays(GL_TRIANGLES, 0, 2*3*len);
	glDisableVertexAttribArray(font->attr_coords);
	glDisableVertexAttribArray(font->attr_vertex);
	glUseProgram(0);
	glDisable(GL_BLEND);
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_text_t* a3d_text_new(a3d_screen_t* screen,
                         int wsize,
                         int anchor,
                         int style_border,
                         int style_line,
                         int style_text,
                         a3d_vec4f_t* color_fill,
                         a3d_vec4f_t* color_line,
                         a3d_vec4f_t* color_text,
                         int max_len,
                         a3d_widget_click_fn click_fn)
{
	// click_fn may be NULL
	assert(screen);
	assert(color_fill);
	assert(color_line);
	assert(color_text);
	LOGD("debug wsize=%i, anchor=%i, style_border=%i, style_line=%i, style_text=%i",
	     wsize, anchor, style_border, style_line, style_text);
	LOGD("debug color_fill: r=%f, g=%f, b=%f, a=%f",
	     color_fill->r, color_fill->g, color_fill->b, color_fill->a);
	LOGD("debug color_line: r=%f, g=%f, b=%f, a=%f",
	     color_line->r, color_line->g, color_line->b, color_line->a);
	LOGD("debug color_text: r=%f, g=%f, b=%f, a=%f",
	     color_text->r, color_text->g, color_text->b, color_text->a);
	LOGD("debug max_len=%i", max_len);

	if(wsize == 0)
	{
		wsize = sizeof(a3d_text_t);
	}

	a3d_text_t* self = (a3d_text_t*) a3d_widget_new(screen,
	                                                wsize,
	                                                anchor,
	                                                A3D_WIDGET_WRAP_SHRINK,
	                                                A3D_WIDGET_WRAP_SHRINK,
	                                                style_border,
	                                                style_line,
	                                                color_line,
	                                                color_fill,
	                                                a3d_text_size,
	                                                click_fn,
	                                                NULL,
	                                                NULL,
	                                                a3d_text_draw);
	if(self == NULL)
	{
		return NULL;
	}

	self->string = calloc(max_len, sizeof(char));
	if(self->string == NULL)
	{
		LOGE("calloc failed");
		goto fail_string;
	}

	int vertex_size = 18 * (max_len - 1);   // 2 * 3 * xyz
	self->vertex = (GLfloat*) malloc(sizeof(GLfloat) * vertex_size);
	if(self->vertex == NULL)
	{
		LOGE("malloc failed");
		goto fail_vertex;
	}

	int coords_size = 12 * (max_len - 1);   // 2 * 3 * uv
	self->coords = (GLfloat*) malloc(sizeof(GLfloat) * coords_size);
	if(self->coords == NULL)
	{
		LOGE("malloc failed");
		goto fail_coords;
	}

	self->max_len = max_len;
	self->style   = style_text;
	a3d_vec4f_copy(color_text, &self->color);
	glGenBuffers(1, &self->id_vertex);
	glGenBuffers(1, &self->id_coords);

	// success
	return self;

	// failure
	fail_coords:
		free(self->vertex);
	fail_vertex:
		free(self->string);
	fail_string:
		a3d_widget_delete((a3d_widget_t**) &self);
	return NULL;
}

void a3d_text_delete(a3d_text_t** _self)
{
	assert(_self);

	a3d_text_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		glDeleteBuffers(1, &self->id_vertex);
		glDeleteBuffers(1, &self->id_coords);

		free(self->coords);
		free(self->vertex);
		free(self->string);

		a3d_widget_delete((a3d_widget_t**) _self);
	}
}

void a3d_text_printf(a3d_text_t* self,
                     const char* fmt, ...)
{
	assert(self);
	assert(fmt);

	// decode string
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(self->string, self->max_len, fmt, argptr);
	va_end(argptr);

	LOGD("debug %s", self->string);

	// compute vertex/coords for glDrawArrays
	// quad vertex/coords order
	// bl.xyz/uv, tl.xyz/uv, br.xyz/uv, tr.xyz/uv
	int i;
	a3d_regionf_t coords;
	a3d_regionf_t vertex;
	a3d_widget_t* widget = (a3d_widget_t*) self;
	a3d_font_t*   font   = a3d_screen_font(widget->screen);
	int len = strlen(self->string);
	float offset = 0.0f;
	for(i = 0; i < len; ++i)
	{
		a3d_font_request(font,
		                 self->string[i],
		                 &coords, &vertex);

		self->vertex[18*i +  0] = vertex.l + offset;   // tl.xyz
		self->vertex[18*i +  1] = vertex.t;
		self->vertex[18*i +  2] = 0.0f;
		self->vertex[18*i +  3] = vertex.l + offset;   // bl.xyz
		self->vertex[18*i +  4] = vertex.b;
		self->vertex[18*i +  5] = 0.0f;
		self->vertex[18*i +  6] = vertex.r + offset;   // br.xyz
		self->vertex[18*i +  7] = vertex.b;
		self->vertex[18*i +  8] = 0.0f;

		self->vertex[18*i +  9] = vertex.l + offset;   // tl.xyz
		self->vertex[18*i + 10] = vertex.t;
		self->vertex[18*i + 11] = 0.0f;
		self->vertex[18*i + 12] = vertex.r + offset;   // br.xyz
		self->vertex[18*i + 13] = vertex.b;
		self->vertex[18*i + 14] = 0.0f;
		self->vertex[18*i + 15] = vertex.r + offset;   // tr.xyz
		self->vertex[18*i + 16] = vertex.t;
		self->vertex[18*i + 17] = 0.0f;

		self->coords[12*i + 0] = coords.l;   // tl.uv
		self->coords[12*i + 1] = coords.t;
		self->coords[12*i + 2] = coords.l;   // bl.uv
		self->coords[12*i + 3] = coords.b;
		self->coords[12*i + 4] = coords.r;   // br.uv
		self->coords[12*i + 5] = coords.b;

		self->coords[12*i +  6] = coords.l;   // tl.uv
		self->coords[12*i +  7] = coords.t;
		self->coords[12*i +  8] = coords.r;   // br.uv
		self->coords[12*i +  9] = coords.b;
		self->coords[12*i + 10] = coords.r;   // tr.uv
		self->coords[12*i + 11] = coords.t;

		// next character offset
		offset += vertex.r;
	}
	int vertex_size = 18*len;   // 2 * 3 * xyz
	int coords_size = 12*len;   // 2 * 3 * uv
	glBindBuffer(GL_ARRAY_BUFFER, self->id_vertex);
	glBufferData(GL_ARRAY_BUFFER, vertex_size*sizeof(GLfloat),
	             self->vertex, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, self->id_coords);
	glBufferData(GL_ARRAY_BUFFER, coords_size*sizeof(GLfloat),
	             self->coords, GL_STATIC_DRAW);
}

void a3d_text_color(a3d_text_t* self,
                    a3d_vec4f_t* color)
{
	assert(self);
	assert(color);
	LOGD("debug r=%f, g=%f, b=%f, a=%f",
	     color->r, color->g, color->b, color->a);

	a3d_vec4f_copy(color, &self->color);
}
