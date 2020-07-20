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
 * MT safe
 */

#include "fluidsynth_priv.h"
#include "fluid_hash.h"


#define HASH_TABLE_MIN_SIZE 7
#define HASH_TABLE_MAX_SIZE 13845163


typedef struct _fluid_hashnode_t fluid_hashnode_t;

struct _fluid_hashnode_t {
  char* key;
  void* value;
  int type;
  fluid_hashnode_t *next;
};

static fluid_hashnode_t* new_fluid_hashnode(char* key, void* value, int type);
static void delete_fluid_hashnode(fluid_hashnode_t *hash_node, fluid_hash_delete_t del);
static void delete_fluid_hashnodes(fluid_hashnode_t *hash_node, fluid_hash_delete_t del);

struct _fluid_hashtable_t {
  unsigned int size;
  unsigned int nnodes;
  fluid_hashnode_t **nodes;
  fluid_hash_delete_t del;
};

#define FLUID_HASHTABLE_RESIZE(hash_table)			\
   if ((3 * hash_table->size <= hash_table->nnodes) 	        \
       && (hash_table->size < HASH_TABLE_MAX_SIZE)) {		\
	   fluid_hashtable_resize(hash_table);			\
   }

static void fluid_hashtable_resize(fluid_hashtable_t *hash_table);
static fluid_hashnode_t** fluid_hashtable_lookup_node(fluid_hashtable_t *hash_table, char* key);

/**
 * new_fluid_hashtable:
 *
 * Creates a new #fluid_hashtable_t.
 *
 * Return value: a new #fluid_hashtable_t.
 **/
fluid_hashtable_t*
new_fluid_hashtable(fluid_hash_delete_t del)
{
  fluid_hashtable_t *hash_table;
  unsigned int i;

  hash_table = FLUID_NEW(fluid_hashtable_t);
  hash_table->size = HASH_TABLE_MIN_SIZE;
  hash_table->nnodes = 0;
  hash_table->nodes = FLUID_ARRAY(fluid_hashnode_t*, hash_table->size);
  hash_table->del = del;

  for (i = 0; i < hash_table->size; i++) {
    hash_table->nodes[i] = NULL;
  }

  return hash_table;
}

/**
 * delete_fluid_hashtable:
 * @hash_table: a #fluid_hashtable_t.
 *
 * Destroys the #fluid_hashtable_t. If keys and/or values are dynamically
 * allocated, you should either free them first or create the #fluid_hashtable_t
 * using fluid_hashtable_new_full(). In the latter case the destroy functions
 * you supplied will be called on all keys and values before destroying
 * the #fluid_hashtable_t.
 **/
void
delete_fluid_hashtable(fluid_hashtable_t *hash_table)
{
  unsigned int i;

  if (hash_table == NULL) {
    return;
  }

  for (i = 0; i < hash_table->size; i++) {
    delete_fluid_hashnodes(hash_table->nodes[i], hash_table->del);
  }

  FLUID_FREE(hash_table->nodes);
  FLUID_FREE(hash_table);
}


static /*inline*/ fluid_hashnode_t**
fluid_hashtable_lookup_node (fluid_hashtable_t* hash_table, char* key)
{
  fluid_hashnode_t **node;

  node = &hash_table->nodes[fluid_str_hash(key) % hash_table->size];

  while (*node && (FLUID_STRCMP((*node)->key, key) != 0)) {
    node = &(*node)->next;
  }

  return node;
}

/**
 * fluid_hashtable_lookup:
 * @hash_table: a #fluid_hashtable_t.
 * @key: the key to look up.
 *
 * Looks up a key in a #fluid_hashtable_t.
 *
 * Return value: the associated value, or %NULL if the key is not found.
 **/
int
fluid_hashtable_lookup(fluid_hashtable_t *hash_table, char* key, void** value, int* type)
{
  fluid_hashnode_t *node;

  node = *fluid_hashtable_lookup_node(hash_table, key);

  if (node) {
    if (value) {
      *value = node->value;
    }
    if (type) {
      *type = node->type;
    }
    return 1;
  } else {
    return 0;
  }
}

/**
 * fluid_hashtable_insert:
 * @hash_table: a #fluid_hashtable_t.
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
fluid_hashtable_insert(fluid_hashtable_t *hash_table, char* key, void* value, int type)
{
  fluid_hashnode_t **node;

  node = fluid_hashtable_lookup_node(hash_table, key);

  if (*node) {
    (*node)->value = value;
    (*node)->type = type;
  } else {
    *node = new_fluid_hashnode(key, value, type);
    hash_table->nnodes++;
    FLUID_HASHTABLE_RESIZE(hash_table);
  }
}


/**
 * fluid_hashtable_replace:
 * @hash_table: a #GHashTable.
 * @key: a key to insert.
 * @value: the value to associate with the key.
 *
 * Inserts a new key and value into a #GHashTable similar to
 * fluid_hashtable_insert(). The difference is that if the key already exists
 * in the #GHashTable, it gets replaced by the new key. If you supplied a
 * @value_destroy_func when creating the #GHashTable, the old value is freed
 * using that function. If you supplied a @key_destroy_func when creating the
 * #GHashTable, the old key is freed using that function.
 **/
