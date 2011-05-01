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
	A3D_GLID_glAlphaFunc,
	A3D_GLID_glClearColor,
	A3D_GLID_glClearDepthf,
	A3D_GLID_glClipPlanef,
	A3D_GLID_glColor4f,
	A3D_GLID_glDepthRangef,
	A3D_GLID_glFogf,
	A3D_GLID_glFogfv,
	A3D_GLID_glFrustumf,
	A3D_GLID_glGetClipPlanef,
	A3D_GLID_glGetFloatv,
	A3D_GLID_glGetLightfv,
	A3D_GLID_glGetMaterialfv,
	A3D_GLID_glGetTexEnvfv,
	A3D_GLID_glGetTexParameterfv,
	A3D_GLID_glLightModelf,
	A3D_GLID_glLightModelfv,
	A3D_GLID_glLightf,
	A3D_GLID_glLightfv,
	A3D_GLID_glLineWidth,
	A3D_GLID_glLoadMatrixf,
	A3D_GLID_glMaterialf,
	A3D_GLID_glMaterialfv,
	A3D_GLID_glMultMatrixf,
	A3D_GLID_glMultiTexCoord4f,
	A3D_GLID_glNormal3f,
	A3D_GLID_glOrthof,
	A3D_GLID_glPointParameterf,
	A3D_GLID_glPointParameterfv,
	A3D_GLID_glPointSize,
	A3D_GLID_glPolygonOffset,
	A3D_GLID_glRotatef,
	A3D_GLID_glScalef,
	A3D_GLID_glTexEnvf,
	A3D_GLID_glTexEnvfv,
	A3D_GLID_glTexParameterf,
	A3D_GLID_glTexParameterfv,
	A3D_GLID_glTranslatef,

	/* Available in both Common and Common-Lite profiles */

	A3D_GLID_glActiveTexture,
	A3D_GLID_glAlphaFuncx,
	A3D_GLID_glBindBuffer,
	A3D_GLID_glBindTexture,
	A3D_GLID_glBlendFunc,
	A3D_GLID_glBufferData,
	A3D_GLID_glBufferSubData,
	A3D_GLID_glClear,
	A3D_GLID_glClearColorx,
	A3D_GLID_glClearDepthx,
	A3D_GLID_glClearStencil,
	A3D_GLID_glClientActiveTexture,
	A3D_GLID_glClipPlanex,
	A3D_GLID_glColor4ub,
	A3D_GLID_glColor4x,
	A3D_GLID_glColorMask,
	A3D_GLID_glColorPointer,
	A3D_GLID_glCompressedTexImage2D,
	A3D_GLID_glCompressedTexSubImage2D,
	A3D_GLID_glCopyTexImage2D,
	A3D_GLID_glCopyTexSubImage2D,
	A3D_GLID_glCullFace,
	A3D_GLID_glDeleteBuffers,
	A3D_GLID_glDeleteTextures,
	A3D_GLID_glDepthFunc,
	A3D_GLID_glDepthMask,
	A3D_GLID_glDepthRangex,
	A3D_GLID_glDisable,
	A3D_GLID_glDisableClientState,
	A3D_GLID_glDrawArrays,
	A3D_GLID_glDrawElements,
	A3D_GLID_glEnable,
	A3D_GLID_glEnableClientState,
	A3D_GLID_glFinish,
	A3D_GLID_glFlush,
	A3D_GLID_glFogx,
	A3D_GLID_glFogxv,
	A3D_GLID_glFrontFace,
	A3D_GLID_glFrustumx,
	A3D_GLID_glGetBooleanv,
	A3D_GLID_glGetBufferParameteriv,
	A3D_GLID_glGetClipPlanex,
	A3D_GLID_glGenBuffers,
	A3D_GLID_glGenTextures,
	A3D_GLID_glGetError,
	A3D_GLID_glGetFixedv,
	A3D_GLID_glGetIntegerv,
	A3D_GLID_glGetLightxv,
	A3D_GLID_glGetMaterialxv,
	A3D_GLID_glGetPointerv,
	A3D_GLID_glGetString,
	A3D_GLID_glGetTexEnviv,
	A3D_GLID_glGetTexEnvxv,
	A3D_GLID_glGetTexParameteriv,
	A3D_GLID_glGetTexParameterxv,
	A3D_GLID_glHint,
	A3D_GLID_glIsBuffer,
	A3D_GLID_glIsEnabled,
	A3D_GLID_glIsTexture,
	A3D_GLID_glLightModelx,
	A3D_GLID_glLightModelxv,
	A3D_GLID_glLightx,
	A3D_GLID_glLightxv,
	A3D_GLID_glLineWidthx,
	A3D_GLID_glLoadIdentity,
	A3D_GLID_glLoadMatrixx,
	A3D_GLID_glLogicOp,
	A3D_GLID_glMaterialx,
	A3D_GLID_glMaterialxv,
	A3D_GLID_glMatrixMode,
	A3D_GLID_glMultMatrixx,
	A3D_GLID_glMultiTexCoord4x,
	A3D_GLID_glNormal3x,
	A3D_GLID_glNormalPointer,
	A3D_GLID_glOrthox,
	A3D_GLID_glPixelStorei,
	A3D_GLID_glPointParameterx,
	A3D_GLID_glPointParameterxv,
	A3D_GLID_glPointSizex,
	A3D_GLID_glPolygonOffsetx,
	A3D_GLID_glPopMatrix,
	A3D_GLID_glPushMatrix,
	A3D_GLID_glReadPixels,
	A3D_GLID_glRotatex,
	A3D_GLID_glSampleCoverage,
	A3D_GLID_glSampleCoveragex,
	A3D_GLID_glScalex,
	A3D_GLID_glScissor,
	A3D_GLID_glShadeModel,
	A3D_GLID_glStencilFunc,
	A3D_GLID_glStencilMask,
	A3D_GLID_glStencilOp,
	A3D_GLID_glTexCoordPointer,
	A3D_GLID_glTexEnvi,
	A3D_GLID_glTexEnvx,
	A3D_GLID_glTexEnviv,
	A3D_GLID_glTexEnvxv,
	A3D_GLID_glTexImage2D,
	A3D_GLID_glTexParameteri,
	A3D_GLID_glTexParameterx,
	A3D_GLID_glTexParameteriv,
	A3D_GLID_glTexParameterxv,
	A3D_GLID_glTexSubImage2D,
	A3D_GLID_glTranslatex,
	A3D_GLID_glVertexPointer,
	A3D_GLID_glViewport,

	/*------------------------------------------------------------------------*
	 * Required OES extension functions
	 *------------------------------------------------------------------------*/

	A3D_GLID_glPointSizePointerOES,

	A3D_GLID_MAX,
} a3d_glid_t;

