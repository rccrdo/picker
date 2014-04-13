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

#include "grapher_cairo.h"


int grapher_cairo_draw (grapher_statstore* store,
                        grapher_draw_opt* opt,
                        const char* file)
{
	unsigned int samples;
	unsigned int i;
	double usec_len;
	double sec_len;
	double w, h, vpad, hpad, gw, gh;
	double page_y;
	cairo_surface_t* surface;  
	cairo_t *cr;

	assert(opt && store && file);
  
	if (opt->pps == 0.)
	{
		printf("warning grapher_cairo_draw, pps is zero. Not drawing anything ..\n");
		return 0;    
	}
	if (opt->sth == 0.)
	{
		printf("warning grapher_cairo_draw, sth is zero. Not drawing anything ..\n");
		return 0;    
	}
    
	samples = cu_vector_ptrs_size(&store->timeline);
	assert(samples);
  
	/* well .. */
	if (!samples)
		return 0;

	usec_len = (double)cu_time_passed((struct timeval*)cu_vector_ptrs_at(&store->timeline, 0),
					  (struct timeval*)cu_vector_ptrs_at(&store->timeline, samples -1));
	sec_len = usec_len / ((double)1000000);

	gw = sec_len*opt->pps ;
	gh = opt->sth + cu_vector_ptrs_size(&store->activep)*(6.7*opt->sth +5);
	hpad = 50;
	vpad = hpad;
	w = gw + hpad*2.;
	h = gh + vpad*2.;
  

	surface = cairo_svg_surface_create (file, w, h);

	cr = cairo_create (surface);
	assert(surface);
  
	/* draw the time grid */
	{
		double hscale = sec_len/gw;
		grapher_rect rect = {hpad, h, hpad +gw, 0.};
		_grapher_cairo_draw_timegrid(surface, &rect, hscale);  
	}


	/* draw sys cpu stats */
	{
		grapher_rect rect = {hpad, vpad + opt->sth, hpad +gw, vpad};

		grapher_track* tracks[4] = {&store->sys.cpu_us,
					    &store->sys.cpu_sy,
					    &store->sys.cpu_wa,
					    0 };

		_grapher_cairo_draw_cpu(opt,
					&rect,
					surface,
					&store->timeline,
					0,
					tracks,
					&store->sys.jiffies);

		_grapher_cairo_stroke_rect(surface,
					   &rect,
					   1.);

		_grapher_cairo_draw_track_text_overlays(opt,
							surface,
							&rect,
							0,
							100,
							"cpu");
	}

  
	page_y = opt->sth + vpad *2.;
  
	printf (" * drawing %d processes ...\n", cu_vector_ptrs_size(&store->activep)); 
	double hscale;
	hscale = w / usec_len;          
	for (i=0; i < cu_vector_ptrs_size(&store->activep); i++)
	{
		grapher_process* p;
		unsigned int start_time;

		struct timeval* start_tick_time;

		p = (grapher_process*) cu_vector_ptrs_at(&store->activep,i);
		assert(p);

		start_tick_time = (struct timeval*) cu_vector_ptrs_at(&store->timeline, p->start_tick);
		start_time = cu_time_passed((struct timeval*) cu_vector_ptrs_at(&store->timeline, 0), start_tick_time);

		_grapher_cairo_draw_process(surface,
					    opt,
					    p,
					    &store->timeline,
					    &store->sys.jiffies,
					    hpad,
					    page_y );
		page_y += opt->sth*6.7 + 5.;
	}

	cairo_surface_finish(surface);
	cairo_destroy (cr);
	cairo_surface_destroy(surface);  
  
	return 1;
}


