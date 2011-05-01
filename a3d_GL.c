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

#include "a3d_GL.h"
#include "a3d_log.h"

GLenum a3d_GL_geterror(const char* func, int line, const char* tag)
{
	GLenum e = glGetError();
	if(e == GL_NO_ERROR)
		;   // do nothing
	else if(e == GL_INVALID_ENUM)
		a3d_log(func, line, ANDROID_LOG_ERROR, tag, "GL error is GL_INVALID_ENUM");
	else if(e == GL_INVALID_VALUE)
		a3d_log(func, line, ANDROID_LOG_ERROR, tag, "GL error is GL_INVALID_VALUE");
	else if(e == GL_INVALID_OPERATION)
		a3d_log(func, line, ANDROID_LOG_ERROR, tag, "GL error is GL_INVALID_OPERATION");
	#ifdef A3D_GLESv1_CM
		else if(e == GL_STACK_OVERFLOW)
			a3d_log(func, line, ANDROID_LOG_ERROR, tag, "GL error is GL_STACK_OVERFLOW");
		else if(e == GL_STACK_UNDERFLOW)
			a3d_log(func, line, ANDROID_LOG_ERROR, tag, "GL error is GL_STACK_UNDERFLOW");
	#endif
	else if(e == GL_OUT_OF_MEMORY)
		a3d_log(func, line, ANDROID_LOG_ERROR, tag, "GL error is GL_OUT_OF_MEMORY");
	else
		a3d_log(func, line, ANDROID_LOG_ERROR, tag, "GL error is 0X%x", (int) e);

	return e;
}
