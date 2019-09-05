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
#include "a3d_key.h"
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

static int a3d_text_resize(a3d_text_t* self, size_t string_size)
{
	assert(self);

	if(self->string_size >= string_size)
	{
		return 1;
	}

	char* string = (char*) realloc(self->string, string_size);
	if(string == NULL)
	{
		LOGE("realloc failed");
		return 0;
	}
	string[string_size - 1] = '\0';

	// allocate size for string and cursor character
	// which is stored in place of the null character
	int vertex_size = 18*string_size;   // 2*3*xyz
	GLfloat* vertex;
	vertex = (GLfloat*)
	         realloc(self->vertex, sizeof(GLfloat)*vertex_size);
	if(vertex == NULL)
	{
		LOGE("realloc failed");
		return 0;
	}

	int coords_size = 12*string_size;   // 2*3*uv
	GLfloat* coords;
	coords = (GLfloat*)
	         realloc(self->coords, sizeof(GLfloat)*coords_size);
	if(coords == NULL)
	{
		LOGE("realloc failed");
		return 0;
	}

	self->string      = string;
	self->string_size = string_size;
	self->vertex      = vertex;
	self->coords      = coords;
	return 1;
}

static void a3d_text_size(a3d_widget_t* widget,
                          float* w, float* h)
{
	assert(widget);
	assert(w);
	assert(h);

	a3d_text_t* self = (a3d_text_t*) widget;

	float size   = a3d_screen_layoutText(widget->screen,
	                                     self->text_size);
	float width  = (float) 0.0f;
	float height = (float) a3d_text_height(self);
	if(a3d_widget_hasFocus(widget))
	{
		width = (float) a3d_text_width(self, 1);
	}
	else
	{
		width = (float) a3d_text_width(self, 0);
	}
	*w = size*(width/height);
	*h = size;
}

static void a3d_text_draw(a3d_widget_t* widget)
{
	assert(widget);

	a3d_text_t* self = (a3d_text_t*) widget;

	int len = strlen(self->string);
	if(a3d_widget_hasFocus(widget))
	{
		// add the cursor
		double period = 1.0;
		double t      = a3d_timestamp();
		if(fmod(t, period) < 0.5*period)
		{
			++len;
		}
	}
	else if(len == 0)
	{
		return;
	}

	a3d_vec4f_t* c     = &self->color;
	float        alpha = c->a;
	if(alpha > 0.0f)
	{
		a3d_mat4f_t mvp;
		float       w    = 0.0f;
		float       h    = 0.0f;
		float       x    = widget->rect_draw.l;
		float       y    = widget->rect_draw.t;
		float       size = widget->rect_draw.h;
		a3d_font_t* font = a3d_screen_font(widget->screen,
		                                   self->font_type);
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
		glUniform4f(font->unif_color, c->r, c->g, c->b, alpha);
		glUniform1i(font->unif_sampler, 0);
		glUniformMatrix4fv(font->unif_mvp, 1, GL_FALSE, (GLfloat*) &mvp);
		glDrawArrays(GL_TRIANGLES, 0, 2*3*len);
		glDisableVertexAttribArray(font->attr_coords);
		glDisableVertexAttribArray(font->attr_vertex);
		glUseProgram(0);
		glDisable(GL_BLEND);
	}
}

