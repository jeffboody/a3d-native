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
#include "../../libpak/pak_file.h"
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
	-0.5f, -0.5f, 0.0f, 1.0f,   // top-left
	-0.5f,  0.5f, 0.0f, 1.0f,   // bottom-left
	 0.5f,  0.5f, 0.0f, 1.0f,   // bottom-right
	 0.5f,  0.5f, 0.0f, 1.0f,   // bottom-right
	 0.5f, -0.5f, 0.0f, 1.0f,   // top-right
	-0.5f, -0.5f, 0.0f, 1.0f,   // top-left
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

static const char* FSHADER_ALPHA =
	"#ifdef GL_ES\n"
	"precision mediump float;\n"
	"precision mediump int;\n"
	"#endif\n"
	"\n"
	"uniform sampler2D sampler;\n"
	"uniform vec4      color;\n"
	"varying vec2      varying_coords;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	float a = texture2D(sampler, varying_coords).a;\n"
	"	gl_FragColor = vec4(color.rgb, a*color.a);\n"
	"}\n";

static const char* FSHADER =
	"#ifdef GL_ES\n"
	"precision mediump float;\n"
	"precision mediump int;\n"
	"#endif\n"
	"\n"
	"uniform sampler2D sampler;\n"
	"uniform vec4      color;\n"
	"varying vec2      varying_coords;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = color*texture2D(sampler,\n"
	"	                               varying_coords);\n"
	"}\n";

