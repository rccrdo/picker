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


#include "picker_procset.h"

#define PROCSET_RESERVE (unsigned int)200 /* in processes slots */

picker_procset*
picker_procset_new(void)
{
	picker_procset* set;
  
	set = malloc(sizeof(picker_procset));
	if (!set)
		return NULL;
  
	set->procs = g_array_sized_new(FALSE, FALSE,
				       sizeof(picker_process),
				       PROCSET_RESERVE);
	if (!set->procs)
	{
		free(set);
		return NULL;
	}
    
	set->pids = picker_bitmap_new();
	if (!set->pids)
	{
		g_array_free(set->procs, TRUE);
		free(set);
		return NULL;    
	}
	return set;
}

#undef PROCSET_RESERVE


void
picker_procset_free(picker_procset *set)
{
	unsigned int i;
	assert(set);

	/* close file descriptors */
	for (i=0; i < picker_procset_size(set); i++)
	{
		picker_process* proc;

		proc = picker_procset_at(set, i);
		assert(proc);
		close(proc->stat_fd);
/*      close(proc->statm_fd);*/
	}

	g_array_free(set->procs, TRUE);
	g_byte_array_free(set->pids, TRUE);
}

