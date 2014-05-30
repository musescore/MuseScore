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


#ifndef _FLUID_GEN_H
#define _FLUID_GEN_H

namespace FluidS {

class Channel;
class Generator;

class GenInfo {
   public:
      char num;        /* Generator number */
      char init;       /* Does the generator need to be initialized (cfr. fluid_voice_init()) */
      char nrpn_scale; /* The scale to convert from NRPN (cfr. fluid_gen_map_nrpn()) */
      float min;       /* The minimum value */
      float max;       /* The maximum value */
      float def;       /* The default value (cfr. fluid_gen_set_default_values()) */
      };

float fluid_gen_scale(int gen, float value);
float fluid_gen_scale_nrpn(int gen, int nrpn);
void fluid_gen_init(Generator* gen, Channel* channel);

}

#endif /* _FLUID_GEN_H */
