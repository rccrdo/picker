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

#include "grapher_filters.h"

grapher_statstore*
grapher_filter_time_trim(grapher_statstore *src,
                         float ltrim,
                         float rtrim)
{
	grapher_statstore *dest;
	struct timeval* start;
	struct timeval* end;
	unsigned int timeline_ltick_pos;
	unsigned int timeline_rtick_pos;
	unsigned int timeline_len;
    
	assert(src);
	assert(ltrim >= 0.);
	assert(rtrim >= 0.);
  
	printf("ltrim %f, rtrim %f\n", ltrim, rtrim);
  
	timeline_len = cu_vector_ptrs_size(&src->timeline);
	assert(timeline_len >=2);
	start = cu_vector_ptrs_at(&src->timeline, 0);
	assert(start);
	end = cu_vector_ptrs_at(&src->timeline, timeline_len -1);
	assert(end);

	timeline_ltick_pos = 0;
	timeline_rtick_pos = timeline_len -1;

	/* find timeline_ltick_pos */
	{
		unsigned int i;
		for (i=1; i<timeline_len -1; i++)
		{
			struct timeval *current;
			float time_passed;
        
			current = cu_vector_ptrs_at(&src->timeline, i);
			time_passed = ((float)cu_time_passed(start, current)) / 1000000.;
			if (time_passed > ltrim)
			{
				timeline_ltick_pos = i;
				break;
			}
		}
	}

	/* find timeline_rtick_pos */
	{
		unsigned int i;

		for (i=timeline_len -1; i>0; i--)
		{
			float time_passed;
			struct timeval *current;

			current = cu_vector_ptrs_at(&src->timeline, i);        
			time_passed = ((float)cu_time_passed(current, end)) / 1000000.;
			if (time_passed > rtrim)
			{
				timeline_rtick_pos = i;
				break;
			}
		}
	}
  
	/* consistency check */
	assert(timeline_ltick_pos <= timeline_rtick_pos);
	if (timeline_ltick_pos == timeline_rtick_pos)
	{
		if (timeline_rtick_pos < (timeline_len -1))
			timeline_rtick_pos++;
		else if (timeline_ltick_pos > 0)
			timeline_ltick_pos--;
		else
			assert(0);
	}
  
	printf("timeline_ltick_pos %u, timeline_rtick_pos %u\n", timeline_ltick_pos, timeline_rtick_pos);

	dest = (grapher_statstore*) malloc(sizeof(grapher_statstore));  
	assert(dest);
	grapher_statstore_init(dest);

	/* clone the relevant bits of the timeline */
	{
		unsigned int i;
		for (i = timeline_ltick_pos; i<= timeline_rtick_pos; i++)
		{
			struct timeval *src_time;
			struct timeval *dest_time;

			src_time = cu_vector_ptrs_at(&src->timeline, i);
			assert(src_time);
			dest_time = (struct timeval*)malloc(sizeof(struct timeval));
			assert(dest_time);
			dest_time->tv_sec = src_time->tv_sec;
			dest_time->tv_usec = src_time->tv_usec;
        
			cu_vector_ptrs_push_back(&dest->timeline, dest_time);
		}
	}

	grapher_sys_slice_clone(&dest->sys, &src->sys,
				timeline_ltick_pos, timeline_rtick_pos);


	/* copy over the relevant processes */
	{
		unsigned int num_procs;
		unsigned int i;
		static int lost =0;
 
		num_procs = cu_vector_ptrs_size(&src->activep);
    
		for (i = 0; i < num_procs; i++)
		{
			grapher_process *p;
			unsigned int process_start_tick;
			unsigned int process_end_tick;
			p = cu_vector_ptrs_at(&src->activep, i);
			assert(p);
			process_start_tick = p->start_tick;
			process_end_tick = process_start_tick + grapher_track_size(&p->utime) -1;
/*         printf("timeline_ltick_pos %u, timeline_rtick_pos %u\n", timeline_ltick_pos, timeline_rtick_pos);*/
			printf("%u\t (%u,%u) %.80s\n", i, process_start_tick, process_end_tick, p->cmdline);

			/*
		  ********+*******+**
		  |-------|
			*/        
			if ((process_start_tick <= timeline_ltick_pos) &&
			    (process_end_tick >= timeline_rtick_pos))
			{
				unsigned int slice_begin;
				unsigned int slice_end;
				grapher_process *new;

				if (process_start_tick == process_end_tick)
				{
					printf("-> lost !\n");
					lost++;
					continue;
				}

				slice_begin = timeline_ltick_pos - process_start_tick;
				slice_end = timeline_rtick_pos - timeline_ltick_pos + slice_begin;
/*             printf("1- slice_begin %u, slice_end %u\n", slice_begin, slice_end);*/

				new = (grapher_process*)malloc(sizeof(grapher_process));
				assert(new);

				/*TODO: need a much better interface with for g_process_init/clone
				  who sets what parameters ? */
				grapher_process_init(new, p->pid, p->cmdline, 0);
/*             printf("1- process inited\n");*/
				grapher_process_slice_clone(new,
							    p,
							    slice_begin,
							    slice_end);

/*             printf("1- slice cloned\n");*/

				cu_vector_ptrs_push_back (&dest->activep, new);
				cu_bitmap_set_bit(&dest->ap_bitmap, new->pid);
			}
			/*
		  ********+******
		  |-------|
			*/        
			else if ((process_start_tick <= timeline_ltick_pos) &&
				 (process_end_tick > timeline_ltick_pos) &&
				 (process_end_tick <= timeline_rtick_pos))
			{
				unsigned int slice_begin;
				unsigned int slice_end;
				grapher_process *new;

				if (process_start_tick == process_end_tick)
				{
					printf("-> lost !\n");
					lost++;
					continue;
				}

				slice_begin = timeline_ltick_pos - process_start_tick;
				slice_end = grapher_track_size(&p->utime) -1;
				printf("2- slice_begin %u, slice_end %u\n", slice_begin, slice_end);

				new = (grapher_process*)malloc(sizeof(grapher_process));
				assert(new);
				grapher_process_init(new, p->pid, p->cmdline, 0);
				grapher_process_slice_clone(new,
							    p,
							    slice_begin,
							    slice_end);

				cu_vector_ptrs_push_back (&dest->activep, new);
				cu_bitmap_set_bit(&dest->ap_bitmap, new->pid);
			}
			/*
		      ****           *****
		      |-------|      |---------|
			*/        
			else if ((process_start_tick >= timeline_ltick_pos) &&
				 (process_start_tick <= timeline_rtick_pos))
			{
				unsigned int slice_end;
				unsigned int tracks_end;
				grapher_process *new;

				if (process_start_tick == process_end_tick)
				{
					printf("-> lost !\n");
					lost++;
					continue;
				}

				slice_end = timeline_rtick_pos - process_start_tick;
				tracks_end = grapher_track_size(&p->utime) -1;
				if (slice_end > tracks_end)
					slice_end = tracks_end;

				printf("3- slice_begin %u, slice_end %u\n", 0, slice_end);

				new = (grapher_process*)malloc(sizeof(grapher_process));
				assert(new);
				grapher_process_init(new, p->pid, p->cmdline, process_start_tick - timeline_ltick_pos);
				grapher_process_slice_clone(new,
							    p,
							    0,
							    slice_end);

				cu_vector_ptrs_push_back (&dest->activep, new);
				cu_bitmap_set_bit(&dest->ap_bitmap, new->pid);
			}
		}
  
		printf("! Lost packages of length 1 are %u\n", lost);
	}
  

	return dest;
}



