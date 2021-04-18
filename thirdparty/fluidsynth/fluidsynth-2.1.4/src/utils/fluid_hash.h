/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02110-1301, USA.
 */

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/.
 */

/*
 * Adapted for FluidSynth use by Josh Green <jgreen@users.sourceforge.net>
 * September 8, 2009 from glib 2.18.4
 *
 * - Self contained (no dependencies on glib)
 * - changed names to fluid_hashtable_...
 */

#ifndef _FLUID_HASH_H
#define _FLUID_HASH_H

#include "fluidsynth_priv.h"
#include "fluid_list.h"
#include "fluid_sys.h"

/* Extracted from gtypes.h */
typedef void (*fluid_destroy_notify_t)(void *data);
typedef unsigned int (*fluid_hash_func_t)(const void *key);
typedef int (*fluid_equal_func_t)(const void *a, const void *b);
/* End gtypes.h extraction */

typedef int (*fluid_hr_func_t)(void *key, void *value, void *user_data);
typedef struct _fluid_hashtable_iter_t fluid_hashtable_iter_t;

typedef struct _fluid_hashnode_t      fluid_hashnode_t;

struct _fluid_hashnode_t
{
    void *key;
    void *value;
    fluid_hashnode_t *next;
    unsigned int key_hash;
};

struct _fluid_hashtable_t
{
    int size;
    int nnodes;
    fluid_hashnode_t **nodes;
    fluid_hash_func_t hash_func;
    fluid_equal_func_t key_equal_func;
    fluid_atomic_int_t ref_count;
    fluid_destroy_notify_t key_destroy_func;
    fluid_destroy_notify_t value_destroy_func;
    fluid_rec_mutex_t mutex;          // Optionally used in other modules (fluid_settings.c for example)
};

struct _fluid_hashtable_iter_t
{
    /*< private >*/
    void 	*dummy1;
    void 	*dummy2;
    void 	*dummy3;
    int		dummy4;
    int		dummy5;		// Bool
    void 	*dummy6;
};

fluid_hashtable_t *new_fluid_hashtable(fluid_hash_func_t hash_func,
                                       fluid_equal_func_t key_equal_func);
fluid_hashtable_t *new_fluid_hashtable_full(fluid_hash_func_t hash_func,
        fluid_equal_func_t key_equal_func,
        fluid_destroy_notify_t key_destroy_func,
        fluid_destroy_notify_t value_destroy_func);
void delete_fluid_hashtable(fluid_hashtable_t *hashtable);

void fluid_hashtable_iter_init(fluid_hashtable_iter_t *iter, fluid_hashtable_t *hashtable);
int fluid_hashtable_iter_next(fluid_hashtable_iter_t *iter, void **key, void **value);
fluid_hashtable_t *fluid_hashtable_iter_get_hash_table(fluid_hashtable_iter_t *iter);
void fluid_hashtable_iter_remove(fluid_hashtable_iter_t *iter);
void fluid_hashtable_iter_steal(fluid_hashtable_iter_t *iter);

fluid_hashtable_t *fluid_hashtable_ref(fluid_hashtable_t *hashtable);
void fluid_hashtable_unref(fluid_hashtable_t *hashtable);

void *fluid_hashtable_lookup(fluid_hashtable_t *hashtable, const void *key);
int fluid_hashtable_lookup_extended(fluid_hashtable_t *hashtable, const void *lookup_key,
                                    void **orig_key, void **value);

void fluid_hashtable_insert(fluid_hashtable_t *hashtable, void *key, void *value);
void fluid_hashtable_replace(fluid_hashtable_t *hashtable, void *key, void *value);

int fluid_hashtable_remove(fluid_hashtable_t *hashtable, const void *key);
int fluid_hashtable_steal(fluid_hashtable_t *hashtable, const void *key);
void fluid_hashtable_remove_all(fluid_hashtable_t *hashtable);
void fluid_hashtable_steal_all(fluid_hashtable_t *hashtable);
unsigned int fluid_hashtable_foreach_steal(fluid_hashtable_t *hashtable,
        fluid_hr_func_t func, void *user_data);
void fluid_hashtable_foreach(fluid_hashtable_t *hashtable, fluid_hr_func_t func,
                             void *user_data);
void *fluid_hashtable_find(fluid_hashtable_t *hashtable, fluid_hr_func_t predicate,
                           void *user_data);
unsigned int fluid_hashtable_size(fluid_hashtable_t *hashtable);
fluid_list_t *fluid_hashtable_get_keys(fluid_hashtable_t *hashtable);
fluid_list_t *fluid_hashtable_get_values(fluid_hashtable_t *hashtable);

int fluid_str_equal(const void *v1, const void *v2);
unsigned int fluid_str_hash(const void *v);
int fluid_direct_equal(const void *v1, const void *v2);
unsigned int fluid_direct_hash(const void *v);
int fluid_int_equal(const void *v1, const void *v2);
unsigned int fluid_int_hash(const void *v);

#endif /* _FLUID_HASH_H */

