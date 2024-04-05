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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
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
 *
 * Adapted for FluidSynth use by Josh Green <jgreen@users.sourceforge.net>
 * September 8, 2009 from glib 2.18.4
 */

/*
 * MT safe
 */

#include "fluid_sys.h"
#include "fluid_hash.h"
#include "fluid_list.h"


#define HASH_TABLE_MIN_SIZE 11
#define HASH_TABLE_MAX_SIZE 13845163


typedef struct
{
    fluid_hashtable_t *hashtable;
    fluid_hashnode_t *prev_node;
    fluid_hashnode_t *node;
    int position;
    int pre_advanced;	// Boolean
    int version;
} RealIter;


/* Excerpt from glib gprimes.c */

static const unsigned int primes[] =
{
    11,
    19,
    37,
    73,
    109,
    163,
    251,
    367,
    557,
    823,
    1237,
    1861,
    2777,
    4177,
    6247,
    9371,
    14057,
    21089,
    31627,
    47431,
    71143,
    106721,
    160073,
    240101,
    360163,
    540217,
    810343,
    1215497,
    1823231,
    2734867,
    4102283,
    6153409,
    9230113,
    13845163,
};

static const unsigned int nprimes = FLUID_N_ELEMENTS(primes);

static unsigned int
spaced_primes_closest(unsigned int num)
{
    unsigned int i;

    for(i = 0; i < nprimes; i++)
    {
        if(primes[i] > num)
        {
            return primes[i];
        }
    }

    return primes[nprimes - 1];
}

/* End excerpt from glib gprimes.c */


/*
 * @hashtable: our #fluid_hashtable_t
 * @key: the key to lookup against
 * @hash_return: optional key hash return location
 * Return value: a pointer to the described #fluid_hashnode_t pointer
 *
 * Performs a lookup in the hash table.  Virtually all hash operations
 * will use this function internally.
 *
 * This function first computes the hash value of the key using the
 * user's hash function.
 *
 * If an entry in the table matching @key is found then this function
 * returns a pointer to the pointer to that entry in the table.  In
 * the case that the entry is at the head of a chain, this pointer
 * will be an item in the nodes[] array.  In the case that the entry
 * is not at the head of a chain, this pointer will be the ->next
 * pointer on the node that precedes it.
 *
 * In the case that no matching entry exists in the table, a pointer
 * to a %NULL pointer will be returned.  To insert a item, this %NULL
 * pointer should be updated to point to the new #fluid_hashnode_t.
 *
 * If @hash_return is a pass-by-reference parameter.  If it is
 * non-%NULL then the computed hash value is returned.  This is to
 * save insertions from having to compute the hash record again for
 * the new record.
 */
static FLUID_INLINE fluid_hashnode_t **
fluid_hashtable_lookup_node(fluid_hashtable_t *hashtable, const void *key,
                            unsigned int *hash_return)
{
    fluid_hashnode_t **node_ptr, *node;
    unsigned int hash_value;

    hash_value = (* hashtable->hash_func)(key);
    node_ptr = &hashtable->nodes[hash_value % hashtable->size];

    if(hash_return)
    {
        *hash_return = hash_value;
    }

    /* Hash table lookup needs to be fast.
     *  We therefore remove the extra conditional of testing
     *  whether to call the key_equal_func or not from
     *  the inner loop.
     *
     *  Additional optimisation: first check if our full hash
     *  values are equal so we can avoid calling the full-blown
     *  key equality function in most cases.
     */
    if(hashtable->key_equal_func)
    {
        while((node = *node_ptr))
        {
            if(node->key_hash == hash_value &&
                    hashtable->key_equal_func(node->key, key))
            {
                break;
            }

            node_ptr = &(*node_ptr)->next;
        }
    }
    else
    {
        while((node = *node_ptr))
        {
            if(node->key == key)
            {
                break;
            }

            node_ptr = &(*node_ptr)->next;
        }
    }

    return node_ptr;
}

/*
 * @hashtable: our #fluid_hashtable_t
 * @node_ptr_ptr: a pointer to the return value from
 *   fluid_hashtable_lookup_node()
 * @notify: %TRUE if the destroy notify handlers are to be called
 *
 * Removes a node from the hash table and updates the node count.  The
 * node is freed.  No table resize is performed.
 *
 * If @notify is %TRUE then the destroy notify functions are called
 * for the key and value of the hash node.
 *
 * @node_ptr_ptr is a pass-by-reference in/out parameter.  When the
 * function is called, it should point to the pointer to the node to
 * remove.  This level of indirection is required so that the pointer
 * may be updated appropriately once the node has been removed.
 *
 * Before the function returns, the pointer at @node_ptr_ptr will be
 * updated to point to the position in the table that contains the
 * pointer to the "next" node in the chain.  This makes this function
 * convenient to use from functions that iterate over the entire
 * table.  If there is no further item in the chain then the
 * #fluid_hashnode_t pointer will be %NULL (ie: **node_ptr_ptr == %NULL).
 *
 * Since the pointer in the table to the removed node is replaced with
 * either a pointer to the next node or a %NULL pointer as
 * appropriate, the pointer at the end of @node_ptr_ptr will never be
 * modified at all.  Stay tuned. :)
 */
