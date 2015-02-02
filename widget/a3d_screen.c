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
#include "a3d_text.h"
#include "a3d_widget.h"
#include "../a3d_shader.h"
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static const GLfloat COORDS[] =
{
	0.0f, 0.0f,   // top-left
	0.0f, 1.0f,   // bottom-left
	1.0f, 1.0f,   // bottom-right
	1.0f, 0.0f,   // top-right
};

static const char* VSHADER =
	"attribute vec2 coords;\n"
	"uniform   mat4 mvp;\n"
	"uniform   vec4 rect;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	float t = rect.x;\n"
	"	float l = rect.y;\n"
	"	float w = rect.z;\n"
	"	float h = rect.w;\n"
	"	vec4  vertex = vec4(l + coords.x*w,\n"
	"	                    t + coords.y*h,\n"
	"	                    0.0, 1.0);\n"
	"	gl_Position = mvp*vertex;\n"
	"}\n";

static const char* FSHADER =
	"#ifdef GL_ES\n"
	"precision mediump float;\n"
	"precision mediump int;\n"
	"#endif\n"
	"\n"
	"uniform vec4 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = color;\n"
	"}\n";

static int a3d_screen_shaders(a3d_screen_t* self)
{
	assert(self);
	LOGD("debug");

	self->prog = a3d_shader_make_source(VSHADER, FSHADER);
	if(self->prog == 0)
	{
		return 0;
	}

	self->attr_coords = glGetAttribLocation(self->prog, "coords");
	self->unif_mvp    = glGetUniformLocation(self->prog, "mvp");
	self->unif_rect   = glGetUniformLocation(self->prog, "rect");
	self->unif_color  = glGetUniformLocation(self->prog, "color");

	return 1;
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_screen_t* a3d_screen_new(a3d_font_t* font)
{
	assert(font);
	LOGD("debug");

	a3d_screen_t* self = (a3d_screen_t*) malloc(sizeof(a3d_screen_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	if(a3d_screen_shaders(self) == 0)
	{
		goto fail_shaders;
	}

	self->w             = 0;
	self->h             = 0;
	self->scale         = A3D_SCREEN_SCALE_MEDIUM;
	self->top_widget    = NULL;
	self->dirty         = 1;
	self->pointer_state = A3D_SCREEN_POINTER_UP;
	self->pointer_drag  = 0;
	self->pointer_x0    = 0.0f;
	self->pointer_y0    = 0.0f;
	self->pointer_t0    = 0.0;
	self->font          = font;

	int coords_size = 8;   // 4*uv
	glGenBuffers(1, &self->id_coords);
	glBindBuffer(GL_ARRAY_BUFFER, self->id_coords);
	glBufferData(GL_ARRAY_BUFFER, coords_size*sizeof(GLfloat),
	             COORDS, GL_STATIC_DRAW);

	// success
	return self;

	// failure
	fail_shaders:
		free(self);
	return NULL;
}

void a3d_screen_delete(a3d_screen_t** _self)
{
	assert(_self);

	a3d_screen_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &self->id_coords);
		glDeleteProgram(self->prog);
		free(self);
		*_self = NULL;
	}
}

void a3d_screen_top(a3d_screen_t* self, a3d_widget_t* top)
{
	assert(self);
	LOGD("debug");

	self->top_widget = top;
	self->dirty      = 1;
}

a3d_font_t* a3d_screen_font(a3d_screen_t* self)
{
	assert(self);
	LOGD("debug");

	return self->font;
}

void a3d_screen_resize(a3d_screen_t* self, int w, int h)
{
	assert(self);
	LOGD("debug w=%i, h=%i");

	self->w     = w;
	self->h     = h;
	self->dirty = 1;
}

void a3d_screen_sizei(a3d_screen_t* self, int* w, int* h)
{
	assert(self);
	LOGD("debug");

	*w = self->w;
	*h = self->h;
}

void a3d_screen_sizef(a3d_screen_t* self, float* w, float* h)
{
	assert(self);
	LOGD("debug");

	*w = (float) self->w;
	*h = (float) self->h;
}

void a3d_screen_rescale(a3d_screen_t* self, int scale)
{
	assert(self);
	LOGD("debug scale=%i", scale);

	if((scale >= A3D_SCREEN_SCALE_XSMALL) &&
	   (scale <= A3D_SCREEN_SCALE_XLARGE))
	{
		self->scale = scale;
		self->dirty = 1;
	}
}

int a3d_screen_scalei(a3d_screen_t* self)
{
	assert(self);
	LOGD("debug");

	return self->scale;
}

float a3d_screen_scalef(a3d_screen_t* self)
{
	assert(self);
	LOGD("debug");

	if(self->scale == A3D_SCREEN_SCALE_XSMALL)
	{
		return 0.64f;
	}
	else if(self->scale == A3D_SCREEN_SCALE_SMALL)
	{
		return 0.8f;
	}
	else if(self->scale == A3D_SCREEN_SCALE_LARGE)
	{
		return 1.25f;
	}
	else if(self->scale == A3D_SCREEN_SCALE_XLARGE)
	{
		return 1.56f;
	}
	return 1.0f;
}

void a3d_screen_dirty(a3d_screen_t* self)
{
	assert(self);
	LOGD("debug");

	self->dirty = 1;
}

float a3d_screen_layoutBorder(a3d_screen_t* self, int style)
{
	assert(self);
	LOGD("debug style=%i", style);

	float size = a3d_screen_layoutText(self, A3D_TEXT_STYLE_MEDIUM);
	if(style == A3D_WIDGET_BORDER_SMALL)
	{
		return 0.125f*size;
	}
	else if(style == A3D_WIDGET_BORDER_MEDIUM)
	{
		return 0.25f*size;
	}
	else if(style == A3D_WIDGET_BORDER_LARGE)
	{
		return 0.5f*size;
	}
	return 0.0f;
}

float a3d_screen_layoutLine(a3d_screen_t* self, int style)
{
	assert(self);
	LOGD("debug style=%i", style);

	// line style requires a mask
	if(style == A3D_WIDGET_LINE_SMALL)
	{
		return 1.0f;
	}
	else if(style == A3D_WIDGET_LINE_LARGE)
	{
		return 5.0f;
	}
	return 3.0f;
}

float a3d_screen_layoutText(a3d_screen_t* self, int style)
{
	assert(self);
	LOGD("debug style=%i", style);

	// default font size is 5% of narrow screen dimension
	float dim = (float) ((self->w < self->h) ? self->w : self->h);
	float size = 0.05f*dim*a3d_screen_scalef(self);
	if(style == A3D_TEXT_STYLE_SMALL)
	{
		return 0.66f*size;
	}
	else if(style == A3D_TEXT_STYLE_LARGE)
	{
		return 1.5f*size;
	}
	return size;
}

void a3d_screen_pointerDown(a3d_screen_t* self,
                            float x, float y, double t0)
{
	assert(self);
	LOGD("debug x=%f, y=%f, t0=%lf", x, y, t0);

	self->pointer_state = A3D_SCREEN_POINTER_DOWN;
	self->pointer_x0    = x;
	self->pointer_y0    = y;
	self->pointer_t0    = t0;
}

int a3d_screen_pointerUp(a3d_screen_t* self,
                         float x, float y, double t0)
{
	assert(self);
	LOGD("debug x=%f, y=%f, t0=%lf", x, y, t0);

	if(self->pointer_state == A3D_SCREEN_POINTER_MOVE)
	{
		// ignore
		return self->pointer_drag;
	}

	if(self->top_widget)
	{
		return a3d_widget_click(self->top_widget, x, y);
	}

	return 0;
}

void a3d_screen_pointerMove(a3d_screen_t* self,
                            float x, float y, double t0)
{
	assert(self);
	LOGD("debug x=%f, y=%f, t0=%lf", x, y, t0);

	if(self->pointer_state == A3D_SCREEN_POINTER_UP)
	{
		// ignore
		return;
	}

	// reject small motions (~1% of min screen dim)
	float  dx = x - self->pointer_x0;
	float  dy = y - self->pointer_y0;
	double dt = t0 - self->pointer_t0;
	if(self->pointer_state == A3D_SCREEN_POINTER_DOWN)
	{
		float d = sqrtf(dx*dx + dy*dy);
		float s = 0.2f*a3d_screen_layoutText(self, A3D_TEXT_STYLE_MEDIUM);
		if(d < s)
		{
			// ignore
			return;
		}

		// avoid sharp acceleration
		self->pointer_t0    = t0;
		self->pointer_drag  = 0;
		self->pointer_state = A3D_SCREEN_POINTER_MOVE;
		return;
	}

	self->pointer_x0 = x;
	self->pointer_y0 = y;
	self->pointer_t0 = t0;

	if(self->top_widget)
	{
		self->pointer_drag |= a3d_widget_drag(self->top_widget,
		                                      x, y, dx, dy, dt);
	}
}

void a3d_screen_scissor(a3d_screen_t* self, a3d_rect4f_t* rect)
{
	assert(self);
	assert(rect);
	LOGD("debug");

	glScissor((GLint) (rect->l + 0.5f),
	          self->h - (GLint) (rect->t + rect->h + 0.5f),
	          (GLsizei) (rect->w + 0.5f),
	          (GLsizei) (rect->h + 0.5f));
}

void a3d_screen_draw(a3d_screen_t* self)
{
	assert(self);
	LOGD("debug");

	a3d_widget_t* top = self->top_widget;
	if(top == NULL)
	{
		// ignore
		return;
	}

	a3d_widget_refresh(top);
	if(self->dirty)
	{
		float        w    = (float) self->w;
		float        h    = (float) self->h;
		a3d_rect4f_t clip = { .t = 0.0f, .l = 0.0f, .w = w, .h = h };
		a3d_widget_layoutSize(top, &w, &h);
		a3d_widget_layoutXYClip(top, 0.0f, 0.0f, &clip);
		self->dirty = 0;
	}
	glEnable(GL_SCISSOR_TEST);
	a3d_widget_draw(self->top_widget);

	// restore the scissor
	glScissor((GLint) 0,
	          (GLint) 0,
	          (GLsizei) self->w,
	          (GLsizei) self->h);
	glDisable(GL_SCISSOR_TEST);
}
