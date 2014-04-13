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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>

#include "picker_public__frame_info.h"

#include "grapher_debug.h"
#include "grapher_statstore.h"
#include "grapher_cairo.h"
#include "grapher_draw.h"
#include "grapher_filters.h"

static void print_help()
{
	printf("Usage: grapher [options]\n");
	printf("Graphs picker's statistics to pretty figures.\n\n");
	printf(" -c  --filter-cputime=<num_ps>\tenable filtering by cpu-time\n");
	printf(" -a  --filter-avgcputime=<avg_percent>\tenable filtering by minimum avg cpu-time\n");
	printf(" -p  --filter-cputime-percent=<percent>\tenable filtering by cpu-time\n");
	printf(" -m  --filter-majflt=<num_ps>\tenable filtering by major faults\n");
	printf(" -l  --ltrim_time=<seconds>\toutput only data since this time\n");
	printf(" -r  --rtrim_time=<seconds>\toutput only data until this time\n");
	printf(" -i  --fin=<str>\tset the path to the processes blacklist file\n");
	printf(" -o  --fout=<str>\tset the path for the output file\n");
	printf(" -s  --show=[cpu,timings]\tprints a statistics resume; args must be comma separated!\n");
	printf(" -n  --no-draw\tdon't output any drawings\n");
	printf(" -h  --help\t\tshows this help\n");
}


