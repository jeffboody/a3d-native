/*
 * Copyright (c) 2010 Jeff Boody
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

#ifndef a3d_GL_H
#define a3d_GL_H

#if defined(A3D_GLESv1_CM_TRACE)
	#define A3D_GLESv1_CM
#elif defined(A3D_GLESv2_TRACE)
	#define A3D_GLESv2
#endif

#ifdef __APPLE__
	#include <SDL2/SDL_opengl.h>
	#define A3D_GL2
#elif defined(A3D_GLESv1_CM)
	#include <GLES/gl.h>
#elif defined(A3D_GLESv2_LOAX)
	#define A3D_GLESv2
	#include <loax/gl2.h>
#elif defined(A3D_GLESv2_RASPI)
	#define A3D_GLESv2
	#include "GLES2/gl2.h"
#elif defined(A3D_GLESv2)
	#include <GLES2/gl2.h>
#elif defined(A3D_GL2)
	#include <GL/glew.h>
	#include <SDL2/SDL_opengl.h>
#endif

/***********************************************************
* utility functions                                        *
***********************************************************/

GLenum a3d_GL_geterror(const char* func, int line, const char* tag);
#define A3D_GL_GETERROR() (a3d_GL_geterror(__func__, __LINE__, LOG_TAG))

/***********************************************************
* control functions                                        *
***********************************************************/

#if defined(A3D_GLESv1_CM) || defined(A3D_GLESv2)
	int  a3d_GL_load(void);
	int  a3d_GL_unload(void);
	void a3d_GL_frame_begin(void);
	void a3d_GL_frame_end(void);
#endif

#endif