static void a3d_sprite_draw(a3d_widget_t* widget)
{
	assert(widget);

	a3d_sprite_t* self   = (a3d_sprite_t*) widget;
	a3d_screen_t* screen = widget->screen;
	a3d_mat4f_t   mvp;

	// draw sprite
	a3d_vec4f_t* c     = &self->color;
	float        alpha = c->a;
	if(alpha > 0.0f)
	{
		float w  = 0.0f;
		float h  = 0.0f;
		float x  = widget->rect_draw.l;
		float y  = widget->rect_draw.t;
		float ww = widget->rect_draw.w;
		float hh = widget->rect_draw.h;
		a3d_screen_sizef(screen, &w, &h);
		a3d_mat4f_ortho(&mvp, 1, 0.0f, w, h, 0.0f, 0.0f, 2.0f);
		a3d_mat4f_translate(&mvp, 0, x + ww/2.0f, y + hh/2.0f, -1.0f);
		a3d_mat4f_scale(&mvp, 0, ww, hh, 1.0f);
		a3d_mat4f_rotate(&mvp, 0, self->theta, 0.0f, 0.0f, 1.0f);

		int index  = self->index;
		int format = self->format[index];
		a3d_spriteShader_t* shader = a3d_screen_spriteShader(screen, format);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUseProgram(shader->prog);
		glEnableVertexAttribArray(shader->attr_vertex);
		glEnableVertexAttribArray(shader->attr_coords);
		glBindTexture(GL_TEXTURE_2D, self->id_tex[index]);
		glBindBuffer(GL_ARRAY_BUFFER, self->id_vertex);
		glVertexAttribPointer(shader->attr_vertex, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, self->id_coords);
		glVertexAttribPointer(shader->attr_coords, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glUniform4f(shader->unif_color, c->r, c->g, c->b, alpha);
		glUniformMatrix4fv(shader->unif_mvp, 1, GL_FALSE, (GLfloat*) &mvp);
		glUniform1i(shader->unif_sampler, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableVertexAttribArray(shader->attr_coords);
		glDisableVertexAttribArray(shader->attr_vertex);
		glUseProgram(0);
		glDisable(GL_BLEND);
	}
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_spriteShader_t* a3d_spriteShader_new(int format)
{
	a3d_spriteShader_t* self = (a3d_spriteShader_t*) malloc(sizeof(a3d_spriteShader_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return 0;
	}

	if(format == A3D_SPRITESHADER_ALPHA)
	{
		self->prog = a3d_shader_make_source(VSHADER, FSHADER_ALPHA);
	}
	else
	{
		self->prog = a3d_shader_make_source(VSHADER, FSHADER);
	}

	if(self->prog == 0)
	{
		goto fail_prog;
	}

	self->attr_vertex  = glGetAttribLocation(self->prog, "vertex");
	self->attr_coords  = glGetAttribLocation(self->prog, "coords");
	self->unif_color   = glGetUniformLocation(self->prog, "color");
	self->unif_mvp     = glGetUniformLocation(self->prog, "mvp");
	self->unif_sampler = glGetUniformLocation(self->prog, "sampler");

	// success
	return self;

	// failure
	fail_prog:
		free(self);
	return NULL;
}

void a3d_spriteShader_delete(a3d_spriteShader_t** _self)
{
	a3d_spriteShader_t* self = *_self;
	if(self)
	{
		glDeleteProgram(self->prog);
		free(self);
		*_self = NULL;
	}
}

a3d_spriteTex_t* a3d_spriteTex_new(const char* fname,
                                   const char* resource)
{
	assert(fname);
	assert(resource);

	a3d_spriteTex_t* self = (a3d_spriteTex_t*) malloc(sizeof(a3d_spriteTex_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	pak_file_t*  pak = NULL;
	texgz_tex_t* tex = NULL;
	if(fname[0] == '$')
	{
		pak = pak_file_open(resource, PAK_FLAG_READ);
		if(pak)
		{
			const char* key = &(fname[1]);
			int size = pak_file_seek(pak, key);
			if(size > 0)
			{
				tex = texgz_tex_importf(pak->f, size);
			}
			else
			{
				LOGE("invalid fname=%s", fname);
			}
			pak_file_close(&pak);
		}
	}
	else
	{
		tex = texgz_tex_import(fname);
	}

	if(tex == NULL)
	{
		goto fail_tex;
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
	if(tex->format == TEXGZ_ALPHA)
	{
		self->format = A3D_SPRITESHADER_ALPHA;
	}
	else
	{
		self->format = A3D_SPRITESHADER_COLOR;
	}

	// no longer needed
	texgz_tex_delete(&tex);

	strncpy(self->fname, fname, 256);
	self->fname[255] = '\0';
	self->ref_count  = 0;

	// success
	return self;

	// failure
	fail_tex:
		free(self);
	return NULL;
}

void a3d_spriteTex_delete(a3d_spriteTex_t** _self)
{
	assert(_self);

	a3d_spriteTex_t* self = *_self;
	if(self)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &self->id_tex);

		free(self);
		*_self = NULL;
	}
}

void a3d_spriteTex_incRef(a3d_spriteTex_t* self)
{
	assert(self);

	++self->ref_count;
}

int a3d_spriteTex_decRef(a3d_spriteTex_t* self)
{
	assert(self);

	--self->ref_count;
	return self->ref_count;
}

a3d_sprite_t* a3d_sprite_new(a3d_screen_t* screen,
                             int wsize,
                             int border,
                             a3d_widgetLayout_t* layout,
                             a3d_vec4f_t* color,
                             int sprite_count,
                             void* priv,
                             a3d_widget_clickFn click_fn,
                             a3d_widget_refreshFn refresh_fn)
{
	// priv, click_fn and refresh_fn may be NULL
	assert(screen);
	assert(layout);
	assert(color);

	if(sprite_count <= 0)
	{
		LOGE("invalid sprite_count=%i", sprite_count);
		return NULL;
	}

	if((layout->wrapx == A3D_WIDGET_WRAP_SHRINK) ||
	   (layout->wrapy == A3D_WIDGET_WRAP_SHRINK))
	{
		LOGE("invalid wrapx=%i, wrapy=%i", layout->wrapx, layout->wrapy);
		return NULL;
	}

	if(wsize == 0)
	{
		wsize = sizeof(a3d_sprite_t);
	}

	a3d_widgetStyle_t style;
	memset(&style, 0, sizeof(a3d_widgetStyle_t));

	a3d_widgetFn_t fn =
	{
		.priv       = priv,
		.click_fn   = click_fn,
		.draw_fn    = a3d_sprite_draw,
		.refresh_fn = refresh_fn
	};

	// TODO - sprite layout
	layout->border     = border;
	layout->scroll_bar = 0;

	a3d_sprite_t* self;
	self = (a3d_sprite_t*)
	       a3d_widget_new(screen, wsize, layout, &style, &fn);
	if(self == NULL)
	{
		return NULL;
	}

	self->format = (int*) calloc(sprite_count, sizeof(int));
	if(self->format == NULL)
	{
		LOGE("malloc failed");
		goto fail_format;
	}

	self->id_tex = (GLuint*) calloc(sprite_count, sizeof(GLuint));
	if(self->id_tex == NULL)
	{
		LOGE("malloc failed");
		goto fail_tex;
	}

	self->sprite_count = sprite_count;
	self->index        = 0;
	self->theta        = 0.0f;
	a3d_vec4f_copy(color, &self->color);

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
		free(self->format);
	fail_format:
		a3d_widget_delete((a3d_widget_t**) &self);
	return NULL;
}

void a3d_sprite_delete(a3d_sprite_t** _self)
{
	assert(_self);

	a3d_sprite_t* self = *_self;
	if(self)
	{
		int i;
		a3d_widget_t* widget = (a3d_widget_t*) self;
		a3d_screen_t* screen = widget->screen;
		for(i = 0; i < self->sprite_count; ++i)
		{
			a3d_screen_spriteTexUnmap(screen, &(self->id_tex[i]));
		}
		free(self->id_tex);
		free(self->format);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &self->id_coords);
		glDeleteBuffers(1, &self->id_vertex);

		a3d_widget_delete((a3d_widget_t**) _self);
	}
}

int a3d_sprite_load(a3d_sprite_t* self, int index, const char* fname)
{
	assert(self);
	assert(fname);

	// check for invalid index
	if((index < 0) || (index >= self->sprite_count))
	{
		LOGW("invalid index=%i, sprite_count=%i", index,
		     self->sprite_count);
		return 0;
	}

	// unmap existing tex
	a3d_widget_t* widget = (a3d_widget_t*) self;
	a3d_screen_t* screen = widget->screen;
	if(self->id_tex[index])
	{
		a3d_screen_spriteTexUnmap(screen, &(self->id_tex[index]));
	}

	// map the new tex
	self->id_tex[index] = a3d_screen_spriteTexMap(screen, fname,
	                                              &self->format[index]);
	return self->id_tex[index] > 0;
}

void a3d_sprite_select(a3d_sprite_t* self, int index)
{
	assert(self);

	// check for invalid index
	if((index < 0) || (index >= self->sprite_count))
	{
		LOGW("invalid index=%i, sprite_count=%i", index,
		     self->sprite_count);
		return;
	}

	self->index = index;
}

void a3d_sprite_rotate(a3d_sprite_t* self,
                       float theta)
{
	assert(self);

	self->theta = theta;
}
