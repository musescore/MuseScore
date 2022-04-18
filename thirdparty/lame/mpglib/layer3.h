/*
 * Copyright (C) 1999-2010 The L.A.M.E. project
 *
 * Initially written by Michael Hipp, see also AUTHORS and README.
 *  
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef LAYER3_H_INCLUDED
#define LAYER3_H_INCLUDED

void    hip_init_tables_layer3(void);
int     decode_layer3_sideinfo(PMPSTR mp);
int     decode_layer3_frame(PMPSTR mp, unsigned char *pcm_sample, int *pcm_point,
                  int (*synth_1to1_mono_ptr) (PMPSTR, real *, unsigned char *, int *),
                  int (*synth_1to1_ptr) (PMPSTR, real *, int, unsigned char *, int *));
int     layer3_audiodata_precedesframes(PMPSTR mp);

#endif