static void
fluid_hashtable_remove_node(fluid_hashtable_t *hashtable,
                            fluid_hashnode_t  ***node_ptr_ptr, int notify)
{
    fluid_hashnode_t **node_ptr, *node;

    node_ptr = *node_ptr_ptr;
    node = *node_ptr;

    *node_ptr = node->next;

    if(notify && hashtable->key_destroy_func)
    {
        hashtable->key_destroy_func(node->key);
    }

    if(notify && hashtable->value_destroy_func)
    {
        hashtable->value_destroy_func(node->value);
    }

    FLUID_FREE(node);

    hashtable->nnodes--;
}

/*
 * fluid_hashtable_remove_all_nodes:
 * @hashtable: our #fluid_hashtable_t
 * @notify: %TRUE if the destroy notify handlers are to be called
 *
 * Removes all nodes from the table.  Since this may be a precursor to
 * freeing the table entirely, no resize is performed.
 *
 * If @notify is %TRUE then the destroy notify functions are called
 * for the key and value of the hash node.
 */
static void
fluid_hashtable_remove_all_nodes(fluid_hashtable_t *hashtable, int notify)
{
    fluid_hashnode_t **node_ptr;
    int i;

    for(i = 0; i < hashtable->size; i++)
    {
        for(node_ptr = &hashtable->nodes[i]; *node_ptr != NULL;)
        {
            fluid_hashtable_remove_node(hashtable, &node_ptr, notify);
        }
    }

    hashtable->nnodes = 0;
}

/*
 * fluid_hashtable_resize:
 * @hashtable: our #fluid_hashtable_t
 *
 * Resizes the hash table to the optimal size based on the number of
 * nodes currently held.  If you call this function then a resize will
 * occur, even if one does not need to occur.  Use
 * fluid_hashtable_maybe_resize() instead.
 */
static void
fluid_hashtable_resize(fluid_hashtable_t *hashtable)
{
    fluid_hashnode_t **new_nodes;
    fluid_hashnode_t *node;
    fluid_hashnode_t *next;
    unsigned int hash_val;
    int new_size;
    int i;

    new_size = spaced_primes_closest(hashtable->nnodes);
    new_size = (new_size < HASH_TABLE_MIN_SIZE) ? HASH_TABLE_MIN_SIZE :
               ((new_size > HASH_TABLE_MAX_SIZE) ? HASH_TABLE_MAX_SIZE : new_size);

    new_nodes = FLUID_ARRAY(fluid_hashnode_t *, new_size);

    if(!new_nodes)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return;
    }

    FLUID_MEMSET(new_nodes, 0, new_size * sizeof(fluid_hashnode_t *));

    for(i = 0; i < hashtable->size; i++)
    {
        for(node = hashtable->nodes[i]; node; node = next)
        {
            next = node->next;

            hash_val = node->key_hash % new_size;

            node->next = new_nodes[hash_val];
            new_nodes[hash_val] = node;
        }
    }

    FLUID_FREE(hashtable->nodes);
    hashtable->nodes = new_nodes;
    hashtable->size = new_size;
}

/*
 * fluid_hashtable_maybe_resize:
 * @hashtable: our #fluid_hashtable_t
 *
 * Resizes the hash table, if needed.
 *
 * Essentially, calls fluid_hashtable_resize() if the table has strayed
 * too far from its ideal size for its number of nodes.
 */
static FLUID_INLINE void
fluid_hashtable_maybe_resize(fluid_hashtable_t *hashtable)
{
    int nnodes = hashtable->nnodes;
    int size = hashtable->size;

    if((size >= 3 * nnodes && size > HASH_TABLE_MIN_SIZE) ||
            (3 * size <= nnodes && size < HASH_TABLE_MAX_SIZE))
    {
        fluid_hashtable_resize(hashtable);
    }
}

/**
 * new_fluid_hashtable:
 * @hash_func: a function to create a hash value from a key.
 *   Hash values are used to determine where keys are stored within the
 *   #fluid_hashtable_t data structure. The fluid_direct_hash(), fluid_int_hash() and
 *   fluid_str_hash() functions are provided for some common types of keys.
 *   If hash_func is %NULL, fluid_direct_hash() is used.
 * @key_equal_func: a function to check two keys for equality.  This is
 *   used when looking up keys in the #fluid_hashtable_t.  The fluid_direct_equal(),
 *   fluid_int_equal() and fluid_str_equal() functions are provided for the most
 *   common types of keys. If @key_equal_func is %NULL, keys are compared
 *   directly in a similar fashion to fluid_direct_equal(), but without the
 *   overhead of a function call.
 *
 * Creates a new #fluid_hashtable_t with a reference count of 1.
 *
 * Return value: a new #fluid_hashtable_t.
 **/
fluid_hashtable_t *
new_fluid_hashtable(fluid_hash_func_t hash_func, fluid_equal_func_t key_equal_func)
{
    return new_fluid_hashtable_full(hash_func, key_equal_func, NULL, NULL);
}


