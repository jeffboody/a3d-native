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

#include "a3d_sprite.h"
#include "a3d_screen.h"
#include "../a3d_shader.h"
#include "../math/a3d_regionf.h"
#include "../../texgz/texgz_tex.h"
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static const GLfloat VERTEX[] =
{
	0.0f, 0.0f, 0.0f, 1.0f,   // top-left
	0.0f, 1.0f, 0.0f, 1.0f,   // bottom-left
	1.0f, 1.0f, 0.0f, 1.0f,   // bottom-right
	1.0f, 1.0f, 0.0f, 1.0f,   // bottom-right
	1.0f, 0.0f, 0.0f, 1.0f,   // top-right
	0.0f, 0.0f, 0.0f, 1.0f,   // top-left
};

static const GLfloat COORDS[] =
{
	0.0f, 0.0f,   // top-left
	0.0f, 1.0f,   // bottom-left
	1.0f, 1.0f,   // bottom-right
	1.0f, 1.0f,   // bottom-right
	1.0f, 0.0f,   // top-right
	0.0f, 0.0f,   // top-left
};

static const char* VSHADER =
	"attribute vec4 vertex;\n"
	"attribute vec2 coords;\n"
	"uniform   mat4 mvp;\n"
	"varying   vec2 varying_coords;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	varying_coords = coords;\n"
	"	gl_Position = mvp*vertex;\n"
	"}\n";

static const char* FSHADER =
	"#ifdef GL_ES\n"
	"precision mediump float;\n"
	"precision mediump int;\n"
	"#endif\n"
	"\n"
	"uniform sampler2D sampler;\n"
	"varying vec2      varying_coords;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = texture2D(sampler, varying_coords);\n"
	"}\n";

static int a3d_sprite_shaders(a3d_sprite_t* self)
{
	assert(self);
	LOGD("debug");

	self->prog = a3d_shader_make_source(VSHADER, FSHADER);
	if(self->prog == 0)
	{
		return 0;
	}

	self->attr_vertex  = glGetAttribLocation(self->prog, "vertex");
	self->attr_coords  = glGetAttribLocation(self->prog, "coords");
	self->unif_mvp     = glGetUniformLocation(self->prog, "mvp");
	self->unif_sampler = glGetUniformLocation(self->prog, "sampler");

	return 1;
}

static int a3d_sprite_tex(a3d_sprite_t* self, const char* fname)
{
	assert(self);
	assert(fname);
	LOGD("debug fname=%s", fname);

	texgz_tex_t* tex = texgz_tex_import(fname);
	if(tex == NULL)
	{
		return 0;
	}

	// load tex
	glGenTextures(1, &self->id_tex);
	glBindTexture(GL_TEXTURE_2D, self->id_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, tex->format,
	             tex->stride, tex->vstride,
	             0, tex->format, tex->type,
	             tex->pixels);

	// no longer needed
	texgz_tex_delete(&tex);

	return 1;
}

static void a3d_sprite_draw(a3d_widget_t* widget)
{
	assert(widget);
	LOGD("debug");

	a3d_sprite_t* self = (a3d_sprite_t*) widget;
	a3d_mat4f_t   mvp;

	float w  = 0.0f;
	float h  = 0.0f;
	float x  = widget->rect_draw.l;
	float y  = widget->rect_draw.t;
	float ww = widget->rect_draw.w;
	float hh = widget->rect_draw.h;
	a3d_screen_sizef(widget->screen, &w, &h);
	a3d_mat4f_ortho(&mvp, 1, 0.0f, w, h, 0.0f, 0.0f, 2.0f);
	a3d_mat4f_translate(&mvp, 0, x, y, -1.0f);
	a3d_mat4f_scale(&mvp, 0, ww, hh, 1.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(self->prog);
	glEnableVertexAttribArray(self->attr_vertex);
	glEnableVertexAttribArray(self->attr_coords);

	// draw sprite
	glBindTexture(GL_TEXTURE_2D, self->id_tex);
	glBindBuffer(GL_ARRAY_BUFFER, self->id_vertex);
	glVertexAttribPointer(self->attr_vertex, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, self->id_coords);
	glVertexAttribPointer(self->attr_coords, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glUniformMatrix4fv(self->unif_mvp, 1, GL_FALSE, (GLfloat*) &mvp);
	glUniform1i(self->unif_sampler, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(self->attr_coords);
	glDisableVertexAttribArray(self->attr_vertex);
	glUseProgram(0);
	glDisable(GL_BLEND);
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_sprite_t* a3d_sprite_new(a3d_screen_t* screen,
                             int wsize,
                             int anchor,
                             int wrapx, int wrapy,
                             int stretch_mode,
                             float stretch_factor,
                             int style_border,
                             int style_line,
                             a3d_vec4f_t* color_fill,
                             a3d_vec4f_t* color_line,
                             a3d_widget_click_fn click_fn,
                             a3d_widget_refresh_fn refresh_fn,
                             const char* fname)
{
	// click_fn and refresh_fn may be NULL
	assert(screen);
	assert(color_fill);
	assert(color_line);
	LOGD("debug wsize=%i, anchor=%i, wrapx=%i, wrapy=%i",
	     wsize, anchor, wrapx, wrapy);
	LOGD("debug stretch_mode=%i, stretch_factor=%f, style_border=%i, style_line=%i",
	     stretch_mode, stretch_factor, style_border, style_line);
	LOGD("debug color_line: r=%f, g=%f, b=%f, a=%f",
	     color_line->r, color_line->g, color_line->b, color_line->a);
	LOGD("debug color_fill: r=%f, g=%f, b=%f, a=%f",
	     color_fill->r, color_fill->g, color_fill->b, color_fill->a);
	LOGD("debug fname=%s", fname);

	if((wrapx == A3D_WIDGET_WRAP_SHRINK) ||
	   (wrapy == A3D_WIDGET_WRAP_SHRINK))
	{
		LOGE("invalid wrapx=%i, wrapy=%i", wrapx, wrapy);
		return NULL;
	}

	if(wsize == 0)
	{
		wsize = sizeof(a3d_sprite_t);
	}

	a3d_sprite_t* self = (a3d_sprite_t*) a3d_widget_new(screen,
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
	                                                    click_fn,
	                                                    NULL,
	                                                    NULL,
	                                                    a3d_sprite_draw,
	                                                    refresh_fn);
	if(self == NULL)
	{
		return NULL;
	}

	if(a3d_sprite_shaders(self) == 0)
	{
		goto fail_shaders;
	}

	if(a3d_sprite_tex(self, fname) == 0)
	{
		goto fail_tex;
	}

	glGenBuffers(1, &self->id_vertex);
	glGenBuffers(1, &self->id_coords);

	int vertex_size = 24;   // 2*3*xyzw
	int coords_size = 12;   // 2*3*uv
	glBindBuffer(GL_ARRAY_BUFFER, self->id_vertex);
	glBufferData(GL_ARRAY_BUFFER, vertex_size*sizeof(GLfloat), VERTEX, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, self->id_coords);
	glBufferData(GL_ARRAY_BUFFER, coords_size*sizeof(GLfloat), COORDS, GL_STATIC_DRAW);

	// success
	return self;

	// failure
	fail_tex:
		glDeleteProgram(self->prog);
	fail_shaders:
		free(self);
	return NULL;
}

void a3d_sprite_delete(a3d_sprite_t** _self)
{
	assert(_self);

	a3d_sprite_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &self->id_tex);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &self->id_coords);
		glDeleteBuffers(1, &self->id_vertex);
		glDeleteProgram(self->prog);

		a3d_widget_delete((a3d_widget_t**) _self);
	}
}
