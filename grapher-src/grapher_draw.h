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

#ifndef grapher_draw_h
#define grapher_draw_h 1

#include <cairo.h>

#include "grapher_debug.h"

#define _GRAPHER_DRAW_OPT_DEF_PPS (double)50
#define _GRAPHER_DRAW_OPT_DEF_SIMPLE_TRACK_HEIGHT (double)40

/* grapher_rect:
  
   (0,0)            (xf,yf)
   -------------------*
   |                  |
   y |                  |
   |                  |
   |                  |
   *-------------------
   (xi,yi)
   x
*/
              
typedef struct {
	double xi,yi;
	double xf,yf;
} grapher_rect;  
              
typedef struct {
	double pps;       /* points per second */
	double sth;       /* simple track height in points */
} grapher_draw_opt;


inline void grapher_draw_opt_default (grapher_draw_opt* opt);


extern inline
void grapher_draw_opt_default (grapher_draw_opt* opt)
{
	assert(opt);

	opt->pps = _GRAPHER_DRAW_OPT_DEF_PPS;
	opt->sth = _GRAPHER_DRAW_OPT_DEF_SIMPLE_TRACK_HEIGHT;
}


#endif /* grapher_cairo_opt_h */

