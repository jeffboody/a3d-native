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
#include "a3d_lineShader.h"

#define LOG_TAG "a3d"
#include "a3d_log.h"

/***********************************************************
* protected                                                *
***********************************************************/

void a3d_lineShader_blend(a3d_lineShader_t* self,
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

/***********************************************************
* private                                                  *
***********************************************************/

static const char* VSHADER =
	"#ifndef GL_ES\n"
	"#define lowp\n"
	"#define mediump\n"
	"#define highp\n"
	"#endif\n"
	"\n"
	"attribute vec2 vtx;\n"
	"attribute highp vec2 st;\n"
	"uniform   mat4 mvp;\n"
	"varying   highp vec2 varying_st;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	varying_st  = st;\n"
	"	gl_Position = mvp*vec4(vtx, 0.0, 1.0);\n"
	"}\n";

static const char* FSHADER =
	"#ifdef GL_ES\n"
	"precision mediump float;\n"
	"precision mediump int;\n"
	"#else\n"
	"#define lowp\n"
	"#define mediump\n"
	"#define highp\n"
	"#endif\n"
	"\n"
	"uniform float width;\n"
	"uniform float length;\n"
	"uniform bool  rounded;\n"
	"uniform float brush1;\n"
	"uniform float brush2;\n"
	"uniform float stripe1;\n"
	"uniform float stripe2;\n"
	"uniform vec4  color1;\n"
	"uniform vec4  color2;\n"
	"varying highp vec2 varying_st;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	vec4  color = color1;\n"
	"	highp float s     = abs(varying_st.x);\n"
	"	highp float ds    = brush1 + brush2;\n"
	"	highp float phase = mod(s, ds*width);\n"
	"	if(phase > brush1*width)\n"
	"	{\n"
	"		color = color2;\n"
	"	}\n"
	"	\n"
	"	float t  = abs(varying_st.y);\n"
	"	if(rounded)\n"
	"	{\n"
	"		float w2 = width/2.0;\n"
	"		if(s < w2)\n"
	"		{\n"
	"			float xx = (w2 - s)/w2;\n"
	"			float yy = t;\n"
	"			if((xx*xx + yy*yy) > 1.0)\n"
	"			{\n"
	"				discard;\n"
	"			}\n"
	"			t = sqrt(xx*xx + yy*yy);\n"
	"		}\n"
	"		else if(s > (length - w2))\n"
	"		{\n"
	"			float xx = (s - length + w2)/w2;\n"
	"			float yy = t;\n"
	"			if((xx*xx + yy*yy) > 1.0)\n"
	"			{\n"
	"				discard;\n"
	"			}\n"
	"			t = sqrt(xx*xx + yy*yy);\n"
	"		}\n"
	"	}\n"
	"	\n"
	"	if((t < stripe1) || (t > stripe2))\n"
	"	{\n"
	"		color = color2;\n"
	"	}\n"
	"	\n"
	"	gl_FragColor = color;\n"
	"}\n";

/***********************************************************
* public                                                   *
***********************************************************/

a3d_lineShader_t* a3d_lineShader_new(void)
{
	a3d_lineShader_t* self = (a3d_lineShader_t *)
	                         malloc(sizeof(a3d_lineShader_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	// line shader
	self->prog = a3d_shader_make_source(VSHADER, FSHADER);
	if(self->prog == 0)
	{
		goto fail_prog;
	}
	self->attr_vtx     = glGetAttribLocation(self->prog, "vtx");
	self->attr_st      = glGetAttribLocation(self->prog, "st");
	self->unif_mvp     = glGetUniformLocation(self->prog, "mvp");
	self->unif_width   = glGetUniformLocation(self->prog, "width");
	self->unif_length  = glGetUniformLocation(self->prog, "length");
	self->unif_rounded = glGetUniformLocation(self->prog, "rounded");
	self->unif_brush1  = glGetUniformLocation(self->prog, "brush1");
	self->unif_brush2  = glGetUniformLocation(self->prog, "brush2");
	self->unif_stripe1 = glGetUniformLocation(self->prog, "stripe1");
	self->unif_stripe2 = glGetUniformLocation(self->prog, "stripe2");
	self->unif_color1  = glGetUniformLocation(self->prog, "color1");
	self->unif_color2  = glGetUniformLocation(self->prog, "color2");

	self->blend = 0;

	// success
	return self;

	// failure
	fail_prog:
		free(self);
	return NULL;
}

void a3d_lineShader_delete(a3d_lineShader_t** _self)
{
	// *_self can be null
	assert(_self);

	a3d_lineShader_t* self = *_self;
	if(self)
	{
		glUseProgram(0);
		glDeleteProgram(self->prog);
		free(self);
		*_self = NULL;
	}
}

void a3d_lineShader_begin(a3d_lineShader_t* self)
{
	glEnableVertexAttribArray(self->attr_vtx);
	glEnableVertexAttribArray(self->attr_st);
	glUseProgram(self->prog);
}

void a3d_lineShader_end(a3d_lineShader_t* self)
{
	a3d_lineShader_blend(self, 0);
	glDisableVertexAttribArray(self->attr_st);
	glDisableVertexAttribArray(self->attr_vtx);
	glUseProgram(0);
}
