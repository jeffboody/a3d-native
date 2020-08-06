/*
 * Copyright (c) 2009-2011 Jeff Boody
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

#include "a3d_GL.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <dlfcn.h>

#define LOG_TAG "a3d"
#include "a3d_log.h"

/***********************************************************
* function ids                                             *
***********************************************************/

typedef enum
{
	A3D_GLID_glActiveTexture,
	A3D_GLID_glAttachShader,
	A3D_GLID_glBindAttribLocation,
	A3D_GLID_glBindBuffer,
	A3D_GLID_glBindFramebuffer,
	A3D_GLID_glBindRenderbuffer,
	A3D_GLID_glBindTexture,
	A3D_GLID_glBlendColor,
	A3D_GLID_glBlendEquation,
	A3D_GLID_glBlendEquationSeparate,
	A3D_GLID_glBlendFunc,
	A3D_GLID_glBlendFuncSeparate,
	A3D_GLID_glBufferData,
	A3D_GLID_glBufferSubData,
	A3D_GLID_glCheckFramebufferStatus,
	A3D_GLID_glClear,
	A3D_GLID_glClearColor,
	A3D_GLID_glClearDepthf,
	A3D_GLID_glClearStencil,
	A3D_GLID_glColorMask,
	A3D_GLID_glCompileShader,
	A3D_GLID_glCompressedTexImage2D,
	A3D_GLID_glCompressedTexSubImage2D,
	A3D_GLID_glCopyTexImage2D,
	A3D_GLID_glCopyTexSubImage2D,
	A3D_GLID_glCreateProgram,
	A3D_GLID_glCreateShader,
	A3D_GLID_glCullFace,
	A3D_GLID_glDeleteBuffers,
	A3D_GLID_glDeleteFramebuffers,
	A3D_GLID_glDeleteProgram,
	A3D_GLID_glDeleteRenderbuffers,
	A3D_GLID_glDeleteShader,
	A3D_GLID_glDeleteTextures,
	A3D_GLID_glDepthFunc,
	A3D_GLID_glDepthMask,
	A3D_GLID_glDepthRangef,
	A3D_GLID_glDetachShader,
	A3D_GLID_glDisable,
	A3D_GLID_glDisableVertexAttribArray,
	A3D_GLID_glDrawArrays,
	A3D_GLID_glDrawElements,
	A3D_GLID_glEnable,
	A3D_GLID_glEnableVertexAttribArray,
	A3D_GLID_glFinish,
	A3D_GLID_glFlush,
	A3D_GLID_glFramebufferRenderbuffer,
	A3D_GLID_glFramebufferTexture2D,
	A3D_GLID_glFrontFace,
	A3D_GLID_glGenBuffers,
	A3D_GLID_glGenerateMipmap,
	A3D_GLID_glGenFramebuffers,
	A3D_GLID_glGenRenderbuffers,
	A3D_GLID_glGenTextures,
	A3D_GLID_glGetActiveAttrib,
	A3D_GLID_glGetActiveUniform,
	A3D_GLID_glGetAttachedShaders,
	A3D_GLID_glGetAttribLocation,
	A3D_GLID_glGetBooleanv,
	A3D_GLID_glGetBufferParameteriv,
	A3D_GLID_glGetError,
	A3D_GLID_glGetFloatv,
	A3D_GLID_glGetFramebufferAttachmentParameteriv,
	A3D_GLID_glGetIntegerv,
	A3D_GLID_glGetProgramiv,
	A3D_GLID_glGetProgramInfoLog,
	A3D_GLID_glGetRenderbufferParameteriv,
	A3D_GLID_glGetShaderiv,
	A3D_GLID_glGetShaderInfoLog,
	A3D_GLID_glGetShaderPrecisionFormat,
	A3D_GLID_glGetShaderSource,
	A3D_GLID_glGetString,
	A3D_GLID_glGetTexParameterfv,
	A3D_GLID_glGetTexParameteriv,
	A3D_GLID_glGetUniformfv,
	A3D_GLID_glGetUniformiv,
	A3D_GLID_glGetUniformLocation,
	A3D_GLID_glGetVertexAttribfv,
	A3D_GLID_glGetVertexAttribiv,
	A3D_GLID_glGetVertexAttribPointerv,
	A3D_GLID_glHint,
	A3D_GLID_glIsBuffer,
	A3D_GLID_glIsEnabled,
	A3D_GLID_glIsFramebuffer,
	A3D_GLID_glIsProgram,
	A3D_GLID_glIsRenderbuffer,
	A3D_GLID_glIsShader,
	A3D_GLID_glIsTexture,
	A3D_GLID_glLineWidth,
	A3D_GLID_glLinkProgram,
	A3D_GLID_glPixelStorei,
	A3D_GLID_glPolygonOffset,
	A3D_GLID_glReadPixels,
	A3D_GLID_glReleaseShaderCompiler,
	A3D_GLID_glRenderbufferStorage,
	A3D_GLID_glSampleCoverage,
	A3D_GLID_glScissor,
	A3D_GLID_glShaderBinary,
	A3D_GLID_glShaderSource,
	A3D_GLID_glStencilFunc,
	A3D_GLID_glStencilFuncSeparate,
	A3D_GLID_glStencilMask,
	A3D_GLID_glStencilMaskSeparate,
	A3D_GLID_glStencilOp,
	A3D_GLID_glStencilOpSeparate,
	A3D_GLID_glTexImage2D,
	A3D_GLID_glTexParameterf,
	A3D_GLID_glTexParameterfv,
	A3D_GLID_glTexParameteri,
	A3D_GLID_glTexParameteriv,
	A3D_GLID_glTexSubImage2D,
	A3D_GLID_glUniform1f,
	A3D_GLID_glUniform1fv,
	A3D_GLID_glUniform1i,
	A3D_GLID_glUniform1iv,
	A3D_GLID_glUniform2f,
	A3D_GLID_glUniform2fv,
	A3D_GLID_glUniform2i,
	A3D_GLID_glUniform2iv,
	A3D_GLID_glUniform3f,
	A3D_GLID_glUniform3fv,
	A3D_GLID_glUniform3i,
	A3D_GLID_glUniform3iv,
	A3D_GLID_glUniform4f,
	A3D_GLID_glUniform4fv,
	A3D_GLID_glUniform4i,
	A3D_GLID_glUniform4iv,
	A3D_GLID_glUniformMatrix2fv,
	A3D_GLID_glUniformMatrix3fv,
	A3D_GLID_glUniformMatrix4fv,
	A3D_GLID_glUseProgram,
	A3D_GLID_glValidateProgram,
	A3D_GLID_glVertexAttrib1f,
	A3D_GLID_glVertexAttrib1fv,
	A3D_GLID_glVertexAttrib2f,
	A3D_GLID_glVertexAttrib2fv,
	A3D_GLID_glVertexAttrib3f,
	A3D_GLID_glVertexAttrib3fv,
	A3D_GLID_glVertexAttrib4f,
	A3D_GLID_glVertexAttrib4fv,
	A3D_GLID_glVertexAttribPointer,
	A3D_GLID_glViewport,
	A3D_GLID_MAX,
} a3d_glid_t;

