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

#include "grapher_statstore.h"

void
grapher_statstore_init(grapher_statstore *store)
{
	assert(store);
  
	grapher_sys_init(&store->sys);
  
	cu_vector_ptrs_init(&store->timeline);
  
	cu_vector_ptrs_init(&store->activep);
	cu_bitmap_init(&store->ap_bitmap, GRAPHER_MAX_NUM_PROCESSES);
	cu_bitmap_clear(&store->ap_bitmap);

	cu_vector_ptrs_init(&store->cmdlinep);  
}


void
grapher_statstore_deinit(grapher_statstore *store)
{
	unsigned int i;

	assert(store);
  
	grapher_sys_deinit(&store->sys);
  
	for (i=0; i < cu_vector_ptrs_size(&store->timeline); i++)
		free(cu_vector_ptrs_at(&store->timeline, i));

	cu_vector_ptrs_deinit(&store->timeline);
  
	cu_vector_ptrs_deinit(&store->activep);
	cu_bitmap_deinit(&store->ap_bitmap);
  
	cu_vector_ptrs_deinit(&store->cmdlinep);  
}


void
grapher_statstore_tick(grapher_statstore *store,
                       const char *buf)
{
	struct timeval* time_value;
  
	assert(store);
	assert(buf);
                  
	time_value = (struct timeval*) malloc(sizeof(struct timeval));
	memcpy(time_value, buf, sizeof(struct timeval));

	cu_vector_ptrs_push_back(&store->timeline, time_value);
}

float
grapher_statstore_get_sampling_time(grapher_statstore *store)
{
	struct timeval* start;
	struct timeval* end;
	unsigned int timeline_size;
  
	timeline_size = cu_vector_ptrs_size(&store->timeline); 
	/* we need at least the start and end samples */
	if (timeline_size < 2)
		return 0.;
	start = cu_vector_ptrs_at(&store->timeline, 0);
	assert(start);
	end = cu_vector_ptrs_at(&store->timeline, timeline_size - 1);
	assert(end);
	return ((float)cu_time_passed(start, end)) / 1000000.;
}


grapher_process*
grapher_statstore_get_active_process_by_pid(grapher_statstore *store,
                                            unsigned int pid)
{
	unsigned int i;
	grapher_process *search_result;

	assert(store);
	assert(pid);

	if (!cu_bitmap_get_bit(&store->ap_bitmap, pid))
		return NULL;
  
	/* search it */
	search_result = NULL;
	for (i = 0; i< cu_vector_ptrs_size(&store->activep); i++)
	{
		grapher_process *p;
      
		p = (grapher_process*) cu_vector_ptrs_at(&store->activep, i);
		assert(p);
      
		if (p->pid==pid)
		{
			search_result = p;
			break;        
		}
	}

	return search_result;
}


void
grapher_statstore_put_stat(grapher_statstore *store,
                           const char *buf)
{
	assert(store);
	assert(buf);
	unsigned int us, ni, sy, id, wa, hi, si, st;
  
	/* we only used cpu usage now (and xo only has one cpu..) */
	sscanf(buf, "%*s %u %u %u %u %u %u %u %u", &us, &ni, &sy, &id, &wa, &hi, &si, &st);
        
	grapher_sys_cpu_push_back(&store->sys, us, ni, sy, id, wa, hi, si, st);
}


void
grapher_statstore_put_pstatm(grapher_statstore *store,
                             unsigned int _pid,
                             const char *buf)
{
	grapher_process *p;

	assert(store);
	assert(_pid);
	assert(buf);  

	p = grapher_statstore_get_active_process_by_pid(store, _pid);

	if (!p)
	{
		printf("warning received a FRAME_CONTENT_T_PROC_PSTATM before a FRAME_CONTENT_T_PROC_PSTATM"
		       " for pid %d, this is probably a bug in picker\n", _pid);
		return;
	}
  
	grapher_process_put_pstatm(p, buf);
}


