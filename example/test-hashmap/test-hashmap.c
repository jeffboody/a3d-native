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

#include <stdlib.h>
#include "a3d/a3d_hashmap.h"

#define LOG_TAG "test-hashmap"
#include "a3d/a3d_log.h"

int main(int argc, char** argv)
{
	a3d_hashmap_t* hashmap = a3d_hashmap_new();
	if(hashmap == NULL)
	{
		return EXIT_FAILURE;
	}

	LOGI("%s", "ADD");
	int zero   = 0;
	int one    = 1;
	int two    = 2;
	int three  = 3;
	int four   = 4;
	int five   = 5;
	int six    = 6;
	int seven  = 7;
	int eight  = 8;
	int nine   = 9;
	int ten    = 10;
	a3d_hashmapIter_t  iterator;
	a3d_hashmapIter_t* iter = &iterator;
	const int*  val;
	const char* key;
	if(a3d_hashmap_add(hashmap,  iter, (const void*) &zero,  "zero"))
	{
		val = (const int*) a3d_hashmap_val(iter);
		key = a3d_hashmap_key(iter);
		LOGI("iter: key=%s, val=%i", key, *val);
	}

	a3d_hashmap_add(hashmap,  iter, (const void*) &one,   "one");
	a3d_hashmap_add(hashmap,  iter, (const void*) &two,   "two");
	a3d_hashmap_add(hashmap,  iter, (const void*) &three, "three");
	a3d_hashmap_add(hashmap,  iter, (const void*) &four,  "four");
	a3d_hashmap_add(hashmap,  iter, (const void*) &five,  "five");
	a3d_hashmap_add(hashmap,  iter, (const void*) &six,   "six");
	a3d_hashmap_add(hashmap,  iter, (const void*) &seven, "seven");
	a3d_hashmap_add(hashmap,  iter, (const void*) &eight, "eight");
	a3d_hashmap_add(hashmap,  iter, (const void*) &nine,  "nine");
	a3d_hashmap_addf(hashmap, iter, (const void*) &ten,   "%i", 10);
	a3d_hashmap_add(hashmap,  iter, (const void*) &one,   "fib");
	a3d_hashmap_add(hashmap,  iter, (const void*) &two,   "fib");
	a3d_hashmap_add(hashmap,  iter, (const void*) &one,   "fib");
	a3d_hashmap_add(hashmap,  iter, (const void*) &three, "fib");
	a3d_hashmap_add(hashmap,  iter, (const void*) &eight, "fib");
	a3d_hashmap_add(hashmap,  iter, (const void*) &five,  "fib");

	LOGI("size=%i", a3d_hashmap_size(hashmap));

	LOGI("%s", "ITER");
	iter = a3d_hashmap_head(hashmap, &iterator);
	while(iter)
	{
		val = (const int*) a3d_hashmap_val(iter);
		key = a3d_hashmap_key(iter);
		LOGI("key=%s, val=%i", key, *val);
		iter = a3d_hashmap_next(iter);
	};

	LOGI("%s", "FIND");
	iter = &iterator;
	if(a3d_hashmap_find(hashmap, iter, "Fib"))
	{
		val = (const int*) a3d_hashmap_val(iter);
		key = a3d_hashmap_key(iter);
		LOGI("FIB: key=%s, val=%i", key, *val);
	}

	LOGI("REPLACE");
	iter = a3d_hashmap_head(hashmap, &iterator);
	if(iter)
	{
		val = (const int*) a3d_hashmap_val(iter);
		key = a3d_hashmap_key(iter);
		LOGI("key=%s, val=%i", key, *val);

		int fortytwo = 42;
		a3d_hashmap_replace(iter, (const void*) &fortytwo);
		val = (const int*) a3d_hashmap_val(iter);
		key = a3d_hashmap_key(iter);
		LOGI("key=%s, val=%i", key, *val);
	}

	a3d_hashmap_discard(hashmap);
	a3d_hashmap_delete(&hashmap);

	return EXIT_SUCCESS;
}
