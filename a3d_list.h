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

typedef struct a3d_listitem_s
{
	struct a3d_listitem_s* next;
	struct a3d_listitem_s* prev;
	const void*            data;
} a3d_listitem_t;

typedef struct
{
	int size;
	a3d_listitem_t* head;
	a3d_listitem_t* tail;
} a3d_list_t;

a3d_list_t* a3d_list_new(void);
void        a3d_list_delete(a3d_list_t** _self);
int         a3d_list_push(a3d_list_t* self, const void* data); // stack
const void* a3d_list_pop(a3d_list_t* self);
int         a3d_list_enqueue(a3d_list_t* self, const void* data); // queue
const void* a3d_list_dequeue(a3d_list_t* self);
int         a3d_list_size(a3d_list_t* self);

#endif
