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

#include <stdlib.h>
#include <assert.h>
#include "a3d_shader.h"
#include "a3d_polygonShader.h"

#define LOG_TAG "a3d"
#include "a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static const char* VSHADER =
	"attribute vec2 vtx;\n"
	"uniform   mat4 mvp;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_Position = mvp*vec4(vtx, 0.0, 1.0);\n"
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

/***********************************************************
* public                                                   *
***********************************************************/

a3d_polygonShader_t* a3d_polygonShader_new(void)
{
	a3d_polygonShader_t* self = (a3d_polygonShader_t*)
	                            malloc(sizeof(a3d_polygonShader_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->prog = a3d_shader_make_source(VSHADER, FSHADER);
	if(self->prog == 0)
	{
	        goto fail_prog;
	}
	self->attr_vtx   = glGetAttribLocation(self->prog, "vtx");
	self->unif_mvp   = glGetUniformLocation(self->prog, "mvp");
	self->unif_color = glGetUniformLocation(self->prog, "color");

	self->blend = 0;

	// success
	return self;

	// failure
	fail_prog:
		free(self);
	return NULL;
}

void a3d_polygonShader_delete(a3d_polygonShader_t** _self)
{
	// *_self can be null
	assert(_self);

	a3d_polygonShader_t* self = *_self;
	if(self)
	{
		glUseProgram(0);
		glDeleteProgram(self->prog);
		free(self);
		*_self = NULL;
	}
}

void a3d_polygonShader_begin(a3d_polygonShader_t* self)
{
	glEnableVertexAttribArray(self->attr_vtx);
	glUseProgram(self->prog);
}

void a3d_polygonShader_end(a3d_polygonShader_t* self)
{
	a3d_polygonShader_blend(self, 0);
	glDisableVertexAttribArray(self->attr_vtx);
	glUseProgram(0);
}

void a3d_polygonShader_blend(a3d_polygonShader_t* self,
                             int blend)
{
	assert(self);

	if(self->blend == blend)
	{
		return;
	}

	if(blend)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	self->blend = blend;
}