static void a3d_text_addc(a3d_text_t* self, char c,
                          int i, float* _offset)
{
	assert(self);

	float         offset = *_offset;
	a3d_widget_t* widget = (a3d_widget_t*) self;
	a3d_font_t*   font   = a3d_screen_font(widget->screen,
	                                       self->font_type);

	a3d_regionf_t tc;
	a3d_regionf_t vc;
	a3d_font_request(font, c,
	                 &tc, &vc);

	// compute vertex/coords for glDrawArrays
	// quad vertex/coords order
	// bl.xyz/uv, tl.xyz/uv, br.xyz/uv, tr.xyz/uv
	self->vertex[18*i +  0] = vc.l + offset;   // tl.xyz
	self->vertex[18*i +  1] = vc.t;
	self->vertex[18*i +  2] = 0.0f;
	self->vertex[18*i +  3] = vc.l + offset;   // bl.xyz
	self->vertex[18*i +  4] = vc.b;
	self->vertex[18*i +  5] = 0.0f;
	self->vertex[18*i +  6] = vc.r + offset;   // br.xyz
	self->vertex[18*i +  7] = vc.b;
	self->vertex[18*i +  8] = 0.0f;

	self->vertex[18*i +  9] = vc.l + offset;   // tl.xyz
	self->vertex[18*i + 10] = vc.t;
	self->vertex[18*i + 11] = 0.0f;
	self->vertex[18*i + 12] = vc.r + offset;   // br.xyz
	self->vertex[18*i + 13] = vc.b;
	self->vertex[18*i + 14] = 0.0f;
	self->vertex[18*i + 15] = vc.r + offset;   // tr.xyz
	self->vertex[18*i + 16] = vc.t;
	self->vertex[18*i + 17] = 0.0f;

	self->coords[12*i + 0] = tc.l;   // tl.uv
	self->coords[12*i + 1] = tc.t;
	self->coords[12*i + 2] = tc.l;   // bl.uv
	self->coords[12*i + 3] = tc.b;
	self->coords[12*i + 4] = tc.r;   // br.uv
	self->coords[12*i + 5] = tc.b;

	self->coords[12*i +  6] = tc.l;   // tl.uv
	self->coords[12*i +  7] = tc.t;
	self->coords[12*i +  8] = tc.r;   // br.uv
	self->coords[12*i +  9] = tc.b;
	self->coords[12*i + 10] = tc.r;   // tr.uv
	self->coords[12*i + 11] = tc.t;

	// next character offset
	*_offset += vc.r;
}

static int
a3d_text_keyPress(a3d_widget_t* widget, void* priv,
                  int keycode, int meta)
{
	assert(widget);

	a3d_text_t* self = (a3d_text_t*) widget;
	a3d_text_enterFn enter_fn = self->enter_fn;
	if(enter_fn == NULL)
	{
		LOGE("enter_fn is NULL");
		return 0;
	}

	int len = strlen(self->string);
	if(keycode == A3D_KEY_ENTER)
	{
		(*enter_fn)(priv, self->string);
	}
	else if(keycode == A3D_KEY_ESCAPE)
	{
		return 0;
	}
	else if(keycode == A3D_KEY_BACKSPACE)
	{
		if(len > 0)
		{
			self->string[len - 1] = '\0';
			len -= 1;
		}
		else
		{
			return 1;
		}
	}
	else
	{
		if(a3d_text_resize(self, len + 1))
		{
			self->string[len]     = (char) keycode;
			self->string[len + 1] = '\0';
			len += 1;
		}
		else
		{
			// accept the keypress but ignore error
			return 1;
		}
	}

	int   i;
	float offset = 0.0f;
	for(i = 0; i < len; ++i)
	{
		a3d_text_addc(self, self->string[i], i, &offset);
	}

	// add the cursor
	a3d_text_addc(self, A3D_FONT_CURSOR, len, &offset);
	++len;

	int vertex_size = 18*len;   // 2 * 3 * xyz
	int coords_size = 12*len;   // 2 * 3 * uv
	glBindBuffer(GL_ARRAY_BUFFER, self->id_vertex);
	glBufferData(GL_ARRAY_BUFFER, vertex_size*sizeof(GLfloat),
	             self->vertex, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, self->id_coords);
	glBufferData(GL_ARRAY_BUFFER, coords_size*sizeof(GLfloat),
	             self->coords, GL_STATIC_DRAW);

	a3d_screen_dirty(widget->screen);
	return 1;
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_text_t* a3d_text_new(a3d_screen_t* screen,
                         int wsize,
                         int border,
                         int text_size,
                         a3d_vec4f_t* color_fill,
                         a3d_vec4f_t* color_text,
                         void* priv,
                         a3d_text_enterFn enter_fn,
                         a3d_widget_clickFn click_fn,
                         a3d_widget_refreshFn refresh_fn)
{
	// priv, enter_fn, click_fn and refresh_fn may be NULL
	assert(screen);
	assert(color_fill);
	assert(color_text);

	if(wsize == 0)
	{
		wsize = sizeof(a3d_text_t);
	}

	a3d_widgetLayout_t layout =
	{
		.wrapx    = A3D_WIDGET_WRAP_SHRINK,
		.wrapy    = A3D_WIDGET_WRAP_SHRINK,
		.aspectx  = A3D_WIDGET_ASPECT_DEFAULT,
		.aspecty  = A3D_WIDGET_ASPECT_DEFAULT,
		.stretchx = 1.0f,
		.stretchy = 1.0f
	};

	a3d_widgetFn_t fn =
	{
		.priv        = priv,
		.size_fn     = a3d_text_size,
		.click_fn    = click_fn,
		.keyPress_fn = a3d_text_keyPress,
		.draw_fn     = a3d_text_draw,
		.refresh_fn  = refresh_fn
	};

	a3d_vec4f_t clear =
	{
		.r = 0.0f,
		.g = 0.0f,
		.b = 0.0f,
		.a = 0.0f
	};

	a3d_text_t* self;
	self = (a3d_text_t*)
	       a3d_widget_new(screen, wsize, &layout, border,
	                      &clear, color_fill, 0, &clear,
	                      &clear, &fn);
	if(self == NULL)
	{
		return NULL;
	}

	self->string_size = 16;
	self->string = calloc(self->string_size, sizeof(char));
	if(self->string == NULL)
	{
		LOGE("calloc failed");
		goto fail_string;
	}

	// allocate size for string and cursor character
	// which is stored in place of the null character
	int vertex_size = 18*self->string_size;   // 2*3*xyz
	self->vertex = (GLfloat*) malloc(sizeof(GLfloat)*vertex_size);
	if(self->vertex == NULL)
	{
		LOGE("malloc failed");
		goto fail_vertex;
	}

	int coords_size = 12*self->string_size;   // 2*3*uv
	self->coords = (GLfloat*) malloc(sizeof(GLfloat)*coords_size);
	if(self->coords == NULL)
	{
		LOGE("malloc failed");
		goto fail_coords;
	}

	self->enter_fn   = enter_fn;
	self->font_type  = A3D_SCREEN_FONT_REGULAR;
	self->text_size  = text_size;
	a3d_vec4f_copy(color_text, &self->color);
	glGenBuffers(1, &self->id_vertex);
	glGenBuffers(1, &self->id_coords);

	// initialize string and cursor
	a3d_text_printf(self, "%s", "");

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
		glDeleteBuffers(1, &self->id_vertex);
		glDeleteBuffers(1, &self->id_coords);

		free(self->coords);
		free(self->vertex);
		free(self->string);

		a3d_widget_delete((a3d_widget_t**) _self);
	}
}