void
grapher_statstore_put_pstat(grapher_statstore *store,
                            unsigned int _pid,
                            const char *buf)
{
	grapher_process *p;

	assert(store);
	assert(_pid);
	assert(buf);

	p = grapher_statstore_get_active_process_by_pid(store, _pid);

	if (!p)
	{
		/* malloc a new grapher_process */
		/* TODO: at this point we can also remove this pointer from here ...*/
		p = grapher_statstore_get_cmdlinep_by_pid(store, _pid);
		assert(p);

		unsigned int pid;
		char pname[32];
		char _pname[32];
		unsigned int ppid;
  
		/* names like (Journal <a53084) are detected badly by scanf */
		unsigned int i=0;
		int found_first = 0;
		/* parse buf for pid ppid nand name */
		while (buf[i]!=')')
		{
			if (buf[i]=='(')
				found_first = 1;
			if (found_first && (buf[i]==' '))
				((char*)buf)[i]='_';      
			i++;
		}
		sscanf(buf, "%d %s %*c %d", &pid, _pname, &ppid);

		assert(pid == _pid);
  
		/* remove '(' and ')' from pname */
		{
			unsigned int pname_len;
    
			pname_len = strlen(_pname);
			memcpy(pname, _pname + 1, pname_len -2);
			pname[pname_len -2] = 0;
		}
      
		p->ppid = ppid;
		strncpy(p->name, pname, GRAPHER_PROCESS_NAME_MAX_LEN);
      
		if (!strcmp(p->cmdline, "<empty-cmdline>"))
			strncpy(p->cmdline, pname, GRAPHER_PROCESS_CMDLINE_MAX_LEN);        

		cu_vector_ptrs_push_back(&store->activep, p);
		cu_bitmap_set_bit(&store->ap_bitmap, pid);
	}

	grapher_process_put_pstat(p, buf);
}


grapher_process*
grapher_statstore_get_cmdlinep_by_pid(grapher_statstore *store,
                                      unsigned int pid)
{
	unsigned int num_cmdlinep;
	unsigned int i;
	grapher_process *p;
  
	assert(store);
	assert(pid);
  
	p = NULL;
	num_cmdlinep = cu_vector_ptrs_size(&store->cmdlinep);
	for (i =0; i <num_cmdlinep; i++)
	{
		grapher_process *tmp;
		tmp = (grapher_process*)cu_vector_ptrs_at(&store->cmdlinep, i);
		if (tmp)
			if (tmp->pid == pid)
			{
				p = tmp;
				break;
			}   
	}

	return p;
}
                                      
                                      
void
grapher_statstore_put_pcmdline(grapher_statstore *store,
                               unsigned int _pid,
                               const char *buf)
{
	grapher_process *p;

	assert(store);
	assert(_pid);
	assert(buf);  

	/* cmdline should be sent before /proc/pid/stat or /proc/pid/statm */
	assert(!grapher_statstore_get_active_process_by_pid(store, _pid));
  
	assert(!grapher_statstore_get_cmdlinep_by_pid(store, _pid));
  
	/* malloc a new grapher_process */
	p = (grapher_process*) malloc(sizeof(grapher_process));
	assert(p);
  
	grapher_process_init(p, _pid, buf, cu_vector_ptrs_size(&store->timeline)-1);
	cu_vector_ptrs_push_back(&store->cmdlinep, p);
}


void
grapher_statstore_put_kthread_alert(grapher_statstore *store,
                                    unsigned int _pid)
{
	grapher_process *p;

	assert(store);
	assert(_pid);
 
	/* kthread_alert should be sent before /proc/pid/stat or /proc/pid/statm */
	assert(!grapher_statstore_get_active_process_by_pid(store, _pid));
  
	/* malloc a new grapher_process */
	p = (grapher_process*) malloc(sizeof(grapher_process));
	assert(p);

	grapher_process_init(p, _pid, "<empty-cmdline>", cu_vector_ptrs_size(&store->timeline)-1);
	cu_vector_ptrs_push_back(&store->cmdlinep, p);
}


