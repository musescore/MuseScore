/*
 * Copyright © 2018  Ebrahim Byagowi
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#include "hb-test.h"

/* Unit tests for hb-map.h */


static void
test_map_basic (void)
{
  hb_map_t *empty = hb_map_get_empty ();
  hb_map_t *m;
  g_assert (hb_map_is_empty (empty));
  g_assert (!hb_map_allocation_successful (empty));
  hb_map_destroy (empty);

  m = hb_map_create ();
  g_assert (hb_map_allocation_successful (m));
  g_assert (hb_map_is_empty (m));

  hb_map_set (m, 213, 223);
  hb_map_set (m, 643, 675);
  g_assert_cmpint (hb_map_get_population (m), ==, 2);

  g_assert_cmpint (hb_map_get (m, 213), ==, 223);
  g_assert (!hb_map_has (m, 123));
  g_assert (hb_map_has (m, 213));

  hb_map_del (m, 213);
  g_assert (!hb_map_has (m, 213));

  g_assert_cmpint (hb_map_get (m, 643), ==, 675);
  hb_map_set (m, 237, 673);
  g_assert (hb_map_has (m, 237));
  hb_map_clear (m);
  g_assert (!hb_map_has (m, 237));
  g_assert (!hb_map_has (m, 643));
  g_assert_cmpint (hb_map_get_population (m), ==, 0);

  hb_map_destroy (m);
}

static void
test_map_userdata (void)
{
  hb_map_t *m = hb_map_create ();

  hb_user_data_key_t key[2];
  int *data = (int *) malloc (sizeof (int));
  int *data2;
  *data = 3123;
  hb_map_set_user_data (m, &key[0], data, free, TRUE);
  g_assert_cmpint (*((int *) hb_map_get_user_data (m, &key[0])), ==, 3123);

  data2 = (int *) malloc (sizeof (int));
  *data2 = 6343;
  hb_map_set_user_data (m, &key[0], data2, free, FALSE);
  g_assert_cmpint (*((int *) hb_map_get_user_data (m, &key[0])), ==, 3123);
  hb_map_set_user_data (m, &key[0], data2, free, TRUE);
  g_assert_cmpint (*((int *) hb_map_get_user_data (m, &key[0])), ==, 6343);

  hb_map_destroy (m);
}

static void
test_map_refcount (void)
{
  hb_map_t *m = hb_map_create ();
  hb_map_t *m2;
  hb_map_set (m, 213, 223);
  g_assert_cmpint (hb_map_get (m, 213), ==, 223);

  m2 = hb_map_reference (m);
  hb_map_destroy (m);

  /* We copied its reference so it is still usable after one destroy */
  g_assert (hb_map_has (m, 213));
  g_assert (hb_map_has (m2, 213));

  hb_map_destroy (m2);

  /* Now you can't access them anymore */
}

static void
test_map_get_population (void)
{
  hb_map_t *m = hb_map_create ();

  hb_map_set (m, 12, 21);
  g_assert_cmpint (hb_map_get_population (m), ==, 1);
  hb_map_set (m, 78, 87);
  g_assert_cmpint (hb_map_get_population (m), ==, 2);

  hb_map_set (m, 78, 87);
  g_assert_cmpint (hb_map_get_population (m), ==, 2);
  hb_map_set (m, 78, 13);
  g_assert_cmpint (hb_map_get_population (m), ==, 2);

  hb_map_set (m, 95, 56);
  g_assert_cmpint (hb_map_get_population (m), ==, 3);

  hb_map_del (m, 78);
  g_assert_cmpint (hb_map_get_population (m), ==, 2);

  hb_map_del (m, 103);
  g_assert_cmpint (hb_map_get_population (m), ==, 2);

  hb_map_destroy (m);
}

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  hb_test_add (test_map_basic);
  hb_test_add (test_map_userdata);
  hb_test_add (test_map_refcount);
  hb_test_add (test_map_get_population);

  return hb_test_run();
}
