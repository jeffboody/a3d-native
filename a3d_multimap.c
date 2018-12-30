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

#include "a3d_multimap.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define LOG_TAG "a3d"
#include "a3d_log.h"

/***********************************************************
* public                                                   *
***********************************************************/

a3d_multimap_t* a3d_multimap_new(a3d_listcmp_fn compare)
{
	// compare may be NULL

	a3d_multimap_t* self = (a3d_multimap_t*)
	                       malloc(sizeof(a3d_multimap_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->hash = a3d_hashmap_new();
	if(self->hash == NULL)
	{
		goto fail_hash;
	}

	self->compare = compare;

	// success
	return self;

	// failure
	fail_hash:
		free(self);
	return NULL;
}

void a3d_multimap_delete(a3d_multimap_t** _self)
{
	assert(_self);

	a3d_multimap_t* self = *_self;
	if(self)
	{
		a3d_hashmap_delete(&self->hash);
		free(self);
		*_self = NULL;
	}
}

void a3d_multimap_discard(a3d_multimap_t* self)
{
	assert(self);

	a3d_hashmap_discard(self->hash);
}

int a3d_multimap_size(const a3d_multimap_t* self)
{
	assert(self);

	return a3d_hashmap_size(self->hash);
}

int a3d_multimap_multimapNodes(const a3d_multimap_t* self)
{
	assert(self);

	return a3d_hashmap_hashmapNodes(self->hash);
}

int a3d_multimap_multimapSize(const a3d_multimap_t* self)
{
	assert(self);

	return a3d_hashmap_hashmapSize(self->hash);
}

int a3d_multimap_empty(const a3d_multimap_t* self)
{
	assert(self);

	return a3d_hashmap_empty(self->hash);
}

a3d_multimapIter_t*
a3d_multimap_head(const a3d_multimap_t* self,
                  a3d_multimapIter_t* iter)
{
	assert(self);
	assert(iter);

	iter->hiter = a3d_hashmap_head(self->hash, &iter->hiterator);
	if(iter->hiter == NULL)
	{
		return NULL;
	}

	a3d_list_t* list = (a3d_list_t*)
	                   a3d_hashmap_val(iter->hiter);
	iter->item = a3d_list_head(list);

	return iter;
}

a3d_multimapIter_t* a3d_multimap_next(a3d_multimapIter_t* iter)
{
	assert(iter);

	iter->item = a3d_list_next(iter->item);
	if(iter->item)
	{
		return iter;
	}

	iter->hiter = a3d_hashmap_next(iter->hiter);
	if(iter->hiter == NULL)
	{
		return NULL;
	}

	a3d_list_t* list = (a3d_list_t*)
	                   a3d_hashmap_val(iter->hiter);
	iter->item = a3d_list_head(list);

	return iter;
}

a3d_multimapIter_t* a3d_multimap_nextItem(a3d_multimapIter_t* iter)
{
	assert(iter);

	iter->item = a3d_list_next(iter->item);
	if(iter->item)
	{
		return iter;
	}

	return NULL;
}

const void* a3d_multimap_val(const a3d_multimapIter_t* iter)
{
	assert(iter);

	return a3d_list_peekitem(iter->item);
}

const char* a3d_multimap_key(const a3d_multimapIter_t* iter)
{
	assert(iter);

	return a3d_hashmap_key(iter->hiter);
}

const a3d_list_t* a3d_multimap_find(const a3d_multimap_t* self,
                                    a3d_multimapIter_t* iter,
                                    const char* key)
{
	assert(self);
	assert(iter);
	assert(key);

	iter->hiter = &iter->hiterator;

	a3d_list_t* list;
	list = (a3d_list_t*)
	       a3d_hashmap_find(self->hash, iter->hiter, key);
	if(list == NULL)
	{
		return NULL;
	}

	iter->item = a3d_list_head(list);

	return list;
}

const a3d_list_t* a3d_multimap_findf(const a3d_multimap_t* self,
                                     a3d_multimapIter_t* iter,
                                     const char* fmt, ...)
{
	assert(self);
	assert(iter);
	assert(fmt);

	char key[256];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(key, 256, fmt, argptr);
	va_end(argptr);

	return a3d_multimap_find(self, iter, key);
}

int a3d_multimap_add(a3d_multimap_t* self,
                     const void* val,
                     const char* key)
{
	assert(self);
	assert(val);
	assert(key);

	a3d_listitem_t* item;

	// check if the list already exists
	a3d_hashmapIter_t iter;
	a3d_list_t* list;
	list = (a3d_list_t*)
	       a3d_hashmap_find(self->hash,
	                        &iter, key);
	if(list && self->compare)
	{
		item = a3d_list_insertSorted(list,
		                             self->compare,
		                             val);
		if(item == NULL)
		{
			return 0;
		}

		return 1;
	}
	else if(list)
	{
		item = a3d_list_append(list, NULL, val);
		if(item == NULL)
		{
			return 0;
		}

		return 1;
	}

	// create a new list and add to hash
	list = a3d_list_new();
	if(list == NULL)
	{
		return 0;
	}

	item = a3d_list_append(list, NULL, val);
	if(item == NULL)
	{
		goto fail_append;
	}

	if(a3d_hashmap_add(self->hash,
	                   (const void*) list,
	                   key) == 0)
	{
		goto fail_add;
	}

	// success
	return 1;

	// failure
	fail_add:
		a3d_list_remove(list, &item);
	fail_append:
		a3d_list_delete(&list);
	return 0;
}

int a3d_multimap_addf(a3d_multimap_t* self,
                      const void* val,
                      const char* fmt, ...)
{
	assert(self);
	assert(val);
	assert(fmt);

	char key[256];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(key, 256, fmt, argptr);
	va_end(argptr);

	return a3d_multimap_add(self, val, key);
}

const void* a3d_multimap_replace(a3d_multimapIter_t* iter,
                                 const void*  val)
{
	assert(iter);
	assert(val);

	a3d_list_t* list;
	list = (a3d_list_t*)
	       a3d_hashmap_val(iter->hiter);
	return a3d_list_replace(list, iter->item, val);
}

const void* a3d_multimap_remove(a3d_multimap_t* self,
                                a3d_multimapIter_t** _iter)
{
	assert(self);
	assert(_iter);
	assert(*_iter);

	a3d_multimapIter_t* iter = *_iter;

	// remove item from list;
	a3d_list_t* list;
	list = (a3d_list_t*)
	       a3d_hashmap_val(iter->hiter);
	const void* data = a3d_list_remove(list,
	                                   &iter->item);

	// check if list is empty
	// or if next item is NULL
	if(a3d_list_empty(list))
	{
		a3d_hashmap_remove(self->hash, &iter->hiter);
		a3d_list_delete(&list);
		if(iter->hiter)
		{
			list = (a3d_list_t*)
			       a3d_hashmap_val(iter->hiter);
			iter->item = a3d_list_head(list);
		}
	}
	else if(iter->item == NULL)
	{
		iter->hiter = a3d_hashmap_next(iter->hiter);
		if(iter->hiter)
		{
			list = (a3d_list_t*)
			       a3d_hashmap_val(iter->hiter);
			iter->item = a3d_list_head(list);
		}
	}

	// check for iteration end
	if(iter->hiter == NULL)
	{
		*_iter = NULL;
	}

	return data;
}
