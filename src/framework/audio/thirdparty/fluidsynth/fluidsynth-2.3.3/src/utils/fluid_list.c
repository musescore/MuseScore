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
 * Modified by the GLib Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/.
 */



#include "fluid_sys.h"
#include "fluid_list.h"


fluid_list_t *
new_fluid_list(void)
{
    fluid_list_t *list;
    list = (fluid_list_t *) FLUID_MALLOC(sizeof(fluid_list_t));
    list->data = NULL;
    list->next = NULL;
    return list;
}

void
delete_fluid_list(fluid_list_t *list)
{
    fluid_list_t *next;
    fluid_return_if_fail(list != NULL);

    while(list)
    {
        next = list->next;
        FLUID_FREE(list);
        list = next;
    }
}

void
delete1_fluid_list(fluid_list_t *list)
{
    FLUID_FREE(list);
}

fluid_list_t *
fluid_list_append(fluid_list_t *list, void  *data)
{
    fluid_list_t *new_list;
    fluid_list_t *last;

    new_list = new_fluid_list();
    new_list->data = data;

    if(list)
    {
        last = fluid_list_last(list);
        /* g_assert (last != NULL); */
        last->next = new_list;

        return list;
    }
    else
    {
        return new_list;
    }
}

fluid_list_t *
fluid_list_prepend(fluid_list_t *list, void *data)
{
    fluid_list_t *new_list;

    new_list = new_fluid_list();
    new_list->data = data;
    new_list->next = list;

    return new_list;
}

fluid_list_t *
fluid_list_nth(fluid_list_t *list, int n)
{
    while((n-- > 0) && list)
    {
        list = list->next;
    }

    return list;
}

fluid_list_t *
fluid_list_remove(fluid_list_t *list, void *data)
{
    fluid_list_t *tmp;
    fluid_list_t *prev;

    prev = NULL;
    tmp = list;

    while(tmp)
    {
        if(tmp->data == data)
        {
            if(prev)
            {
                prev->next = tmp->next;
            }

            if(list == tmp)
            {
                list = list->next;
            }

            tmp->next = NULL;
            delete_fluid_list(tmp);

            break;
        }

        prev = tmp;
        tmp = tmp->next;
    }

    return list;
}

fluid_list_t *
fluid_list_remove_link(fluid_list_t *list, fluid_list_t *link)
{
    fluid_list_t *tmp;
    fluid_list_t *prev;

    prev = NULL;
    tmp = list;

    while(tmp)
    {
        if(tmp == link)
        {
            if(prev)
            {
                prev->next = tmp->next;
            }

            if(list == tmp)
            {
                list = list->next;
            }

            tmp->next = NULL;
            break;
        }

        prev = tmp;
        tmp = tmp->next;
    }

    return list;
}

static fluid_list_t *
fluid_list_sort_merge(fluid_list_t *l1, fluid_list_t *l2, fluid_compare_func_t compare_func)
{
    fluid_list_t list, *l;

    l = &list;

    while(l1 && l2)
    {
        if(compare_func(l1->data, l2->data) < 0)
        {
            l = l->next = l1;
            l1 = l1->next;
        }
        else
        {
            l = l->next = l2;
            l2 = l2->next;
        }
    }

    l->next = l1 ? l1 : l2;

    return list.next;
}

fluid_list_t *
fluid_list_sort(fluid_list_t *list, fluid_compare_func_t compare_func)
{
    fluid_list_t *l1, *l2;

    if(!list)
    {
        return NULL;
    }

    if(!list->next)
    {
        return list;
    }

    l1 = list;
    l2 = list->next;

    while((l2 = l2->next) != NULL)
    {
        if((l2 = l2->next) == NULL)
        {
            break;
        }

        l1 = l1->next;
    }

    l2 = l1->next;
    l1->next = NULL;

    return fluid_list_sort_merge(fluid_list_sort(list, compare_func),
                                 fluid_list_sort(l2, compare_func),
                                 compare_func);
}


fluid_list_t *
fluid_list_last(fluid_list_t *list)
{
    if(list)
    {
        while(list->next)
        {
            list = list->next;
        }
    }

    return list;
}

int
fluid_list_size(fluid_list_t *list)
{
    int n = 0;

    while(list)
    {
        n++;
        list = list->next;
    }

    return n;
}

fluid_list_t *fluid_list_insert_at(fluid_list_t *list, int n, void *data)
{
    fluid_list_t *new_list;
    fluid_list_t *cur;
    fluid_list_t *prev = NULL;

    new_list = new_fluid_list();
    new_list->data = data;

    cur = list;

    while((n-- > 0) && cur)
    {
        prev = cur;
        cur = cur->next;
    }

    new_list->next = cur;

    if(prev)
    {
        prev->next = new_list;
        return list;
    }
    else
    {
        return new_list;
    }
}

/* Compare function to sort strings alphabetically,
 * for use with fluid_list_sort(). */
int
fluid_list_str_compare_func(const void *a, const void *b)
{
    if(a && b)
    {
        return FLUID_STRCMP(a, b);
    }

    if(!a && !b)
    {
        return 0;
    }

    if(a)
    {
        return -1;
    }

    return 1;
}

int fluid_list_idx(fluid_list_t *list, void *data)
{
    int i = 0;

    while(list)
    {
        if (list->data == data)
        {
            return i;
        }
        list = list->next;
    }

    return -1;
}