void _grapher_cairo_draw_process (cairo_surface_t* surface,
                                  grapher_draw_opt* opt,
                                  grapher_process* p,
                                  cu_vector_ptrs* timeline,
                                  cu_vector_uint* jiffies,
                                  double page_hpad,
                                  double page_y )
{
	unsigned int samples;
	unsigned int i;
	double usec_len;
	double sec_len;
	double hscale;
	grapher_rect prect;
  
	cairo_t *cr;
  
	assert(surface);
	assert(opt);
	assert(p);
	assert(timeline);
	assert(jiffies);
	assert(page_y);

	assert( (grapher_track_size(&p->minflt) == grapher_track_size(&p->majflt))
		&& (grapher_track_size(&p->minflt) == grapher_track_size(&p->utime))
		&& (grapher_track_size(&p->minflt) == grapher_track_size(&p->stime)) );
  
	/* value is the same for all tracks */
	samples = grapher_track_size(&p->utime);
	assert(samples);
  
	usec_len = (double)cu_time_passed((struct timeval*)cu_vector_ptrs_at(timeline, p->start_tick),
					  (struct timeval*)cu_vector_ptrs_at(timeline, p->start_tick + samples -1));
	sec_len = usec_len / ((double)1000000);
  
	cr = cairo_create (surface);
	assert(cr);
  
	/* process rect */
 
	hscale = opt->pps / 1000000.0;
	prect.xi = ((double)cu_time_passed((struct timeval*)cu_vector_ptrs_at(timeline, 0),
					   (struct timeval*)cu_vector_ptrs_at(timeline, p->start_tick))) *hscale
		+ page_hpad - 8;
                                       
	prect.yi = page_y + opt->sth*6.7;
	prect.xf = hscale*usec_len + prect.xi +8 +8;
	prect.yf = page_y;

	{
		double r = 50; /* radius */
		double rr = 25; /* radius */
		double x, y, w, h;
		char cmdline[GRAPHER_PROCESS_CMDLINE_MAX_LEN];
    
		x = prect.xi;
		y = prect.yf;
		w = prect.xf - prect.xi;
		h = prect.yi - prect.yf;
    
		cairo_move_to (cr, x+r, y);
		cairo_line_to (cr, x +w-rr, y);
		cairo_curve_to(cr, x+w,y,x+w,y,x+w,y+rr);
		cairo_line_to (cr, x+w,y+h-rr);
		cairo_curve_to(cr, x+w,y+h,x+w,y+h,x+w-rr,y+h);
		cairo_line_to (cr, x+r,y+h);
		cairo_curve_to(cr, x,y+h,x,y+h,x,y+h-r);
		cairo_line_to (cr, x,y+r);
		cairo_curve_to(cr, x,y,x,y,x+r,y);
		cairo_set_source_rgba(cr, 0.91, 0.91, 0.91, 0.5);
		cairo_fill_preserve(cr);
		cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 0.8);
		cairo_stroke(cr);

		/* draw the process name */
		cairo_select_font_face (cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size (cr, 35);

		cairo_move_to (cr, prect.xi +26, prect.yf + 37);
		cairo_set_source_rgba (cr, 0., 0., 0., 0.4);
		sprintf(cmdline, "%.80s (%d)", p->cmdline, p->pid);
		cairo_show_text (cr, cmdline);
	}

	if (samples == 1) 
	{
		cairo_destroy (cr);
		return;
	}
  
	{
		grapher_track cpu_us;
		grapher_track cpu_sy;
		grapher_track* tracks[3] = {&cpu_us,
					    &cpu_sy,
					    0 };
		grapher_rect cpurect;

		/* cpu usage in percent */
		grapher_track_init(&cpu_us, "cpu user");
		grapher_track_init(&cpu_sy, "cpu sys");
  
		/* we cannot say anything for the first sample */
		grapher_track_push_back(&cpu_us, 0);
		grapher_track_push_back(&cpu_sy, 0);
  
		for (i= 1; i< samples; i++)
		{
			unsigned int dus, dsy, tot;
			double vscale;    

			/* change jiffies to percentuages */  
			dus = grapher_track_at(&p->utime, i) - grapher_track_at(&p->utime, i-1);
			dsy = grapher_track_at(&p->stime, i) - grapher_track_at(&p->stime, i-1);
    
			tot = cu_vector_uint_at(jiffies, i + p->start_tick);
			if (tot == 0)
				vscale = 0.;
			else
				vscale = 100. / ((double)tot);

			grapher_track_push_back(&cpu_us, (unsigned int) (((double)dus) *vscale) );
			grapher_track_push_back(&cpu_sy, (unsigned int) (((double)dsy) *vscale) );
		}
  
		cpu_us.max = 100;
		cpu_sy.max = 100;

		cpurect.xi = ((double)cu_time_passed((struct timeval*)cu_vector_ptrs_at(timeline, 0),
						     (struct timeval*)cu_vector_ptrs_at(timeline, p->start_tick))) *hscale
			+ page_hpad;                 
		cpurect.yi = page_y + opt->sth*2.2;
		cpurect.xf = hscale*usec_len + cpurect.xi;
		cpurect.yf = page_y + opt->sth*1.2;  
  
		_grapher_cairo_draw_cpu(opt,
					&cpurect,
					surface,
					timeline,
					p->start_tick,
					tracks,
					jiffies);

		grapher_track_deinit(&cpu_us);
		grapher_track_deinit(&cpu_sy);

		_grapher_cairo_stroke_rect(surface,
					   &cpurect,
					   1. );

		_grapher_cairo_draw_track_text_overlays(opt,
							surface,
							&cpurect,
							0,
							100,
							"cpu");

	}

	{
		grapher_rect minflt;

		minflt.xi = ((double)cu_time_passed((struct timeval*)cu_vector_ptrs_at(timeline, 0),
						    (struct timeval*)cu_vector_ptrs_at(timeline, p->start_tick))) *hscale
			+ page_hpad;                 
		minflt.yi = page_y + opt->sth*3.3;
		minflt.xf = hscale*usec_len + minflt.xi;
		minflt.yf = page_y + opt->sth*2.3;  
  
		_grapher_cairo_draw_track (surface,
					   &minflt,
					   opt,
					   &p->minflt,
					   timeline,
					   p->start_tick);

		_grapher_cairo_stroke_rect(surface,
					   &minflt,
					   1. );

		_grapher_cairo_draw_track_text_overlays(opt,
							surface,
							&minflt,
							p->minflt.min,
							p->minflt.max,
							p->minflt.name);
	}
  
	{
		grapher_rect majflt;

		majflt.xi = ((double)cu_time_passed((struct timeval*)cu_vector_ptrs_at(timeline, 0),
						    (struct timeval*)cu_vector_ptrs_at(timeline, p->start_tick))) *hscale
			+ page_hpad;                 
		majflt.yi = page_y + opt->sth*4.4;
		majflt.xf = hscale*usec_len + majflt.xi;
		majflt.yf = page_y + opt->sth*3.4;  
  
		_grapher_cairo_draw_track (surface,
					   &majflt,
					   opt,
					   &p->majflt,
					   timeline,
					   p->start_tick);

		_grapher_cairo_stroke_rect(surface,
					   &majflt,
					   1.);

		_grapher_cairo_draw_track_text_overlays(opt,
							surface,
							&majflt,
							p->majflt.min,
							p->majflt.max,
							p->majflt.name);
	}
  
	{
		grapher_rect size;

		size.xi = ((double)cu_time_passed((struct timeval*)cu_vector_ptrs_at(timeline, 0),
						  (struct timeval*)cu_vector_ptrs_at(timeline, p->start_tick))) *hscale
			+ page_hpad;                 
		size.yi = page_y + opt->sth*5.5;
		size.xf = hscale*usec_len + size.xi;
		size.yf = page_y + opt->sth*4.5;  
  
		_grapher_cairo_draw_track (surface,
					   &size,
					   opt,
					   &p->mem_size,
					   timeline,
					   p->start_tick);

		_grapher_cairo_stroke_rect(surface,
					   &size,
					   1.);

		_grapher_cairo_draw_track_text_overlays(opt,
							surface,
							&size,
							p->mem_size.min,
							p->mem_size.max,
							p->mem_size.name);
	}
  
	{
		grapher_rect resident;

		resident.xi = ((double)cu_time_passed((struct timeval*)cu_vector_ptrs_at(timeline, 0),
						      (struct timeval*)cu_vector_ptrs_at(timeline, p->start_tick))) *hscale
			+ page_hpad;                 
		resident.yi = page_y + opt->sth*6.6;
		resident.xf = hscale*usec_len + resident.xi;
		resident.yf = page_y + opt->sth*5.6;  
  
		_grapher_cairo_draw_track (surface,
					   &resident,
					   opt,
					   &p->mem_resident,
					   timeline,
					   p->start_tick);

		_grapher_cairo_stroke_rect(surface,
					   &resident,
					   1.);

		_grapher_cairo_draw_track_text_overlays(opt,
							surface,
							&resident,
							p->mem_resident.min,
							p->mem_resident.max,
							p->mem_resident.name);
	}
 
	cairo_destroy (cr);
	return;
}
                                             

