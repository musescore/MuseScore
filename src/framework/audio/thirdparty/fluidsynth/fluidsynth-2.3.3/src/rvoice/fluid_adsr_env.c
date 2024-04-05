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

#include "fluid_adsr_env.h"

DECLARE_FLUID_RVOICE_FUNCTION(fluid_adsr_env_set_data)
{
    fluid_adsr_env_t *env = obj;
    fluid_adsr_env_section_t section = param[0].i;
    unsigned int count = param[1].i;
    fluid_real_t coeff = param[2].real;
    fluid_real_t increment = param[3].real;
    fluid_real_t min = param[4].real;
    fluid_real_t max = param[5].real;

    env->data[section].count = count;
    env->data[section].coeff = coeff;
    env->data[section].increment = increment;
    env->data[section].min = min;
    env->data[section].max = max;
}

