/*
 * Copyright (c) 2013 Jeff Boody
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

#ifndef a3d_quaternion_H
#define a3d_quaternion_H

#include "../a3d_GL.h"
#include "a3d_vec3f.h"

typedef struct
{
	a3d_vec3f_t v;
	GLfloat     s;
} a3d_quaternion_t;

// quaternion operations
// enforce special case of unit quaternions to be used for rotations
// http://content.gpwiki.org/index.php/OpenGL:Tutorials:Using_Quaternions_to_represent_rotation
// http://www.flipcode.com/documents/matrfaq.html
void a3d_quaternion_load(a3d_quaternion_t* self,
                         GLfloat x, GLfloat y, GLfloat z, GLfloat s);
void a3d_quaternion_loadaxis(a3d_quaternion_t* self,
                             GLfloat a, GLfloat x, GLfloat y, GLfloat z);
void a3d_quaternion_loadeuler(a3d_quaternion_t* self,
                              GLfloat rx, GLfloat ry, GLfloat rz);
void a3d_quaternion_rotateq(a3d_quaternion_t* self,
                            const a3d_quaternion_t* q);
void a3d_quaternion_rotateq_copy(const a3d_quaternion_t* self,
                                 const a3d_quaternion_t* q,
                                 a3d_quaternion_t* copy);

#endif