/**
 * new_fluid_hashtable_full:
 * @hash_func: a function to create a hash value from a key.
 * @key_equal_func: a function to check two keys for equality.
 * @key_destroy_func: a function to free the memory allocated for the key
 *   used when removing the entry from the #fluid_hashtable_t or %NULL if you
 *   don't want to supply such a function.
 * @value_destroy_func: a function to free the memory allocated for the
 *   value used when removing the entry from the #fluid_hashtable_t or %NULL if
 *   you don't want to supply such a function.
 *
 * Creates a new #fluid_hashtable_t like fluid_hashtable_new() with a reference count
 * of 1 and allows to specify functions to free the memory allocated for the
 * key and value that get called when removing the entry from the #fluid_hashtable_t.
 *
 * Return value: a new #fluid_hashtable_t.
 **/
fluid_hashtable_t *
new_fluid_hashtable_full(fluid_hash_func_t hash_func,
                         fluid_equal_func_t key_equal_func,
                         fluid_destroy_notify_t key_destroy_func,
                         fluid_destroy_notify_t value_destroy_func)
{
    fluid_hashtable_t *hashtable;

    hashtable = FLUID_NEW(fluid_hashtable_t);

    if(!hashtable)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    hashtable->size               = HASH_TABLE_MIN_SIZE;
    hashtable->nnodes             = 0;
    hashtable->hash_func          = hash_func ? hash_func : fluid_direct_hash;
    hashtable->key_equal_func     = key_equal_func;
    fluid_atomic_int_set(&hashtable->ref_count, 1);
    hashtable->key_destroy_func   = key_destroy_func;
    hashtable->value_destroy_func = value_destroy_func;
    hashtable->nodes              = FLUID_ARRAY(fluid_hashnode_t *, hashtable->size);
    if(hashtable->nodes == NULL)
    {
        delete_fluid_hashtable(hashtable);
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }
    FLUID_MEMSET(hashtable->nodes, 0, hashtable->size * sizeof(*hashtable->nodes));

    return hashtable;
}

/**
 * fluid_hashtable_iter_init:
 * @iter: an uninitialized #fluid_hashtable_iter_t.
 * @hashtable: a #fluid_hashtable_t.
 *
 * Initializes a key/value pair iterator and associates it with
 * @hashtable. Modifying the hash table after calling this function
 * invalidates the returned iterator.
 * |[
 * fluid_hashtable_iter_t iter;
 * gpointer key, value;
 *
 * fluid_hashtable_iter_init (&iter, hashtable);
 * while (fluid_hashtable_iter_next (&iter, &key, &value))
 *   {
 *     /&ast; do something with key and value &ast;/
 *   }
 * ]|
 *
 * Since: 2.16
 **/
void
fluid_hashtable_iter_init(fluid_hashtable_iter_t *iter,
                          fluid_hashtable_t *hashtable)
{
    RealIter *ri = (RealIter *) iter;

    fluid_return_if_fail(iter != NULL);
    fluid_return_if_fail(hashtable != NULL);

    ri->hashtable = hashtable;
    ri->prev_node = NULL;
    ri->node = NULL;
    ri->position = -1;
    ri->pre_advanced = FALSE;
}

/**
 * fluid_hashtable_iter_next:
 * @iter: an initialized #fluid_hashtable_iter_t.
 * @key: a location to store the key, or %NULL.
 * @value: a location to store the value, or %NULL.
 *
 * Advances @iter and retrieves the key and/or value that are now
 * pointed to as a result of this advancement. If %FALSE is returned,
 * @key and @value are not set, and the iterator becomes invalid.
 *
 * Return value: %FALSE if the end of the #fluid_hashtable_t has been reached.
 *
 * Since: 2.16
 **/
int
fluid_hashtable_iter_next(fluid_hashtable_iter_t *iter, void **key,
                          void **value)
{
    RealIter *ri = (RealIter *) iter;

    fluid_return_val_if_fail(iter != NULL, FALSE);

    if(ri->pre_advanced)
    {
        ri->pre_advanced = FALSE;

        if(ri->node == NULL)
        {
            return FALSE;
        }
    }
    else
    {
        if(ri->node != NULL)
        {
            ri->prev_node = ri->node;
            ri->node = ri->node->next;
        }

        while(ri->node == NULL)
        {
            ri->position++;

            if(ri->position >= ri->hashtable->size)
            {
                return FALSE;
            }

            ri->prev_node = NULL;
            ri->node = ri->hashtable->nodes[ri->position];
        }
    }

    if(key != NULL)
    {
        *key = ri->node->key;
    }

    if(value != NULL)
    {
        *value = ri->node->value;
    }

    return TRUE;
}

/**
 * fluid_hashtable_iter_get_hash_table:
 * @iter: an initialized #fluid_hashtable_iter_t.
 *
 * Returns the #fluid_hashtable_t associated with @iter.
 *
 * Return value: the #fluid_hashtable_t associated with @iter.
 *
 * Since: 2.16
 **/
fluid_hashtable_t *
fluid_hashtable_iter_get_hash_table(fluid_hashtable_iter_t *iter)
{
    fluid_return_val_if_fail(iter != NULL, NULL);

    return ((RealIter *) iter)->hashtable;
}

