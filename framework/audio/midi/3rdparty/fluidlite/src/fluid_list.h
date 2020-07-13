/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _FLUID_LIST_H
#define _FLUID_LIST_H

#include "fluidsynth_priv.h"

/*
 *
 * Lists
 *
 * A sound font loader has to pack the data from the .SF2 file into
 * list structures of this type.
 *
 */

typedef struct _fluid_list_t fluid_list_t;

typedef int (*fluid_compare_func_t)(void* a, void* b);

struct _fluid_list_t
{
  void* data;
  fluid_list_t *next;
};

fluid_list_t* new_fluid_list(void);
void delete_fluid_list(fluid_list_t *list);
void delete1_fluid_list(fluid_list_t *list);
fluid_list_t* fluid_list_sort(fluid_list_t *list, fluid_compare_func_t compare_func);
fluid_list_t* fluid_list_append(fluid_list_t *list, void* data);
fluid_list_t* fluid_list_prepend(fluid_list_t *list, void* data);
fluid_list_t* fluid_list_remove(fluid_list_t *list, void* data);
fluid_list_t* fluid_list_remove_link(fluid_list_t *list, fluid_list_t *llink);
fluid_list_t* fluid_list_nth(fluid_list_t *list, int n);
fluid_list_t* fluid_list_last(fluid_list_t *list);
fluid_list_t* fluid_list_insert_at(fluid_list_t *list, int n, void* data);
int fluid_list_size(fluid_list_t *list);

#define fluid_list_next(slist)	((slist) ? (((fluid_list_t *)(slist))->next) : NULL)
#define fluid_list_get(slist)	((slist) ? ((slist)->data) : NULL)


#endif  /* _FLUID_LIST_H */
