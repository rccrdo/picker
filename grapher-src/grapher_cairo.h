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

#ifndef grapher_cairo_h
#define grapher_cairo_h 1

#include <cairo.h>
#include <cairo-svg.h> 

#include "cu_time_utils.h"

#include "grapher_debug.h"
#include "grapher_statstore.h"
#include "grapher_cairo.h"
#include "grapher_sys.h"
#include "grapher_draw.h"

int grapher_cairo_draw (grapher_statstore* store,
                        grapher_draw_opt* opt,
                        const char* file);
                                 
void _grapher_cairo_draw_process (cairo_surface_t* surface,
                                  grapher_draw_opt* opt,
                                  grapher_process* p,
                                  cu_vector_ptrs* timeline,
                                  cu_vector_uint* jiffies,
                                  double page_hpad,
                                  double page_y );
                                           
void _grapher_cairo_draw_track (cairo_surface_t* surface,
                                grapher_rect* rect,
                                grapher_draw_opt* opt,
                                grapher_track* track,
                                cu_vector_ptrs* timeline,
                                unsigned int start_tick);

/**
 *
 * tracks must be NULL terminated
 */
void  _grapher_cairo_draw_cpu(grapher_draw_opt* opt,
                              grapher_rect* rect,
                              cairo_surface_t* surface,
                              cu_vector_ptrs* timeline,
                              unsigned int start_tick,
                              grapher_track** tracks,
                              cu_vector_uint* jiffies );

void  _grapher_cairo_stroke_rect(cairo_surface_t* surface,
                                 grapher_rect* rect,
                                 double line_width );

void  _grapher_cairo_draw_timegrid(cairo_surface_t* surface,
                                   grapher_rect* rect,
                                   double hscale);

void _grapher_cairo_draw_track_text_overlays(grapher_draw_opt* opt,
                                             cairo_surface_t* surface,
                                             grapher_rect* rect,
                                             unsigned int min,
                                             unsigned int max,
                                             const char* name );

#endif /* grapher_cairo_h */

