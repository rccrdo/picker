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

#include "grapher_process.h"


void
grapher_process_init(grapher_process *p,
                     unsigned int pid,
                     const char *cmdline,
                     unsigned int start_tick )
{
	unsigned int cmdline_len;
 
	assert(p);
	assert(cmdline);

	p->pid = pid;
	p->ppid = 0;
	p->start_tick = start_tick;

	cmdline_len = strnlen(cmdline, GRAPHER_PROCESS_CMDLINE_MAX_LEN);
	assert(cmdline_len);
	memcpy(p->cmdline, cmdline, cmdline_len);
  
	if ( (cmdline_len == GRAPHER_PROCESS_CMDLINE_MAX_LEN)
	     && cmdline[GRAPHER_PROCESS_CMDLINE_MAX_LEN-1]!=0 )
	{
		sprintf(p->cmdline + GRAPHER_PROCESS_CMDLINE_MAX_LEN -3, "..");
		p->cmdline[GRAPHER_PROCESS_CMDLINE_MAX_LEN-1] = 0;
	}
    
	/* init tracks */
	grapher_track_init(&p->minflt, "minflt");
	grapher_track_init(&p->majflt, "majflt");
	grapher_track_init(&p->utime, "utime");
	grapher_track_init(&p->stime, "stime");

	p->tot_majflt = 0;
	p->tot_minflt = 0;
  
	grapher_track_init(&p->mem_size, "size");
	grapher_track_init(&p->mem_resident, "resident");

	/* set min to 0 for nycier graphs */
	p->mem_size.min = 0;
	p->mem_resident.min = 0;
}


void
grapher_process_deinit(grapher_process *p)
{
	assert(p);
	grapher_track_deinit(&p->minflt);
	grapher_track_deinit(&p->majflt);
	grapher_track_deinit(&p->utime);
	grapher_track_deinit(&p->stime);
  
	grapher_track_deinit(&p->mem_size);
	grapher_track_deinit(&p->mem_resident);
}


void
grapher_process_put_pstat(grapher_process *p,
                          const char *buf)
{
	unsigned int minflt;
	unsigned int majflt;
	unsigned int utime;
	unsigned int stime;
  
	assert(p && buf);
  
	/* parse buf (man proc)*/
  
	/* name like  1502 (Journal <a53084) are detected badly by scanf */
	unsigned int i=0;
	int found_first = 0;

	while (buf[i]!=')')
	{
		if (buf[i]=='(')
			found_first = 1;
		if (found_first && (buf[i]==' '))
			((char*)buf)[i]='_';      
		i++;
	}
	sscanf(buf, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %u %*u %u %*u %u %u",
	       &minflt, &majflt, &utime, &stime);
         
	grapher_track_push_back(&p->utime, utime);
	grapher_track_push_back(&p->stime, stime);
  
	/* we always need at least two samples to calculate the difference */
	if (grapher_track_size(&p->minflt) == 0)
	{
		/* as we cannot say anything we set them all to 0 */
		grapher_track_push_back(&p->minflt, 0);
		grapher_track_push_back(&p->majflt, 0);
      
		/* store values for next iteration */
		p->minflt_last = minflt;
		p->majflt_last = majflt;
		return;
	}
  
	/* store faults for this iteration */
	grapher_track_push_back(&p->minflt, minflt - p->minflt_last);
	grapher_track_push_back(&p->majflt, majflt - p->majflt_last);

	p->tot_minflt = minflt;
	p->tot_majflt = majflt;

	/* store values for next iteration */
	p->minflt_last = minflt;
	p->majflt_last = majflt;
}


void
grapher_process_put_pstatm(grapher_process *p,
                           const char *buf)
{
	unsigned int size;
	unsigned int resident;
	unsigned int share;
	unsigned int res;
   
	assert(p && buf);
  
	/* parse buf (man proc)*/
	res = sscanf(buf, "%u %u %u", &size, &resident, &share);
	assert(res>=3);

	grapher_track_push_back(&p->mem_size, size);
	grapher_track_push_back(&p->mem_resident, resident);
}


void
grapher_process_slice_clone(grapher_process *dest,
                            grapher_process *src,
                            unsigned int lpos,
                            unsigned int rpos)
{
	assert(dest);
	assert(src);
	assert(lpos < rpos);
	assert(rpos < grapher_track_size(&src->utime));

/*  printf("grapher_process_slice_clone, lpos %u, rpos %u\n", lpos, rpos);*/
	dest->pid = src->pid;
	dest->ppid = src->ppid;
	memcpy(dest->name, src->name, GRAPHER_PROCESS_NAME_MAX_LEN); 
	memcpy(dest->cmdline, src->cmdline, GRAPHER_PROCESS_CMDLINE_MAX_LEN); 
  
/*  dest->start_tick = src->start_tick + lpos;*/

	grapher_track_slice_clone(&dest->minflt, &src->minflt, lpos, rpos);
	grapher_track_slice_clone(&dest->majflt, &src->majflt, lpos, rpos);
	grapher_track_slice_clone(&dest->utime, &src->utime, lpos, rpos);
	grapher_track_slice_clone(&dest->stime, &src->stime, lpos, rpos);

	dest->tot_minflt = 0;
	dest->tot_majflt = 0;
	{
		unsigned int i;
		unsigned int track_size;
    
		track_size = grapher_track_size(&dest->stime);
		for (i = 0; i< track_size; i++)
		{
			dest->tot_minflt  += grapher_track_at(&dest->minflt, i);
			dest->tot_majflt  += grapher_track_at(&dest->majflt, i);
		}    
	}


	dest->minflt_last = src->minflt_last;
	dest->majflt_last = src->majflt_last;;

	grapher_track_slice_clone(&dest->mem_size, &src->mem_size, lpos, rpos);
	grapher_track_slice_clone(&dest->mem_resident, &src->mem_resident, lpos, rpos);
}