void _grapher_cairo_draw_track (cairo_surface_t* surface,
                                grapher_rect* rect,
                                grapher_draw_opt* opt,
                                grapher_track* track,
                                cu_vector_ptrs* timeline,
                                unsigned int start_tick)
{
	unsigned int samples;
	double usec_len;
	double sec_len;
	cairo_t *cr;

	assert(surface);
	assert(rect);
	assert(opt);
	assert(track);
	assert(timeline);
	assert(start_tick < cu_vector_ptrs_size(timeline));

	samples = grapher_track_size(track);
	assert(samples);
  
	usec_len = (double)cu_time_passed((struct timeval*)cu_vector_ptrs_at(timeline, start_tick),
					  (struct timeval*)cu_vector_ptrs_at(timeline, start_tick + samples -1));
	sec_len = usec_len / ((double)1000000);
  
	if (sec_len ==0) 
	{
		printf("warning _grapher_cairo_draw_track, hack!\n");
		sec_len = 0.1;
	}
  
	cr = cairo_create (surface);
  
	/* draw the track */
	{ 
		double hscale;
		double vscale;
		struct timeval* start_tick_time;

		vscale = (rect->yi - rect->yf) / ((double)(track->max - track->min));
		hscale = (rect->xf - rect->xi) / usec_len;
    
		start_tick_time = (struct timeval*)cu_vector_ptrs_at(timeline, start_tick);
		assert(start_tick_time);
		assert(grapher_track_size(track));

		cairo_move_to(cr, rect->xi, rect->yi - ((double)grapher_track_at(track,0))*vscale);
		{
			unsigned int i;
			for (i=1; i< grapher_track_size(track); i++)
			{
				double usec_from_start_tick;
				double x, y;
				struct timeval* current_sample_time;

				current_sample_time = (struct timeval*)cu_vector_ptrs_at(timeline, start_tick + i);
				assert(current_sample_time);
				usec_from_start_tick = (double)cu_time_passed(start_tick_time, current_sample_time);
				x = rect->xi + usec_from_start_tick*hscale;
				double value =  ((double)(grapher_track_at(track,i) -track->min))*vscale;
				y = rect->yi - value;
				cairo_line_to(cr, x, y);
			}
		}
		cairo_set_source_rgba (cr, 0.9, 0.7, 0.3, 1.);
		cairo_set_line_width (cr, 1.);
		cairo_stroke (cr);
	}

	cairo_destroy (cr);
}


