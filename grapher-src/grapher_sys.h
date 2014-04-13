/*
 * Copyright (c) 2008 Riccardo Lucchese, riccardo.lucchese at gmail.com
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

#ifndef grapher_sys_h
#define grapher_sys_h 1

#include <stdio.h>

#include "grapher_debug.h"
#include "grapher_track.h"

#include "cu_vector_uint.h"

typedef struct {
	/* from /proc/stat */
	grapher_track cpu_us;
	grapher_track cpu_ni;
	grapher_track cpu_sy;
	grapher_track cpu_id;
	grapher_track cpu_wa;
	grapher_track cpu_hi;
	grapher_track cpu_si;
	grapher_track cpu_st;

	cu_vector_uint jiffies;  /* used when drawing processes */

	unsigned int cpu_us_last;
	unsigned int cpu_ni_last;
	unsigned int cpu_sy_last;
	unsigned int cpu_id_last;
	unsigned int cpu_wa_last;
	unsigned int cpu_hi_last;
	unsigned int cpu_si_last;
	unsigned int cpu_st_last;
} grapher_sys;

void
grapher_sys_init(grapher_sys *sys);

void
grapher_sys_cpu_push_back(grapher_sys *sys,
                          unsigned int us,
                          unsigned int ni,
                          unsigned int sy,
                          unsigned int id,
                          unsigned int wa,
                          unsigned int hi,
                          unsigned int si,
                          unsigned int st );

void grapher_sys_deinit(grapher_sys *sys);

void
grapher_sys_clone(grapher_sys *dest,
                  grapher_sys *src);

void
grapher_sys_slice_clone(grapher_sys *dest,
                        grapher_sys *src,
                        unsigned int lpos,
                        unsigned int rpos);

#endif /* grapher_sys_h */

