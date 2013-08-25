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

#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include "test_orientation.h"
#include "a3d/math/a3d_orientation.h"
#include "a3d/a3d_time.h"

#define LOG_TAG "test_orientation"
#include "a3d/a3d_log.h"

a3d_orientation_t* g_orientation = NULL;

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

static float rad(float a)
{
	return a*(M_PI/180.0f);
}

static void reset(void)
{
	a3d_orientation_reset(g_orientation);
}

static void accelerometer(void)
{
	double utime = a3d_utime();
	a3d_orientation_accelerometer(g_orientation, utime, 0.0f, 0.0f, 9.8f, 0);
}

static void magnetometer(float a)
{
	LOGD("a=%f", a);

	double utime = a3d_utime();
	float mx = sinf(rad(a));
	float my = cosf(rad(a));
	float mz = 0.0f;
	a3d_orientation_magnetometer(g_orientation, utime, mx, my, mz);
}

void test_orientation(void)
{
	g_orientation = a3d_orientation_new();
	if(g_orientation == NULL)
	{
		return;
	}

	// test 45 degrees
	{
		LOGI("ORIENTATION 45 degrees");

		accelerometer();
		magnetometer(30.0f);
		magnetometer(60.0f);

		a3d_mat4f_t m;
		a3d_orientation_mat4f(g_orientation, &m);

		testeqf( cosf(rad(45.0f)), m.m00, 0.001f);
		testeqf(-sinf(rad(45.0f)), m.m01, 0.001f);
		testeqf(             0.0f, m.m02, 0.001f);
		testeqf(             0.0f, m.m10, 0.001f);
		testeqf(             0.0f, m.m11, 0.001f);
		testeqf(             1.0f, m.m12, 0.001f);
		testeqf( sinf(rad(45.0f)), m.m20, 0.001f);
		testeqf( cosf(rad(45.0f)), m.m21, 0.001f);
		testeqf(             0.0f, m.m22, 0.001f);
	}

	// test 0 degrees
	{
		LOGI("ORIENTATION 0 degrees");

		reset();
		accelerometer();
		magnetometer(1.0f);
		magnetometer(2.0f);
		magnetometer(3.0f);
		magnetometer(4.0f);
		magnetometer(5.0f);
		magnetometer(6.0f);
		magnetometer(7.0f);
		magnetometer(8.0f);
		magnetometer(-1.0f);
		magnetometer(-2.0f);
		magnetometer(-3.0f);
		magnetometer(-4.0f);
		magnetometer(-5.0f);
		magnetometer(-6.0f);
		magnetometer(-7.0f);
		magnetometer(-8.0f);

		a3d_mat4f_t m;
		a3d_orientation_mat4f(g_orientation, &m);

		testeqf(1.0f, m.m00, 0.001f);
		testeqf(0.0f, m.m01, 0.001f);
		testeqf(0.0f, m.m02, 0.001f);
		testeqf(0.0f, m.m10, 0.001f);
		testeqf(0.0f, m.m11, 0.001f);
		testeqf(1.0f, m.m12, 0.001f);
		testeqf(0.0f, m.m20, 0.001f);
		testeqf(1.0f, m.m21, 0.001f);
		testeqf(0.0f, m.m22, 0.001f);
	}

	a3d_orientation_delete(&g_orientation);
}