void  _grapher_cairo_draw_cpu(grapher_draw_opt* opt,
                              grapher_rect* rect,
                              cairo_surface_t* surface,
                              cu_vector_ptrs* timeline,
                              unsigned int start_tick,
                              grapher_track** tracks,
                              cu_vector_uint* jiffies )
{
	unsigned int samples;
	unsigned int k,i;
	unsigned int num_tracks;
	double usec_len, sec_len;
	double vscale, hscale;
	struct timeval* start_tick_time;
	cairo_t *cr;

	assert(opt);
	assert(rect);
	assert(surface);
	assert(timeline);
	assert((start_tick < cu_vector_ptrs_size(timeline)));
	assert(tracks);
	assert(jiffies);

	/* for sys stats we pass 3, for per process stats we pass only 2 */
	num_tracks = 0;
	while (tracks[num_tracks])
		num_tracks ++;
  
	assert(num_tracks);

	/* number of samples in the tracks */
	samples = grapher_track_size(tracks[0]);
	assert(samples);

	/* tracks time length */
	usec_len = (double)cu_time_passed((struct timeval*)cu_vector_ptrs_at(timeline, start_tick),
					  (struct timeval*)cu_vector_ptrs_at(timeline, start_tick + samples -1));
	sec_len = usec_len / ((double)1000000);

	/* sec_len is zero if we have only  one sample */  
	if (sec_len==0.) 
	{
		printf("warning _grapher_cairo_draw_cpu, hack!\n");
		sec_len = 0.1;
	}

	/* start drawing */
	cr = cairo_create (surface);
  
	vscale = (rect->yi - rect->yf) / 100.;
	hscale = (rect->xf - rect->xi) / usec_len;

	start_tick_time = (struct timeval*)cu_vector_ptrs_at(timeline, start_tick);
	assert(start_tick_time);

	for (k = 0; k< num_tracks; k++)
	{ 
		double value;
		int j;
       
		value = 0.;
		for (j=num_tracks-k-1; j>=0; j--)
			value += ((double)grapher_track_at(tracks[j],0));
       
		cairo_move_to(cr, rect->xi, rect->yi - value*vscale);
		for (i=1; i< samples; i++)
		{
			int l;
			double usec_from_start_tick;
			double x, y;
			struct timeval* current_sample_time;

			value = 0.;
			for (l=num_tracks-k-1; l>=0; l--)
				value += ((double)grapher_track_at(tracks[l],i));

			current_sample_time = (struct timeval*)cu_vector_ptrs_at(timeline,
										 start_tick + i);
			assert(current_sample_time);
       
			usec_from_start_tick = (double)cu_time_passed(start_tick_time,
								      current_sample_time);
			x = rect->xi + usec_from_start_tick*hscale;
			y = rect->yi - value*vscale;
			cairo_line_to(cr, x, y);
		}

		/* close path and fill*/
		cairo_line_to(cr, rect->xf, rect->yi);
		cairo_line_to(cr, rect->xi, rect->yi);
		cairo_close_path(cr);

		{
			unsigned int color;
			color = k;
			if (num_tracks == 2)
				color = k+1;        
			if (color==0) /* wa time */
				cairo_set_source_rgba (cr, 0.94, 0.4, 0.0, 1.);
			else if (color==1) /* sys time */
				cairo_set_source_rgba (cr, 0.909, 0.816, 0.07, 1.);
			else if (color==2) /* user time */
				cairo_set_source_rgba (cr, 0.27, 0.72, 0.88, 1.);
		}
		cairo_fill(cr);
	}

	cairo_destroy (cr);
}


