/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */


#ifndef _PRIV_FLUID_SFONT_H
#define _PRIV_FLUID_SFONT_H


/*
 * Utility macros to access soundfonts, presets, and samples
 */

#define fluid_sfloader_delete(_loader) { if ((_loader) && (_loader)->free) (*(_loader)->free)(_loader); }
#define fluid_sfloader_load(_loader, _filename) (*(_loader)->load)(_loader, _filename)


#define delete_fluid_sfont(_sf)   ( ((_sf) && (_sf)->free)? (*(_sf)->free)(_sf) : 0)
#define fluid_sfont_get_name(_sf) (*(_sf)->get_name)(_sf)
#define fluid_sfont_get_preset(_sf,_bank,_prenum) (*(_sf)->get_preset)(_sf,_bank,_prenum)
#define fluid_sfont_iteration_start(_sf) (*(_sf)->iteration_start)(_sf)
#define fluid_sfont_iteration_next(_sf,_pr) (*(_sf)->iteration_next)(_sf,_pr)
#define fluid_sfont_get_data(_sf) (_sf)->data
#define fluid_sfont_set_data(_sf,_p) { (_sf)->data = (void*) (_p); }


#define delete_fluid_preset(_preset) \
  { if ((_preset) && (_preset)->free) { (*(_preset)->free)(_preset); }}

#define fluid_preset_get_data(_preset) (_preset)->data
#define fluid_preset_set_data(_preset,_p) { (_preset)->data = (void*) (_p); }
#define fluid_preset_get_name(_preset) (*(_preset)->get_name)(_preset)
#define fluid_preset_get_banknum(_preset) (*(_preset)->get_banknum)(_preset)
#define fluid_preset_get_num(_preset) (*(_preset)->get_num)(_preset)

#define fluid_preset_noteon(_preset,_synth,_ch,_key,_vel) \
  (*(_preset)->noteon)(_preset,_synth,_ch,_key,_vel)

#define fluid_preset_notify(_preset,_reason,_chan) \
  { if ((_preset) && (_preset)->notify) { (*(_preset)->notify)(_preset,_reason,_chan); }}


#define fluid_sample_incr_ref(_sample) { (_sample)->refcount++; }

#define fluid_sample_decr_ref(_sample) \
  (_sample)->refcount--; \
  if (((_sample)->refcount == 0) && ((_sample)->notify)) \
    (*(_sample)->notify)(_sample, FLUID_SAMPLE_DONE);



#endif /* _PRIV_FLUID_SFONT_H */