int main (int argc, char** argv)
{
	unsigned int buffer_used;
	unsigned int opt_filter_by_cpu_time_numps;
	unsigned int opt_filter_by_cpu_time_percent;
	unsigned int opt_filter_by_majflt_numps;
	unsigned int opt_print_stats_flags;
	unsigned int opt_no_draw;
	float opt_filter_by_avg_cpu_time_percent;
	float opt_filter_time_rtrim;
	float opt_filter_time_ltrim;
  
	char buffer[PICKER_FRAME_MAX_SIZE];
	char fin_path[256] = "stats.picker";
	char fout_path[256] = "\0";
	FILE* fin;
	grapher_statstore statstore;

	opt_filter_by_cpu_time_numps = 0;
	opt_filter_by_avg_cpu_time_percent = 0.;
	opt_filter_time_rtrim = 0.;
	opt_filter_time_ltrim = 0.;
	opt_filter_by_cpu_time_percent = 0;
	opt_filter_by_majflt_numps = 0;
	opt_print_stats_flags = 0;
	opt_no_draw = 0;

	/* get options */
	while (1)
	{
		int c;
		int option_index;
		static struct option long_options[] =
			{
				{"filter-cputime", required_argument, 0, 'c'},
				{"filter-avgcputime", required_argument, 0, 'a'},
				{"time-ltrim", required_argument, 0, 'l'},
				{"time-rtrim", required_argument, 0, 'r'},
				{"filter-cputime-percent", required_argument, 0, 'p'},
				{"filter-majflt", required_argument, 0, 'm'},
				{"fin", required_argument, 0, 'i'},
				{"fout", required_argument, 0, 'o'},
				{"show", required_argument, 0, 's'},
				{"no-draw", no_argument, 0, 'n'},
				{"help", no_argument, 0, 'h'},
				{0, 0, 0, 0}
			};

		option_index = 0;
		c = getopt_long (argc,
				 argv,
				 "c:a:p:m:i:o:s:r:l:hn",
				 long_options,
				 &option_index);
     
		/* check for the end of options */
		if (c == -1)
			break;
     
		switch (c)
		{
		case 'h':
		{
			print_help();
			return 1;
		}
		case 'c':
		{
			unsigned int value;
			sscanf(optarg, "%u", &value);
			if (value)
				opt_filter_by_cpu_time_numps = value;
			break;
		}
		case 'a':
		{
			float value;
			sscanf(optarg, "%f", &value);
			if (value)
				opt_filter_by_avg_cpu_time_percent = value;
			break;
		}
		case 'l':
		{
			float value;
			sscanf(optarg, "%f", &value);
			opt_filter_time_ltrim = abs(value);
			break;
		}
		case 'r':
		{
			float value;
			sscanf(optarg, "%f", &value);
			opt_filter_time_rtrim = abs(value);
			break;
		}
		case 'p':
		{
			unsigned int value;
			sscanf(optarg, "%u", &value);
			if (value)
				opt_filter_by_cpu_time_percent = value;
			break;
		}
		case 'm':
		{
			unsigned int value;
			sscanf(optarg, "%u", &value);
			if (value)
				opt_filter_by_majflt_numps = value;
			break;
		}
		case 'i':
		{
			strcpy(fin_path, optarg);
			break;
		}            
		case 'o':
		{
			strncpy(fout_path, optarg, 256);
			break;
		}            
		case 's':
		{
			if (strstr(optarg, "cpu"))
				opt_print_stats_flags |= GRAPHER_STATSTORE_STATS_FLAGS_CPU;
			if (strstr(optarg, "faults"))
				opt_print_stats_flags |= GRAPHER_STATSTORE_STATS_FLAGS_FAULTS;
			if (strstr(optarg, "timings"))
				opt_print_stats_flags |= GRAPHER_STATSTORE_STATS_FLAGS_TIMINGS;
			if (strstr(optarg, "all"))
				opt_print_stats_flags = 0xFFFFFFFF;
			break;
		}   
		case 'n':
		{
			opt_no_draw = 1;
			break;
		}   
		default:
		{
			print_help();
			return 1;
		}
           
		}
	}

	/* check filters */
	assert(!(opt_filter_by_cpu_time_numps && opt_filter_by_cpu_time_percent));

	/* open picker file */
	fin = fopen(fin_path, "r");
	if (!fin)
	{
		char buf[300];
		sprintf(buf, "grapher: %s", fin_path);
		perror(buf);
		return 1; 
	}  
    
	/* init the statstore */
	grapher_statstore_init(&statstore);  

	/* extract frames from blob */
	while(1)
	{
		unsigned int res;
		picker_frame_info frame_info;
		memset(&frame_info, 0, sizeof(picker_frame_info));
      
		res = fread(&frame_info, 1, sizeof(picker_frame_info), fin);

		if (!res)  /* end of file */
			break;
        
		if (res < sizeof(picker_frame_info))
		{
			printf("warning it seems the .picker file is corrupted; will show"
			       " only data loaded until this point\n");
			break;
		}
        
		if (!picker_frame_info_is_valid(&frame_info))
		{
			printf("warning frames in the .picker file are corrupted; will show"
			       " only data loaded until this point\n");
			break;
		}

		/* load frame data into the buffer and null terminate it*/
		buffer_used = 0;
		buffer[0]= 0;
		if (frame_info.blob_len);
		{
			buffer_used = fread(buffer, 1, frame_info.blob_len, fin);
			buffer[buffer_used] = 0;
		}

		if (buffer_used < frame_info.blob_len)
		{
			printf("warning it seems a data section in the .picker file is corrupted;"
			       " will show only data loaded until this point\n");
			break;
		}

		switch (frame_info.type)
		{
		case(FRAME_CONTENT_T_TIME): 
		{
			assert(buffer_used == sizeof(struct timeval));
			grapher_statstore_tick(&statstore, buffer);              
			break;
		}
		case(FRAME_CONTENT_T_PROC_STAT):
		{
			grapher_statstore_put_stat(&statstore, buffer);
			break;
		}            
		case(FRAME_CONTENT_T_PROC_PSTAT): 
		{
			grapher_statstore_put_pstat(&statstore, frame_info.pid, buffer);
			break;
		}
		case(FRAME_CONTENT_T_PROC_PSTATM):
		{
			grapher_statstore_put_pstatm(&statstore, frame_info.pid, buffer);
			break;
		}
		case(FRAME_CONTENT_T_PROC_PCMDLINE):
		{
			/* cmdline has a bogus format */
			int i = frame_info.blob_len -1;
			while(i--)
				if(buffer[i]<' ' || buffer[i]>'~')
					buffer[i]=' ';
       
			grapher_statstore_put_pcmdline(&statstore, frame_info.pid, buffer);
			break;
		}            
		case(FRAME_CONTENT_T_PROC_KTHREAD_ALERT):
		{
			grapher_statstore_put_kthread_alert(&statstore, frame_info.pid);
			break;
		}
		default: printf("warning, unknown frame type in the blob; I guess I'll crash  :/\n");
		}
	}

	fclose(fin);
  
	/* draw the figure */
	{
		grapher_statstore* time_filtered_store;
		grapher_statstore* store;
    
		time_filtered_store = &statstore;
		store = &statstore;

		/* precedence to time filters */
		{
			float sampling_time;
			sampling_time = grapher_statstore_get_sampling_time(&statstore);
			/* TODO calc the smallest permitted window based on sampling frequency */
			if (sampling_time - opt_filter_time_ltrim - opt_filter_time_rtrim < 1.)
			{
				printf("time trim filter: choosen time window (l=%.1fs, r=%.1fs) is "
				       "too small (sampling time=%.3f); discarding this filter ...\n",
				       opt_filter_time_ltrim,
				       opt_filter_time_rtrim,
				       sampling_time );
				opt_filter_time_ltrim = 0.;
				opt_filter_time_rtrim = 0.;
			}

			if (opt_filter_time_ltrim || opt_filter_time_rtrim)
			{
				time_filtered_store = grapher_filter_time_trim(&statstore, opt_filter_time_ltrim, opt_filter_time_rtrim);
				store = time_filtered_store;
			}
		}

		if (opt_filter_by_cpu_time_numps)
			store = grapher_filter_by_cputime(time_filtered_store, opt_filter_by_cpu_time_numps);  
		else if (opt_filter_by_avg_cpu_time_percent)
			store = grapher_filter_by_avg_cputime_percent(time_filtered_store, opt_filter_by_avg_cpu_time_percent);
		else if (opt_filter_by_cpu_time_percent)
			store = grapher_filter_by_cputime_percent(time_filtered_store, opt_filter_by_cpu_time_percent);
		else if (opt_filter_by_majflt_numps)
			store = grapher_filter_by_majflt(time_filtered_store, opt_filter_by_majflt_numps);
      
		assert(store);

		if (opt_print_stats_flags)
			grapher_statstore_print_stats(store, opt_print_stats_flags);

		if (!opt_no_draw)
		{
			/* init drawing preferencies to default */
			grapher_draw_opt opt;
			grapher_draw_opt_default(&opt);

			if (!*fout_path)
				sprintf(fout_path, "%s.svg", fin_path);
  
			if (grapher_cairo_draw(store, &opt, fout_path))
				printf(" - graph written to %s\n", fout_path);
			else
				printf("! errors occured while drawing\n");
		}
	}
  
	grapher_statstore_deinit(&statstore);
  
	return 0;
}

