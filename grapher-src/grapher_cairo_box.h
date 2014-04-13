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

#ifndef grapher_cairo_box_h
#define grapher_cairo_box_h 1

#include <cairo.h>


#define _GRAPHER_CAIRO_BOX_PAD_MULTIPLIER (double)0.1
#define _GRAPHER_CAIRO_BOX_COUNTOUR_MULTIPLIER (double)0.15
#define _GRAPHER_CAIRO_BOX_COUNTOUR_LINE_WIDTH_MULTIPLIER (double)0.05
#define _GRAPHER_CAIRO_BOX_THTXT_SIZE_MULTIPLIER (double)0.1
#define _GRAPHER_CAIRO_BOX_THTXT_X_COORD_MULTIPLIER (double)0.1
#define _GRAPHER_CAIRO_BOX_THTXT_Y1_COORD_MULTIPLIER (double)0.1
#define _GRAPHER_CAIRO_BOX_THTXT_Y2_COORD_MULTIPLIER (double)0.9
#define _GRAPHER_CAIRO_BOX_TXT_SIZE_MULTIPLIER (double)0.2
#define _GRAPHER_CAIRO_BOX_NAME_X_MULTIPLIER (double)0.2
#define _GRAPHER_CAIRO_BOX_NAME_Y_MULTIPLIER (double)0.2

/*
  ---------------------------
  |    |           p   xf,yf|
  |    |    ------------    |
  | np | p |   h        | p |
  |    |    ------------    |
  |    | xi,yi    p         |
  ---------------------------
  surf
*/

	struct grapher_cairo_box {
		double np;	          /* horizontal padding for a name */
		double gh,gw;           /* width and height for the graphicable area */
		double p;               /* horizontal and vertical padding for the graphicable area */
		double h,w;             /* width and height for the whole surface */
		double xi, yi;          /* starting coords for the graphicable area */
		double xf, yf;          /* ending coords for the graphicable area */
		double cradius;	  /* countur radius */
		double clw;       	  /* countur line width */
		double thtxt_size;      /* thresholds text size */
		double thx;             /* x coord for threshold texts (when used) */
		double th1y, th2y;      /* coords for upper threshold text (when used) */
		double txt_size;        /* other text size */
		double name_x, name_y;  /* coords for the name text */
		cairo_surface_t * surf; /* we put everything inside here */
	};


inline void grapher_cairo_box_init_simple_track(struct grapher_cairo_box* box,
                                                struct grapher_draw_opt* opt,
                                                double sec_len);

inline void grapher_cairo_box_init_sys_cpu_track(struct grapher_cairo_box* box,
                                                 struct grapher_draw_opt* opt,
                                                 double sec_len);

void grapher_cairo_box_init_simple_track(struct grapher_cairo_box* box,
                                         struct grapher_draw_opt* opt,
                                         double sec_len)
{
	assert(box && opt);
	assert(!box->surf); /* force users of the struct to free surf */

	box->np = 0;
	box->gh = opt->sth;
	box->gw = sec_len*opt->pps;
	box->p = opt->sth*_GRAPHER_CAIRO_BOX_PAD_MULTIPLIER;
	box->h = box->gh + box->p*2.;
	box->w = box->gw + box->p*2.;

	box->xi = box->p;
	box->yi = box->p + box->gh;
  
	box->xf = box->p + box->gw;
	box->yf = box->p;
	box->cradius = box->gh*_GRAPHER_CAIRO_BOX_COUNTOUR_MULTIPLIER;
	box->clw = box->gh*_GRAPHER_CAIRO_BOX_COUNTOUR_LINE_WIDTH_MULTIPLIER;
	box->thtxt_size = box->gh*_GRAPHER_CAIRO_BOX_THTXT_SIZE_MULTIPLIER;
	box->thx = box->xi + box->gh*_GRAPHER_CAIRO_BOX_THTXT_X_COORD_MULTIPLIER;
	box->th1y = box->p + box->gh*_GRAPHER_CAIRO_BOX_THTXT_Y1_COORD_MULTIPLIER;
	box->th2y = box->p + box->gh*_GRAPHER_CAIRO_BOX_THTXT_Y2_COORD_MULTIPLIER;
	box->txt_size = box->gh*_GRAPHER_CAIRO_BOX_TXT_SIZE_MULTIPLIER;
	box->name_x = box->xi + box->gh*_GRAPHER_CAIRO_BOX_NAME_X_MULTIPLIER;
	box->name_y = box->p + box->gh*_GRAPHER_CAIRO_BOX_NAME_Y_MULTIPLIER;
	box->surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)box->w, (int)box->h);
	assert(box->surf);
}

void grapher_cairo_box_init_sys_cpu_track(struct grapher_cairo_box* box,
                                          struct grapher_draw_opt* opt,
                                          double sec_len)
{
	assert(box && opt);
	assert(!box->surf); /* force users of the struct to free surf */

	box->np = 0;
	box->gh = opt->sth*2.5;
	box->gw = sec_len*opt->pps;
	box->p = opt->sth*_GRAPHER_CAIRO_BOX_PAD_MULTIPLIER;
	box->h = box->gh + box->p*2.;
	box->w = box->gw + box->p*2.;

	box->xi = box->p;
	box->yi = box->p + box->gh;
  
	box->xf = box->p + box->gw;
	box->yf = box->p;
	box->cradius = box->gh*_GRAPHER_CAIRO_BOX_COUNTOUR_MULTIPLIER;
	box->clw = box->gh*_GRAPHER_CAIRO_BOX_COUNTOUR_LINE_WIDTH_MULTIPLIER;
	box->thtxt_size = box->gh*_GRAPHER_CAIRO_BOX_THTXT_SIZE_MULTIPLIER;
	box->thx = box->xi + box->gh*_GRAPHER_CAIRO_BOX_THTXT_X_COORD_MULTIPLIER;
	box->th1y = box->p + box->gh*_GRAPHER_CAIRO_BOX_THTXT_Y1_COORD_MULTIPLIER;
	box->th2y = box->p + box->gh*_GRAPHER_CAIRO_BOX_THTXT_Y2_COORD_MULTIPLIER;
	box->txt_size = box->gh*_GRAPHER_CAIRO_BOX_TXT_SIZE_MULTIPLIER;
	box->name_x = box->xi + box->gh*_GRAPHER_CAIRO_BOX_NAME_X_MULTIPLIER;
	box->name_y = box->p + box->gh*_GRAPHER_CAIRO_BOX_NAME_Y_MULTIPLIER;
	box->surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)box->w, (int)box->h);
	assert(box->surf);
}

#endif /* grapher_cairo_box_h */

