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

#include <unistd.h>
#include <stdio.h>
#include <getopt.h>

#include "picker_mainloop.h"
#include "picker_dump.h"

#include <glib/garray.h>

#define MIN_FREQUENCY 2
#define MAX_FREQUENCY 20

static void
print_help()
{
	printf("Usage: picker [options]\n");
	printf("Gathers system and processes statistics (cpu/mem/io) from /proc.\n\n");
	printf(" -h  --help\t\tshows this help\n");
	printf(" -n  --fork\t\tfork ASAP and continue on child process\n");
	printf("     --nozip\t\tdon't zip output data\n");
	printf(" -t  --time=<seconds>\tset the sampling time\n");
	printf(" -f  --freq=<hertz>\tset the sampling frequency\n");
	printf(" -o  --of=<str>\tset the path for the output file\n");
}


int main (int argc, char** argv)
{
	/* cli settable options */
	long job_length_sec;
	unsigned int freq;
	static int zip_it_flag;

	GByteArray* memstore;
	char* ofile = NULL;

	/* set defaults */
	job_length_sec = 30;
	freq = 10;
	zip_it_flag = 1;
  
	/* get options */
	while (1)
	{
		int opt;
		int optindex;
		static struct option long_opts[] =
			{
				{"nozip", no_argument, &zip_it_flag, 0},
				{"fork", no_argument, 0, 'n'},
				{"help", no_argument, 0, 'h'},
				{"time", required_argument, 0, 't'},
				{"freq", required_argument, 0, 'f'},
				{"of", required_argument, 0, 'o'},
				{0, 0, 0, 0}
			};

		optindex = 0;
		opt = getopt_long (argc, argv, "nt:f:o:h", long_opts, &optindex);
		if (opt == -1) /* no more options */
			break;
  
		switch (opt)
		{
		case 0:
			/* this is needed for flag options like zip_it_flag */
			break;
		case 'n':
			if (fork())
				return 0;
			break;
		case 'f':
			sscanf(optarg, "%u", &freq);
			if (freq < MIN_FREQUENCY)
			{
				printf("warning, frequency set to %dHz (was %d)\n", MIN_FREQUENCY, freq);
				freq = MIN_FREQUENCY;
			}
			else if (freq > MAX_FREQUENCY)
			{
				printf("warning, frequency set to %dHz (was %d)\n", MAX_FREQUENCY, freq);
				freq = MAX_FREQUENCY;
			}
			break;
		case 't':
		{
			unsigned int time;
			sscanf(optarg, "%u", &time);
			if (time)
				job_length_sec = time;
			break;
		}
		case 'o':
			ofile = strdup (optarg);
			break;
		case 'h':
			print_help();
			return 1;       
		default:
			print_help();
			return 1;
		}
	}

	memstore = g_byte_array_sized_new(10000000); /* avoid many reallocs */
	/* start sampling */
	picker_mainloop(memstore, job_length_sec, freq);
	picker_dump(memstore, ofile, zip_it_flag);
	g_byte_array_free(memstore, TRUE);

	return 0;
}

