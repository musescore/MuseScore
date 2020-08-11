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
#ifndef _FLUID_LASH_H
#define _FLUID_LASH_H

#include "config.h"

#ifdef HAVE_LASH

#include "fluid_synth.h"

#include <lash/lash.h>
extern lash_client_t *fluid_lash_client;
#define fluid_lash_args_t  lash_args_t
#define fluid_lash_alsa_client_id  lash_alsa_client_id
#define fluid_lash_jack_client_name  lash_jack_client_name


FLUIDSYNTH_API fluid_lash_args_t *fluid_lash_extract_args(int *pargc, char  ***pargv);
FLUIDSYNTH_API int fluid_lash_connect(fluid_lash_args_t *args);
FLUIDSYNTH_API void fluid_lash_create_thread(fluid_synth_t *synth);

#endif /* defined(HAVE_LASH) */
#endif /* _FLUID_LASH_H */