/***********************************************************
* function stats                                           *
***********************************************************/

#include "a3d_time.h"

typedef struct
{
	const char*  fname;
	unsigned int count;
	double       enter;
	double       total;
} a3d_glstat_t;

#define A3D_TOSTRING(f) #f

#define A3D_ENTER(f) \
	LOGD("debug"); \
	glstat[A3D_GLID_##f].enter = a3d_utime(); \
	++glstat[A3D_GLID_##f].count;

#define A3D_EXIT(f) \
	glstat[A3D_GLID_##f].total += a3d_utime() - glstat[A3D_GLID_##f].enter;

#define A3D_GLSTAT(f) \
	{ \
		.fname = A3D_TOSTRING(f), \
		.count = 0, \
		.enter = 0.0, \
		.total = 0.0 \
	},

static double       glstat_enter = 0.0;
static unsigned int glstat_draw_count = 0;
static double       glstat_draw_enter = 0.0;
static double       glstat_draw_total = 0.0;

static a3d_glstat_t glstat[] =
{
	/*-------------------------------------------------------------------------
	 * GL core functions.
	 *-----------------------------------------------------------------------*/

	A3D_GLSTAT(glActiveTexture)
	A3D_GLSTAT(glAttachShader)
	A3D_GLSTAT(glBindAttribLocation)
	A3D_GLSTAT(glBindBuffer)
	A3D_GLSTAT(glBindFramebuffer)
	A3D_GLSTAT(glBindRenderbuffer)
	A3D_GLSTAT(glBindTexture)
	A3D_GLSTAT(glBlendColor)
	A3D_GLSTAT(glBlendEquation)
	A3D_GLSTAT(glBlendEquationSeparate)
	A3D_GLSTAT(glBlendFunc)
	A3D_GLSTAT(glBlendFuncSeparate)
	A3D_GLSTAT(glBufferData)
	A3D_GLSTAT(glBufferSubData)
	A3D_GLSTAT(glCheckFramebufferStatus)
	A3D_GLSTAT(glClear)
	A3D_GLSTAT(glClearColor)
	A3D_GLSTAT(glClearDepthf)
	A3D_GLSTAT(glClearStencil)
	A3D_GLSTAT(glColorMask)
	A3D_GLSTAT(glCompileShader)
	A3D_GLSTAT(glCompressedTexImage2D)
	A3D_GLSTAT(glCompressedTexSubImage2D)
	A3D_GLSTAT(glCopyTexImage2D)
	A3D_GLSTAT(glCopyTexSubImage2D)
	A3D_GLSTAT(glCreateProgram)
	A3D_GLSTAT(glCreateShader)
	A3D_GLSTAT(glCullFace)
	A3D_GLSTAT(glDeleteBuffers)
	A3D_GLSTAT(glDeleteFramebuffers)
	A3D_GLSTAT(glDeleteProgram)
	A3D_GLSTAT(glDeleteRenderbuffers)
	A3D_GLSTAT(glDeleteShader)
	A3D_GLSTAT(glDeleteTextures)
	A3D_GLSTAT(glDepthFunc)
	A3D_GLSTAT(glDepthMask)
	A3D_GLSTAT(glDepthRangef)
	A3D_GLSTAT(glDetachShader)
	A3D_GLSTAT(glDisable)
	A3D_GLSTAT(glDisableVertexAttribArray)
	A3D_GLSTAT(glDrawArrays)
	A3D_GLSTAT(glDrawElements)
	A3D_GLSTAT(glEnable)
	A3D_GLSTAT(glEnableVertexAttribArray)
	A3D_GLSTAT(glFinish)
	A3D_GLSTAT(glFlush)
	A3D_GLSTAT(glFramebufferRenderbuffer)
	A3D_GLSTAT(glFramebufferTexture2D)
	A3D_GLSTAT(glFrontFace)
	A3D_GLSTAT(glGenBuffers)
	A3D_GLSTAT(glGenerateMipmap)
	A3D_GLSTAT(glGenFramebuffers)
	A3D_GLSTAT(glGenRenderbuffers)
	A3D_GLSTAT(glGenTextures)
	A3D_GLSTAT(glGetActiveAttrib)
	A3D_GLSTAT(glGetActiveUniform)
	A3D_GLSTAT(glGetAttachedShaders)
	A3D_GLSTAT(glGetAttribLocation)
	A3D_GLSTAT(glGetBooleanv)
	A3D_GLSTAT(glGetBufferParameteriv)
	A3D_GLSTAT(glGetError)
	A3D_GLSTAT(glGetFloatv)
	A3D_GLSTAT(glGetFramebufferAttachmentParameteriv)
	A3D_GLSTAT(glGetIntegerv)
	A3D_GLSTAT(glGetProgramiv)
	A3D_GLSTAT(glGetProgramInfoLog)
	A3D_GLSTAT(glGetRenderbufferParameteriv)
	A3D_GLSTAT(glGetShaderiv)
	A3D_GLSTAT(glGetShaderInfoLog)
	A3D_GLSTAT(glGetShaderPrecisionFormat)
	A3D_GLSTAT(glGetShaderSource)
	A3D_GLSTAT(glGetString)
	A3D_GLSTAT(glGetTexParameterfv)
	A3D_GLSTAT(glGetTexParameteriv)
	A3D_GLSTAT(glGetUniformfv)
	A3D_GLSTAT(glGetUniformiv)
	A3D_GLSTAT(glGetUniformLocation)
	A3D_GLSTAT(glGetVertexAttribfv)
	A3D_GLSTAT(glGetVertexAttribiv)
	A3D_GLSTAT(glGetVertexAttribPointerv)
	A3D_GLSTAT(glHint)
	A3D_GLSTAT(glIsBuffer)
	A3D_GLSTAT(glIsEnabled)
	A3D_GLSTAT(glIsFramebuffer)
	A3D_GLSTAT(glIsProgram)
	A3D_GLSTAT(glIsRenderbuffer)
	A3D_GLSTAT(glIsShader)
	A3D_GLSTAT(glIsTexture)
	A3D_GLSTAT(glLineWidth)
	A3D_GLSTAT(glLinkProgram)
	A3D_GLSTAT(glPixelStorei)
	A3D_GLSTAT(glPolygonOffset)
	A3D_GLSTAT(glReadPixels)
	A3D_GLSTAT(glReleaseShaderCompiler)
	A3D_GLSTAT(glRenderbufferStorage)
	A3D_GLSTAT(glSampleCoverage)
	A3D_GLSTAT(glScissor)
	A3D_GLSTAT(glShaderBinary)
	A3D_GLSTAT(glShaderSource)
	A3D_GLSTAT(glStencilFunc)
	A3D_GLSTAT(glStencilFuncSeparate)
	A3D_GLSTAT(glStencilMask)
	A3D_GLSTAT(glStencilMaskSeparate)
	A3D_GLSTAT(glStencilOp)
	A3D_GLSTAT(glStencilOpSeparate)
	A3D_GLSTAT(glTexImage2D)
	A3D_GLSTAT(glTexParameterf)
	A3D_GLSTAT(glTexParameterfv)
	A3D_GLSTAT(glTexParameteri)
	A3D_GLSTAT(glTexParameteriv)
	A3D_GLSTAT(glTexSubImage2D)
	A3D_GLSTAT(glUniform1f)
	A3D_GLSTAT(glUniform1fv)
	A3D_GLSTAT(glUniform1i)
	A3D_GLSTAT(glUniform1iv)
	A3D_GLSTAT(glUniform2f)
	A3D_GLSTAT(glUniform2fv)
	A3D_GLSTAT(glUniform2i)
	A3D_GLSTAT(glUniform2iv)
	A3D_GLSTAT(glUniform3f)
	A3D_GLSTAT(glUniform3fv)
	A3D_GLSTAT(glUniform3i)
	A3D_GLSTAT(glUniform3iv)
	A3D_GLSTAT(glUniform4f)
	A3D_GLSTAT(glUniform4fv)
	A3D_GLSTAT(glUniform4i)
	A3D_GLSTAT(glUniform4iv)
	A3D_GLSTAT(glUniformMatrix2fv)
	A3D_GLSTAT(glUniformMatrix3fv)
	A3D_GLSTAT(glUniformMatrix4fv)
	A3D_GLSTAT(glUseProgram)
	A3D_GLSTAT(glValidateProgram)
	A3D_GLSTAT(glVertexAttrib1f)
	A3D_GLSTAT(glVertexAttrib1fv)
	A3D_GLSTAT(glVertexAttrib2f)
	A3D_GLSTAT(glVertexAttrib2fv)
	A3D_GLSTAT(glVertexAttrib3f)
	A3D_GLSTAT(glVertexAttrib3fv)
	A3D_GLSTAT(glVertexAttrib4f)
	A3D_GLSTAT(glVertexAttrib4fv)
	A3D_GLSTAT(glVertexAttribPointer)
	A3D_GLSTAT(glViewport)
};

/***********************************************************
* implementation                                           *
***********************************************************/

#define A3D_GLVOIDFUNC(ret, f, args, params) \
	typedef ret (*cb_##f) args; \
	static cb_##f gl_##f = NULL; \
	GL_APICALL ret GL_APIENTRY f args \
	{ \
		A3D_ENTER(f) \
		gl_##f params; \
		A3D_EXIT(f) \
	}

#define A3D_GLTYPEFUNC(ret, f, args, params) \
	typedef ret (*cb_##f) args; \
	static cb_##f gl_##f = NULL; \
	GL_APICALL ret GL_APIENTRY f args \
	{ \
		A3D_ENTER(f) \
		ret r = gl_##f params; \
		A3D_EXIT(f) \
		return r; \
	}

/*-------------------------------------------------------------------------
 * GL core functions.
 *-----------------------------------------------------------------------*/

A3D_GLVOIDFUNC(void, glActiveTexture, (GLenum texture), (texture))
A3D_GLVOIDFUNC(void, glAttachShader, (GLuint program, GLuint shader), (program, shader))
A3D_GLVOIDFUNC(void, glBindAttribLocation, (GLuint program, GLuint index, const char* name), (program, index, name))
A3D_GLVOIDFUNC(void, glBindBuffer, (GLenum target, GLuint buffer), (target, buffer))
A3D_GLVOIDFUNC(void, glBindFramebuffer, (GLenum target, GLuint framebuffer), (target, framebuffer))
A3D_GLVOIDFUNC(void, glBindRenderbuffer, (GLenum target, GLuint renderbuffer), (target, renderbuffer))
A3D_GLVOIDFUNC(void, glBindTexture, (GLenum target, GLuint texture), (target, texture))
A3D_GLVOIDFUNC(void, glBlendColor, (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha), (red, green, blue, alpha))
A3D_GLVOIDFUNC(void, glBlendEquation, ( GLenum mode ), (mode))
A3D_GLVOIDFUNC(void, glBlendEquationSeparate, (GLenum modeRGB, GLenum modeAlpha), (modeRGB, modeAlpha))
A3D_GLVOIDFUNC(void, glBlendFunc, (GLenum sfactor, GLenum dfactor), (sfactor, dfactor))
A3D_GLVOIDFUNC(void, glBlendFuncSeparate, (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha), (srcRGB, dstRGB, srcAlpha, dstAlpha))
A3D_GLVOIDFUNC(void, glBufferData, (GLenum target, GLsizeiptr size, const void* data, GLenum usage), (target, size, data, usage))
A3D_GLVOIDFUNC(void, glBufferSubData, (GLenum target, GLintptr offset, GLsizeiptr size, const void* data), (target, offset, size, data))
A3D_GLTYPEFUNC(GLenum, glCheckFramebufferStatus, (GLenum target), (target))
A3D_GLVOIDFUNC(void, glClear, (GLbitfield mask), (mask))
A3D_GLVOIDFUNC(void, glClearColor, (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha), (red, green, blue, alpha))
A3D_GLVOIDFUNC(void, glClearDepthf, (GLclampf depth), (depth))
A3D_GLVOIDFUNC(void, glClearStencil, (GLint s), (s))
A3D_GLVOIDFUNC(void, glColorMask, (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha), (red, green, blue, alpha))
A3D_GLVOIDFUNC(void, glCompileShader, (GLuint shader), (shader))
A3D_GLVOIDFUNC(void, glCompressedTexImage2D, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data), (target, level, internalformat, width, height, border, imageSize, data))
A3D_GLVOIDFUNC(void, glCompressedTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data), (target, level, xoffset, yoffset, width, height, format, imageSize, data))
A3D_GLVOIDFUNC(void, glCopyTexImage2D, (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border), (target, level, internalformat, x, y, width, height, border))
A3D_GLVOIDFUNC(void, glCopyTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height), (target, level, xoffset, yoffset, x, y, width, height))
A3D_GLTYPEFUNC(GLuint, glCreateProgram, (void), ())
A3D_GLTYPEFUNC(GLuint, glCreateShader, (GLenum type), (type))
A3D_GLVOIDFUNC(void, glCullFace, (GLenum mode), (mode))
A3D_GLVOIDFUNC(void, glDeleteBuffers, (GLsizei n, const GLuint* buffers), (n, buffers))
A3D_GLVOIDFUNC(void, glDeleteFramebuffers, (GLsizei n, const GLuint* framebuffers), (n, framebuffers))
A3D_GLVOIDFUNC(void, glDeleteProgram, (GLuint program), (program))
A3D_GLVOIDFUNC(void, glDeleteRenderbuffers, (GLsizei n, const GLuint* renderbuffers), (n, renderbuffers))
A3D_GLVOIDFUNC(void, glDeleteShader, (GLuint shader), (shader))
A3D_GLVOIDFUNC(void, glDeleteTextures, (GLsizei n, const GLuint* textures), (n, textures))
A3D_GLVOIDFUNC(void, glDepthFunc, (GLenum func), (func))
A3D_GLVOIDFUNC(void, glDepthMask, (GLboolean flag), (flag))
A3D_GLVOIDFUNC(void, glDepthRangef, (GLclampf zNear, GLclampf zFar), (zNear, zFar))
A3D_GLVOIDFUNC(void, glDetachShader, (GLuint program, GLuint shader), (program, shader))
A3D_GLVOIDFUNC(void, glDisable, (GLenum cap), (cap))
A3D_GLVOIDFUNC(void, glDisableVertexAttribArray, (GLuint index), (index))
A3D_GLVOIDFUNC(void, glDrawArrays, (GLenum mode, GLint first, GLsizei count), (mode, first, count))
A3D_GLVOIDFUNC(void, glDrawElements, (GLenum mode, GLsizei count, GLenum type, const void* indices), (mode, count, type, indices))
A3D_GLVOIDFUNC(void, glEnable, (GLenum cap), (cap))
A3D_GLVOIDFUNC(void, glEnableVertexAttribArray, (GLuint index), (index))
A3D_GLVOIDFUNC(void, glFinish, (void), ())
A3D_GLVOIDFUNC(void, glFlush, (void), ())
A3D_GLVOIDFUNC(void, glFramebufferRenderbuffer, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer), (target, attachment, renderbuffertarget, renderbuffer))
A3D_GLVOIDFUNC(void, glFramebufferTexture2D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level), (target, attachment, textarget, texture, level))
A3D_GLVOIDFUNC(void, glFrontFace, (GLenum mode), (mode))
A3D_GLVOIDFUNC(void, glGenBuffers, (GLsizei n, GLuint* buffers), (n, buffers))
A3D_GLVOIDFUNC(void, glGenerateMipmap, (GLenum target), (target))
A3D_GLVOIDFUNC(void, glGenFramebuffers, (GLsizei n, GLuint* framebuffers), (n, framebuffers))
A3D_GLVOIDFUNC(void, glGenRenderbuffers, (GLsizei n, GLuint* renderbuffers), (n, renderbuffers))
A3D_GLVOIDFUNC(void, glGenTextures, (GLsizei n, GLuint* textures), (n, textures))
A3D_GLVOIDFUNC(void, glGetActiveAttrib, (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name), (program, index, bufsize, length, size, type, name))
A3D_GLVOIDFUNC(void, glGetActiveUniform, (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name), (program, index, bufsize, length, size, type, name))
A3D_GLVOIDFUNC(void, glGetAttachedShaders, (GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders), (program, maxcount, count, shaders))
A3D_GLTYPEFUNC(int, glGetAttribLocation, (GLuint program, const char* name), (program, name))
A3D_GLVOIDFUNC(void, glGetBooleanv, (GLenum pname, GLboolean* params), (pname, params))
A3D_GLVOIDFUNC(void, glGetBufferParameteriv, (GLenum target, GLenum pname, GLint* params), (target, pname, params))
A3D_GLTYPEFUNC(GLenum, glGetError, (void), ())
A3D_GLVOIDFUNC(void, glGetFloatv, (GLenum pname, GLfloat* params), (pname, params))
A3D_GLVOIDFUNC(void, glGetFramebufferAttachmentParameteriv, (GLenum target, GLenum attachment, GLenum pname, GLint* params), (target, attachment, pname, params))
A3D_GLVOIDFUNC(void, glGetIntegerv, (GLenum pname, GLint* params), (pname, params))
A3D_GLVOIDFUNC(void, glGetProgramiv, (GLuint program, GLenum pname, GLint* params), (program, pname, params))
A3D_GLVOIDFUNC(void, glGetProgramInfoLog, (GLuint program, GLsizei bufsize, GLsizei* length, char* infolog), (program, bufsize, length, infolog))
A3D_GLVOIDFUNC(void, glGetRenderbufferParameteriv, (GLenum target, GLenum pname, GLint* params), (target, pname, params))
A3D_GLVOIDFUNC(void, glGetShaderiv, (GLuint shader, GLenum pname, GLint* params), (shader, pname, params))
A3D_GLVOIDFUNC(void, glGetShaderInfoLog, (GLuint shader, GLsizei bufsize, GLsizei* length, char* infolog), (shader, bufsize, length, infolog))
A3D_GLVOIDFUNC(void, glGetShaderPrecisionFormat, (GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision), (shadertype, precisiontype, range, precision))
A3D_GLVOIDFUNC(void, glGetShaderSource, (GLuint shader, GLsizei bufsize, GLsizei* length, char* source), (shader, bufsize, length, source))
A3D_GLTYPEFUNC(const GLubyte*, glGetString, (GLenum name), (name))
A3D_GLVOIDFUNC(void, glGetTexParameterfv, (GLenum target, GLenum pname, GLfloat* params), (target, pname, params))
A3D_GLVOIDFUNC(void, glGetTexParameteriv, (GLenum target, GLenum pname, GLint* params), (target, pname, params))
A3D_GLVOIDFUNC(void, glGetUniformfv, (GLuint program, GLint location, GLfloat* params), (program, location, params))
A3D_GLVOIDFUNC(void, glGetUniformiv, (GLuint program, GLint location, GLint* params), (program, location, params))
A3D_GLTYPEFUNC(int, glGetUniformLocation, (GLuint program, const char* name), (program, name))
A3D_GLVOIDFUNC(void, glGetVertexAttribfv, (GLuint index, GLenum pname, GLfloat* params), (index, pname, params))
A3D_GLVOIDFUNC(void, glGetVertexAttribiv, (GLuint index, GLenum pname, GLint* params), (index, pname, params))
A3D_GLVOIDFUNC(void, glGetVertexAttribPointerv, (GLuint index, GLenum pname, void** pointer), (index, pname, pointer))
A3D_GLVOIDFUNC(void, glHint, (GLenum target, GLenum mode), (target, mode))
A3D_GLTYPEFUNC(GLboolean, glIsBuffer, (GLuint buffer), (buffer))
A3D_GLTYPEFUNC(GLboolean, glIsEnabled, (GLenum cap), (cap))
A3D_GLTYPEFUNC(GLboolean, glIsFramebuffer, (GLuint framebuffer), (framebuffer))
A3D_GLTYPEFUNC(GLboolean, glIsProgram, (GLuint program), (program))
A3D_GLTYPEFUNC(GLboolean, glIsRenderbuffer, (GLuint renderbuffer), (renderbuffer))
A3D_GLTYPEFUNC(GLboolean, glIsShader, (GLuint shader), (shader))
A3D_GLTYPEFUNC(GLboolean, glIsTexture, (GLuint texture), (texture))
A3D_GLVOIDFUNC(void, glLineWidth, (GLfloat width), (width))
A3D_GLVOIDFUNC(void, glLinkProgram, (GLuint program), (program))
A3D_GLVOIDFUNC(void, glPixelStorei, (GLenum pname, GLint param), (pname, param))
A3D_GLVOIDFUNC(void, glPolygonOffset, (GLfloat factor, GLfloat units), (factor, units))
A3D_GLVOIDFUNC(void, glReadPixels, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels), (x, y, width, height, format, type, pixels))
A3D_GLVOIDFUNC(void, glReleaseShaderCompiler, (void), ())
A3D_GLVOIDFUNC(void, glRenderbufferStorage, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height), (target, internalformat, width, height))
A3D_GLVOIDFUNC(void, glSampleCoverage, (GLclampf value, GLboolean invert), (value, invert))
A3D_GLVOIDFUNC(void, glScissor, (GLint x, GLint y, GLsizei width, GLsizei height), (x, y, width, height))
A3D_GLVOIDFUNC(void, glShaderBinary, (GLsizei n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length), (n, shaders, binaryformat, binary, length))
A3D_GLVOIDFUNC(void, glShaderSource, (GLuint shader, GLsizei count, const GLchar * const *string, const GLint* length), (shader, count, string, length))
A3D_GLVOIDFUNC(void, glStencilFunc, (GLenum func, GLint ref, GLuint mask), (func, ref, mask))
A3D_GLVOIDFUNC(void, glStencilFuncSeparate, (GLenum face, GLenum func, GLint ref, GLuint mask), (face, func, ref, mask))
A3D_GLVOIDFUNC(void, glStencilMask, (GLuint mask), (mask))
A3D_GLVOIDFUNC(void, glStencilMaskSeparate, (GLenum face, GLuint mask), (face, mask))
A3D_GLVOIDFUNC(void, glStencilOp, (GLenum fail, GLenum zfail, GLenum zpass), (fail, zfail, zpass))
A3D_GLVOIDFUNC(void, glStencilOpSeparate, (GLenum face, GLenum fail, GLenum zfail, GLenum zpass), (face, fail, zfail, zpass))
A3D_GLVOIDFUNC(void, glTexImage2D, (GLenum target, GLint level, GLint internalformat,  GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels), (target, level, internalformat, width, height, border, format, type, pixels))
A3D_GLVOIDFUNC(void, glTexParameterf, (GLenum target, GLenum pname, GLfloat param), (target, pname, param))
A3D_GLVOIDFUNC(void, glTexParameterfv, (GLenum target, GLenum pname, const GLfloat* params), (target, pname, params))
A3D_GLVOIDFUNC(void, glTexParameteri, (GLenum target, GLenum pname, GLint param), (target, pname, param))
A3D_GLVOIDFUNC(void, glTexParameteriv, (GLenum target, GLenum pname, const GLint* params), (target, pname, params))
A3D_GLVOIDFUNC(void, glTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels), (target, level, xoffset, yoffset, width, height, format, type, pixels))
A3D_GLVOIDFUNC(void, glUniform1f, (GLint location, GLfloat x), (location, x))
A3D_GLVOIDFUNC(void, glUniform1fv, (GLint location, GLsizei count, const GLfloat* v), (location, count, v))
A3D_GLVOIDFUNC(void, glUniform1i, (GLint location, GLint x), (location, x))
A3D_GLVOIDFUNC(void, glUniform1iv, (GLint location, GLsizei count, const GLint* v), (location, count, v))
A3D_GLVOIDFUNC(void, glUniform2f, (GLint location, GLfloat x, GLfloat y), (location, x, y))
A3D_GLVOIDFUNC(void, glUniform2fv, (GLint location, GLsizei count, const GLfloat* v), (location, count, v))
A3D_GLVOIDFUNC(void, glUniform2i, (GLint location, GLint x, GLint y), (location, x, y))
A3D_GLVOIDFUNC(void, glUniform2iv, (GLint location, GLsizei count, const GLint* v), (location, count, v))
A3D_GLVOIDFUNC(void, glUniform3f, (GLint location, GLfloat x, GLfloat y, GLfloat z), (location, x, y, z))
A3D_GLVOIDFUNC(void, glUniform3fv, (GLint location, GLsizei count, const GLfloat* v), (location, count, v))
A3D_GLVOIDFUNC(void, glUniform3i, (GLint location, GLint x, GLint y, GLint z), (location, x, y, z))
A3D_GLVOIDFUNC(void, glUniform3iv, (GLint location, GLsizei count, const GLint* v), (location, count, v))
A3D_GLVOIDFUNC(void, glUniform4f, (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w), (location, x, y, z, w))
A3D_GLVOIDFUNC(void, glUniform4fv, (GLint location, GLsizei count, const GLfloat* v), (location, count, v))
A3D_GLVOIDFUNC(void, glUniform4i, (GLint location, GLint x, GLint y, GLint z, GLint w), (location, x, y, z, w))
A3D_GLVOIDFUNC(void, glUniform4iv, (GLint location, GLsizei count, const GLint* v), (location, count, v))
A3D_GLVOIDFUNC(void, glUniformMatrix2fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value), (location, count, transpose, value))
A3D_GLVOIDFUNC(void, glUniformMatrix3fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value), (location, count, transpose, value))
A3D_GLVOIDFUNC(void, glUniformMatrix4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value), (location, count, transpose, value))
A3D_GLVOIDFUNC(void, glUseProgram, (GLuint program), (program))
A3D_GLVOIDFUNC(void, glValidateProgram, (GLuint program), (program))
A3D_GLVOIDFUNC(void, glVertexAttrib1f, (GLuint indx, GLfloat x), (indx, x))
A3D_GLVOIDFUNC(void, glVertexAttrib1fv, (GLuint indx, const GLfloat* values), (indx, values))
A3D_GLVOIDFUNC(void, glVertexAttrib2f, (GLuint indx, GLfloat x, GLfloat y), (indx, x, y))
A3D_GLVOIDFUNC(void, glVertexAttrib2fv, (GLuint indx, const GLfloat* values), (indx, values))
A3D_GLVOIDFUNC(void, glVertexAttrib3f, (GLuint indx, GLfloat x, GLfloat y, GLfloat z), (indx, x, y, z))
A3D_GLVOIDFUNC(void, glVertexAttrib3fv, (GLuint indx, const GLfloat* values), (indx, values))
A3D_GLVOIDFUNC(void, glVertexAttrib4f, (GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w), (indx, x, y, z, w))
A3D_GLVOIDFUNC(void, glVertexAttrib4fv, (GLuint indx, const GLfloat* values), (indx, values))
A3D_GLVOIDFUNC(void, glVertexAttribPointer, (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* ptr), (indx, size, type, normalized, stride, ptr))
A3D_GLVOIDFUNC(void, glViewport, (GLint x, GLint y, GLsizei width, GLsizei height), (x, y, width, height))

