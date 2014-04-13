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


#include "picker_mainloop.h"

int
picker_mainloop(GByteArray *store,
                int job_length_sec,
                unsigned int freq)
{
	/* `/proc/...` stuff */
	DIR* proc_dir;
	off_t proc_1_dirent_offset;
	int proc_stat_fd;
/*int proc_meminfo_fd;*/

	/* main loop timing vars */
	unsigned int ticks; /* TODO move as static in the loop <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
	unsigned int sleep_usecs; /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
	char proc_path_buf[32]; /* ^^^^^^^^^^^^^^^^^^^ */
	struct timeval start_time;
	struct timeval last_time;
	struct timeval current_time;

	picker_procset* procset;

	ticks = 0;
	sleep_usecs = 1000000/freq;
  
	proc_path_buf[0]='/';
	proc_path_buf[1]='p';
	proc_path_buf[2]='r';
	proc_path_buf[3]='o';
	proc_path_buf[4]='c';
	proc_path_buf[5]='/';

	procset = picker_procset_new();
	if (!procset)
	{
		perror("cannot alloc the procset");
		/* ~ENOMEM */
		return -1;
	}

	/* check for /proc */
	proc_dir = opendir("/proc");
	if (!proc_dir)
	{
		perror("cannot open /proc\n");
		/* ~ENOPROC */
		return -2; 
	}

	proc_stat_fd = open("/proc/stat", O_RDONLY);
	if (proc_stat_fd == -1)
	{
		printf("cannot open /proc/stat\n");
		/* ~ENOPROC */
		return -2; 
	}
/*
  proc_meminfo_fd = open("/proc/meminfo", O_RDONLY);
  if (proc_meminfo_fd==-1)
  {
  printf("cannot open /proc/meminfo\n");*/
	/* ~ENOPROC */
/*      return -2; 
	}*/

	/* find `/proc/1`'s dirent offset; entries returned by readdir() are
	 * ordered so that text-only entries are at the top of the list */
	{
		off_t last_visited;
		struct dirent *entry;
		rewinddir(proc_dir);
		last_visited = proc_1_dirent_offset = telldir (proc_dir);

		while ((entry = readdir(proc_dir)))
		{
			if (entry->d_name[0] == '1' && entry->d_name[1] == 0)
			{
				proc_1_dirent_offset = last_visited;
				rewinddir(proc_dir);
				break;
			}
			last_visited = telldir(proc_dir);
		}
	}

	{ /* pack the uptime chunk */
		int uptime_fd;
		uptime_fd = open("/proc/uptime", O_RDONLY);
		if (uptime_fd!=-1)
		{
			picker_append_fd(store, CHUNK_T_SYS_UPTIME, 0, uptime_fd);
			close(uptime_fd);
		}
	}

	/* start sampling */
	printf(" * sampling (%dsec at %dHz) ...\n", job_length_sec, freq);
	gettimeofday(&start_time, NULL);
	memcpy(&last_time, &start_time, sizeof(struct timeval));
	do
	{
		struct dirent* entry;

		gettimeofday(&current_time, NULL);
		picker_append_timechunk(store, &current_time);
		ticks++;

		/* seek to `/proc/1'*/
		rewinddir(proc_dir);
		seekdir(proc_dir, proc_1_dirent_offset);
      
		/* look for new processes */
		while((entry=readdir(proc_dir)))
		{
			unsigned int pid, pidstr_len, i;
			int stat_fd; /*, statm_fd;*/
          
			assert((entry->d_name[0]>='0') &&
			       (entry->d_name[0]<='9'));

			/* obtain pid by converting d_name */
			pid = entry->d_name[0] - '0';
			pidstr_len = 0;
			for (i=1; i< 6; i++)
			{
				if (!(entry->d_name[i]))
				{
					pidstr_len = i;
					break;              
				}
				pid *= 10;
				pid += entry->d_name[i] - '0';
			}
			assert(pidstr_len);

			if (picker_procset_has(procset, pid))
				continue;

			/* do not count the extra NULL char */
#define SIZEOF_SLASHPROCSLASH sizeof("/proc/") -1
          
			/* build the path to `/proc/$pid/stat` in the buffer */
			memcpy(proc_path_buf + SIZEOF_SLASHPROCSLASH, 
			       entry->d_name,
			       pidstr_len);
			memcpy(proc_path_buf + SIZEOF_SLASHPROCSLASH + pidstr_len,
			       "/stat\0",
			       sizeof("/stat\0"));
			stat_fd = open(proc_path_buf, O_RDONLY);
			if (stat_fd!=-1)
			{
				/* build the path to `/proc/$pid/statm` in the buffer */
/*              memcpy(proc_path_buf + SIZEOF_SLASHPROCSLASH + pidstr_len,
		"/statm\0",
		sizeof("/statm\0"));
		statm_fd = open(proc_path_buf, O_RDONLY);
		if (statm_fd!=-1)
                {*/
				int cmdline_fd;
				/* build the path to `/proc/$pid/cmdline` in the buffer */
				memcpy(proc_path_buf + SIZEOF_SLASHPROCSLASH + pidstr_len,
				       "/cmdline\0",
				       sizeof("/cmdline\0"));
				cmdline_fd = open(proc_path_buf, O_RDONLY);
				if (cmdline_fd!=-1)
				{
					picker_append_fd(store, CHUNK_T_PROCESS_CMDLINE,
							 pid, cmdline_fd);

					assert(!picker_procset_has(procset, pid));
					picker_procset_add(procset, pid, stat_fd/*, statm_fd*/);
					assert(picker_procset_has(procset, pid));
					close(cmdline_fd);
				}
				else 
				{
					close(stat_fd);
/*                      close(statm_fd);*/
				}
/*                }
		  else 
		  close(stat_fd);*/
			}
#undef SIZEOF_SLASHPROCSLASH
		}
        
		/* push /proc/stat{m} */
		picker_append_fd(store, CHUNK_T_SYS_STAT, 0, proc_stat_fd);
/*        picker_append_fd(&store, CHUNK_T_SYS_MEMINFO, 0, proc_meminfo_fd);*/


		{ /* gather the boring stats for every process in procset */
			unsigned int set_size, i;
			i = 0;
			set_size = picker_procset_size(procset);
			while (i < set_size)
			{
				unsigned int err1;/*, err2;*/
				picker_process* proc;
				proc = picker_procset_at(procset, i);
				assert(proc);
				assert(picker_procset_has(procset, proc->pid));
				err1 = picker_append_fd(store, CHUNK_T_PROCESS_STAT,
							proc->pid, proc->stat_fd);
				if (!err1)
				{
/*                  err2 = picker_append_fd(store, CHUNK_T_PROCESS_STATM,
		    proc->pid, proc->statm_fd);
		    if (!err2)
                    {*/
					i++;
					continue;
/*                    }*/
				}                       
				close(proc->stat_fd);
/*              close(proc->statm_fd);*/
				picker_procset_remove(procset, i);
				set_size --;
				assert (set_size == picker_procset_size(procset));
			}
		}

		/* the naive frequency regulation algorithm */
		if (current_time.tv_sec > last_time.tv_sec)
		{
			if (current_time.tv_sec == (last_time.tv_sec+1))
			{
				sleep_usecs = ((sleep_usecs*ticks)/freq);   
				/* avoid sleep_usecs blocking to 0 */
				if (!sleep_usecs)
					sleep_usecs = 50;
			}
			else 
				printf("warning, I'm having troubles to work at this frequency (%dHz)\n", freq);
			ticks = 0;
		}
      
		/* copy for next iteration */
		last_time.tv_sec = current_time.tv_sec;
		last_time.tv_usec = current_time.tv_usec;
		usleep(sleep_usecs);
	} while (picker_sec_passed(&start_time, &current_time) < job_length_sec);

	/* close open streams */
	closedir(proc_dir);
	close(proc_stat_fd);
/*close(proc_meminfo_fd);*/
	picker_procset_free(procset);
	return 0; 
}