static int grapher_process_rcompare_by_cputime(const void* a,
                                               const void* b)
{
	double atime, btime;
	unsigned int asamples, bsamples;
	grapher_process *aprocess;
	grapher_process *bprocess;

	assert(a);
	assert(b);
  
	aprocess = *((grapher_process**) a);
	bprocess = *((grapher_process**) b);

	asamples = grapher_track_size(&aprocess->utime);
	bsamples = grapher_track_size(&bprocess->utime);
  
	/* a */
	atime = (double) grapher_track_at(&aprocess->utime, asamples -1);
	atime += (double) grapher_track_at(&aprocess->stime, asamples -1);
	atime -= (double) grapher_track_at(&aprocess->utime, 0);
	atime -= (double) grapher_track_at(&aprocess->stime, 0);

	/* b */
	btime = (double) grapher_track_at(&bprocess->utime, bsamples -1);
	btime += (double) grapher_track_at(&bprocess->stime, bsamples -1);
	btime -= (double) grapher_track_at(&bprocess->utime, 0);
	btime -= (double) grapher_track_at(&bprocess->stime, 0);
  
	/* we invert return values to have descendant order in the sorted array */
	if (atime==btime)
		return 0;
	if (atime > btime)
		return -1;
	return 1;
}


static int grapher_process_rcompare_by_majflt(const void* a,
                                              const void* b)
{
	unsigned int asamples, bsamples;
	grapher_process *aprocess;
	grapher_process *bprocess;

	assert(a);
	assert(b);
  
	aprocess = *((grapher_process**) a);
	bprocess = *((grapher_process**) b);

	asamples = grapher_track_size(&aprocess->utime);
	bsamples = grapher_track_size(&bprocess->utime);
  
	/* we invert return values to have descendant order in the sorted array */
	if (aprocess->tot_majflt == bprocess->tot_majflt)
		return 0;
	if (aprocess->tot_majflt > bprocess->tot_majflt)
		return -1;
	return 1;
}


