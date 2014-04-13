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


#ifndef picker_bitmap_h
#define picker_bitmap_h

#include <stdio.h>

#include "picker_debug.h"
#include <glib/garray.h>

GByteArray*
picker_bitmap_new(void);

void
_picker_bitmap_grow(GByteArray *barray, unsigned int bits);


__attribute__((always_inline)) extern inline unsigned int
picker_bitmap_size(GByteArray* barray)
{
	assert(barray);
	return barray->len*8;
}


__attribute__((always_inline)) extern inline unsigned int
picker_bitmap_get_bit(GByteArray *barray, unsigned int at)
{
	assert(barray);
	if (at >= picker_bitmap_size(barray))
	{
		_picker_bitmap_grow(barray, at);
		printf ("picker_bitmap_get_bit, grow! (at = %u)\n", at);
		/* we growed the bitmap with zeros, so.. */
		return 0;
	}
/*  return ((unsigned int*)barray->data)[at/(sizeof(unsigned int)*8)] & (0x1 << (at%(sizeof(unsigned int)*8)));*/
	return ((((unsigned int*)barray->data)[at/(sizeof(unsigned int)*8)]) >> (at%(sizeof(unsigned int)*8))) & 0x1;
}


__attribute__((always_inline)) extern inline void
picker_bitmap_set_bit(GByteArray *barray, unsigned int at)
{
	assert(barray);
	if (at >= picker_bitmap_size(barray))
		_picker_bitmap_grow(barray, at);

	((unsigned int*)barray->data)[at/(sizeof(unsigned int)*8)] |= (0x1 << (at%(sizeof(unsigned int)*8)));
	assert( ((((unsigned int*)barray->data)[at/(sizeof(unsigned int)*8)]) >> (at%(sizeof(unsigned int)*8))) & 0x1 );
}


__attribute__((always_inline)) extern inline void
picker_bitmap_clear_bit(GByteArray *barray, unsigned int at)
{
	assert(barray);
	if (at >= picker_bitmap_size(barray))
	{
		_picker_bitmap_grow(barray, at);
		return;
	}

	((unsigned int*)barray->data)[at/(sizeof(unsigned int)*8)] &= ~(0x1 << (at%(sizeof(unsigned int)*8)));
}

#endif /* picker_bitmap_h */