static void
iter_remove_or_steal(RealIter *ri, int notify)
{
    fluid_hashnode_t *prev;
    fluid_hashnode_t *node;
    int position;

    fluid_return_if_fail(ri != NULL);
    fluid_return_if_fail(ri->node != NULL);

    prev = ri->prev_node;
    node = ri->node;
    position = ri->position;

    /* pre-advance the iterator since we will remove the node */

    ri->node = ri->node->next;
    /* ri->prev_node is still the correct previous node */

    while(ri->node == NULL)
    {
        ri->position++;

        if(ri->position >= ri->hashtable->size)
        {
            break;
        }

        ri->prev_node = NULL;
        ri->node = ri->hashtable->nodes[ri->position];
    }

    ri->pre_advanced = TRUE;

    /* remove the node */

    if(prev != NULL)
    {
        prev->next = node->next;
    }
    else
    {
        ri->hashtable->nodes[position] = node->next;
    }

    if(notify)
    {
        if(ri->hashtable->key_destroy_func)
        {
            ri->hashtable->key_destroy_func(node->key);
        }

        if(ri->hashtable->value_destroy_func)
        {
            ri->hashtable->value_destroy_func(node->value);
        }
    }

    FLUID_FREE(node);

    ri->hashtable->nnodes--;
}

/**
 * fluid_hashtable_iter_remove():
 * @iter: an initialized #fluid_hashtable_iter_t.
 *
 * Removes the key/value pair currently pointed to by the iterator
 * from its associated #fluid_hashtable_t. Can only be called after
 * fluid_hashtable_iter_next() returned %TRUE, and cannot be called more
 * than once for the same key/value pair.
 *
 * If the #fluid_hashtable_t was created using fluid_hashtable_new_full(), the
 * key and value are freed using the supplied destroy functions, otherwise
 * you have to make sure that any dynamically allocated values are freed
 * yourself.
 *
 * Since: 2.16
 **/
void
fluid_hashtable_iter_remove(fluid_hashtable_iter_t *iter)
{
    iter_remove_or_steal((RealIter *) iter, TRUE);
}

/**
 * fluid_hashtable_iter_steal():
 * @iter: an initialized #fluid_hashtable_iter_t.
 *
 * Removes the key/value pair currently pointed to by the iterator
 * from its associated #fluid_hashtable_t, without calling the key and value
 * destroy functions. Can only be called after
 * fluid_hashtable_iter_next() returned %TRUE, and cannot be called more
 * than once for the same key/value pair.
 *
 * Since: 2.16
 **/
void
fluid_hashtable_iter_steal(fluid_hashtable_iter_t *iter)
{
    iter_remove_or_steal((RealIter *) iter, FALSE);
}


/**
 * fluid_hashtable_ref:
 * @hashtable: a valid #fluid_hashtable_t.
 *
 * Atomically increments the reference count of @hashtable by one.
 * This function is MT-safe and may be called from any thread.
 *
 * Return value: the passed in #fluid_hashtable_t.
 *
 * Since: 2.10
 **/
fluid_hashtable_t *
fluid_hashtable_ref(fluid_hashtable_t *hashtable)
{
    fluid_return_val_if_fail(hashtable != NULL, NULL);
    fluid_return_val_if_fail(fluid_atomic_int_get(&hashtable->ref_count) > 0, hashtable);

    fluid_atomic_int_add(&hashtable->ref_count, 1);
    return hashtable;
}

/**
 * fluid_hashtable_unref:
 * @hashtable: a valid #fluid_hashtable_t.
 *
 * Atomically decrements the reference count of @hashtable by one.
 * If the reference count drops to 0, all keys and values will be
 * destroyed, and all memory allocated by the hash table is released.
 * This function is MT-safe and may be called from any thread.
 *
 * Since: 2.10
 **/
void
fluid_hashtable_unref(fluid_hashtable_t *hashtable)
{
    fluid_return_if_fail(hashtable != NULL);
    fluid_return_if_fail(fluid_atomic_int_get(&hashtable->ref_count) > 0);

    if(fluid_atomic_int_exchange_and_add(&hashtable->ref_count, -1) - 1 == 0)
    {
        fluid_hashtable_remove_all_nodes(hashtable, TRUE);
        FLUID_FREE(hashtable->nodes);
        FLUID_FREE(hashtable);
    }
}

/**
 * delete_fluid_hashtable:
 * @hashtable: a #fluid_hashtable_t.
 *
 * Destroys all keys and values in the #fluid_hashtable_t and decrements its
 * reference count by 1. If keys and/or values are dynamically allocated,
 * you should either free them first or create the #fluid_hashtable_t with destroy
 * notifiers using fluid_hashtable_new_full(). In the latter case the destroy
 * functions you supplied will be called on all keys and values during the
 * destruction phase.
 **/
void
delete_fluid_hashtable(fluid_hashtable_t *hashtable)
{
    fluid_return_if_fail(hashtable != NULL);
    fluid_return_if_fail(fluid_atomic_int_get(&hashtable->ref_count) > 0);

    fluid_hashtable_remove_all(hashtable);
    fluid_hashtable_unref(hashtable);
}