/***********************************************************
* control functions                                        *
***********************************************************/

static void* library = NULL;

#define A3D_GLLOAD(f) \
	(gl_##f = (cb_##f) dlsym(library, #f)); \
	if(gl_##f == NULL) \
	{ \
		LOGE("dlsym failed"); \
		return 0; \
	}

static void a3d_GLES_dump(void)
{
	// dump stats
	double total = a3d_utime() - glstat_enter;
	LOGI("total=%.0lf usec", total);
	LOGI("draw=%.0lf usec, %.0lf percent", glstat_draw_total, 100.0 * glstat_draw_total / total);
	LOGI("frames=%u", glstat_draw_count);
	LOGI("fps=%.0lf", glstat_draw_count / (total / A3D_USEC));
	LOGI("|---------------------------------|--------------|--------------|", "fname", "count", "total");
	LOGI("|             fname               |     count    | total (usec) |");
	LOGI("|---------------------------------|--------------|--------------|", "fname", "count", "total");
	int i = 0;
	for(i = 0; i < (int) A3D_GLID_MAX; ++i)
	{
		if(glstat[i].count > 0)
		{
			LOGI("|%32s | %12i | %12.0lf |", glstat[i].fname, glstat[i].count, glstat[i].total);
		}
	}
	LOGI("|---------------------------------|--------------|--------------|", "fname", "count", "total");
}

static void a3d_GLES_reset(void)
{
	// reset stats
	glstat_enter = a3d_utime();
	glstat_draw_count = 0;
	glstat_draw_enter = glstat_enter;
	glstat_draw_total = 0.0;
	int i = 0;
	for(i = 0; i < (int) A3D_GLID_MAX; ++i)
	{
		glstat[i].count = 0;
		glstat[i].enter = 0.0;
		glstat[i].total = 0.0;
	}
}

int  a3d_GL_load(void)
{
	if(library != NULL)
	{
		LOGE("libGLESv2_CM.so already loaded");
		return 0;
	}

    library = dlopen("libGLESv2.so", RTLD_NOW);
	if(library == NULL)
	{
		LOGE("dlopen failed");
		return 0;
	}

	/*-------------------------------------------------------------------------
	 * GL core functions.
	 *-----------------------------------------------------------------------*/

	A3D_GLLOAD(glActiveTexture)
	A3D_GLLOAD(glAttachShader)
	A3D_GLLOAD(glBindAttribLocation)
	A3D_GLLOAD(glBindBuffer)
	A3D_GLLOAD(glBindFramebuffer)
	A3D_GLLOAD(glBindRenderbuffer)
	A3D_GLLOAD(glBindTexture)
	A3D_GLLOAD(glBlendColor)
	A3D_GLLOAD(glBlendEquation)
	A3D_GLLOAD(glBlendEquationSeparate)
	A3D_GLLOAD(glBlendFunc)
	A3D_GLLOAD(glBlendFuncSeparate)
	A3D_GLLOAD(glBufferData)
	A3D_GLLOAD(glBufferSubData)
	A3D_GLLOAD(glCheckFramebufferStatus)
	A3D_GLLOAD(glClear)
	A3D_GLLOAD(glClearColor)
	A3D_GLLOAD(glClearDepthf)
	A3D_GLLOAD(glClearStencil)
	A3D_GLLOAD(glColorMask)
	A3D_GLLOAD(glCompileShader)
	A3D_GLLOAD(glCompressedTexImage2D)
	A3D_GLLOAD(glCompressedTexSubImage2D)
	A3D_GLLOAD(glCopyTexImage2D)
	A3D_GLLOAD(glCopyTexSubImage2D)
	A3D_GLLOAD(glCreateProgram)
	A3D_GLLOAD(glCreateShader)
	A3D_GLLOAD(glCullFace)
	A3D_GLLOAD(glDeleteBuffers)
	A3D_GLLOAD(glDeleteFramebuffers)
	A3D_GLLOAD(glDeleteProgram)
	A3D_GLLOAD(glDeleteRenderbuffers)
	A3D_GLLOAD(glDeleteShader)
	A3D_GLLOAD(glDeleteTextures)
	A3D_GLLOAD(glDepthFunc)
	A3D_GLLOAD(glDepthMask)
	A3D_GLLOAD(glDepthRangef)
	A3D_GLLOAD(glDetachShader)
	A3D_GLLOAD(glDisable)
	A3D_GLLOAD(glDisableVertexAttribArray)
	A3D_GLLOAD(glDrawArrays)
	A3D_GLLOAD(glDrawElements)
	A3D_GLLOAD(glEnable)
	A3D_GLLOAD(glEnableVertexAttribArray)
	A3D_GLLOAD(glFinish)
	A3D_GLLOAD(glFlush)
	A3D_GLLOAD(glFramebufferRenderbuffer)
	A3D_GLLOAD(glFramebufferTexture2D)
	A3D_GLLOAD(glFrontFace)
	A3D_GLLOAD(glGenBuffers)
	A3D_GLLOAD(glGenerateMipmap)
	A3D_GLLOAD(glGenFramebuffers)
	A3D_GLLOAD(glGenRenderbuffers)
	A3D_GLLOAD(glGenTextures)
	A3D_GLLOAD(glGetActiveAttrib)
	A3D_GLLOAD(glGetActiveUniform)
	A3D_GLLOAD(glGetAttachedShaders)
	A3D_GLLOAD(glGetAttribLocation)
	A3D_GLLOAD(glGetBooleanv)
	A3D_GLLOAD(glGetBufferParameteriv)
	A3D_GLLOAD(glGetError)
	A3D_GLLOAD(glGetFloatv)
	A3D_GLLOAD(glGetFramebufferAttachmentParameteriv)
	A3D_GLLOAD(glGetIntegerv)
	A3D_GLLOAD(glGetProgramiv)
	A3D_GLLOAD(glGetProgramInfoLog)
	A3D_GLLOAD(glGetRenderbufferParameteriv)
	A3D_GLLOAD(glGetShaderiv)
	A3D_GLLOAD(glGetShaderInfoLog)
	A3D_GLLOAD(glGetShaderPrecisionFormat)
	A3D_GLLOAD(glGetShaderSource)
	A3D_GLLOAD(glGetString)
	A3D_GLLOAD(glGetTexParameterfv)
	A3D_GLLOAD(glGetTexParameteriv)
	A3D_GLLOAD(glGetUniformfv)
	A3D_GLLOAD(glGetUniformiv)
	A3D_GLLOAD(glGetUniformLocation)
	A3D_GLLOAD(glGetVertexAttribfv)
	A3D_GLLOAD(glGetVertexAttribiv)
	A3D_GLLOAD(glGetVertexAttribPointerv)
	A3D_GLLOAD(glHint)
	A3D_GLLOAD(glIsBuffer)
	A3D_GLLOAD(glIsEnabled)
	A3D_GLLOAD(glIsFramebuffer)
	A3D_GLLOAD(glIsProgram)
	A3D_GLLOAD(glIsRenderbuffer)
	A3D_GLLOAD(glIsShader)
	A3D_GLLOAD(glIsTexture)
	A3D_GLLOAD(glLineWidth)
	A3D_GLLOAD(glLinkProgram)
	A3D_GLLOAD(glPixelStorei)
	A3D_GLLOAD(glPolygonOffset)
	A3D_GLLOAD(glReadPixels)
	A3D_GLLOAD(glReleaseShaderCompiler)
	A3D_GLLOAD(glRenderbufferStorage)
	A3D_GLLOAD(glSampleCoverage)
	A3D_GLLOAD(glScissor)
	A3D_GLLOAD(glShaderBinary)
	A3D_GLLOAD(glShaderSource)
	A3D_GLLOAD(glStencilFunc)
	A3D_GLLOAD(glStencilFuncSeparate)
	A3D_GLLOAD(glStencilMask)
	A3D_GLLOAD(glStencilMaskSeparate)
	A3D_GLLOAD(glStencilOp)
	A3D_GLLOAD(glStencilOpSeparate)
	A3D_GLLOAD(glTexImage2D)
	A3D_GLLOAD(glTexParameterf)
	A3D_GLLOAD(glTexParameterfv)
	A3D_GLLOAD(glTexParameteri)
	A3D_GLLOAD(glTexParameteriv)
	A3D_GLLOAD(glTexSubImage2D)
	A3D_GLLOAD(glUniform1f)
	A3D_GLLOAD(glUniform1fv)
	A3D_GLLOAD(glUniform1i)
	A3D_GLLOAD(glUniform1iv)
	A3D_GLLOAD(glUniform2f)
	A3D_GLLOAD(glUniform2fv)
	A3D_GLLOAD(glUniform2i)
	A3D_GLLOAD(glUniform2iv)
	A3D_GLLOAD(glUniform3f)
	A3D_GLLOAD(glUniform3fv)
	A3D_GLLOAD(glUniform3i)
	A3D_GLLOAD(glUniform3iv)
	A3D_GLLOAD(glUniform4f)
	A3D_GLLOAD(glUniform4fv)
	A3D_GLLOAD(glUniform4i)
	A3D_GLLOAD(glUniform4iv)
	A3D_GLLOAD(glUniformMatrix2fv)
	A3D_GLLOAD(glUniformMatrix3fv)
	A3D_GLLOAD(glUniformMatrix4fv)
	A3D_GLLOAD(glUseProgram)
	A3D_GLLOAD(glValidateProgram)
	A3D_GLLOAD(glVertexAttrib1f)
	A3D_GLLOAD(glVertexAttrib1fv)
	A3D_GLLOAD(glVertexAttrib2f)
	A3D_GLLOAD(glVertexAttrib2fv)
	A3D_GLLOAD(glVertexAttrib3f)
	A3D_GLLOAD(glVertexAttrib3fv)
	A3D_GLLOAD(glVertexAttrib4f)
	A3D_GLLOAD(glVertexAttrib4fv)
	A3D_GLLOAD(glVertexAttribPointer)
	A3D_GLLOAD(glViewport)

	a3d_GLES_reset();
	return 1;
}

int a3d_GL_unload(void)
{
	a3d_GLES_dump();
	dlclose(library);
	library = NULL;
	return 0;
}

void a3d_GL_frame_begin(void)
{
	glstat_draw_enter = a3d_utime();
}

void a3d_GL_frame_end(void)
{
	++glstat_draw_count;
	glstat_draw_total += a3d_utime() - glstat_draw_enter;

	if(glstat_draw_count >= 1000)
	{
		a3d_GLES_dump();
		a3d_GLES_reset();
	}
}
