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

#ifndef a3d_multimap_H
#define a3d_multimap_H

#include "a3d_hashmap.h"
#include "a3d_list.h"

typedef struct
{
	a3d_hashmapIter_t  hiterator;
	a3d_hashmapIter_t* hiter;
	a3d_listitem_t*    item;
} a3d_multimapIter_t;

typedef struct
{
	a3d_hashmap_t* hash;
	a3d_listcmp_fn compare;
} a3d_multimap_t;

a3d_multimap_t*     a3d_multimap_new(a3d_listcmp_fn compare);
void                a3d_multimap_delete(a3d_multimap_t** _self);
void                a3d_multimap_discard(a3d_multimap_t* self);
int                 a3d_multimap_size(const a3d_multimap_t* self);
int                 a3d_multimap_multimapNodes(const a3d_multimap_t* self);
int                 a3d_multimap_multimapSize(const a3d_multimap_t* self);
int                 a3d_multimap_empty(const a3d_multimap_t* self);
a3d_multimapIter_t* a3d_multimap_head(const a3d_multimap_t* self,
                                      a3d_multimapIter_t* iter);
a3d_multimapIter_t* a3d_multimap_next(a3d_multimapIter_t* iter);
a3d_multimapIter_t* a3d_multimap_nextItem(a3d_multimapIter_t* iter);
a3d_multimapIter_t* a3d_multimap_nextList(a3d_multimapIter_t* iter);
const void*         a3d_multimap_val(const a3d_multimapIter_t* iter);
const a3d_list_t*   a3d_multimap_list(const a3d_multimapIter_t* iter);
const char*         a3d_multimap_key(const a3d_multimapIter_t* iter);
const a3d_list_t*   a3d_multimap_find(const a3d_multimap_t* self,
                                      a3d_multimapIter_t* iter,
                                      const char* key);
const a3d_list_t*   a3d_multimap_findf(const a3d_multimap_t* self,
                                       a3d_multimapIter_t* iter,
                                       const char* fmt, ...);
int                 a3d_multimap_add(a3d_multimap_t* self,
                                     const void* val,
                                     const char* key);
int                 a3d_multimap_addf(a3d_multimap_t* self,
                                      const void* val,
                                      const char* fmt, ...);
const void*         a3d_multimap_replace(a3d_multimapIter_t* iter,
                                         const void* val);
const void*         a3d_multimap_remove(a3d_multimap_t* self,
                                        a3d_multimapIter_t** _iter);

#endif
