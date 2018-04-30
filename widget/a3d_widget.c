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
	"attribute vec2 vertex;\n"
	"uniform   mat4 mvp;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_Position = mvp*vec4(vertex, 0.0, 1.0);\n"
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

static const char* VSHADER2 =
	"attribute vec2  vertex;\n"
	"uniform   mat4  mvp;\n"
	"\n"
	"varying   float varying_y;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	varying_y = vertex.y;\n"
	"	gl_Position = mvp*vec4(vertex, 0.0, 1.0);\n"
	"}\n";

static const char* FSHADER2 =
	"#ifdef GL_ES\n"
	"precision mediump float;\n"
	"precision mediump int;\n"
	"#endif\n"
	"\n"
	"uniform vec4  colora;\n"
	"uniform vec4  colorb;\n"
	"uniform float y;\n"
	"\n"
	"varying float varying_y;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	if(varying_y > y)\n"
	"	{\n"
	"		gl_FragColor = colora;\n"
	"	}\n"
	"	else\n"
	"	{\n"
	"		gl_FragColor = colorb;\n"
	"	}\n"
	"}\n";

static const char* VSHADER_SCROLL =
	"attribute vec4  xyuv;\n"
	"uniform   mat4  mvp;\n"
	"\n"
	"varying   float varying_v;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	varying_v = xyuv.w;\n"
	"	gl_Position = mvp*vec4(xyuv.xy, 0.0, 1.0);\n"
	"}\n";

static const char* FSHADER_SCROLL =
	"#ifdef GL_ES\n"
	"precision mediump float;\n"
	"precision mediump int;\n"
	"#endif\n"
	"\n"
	"uniform vec4  color0;\n"
	"uniform vec4  color1;\n"
	"uniform float a;\n"
	"uniform float b;\n"
	"\n"
	"varying float varying_v;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	if((varying_v < a) ||\n"
	"	   (varying_v > b))\n"
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
	LOGD("debug");

	self->prog = a3d_shader_make_source(VSHADER, FSHADER);
	if(self->prog == 0)
	{
		return 0;
	}

	self->prog2 = a3d_shader_make_source(VSHADER2, FSHADER2);
	if(self->prog2 == 0)
	{
		goto fail_prog2;
	}

	self->scroll_prog = a3d_shader_make_source(VSHADER_SCROLL,
	                                           FSHADER_SCROLL);
	if(self->scroll_prog == 0)
	{
		goto fail_scroll_prog;
	}

	self->attr_vertex  = glGetAttribLocation(self->prog, "vertex");
	self->unif_mvp     = glGetUniformLocation(self->prog, "mvp");
	self->unif_color   = glGetUniformLocation(self->prog, "color");
	self->attr_vertex2 = glGetAttribLocation(self->prog2, "vertex");
	self->unif_mvp2    = glGetUniformLocation(self->prog2, "mvp");
	self->unif_color2a = glGetUniformLocation(self->prog2, "colora");
	self->unif_color2b = glGetUniformLocation(self->prog2, "colorb");
	self->unif_y2      = glGetUniformLocation(self->prog2, "y");
	self->scroll_attr_vertex = glGetAttribLocation(self->scroll_prog, "xyuv");
	self->scroll_unif_mvp    = glGetUniformLocation(self->scroll_prog, "mvp");
	self->scroll_unif_color0 = glGetUniformLocation(self->scroll_prog, "color0");
	self->scroll_unif_color1 = glGetUniformLocation(self->scroll_prog, "color1");
	self->scroll_unif_a      = glGetUniformLocation(self->scroll_prog, "a");
	self->scroll_unif_b      = glGetUniformLocation(self->scroll_prog, "b");

	// success
	return 1;

	// failure
	fail_scroll_prog:
		glDeleteProgram(self->prog2);
	fail_prog2:
		glDeleteProgram(self->prog);
	return 0;
}

