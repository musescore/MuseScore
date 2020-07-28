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
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/.
 */

/*
 * Demolished by Peter Hanappe [December 2002]
 *
 * - only string as key
 * - stores additional type info
 * - removed use of GLib types (gpointer, gint, ...)
 * - reduced the number of API functions
 * - changed names to fluid_hashtable_...
 */

#ifndef _FLUID_HASH_H
#define _FLUID_HASH_H


typedef int (*fluid_hash_iter_t)(char* key, void* value, int type, void* data);
typedef void (*fluid_hash_delete_t)(void* value, int type);

fluid_hashtable_t* new_fluid_hashtable(fluid_hash_delete_t delete);
void delete_fluid_hashtable(fluid_hashtable_t *hash_table);

void fluid_hashtable_insert(fluid_hashtable_t *hash_table, char* key, void* value, int type);

void fluid_hashtable_replace(fluid_hashtable_t *hash_table, char* key, void* value, int type);

/* Returns non-zero if found, 0 if not found */
int fluid_hashtable_lookup(fluid_hashtable_t *hash_table, char* key, void** value, int* type);

/* Returns non-zero if removed, 0 if not removed */
int fluid_hashtable_remove(fluid_hashtable_t *hash_table, char* key);

void fluid_hashtable_foreach(fluid_hashtable_t *hashtable, fluid_hash_iter_t fun, void* data);

unsigned int fluid_hashtable_size(fluid_hashtable_t *hash_table);

unsigned int fluid_str_hash(char* v);

#endif /* _FLUID_HASH_H */

