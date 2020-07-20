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


#ifndef _FLUID_SETTINGS_H
#define _FLUID_SETTINGS_H



/** returns 1 if the option was added, 0 otherwise */
int fluid_settings_add_option(fluid_settings_t* settings, const char* name, char* s);

/** returns 1 if the option was added, 0 otherwise */
int fluid_settings_remove_option(fluid_settings_t* settings, const char* name, char* s);


typedef int (*fluid_num_update_t)(void* data, const char* name, double value);
typedef int (*fluid_str_update_t)(void* data, const char* name, char* value);
typedef int (*fluid_int_update_t)(void* data, const char* name, int value);

/** returns 0 if the value has been resgister correctly, non-zero
    otherwise */
int fluid_settings_register_str(fluid_settings_t* settings, const char* name, char* def, int hints,
			       fluid_str_update_t fun, void* data);

/** returns 0 if the value has been resgister correctly, non-zero
    otherwise */
int fluid_settings_register_num(fluid_settings_t* settings, const char* name, double min, double max,
			       double def, int hints, fluid_num_update_t fun, void* data);


/** returns 0 if the value has been resgister correctly, non-zero
    otherwise */
int fluid_settings_register_int(fluid_settings_t* settings, const char* name, int min, int max,
			       int def, int hints, fluid_int_update_t fun, void* data);


#endif /* _FLUID_SETTINGS_H */
