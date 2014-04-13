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

#ifndef grapher_track_h
#define grapher_track_h 1

#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "cu_vector_uint.h"

#include "grapher_debug.h"

#define GRAPHER_TRACK_NAME_MAX_LEN (unsigned int)32

typedef struct {
	unsigned int min;
	unsigned int max;  
	char name[GRAPHER_TRACK_NAME_MAX_LEN];
	cu_vector_uint data;
} grapher_track;


void
grapher_track_init(grapher_track *track,
                   const char* name);

void
grapher_track_deinit(grapher_track *track);

void
grapher_track_push_back(grapher_track *track,
                        unsigned int value);

unsigned int
grapher_track_size(grapher_track *track);

unsigned int
grapher_track_at(grapher_track *track,
                 unsigned int at);

void
grapher_track_clone(grapher_track *dest,
                    grapher_track *src);
                    
void
grapher_track_slice_clone(grapher_track *dest,
                          grapher_track *src,
                          unsigned int lpos,
                          unsigned int rpos);
#endif /* grapher_track_h */
