/*
 * Copyright (c) 2009-2010 Jeff Boody
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

#include "a3d_shader.h"
#include <stdlib.h>
#include <assert.h>

#define LOG_TAG "a3d"
#include "a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static GLuint a3d_shader_source(GLuint type, const char* source)
{
	assert(source);
	LOGD("debug type=0x%x", (int) type);

	GLuint shader = glCreateShader(type);
	if(shader == 0)
	{
		LOGE("glCreateShader failed");
		return 0;
	}

	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if(status == GL_FALSE)
	{
		LOGE("compile failed");

		char buffer[512];
		glGetShaderInfoLog(shader, 512, NULL, buffer);
		LOGE("ShaderInfoLog: %s", buffer);
		goto fail_compile;
	}

	if(glIsShader(shader) == GL_FALSE)
	{
		LOGE("not a shader");
		goto fail_compile;
	}

	GLenum e = A3D_GL_GETERROR();
	if(e != GL_NO_ERROR)
		goto fail_error;

	// success
	return shader;

	// failure
	fail_error:
	fail_compile:
		glDeleteShader(shader);
	return 0;
}

static GLuint a3d_shader_link(GLuint vshader, GLuint fshader)
{
	assert(vshader);
	assert(fshader);
	LOGD("debug");

	GLuint program = glCreateProgram();
	if(program == 0)
	{
		LOGE("glCreateProgram failed");
		return 0;
	}

	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if(status == GL_FALSE)
	{
		LOGE("link failed");

		char buffer[512];
		glGetProgramInfoLog(program, 512, NULL, buffer);
		LOGI("ProgramInfoLog: %s", buffer);
		goto fail_link;
	}

	if(glIsProgram(program) == GL_FALSE)
	{
		LOGE("not a program");
		goto fail_link;
	}

	GLenum e = A3D_GL_GETERROR();
	if(e != GL_NO_ERROR)
		goto fail_error;

	// success
	return program;

	// failure
	fail_error:
	fail_link:
		glDeleteProgram(program);
	return 0;
}

/***********************************************************
* public                                                   *
***********************************************************/

GLuint a3d_shader_make_source(const char* vsource, const char* fsource)
{
	assert(vsource);
	assert(fsource);
	LOGD("debug");

	GLuint vshader = a3d_shader_source(GL_VERTEX_SHADER, vsource);
	if(vshader == 0)
		return 0;

	GLuint fshader = a3d_shader_source(GL_FRAGMENT_SHADER, fsource);
	if(fshader == 0)
		goto fail_fshader;

	GLuint program = a3d_shader_link(vshader, fshader);
	if(program == 0)
		goto fail_program;

	glDeleteShader(fshader);
	glDeleteShader(vshader);

	// success
	return program;

	// failure
	fail_program:
		glDeleteShader(fshader);
	fail_fshader:
		glDeleteShader(vshader);
	return 0;
}