/**
 * fluid_hashtable_lookup:
 * @hashtable: a #fluid_hashtable_t.
 * @key: the key to look up.
 *
 * Looks up a key in a #fluid_hashtable_t. Note that this function cannot
 * distinguish between a key that is not present and one which is present
 * and has the value %NULL. If you need this distinction, use
 * fluid_hashtable_lookup_extended().
 *
 * Return value: the associated value, or %NULL if the key is not found.
 **/
void *
fluid_hashtable_lookup(fluid_hashtable_t *hashtable, const void *key)
{
    fluid_hashnode_t *node;

    fluid_return_val_if_fail(hashtable != NULL, NULL);

    node = *fluid_hashtable_lookup_node(hashtable, key, NULL);

    return node ? node->value : NULL;
}

/**
 * fluid_hashtable_lookup_extended:
 * @hashtable: a #fluid_hashtable_t.
 * @lookup_key: the key to look up.
 * @orig_key: returns the original key.
 * @value: returns the value associated with the key.
 *
 * Looks up a key in the #fluid_hashtable_t, returning the original key and the
 * associated value and a #gboolean which is %TRUE if the key was found. This
 * is useful if you need to free the memory allocated for the original key,
 * for example before calling fluid_hashtable_remove().
 *
 * Return value: %TRUE if the key was found in the #fluid_hashtable_t.
 **/
int
fluid_hashtable_lookup_extended(fluid_hashtable_t *hashtable,
                                const void *lookup_key,
                                void **orig_key, void **value)
{
    fluid_hashnode_t *node;

    fluid_return_val_if_fail(hashtable != NULL, FALSE);

    node = *fluid_hashtable_lookup_node(hashtable, lookup_key, NULL);

    if(node == NULL)
    {
        return FALSE;
    }

    if(orig_key)
    {
        *orig_key = node->key;
    }

    if(value)
    {
        *value = node->value;
    }

    return TRUE;
}

/*
 * fluid_hashtable_insert_internal:
 * @hashtable: our #fluid_hashtable_t
 * @key: the key to insert
 * @value: the value to insert
 * @keep_new_key: if %TRUE and this key already exists in the table
 *   then call the destroy notify function on the old key.  If %FALSE
 *   then call the destroy notify function on the new key.
 *
 * Implements the common logic for the fluid_hashtable_insert() and
 * fluid_hashtable_replace() functions.
 *
 * Do a lookup of @key.  If it is found, replace it with the new
 * @value (and perhaps the new @key).  If it is not found, create a
 * new node.
 */
static void
fluid_hashtable_insert_internal(fluid_hashtable_t *hashtable, void *key,
                                void *value, int keep_new_key)
{
    fluid_hashnode_t **node_ptr, *node;
    unsigned int key_hash;

    fluid_return_if_fail(hashtable != NULL);
    fluid_return_if_fail(fluid_atomic_int_get(&hashtable->ref_count) > 0);

    node_ptr = fluid_hashtable_lookup_node(hashtable, key, &key_hash);

    if((node = *node_ptr))
    {
        if(keep_new_key)
        {
            if(hashtable->key_destroy_func)
            {
                hashtable->key_destroy_func(node->key);
            }

            node->key = key;
        }
        else
        {
            if(hashtable->key_destroy_func)
            {
                hashtable->key_destroy_func(key);
            }
        }

        if(hashtable->value_destroy_func)
        {
            hashtable->value_destroy_func(node->value);
        }

        node->value = value;
    }
    else
    {
        node = FLUID_NEW(fluid_hashnode_t);

        if(!node)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            return;
        }

        node->key = key;
        node->value = value;
        node->key_hash = key_hash;
        node->next = NULL;

        *node_ptr = node;
        hashtable->nnodes++;
        fluid_hashtable_maybe_resize(hashtable);
    }
}

/**
 * fluid_hashtable_insert:
 * @hashtable: a #fluid_hashtable_t.
 * @key: a key to insert.
 * @value: the value to associate with the key.
 *
 * Inserts a new key and value into a #fluid_hashtable_t.
 *
 * If the key already exists in the #fluid_hashtable_t its current value is replaced
 * with the new value. If you supplied a @value_destroy_func when creating the
 * #fluid_hashtable_t, the old value is freed using that function. If you supplied
 * a @key_destroy_func when creating the #fluid_hashtable_t, the passed key is freed
 * using that function.
 **/
void
fluid_hashtable_insert(fluid_hashtable_t *hashtable, void *key, void *value)
{
    fluid_hashtable_insert_internal(hashtable, key, value, FALSE);
}

/**
 * fluid_hashtable_replace:
 * @hashtable: a #fluid_hashtable_t.
 * @key: a key to insert.
 * @value: the value to associate with the key.
 *
 * Inserts a new key and value into a #fluid_hashtable_t similar to
 * fluid_hashtable_insert(). The difference is that if the key already exists
 * in the #fluid_hashtable_t, it gets replaced by the new key. If you supplied a
 * @value_destroy_func when creating the #fluid_hashtable_t, the old value is freed
 * using that function. If you supplied a @key_destroy_func when creating the
 * #fluid_hashtable_t, the old key is freed using that function.
 **/
void
fluid_hashtable_replace(fluid_hashtable_t *hashtable, void *key, void *value)
{
    fluid_hashtable_insert_internal(hashtable, key, value, TRUE);
}

