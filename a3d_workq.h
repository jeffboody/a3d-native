/*
 * Copyright (c) 2010 Jeff Boody
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

#ifndef a3d_workq_H
#define a3d_workq_H

#include "a3d_list.h"
#include <pthread.h>

// task state
// COMPLETE : returned by fn/run when task is complete
// PENDING  : returned by run when task is incomplete
// FINISHED : returned by fn if task is finished
//            (don't place on complete queue)
#define A3D_WORKQ_ERROR    0
#define A3D_WORKQ_COMPLETE 1
#define A3D_WORKQ_PENDING  2
#define A3D_WORKQ_FINISHED 3

typedef int (*a3d_workq_fn)(void* task);

typedef struct
{
	int   status;
	int   purge_id;
	void* task;
} a3d_workqnode_t;

// note that the workq is not a container due to
// the fact that tasks may be purged or deleted
// without notification

typedef struct
{
	int              state;
	int              purge_id;
	a3d_list_t*      queue_pending;
	a3d_list_t*      queue_complete;
	a3d_workqnode_t* active_node;
	a3d_workq_fn     workq_fn;

	// workq thread
	pthread_t       thread;
	pthread_mutex_t queue_mutex;
	pthread_cond_t  queue_cond;
} a3d_workq_t;

a3d_workq_t* a3d_workq_new(a3d_workq_fn workq_fn);
void         a3d_workq_delete(a3d_workq_t** _self);
void         a3d_workq_purge(a3d_workq_t* self);
int          a3d_workq_run(a3d_workq_t* self, void* task);
void         a3d_workq_cancel(a3d_workq_t* self, void* task);
int          a3d_workq_pending(a3d_workq_t* self);

#endif