grapher_statstore*
grapher_filter_by_cputime(grapher_statstore *src,
                          unsigned int num_ps )
{
	unsigned int i;
	grapher_statstore *dest;
	grapher_process** parray;
	unsigned int num_stored_ps;
  
	assert(src);
	assert(num_ps);
  
	dest = (grapher_statstore*) malloc(sizeof(grapher_statstore));  
	assert(dest);
  
	grapher_statstore_init(dest);

	/* clone everything but processes entries */
	cu_vector_ptrs_clone(&dest->timeline, &src->timeline);
	grapher_sys_clone(&dest->sys, &src->sys);
	cu_bitmap_clone(&dest->ap_bitmap, &src->ap_bitmap);

	/* find the #num_ps processed whom sucked more cputime */
	if (cu_vector_ptrs_size(&src->activep)<= num_ps)
	{
		/* FIXME: we are copying pointers, better to clone processes */
		cu_vector_ptrs_clone(&dest->activep, &src->activep);
		return dest;
	}
  
	/* sort processes by cpu_usage */
	num_stored_ps = cu_vector_ptrs_size(&src->activep);
	parray = (grapher_process**) malloc(sizeof(grapher_process*)
					    * cu_vector_ptrs_size(&src->activep));
	assert(parray);
  
	for (i =0; i< num_stored_ps; i++)
		parray[i] = (grapher_process*)cu_vector_ptrs_at(&src->activep, i);

	qsort(parray, num_stored_ps,
	      sizeof(grapher_process*),
	      &grapher_process_rcompare_by_cputime);
        
	if (num_ps > num_stored_ps)
		num_ps = num_stored_ps;

	/* push in the destination store only the first #num_ps processes in parray */
	for (i = 0; i<num_ps; i++)
	{
		assert(parray[i]);
		cu_vector_ptrs_push_back(&dest->activep, parray[i]);
	}
 
	free(parray);
	return dest;
}


grapher_statstore*
grapher_filter_by_cputime_percent(grapher_statstore *src,
                                  unsigned int percent )
{
	unsigned int i;
	grapher_statstore *dest;
	grapher_process** parray;
	unsigned int num_stored_ps;
  
	assert(src);
	assert(percent);
  
	dest = (grapher_statstore*) malloc(sizeof(grapher_statstore));  
	assert(dest);
  
	grapher_statstore_init(dest);

	/* clone everything but processes entries */
	cu_vector_ptrs_clone(&dest->timeline, &src->timeline);
	grapher_sys_clone(&dest->sys, &src->sys);
	cu_bitmap_clone(&dest->ap_bitmap, &src->ap_bitmap);

	/* sort processes by cpu_usage */
	num_stored_ps = cu_vector_ptrs_size(&src->activep);
	parray = (grapher_process**) malloc(sizeof(grapher_process*)
					    * cu_vector_ptrs_size(&src->activep));
	assert(parray);
  
	for (i =0; i< num_stored_ps; i++)
		parray[i] = (grapher_process*)cu_vector_ptrs_at(&src->activep, i);

	qsort(parray, num_stored_ps,
	      sizeof(grapher_process*),
	      &grapher_process_rcompare_by_cputime);

	/* push in the destination store only until the percent constraint is satisfied */
	{
		unsigned int tot_sys_jiffies = 0;
		unsigned int selection_cpu_time = 0;
    
		for (i =0; i< cu_vector_uint_size(&src->sys.jiffies); i++)
			tot_sys_jiffies += cu_vector_uint_at(&src->sys.jiffies, i);


		for (i = 0; i< num_stored_ps; i++)
		{
			unsigned int samples;
			double ratio;
			grapher_process *p;

			p = parray[i];
			assert(p);

			cu_vector_ptrs_push_back(&dest->activep, parray[i]);
                
			samples = grapher_track_size(&p->utime);
			selection_cpu_time += grapher_track_at(&p->utime, samples -1);
			selection_cpu_time -= grapher_track_at(&p->utime, 0);
			selection_cpu_time += grapher_track_at(&p->stime, samples -1);
			selection_cpu_time -= grapher_track_at(&p->stime, 0);
        
			ratio = ((double)selection_cpu_time)/((double)tot_sys_jiffies ) ;
        
			if (((unsigned int)(ratio*100.)) > percent)
				break;      
		}
	}  
 
	free(parray);
	return dest;
}


