/*
 * Copyright (c) 2011 Jeff Boody
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

#include "a3d_list.h"
#include <stdlib.h>
#include <assert.h>

#define LOG_TAG "a3d"
#include "a3d_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static a3d_listitem_t* a3d_listitem_new(a3d_list_t* list,
                                        a3d_listitem_t* prev,
                                        a3d_listitem_t* next,
                                        const void* data)
{
	// prev, next and data can be NULL
	assert(list);
	LOGD("debug");

	a3d_listitem_t* self = (a3d_listitem_t*) malloc(sizeof(a3d_listitem_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->next = next;
	self->prev = prev;
	self->data = data;

	// update next/prev nodes
	if(next)
	{
		next->prev = self;
	}
	if(prev)
	{
		prev->next = self;
	}

	// update the list
	if(prev == NULL)
	{
		list->head = self;
	}
	if(next == NULL)
	{
		list->tail = self;
	}
	++list->size;

	return self;
}

static const void* a3d_listitem_delete(a3d_listitem_t** _self,
                                       a3d_list_t* list)
{
	assert(_self);
	assert(list);

	a3d_listitem_t* self = *_self;
	a3d_listitem_t* next = NULL;
	const void*     data = NULL;
	if(self)
	{
		LOGD("debug");

		// update next/prev nodes
		if(self->prev)
		{
			self->prev->next = self->next;
		}
		if(self->next)
		{
			self->next->prev = self->prev;
		}

		// update the list
		if(self == list->head)
		{
			list->head = self->next;
		}
		if(self == list->tail)
		{
			list->tail = self->prev;
		}
		--list->size;

		next = self->next;
		data = self->data;
		free(self);
		*_self = next;
	}

	return data;
}

/***********************************************************
* public                                                   *
***********************************************************/

a3d_list_t* a3d_list_new(void)
{
	LOGD("debug");

	a3d_list_t* self = (a3d_list_t*) malloc(sizeof(a3d_list_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->size = 0;
	self->head = NULL;
	self->tail = NULL;

	return self;
}

void a3d_list_delete(a3d_list_t** _self)
{
	// *_self can be NULL
	assert(_self);

	a3d_list_t* self = *_self;
	if(self)
	{
		LOGD("debug");

		if(self->size > 0)
		{
			LOGE("memory leak detected: size=%i", self->size);
		}

		while(self->size > 0)
		{
			a3d_list_pop(self);
		}

		free(self);
		*_self = NULL;
	}
}

int a3d_list_push(a3d_list_t* self, const void* data)
{
	// data can be null
	assert(self);
	LOGD("debug");

	a3d_listitem_t* item = a3d_listitem_new(self, NULL, self->head, data);
	return (item == NULL) ? 0 : 1;
}

const void* a3d_list_pop(a3d_list_t* self)
{
	assert(self);
	LOGD("debug");

	return a3d_listitem_delete(&self->head, self);
}

int a3d_list_enqueue(a3d_list_t* self, const void* data)
{
	// data can be null
	assert(self);
	LOGD("debug");

	a3d_listitem_t* item = a3d_listitem_new(self, self->tail, NULL, data);
	return (item == NULL) ? 0 : 1;
}

const void* a3d_list_dequeue(a3d_list_t* self)
{
	assert(self);
	LOGD("debug");

	return a3d_list_pop(self);
}

int a3d_list_size(a3d_list_t* self)
{
	assert(self);
	LOGD("debug");
	return self->size;
}
