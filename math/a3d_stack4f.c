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

#include "a3d_stack4f.h"
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#define LOG_TAG "a3d"
#include "../a3d_log.h"

/***********************************************************
* public                                                   *
***********************************************************/

a3d_stack4f_t* a3d_stack4f_new(void)
{
	LOGD("debug");

	a3d_stack4f_t* self = (a3d_stack4f_t*) malloc(sizeof(a3d_stack4f_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->matrix_stack = a3d_list_new();
	if(self->matrix_stack == NULL)
		goto fail_matrix_stack;

	// success
	return self;

	// failure
	fail_matrix_stack:
		free(self);
	return NULL;
}

void a3d_stack4f_delete(a3d_stack4f_t** _self)
{
	// *_self can be null
	assert(_self);

	a3d_stack4f_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		a3d_mat4f_t* m = (a3d_mat4f_t*) a3d_list_pop(self->matrix_stack);
		while(m)
		{
			free(m);
			m = (a3d_mat4f_t*) a3d_list_pop(self->matrix_stack);
		}
		a3d_list_delete(&self->matrix_stack);
		free(self);
		*_self = NULL;
	}
}

void a3d_stack4f_push(a3d_stack4f_t* self, const a3d_mat4f_t* m)
{
	assert(self);
	assert(m);
	LOGD("debug");

	a3d_mat4f_t* c = (a3d_mat4f_t*) malloc(sizeof(a3d_mat4f_t));
	if(c == NULL)
	{
		LOGE("malloc failed");
		return;
	}
	a3d_mat4f_copy(m, c);
	a3d_list_push(self->matrix_stack, (const void*) c);
}

void a3d_stack4f_pop(a3d_stack4f_t* self, a3d_mat4f_t* m)
{
	assert(self);
	assert(m);
	LOGD("debug");

	a3d_mat4f_t* c = (a3d_mat4f_t*) a3d_list_pop(self->matrix_stack);
	if(c == NULL)
		return;
	a3d_mat4f_copy(c, m);
	free(c);
}
