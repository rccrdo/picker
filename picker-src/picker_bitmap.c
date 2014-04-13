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


#include "picker_bitmap.h"


void
_picker_bitmap_grow(GByteArray *barray, unsigned int bits)
{
	assert(barray);
	unsigned int bytes;
	unsigned int i;
  
	bytes = bits / 8;
	if (bytes*8 < bits)
		bytes++;
  
	if (bytes > barray->len)
	{/* append the missing bytes */
		bytes -= barray->len;
		for (i=0; i< bytes; i++) 
		{
			const unsigned char buf[1] = {0};
			g_byte_array_append (barray, buf, 1);
		}   
	}  
}


/* picker_bitmap will be used as a binary map with pid values as indices.
   TODO: is "V pid in {`possible pid values`} => 0<=pid<=65535" true ? */
#define BITMAP_START_SIZE (unsigned int)65536 /* in bits */

GByteArray*
picker_bitmap_new(void)
{
	GByteArray* barray;
  
	barray = g_byte_array_new();
	assert(barray);
	if (!barray)
		return NULL;

	_picker_bitmap_grow(barray, BITMAP_START_SIZE);
	return barray;
}

#undef BITMAP_START_SIZE

