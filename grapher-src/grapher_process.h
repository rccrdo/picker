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

#ifndef grapher_process_h
#define grapher_process_h 1

#include <stdio.h>
#include <string.h>

#include "grapher_debug.h"
#include "grapher_track.h"

#define GRAPHER_PROCESS_NAME_MAX_LEN (unsigned int)255
#define GRAPHER_PROCESS_CMDLINE_MAX_LEN (unsigned int)255


typedef struct {
	unsigned int pid;
	unsigned int ppid;
	char name[GRAPHER_PROCESS_NAME_MAX_LEN];
	char cmdline[GRAPHER_PROCESS_CMDLINE_MAX_LEN];
	unsigned int start_tick;

	/* from /proc/$pid/stat */
	grapher_track minflt;
	grapher_track majflt;
	grapher_track utime;
	grapher_track stime;
  
	unsigned int tot_minflt;
	unsigned int tot_majflt;
	unsigned int minflt_last;
	unsigned int majflt_last;
  

	/* from /proc/$pid/statm */
	grapher_track mem_size;
	grapher_track mem_resident;
} grapher_process;


void
grapher_process_init(grapher_process *p,
                     unsigned int pid,
                     const char *cmdline,
                     unsigned int start_tick );

void
grapher_process_deinit(grapher_process *p);

void
grapher_process_put_pstat(grapher_process *p,
                          const char *buf);

void
grapher_process_put_pstatm(grapher_process *p,
                           const char *buf);

void
grapher_process_slice_clone(grapher_process *dest,
                            grapher_process *src,
                            unsigned int lpos,
                            unsigned int rpos);

#endif /* grapher_process_h */