void
grapher_statstore_print_stats(grapher_statstore *store,
                              unsigned int flags)
{
	unsigned int i;
	unsigned int tot_jiffies;
	double sys;
	struct timeval* picking_start_time;

	assert(store);
	assert(flags);
    
	picking_start_time = cu_vector_ptrs_at(&store->timeline, 0);
	assert(picking_start_time);
  
	tot_jiffies = 0;
	for (i =0; i< cu_vector_uint_size(&store->sys.jiffies); i++)
		tot_jiffies += cu_vector_uint_at(&store->sys.jiffies, i);

	printf("\n * Statistics resume\n");
	if (flags & GRAPHER_STATSTORE_STATS_FLAGS_CPU)
		printf(" sys%%\tps%%");
	if (flags & GRAPHER_STATSTORE_STATS_FLAGS_FAULTS)
	{
		if (flags & GRAPHER_STATSTORE_STATS_FLAGS_CPU)
			printf("\t");
		else 
			printf(" ");
		printf("minflt\tmajflt");
	}
	if (flags & GRAPHER_STATSTORE_STATS_FLAGS_TIMINGS)
	{
		if ((flags & GRAPHER_STATSTORE_STATS_FLAGS_CPU) ||
		    (flags & GRAPHER_STATSTORE_STATS_FLAGS_FAULTS) )
			printf("\t");
		else 
			printf(" ");
		printf("start\tend\tlength");
	}

	printf("\tcmdline\n");

	printf(" ");
	if (flags & GRAPHER_STATSTORE_STATS_FLAGS_CPU)
		printf("---------------");
	if (flags & GRAPHER_STATSTORE_STATS_FLAGS_FAULTS)
		printf("---------------");
	if (flags & GRAPHER_STATSTORE_STATS_FLAGS_TIMINGS)
		printf("-----------------------");
	printf ("--------\n");

	sys = 0.;
	for (i=0; i< cu_vector_ptrs_size(&store->activep); i++)
	{
		unsigned int cpu_time;
		unsigned int samples;
		double start_time;
		double end_time;
		double time_length;

		grapher_process *p;
      
		p = cu_vector_ptrs_at(&store->activep, i);
		assert(p);

		samples = grapher_track_size(&p->utime);
		cpu_time = 0;
		cpu_time += grapher_track_at(&p->utime, samples -1);
		cpu_time -= grapher_track_at(&p->utime, 0);
		cpu_time += grapher_track_at(&p->stime, samples -1);
		cpu_time -= grapher_track_at(&p->stime, 0);
      
		double ratio = ((double) cpu_time) / ((double) tot_jiffies)*100.;
		sys += ratio;

		start_time =(double)cu_time_passed(picking_start_time,
						   (struct timeval*)cu_vector_ptrs_at(&store->timeline, p->start_tick));
		start_time /=1000000.;
		end_time =(double)cu_time_passed(picking_start_time,
						 (struct timeval*)cu_vector_ptrs_at(&store->timeline, p->start_tick + samples -1));
		end_time /=1000000.;
		time_length = end_time - start_time;
		if (!time_length)
		{
			time_length = (double)cu_time_passed(picking_start_time,
							     (struct timeval*)cu_vector_ptrs_at(&store->timeline, 1));
			time_length /=1000000.;
		}

		if (flags & GRAPHER_STATSTORE_STATS_FLAGS_CPU)
		{
			if (!i)
				printf ("   \t%.1f", ratio);
			else
				printf (" %.1f\t%.1f", sys, ratio);
		}
		if (flags & GRAPHER_STATSTORE_STATS_FLAGS_FAULTS)
		{
			if (flags & GRAPHER_STATSTORE_STATS_FLAGS_CPU)
				printf("\t");
			else 
				printf(" ");
			printf ("%u\t%u", p->tot_minflt, p->tot_majflt);
		}
		if (flags & GRAPHER_STATSTORE_STATS_FLAGS_TIMINGS)
		{
			if ((flags & GRAPHER_STATSTORE_STATS_FLAGS_FAULTS) ||
			    (flags & GRAPHER_STATSTORE_STATS_FLAGS_CPU) )
				printf("\t");
			else 
				printf(" ");
			printf ("%.2f\t%.2f\t%.2f", start_time, end_time, time_length);
		}
		printf("\t%.64s\n", p->cmdline);
	} 
	printf ("\n");
}

