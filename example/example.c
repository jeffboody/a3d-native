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

#include <stdlib.h>
#include <assert.h>
#include "a3d/a3d_list.h"

#define LOG_TAG "example"
#include "a3d/a3d_log.h"

int compare(const void* _a, const void* _b)
{
	int a = (int) _a;
	int b = (int) _b;
	if(a == b)
	{
		return 0;
	}
	else if(a >= b)
	{
		return 1;
	}
	return -1;
}

void testeq(int a, int b)
{
	if(a == b)
	{
		LOGI("[pass] %i %i", a, b);
	}
	else
	{
		LOGI("[fail] %i %i", a, b);
	}
}

int main(int argc, char** argv)
{
	int i;

	a3d_list_t* list = a3d_list_new();
	if(list == NULL)
	{
		return EXIT_FAILURE;
	}

	// test stack push/pop
	{
		LOGI("STACK");
		for(i = 0; i < 10; ++i)
		{
			a3d_list_push(list, (const void*) i);
		}

		testeq(10, a3d_list_size(list));
		testeq(0, a3d_list_empty(list));

		for(i = 9; i >= 0; --i)
		{
			int x = (int) a3d_list_pop(list);
			testeq(i, x);
		}

		testeq(1, a3d_list_empty(list));
	}

	// test queue enqueue/dequeue
	{
		LOGI("QUEUE");
		for(i = 0; i < 10; ++i)
		{
			a3d_list_enqueue(list, (const void*) i);
		}

		testeq(10, a3d_list_size(list));
		testeq(0, a3d_list_empty(list));

		for(i = 0; i < 10; ++i)
		{
			int x = (int) a3d_list_dequeue(list);
			testeq(i, x);
		}

		testeq(1, a3d_list_empty(list));
	}

	// test peek
	{
		LOGI("PEEK");
		a3d_list_push(list, (const void*) 0);
		testeq(0, (int) a3d_list_peekhead(list));
		testeq(0, (int) a3d_list_peektail(list));
		a3d_list_push(list, (const void*) 1);
		testeq(1, (int) a3d_list_peekhead(list));
		testeq(0, (int) a3d_list_peektail(list));
		a3d_list_push(list, (const void*) 2);
		testeq(2, (int) a3d_list_peekhead(list));
		testeq(0, (int) a3d_list_peektail(list));
		a3d_list_enqueue(list, (const void*) 3);
		testeq(2, (int) a3d_list_peekhead(list));
		testeq(3, (int) a3d_list_peektail(list));

		for(i = 0; i < 4; ++i)
		{
			a3d_list_dequeue(list);
		}

		testeq(1, a3d_list_empty(list));
	}

	// test insert/append/replace/remove/find
	{
		LOGI("INSERT/APPEND/REPLACE/REMOVE/FIND");
		for(i = 0; i < 10; ++i)
		{
			a3d_list_enqueue(list, (const void*) i);
		}

		a3d_listitem_t* item = a3d_list_find(list, (const void*) 7, compare);
		testeq(7, (int) a3d_list_peekitem(item));
		a3d_list_insert(list, item, (const void*) 77);
		a3d_list_append(list, item, (const void*) 88);
		a3d_list_replace(list, item, (const void*) 42);

		item = a3d_list_find(list, (const void*) 2, compare);
		a3d_list_remove(list, &item);
		testeq(3, (int) a3d_list_peekitem(item));

		int x;
		for(i = 0; i < 2; ++i)
		{
			x = (int) a3d_list_dequeue(list);
			testeq(i, x);
		}
		for(i = 3; i < 7; ++i)
		{
			x = (int) a3d_list_dequeue(list);
			testeq(i, x);
		}
		x = (int) a3d_list_dequeue(list);
		testeq(77, x);
		x = (int) a3d_list_dequeue(list);
		testeq(42, x);
		x = (int) a3d_list_dequeue(list);
		testeq(88, x);
		for(i = 8; i < 10; ++i)
		{
			x = (int) a3d_list_dequeue(list);
			testeq(i, x);
		}

		testeq(1, a3d_list_empty(list));
	}

	a3d_list_delete(&list);
	return EXIT_SUCCESS;
}
