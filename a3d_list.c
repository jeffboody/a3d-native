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

static void a3d_listitem_add(a3d_listitem_t* self,
                             a3d_list_t* list,
                             a3d_listitem_t* prev,
                             a3d_listitem_t* next)
{
	// prev and next can be NULL
	assert(self);
	assert(list);
	LOGD("debug");

	self->next = next;
	self->prev = prev;

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

	a3d_listnotify_fn add_fn = list->add_fn;
	if(add_fn)
	{
		(*add_fn)(list->owner, self);
	}
}

static void a3d_listitem_remove(a3d_listitem_t* self,
                                a3d_list_t* list)
{
	assert(self);
	assert(list);
	LOGD("debug");

	a3d_listnotify_fn del_fn = list->del_fn;
	if(del_fn)
	{
		(*del_fn)(list->owner, self);
	}

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

	self->next = NULL;
	self->prev = NULL;
}

static void a3d_listitem_move(a3d_listitem_t* self,
                              a3d_list_t* list,
                              a3d_listitem_t* prev,
                              a3d_listitem_t* next)
{
	// prev or next may be NULL but not both
	assert(self);
	assert(list);
	assert(prev || next);
	LOGD("debug");

	// remove node
	if(self->prev)
	{
		self->prev->next = self->next;
	}
	if(self->next)
	{
		self->next->prev = self->prev;
	}

	// update the list
	// use "else if" since size >= 2 for move
	if(self == list->head)
	{
		list->head = self->next;
	}
	else if(self == list->tail)
	{
		list->tail = self->prev;
	}

	// add node
	self->prev = prev;
	self->next = next;

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
	// use "else if" since size >= 2 for move
	if(prev == NULL)
	{
		list->head = self;
	}
	else if(next == NULL)
	{
		list->tail = self;
	}

	a3d_listnotify_fn mov_fn = list->mov_fn;
	if(mov_fn)
	{
		(*mov_fn)(list->owner, self);
	}
}

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

	self->next = NULL;
	self->prev = NULL;
	self->data = data;
	a3d_listitem_add(self, list, prev, next);
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

		next = self->next;
		data = self->data;
		a3d_listitem_remove(self, list);
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

	self->size   = 0;
	self->head   = NULL;
	self->tail   = NULL;
	self->owner  = NULL;
	self->add_fn = NULL;
	self->del_fn = NULL;
	self->mov_fn = NULL;

	return self;
}

