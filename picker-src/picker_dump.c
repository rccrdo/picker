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


#include "picker_dump.h"

#define DEFAULT_OUTPUT_PATH "data.picker"

void
picker_dump(GByteArray *store,
            const char *path,
            int zip_it)
{
	char* real_path;
	FILE* stream;
	int popened_stream;
  
	assert(store);
	stream = NULL;
	real_path = NULL;
	popened_stream = 0;

	if (!path)
		real_path = strdup(DEFAULT_OUTPUT_PATH);
	else
		real_path = strdup(path);
	if (!real_path)
	{
		perror(":/");
		return;
	}

	if (zip_it)
	{
		char* cmd;
		cmd = g_strdup_printf ("gzip -9 > %s.gz", real_path);
		if (!cmd)
		{
			perror(":/_2");
			return;
		}
		stream = popen(cmd, "w"); 
		popened_stream = 1;
	}
	if (!zip_it || !stream)
	{
		stream = fopen(real_path, "w");
		popened_stream = 0;
	}
	free (real_path);
	if (!stream) 
	{
		perror(":/_3");
		return;
	}

	/* unroll the blob into a (almost) human readable format */
	printf(" * dumping data ..\n");
	unsigned int copied = 0;
	while (copied!=store->len)
	{
		picker_chunk_header header;

		assert(store->len >= (copied + sizeof(picker_chunk_header)));
		memcpy(&header, store->data + copied, sizeof(picker_chunk_header));
		copied += sizeof(picker_chunk_header);
		assert(store->len >= (copied + header.blob_size));

		switch (header.type)
		{
		case(CHUNK_T_SYS_TIME): 
		{
			double v;
			char* str;
			struct timeval time;
			fwrite("chunk_t SYS_TIME\n", 1, sizeof("chunk_t SYS_TIME\n") -1, stream);
			memcpy(&time, store->data + copied, sizeof(struct timeval));
			copied += sizeof(struct timeval);
			v = (double)time.tv_sec + ((double)time.tv_usec) /1000000.;
			str = g_strdup_printf("%f\n\n", v);
			fwrite(str, 1, strlen(str), stream);
			free (str);              
			break;
		}
		case(CHUNK_T_SYS_STAT):
		{
			fwrite("chunk_t SYS_STAT\n", 1, sizeof("chunk_t SYS_STAT\n") -1, stream);
			fwrite(store->data + copied, 1, header.blob_size, stream);
			copied += header.blob_size;
			fwrite("\n", 1, 1, stream);
			break;
		}
		case(CHUNK_T_PROCESS_STAT): 
		{
			char* str;
			fwrite("chunk_t PROCESS_STAT\n", 1, sizeof("chunk_t PROCESS_STAT\n") -1, stream);
			str = g_strdup_printf("pid     %d\n", header.src);
			fwrite(str, 1, strlen(str), stream);
			free (str);

			fwrite(store->data + copied, 1, header.blob_size, stream);
			copied += header.blob_size;
			fwrite("\n", 1, 1, stream);
			break;
		}
/*          case(CHUNK_T_PROCESS_STATM):
            {
	    char* str;
	    fwrite("chunk_t PROCESS_STATM\n", 1, sizeof("chunk_t PROCESS_STATM\n") -1, stream);
	    str = g_strdup_printf("pid     %d\n", header.src);
	    fwrite(str, 1, strlen(str), stream);
	    free (str);

	    fwrite(store->data + copied, 1, header.blob_size, stream);
	    copied += header.blob_size;
	    fwrite("\n", 1, 1, stream);
	    break;
            }*/
		case(CHUNK_T_PROCESS_CMDLINE):
		{
			int i;
			char* str;
			fwrite("chunk_t PROCESS_CMDLINE\n", 1, sizeof("chunk_t PROCESS_CMDLINE\n") -1, stream);
			str = g_strdup_printf("pid     %d\n", header.src);
			fwrite(str, 1, strlen(str), stream);
			free (str);
 
			/* /proc/$pid/cmdline has a bogus format */
			for (i=copied; i< copied + header.blob_size; i++)
				if(store->data[i]<' ' || store->data[i]>'~')
					store->data[i]=' ';

			fwrite(store->data + copied, 1, header.blob_size, stream);
			copied += header.blob_size;
			/* /proc/$pid/cmdline has no trailing \n */
			fwrite("\n\n", 1, 2, stream);
			break;
		}
		case(CHUNK_T_SYS_UPTIME):
		{
			fwrite("chunk_t SYS_UPTIME\n", 1, sizeof("chunk_t SYS_UPTIME\n") -1, stream);
			fwrite(store->data + copied, 1, header.blob_size, stream);
			copied += header.blob_size;
			fwrite("\n", 1, 1, stream);
			break;
		}
		default: 
			printf("warning, unknown frame type in the blob; I guess I'll crash  :/\n");
			assert(0);
		}
	}
    
	if (popened_stream)
		pclose(stream);
	else fclose(stream);
}

#undef DEFAULT_OUTPUT_PATH