/***********************************************************
* function stats                                           *
***********************************************************/

#include "a3d/a3d_time.h"

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
	/* Available only in Common profile */

	A3D_GLSTAT(glAlphaFunc)
	A3D_GLSTAT(glClearColor)
	A3D_GLSTAT(glClearDepthf)
	A3D_GLSTAT(glClipPlanef)
	A3D_GLSTAT(glColor4f)
	A3D_GLSTAT(glDepthRangef)
	A3D_GLSTAT(glFogf)
	A3D_GLSTAT(glFogfv)
	A3D_GLSTAT(glFrustumf)
	A3D_GLSTAT(glGetClipPlanef)
	A3D_GLSTAT(glGetFloatv)
	A3D_GLSTAT(glGetLightfv)
	A3D_GLSTAT(glGetMaterialfv)
	A3D_GLSTAT(glGetTexEnvfv)
	A3D_GLSTAT(glGetTexParameterfv)
	A3D_GLSTAT(glLightModelf)
	A3D_GLSTAT(glLightModelfv)
	A3D_GLSTAT(glLightf)
	A3D_GLSTAT(glLightfv)
	A3D_GLSTAT(glLineWidth)
	A3D_GLSTAT(glLoadMatrixf)
	A3D_GLSTAT(glMaterialf)
	A3D_GLSTAT(glMaterialfv)
	A3D_GLSTAT(glMultMatrixf)
	A3D_GLSTAT(glMultiTexCoord4f)
	A3D_GLSTAT(glNormal3f)
	A3D_GLSTAT(glOrthof)
	A3D_GLSTAT(glPointParameterf)
	A3D_GLSTAT(glPointParameterfv)
	A3D_GLSTAT(glPointSize)
	A3D_GLSTAT(glPolygonOffset)
	A3D_GLSTAT(glRotatef)
	A3D_GLSTAT(glScalef)
	A3D_GLSTAT(glTexEnvf)
	A3D_GLSTAT(glTexEnvfv)
	A3D_GLSTAT(glTexParameterf)
	A3D_GLSTAT(glTexParameterfv)
	A3D_GLSTAT(glTranslatef)

	/* Available in both Common and Common-Lite profiles */

	A3D_GLSTAT(glActiveTexture)
	A3D_GLSTAT(glAlphaFuncx)
	A3D_GLSTAT(glBindBuffer)
	A3D_GLSTAT(glBindTexture)
	A3D_GLSTAT(glBlendFunc)
	A3D_GLSTAT(glBufferData)
	A3D_GLSTAT(glBufferSubData)
	A3D_GLSTAT(glClear)
	A3D_GLSTAT(glClearColorx)
	A3D_GLSTAT(glClearDepthx)
	A3D_GLSTAT(glClearStencil)
	A3D_GLSTAT(glClientActiveTexture)
	A3D_GLSTAT(glClipPlanex)
	A3D_GLSTAT(glColor4ub)
	A3D_GLSTAT(glColor4x)
	A3D_GLSTAT(glColorMask)
	A3D_GLSTAT(glColorPointer)
	A3D_GLSTAT(glCompressedTexImage2D)
	A3D_GLSTAT(glCompressedTexSubImage2D)
	A3D_GLSTAT(glCopyTexImage2D)
	A3D_GLSTAT(glCopyTexSubImage2D)
	A3D_GLSTAT(glCullFace)
	A3D_GLSTAT(glDeleteBuffers)
	A3D_GLSTAT(glDeleteTextures)
	A3D_GLSTAT(glDepthFunc)
	A3D_GLSTAT(glDepthMask)
	A3D_GLSTAT(glDepthRangex)
	A3D_GLSTAT(glDisable)
	A3D_GLSTAT(glDisableClientState)
	A3D_GLSTAT(glDrawArrays)
	A3D_GLSTAT(glDrawElements)
	A3D_GLSTAT(glEnable)
	A3D_GLSTAT(glEnableClientState)
	A3D_GLSTAT(glFinish)
	A3D_GLSTAT(glFlush)
	A3D_GLSTAT(glFogx)
	A3D_GLSTAT(glFogxv)
	A3D_GLSTAT(glFrontFace)
	A3D_GLSTAT(glFrustumx)
	A3D_GLSTAT(glGetBooleanv)
	A3D_GLSTAT(glGetBufferParameteriv)
	A3D_GLSTAT(glGetClipPlanex)
	A3D_GLSTAT(glGenBuffers)
	A3D_GLSTAT(glGenTextures)
	A3D_GLSTAT(glGetError)
	A3D_GLSTAT(glGetFixedv)
	A3D_GLSTAT(glGetIntegerv)
	A3D_GLSTAT(glGetLightxv)
	A3D_GLSTAT(glGetMaterialxv)
	A3D_GLSTAT(glGetPointerv)
	A3D_GLSTAT(glGetString)
	A3D_GLSTAT(glGetTexEnviv)
	A3D_GLSTAT(glGetTexEnvxv)
	A3D_GLSTAT(glGetTexParameteriv)
	A3D_GLSTAT(glGetTexParameterxv)
	A3D_GLSTAT(glHint)
	A3D_GLSTAT(glIsBuffer)
	A3D_GLSTAT(glIsEnabled)
	A3D_GLSTAT(glIsTexture)
	A3D_GLSTAT(glLightModelx)
	A3D_GLSTAT(glLightModelxv)
	A3D_GLSTAT(glLightx)
	A3D_GLSTAT(glLightxv)
	A3D_GLSTAT(glLineWidthx)
	A3D_GLSTAT(glLoadIdentity)
	A3D_GLSTAT(glLoadMatrixx)
	A3D_GLSTAT(glLogicOp)
	A3D_GLSTAT(glMaterialx)
	A3D_GLSTAT(glMaterialxv)
	A3D_GLSTAT(glMatrixMode)
	A3D_GLSTAT(glMultMatrixx)
	A3D_GLSTAT(glMultiTexCoord4x)
	A3D_GLSTAT(glNormal3x)
	A3D_GLSTAT(glNormalPointer)
	A3D_GLSTAT(glOrthox)
	A3D_GLSTAT(glPixelStorei)
	A3D_GLSTAT(glPointParameterx)
	A3D_GLSTAT(glPointParameterxv)
	A3D_GLSTAT(glPointSizex)
	A3D_GLSTAT(glPolygonOffsetx)
	A3D_GLSTAT(glPopMatrix)
	A3D_GLSTAT(glPushMatrix)
	A3D_GLSTAT(glReadPixels)
	A3D_GLSTAT(glRotatex)
	A3D_GLSTAT(glSampleCoverage)
	A3D_GLSTAT(glSampleCoveragex)
	A3D_GLSTAT(glScalex)
	A3D_GLSTAT(glScissor)
	A3D_GLSTAT(glShadeModel)
	A3D_GLSTAT(glStencilFunc)
	A3D_GLSTAT(glStencilMask)
	A3D_GLSTAT(glStencilOp)
	A3D_GLSTAT(glTexCoordPointer)
	A3D_GLSTAT(glTexEnvi)
	A3D_GLSTAT(glTexEnvx)
	A3D_GLSTAT(glTexEnviv)
	A3D_GLSTAT(glTexEnvxv)
	A3D_GLSTAT(glTexImage2D)
	A3D_GLSTAT(glTexParameteri)
	A3D_GLSTAT(glTexParameterx)
	A3D_GLSTAT(glTexParameteriv)
	A3D_GLSTAT(glTexParameterxv)
	A3D_GLSTAT(glTexSubImage2D)
	A3D_GLSTAT(glTranslatex)
	A3D_GLSTAT(glVertexPointer)
	A3D_GLSTAT(glViewport)

	/*------------------------------------------------------------------------*
	 * Required OES extension functions
	 *------------------------------------------------------------------------*/

	A3D_GLSTAT(glPointSizePointerOES)
};