void  _grapher_cairo_stroke_rect(cairo_surface_t* surface,
                                 grapher_rect* rect,
                                 double line_width )
{
	assert(surface);
	assert(rect);
	assert(line_width);
	cairo_t* cr;
  
	cr = cairo_create(surface);

	cairo_set_source_rgba (cr, 0.7, 0.7, 0.7, 1.);

	cairo_set_line_width (cr, line_width);
                                   
	cairo_move_to(cr, rect->xi, rect->yi);
	cairo_line_to(cr, rect->xi, rect->yf);
	cairo_line_to(cr, rect->xf, rect->yf);
	cairo_line_to(cr, rect->xf, rect->yi);  
	cairo_line_to(cr, rect->xi, rect->yi);
	cairo_stroke(cr);

	cairo_destroy(cr);
}


void  _grapher_cairo_draw_timegrid(cairo_surface_t* surface,
                                   grapher_rect* rect,
                                   double hscale)
{
	unsigned int i;
	unsigned int sec_len;
	char buf[10];
	cairo_t* cr;

	assert(surface);
	assert(rect);
	assert(hscale);
        
	sec_len = (unsigned int) (hscale * (rect->xf - rect->xi));

	cr = cairo_create(surface);
	cairo_set_source_rgba (cr, 0., 0., 0., 0.2);
	cairo_set_font_size (cr, 15);
	cairo_set_line_width (cr, 1.);

	cairo_move_to (cr, rect->xi, rect->yf + 25);
	cairo_show_text (cr, "0");
	cairo_move_to(cr, rect->xi, rect->yf +30);
	cairo_line_to(cr, rect->xi, rect->yi);
	cairo_stroke(cr);
	for (i=1; i< sec_len; i++)
	{
		double xpos;
      
		xpos = rect->xi +  ((double)i)/hscale;
      
		sprintf(buf, "%u", i);
		cairo_move_to (cr, xpos, rect->yf + 25);
		cairo_show_text (cr, buf);    

		cairo_move_to(cr, xpos, rect->yf + 30);
		cairo_line_to(cr, xpos, rect->yi);
		cairo_stroke(cr);
	}

	cairo_destroy(cr);
}


void _grapher_cairo_draw_track_text_overlays(grapher_draw_opt* opt,
                                             cairo_surface_t* surface,
                                             grapher_rect* rect,
                                             unsigned int min,
                                             unsigned int max,
                                             const char* name )
{
	cairo_t* cr;

	assert(opt);
	assert(surface);
	assert(rect);
	assert(max>min);
	assert(name);

	cr = cairo_create (surface);
	assert(cr);

	/* draw min max text */
	{ 
		char buffer [20];
		sprintf(buffer,"%d", max);
		cairo_select_font_face (cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size (cr, 8);
		cairo_set_source_rgba (cr, 0.3, 0.3, 0.3, 0.6);
                               
		cairo_move_to (cr, rect->xi + 1.6,
			       rect->yf + 8.8);
		cairo_show_text (cr, buffer);
    
		sprintf(buffer,"%d", min);
		cairo_move_to (cr, rect->xi + 1.6,
			       rect->yi - 1.6);
		cairo_show_text (cr, buffer);
	}
  
	/* draw track name text */
	cairo_set_font_size (cr, 10);
	cairo_set_source_rgba (cr, 0.3, 0.3, 0.3, 0.6);

	cairo_move_to (cr, rect->xi + 1.6,
		       rect->yf + 22.);
	cairo_show_text (cr, name);

	cairo_destroy(cr);
}