int a3d_text_width(a3d_text_t* self, int cursor)
{
	assert(self);

	a3d_widget_t* widget = (a3d_widget_t*) self;
	a3d_font_t*   font   = a3d_screen_font(widget->screen,
	                                       self->font_type);

	int   width = 0;
	char* s     = self->string;
	while(s[0] != '\0')
	{
		width += a3d_font_width(font, s[0]);
		++s;
	}

	if(cursor)
	{
		width += a3d_font_width(font, A3D_FONT_CURSOR);
	}

	return width;
}

int a3d_text_height(a3d_text_t* self)
{
	assert(self);

	a3d_widget_t* widget = (a3d_widget_t*) self;
	a3d_font_t*   font   = a3d_screen_font(widget->screen,
	                                       self->font_type);
	return a3d_font_height(font);
}

void a3d_text_printf(a3d_text_t* self,
                     const char* fmt, ...)
{
	assert(self);
	assert(fmt);

	a3d_widget_t* widget = (a3d_widget_t*) self;

	// decode string
	char tmp_string[256];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(tmp_string, 256, fmt, argptr);
	va_end(argptr);

	// fill the string
	size_t len1        = strlen(tmp_string);
	size_t string_size = len1 + 1;
	if(a3d_text_resize(self, string_size) == 0)
	{
		return;
	}
	snprintf(self->string, string_size, "%s", tmp_string);

	int   i;
	float offset = 0.0f;
	for(i = 0; i < len1; ++i)
	{
		a3d_text_addc(self, self->string[i], i, &offset);
	}

	// add the cursor
	a3d_text_addc(self, A3D_FONT_CURSOR, len1, &offset);
	++len1;

	int vertex_size = 18*len1;   // 2 * 3 * xyz
	int coords_size = 12*len1;   // 2 * 3 * uv
	glBindBuffer(GL_ARRAY_BUFFER, self->id_vertex);
	glBufferData(GL_ARRAY_BUFFER, vertex_size*sizeof(GLfloat),
	             self->vertex, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, self->id_coords);
	glBufferData(GL_ARRAY_BUFFER, coords_size*sizeof(GLfloat),
	             self->coords, GL_STATIC_DRAW);

	a3d_screen_dirty(widget->screen);
}

void a3d_text_font(a3d_text_t* self, int font_type)
{
	assert(self);

	self->font_type = font_type;
}
