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

#ifndef _FLUID_CONV_H
#define _FLUID_CONV_H

namespace FluidS {

#define FLUID_CENTS_HZ_SIZE     1200
#define FLUID_VEL_CB_SIZE       128
#define FLUID_CB_AMP_SIZE       961
#define FLUID_ATTEN_AMP_SIZE    1441
#define FLUID_PAN_SIZE          1002

/* EMU 8k/10k don't follow spec in regards to volume attenuation.
 * This factor is used in the equation pow (10.0, cb / FLUID_ATTEN_POWER_FACTOR).
 */
#define FLUID_ATTEN_POWER_FACTOR  (-200.0)

void fluid_conversion_config(void);

float fluid_cb2amp(float cb);
float fluid_atten2amp(float atten);
float fluid_tc2sec(float tc);
float fluid_tc2sec_delay(float tc);
float fluid_tc2sec_attack(float tc);
float fluid_tc2sec_release(float tc);
float fluid_act2hz(float c);
float fluid_pan(float c, int left);
float fluid_concave(float val);
float fluid_convex(float val);

}
#endif /* _FLUID_CONV_H */