/*
 * fluid_hashtable_remove_internal:
 * @hashtable: our #fluid_hashtable_t
 * @key: the key to remove
 * @notify: %TRUE if the destroy notify handlers are to be called
 * Return value: %TRUE if a node was found and removed, else %FALSE
 *
 * Implements the common logic for the fluid_hashtable_remove() and
 * fluid_hashtable_steal() functions.
 *
 * Do a lookup of @key and remove it if it is found, calling the
 * destroy notify handlers only if @notify is %TRUE.
 */
static int
fluid_hashtable_remove_internal(fluid_hashtable_t *hashtable, const void *key,
                                int notify)
{
    fluid_hashnode_t **node_ptr;

    fluid_return_val_if_fail(hashtable != NULL, FALSE);

    node_ptr = fluid_hashtable_lookup_node(hashtable, key, NULL);

    if(*node_ptr == NULL)
    {
        return FALSE;
    }

    fluid_hashtable_remove_node(hashtable, &node_ptr, notify);
    fluid_hashtable_maybe_resize(hashtable);

    return TRUE;
}

/**
 * fluid_hashtable_remove:
 * @hashtable: a #fluid_hashtable_t.
 * @key: the key to remove.
 *
 * Removes a key and its associated value from a #fluid_hashtable_t.
 *
 * If the #fluid_hashtable_t was created using fluid_hashtable_new_full(), the
 * key and value are freed using the supplied destroy functions, otherwise
 * you have to make sure that any dynamically allocated values are freed
 * yourself.
 *
 * Return value: %TRUE if the key was found and removed from the #fluid_hashtable_t.
 **/
int
fluid_hashtable_remove(fluid_hashtable_t *hashtable, const void *key)
{
    return fluid_hashtable_remove_internal(hashtable, key, TRUE);
}

/**
 * fluid_hashtable_steal:
 * @hashtable: a #fluid_hashtable_t.
 * @key: the key to remove.
 *
 * Removes a key and its associated value from a #fluid_hashtable_t without
 * calling the key and value destroy functions.
 *
 * Return value: %TRUE if the key was found and removed from the #fluid_hashtable_t.
 **/
int
fluid_hashtable_steal(fluid_hashtable_t *hashtable, const void *key)
{
    return fluid_hashtable_remove_internal(hashtable, key, FALSE);
}

/**
 * fluid_hashtable_remove_all:
 * @hashtable: a #fluid_hashtable_t
 *
 * Removes all keys and their associated values from a #fluid_hashtable_t.
 *
 * If the #fluid_hashtable_t was created using fluid_hashtable_new_full(), the keys
 * and values are freed using the supplied destroy functions, otherwise you
 * have to make sure that any dynamically allocated values are freed
 * yourself.
 *
 * Since: 2.12
 **/
void
fluid_hashtable_remove_all(fluid_hashtable_t *hashtable)
{
    fluid_return_if_fail(hashtable != NULL);

    fluid_hashtable_remove_all_nodes(hashtable, TRUE);
    fluid_hashtable_maybe_resize(hashtable);
}

/**
 * fluid_hashtable_steal_all:
 * @hashtable: a #fluid_hashtable_t.
 *
 * Removes all keys and their associated values from a #fluid_hashtable_t
 * without calling the key and value destroy functions.
 *
 * Since: 2.12
 **/
void
fluid_hashtable_steal_all(fluid_hashtable_t *hashtable)
{
    fluid_return_if_fail(hashtable != NULL);

    fluid_hashtable_remove_all_nodes(hashtable, FALSE);
    fluid_hashtable_maybe_resize(hashtable);
}

/*
 * fluid_hashtable_foreach_remove_or_steal:
 * @hashtable: our #fluid_hashtable_t
 * @func: the user's callback function
 * @user_data: data for @func
 * @notify: %TRUE if the destroy notify handlers are to be called
 *
 * Implements the common logic for fluid_hashtable_foreach_remove() and
 * fluid_hashtable_foreach_steal().
 *
 * Iterates over every node in the table, calling @func with the key
 * and value of the node (and @user_data).  If @func returns %TRUE the
 * node is removed from the table.
 *
 * If @notify is true then the destroy notify handlers will be called
 * for each removed node.
 */
static unsigned int
fluid_hashtable_foreach_remove_or_steal(fluid_hashtable_t *hashtable,
                                        fluid_hr_func_t func, void *user_data,
                                        int notify)
{
    fluid_hashnode_t *node, **node_ptr;
    unsigned int deleted = 0;
    int i;

    for(i = 0; i < hashtable->size; i++)
    {
        for(node_ptr = &hashtable->nodes[i]; (node = *node_ptr) != NULL;)
        {
            if((* func)(node->key, node->value, user_data))
            {
                fluid_hashtable_remove_node(hashtable, &node_ptr, notify);
                deleted++;
            }
            else
            {
                node_ptr = &node->next;
            }
        }
    }

    fluid_hashtable_maybe_resize(hashtable);

    return deleted;
}

