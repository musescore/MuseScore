/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */


#ifndef _FLUID_SAMPLECACHE_H
#define _FLUID_SAMPLECACHE_H

#include "fluid_sfont.h"
#include "fluid_sffile.h"

int fluid_samplecache_load(SFData *sf,
                           unsigned int sample_start, unsigned int sample_end, int sample_type,
                           int try_mlock, short **data, char **data24);

int fluid_samplecache_unload(const short *sample_data);

/* Only used for tests */
int fluid_samplecache_count_entries(void);

#endif /* _FLUID_SAMPLECACHE_H */