void
fluid_hashtable_replace(fluid_hashtable_t *hash_table, char* key, void* value, int type)
{
  fluid_hashnode_t **node;

  node = fluid_hashtable_lookup_node(hash_table, key);

  if (*node) {
    if (hash_table->del) {
      hash_table->del((*node)->value, (*node)->type);
    }
    (*node)->value = value;

  } else {
    *node = new_fluid_hashnode(key, value, type);
    hash_table->nnodes++;
    FLUID_HASHTABLE_RESIZE(hash_table);
  }
}

/**
 * fluid_hashtable_remove:
 * @hash_table: a #fluid_hashtable_t.
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
fluid_hashtable_remove (fluid_hashtable_t *hash_table, char* key)
{
  fluid_hashnode_t **node, *dest;

  node = fluid_hashtable_lookup_node(hash_table, key);
  if (*node) {
    dest = *node;
    (*node) = dest->next;
    delete_fluid_hashnode(dest, hash_table->del);
    hash_table->nnodes--;

    FLUID_HASHTABLE_RESIZE (hash_table);

    return 1;
  }

  return 0;
}

/**
 * fluid_hashtable_foreach:
 * @hash_table: a #fluid_hashtable_t.
 * @func: the function to call for each key/value pair.
 * @user_data: user data to pass to the function.
 *
 * Calls the given function for each of the key/value pairs in the
 * #fluid_hashtable_t.  The function is passed the key and value of each
 * pair, and the given @user_data parameter.  The hash table may not
 * be modified while iterating over it (you can't add/remove
 * items). To remove all items matching a predicate, use
 * fluid_hashtable_remove().
 **/
void
fluid_hashtable_foreach(fluid_hashtable_t *hash_table, fluid_hash_iter_t func, void* data)
{
  fluid_hashnode_t *node = NULL;
  unsigned int i;

  for (i = 0; i < hash_table->size; i++) {
    for (node = hash_table->nodes[i]; node != NULL; node = node->next) {
      (*func)(node->key, node->value, node->type, data);
    }
  }
}

/**
 * fluid_hashtable_size:
 * @hash_table: a #fluid_hashtable_t.
 *
 * Returns the number of elements contained in the #fluid_hashtable_t.
 *
 * Return value: the number of key/value pairs in the #fluid_hashtable_t.
 **/
unsigned int
fluid_hashtable_size(fluid_hashtable_t *hash_table)
{
  return hash_table->nnodes;
}

static void
fluid_hashtable_resize(fluid_hashtable_t *hash_table)
{
  fluid_hashnode_t **new_nodes;
  fluid_hashnode_t *node;
  fluid_hashnode_t *next;
  unsigned int hash_val;
  int new_size;
  unsigned int i;

  new_size = 3 * hash_table->size + 1;
  new_size = (new_size > HASH_TABLE_MAX_SIZE)? HASH_TABLE_MAX_SIZE : new_size;

/*   printf("%s: %d: resizing, new size = %d\n", __FILE__, __LINE__, new_size); */

  new_nodes = FLUID_ARRAY(fluid_hashnode_t*, new_size);
  FLUID_MEMSET(new_nodes, 0, new_size * sizeof(fluid_hashnode_t*));

  for (i = 0; i < hash_table->size; i++) {
    for (node = hash_table->nodes[i]; node; node = next) {
      next = node->next;
      hash_val = fluid_str_hash(node->key) % new_size;
      node->next = new_nodes[hash_val];
      new_nodes[hash_val] = node;
    }
  }

  FLUID_FREE(hash_table->nodes);
  hash_table->nodes = new_nodes;
  hash_table->size = new_size;
}

static fluid_hashnode_t*
new_fluid_hashnode(char* key, void* value, int type)
{
  fluid_hashnode_t *hash_node;

  hash_node = FLUID_NEW(fluid_hashnode_t);

  hash_node->key = FLUID_STRDUP(key);
  hash_node->value = value;
  hash_node->type = type;
  hash_node->next = NULL;

  return hash_node;
}

static void
delete_fluid_hashnode(fluid_hashnode_t *hash_node, fluid_hash_delete_t del)
{
  if (del) {
    (*del)(hash_node->value, hash_node->type);
  }
  if (hash_node->key) {
    FLUID_FREE(hash_node->key);
  }
  FLUID_FREE(hash_node);
}

static void
delete_fluid_hashnodes(fluid_hashnode_t *hash_node, fluid_hash_delete_t del)
{
  while (hash_node) {
    fluid_hashnode_t *next = hash_node->next;
    delete_fluid_hashnode(hash_node, del);
    hash_node = next;
  }
}


/* 31 bit hash function */
unsigned int
fluid_str_hash(char* key)
{
  char *p = key;
  unsigned int h = *p;

  if (h) {
    for (p += 1; *p != '\0'; p++) {
      h = (h << 5) - h + *p;
    }
  }

  return h;
}