/***********************************************************
* implementation                                           *
***********************************************************/

#define A3D_GLVOIDFUNC(ret, f, args, params) \
	typedef ret (*cb_##f) args; \
	static cb_##f gl_##f = NULL; \
	GL_API ret GL_APIENTRY f args \
	{ \
		A3D_ENTER(f) \
		gl_##f params; \
		A3D_EXIT(f) \
	}

#define A3D_GLTYPEFUNC(ret, f, args, params) \
	typedef ret (*cb_##f) args; \
	static cb_##f gl_##f = NULL; \
	GL_API ret GL_APIENTRY f args \
	{ \
		A3D_ENTER(f) \
		ret r = gl_##f params; \
		A3D_EXIT(f) \
		return r; \
	}

/* Available only in Common profile */

A3D_GLVOIDFUNC(void, glAlphaFunc, (GLenum func, GLclampf ref), (func, ref))
A3D_GLVOIDFUNC(void, glClearColor, (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha), (red, green, blue, alpha))
A3D_GLVOIDFUNC(void, glClearDepthf, (GLclampf depth), (depth))
A3D_GLVOIDFUNC(void, glClipPlanef, (GLenum plane, const GLfloat *equation), (plane, equation))
A3D_GLVOIDFUNC(void, glColor4f, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha), (red, green, blue, alpha))
A3D_GLVOIDFUNC(void, glDepthRangef, (GLclampf zNear, GLclampf zFar), (zNear, zFar))
A3D_GLVOIDFUNC(void, glFogf, (GLenum pname, GLfloat param), (pname, param))
A3D_GLVOIDFUNC(void, glFogfv, (GLenum pname, const GLfloat *params), (pname, params))
A3D_GLVOIDFUNC(void, glFrustumf, (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar), (left, right, bottom, top, zNear, zFar))
A3D_GLVOIDFUNC(void, glGetClipPlanef, (GLenum pname, GLfloat eqn[4]), (pname, eqn))
A3D_GLVOIDFUNC(void, glGetFloatv, (GLenum pname, GLfloat *params), (pname, params))
A3D_GLVOIDFUNC(void, glGetLightfv, (GLenum light, GLenum pname, GLfloat *params), (light, pname, params))
A3D_GLVOIDFUNC(void, glGetMaterialfv, (GLenum face, GLenum pname, GLfloat *params), (face, pname, params))
A3D_GLVOIDFUNC(void, glGetTexEnvfv, (GLenum env, GLenum pname, GLfloat *params), (env, pname, params))
A3D_GLVOIDFUNC(void, glGetTexParameterfv, (GLenum target, GLenum pname, GLfloat *params), (target, pname, params))
A3D_GLVOIDFUNC(void, glLightModelf, (GLenum pname, GLfloat param), (pname, param))
A3D_GLVOIDFUNC(void, glLightModelfv, (GLenum pname, const GLfloat *params), (pname, params))
A3D_GLVOIDFUNC(void, glLightf, (GLenum light, GLenum pname, GLfloat param), (light, pname, param))
A3D_GLVOIDFUNC(void, glLightfv, (GLenum light, GLenum pname, const GLfloat *params), (light, pname, params))
A3D_GLVOIDFUNC(void, glLineWidth, (GLfloat width), (width))
A3D_GLVOIDFUNC(void, glLoadMatrixf, (const GLfloat *m), (m))
A3D_GLVOIDFUNC(void, glMaterialf, (GLenum face, GLenum pname, GLfloat param), (face, pname, param))
A3D_GLVOIDFUNC(void, glMaterialfv, (GLenum face, GLenum pname, const GLfloat *params), (face, pname, params))
A3D_GLVOIDFUNC(void, glMultMatrixf, (const GLfloat *m), (m))
A3D_GLVOIDFUNC(void, glMultiTexCoord4f, (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q), (target, s, t, r, q))
A3D_GLVOIDFUNC(void, glNormal3f, (GLfloat nx, GLfloat ny, GLfloat nz), (nx, ny, nz))
A3D_GLVOIDFUNC(void, glOrthof, (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar), (left, right, bottom, top, zNear, zFar))
A3D_GLVOIDFUNC(void, glPointParameterf, (GLenum pname, GLfloat param), (pname, param))
A3D_GLVOIDFUNC(void, glPointParameterfv, (GLenum pname, const GLfloat *params), (pname, params))
A3D_GLVOIDFUNC(void, glPointSize, (GLfloat size), (size))
A3D_GLVOIDFUNC(void, glPolygonOffset, (GLfloat factor, GLfloat units), (factor, units))
A3D_GLVOIDFUNC(void, glRotatef, (GLfloat angle, GLfloat x, GLfloat y, GLfloat z), (angle, x, y, z))
A3D_GLVOIDFUNC(void, glScalef, (GLfloat x, GLfloat y, GLfloat z), (x, y, z))
A3D_GLVOIDFUNC(void, glTexEnvf, (GLenum target, GLenum pname, GLfloat param), (target, pname, param))
A3D_GLVOIDFUNC(void, glTexEnvfv, (GLenum target, GLenum pname, const GLfloat *params), (target, pname, params))
A3D_GLVOIDFUNC(void, glTexParameterf, (GLenum target, GLenum pname, GLfloat param), (target, pname, param))
A3D_GLVOIDFUNC(void, glTexParameterfv, (GLenum target, GLenum pname, const GLfloat *params), (target, pname, params))
A3D_GLVOIDFUNC(void, glTranslatef, (GLfloat x, GLfloat y, GLfloat z), (x, y, z))

/* Available in both Common and Common-Lite profiles */

A3D_GLVOIDFUNC(void, glActiveTexture, (GLenum texture), (texture))
A3D_GLVOIDFUNC(void, glAlphaFuncx, (GLenum func, GLclampx ref), (func, ref))
A3D_GLVOIDFUNC(void, glBindBuffer, (GLenum target, GLuint buffer), (target, buffer))
A3D_GLVOIDFUNC(void, glBindTexture, (GLenum target, GLuint texture), (target, texture))
A3D_GLVOIDFUNC(void, glBlendFunc, (GLenum sfactor, GLenum dfactor), (sfactor, dfactor))
A3D_GLVOIDFUNC(void, glBufferData, (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage), (target, size, data, usage))
A3D_GLVOIDFUNC(void, glBufferSubData, (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data), (target, offset, size, data))
A3D_GLVOIDFUNC(void, glClear, (GLbitfield mask), (mask))
A3D_GLVOIDFUNC(void, glClearColorx, (GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha), (red, green, blue, alpha))
A3D_GLVOIDFUNC(void, glClearDepthx, (GLclampx depth), (depth))
A3D_GLVOIDFUNC(void, glClearStencil, (GLint s), (s))
A3D_GLVOIDFUNC(void, glClientActiveTexture, (GLenum texture), (texture))
A3D_GLVOIDFUNC(void, glClipPlanex, (GLenum plane, const GLfixed *equation), (plane, equation))
A3D_GLVOIDFUNC(void, glColor4ub, (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha), (red, green, blue, alpha))
A3D_GLVOIDFUNC(void, glColor4x, (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha), (red, green, blue, alpha))
A3D_GLVOIDFUNC(void, glColorMask, (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha), (red, green, blue, alpha))
A3D_GLVOIDFUNC(void, glColorPointer, (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer), (size, type, stride, pointer))
A3D_GLVOIDFUNC(void, glCompressedTexImage2D, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data), (target, level, internalformat, width, height, border, imageSize, data))
A3D_GLVOIDFUNC(void, glCompressedTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data), (target, level, xoffset, yoffset, width, height, format, imageSize, data))
A3D_GLVOIDFUNC(void, glCopyTexImage2D, (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border), (target, level, internalformat, x, y, width, height, border))
A3D_GLVOIDFUNC(void, glCopyTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height), (target, level, xoffset, yoffset, x, y, width, height))
A3D_GLVOIDFUNC(void, glCullFace, (GLenum mode), (mode))
A3D_GLVOIDFUNC(void, glDeleteBuffers, (GLsizei n, const GLuint *buffers), (n, buffers))
A3D_GLVOIDFUNC(void, glDeleteTextures, (GLsizei n, const GLuint *textures), (n, textures))
A3D_GLVOIDFUNC(void, glDepthFunc, (GLenum func), (func))
A3D_GLVOIDFUNC(void, glDepthMask, (GLboolean flag), (flag))
A3D_GLVOIDFUNC(void, glDepthRangex, (GLclampx zNear, GLclampx zFar), (zNear, zFar))
A3D_GLVOIDFUNC(void, glDisable, (GLenum cap), (cap))
A3D_GLVOIDFUNC(void, glDisableClientState, (GLenum array), (array))
A3D_GLVOIDFUNC(void, glDrawArrays, (GLenum mode, GLint first, GLsizei count), (mode, first, count))
A3D_GLVOIDFUNC(void, glDrawElements, (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices), (mode, count, type, indices))
A3D_GLVOIDFUNC(void, glEnable, (GLenum cap), (cap))
A3D_GLVOIDFUNC(void, glEnableClientState, (GLenum array), (array))
A3D_GLVOIDFUNC(void, glFinish, (void), ())
A3D_GLVOIDFUNC(void, glFlush, (void), ())
A3D_GLVOIDFUNC(void, glFogx, (GLenum pname, GLfixed param), (pname, param))
A3D_GLVOIDFUNC(void, glFogxv, (GLenum pname, const GLfixed *params), (pname, params))
A3D_GLVOIDFUNC(void, glFrontFace, (GLenum mode), (mode))
A3D_GLVOIDFUNC(void, glFrustumx, (GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar), (left, right, bottom, top, zNear, zFar))
A3D_GLVOIDFUNC(void, glGetBooleanv, (GLenum pname, GLboolean *params), (pname, params))
A3D_GLVOIDFUNC(void, glGetBufferParameteriv, (GLenum target, GLenum pname, GLint *params), (target, pname, params))
A3D_GLVOIDFUNC(void, glGetClipPlanex, (GLenum pname, GLfixed eqn[4]), (pname, eqn))
A3D_GLVOIDFUNC(void, glGenBuffers, (GLsizei n, GLuint *buffers), (n, buffers))
A3D_GLVOIDFUNC(void, glGenTextures, (GLsizei n, GLuint *textures), (n, textures))
A3D_GLTYPEFUNC(GLenum, glGetError, (void), ())
A3D_GLVOIDFUNC(void, glGetFixedv, (GLenum pname, GLfixed *params), (pname, params))
A3D_GLVOIDFUNC(void, glGetIntegerv, (GLenum pname, GLint *params), (pname, params))
A3D_GLVOIDFUNC(void, glGetLightxv, (GLenum light, GLenum pname, GLfixed *params), (light, pname, params))
A3D_GLVOIDFUNC(void, glGetMaterialxv, (GLenum face, GLenum pname, GLfixed *params), (face, pname, params))
A3D_GLVOIDFUNC(void, glGetPointerv, (GLenum pname, void **params), (pname, params))
A3D_GLTYPEFUNC(const GLubyte *, glGetString, (GLenum name), (name))
A3D_GLVOIDFUNC(void, glGetTexEnviv, (GLenum env, GLenum pname, GLint *params), (env, pname, params))
A3D_GLVOIDFUNC(void, glGetTexEnvxv, (GLenum env, GLenum pname, GLfixed *params), (env, pname, params))
A3D_GLVOIDFUNC(void, glGetTexParameteriv, (GLenum target, GLenum pname, GLint *params), (target, pname, params))
A3D_GLVOIDFUNC(void, glGetTexParameterxv, (GLenum target, GLenum pname, GLfixed *params), (target, pname, params))
A3D_GLVOIDFUNC(void, glHint, (GLenum target, GLenum mode), (target, mode))
A3D_GLTYPEFUNC(GLboolean, glIsBuffer, (GLuint buffer), (buffer))
A3D_GLTYPEFUNC(GLboolean, glIsEnabled, (GLenum cap), (cap))
A3D_GLTYPEFUNC(GLboolean, glIsTexture, (GLuint texture), (texture))
A3D_GLVOIDFUNC(void, glLightModelx, (GLenum pname, GLfixed param), (pname, param))
A3D_GLVOIDFUNC(void, glLightModelxv, (GLenum pname, const GLfixed *params), (pname, params))
A3D_GLVOIDFUNC(void, glLightx, (GLenum light, GLenum pname, GLfixed param), (light, pname, param))
A3D_GLVOIDFUNC(void, glLightxv, (GLenum light, GLenum pname, const GLfixed *params), (light, pname, params))
A3D_GLVOIDFUNC(void, glLineWidthx, (GLfixed width), (width))
A3D_GLVOIDFUNC(void, glLoadIdentity, (void), ())
A3D_GLVOIDFUNC(void, glLoadMatrixx, (const GLfixed *m), (m))
A3D_GLVOIDFUNC(void, glLogicOp, (GLenum opcode), (opcode))
A3D_GLVOIDFUNC(void, glMaterialx, (GLenum face, GLenum pname, GLfixed param), (face, pname, param))
A3D_GLVOIDFUNC(void, glMaterialxv, (GLenum face, GLenum pname, const GLfixed *params), (face, pname, params))
A3D_GLVOIDFUNC(void, glMatrixMode, (GLenum mode), (mode))
A3D_GLVOIDFUNC(void, glMultMatrixx, (const GLfixed *m), (m))
A3D_GLVOIDFUNC(void, glMultiTexCoord4x, (GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q), (target, s, t, r, q))
A3D_GLVOIDFUNC(void, glNormal3x, (GLfixed nx, GLfixed ny, GLfixed nz), (nx, ny, nz))
A3D_GLVOIDFUNC(void, glNormalPointer, (GLenum type, GLsizei stride, const GLvoid *pointer), (type, stride, pointer))
A3D_GLVOIDFUNC(void, glOrthox, (GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar), (left, right, bottom, top, zNear, zFar))
A3D_GLVOIDFUNC(void, glPixelStorei, (GLenum pname, GLint param), (pname, param))
A3D_GLVOIDFUNC(void, glPointParameterx, (GLenum pname, GLfixed param), (pname, param))
A3D_GLVOIDFUNC(void, glPointParameterxv, (GLenum pname, const GLfixed *params), (pname, params))
A3D_GLVOIDFUNC(void, glPointSizex, (GLfixed size), (size))
A3D_GLVOIDFUNC(void, glPolygonOffsetx, (GLfixed factor, GLfixed units), (factor, units))
A3D_GLVOIDFUNC(void, glPopMatrix, (void), ())
A3D_GLVOIDFUNC(void, glPushMatrix, (void), ())
A3D_GLVOIDFUNC(void, glReadPixels, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels), (x, y, width, height, format, type, pixels))
A3D_GLVOIDFUNC(void, glRotatex, (GLfixed angle, GLfixed x, GLfixed y, GLfixed z), (angle, x, y, z))
A3D_GLVOIDFUNC(void, glSampleCoverage, (GLclampf value, GLboolean invert), (value, invert))
A3D_GLVOIDFUNC(void, glSampleCoveragex, (GLclampx value, GLboolean invert), (value, invert))
A3D_GLVOIDFUNC(void, glScalex, (GLfixed x, GLfixed y, GLfixed z), (x, y, z))
A3D_GLVOIDFUNC(void, glScissor, (GLint x, GLint y, GLsizei width, GLsizei height), (x, y, width, height))
A3D_GLVOIDFUNC(void, glShadeModel, (GLenum mode), (mode))
A3D_GLVOIDFUNC(void, glStencilFunc, (GLenum func, GLint ref, GLuint mask), (func, ref, mask))
A3D_GLVOIDFUNC(void, glStencilMask, (GLuint mask), (mask))
A3D_GLVOIDFUNC(void, glStencilOp, (GLenum fail, GLenum zfail, GLenum zpass), (fail, zfail, zpass))
A3D_GLVOIDFUNC(void, glTexCoordPointer, (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer), (size, type, stride, pointer))
A3D_GLVOIDFUNC(void, glTexEnvi, (GLenum target, GLenum pname, GLint param), (target, pname, param))
A3D_GLVOIDFUNC(void, glTexEnvx, (GLenum target, GLenum pname, GLfixed param), (target, pname, param))
A3D_GLVOIDFUNC(void, glTexEnviv, (GLenum target, GLenum pname, const GLint *params), (target, pname, params))
A3D_GLVOIDFUNC(void, glTexEnvxv, (GLenum target, GLenum pname, const GLfixed *params), (target, pname, params))
A3D_GLVOIDFUNC(void, glTexImage2D, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels), (target, level, internalformat, width, height, border, format, type, pixels))
A3D_GLVOIDFUNC(void, glTexParameteri, (GLenum target, GLenum pname, GLint param), (target, pname, param))
A3D_GLVOIDFUNC(void, glTexParameterx, (GLenum target, GLenum pname, GLfixed param), (target, pname, param))
A3D_GLVOIDFUNC(void, glTexParameteriv, (GLenum target, GLenum pname, const GLint *params), (target, pname, params))
A3D_GLVOIDFUNC(void, glTexParameterxv, (GLenum target, GLenum pname, const GLfixed *params), (target, pname, params))
A3D_GLVOIDFUNC(void, glTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels), (target, level, xoffset, yoffset, width, height, format, type, pixels))
A3D_GLVOIDFUNC(void, glTranslatex, (GLfixed x, GLfixed y, GLfixed z), (x, y, z))
A3D_GLVOIDFUNC(void, glVertexPointer, (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer), (size, type, stride, pointer))
A3D_GLVOIDFUNC(void, glViewport, (GLint x, GLint y, GLsizei width, GLsizei height), (x, y, width, height))

