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

#include "grapher_sys.h"

void
grapher_sys_init(grapher_sys *sys)
{
	assert(sys);
  
	grapher_track_init(&sys->cpu_us, "user");
	grapher_track_init(&sys->cpu_ni, "nice");
	grapher_track_init(&sys->cpu_sy, "sys");
	grapher_track_init(&sys->cpu_id, "idle");
	grapher_track_init(&sys->cpu_wa, "io-wait");
	grapher_track_init(&sys->cpu_hi, "hw-irq");
	grapher_track_init(&sys->cpu_si, "sw-irq");
	grapher_track_init(&sys->cpu_st, "st");

	cu_vector_uint_init(&sys->jiffies);
}


void
grapher_sys_deinit(grapher_sys *sys)
{
	assert(sys);
  
	grapher_track_deinit(&sys->cpu_us);
	grapher_track_deinit(&sys->cpu_ni);
	grapher_track_deinit(&sys->cpu_sy);
	grapher_track_deinit(&sys->cpu_id);
	grapher_track_deinit(&sys->cpu_wa);
	grapher_track_deinit(&sys->cpu_hi);
	grapher_track_deinit(&sys->cpu_si);
	grapher_track_deinit(&sys->cpu_st);
  
	cu_vector_uint_deinit(&sys->jiffies);
}


void
grapher_sys_cpu_push_back(grapher_sys *sys,
                          unsigned int us,
                          unsigned int ni,
                          unsigned int sy,
                          unsigned int id,                                                 
                          unsigned int wa,                                                
                          unsigned int hi,                                               
                          unsigned int si,                                              
                          unsigned int st )
{
	unsigned int i, dus, dni, dsy, did, dwa, dhi, dsi, dst, tot;
	double vscale;

	assert(sys);
  
	/* we always need at least two samples to calculate percentuages */
	i = grapher_track_size(&sys->cpu_us);
	if (i == 0)
	{
		/* as we cannot say anything we set them all to 0 */
		grapher_track_push_back(&sys->cpu_us, 0);
		grapher_track_push_back(&sys->cpu_ni, 0);
		grapher_track_push_back(&sys->cpu_sy, 0);
		grapher_track_push_back(&sys->cpu_id, 0);
		grapher_track_push_back(&sys->cpu_wa, 0);
		grapher_track_push_back(&sys->cpu_hi, 0);
		grapher_track_push_back(&sys->cpu_si, 0);
		grapher_track_push_back(&sys->cpu_st, 0);
      
		cu_vector_uint_push_back(&sys->jiffies, 0);

		/* store values for next iteration */
		sys->cpu_us_last = us;
		sys->cpu_ni_last = ni;
		sys->cpu_sy_last = sy;
		sys->cpu_id_last = id;
		sys->cpu_wa_last = wa;
		sys->cpu_hi_last = hi;
		sys->cpu_si_last = si;
		sys->cpu_st_last = st;
		return;
	}
  
	/* change jiffies to percentuages */  
	dus = us - sys->cpu_us_last;
	dni = ni - sys->cpu_ni_last;
	dsy = sy - sys->cpu_sy_last;
	did = id - sys->cpu_id_last;
	dwa = wa - sys->cpu_wa_last;
	dhi = hi - sys->cpu_hi_last;
	dsi = si - sys->cpu_si_last;
	dst = st - sys->cpu_st_last;
	tot = dus + dsy + dni + did + dwa + dhi + dsi + dst;
  
	/* save jiffies for this iteration */
	cu_vector_uint_push_back(&sys->jiffies, tot);

	if (tot == 0)
		vscale = 0.;
	else
		vscale = 100. / ((double)tot);

	grapher_track_push_back(&sys->cpu_us, (unsigned int) (((double)dus) *vscale) );
	grapher_track_push_back(&sys->cpu_ni, (unsigned int) (((double)dni) *vscale) );
	grapher_track_push_back(&sys->cpu_sy, (unsigned int) (((double)dsy) *vscale) );
	grapher_track_push_back(&sys->cpu_id, (unsigned int) (((double)did) *vscale) );
	grapher_track_push_back(&sys->cpu_wa, (unsigned int) (((double)dwa) *vscale) );
	grapher_track_push_back(&sys->cpu_hi, (unsigned int) (((double)dhi) *vscale) );
	grapher_track_push_back(&sys->cpu_si, (unsigned int) (((double)dsi) *vscale) );
	grapher_track_push_back(&sys->cpu_st, (unsigned int) (((double)dst) *vscale) );

	/* store values for next iteration */
	sys->cpu_us_last = us;
	sys->cpu_ni_last = ni;
	sys->cpu_sy_last = sy;
	sys->cpu_id_last = id;
	sys->cpu_wa_last = wa;
	sys->cpu_hi_last = hi;
	sys->cpu_si_last = si;
	sys->cpu_st_last = st;
}


