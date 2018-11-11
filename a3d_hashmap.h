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

#ifndef a3d_hashmap_H
#define a3d_hashmap_H

struct a3d_hashmap_s;

#define A3D_HASHMAP_KEY_LEN   256
#define A3D_HASHMAP_KEY_COUNT 36

typedef struct
{
	int  key_len;
	char key[A3D_HASHMAP_KEY_LEN];

	struct a3d_hashmapNode_s* node;
} a3d_hashmapIter_t;

typedef struct a3d_hashmapNode_s
{
	// nodes are allocated on demand
	// and freed when empty

	// key: 0-9, A-Z
	struct a3d_hashmapNode_s* nodes[A3D_HASHMAP_KEY_COUNT];
	struct a3d_hashmapNode_s* parent;

	// matching value
	const void* val;
} a3d_hashmapNode_t;

typedef struct
{
	int size;
	a3d_hashmapNode_t* head;
} a3d_hashmap_t;

a3d_hashmap_t*     a3d_hashmap_new(void);
void               a3d_hashmap_delete(a3d_hashmap_t** _self);
void               a3d_hashmap_discard(a3d_hashmap_t* self);
int                a3d_hashmap_size(const a3d_hashmap_t* self);
int                a3d_hashmap_empty(const a3d_hashmap_t* self);
a3d_hashmapIter_t* a3d_hashmap_head(const a3d_hashmap_t* self,
                                    a3d_hashmapIter_t* iter);
a3d_hashmapIter_t* a3d_hashmap_next(a3d_hashmapIter_t* iter);
const void*        a3d_hashmap_val(const a3d_hashmapIter_t* iter);
const char*        a3d_hashmap_key(const a3d_hashmapIter_t* iter);
const void*        a3d_hashmap_find(const a3d_hashmap_t* self,
                                    a3d_hashmapIter_t* iter,
                                    const char* key);
const void*        a3d_hashmap_findf(const a3d_hashmap_t* self,
                                     a3d_hashmapIter_t* iter,
                                     const char* fmt, ...);
int                a3d_hashmap_add(a3d_hashmap_t* self,
                                   a3d_hashmapIter_t* iter,
                                   const void* val,
                                   const char* key);
int                a3d_hashmap_addf(a3d_hashmap_t* self,
                                    a3d_hashmapIter_t* iter,
                                    const void* val,
                                    const char* fmt, ...);
const void*        a3d_hashmap_replace(a3d_hashmapIter_t* iter,
                                       const void* val);
const void*        a3d_hashmap_remove(a3d_hashmap_t* self,
                                      a3d_hashmapIter_t** _iter);
void               a3d_hashmap_base36(double x, int len, char* b);

#endif