#if 0
/**
 * fluid_hashtable_foreach_remove:
 * @hashtable: a #fluid_hashtable_t.
 * @func: the function to call for each key/value pair.
 * @user_data: user data to pass to the function.
 *
 * Calls the given function for each key/value pair in the #fluid_hashtable_t.
 * If the function returns %TRUE, then the key/value pair is removed from the
 * #fluid_hashtable_t. If you supplied key or value destroy functions when creating
 * the #fluid_hashtable_t, they are used to free the memory allocated for the removed
 * keys and values.
 *
 * See #fluid_hashtable_iter_t for an alternative way to loop over the
 * key/value pairs in the hash table.
 *
 * Return value: the number of key/value pairs removed.
 **/
static unsigned int
fluid_hashtable_foreach_remove(fluid_hashtable_t *hashtable,
                               fluid_hr_func_t func, void *user_data)
{
    fluid_return_val_if_fail(hashtable != NULL, 0);
    fluid_return_val_if_fail(func != NULL, 0);

    return fluid_hashtable_foreach_remove_or_steal(hashtable, func, user_data, TRUE);
}
#endif

/**
 * fluid_hashtable_foreach_steal:
 * @hashtable: a #fluid_hashtable_t.
 * @func: the function to call for each key/value pair.
 * @user_data: user data to pass to the function.
 *
 * Calls the given function for each key/value pair in the #fluid_hashtable_t.
 * If the function returns %TRUE, then the key/value pair is removed from the
 * #fluid_hashtable_t, but no key or value destroy functions are called.
 *
 * See #fluid_hashtable_iter_t for an alternative way to loop over the
 * key/value pairs in the hash table.
 *
 * Return value: the number of key/value pairs removed.
 **/
unsigned int
fluid_hashtable_foreach_steal(fluid_hashtable_t *hashtable,
                              fluid_hr_func_t func, void *user_data)
{
    fluid_return_val_if_fail(hashtable != NULL, 0);
    fluid_return_val_if_fail(func != NULL, 0);

    return fluid_hashtable_foreach_remove_or_steal(hashtable, func, user_data, FALSE);
}

/**
 * fluid_hashtable_foreach:
 * @hashtable: a #fluid_hashtable_t.
 * @func: the function to call for each key/value pair.
 * @user_data: user data to pass to the function.
 *
 * Calls the given function for each of the key/value pairs in the
 * #fluid_hashtable_t.  The function is passed the key and value of each
 * pair, and the given @user_data parameter.  The hash table may not
 * be modified while iterating over it (you can't add/remove
 * items). To remove all items matching a predicate, use
 * fluid_hashtable_foreach_remove().
 *
 * See fluid_hashtable_find() for performance caveats for linear
 * order searches in contrast to fluid_hashtable_lookup().
 **/
void
fluid_hashtable_foreach(fluid_hashtable_t *hashtable, fluid_hr_func_t func,
                        void *user_data)
{
    fluid_hashnode_t *node;
    int i;

    fluid_return_if_fail(hashtable != NULL);
    fluid_return_if_fail(func != NULL);

    for(i = 0; i < hashtable->size; i++)
    {
        for(node = hashtable->nodes[i]; node; node = node->next)
        {
            (* func)(node->key, node->value, user_data);
        }
    }
}

/**
 * fluid_hashtable_find:
 * @hashtable: a #fluid_hashtable_t.
 * @predicate:  function to test the key/value pairs for a certain property.
 * @user_data:  user data to pass to the function.
 *
 * Calls the given function for key/value pairs in the #fluid_hashtable_t until
 * @predicate returns %TRUE.  The function is passed the key and value of
 * each pair, and the given @user_data parameter. The hash table may not
 * be modified while iterating over it (you can't add/remove items).
 *
 * Note, that hash tables are really only optimized for forward lookups,
 * i.e. fluid_hashtable_lookup().
 * So code that frequently issues fluid_hashtable_find() or
 * fluid_hashtable_foreach() (e.g. in the order of once per every entry in a
 * hash table) should probably be reworked to use additional or different
 * data structures for reverse lookups (keep in mind that an O(n) find/foreach
 * operation issued for all n values in a hash table ends up needing O(n*n)
 * operations).
 *
 * Return value: The value of the first key/value pair is returned, for which
 * func evaluates to %TRUE. If no pair with the requested property is found,
 * %NULL is returned.
 *
 * Since: 2.4
 **/
void *
fluid_hashtable_find(fluid_hashtable_t *hashtable, fluid_hr_func_t predicate,
                     void *user_data)
{
    fluid_hashnode_t *node;
    int i;

    fluid_return_val_if_fail(hashtable != NULL, NULL);
    fluid_return_val_if_fail(predicate != NULL, NULL);

    for(i = 0; i < hashtable->size; i++)
    {
        for(node = hashtable->nodes[i]; node; node = node->next)
        {
            if(predicate(node->key, node->value, user_data))
            {
                return node->value;
            }
        }
    }

    return NULL;
}

/**
 * fluid_hashtable_size:
 * @hashtable: a #fluid_hashtable_t.
 *
 * Returns the number of elements contained in the #fluid_hashtable_t.
 *
 * Return value: the number of key/value pairs in the #fluid_hashtable_t.
 **/