void
grapher_sys_clone(grapher_sys *dest,
                  grapher_sys *src)
{
	assert(dest);
	assert(src);
  
	grapher_track_clone(&dest->cpu_us, &src->cpu_us);
	grapher_track_clone(&dest->cpu_ni, &src->cpu_ni);
	grapher_track_clone(&dest->cpu_sy, &src->cpu_sy);
	grapher_track_clone(&dest->cpu_id, &src->cpu_id);
	grapher_track_clone(&dest->cpu_wa, &src->cpu_wa);
	grapher_track_clone(&dest->cpu_hi, &src->cpu_hi);
	grapher_track_clone(&dest->cpu_si, &src->cpu_si);
	grapher_track_clone(&dest->cpu_st, &src->cpu_st);
  
	cu_vector_uint_clone(&dest->jiffies, &src->jiffies);
  
	dest->cpu_us_last = src->cpu_us_last;
	dest->cpu_ni_last = src->cpu_ni_last;
	dest->cpu_sy_last = src->cpu_sy_last;
	dest->cpu_id_last = src->cpu_id_last;
	dest->cpu_wa_last = src->cpu_wa_last;
	dest->cpu_hi_last = src->cpu_hi_last;
	dest->cpu_si_last = src->cpu_si_last;
	dest->cpu_st_last = src->cpu_st_last;
}

void
grapher_sys_slice_clone(grapher_sys *dest,
                        grapher_sys *src,
                        unsigned int lpos,
                        unsigned int rpos)
{
	unsigned int track_size;
  
	assert(dest);
	assert(src);
	assert(lpos < rpos);
	track_size = grapher_track_size(&src->cpu_us);
	assert(rpos <= track_size);

	grapher_track_slice_clone(&dest->cpu_us, &src->cpu_us, lpos, rpos);
	grapher_track_slice_clone(&dest->cpu_ni, &src->cpu_ni, lpos, rpos);
	grapher_track_slice_clone(&dest->cpu_sy, &src->cpu_sy, lpos, rpos);
	grapher_track_slice_clone(&dest->cpu_id, &src->cpu_id, lpos, rpos);
	grapher_track_slice_clone(&dest->cpu_wa, &src->cpu_wa, lpos, rpos);
	grapher_track_slice_clone(&dest->cpu_hi, &src->cpu_hi, lpos, rpos);
	grapher_track_slice_clone(&dest->cpu_si, &src->cpu_si, lpos, rpos);
	grapher_track_slice_clone(&dest->cpu_st, &src->cpu_st, lpos, rpos);
  
	{
		unsigned int i;
		for (i = lpos; i <= rpos; i++)
		{
			unsigned int current_jiffies;
			current_jiffies = cu_vector_uint_at(&src->jiffies, i);
			cu_vector_uint_push_back(&dest->jiffies, current_jiffies);
		}
	}  

	dest->cpu_us_last = src->cpu_us_last;
	dest->cpu_ni_last = src->cpu_ni_last;
	dest->cpu_sy_last = src->cpu_sy_last;
	dest->cpu_id_last = src->cpu_id_last;
	dest->cpu_wa_last = src->cpu_wa_last;
	dest->cpu_hi_last = src->cpu_hi_last;
	dest->cpu_si_last = src->cpu_si_last;
	dest->cpu_st_last = src->cpu_st_last;
}