/*------------------------------------------------------------------------*
 * Required OES extension functions
 *------------------------------------------------------------------------*/

A3D_GLVOIDFUNC(void, glPointSizePointerOES, (GLenum type, GLsizei stride, const GLvoid *pointer), (type, stride, pointer))

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

int a3d_GL_load(void)
{
	if(library != NULL)
	{
		LOGE("libGLESv1_CM.so already loaded");
		return 0;
	}

    library = dlopen("libGLESv1_CM.so", RTLD_NOW);
	if(library == NULL)
	{
		LOGE("dlopen failed");
		return 0;
	}

	/* Available only in Common profile */

	A3D_GLLOAD(glAlphaFunc)
	A3D_GLLOAD(glClearColor)
	A3D_GLLOAD(glClearDepthf)
	A3D_GLLOAD(glClipPlanef)
	A3D_GLLOAD(glColor4f)
	A3D_GLLOAD(glDepthRangef)
	A3D_GLLOAD(glFogf)
	A3D_GLLOAD(glFogfv)
	A3D_GLLOAD(glFrustumf)
	A3D_GLLOAD(glGetClipPlanef)
	A3D_GLLOAD(glGetFloatv)
	A3D_GLLOAD(glGetLightfv)
	A3D_GLLOAD(glGetMaterialfv)
	A3D_GLLOAD(glGetTexEnvfv)
	A3D_GLLOAD(glGetTexParameterfv)
	A3D_GLLOAD(glLightModelf)
	A3D_GLLOAD(glLightModelfv)
	A3D_GLLOAD(glLightf)
	A3D_GLLOAD(glLightfv)
	A3D_GLLOAD(glLineWidth)
	A3D_GLLOAD(glLoadMatrixf)
	A3D_GLLOAD(glMaterialf)
	A3D_GLLOAD(glMaterialfv)
	A3D_GLLOAD(glMultMatrixf)
	A3D_GLLOAD(glMultiTexCoord4f)
	A3D_GLLOAD(glNormal3f)
	A3D_GLLOAD(glOrthof)
	A3D_GLLOAD(glPointParameterf)
	A3D_GLLOAD(glPointParameterfv)
	A3D_GLLOAD(glPointSize)
	A3D_GLLOAD(glPolygonOffset)
	A3D_GLLOAD(glRotatef)
	A3D_GLLOAD(glScalef)
	A3D_GLLOAD(glTexEnvf)
	A3D_GLLOAD(glTexEnvfv)
	A3D_GLLOAD(glTexParameterf)
	A3D_GLLOAD(glTexParameterfv)
	A3D_GLLOAD(glTranslatef)

	/* Available in both Common and Common-Lite profiles */

	A3D_GLLOAD(glActiveTexture)
	A3D_GLLOAD(glAlphaFuncx)
	A3D_GLLOAD(glBindBuffer)
	A3D_GLLOAD(glBindTexture)
	A3D_GLLOAD(glBlendFunc)
	A3D_GLLOAD(glBufferData)
	A3D_GLLOAD(glBufferSubData)
	A3D_GLLOAD(glClear)
	A3D_GLLOAD(glClearColorx)
	A3D_GLLOAD(glClearDepthx)
	A3D_GLLOAD(glClearStencil)
	A3D_GLLOAD(glClientActiveTexture)
	A3D_GLLOAD(glClipPlanex)
	A3D_GLLOAD(glColor4ub)
	A3D_GLLOAD(glColor4x)
	A3D_GLLOAD(glColorMask)
	A3D_GLLOAD(glColorPointer)
	A3D_GLLOAD(glCompressedTexImage2D)
	A3D_GLLOAD(glCompressedTexSubImage2D)
	A3D_GLLOAD(glCopyTexImage2D)
	A3D_GLLOAD(glCopyTexSubImage2D)
	A3D_GLLOAD(glCullFace)
	A3D_GLLOAD(glDeleteBuffers)
	A3D_GLLOAD(glDeleteTextures)
	A3D_GLLOAD(glDepthFunc)
	A3D_GLLOAD(glDepthMask)
	A3D_GLLOAD(glDepthRangex)
	A3D_GLLOAD(glDisable)
	A3D_GLLOAD(glDisableClientState)
	A3D_GLLOAD(glDrawArrays)
	A3D_GLLOAD(glDrawElements)
	A3D_GLLOAD(glEnable)
	A3D_GLLOAD(glEnableClientState)
	A3D_GLLOAD(glFinish)
	A3D_GLLOAD(glFlush)
	A3D_GLLOAD(glFogx)
	A3D_GLLOAD(glFogxv)
	A3D_GLLOAD(glFrontFace)
	A3D_GLLOAD(glFrustumx)
	A3D_GLLOAD(glGetBooleanv)
	A3D_GLLOAD(glGetBufferParameteriv)
	A3D_GLLOAD(glGetClipPlanex)
	A3D_GLLOAD(glGenBuffers)
	A3D_GLLOAD(glGenTextures)
	A3D_GLLOAD(glGetError)
	A3D_GLLOAD(glGetFixedv)
	A3D_GLLOAD(glGetIntegerv)
	A3D_GLLOAD(glGetLightxv)
	A3D_GLLOAD(glGetMaterialxv)
	A3D_GLLOAD(glGetPointerv)
	A3D_GLLOAD(glGetString)
	A3D_GLLOAD(glGetTexEnviv)
	A3D_GLLOAD(glGetTexEnvxv)
	A3D_GLLOAD(glGetTexParameteriv)
	A3D_GLLOAD(glGetTexParameterxv)
	A3D_GLLOAD(glHint)
	A3D_GLLOAD(glIsBuffer)
	A3D_GLLOAD(glIsEnabled)
	A3D_GLLOAD(glIsTexture)
	A3D_GLLOAD(glLightModelx)
	A3D_GLLOAD(glLightModelxv)
	A3D_GLLOAD(glLightx)
	A3D_GLLOAD(glLightxv)
	A3D_GLLOAD(glLineWidthx)
	A3D_GLLOAD(glLoadIdentity)
	A3D_GLLOAD(glLoadMatrixx)
	A3D_GLLOAD(glLogicOp)
	A3D_GLLOAD(glMaterialx)
	A3D_GLLOAD(glMaterialxv)
	A3D_GLLOAD(glMatrixMode)
	A3D_GLLOAD(glMultMatrixx)
	A3D_GLLOAD(glMultiTexCoord4x)
	A3D_GLLOAD(glNormal3x)
	A3D_GLLOAD(glNormalPointer)
	A3D_GLLOAD(glOrthox)
	A3D_GLLOAD(glPixelStorei)
	A3D_GLLOAD(glPointParameterx)
	A3D_GLLOAD(glPointParameterxv)
	A3D_GLLOAD(glPointSizex)
	A3D_GLLOAD(glPolygonOffsetx)
	A3D_GLLOAD(glPopMatrix)
	A3D_GLLOAD(glPushMatrix)
	A3D_GLLOAD(glReadPixels)
	A3D_GLLOAD(glRotatex)
	A3D_GLLOAD(glSampleCoverage)
	A3D_GLLOAD(glSampleCoveragex)
	A3D_GLLOAD(glScalex)
	A3D_GLLOAD(glScissor)
	A3D_GLLOAD(glShadeModel)
	A3D_GLLOAD(glStencilFunc)
	A3D_GLLOAD(glStencilMask)
	A3D_GLLOAD(glStencilOp)
	A3D_GLLOAD(glTexCoordPointer)
	A3D_GLLOAD(glTexEnvi)
	A3D_GLLOAD(glTexEnvx)
	A3D_GLLOAD(glTexEnviv)
	A3D_GLLOAD(glTexEnvxv)
	A3D_GLLOAD(glTexImage2D)
	A3D_GLLOAD(glTexParameteri)
	A3D_GLLOAD(glTexParameterx)
	A3D_GLLOAD(glTexParameteriv)
	A3D_GLLOAD(glTexParameterxv)
	A3D_GLLOAD(glTexSubImage2D)
	A3D_GLLOAD(glTranslatex)
	A3D_GLLOAD(glVertexPointer)
	A3D_GLLOAD(glViewport)

	/*------------------------------------------------------------------------*
	 * Required OES extension functions
	 *------------------------------------------------------------------------*/

	A3D_GLLOAD(glPointSizePointerOES)

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
