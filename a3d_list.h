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

#ifndef a3d_list_H
#define a3d_list_H

typedef int (*a3d_listcmp_fn)(const void* a, const void* b);

typedef struct a3d_listitem_s
{
	struct a3d_listitem_s* next;
	struct a3d_listitem_s* prev;
	const void*            data;
} a3d_listitem_t;

typedef void (*a3d_listnotify_fn)(void* owner, a3d_listitem_t* item);

typedef struct
{
	int size;
	a3d_listitem_t* head;
	a3d_listitem_t* tail;

	// notify callbacks
	void*             owner;
	a3d_listnotify_fn add_fn;
	a3d_listnotify_fn del_fn;
	a3d_listnotify_fn mov_fn;
} a3d_list_t;

a3d_list_t*     a3d_list_new(void);
void            a3d_list_delete(a3d_list_t** _self);
void            a3d_list_discard(a3d_list_t* self);
int             a3d_list_push(a3d_list_t* self,
                              const void* data);
const void*     a3d_list_pop(a3d_list_t* self);
int             a3d_list_enqueue(a3d_list_t* self,
                                 const void* data);
const void*     a3d_list_dequeue(a3d_list_t* self);
int             a3d_list_size(a3d_list_t* self);
int             a3d_list_empty(a3d_list_t* self);
const void*     a3d_list_peekhead(a3d_list_t* self);
const void*     a3d_list_peektail(a3d_list_t* self);
const void*     a3d_list_peekitem(a3d_listitem_t* item);
a3d_listitem_t* a3d_list_head(a3d_list_t* self);
a3d_listitem_t* a3d_list_tail(a3d_list_t* self);
a3d_listitem_t* a3d_list_next(a3d_listitem_t* item);
a3d_listitem_t* a3d_list_prev(a3d_listitem_t* item);
a3d_listitem_t* a3d_list_find(a3d_list_t* self,
                              const void* data,
                              a3d_listcmp_fn compare);
a3d_listitem_t* a3d_list_findn(a3d_list_t* self,
                               const void* data,
                               a3d_listcmp_fn compare);
a3d_listitem_t* a3d_list_insert(a3d_list_t* self,
                                a3d_listitem_t* item,
                                const void* data);
a3d_listitem_t* a3d_list_append(a3d_list_t* self,
                                a3d_listitem_t* item,
                                const void* data);
const void*     a3d_list_replace(a3d_list_t* self,
                                a3d_listitem_t* item,
                                const void* data);
const void*     a3d_list_remove(a3d_list_t* self,
                                a3d_listitem_t** _item);
void            a3d_list_move(a3d_list_t* self,
                              a3d_listitem_t* from,
                              a3d_listitem_t* to);
void            a3d_list_moven(a3d_list_t* self,
                               a3d_listitem_t* from,
                               a3d_listitem_t* to);
void            a3d_list_swap(a3d_list_t* fromList,
                              a3d_list_t* toList,
                              a3d_listitem_t* from,
                              a3d_listitem_t* to);
void            a3d_list_swapn(a3d_list_t* fromList,
                               a3d_list_t* toList,
                               a3d_listitem_t* from,
                               a3d_listitem_t* to);
void            a3d_list_notify(a3d_list_t* self,
                                void* owner,
                                a3d_listnotify_fn add_fn,
                                a3d_listnotify_fn del_fn,
                                a3d_listnotify_fn mov_fn);

#endif