grapher_statstore*
grapher_filter_by_avg_cputime_percent(grapher_statstore *src,
                                      float percent )
{
	unsigned int i;
	grapher_statstore *dest;
  
	assert(src);
	assert(percent);
  
	dest = (grapher_statstore*) malloc(sizeof(grapher_statstore));  
	assert(dest);
  
	grapher_statstore_init(dest);

	/* clone everything but processes entries */
	cu_vector_ptrs_clone(&dest->timeline, &src->timeline);
	grapher_sys_clone(&dest->sys, &src->sys);
	cu_bitmap_clone(&dest->ap_bitmap, &src->ap_bitmap);

	/* push in the destination store only if the avg percent constraint is satisfied */
	{
		unsigned int tot_sys_jiffies = 0;
    
		for (i =0; i< cu_vector_uint_size(&src->sys.jiffies); i++)
			tot_sys_jiffies += cu_vector_uint_at(&src->sys.jiffies, i);

		for (i = 0; i< cu_vector_ptrs_size(&src->activep); i++)
		{
			unsigned int process_cpu_time;
			unsigned int samples;
			double ratio;
			grapher_process *p;

			p = (grapher_process*)cu_vector_ptrs_at(&src->activep, i);
			assert(p);

			samples = grapher_track_size(&p->utime);
			process_cpu_time = 0;
			process_cpu_time += grapher_track_at(&p->utime, samples -1);
			process_cpu_time -= grapher_track_at(&p->utime, 0);
			process_cpu_time += grapher_track_at(&p->stime, samples -1);
			process_cpu_time -= grapher_track_at(&p->stime, 0);
        
			ratio = ((double)process_cpu_time)/((double)tot_sys_jiffies ) ;
        
			if ((ratio*100.) > percent)
				cu_vector_ptrs_push_back(&dest->activep, p);
		}
	}  
	return dest;
}


grapher_statstore*
grapher_filter_by_majflt(grapher_statstore *src,
                         unsigned int num_ps )
{
	unsigned int i;
	grapher_statstore *dest;
	grapher_process** parray;
	unsigned int num_stored_ps;
  
	assert(src);
	assert(num_ps);
  
	dest = (grapher_statstore*) malloc(sizeof(grapher_statstore));  
	assert(dest);
  
	grapher_statstore_init(dest);

	/* clone everything but processes entries */
	cu_vector_ptrs_clone(&dest->timeline, &src->timeline);
	grapher_sys_clone(&dest->sys, &src->sys);
	cu_bitmap_clone(&dest->ap_bitmap, &src->ap_bitmap);

	/* find the #num_ps processed whom sucked more cputime */
	if (cu_vector_ptrs_size(&src->activep)<= num_ps)
	{
		/* FIXME: we are copying pointers, better to clone processes */
		cu_vector_ptrs_clone(&dest->activep, &src->activep);
		return dest;
	}
  
	/* sort processes by majflt */
	num_stored_ps = cu_vector_ptrs_size(&src->activep);
	parray = (grapher_process**) malloc(sizeof(grapher_process*)
					    * cu_vector_ptrs_size(&src->activep));
	assert(parray);
  
	for (i =0; i< num_stored_ps; i++)
		parray[i] = (grapher_process*)cu_vector_ptrs_at(&src->activep, i);

	qsort(parray, num_stored_ps,
	      sizeof(grapher_process*),
	      &grapher_process_rcompare_by_majflt);
        
	if (num_ps > num_stored_ps)
		num_ps = num_stored_ps;

	/* push in the destination store only the first #num_ps processes in parray */
	for (i = 0; i<num_ps; i++)
	{
		assert(parray[i]);
		cu_vector_ptrs_push_back(&dest->activep, parray[i]);
	}
 
	free(parray);
	return dest;
}
