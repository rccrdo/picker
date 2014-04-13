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


#include "picker_append_helpers.h"

#define READ_CHUNK_SIZE (int)10000

int
picker_append_fd(GByteArray *barray,
                 picker_chunk_t chunk_t,
                 int src,
                 int fd)
{
	guint barray_orig_len;
	off_t seek_cur;
	ssize_t file_size;
	picker_chunk_header header;
  
	assert(barray);
	assert(CHUNK_T_IS_VALID(chunk_t));
	assert(fd!=-1);

	seek_cur = lseek(fd, 0, SEEK_SET); 
	if (seek_cur==-1)
		/* ~ESEEK */
		return -1;

	/* cache barray len in case something goes wrong later */
	barray_orig_len = barray->len;

	header.type = chunk_t;
	header.src = src;
	/*    .blob_len ? we don't know it now */
	g_byte_array_append(barray, (const guint8*) &header, sizeof(picker_chunk_header));

  
	file_size = 0;
	while (1)
	{
		static char buf[READ_CHUNK_SIZE];
		ssize_t n;

		n = read(fd, buf, READ_CHUNK_SIZE);
		if (n==-1)
		{
			if (errno==EINTR)
				continue;
			else
				/* ~ESOMETHING */
				g_byte_array_remove_range(barray, barray_orig_len, 
							  sizeof(picker_chunk_header) + file_size);
			return -3;
		}
		else if (!n) /* EOF */
			break;

		g_byte_array_append(barray, (const guint8*)buf, n);
		file_size += n;
	}

	if (!file_size) /* check for empty files */
	{
		/* ~EEMPTYFILE */
		g_byte_array_remove_range(barray, barray_orig_len, sizeof(picker_chunk_header));
		return -4;
	}

	((picker_chunk_header*) &(barray->data[barray_orig_len]))->blob_size = file_size;
	return 0;
}

#undef READ_CHUNK_SIZE


void
picker_append_timechunk(GByteArray *barray, struct timeval *time)
{
	static picker_chunk_header header = {CHUNK_T_SYS_TIME, 0, sizeof(struct timeval)};
  
	assert(barray);
	assert(time);
	g_byte_array_append(barray, (const guint8*)&header, sizeof(picker_chunk_header));
	g_byte_array_append(barray, (const guint8*)time, sizeof(struct timeval));
}

