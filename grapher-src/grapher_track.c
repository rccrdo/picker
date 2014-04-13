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

#include "grapher_track.h"

void
grapher_track_init(grapher_track *track,
                   const char *name)
{
	unsigned int name_len;
  
	assert(track);
	assert(name);
  
	track->min = 4294967295U;
	track->max = 0;
  
	name_len = strnlen(name, GRAPHER_TRACK_NAME_MAX_LEN);
	memcpy(track->name, name, name_len);
	track->name[name_len] =0;
  
	if ( (name_len == GRAPHER_TRACK_NAME_MAX_LEN)
	     && name[GRAPHER_TRACK_NAME_MAX_LEN-1]!=0 )
	{
		track->name[GRAPHER_TRACK_NAME_MAX_LEN -3] = '.';
		track->name[GRAPHER_TRACK_NAME_MAX_LEN -2] = '.';
		track->name[GRAPHER_TRACK_NAME_MAX_LEN -1] = 0;
	}
    
	cu_vector_uint_init(&track->data);
}


void
grapher_track_deinit(grapher_track *track)
{
	assert(track);
	cu_vector_uint_deinit(&track->data);
}


void
grapher_track_push_back(grapher_track *track,
                        unsigned int value)
{
	assert(track);
  
	if (value < track->min)
		track->min = value;

	if (value > track->max)
		track->max = value;
      
	if(track->min==track->max) 
		track->max++;
      
	cu_vector_uint_push_back(&track->data, value);
}


unsigned int
grapher_track_size(grapher_track *track)
{
	assert(track);
	return cu_vector_uint_size(&track->data);
}


unsigned int
grapher_track_at(grapher_track *track,
                 unsigned int at)
{
	assert(track);
	assert(at < cu_vector_uint_size(&track->data));
  
	return cu_vector_uint_at(&track->data, at);
}


void
grapher_track_clone(grapher_track *dest,
                    grapher_track *src)
{
	assert(dest);
	assert(src);
	dest->min = src->min;
	dest->max = src->max;  
	memcpy(dest->name, src->name, GRAPHER_TRACK_NAME_MAX_LEN);
	cu_vector_uint_clone(&dest->data, &src->data);
}


void
grapher_track_slice_clone(grapher_track *dest,
                          grapher_track *src,
                          unsigned int lpos,
                          unsigned int rpos)
{
	assert(dest);
	assert(src);
	assert(lpos < rpos);
  
	assert(rpos <= grapher_track_size(src));
	memcpy(dest->name, src->name, GRAPHER_TRACK_NAME_MAX_LEN);

	{
		unsigned int i;
		for (i = lpos; i <= rpos; i++)
		{
			unsigned int current_value;
			current_value = grapher_track_at(src, i);
			grapher_track_push_back(dest, current_value);
		}
	}  
}