static void a3d_widget_makeRoundRect(GLfloat* vtx, int steps,
                                     float t, float l,
                                     float b, float r,
                                     float radius)
{
	assert(vtx);

	// top-right
	int   i;
	int   idx = 0;
	float s   = (float) (steps - 1);
	for(i = 0; i < steps; ++i)
	{
		float ang = 0.0f + 90.0f*((float) i/s);
		vtx[idx++] = r + radius*cosf(ang*M_PI/180.0f);
		vtx[idx++] = t - radius*sinf(ang*M_PI/180.0f);
	}

	// top-left
	for(i = 0; i < steps; ++i)
	{
		float ang = 90.0f + 90.0f*((float) i/s);
		vtx[idx++] = l + radius*cosf(ang*M_PI/180.0f);
		vtx[idx++] = t - radius*sinf(ang*M_PI/180.0f);
	}

	// bottom-left
	for(i = 0; i < steps; ++i)
	{
		float ang = 180.0f + 90.0f*((float) i/s);
		vtx[idx++] = l + radius*cosf(ang*M_PI/180.0f);
		vtx[idx++] = b - radius*sinf(ang*M_PI/180.0f);
	}

	// bottom-right
	for(i = 0; i < steps; ++i)
	{
		float ang = 270.0f + 90.0f*((float) i/s);
		vtx[idx++] = r + radius*cosf(ang*M_PI/180.0f);
		vtx[idx++] = b - radius*sinf(ang*M_PI/180.0f);
	}
}