void a3d_list_delete(a3d_list_t** _self)
{
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

void a3d_list_discard(a3d_list_t* self)
{
	assert(self);

	// discard all items in the list without freeing
	// this is useful when the list just holds references
	a3d_listitem_t* iter = a3d_list_head(self);
	while(iter)
	{
		a3d_list_remove(self, &iter);
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

int a3d_list_empty(a3d_list_t* self)
{
	assert(self);
	LOGD("debug");

	return self->size == 0;
}

const void* a3d_list_peekhead(a3d_list_t* self)
{
	assert(self);
	LOGD("debug");

	return (self->size == 0) ? NULL : self->head->data;
}

const void* a3d_list_peektail(a3d_list_t* self)
{
	assert(self);
	LOGD("debug");

	return (self->size == 0) ? NULL : self->tail->data;
}

const void* a3d_list_peekitem(a3d_listitem_t* item)
{
	assert(item);
	LOGD("debug");

	return item->data;
}

a3d_listitem_t* a3d_list_head(a3d_list_t* self)
{
	assert(self);
	LOGD("debug");

	return self->head;
}

a3d_listitem_t* a3d_list_tail(a3d_list_t* self)
{
	assert(self);
	LOGD("debug");

	return self->tail;
}

a3d_listitem_t* a3d_list_next(a3d_listitem_t* item)
{
	assert(item);
	LOGD("debug");

	return item->next;
}

a3d_listitem_t* a3d_list_prev(a3d_listitem_t* item)
{
	assert(item);
	LOGD("debug");

	return item->prev;
}

a3d_listitem_t* a3d_list_find(a3d_list_t* self,
                              const void* data,
                              a3d_listcmp_fn compare)
{
	// data can be NULL
	assert(self);
	assert(compare);
	LOGD("debug");

	a3d_listitem_t* item = self->head;
	while(item)
	{
		if((*compare)(item->data, data) == 0)
		{
			break;
		}
		item = item->next;
	}
	return item;
}

a3d_listitem_t* a3d_list_findn(a3d_list_t* self,
                               const void* data,
                               a3d_listcmp_fn compare)
{
	// data can be NULL
	assert(self);
	assert(compare);
	LOGD("debug");

	a3d_listitem_t* item = self->tail;
	while(item)
	{
		if((*compare)(item->data, data) == 0)
		{
			break;
		}
		item = item->prev;
	}
	return item;
}

a3d_listitem_t* a3d_list_insert(a3d_list_t* self,
                                a3d_listitem_t* item,
                                const void* data)
{
	// data can be null
	// item may be null for empty list or to insert at head
	// a3d_list_insert(list, NULL, data) may be preferred over
	// a3d_list_push(list, data) when a listitem is needed
	assert(self);
	LOGD("debug");

	if(item)
	{
		return a3d_listitem_new(self, item->prev, item, data);
	}
	else
	{
		return a3d_listitem_new(self, NULL, self->head, data);
	}
}

a3d_listitem_t* a3d_list_append(a3d_list_t* self,
                                a3d_listitem_t* item,
                                const void* data)
{
	// data can be null
	// item may be null for empty list or to append at tail
	// a3d_list_append(list, NULL, data) may be preferred over
	// a3d_list_enqueue(list, data) when a listitem is needed
	assert(self);
	LOGD("debug");

	if(item)
	{
		return a3d_listitem_new(self, item, item->next, data);
	}
	else
	{
		return a3d_listitem_new(self, self->tail, NULL, data);
	}
}

const void* a3d_list_replace(a3d_list_t* self,
                             a3d_listitem_t* item,
                             const void* data)
{
	// data can be null
	assert(self);
	assert(item);
	LOGD("debug");

	a3d_listnotify_fn mov_fn = self->mov_fn;
	if(mov_fn)
	{
		(*mov_fn)(self->owner, item);
	}

	const void* tmp = item->data;
	item->data = data;

	return tmp;
}

const void* a3d_list_remove(a3d_list_t* self,
                            a3d_listitem_t** _item)
{
	// _item can be NULL
	assert(self);
	LOGD("debug");

	return a3d_listitem_delete(_item, self);
}

void a3d_list_move(a3d_list_t* self,
                   a3d_listitem_t* from,
                   a3d_listitem_t* to)
{
	// to may be NULL
	assert(self);
	assert(from);
	LOGD("debug");

	if(to == NULL)
	{
		to = a3d_list_head(self);
	}

	if(from == to)
	{
		return;
	}

	a3d_listitem_move(from, self, to->prev, to);
}

void a3d_list_moven(a3d_list_t* self,
                    a3d_listitem_t* from,
                    a3d_listitem_t* to)
{
	// to may be NULL
	assert(self);
	assert(from);
	LOGD("debug");

	if(to == NULL)
	{
		to = a3d_list_tail(self);
	}

	if(from == to)
	{
		return;
	}

	a3d_listitem_move(from, self, to, to->next);
}

void a3d_list_swap(a3d_list_t* fromList,
                   a3d_list_t* toList,
                   a3d_listitem_t* from,
                   a3d_listitem_t* to)
{
	// to may be NULL
	assert(fromList);
	assert(toList);
	assert(from);
	LOGD("debug");

	if(fromList == toList)
	{
		a3d_list_move(fromList,
		              from,
		              to);
		return;
	}

	a3d_listitem_remove(from, fromList);
	if(to == NULL)
	{
		a3d_listitem_t* head = a3d_list_head(toList);
		a3d_listitem_add(from, toList, NULL, head);
	}
	else
	{
		a3d_listitem_add(from, toList, to->prev, to);
	}
}

void a3d_list_swapn(a3d_list_t* fromList,
                    a3d_list_t* toList,
                    a3d_listitem_t* from,
                    a3d_listitem_t* to)
{
	// to may be NULL
	assert(fromList);
	assert(toList);
	assert(from);
	LOGD("debug");

	if(fromList == toList)
	{
		a3d_list_moven(fromList,
		               from,
		               to);
		return;
	}

	a3d_listitem_remove(from, fromList);
	if(to == NULL)
	{
		a3d_listitem_t* tail = a3d_list_tail(toList);
		a3d_listitem_add(from, toList, tail, NULL);
	}
	else
	{
		a3d_listitem_add(from, toList, to, to->next);
	}
}

void a3d_list_notify(a3d_list_t* self,
                     void* owner,
                     a3d_listnotify_fn add_fn,
                     a3d_listnotify_fn del_fn,
                     a3d_listnotify_fn mov_fn)
{
	// owner, add_fn, del_fn, mov_fn may be NULL
	assert(self);
	LOGD("debug");

	// optionally notify the owner when a list item is added,
	// deleted or moved. For example, a listbox widget may want
	// to know if a new widget has been added in order to mark
	// the draw state dirty.
	self->owner  = owner;
	self->add_fn = add_fn;
	self->del_fn = del_fn;
	self->mov_fn = mov_fn;
}
