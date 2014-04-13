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


#ifndef picker_procset_h
#define picker_procset_h

#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <dirent.h>
#include <stdlib.h>

#include "picker_debug.h"
#include "picker_bitmap.h"

#include <glib/garray.h>

typedef struct 
{
	GArray* procs;
	GByteArray* pids; /* manipulate only trough picker_bitmap's api ! */
} picker_procset;


typedef struct {
	int pid;
	int stat_fd;
/*  int statm_fd;*/
} picker_process;


picker_procset*
picker_procset_new(void);

void
picker_procset_free(picker_procset *set);


__attribute__((always_inline)) extern inline unsigned int
picker_procset_has(picker_procset *set, int pid)
{
	assert(set);
	assert(pid > 0);

	return picker_bitmap_get_bit(set->pids, pid);
}


__attribute__((always_inline)) extern inline picker_process*
picker_procset_at(picker_procset *set,
                  unsigned int at)
{
	assert(set);
	assert(at < set->procs->len);

	return &g_array_index(set->procs, picker_process, at);
}


__attribute__((always_inline)) extern inline unsigned int
picker_procset_size(picker_procset *set)
{
	assert(set);
	return set->procs->len;
}


__attribute__((always_inline)) extern inline void
picker_procset_add(picker_procset *set,
                   int pid,
                   int stat_fd)/*,
				 int statm_fd)*/
{
	picker_process proc;

	assert(set);
	assert(pid > 0);
	assert(stat_fd != -1);
/*  assert(statm_fd != -1);*/
	proc.pid = pid;
	proc.stat_fd = stat_fd;
/*  proc.statm_fd = statm_fd;*/

	g_array_append_val(set->procs, proc);
	picker_bitmap_set_bit(set->pids, pid);
}


__attribute__((always_inline)) extern inline void
picker_procset_remove(picker_procset *set,
                      unsigned int at)
{
	picker_process *proc;
  
	assert(set);
	assert(at < set->procs->len);
  
	proc = picker_procset_at(set, at);
	assert(proc);
	picker_bitmap_clear_bit(set->pids, proc->pid);
	assert(!picker_bitmap_get_bit(set->pids, proc->pid));
	g_array_remove_index_fast(set->procs, at);
}

#endif /* picker_procset_h */