static void a3d_widget_makeRoundLines(GLfloat* vtx, int steps,
                                      float t, float l,
                                      float b, float r,
                                      float radius, float lw)
{
	assert(vtx);

	float outer = radius;
	float inner = radius - lw;

	// top-right
	int   i;
	int   idx = 0;
	float s   = (float) (steps - 1);
	for(i = 0; i < steps; ++i)
	{
		float ang = 0.0f + 90.0f*((float) i/s);
		vtx[idx++] = r + inner*cosf(ang*M_PI/180.0f);
		vtx[idx++] = t - inner*sinf(ang*M_PI/180.0f);
		vtx[idx++] = r + outer*cosf(ang*M_PI/180.0f);
		vtx[idx++] = t - outer*sinf(ang*M_PI/180.0f);
	}

	// top-left
	for(i = 0; i < steps; ++i)
	{
		float ang = 90.0f + 90.0f*((float) i/s);
		vtx[idx++] = l + inner*cosf(ang*M_PI/180.0f);
		vtx[idx++] = t - inner*sinf(ang*M_PI/180.0f);
		vtx[idx++] = l + outer*cosf(ang*M_PI/180.0f);
		vtx[idx++] = t - outer*sinf(ang*M_PI/180.0f);
	}

	// bottom-left
	for(i = 0; i < steps; ++i)
	{
		float ang = 180.0f + 90.0f*((float) i/s);
		vtx[idx++] = l + inner*cosf(ang*M_PI/180.0f);
		vtx[idx++] = b - inner*sinf(ang*M_PI/180.0f);
		vtx[idx++] = l + outer*cosf(ang*M_PI/180.0f);
		vtx[idx++] = b - outer*sinf(ang*M_PI/180.0f);
	}

	// bottom-right
	for(i = 0; i < steps; ++i)
	{
		float ang = 270.0f + 90.0f*((float) i/s);
		vtx[idx++] = r + inner*cosf(ang*M_PI/180.0f);
		vtx[idx++] = b - inner*sinf(ang*M_PI/180.0f);
		vtx[idx++] = r + outer*cosf(ang*M_PI/180.0f);
		vtx[idx++] = b - outer*sinf(ang*M_PI/180.0f);
	}

	// finish line loop
	vtx[idx++] = vtx[0];
	vtx[idx++] = vtx[1];
	vtx[idx++] = vtx[2];
	vtx[idx++] = vtx[3];
}

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
	self->scroll_bar     = 0;
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
	a3d_vec4f_load(&self->color_fill2,  0.0f, 0.0f, 0.0f, 0.0f);
	a3d_vec4f_load(&self->color_scroll0, 0.0f, 0.0f, 0.0f, 0.0f);
	a3d_vec4f_load(&self->color_scroll1, 0.0f, 0.0f, 0.0f, 0.0f);
	self->tone_y2    = 0.0f;

	glGenBuffers(1, &self->id_vtx_rect);
	glGenBuffers(1, &self->id_vtx_line);
	glGenBuffers(1, &self->scroll_id_vtx_rect);

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
		LOGD("debug");

		// TODO - screen top

		if(a3d_widget_hasFocus(self))
		{
			a3d_screen_focus(self->screen, NULL);
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &self->scroll_id_vtx_rect);
		glDeleteBuffers(1, &self->id_vtx_line);
		glDeleteBuffers(1, &self->id_vtx_rect);
		glDeleteProgram(self->scroll_prog);
		glDeleteProgram(self->prog2);
		glDeleteProgram(self->prog);
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

	// initialize rounded rectangle
	float b = self->rect_border.t + self->rect_border.h;
	float r = self->rect_border.l + self->rect_border.w;
	float radius = (h_bo == v_bo) ? h_bo : 0.0f;
	int steps    = A3D_WIDGET_BEZEL;
	int size_rect = 4*steps*2;   // corners*steps*xy
	GLfloat vtx_rect[size_rect];
	a3d_widget_makeRoundRect(vtx_rect, steps,
	                         t + v_bo, l + h_bo,
	                         b - v_bo, r - v_bo,
	                         radius);
	glBindBuffer(GL_ARRAY_BUFFER, self->id_vtx_rect);
	glBufferData(GL_ARRAY_BUFFER, size_rect*sizeof(GLfloat),
	             vtx_rect, GL_STATIC_DRAW);

	a3d_rect4f_t rect_border_clip;
	if(a3d_rect4f_intersect(&self->rect_border,
	                        &self->rect_clip,
	                        &rect_border_clip))
	{
		if(self->scroll_bar)
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
			int   sz = 16;   // 4*xyuv
			GLfloat xyuv[] =
			{
				l, t, 0.0f, 0.0f,   // top-left
				l, b, 0.0f, 1.0f,   // bottom-left
				r, t, 1.0f, 0.0f,   // top-right
				r, b, 1.0f, 1.0f,   // bottom-right
			};
			glBindBuffer(GL_ARRAY_BUFFER, self->scroll_id_vtx_rect);
			glBufferData(GL_ARRAY_BUFFER, sz*sizeof(GLfloat),
			             xyuv, GL_STATIC_DRAW);
		}
	}

	// initialize rounded line
	float lw = a3d_screen_layoutLine(self->screen, self->style_line);
	int size_line = 2*4*steps*2 + 2*2;   // edges*corners*steps*xy + edges*xy
	GLfloat vtx_line[size_line];
	a3d_widget_makeRoundLines(vtx_line, steps,
	                         t + v_bo, l + h_bo,
	                         b - v_bo, r - v_bo,
	                         radius, lw);
	glBindBuffer(GL_ARRAY_BUFFER, self->id_vtx_line);
	glBufferData(GL_ARRAY_BUFFER, size_line*sizeof(GLfloat),
	             vtx_line, GL_STATIC_DRAW);
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

	a3d_font_t* font = a3d_screen_font(self->screen,
	                                   A3D_SCREEN_FONT_REGULAR);
	float ar = a3d_font_aspectRatioAvg(font);
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
                     float dx, float dy)
{
	assert(self);
	LOGD("debug dx=%f, dy=%f", dx, dy);

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
		(*drag_fn)(self, x, y, dx, dy);
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

		a3d_mat4f_t mvp;
		glBindBuffer(GL_ARRAY_BUFFER, self->id_vtx_rect);
		a3d_mat4f_ortho(&mvp, 1, 0.0f, screen->w, screen->h, 0.0f, 0.0f, 2.0f);

		if(self->tone_y2 == 0.0f)
		{
			glEnableVertexAttribArray(self->attr_vertex);
			glVertexAttribPointer(self->attr_vertex, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glUseProgram(self->prog);
			glUniform4f(self->unif_color, c->r, c->g, c->b, alpha);
			glUniformMatrix4fv(self->unif_mvp, 1, GL_FALSE, (GLfloat*) &mvp);
		}
		else
		{
			a3d_vec4f_t* c2     = &self->color_fill2;
			float        alpha2 = self->fade*c2->a;
			glEnableVertexAttribArray(self->attr_vertex2);
			glVertexAttribPointer(self->attr_vertex2, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glUseProgram(self->prog2);
			glUniform4f(self->unif_color2a, c->r, c->g, c->b, alpha);
			glUniform4f(self->unif_color2b, c2->r, c2->g, c2->b, alpha2);
			glUniform1f(self->unif_y2, self->tone_y2);
			glUniformMatrix4fv(self->unif_mvp2, 1, GL_FALSE, (GLfloat*) &mvp);
		}
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4*A3D_WIDGET_BEZEL);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		if(self->tone_y2 == 0.0f)
		{
			glDisableVertexAttribArray(self->attr_vertex);
		}
		else
		{
			glDisableVertexAttribArray(self->attr_vertex2);
		}
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
		a3d_widget_draw_fn draw_fn = self->draw_fn;
		if(draw_fn)
		{
			a3d_screen_scissor(screen, &rect_draw_clip);
			(*draw_fn)(self);
		}

		// draw the scroll bar
		float s = rect_draw_clip.h/self->rect_draw.h;
		if(self->scroll_bar && (s < 1.0f))
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

			a3d_vec4f_t* c0 = &self->color_scroll0;
			a3d_vec4f_t* c1 = &self->color_scroll1;
			a3d_screen_scissor(screen, &rect_border_clip);
			if((c0->a < 1.0f) || (c1->a < 1.0f))
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}

			a3d_mat4f_t mvp;
			glBindBuffer(GL_ARRAY_BUFFER, self->scroll_id_vtx_rect);
			a3d_mat4f_ortho(&mvp, 1, 0.0f, screen->w, screen->h, 0.0f, 0.0f, 2.0f);

			glEnableVertexAttribArray(self->scroll_attr_vertex);
			glVertexAttribPointer(self->scroll_attr_vertex, 4, GL_FLOAT, GL_FALSE, 0, 0);
			glUseProgram(self->scroll_prog);
			glUniform4f(self->scroll_unif_color0, c0->r, c0->g, c0->b, c0->a);
			glUniform4f(self->scroll_unif_color1, c1->r, c1->g, c1->b, c1->a);
			glUniform1f(self->scroll_unif_a, a);
			glUniform1f(self->scroll_unif_b, b);
			glUniformMatrix4fv(self->scroll_unif_mvp, 1, GL_FALSE, (GLfloat*) &mvp);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDisableVertexAttribArray(self->scroll_attr_vertex);
			glUseProgram(0);
			if((c0->a < 1.0f) || (c1->a < 1.0f))
			{
				glDisable(GL_BLEND);
			}
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
		glUseProgram(self->prog);
		glEnableVertexAttribArray(self->attr_vertex);

		// draw rounded line
		glBindBuffer(GL_ARRAY_BUFFER, self->id_vtx_line);
		glVertexAttribPointer(self->attr_vertex, 2, GL_FLOAT, GL_FALSE, 0, 0);
		a3d_mat4f_t mvp;
		a3d_mat4f_ortho(&mvp, 1, 0.0f, screen->w, screen->h, 0.0f, 0.0f, 2.0f);
		glUniformMatrix4fv(self->unif_mvp, 1, GL_FALSE, (GLfloat*) &mvp);
		glUniform4f(self->unif_color, c->r, c->g, c->b, alpha);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 2*(4*A3D_WIDGET_BEZEL + 1));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableVertexAttribArray(self->attr_vertex);
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

	// disable fade by default
	#if 1
		self->fade = fade;
	#else
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
	#endif

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

void a3d_widget_twoTone(a3d_widget_t* self,
                        a3d_vec4f_t* color_fill2)
{
	assert(self);
	assert(color_fill2);

	a3d_vec4f_copy(color_fill2, &self->color_fill2);
}

void a3d_widget_twoToneY(a3d_widget_t* self, float y)
{
	assert(self);

	self->tone_y2 = y;
}

void a3d_widget_scrollbar(a3d_widget_t* self,
                          a3d_vec4f_t* color_scroll0,
                          a3d_vec4f_t* color_scroll1)
{
	assert(self);
	assert(color_scroll0);
	assert(color_scroll1);

	self->scroll_bar = 1;
	a3d_vec4f_copy(color_scroll0, &self->color_scroll0);
	a3d_vec4f_copy(color_scroll1, &self->color_scroll1);
}
