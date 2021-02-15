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


#ifndef _FLUID_SETTINGS_H
#define _FLUID_SETTINGS_H

int fluid_settings_add_option(fluid_settings_t *settings, const char *name, const char *s);
int fluid_settings_remove_option(fluid_settings_t *settings, const char *name, const char *s);


typedef void (*fluid_str_update_t)(void *data, const char *name, const char *value);

int fluid_settings_register_str(fluid_settings_t *settings, const char *name, const char *def, int hints);
int fluid_settings_callback_str(fluid_settings_t *settings, const char *name,
                                fluid_str_update_t fun, void *data);


typedef void (*fluid_num_update_t)(void *data, const char *name, double value);

int fluid_settings_register_num(fluid_settings_t *settings, const char *name, double def,
                                double min, double max, int hints);
int fluid_settings_callback_num(fluid_settings_t *settings, const char *name,
                                fluid_num_update_t fun, void *data);

/* Type specific wrapper for fluid_settings_getnum */
int fluid_settings_getnum_float(fluid_settings_t *settings, const char *name, float *val);


typedef void (*fluid_int_update_t)(void *data, const char *name, int value);
int fluid_settings_register_int(fluid_settings_t *settings, const char *name, int def,
                                int min, int max, int hints);
int fluid_settings_callback_int(fluid_settings_t *settings, const char *name,
                                fluid_int_update_t fun, void *data);

int fluid_settings_split_csv(const char *str, int *buf, int buf_len);

void* fluid_settings_get_user_data(fluid_settings_t * settings, const char *name);

#endif /* _FLUID_SETTINGS_H */