unsigned int
fluid_hashtable_size(fluid_hashtable_t *hashtable)
{
    fluid_return_val_if_fail(hashtable != NULL, 0);

    return hashtable->nnodes;
}

/**
 * fluid_hashtable_get_keys:
 * @hashtable: a #fluid_hashtable_t
 *
 * Retrieves every key inside @hashtable. The returned data is valid
 * until @hashtable is modified.
 *
 * Return value: a #GList containing all the keys inside the hash
 *   table. The content of the list is owned by the hash table and
 *   should not be modified or freed. Use delete_fluid_list() when done
 *   using the list.
 *
 * Since: 2.14
 */
fluid_list_t *
fluid_hashtable_get_keys(fluid_hashtable_t *hashtable)
{
    fluid_hashnode_t *node;
    int i;
    fluid_list_t *retval;

    fluid_return_val_if_fail(hashtable != NULL, NULL);

    retval = NULL;

    for(i = 0; i < hashtable->size; i++)
    {
        for(node = hashtable->nodes[i]; node; node = node->next)
        {
            retval = fluid_list_prepend(retval, node->key);
        }
    }

    return retval;
}

/**
 * fluid_hashtable_get_values:
 * @hashtable: a #fluid_hashtable_t
 *
 * Retrieves every value inside @hashtable. The returned data is
 * valid until @hashtable is modified.
 *
 * Return value: a #GList containing all the values inside the hash
 *   table. The content of the list is owned by the hash table and
 *   should not be modified or freed. Use delete_fluid_list() when done
 *   using the list.
 *
 * Since: 2.14
 */
fluid_list_t *
fluid_hashtable_get_values(fluid_hashtable_t *hashtable)
{
    fluid_hashnode_t *node;
    int i;
    fluid_list_t *retval;

    fluid_return_val_if_fail(hashtable != NULL, NULL);

    retval = NULL;

    for(i = 0; i < hashtable->size; i++)
    {
        for(node = hashtable->nodes[i]; node; node = node->next)
        {
            retval = fluid_list_prepend(retval, node->value);
        }
    }

    return retval;
}


/* Extracted from glib/gstring.c */


/**
 * fluid_str_equal:
 * @v1: a key
 * @v2: a key to compare with @v1
 *
 * Compares two strings for byte-by-byte equality and returns %TRUE
 * if they are equal. It can be passed to new_fluid_hashtable() as the
 * @key_equal_func parameter, when using strings as keys in a #Ghashtable.
 *
 * Returns: %TRUE if the two keys match
 */
int
fluid_str_equal(const void *v1, const void *v2)
{
    const char *string1 = v1;
    const char *string2 = v2;

    return FLUID_STRCMP(string1, string2) == 0;
}

/**
 * fluid_str_hash:
 * @v: a string key
 *
 * Converts a string to a hash value.
 * It can be passed to new_fluid_hashtable() as the @hash_func
 * parameter, when using strings as keys in a #fluid_hashtable_t.
 *
 * Returns: a hash value corresponding to the key
 */
unsigned int
fluid_str_hash(const void *v)
{
    /* 31 bit hash function */
    const signed char *p = v;
    uint32_t h = *p;

    if(h)
    {
        for(p += 1; *p != '\0'; p++)
        {
            h = (h << 5) - h + *p;
        }
    }

    return h;
}


/* Extracted from glib/gutils.c */


/**
 * fluid_direct_equal:
 * @v1: a key.
 * @v2: a key to compare with @v1.
 *
 * Compares two #gpointer arguments and returns %TRUE if they are equal.
 * It can be passed to new_fluid_hashtable() as the @key_equal_func
 * parameter, when using pointers as keys in a #fluid_hashtable_t.
 *
 * Returns: %TRUE if the two keys match.
 */
int
fluid_direct_equal(const void *v1, const void *v2)
{
    return v1 == v2;
}

/**
 * fluid_direct_hash:
 * @v: a void * key
 *
 * Converts a gpointer to a hash value.
 * It can be passed to g_hashtable_new() as the @hash_func parameter,
 * when using pointers as keys in a #fluid_hashtable_t.
 *
 * Returns: a hash value corresponding to the key.
 */
unsigned int
fluid_direct_hash(const void *v)
{
    return FLUID_POINTER_TO_UINT(v);
}

/**
 * fluid_int_equal:
 * @v1: a pointer to a int key.
 * @v2: a pointer to a int key to compare with @v1.
 *
 * Compares the two #gint values being pointed to and returns
 * %TRUE if they are equal.
 * It can be passed to g_hashtable_new() as the @key_equal_func
 * parameter, when using pointers to integers as keys in a #fluid_hashtable_t.
 *
 * Returns: %TRUE if the two keys match.
 */
int
fluid_int_equal(const void *v1, const void *v2)
{
    return *((const int *) v1) == *((const int *) v2);
}

/**
 * fluid_int_hash:
 * @v: a pointer to a int key
 *
 * Converts a pointer to a #gint to a hash value.
 * It can be passed to g_hashtable_new() as the @hash_func parameter,
 * when using pointers to integers values as keys in a #fluid_hashtable_t.
 *
 * Returns: a hash value corresponding to the key.
 */
unsigned int
fluid_int_hash(const void *v)
{
    return *(const int *) v;
}
