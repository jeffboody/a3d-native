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
#include "test_plane.h"
#include "a3d/math/a3d_plane.h"

#define LOG_TAG "test_plane"
#include "a3d/a3d_log.h"

static void testeq(int a, int b)
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

static void testeqf(float a, float b, float delta)
{
	if(a == b)
	{
		LOGI("[pass] %f %f", a, b);
	}
	else if((a > b) && (a <= (b + delta)))
	{
		LOGI("[pass] %f %f", a, b);
	}
	else if((a < b) && (a >= (b - delta)))
	{
		LOGI("[pass] %f %f", a, b);
	}
	else
	{
		LOGI("[fail] %f %f", a, b);
	}
}

void test_plane(void)
{
	// test distance
	{
		LOGI("DISTANCE");

		a3d_vec3f_t q;
		a3d_plane_t p;

		a3d_vec3f_load(&q, 0.0f, 0.0f, 10.0f);
		a3d_plane_load(&p, 0.0f, 0.0f, 1.0f, 0.0f);
		testeqf(a3d_plane_distance(&p, &q), 10.0f, 0.001f);

		a3d_plane_load(&p, 0.0f, 0.0f, 1.0f, 5.0f);
		testeqf(a3d_plane_distance(&p, &q), 5.0f, 0.001f);

		a3d_plane_load(&p, 0.0f, 0.0f, 1.0f, 15.0f);
		testeqf(a3d_plane_distance(&p, &q), -5.0f, 0.001f);

		a3d_vec3f_load(&q, 10.0f, 10.0f, 10.0f);
		a3d_plane_load(&p, 0.0f, 1.0f, 0.0f, 10.0f);
		testeqf(a3d_plane_distance(&p, &q), 0.0f, 0.001f);
	}

	// test clipsphere
	{
		LOGI("CLIPSPHERE");

		a3d_sphere_t s;
		a3d_plane_t  p;

		a3d_sphere_load(&s, 0.0f, 0.0f, 5.0f, 3.0f);
		a3d_plane_load(&p, 0.0f, 0.0f, 1.0f, 0.0f);
		testeq(a3d_plane_clipsphere(&p, &s), 0);

		a3d_plane_load(&p, 0.0f, 0.0f, 1.0f, 5.0f);
		testeq(a3d_plane_clipsphere(&p, &s), 0);

		a3d_plane_load(&p, 0.0f, 0.0f, 1.0f, 7.0f);
		testeq(a3d_plane_clipsphere(&p, &s), 0);

		a3d_plane_load(&p, 0.0f, 0.0f, 1.0f, 9.0f);
		testeq(a3d_plane_clipsphere(&p, &s), 1);
	}
}
