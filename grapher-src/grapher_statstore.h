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

#ifndef grapher_statstore_h
#define grapher_statstore_h 1

#include <sys/time.h>
#include <string.h>

#include "cu_vector_ptrs.h"
#include "cu_bitmap.h"
#include "cu_time_utils.h"

#include "grapher_debug.h"
#include "grapher_sys.h"
#include "grapher_process.h"


#define GRAPHER_MAX_NUM_PROCESSES (unsigned int)100000
#define GRAPHER_STATSTORE_STATS_FLAGS_CPU (unsigned int) 1
#define GRAPHER_STATSTORE_STATS_FLAGS_FAULTS (unsigned int) 2
#define GRAPHER_STATSTORE_STATS_FLAGS_TIMINGS (unsigned int) 4

typedef struct {
	cu_vector_ptrs timeline; /* a vector of ptrs to timeval structs */

	/* sys stats */
	grapher_sys sys;

	/* processes stats */
	cu_vector_ptrs activep;  /* active processes */
	cu_bitmap ap_bitmap;
  
	/* waiting processes for whom we only have the cmdline */
	cu_vector_ptrs cmdlinep;
} grapher_statstore;


void
grapher_statstore_init(grapher_statstore *store);

void
grapher_statstore_deinit(grapher_statstore *store);

void
grapher_statstore_tick(grapher_statstore *store,
		       const char *buf);

float
grapher_statstore_get_sampling_time(grapher_statstore *store);
                            
void
grapher_statstore_put_stat(grapher_statstore *store,
                           const char *buf);

void
grapher_statstore_put_pstat(grapher_statstore *store,
                            unsigned int _pid,
                            const char *buf);

void
grapher_statstore_put_pstatm(grapher_statstore *store,
                             unsigned int _pid,
                             const char *buf);

void
grapher_statstore_put_pcmdline(grapher_statstore *store,
			       unsigned int _pid,
			       const char *buf);
                                   
void
grapher_statstore_put_kthread_alert(grapher_statstore *store,
                                    unsigned int _pid);
                                 
grapher_process*
grapher_statstore_get_active_process_by_pid(grapher_statstore *store,
                                            unsigned int pid);

grapher_process*
grapher_statstore_get_cmdlinep_by_pid(grapher_statstore *store,
                                      unsigned int pid);
                                      
void
grapher_statstore_print_stats(grapher_statstore *store,
                              unsigned int flags);

#endif /* grapher_statstore_h */

