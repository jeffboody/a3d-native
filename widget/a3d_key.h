/*
 * Copyright (c) 2016 Jeff Boody
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

#ifndef a3d_key_H
#define a3d_key_H

// meta key mask
#define A3D_KEY_ALT     0x00000032
#define A3D_KEY_ALT_L   0x00000010
#define A3D_KEY_ALT_R   0x00000020
#define A3D_KEY_CTRL    0x00007000
#define A3D_KEY_CTRL_L  0x00002000
#define A3D_KEY_CTRL_R  0x00004000
#define A3D_KEY_SHIFT   0x000000C1
#define A3D_KEY_SHIFT_L 0x00000040
#define A3D_KEY_SHIFT_R 0x00000080
#define A3D_KEY_CAPS    0x00100000

// special keys
#define A3D_KEY_ENTER     0x00D
#define A3D_KEY_ESCAPE    0x01B
#define A3D_KEY_BACKSPACE 0x008
#define A3D_KEY_DELETE    0x07F
#define A3D_KEY_UP        0x100
#define A3D_KEY_DOWN      0x101
#define A3D_KEY_LEFT      0x102
#define A3D_KEY_RIGHT     0x103
#define A3D_KEY_HOME      0x104
#define A3D_KEY_END       0x105
#define A3D_KEY_PGUP      0x106
#define A3D_KEY_PGDOWN    0x107
#define A3D_KEY_INSERT    0x108

#endif
