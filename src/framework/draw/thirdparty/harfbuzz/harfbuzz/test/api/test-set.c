/*
 * Copyright © 2013  Google, Inc.
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
 *
 * Google Author(s): Behdad Esfahbod
 */

#include "hb-test.h"

/* Unit tests for hb-set.h */


static void
test_empty (hb_set_t *s)
{
  hb_codepoint_t next;
  g_assert_cmpint (hb_set_get_population (s), ==, 0);
  g_assert_cmpint (hb_set_get_min (s), ==, HB_SET_VALUE_INVALID);
  g_assert_cmpint (hb_set_get_max (s), ==, HB_SET_VALUE_INVALID);
  g_assert (!hb_set_has (s, 13));
  next = 53043;
  g_assert (!hb_set_next (s, &next));
  g_assert_cmpint (next, ==, HB_SET_VALUE_INVALID);
  next = 07734;
  g_assert (!hb_set_previous (s, &next));
  g_assert_cmpint (next, ==, HB_SET_VALUE_INVALID);
  g_assert (hb_set_is_empty (s));
}

static void
test_not_empty (hb_set_t *s)
{
  hb_codepoint_t next;
  g_assert_cmpint (hb_set_get_population (s), !=, 0);
  g_assert_cmpint (hb_set_get_min (s), !=, HB_SET_VALUE_INVALID);
  g_assert_cmpint (hb_set_get_max (s), !=, HB_SET_VALUE_INVALID);
  next = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (s, &next));
  g_assert_cmpint (next, !=, HB_SET_VALUE_INVALID);
  next = HB_SET_VALUE_INVALID;
  g_assert (hb_set_previous (s, &next));
  g_assert_cmpint (next, !=, HB_SET_VALUE_INVALID);
}

static void
test_set_basic (void)
{
  hb_set_t *s = hb_set_create ();

  test_empty (s);
  hb_set_add (s, 13);
  test_not_empty (s);

  hb_set_clear (s);
  test_empty (s);

  hb_set_add (s, 33000);
  test_not_empty (s);
  hb_set_clear (s);

  hb_set_add_range (s, 10, 29);
  test_not_empty (s);
  g_assert (hb_set_has (s, 13));
  g_assert_cmpint (hb_set_get_population (s), ==, 20);
  g_assert_cmpint (hb_set_get_min (s), ==, 10);
  g_assert_cmpint (hb_set_get_max (s), ==, 29);

  test_not_empty (s);
  g_assert (hb_set_has (s, 13));
  g_assert_cmpint (hb_set_get_population (s), ==, 20);
  g_assert_cmpint (hb_set_get_min (s), ==, 10);
  g_assert_cmpint (hb_set_get_max (s), ==, 29);

  hb_set_del_range (s, 10, 18);
  test_not_empty (s);
  g_assert (!hb_set_has (s, 13));

  hb_set_add_range (s, 200, 800);
  test_not_empty (s);
  g_assert (!hb_set_has (s, 100));
  g_assert (!hb_set_has (s, 199));
  g_assert (hb_set_has (s, 200));
  g_assert (hb_set_has (s, 201));
  g_assert (hb_set_has (s, 243));
  g_assert (hb_set_has (s, 254));
  g_assert (hb_set_has (s, 255));
  g_assert (hb_set_has (s, 256));
  g_assert (hb_set_has (s, 257));
  g_assert (hb_set_has (s, 511));
  g_assert (hb_set_has (s, 512));
  g_assert (hb_set_has (s, 600));
  g_assert (hb_set_has (s, 767));
  g_assert (hb_set_has (s, 768));
  g_assert (hb_set_has (s, 769));
  g_assert (hb_set_has (s, 782));
  g_assert (hb_set_has (s, 798));
  g_assert (hb_set_has (s, 799));
  g_assert (hb_set_has (s, 800));
  g_assert (!hb_set_has (s, 801));
  g_assert (!hb_set_has (s, 802));

  hb_set_del (s, 800);
  g_assert (!hb_set_has (s, 800));

  g_assert_cmpint (hb_set_get_max (s), ==, 799);

  hb_set_del_range (s, 0, 799);
  g_assert_cmpint (hb_set_get_max (s), ==, HB_SET_VALUE_INVALID);

  hb_set_destroy (s);
}


// static inline void
// print_set (hb_set_t *s)
// {
//   hb_codepoint_t next;
//   printf ("{");
//   for (next = HB_SET_VALUE_INVALID; hb_set_next (s, &next); )
//     printf ("%d, ", next);
//   printf ("}\n");
// }

static void test_set_intersect_empty (void)
{
  hb_set_t* a = hb_set_create ();
  hb_set_add (a, 3585);
  hb_set_add (a, 21333);
  hb_set_add (a, 24405);

  hb_set_t* b = hb_set_create();
  hb_set_add (b, 21483);
  hb_set_add (b, 24064);

  hb_set_intersect (a, b);
  g_assert (hb_set_is_empty (a));

  hb_set_destroy (a);
  hb_set_destroy (b);


  a = hb_set_create ();
  hb_set_add (a, 16777216);

  b = hb_set_create();
  hb_set_add (b, 0);

  hb_set_intersect (a, b);
  g_assert (hb_set_is_empty (a));

  hb_set_destroy (a);
  hb_set_destroy (b);
}

static void test_set_intersect_page_reduction (void)
{
  hb_set_t* a = hb_set_create ();
  hb_set_add (a, 3585);
  hb_set_add (a, 21333);
  hb_set_add (a, 24405);

  hb_set_t* b = hb_set_create();
  hb_set_add (b, 3585);
  hb_set_add (b, 24405);

  hb_set_intersect(a, b);
  g_assert (hb_set_is_equal (a, b));

  hb_set_destroy (a);
  hb_set_destroy (b);
}

static void test_set_union (void)
{
  hb_set_t* a = hb_set_create();
  hb_set_add (a, 3585);
  hb_set_add (a, 21333);
  hb_set_add (a, 24405);

  hb_set_t* b = hb_set_create();
  hb_set_add (b, 21483);
  hb_set_add (b, 24064);

  hb_set_t* u = hb_set_create ();
  hb_set_add (u, 3585);
  hb_set_add (u, 21333);
  hb_set_add (u, 21483);
  hb_set_add (u, 24064);
  hb_set_add (u, 24405);

  hb_set_union(b, a);
  g_assert (hb_set_is_equal (u, b));

  hb_set_destroy (a);
  hb_set_destroy (b);
  hb_set_destroy (u);
}

static void
test_set_subsets (void)
{
  hb_set_t *s = hb_set_create ();
  hb_set_t *l = hb_set_create ();

  hb_set_add (l, 0x0FFFF);
  hb_set_add (s, 0x1FFFF);
  g_assert (!hb_set_is_subset (s, l));
  hb_set_clear (s);

  hb_set_add (s, 0x0FFF0);
  g_assert (!hb_set_is_subset (s, l));
  hb_set_clear (s);

  hb_set_add (s, 0x0AFFF);
  g_assert (!hb_set_is_subset (s, l));

  hb_set_clear (s);
  g_assert (hb_set_is_subset (s, l));

  hb_set_clear (l);
  g_assert (hb_set_is_subset (s, l));

  hb_set_add (s, 0x1FFFF);
  g_assert (!hb_set_is_subset (s, l));
  hb_set_clear (s);

  hb_set_add (s, 0xFF);
  hb_set_add (s, 0x1FFFF);
  hb_set_add (s, 0x2FFFF);

  hb_set_add (l, 0xFF);
  hb_set_add (l, 0x1FFFF);
  hb_set_add (l, 0x2FFFF);

  g_assert (hb_set_is_subset (s, l));
  hb_set_del (l, 0xFF);
  g_assert (!hb_set_is_subset (s, l));
  hb_set_add (l, 0xFF);

  hb_set_del (l, 0x2FFFF);
  g_assert (!hb_set_is_subset (s, l));
  hb_set_add (l, 0x2FFFF);

  hb_set_del (l, 0x1FFFF);
  g_assert (!hb_set_is_subset (s, l));

  hb_set_destroy (s);
  hb_set_destroy (l);
}

static void
test_set_algebra (void)
{
  hb_set_t *s = hb_set_create ();
  hb_set_t *o = hb_set_create ();
  hb_set_t *o2 = hb_set_create ();

  hb_set_add (o, 13);
  hb_set_add (o, 19);

  hb_set_add (o2, 0x660E);

  test_empty (s);
  g_assert (!hb_set_is_equal (s, o));
  g_assert (hb_set_is_subset (s, o));
  g_assert (!hb_set_is_subset (o, s));
  hb_set_set (s, o);
  g_assert (hb_set_is_equal (s, o));
  g_assert (hb_set_is_subset (s, o));
  g_assert (hb_set_is_subset (o, s));
  test_not_empty (s);
  g_assert_cmpint (hb_set_get_population (s), ==, 2);

  hb_set_clear (s);
  test_empty (s);
  hb_set_add (s, 10);
  g_assert_cmpint (hb_set_get_population (s), ==, 1);
  hb_set_union (s, o);
  g_assert_cmpint (hb_set_get_population (s), ==, 3);
  g_assert (hb_set_has (s, 10));
  g_assert (hb_set_has (s, 13));

  hb_set_clear (s);
  test_empty (s);
  g_assert_cmpint (hb_set_get_population (s), ==, 0);
  hb_set_union (s, o2);
  g_assert_cmpint (hb_set_get_population (s), ==, 1);
  g_assert (hb_set_has (s, 0x660E));

  hb_set_clear (s);
  test_empty (s);
  hb_set_add_range (s, 10, 17);
  g_assert (!hb_set_is_equal (s, o));
  hb_set_intersect (s, o);
  g_assert (!hb_set_is_equal (s, o));
  test_not_empty (s);
  g_assert_cmpint (hb_set_get_population (s), ==, 1);
  g_assert (!hb_set_has (s, 10));
  g_assert (hb_set_has (s, 13));

  hb_set_clear (s);
  test_empty (s);
  hb_set_add_range (s, 10, 17);
  g_assert (!hb_set_is_equal (s, o));
  hb_set_subtract (s, o);
  g_assert (!hb_set_is_equal (s, o));
  test_not_empty (s);
  g_assert_cmpint (hb_set_get_population (s), ==, 7);
  g_assert (hb_set_has (s, 12));
  g_assert (!hb_set_has (s, 13));
  g_assert (!hb_set_has (s, 19));

  hb_set_clear (s);
  test_empty (s);
  hb_set_add_range (s, 10, 17);
  g_assert (!hb_set_is_equal (s, o));
  hb_set_symmetric_difference (s, o);
  g_assert (!hb_set_is_equal (s, o));
  test_not_empty (s);
  g_assert_cmpint (hb_set_get_population (s), ==, 8);
  g_assert (hb_set_has (s, 12));
  g_assert (!hb_set_has (s, 13));
  g_assert (hb_set_has (s, 19));

  /* https://github.com/harfbuzz/harfbuzz/issues/579 */
  hb_set_clear (s);
  test_empty (s);
  hb_set_add_range (s, 886, 895);
  hb_set_add (s, 1024);
  hb_set_add (s, 1152);
  hb_set_clear (o);
  test_empty (o);
  hb_set_add (o, 889);
  hb_set_add (o, 1024);
  g_assert (!hb_set_is_equal (s, o));
  hb_set_intersect (o, s);
  test_not_empty (o);
  g_assert (!hb_set_is_equal (s, o));
  g_assert_cmpint (hb_set_get_population (o), ==, 2);
  g_assert (hb_set_has (o, 889));
  g_assert (hb_set_has (o, 1024));
  hb_set_clear (o);
  test_empty (o);
  hb_set_add_range (o, 887, 889);
  hb_set_add (o, 1121);
  g_assert (!hb_set_is_equal (s, o));
  hb_set_intersect (o, s);
  test_not_empty (o);
  g_assert (!hb_set_is_equal (s, o));
  g_assert_cmpint (hb_set_get_population (o), ==, 3);
  g_assert (hb_set_has (o, 887));
  g_assert (hb_set_has (o, 888));
  g_assert (hb_set_has (o, 889));

  hb_set_clear (s);
  test_empty (s);
  hb_set_add_range (s, 886, 895);
  hb_set_add (s, 1014);
  hb_set_add (s, 1017);
  hb_set_add (s, 1024);
  hb_set_add (s, 1113);
  hb_set_add (s, 1121);
  g_assert_cmpint (hb_set_get_population (s), ==, 15);

  hb_set_clear (o);
  test_empty (o);
  hb_set_add (o, 889);
  g_assert_cmpint (hb_set_get_population (o), ==, 1);
  hb_set_intersect (o, s);
  g_assert_cmpint (hb_set_get_population (o), ==, 1);
  g_assert (hb_set_has (o, 889));

  hb_set_add (o, 511);
  g_assert_cmpint (hb_set_get_population (o), ==, 2);
  hb_set_intersect (o, s);
  g_assert_cmpint (hb_set_get_population (o), ==, 1);
  g_assert (hb_set_has (o, 889));

  hb_set_destroy (s);
  hb_set_destroy (o);
  hb_set_destroy (o2);
}

static void
test_set_iter (void)
{
  hb_codepoint_t next, first, last;
  hb_set_t *s = hb_set_create ();

  hb_set_add (s, 13);
  hb_set_add_range (s, 6, 6);
  hb_set_add_range (s, 10, 15);
  hb_set_add (s, 1100);
  hb_set_add (s, 1200);
  hb_set_add (s, 20005);

  test_not_empty (s);

  next = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (s, &next));
  g_assert_cmpint (next, ==, 6);
  g_assert (hb_set_next (s, &next));
  g_assert_cmpint (next, ==, 10);
  g_assert (hb_set_next (s, &next));
  g_assert (hb_set_next (s, &next));
  g_assert (hb_set_next (s, &next));
  g_assert_cmpint (next, ==, 13);
  g_assert (hb_set_next (s, &next));
  g_assert (hb_set_next (s, &next));
  g_assert_cmpint (next, ==, 15);
  g_assert (hb_set_next (s, &next));
  g_assert_cmpint (next, ==, 1100);
  g_assert (hb_set_next (s, &next));
  g_assert_cmpint (next, ==, 1200);
  g_assert (hb_set_next (s, &next));
  g_assert_cmpint (next, ==, 20005);
  g_assert (!hb_set_next (s, &next));
  g_assert_cmpint (next, ==, HB_SET_VALUE_INVALID);

  next = HB_SET_VALUE_INVALID;
  g_assert (hb_set_previous (s, &next));
  g_assert_cmpint (next, ==, 20005);
  g_assert (hb_set_previous (s, &next));
  g_assert_cmpint (next, ==, 1200);
  g_assert (hb_set_previous (s, &next));
  g_assert_cmpint (next, ==, 1100);
  g_assert (hb_set_previous (s, &next));
  g_assert_cmpint (next, ==, 15);
  g_assert (hb_set_previous (s, &next));
  g_assert (hb_set_previous (s, &next));
  g_assert_cmpint (next, ==, 13);
  g_assert (hb_set_previous (s, &next));
  g_assert (hb_set_previous (s, &next));
  g_assert (hb_set_previous (s, &next));
  g_assert_cmpint (next, ==, 10);
  g_assert (hb_set_previous (s, &next));
  g_assert_cmpint (next, ==, 6);
  g_assert (!hb_set_previous (s, &next));
  g_assert_cmpint (next, ==, HB_SET_VALUE_INVALID);

  first = last = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next_range (s, &first, &last));
  g_assert_cmpint (first, ==, 6);
  g_assert_cmpint (last,  ==, 6);
  g_assert (hb_set_next_range (s, &first, &last));
  g_assert_cmpint (first, ==, 10);
  g_assert_cmpint (last,  ==, 15);
  g_assert (hb_set_next_range (s, &first, &last));
  g_assert_cmpint (first, ==, 1100);
  g_assert_cmpint (last,  ==, 1100);
  g_assert (hb_set_next_range (s, &first, &last));
  g_assert_cmpint (first, ==, 1200);
  g_assert_cmpint (last,  ==, 1200);
  g_assert (hb_set_next_range (s, &first, &last));
  g_assert_cmpint (first, ==, 20005);
  g_assert_cmpint (last,  ==, 20005);
  g_assert (!hb_set_next_range (s, &first, &last));
  g_assert_cmpint (first, ==, HB_SET_VALUE_INVALID);
  g_assert_cmpint (last,  ==, HB_SET_VALUE_INVALID);

  first = last = HB_SET_VALUE_INVALID;
  g_assert (hb_set_previous_range (s, &first, &last));
  g_assert_cmpint (first, ==, 20005);
  g_assert_cmpint (last,  ==, 20005);
  g_assert (hb_set_previous_range (s, &first, &last));
  g_assert_cmpint (first, ==, 1200);
  g_assert_cmpint (last,  ==, 1200);
  g_assert (hb_set_previous_range (s, &first, &last));
  g_assert_cmpint (first, ==, 1100);
  g_assert_cmpint (last,  ==, 1100);
  g_assert (hb_set_previous_range (s, &first, &last));
  g_assert_cmpint (first, ==, 10);
  g_assert_cmpint (last,  ==, 15);
  g_assert (hb_set_previous_range (s, &first, &last));
  g_assert_cmpint (first, ==, 6);
  g_assert_cmpint (last,  ==, 6);
  g_assert (!hb_set_previous_range (s, &first, &last));
  g_assert_cmpint (first, ==, HB_SET_VALUE_INVALID);
  g_assert_cmpint (last,  ==, HB_SET_VALUE_INVALID);

  hb_set_destroy (s);
}

static void
test_set_empty (void)
{
  hb_set_t *b = hb_set_get_empty ();

  g_assert (hb_set_get_empty ());
  g_assert (hb_set_get_empty () == b);

  g_assert (!hb_set_allocation_successful (b));

  test_empty (b);

  hb_set_add (b, 13);

  test_empty (b);

  g_assert (!hb_set_allocation_successful (b));

  hb_set_clear (b);

  test_empty (b);

  g_assert (!hb_set_allocation_successful (b));

  hb_set_destroy (b);
}

static void
test_set_delrange (void)
{
  const unsigned P = 512;	/* Page size. */
  struct { unsigned b, e; } ranges[] = {
    { 35, P-15 },		/* From page middle thru middle. */
    { P, P+100 },		/* From page start thru middle. */
    { P+300, P*2-1 },		/* From page middle thru end. */
    { P*3, P*4+100 },		/* From page start thru next page middle. */
    { P*4+300, P*6-1 },		/* From page middle thru next page end. */
    { P*6+200,P*8+100 },	/* From page middle covering one page thru page middle. */
    { P*9, P*10+105 },		/* From page start covering one page thru page middle. */
    { P*10+305, P*12-1 },	/* From page middle covering one page thru page end. */
    { P*13, P*15-1 },		/* From page start covering two pages thru page end. */
    { P*15+100, P*18+100 }	/* From page middle covering two pages thru page middle. */
  };
  unsigned n = sizeof (ranges) / sizeof(ranges[0]);

  hb_set_t *s = hb_set_create ();

  test_empty (s);
  for (unsigned int g = 0; g < ranges[n - 1].e + P; g += 2)
    hb_set_add (s, g);

  hb_set_add (s, P*2-1);
  hb_set_add (s, P*6-1);
  hb_set_add (s, P*12-1);
  hb_set_add (s, P*15-1);

  for (unsigned i = 0; i < n; i++)
    hb_set_del_range (s, ranges[i].b, ranges[i].e);

  hb_set_del_range (s, P*13+5, P*15-10);	/* Deletion from deleted pages. */

  for (unsigned i = 0; i < n; i++)
  {
    unsigned b = ranges[i].b;
    unsigned e = ranges[i].e;
    g_assert (hb_set_has (s, (b-2)&~1));
    while (b <= e)
      g_assert (!hb_set_has (s, b++));
    g_assert (hb_set_has (s, (e+2)&~1));
  }

  hb_set_destroy (s);
}

static const unsigned max_set_elements = -1;

static void
test_set_inverted_basics (void)
{
  // Tests:
  // add, del, has, get_population, is_empty, get_min, get_max
  // for inverted sets.
  hb_set_t *s = hb_set_create ();
  hb_set_invert (s);

  g_assert_cmpint (hb_set_get_population (s), ==, max_set_elements);
  g_assert (hb_set_has (s, 0));
  g_assert (hb_set_has (s, 13));
  g_assert (hb_set_has (s, max_set_elements - 1));
  g_assert (!hb_set_is_empty (s));
  g_assert_cmpint (hb_set_get_min (s), ==, 0);
  g_assert_cmpint (hb_set_get_max (s), ==, max_set_elements - 1);

  hb_set_del (s, 13);
  g_assert (!hb_set_has (s, 13));
  g_assert_cmpint (hb_set_get_population (s), ==, max_set_elements - 1);
  g_assert_cmpint (hb_set_get_min (s), ==, 0);
  g_assert_cmpint (hb_set_get_max (s), ==, max_set_elements - 1);

  hb_set_add (s, 13);
  g_assert (hb_set_has (s, 13));
  g_assert_cmpint (hb_set_get_population (s), ==, max_set_elements);

  hb_set_del (s, 0);
  hb_set_del (s, max_set_elements - 1);
  g_assert (!hb_set_has (s, 0));
  g_assert (hb_set_has (s, 13));
  g_assert (!hb_set_has (s, max_set_elements - 1));
  g_assert (!hb_set_is_empty (s));
  g_assert_cmpint (hb_set_get_population (s), ==, max_set_elements - 2);
  g_assert_cmpint (hb_set_get_min (s), ==, 1);
  g_assert_cmpint (hb_set_get_max (s), ==, max_set_elements - 2);

  hb_set_destroy (s);
}

static void
test_set_inverted_ranges (void)
{
  // Tests:
  // add_range, del_range, has, get_population, is_empty, get_min, get_max
  // for inverted sets.
  hb_set_t *s = hb_set_create ();
  hb_set_invert (s);

  hb_set_del_range (s, 41, 4000);
  hb_set_add_range (s, 78, 601);

  g_assert (hb_set_has (s, 40));
  g_assert (!hb_set_has (s, 41));
  g_assert (!hb_set_has (s, 64));
  g_assert (!hb_set_has (s, 77));
  g_assert (hb_set_has (s, 78));
  g_assert (hb_set_has (s, 300));
  g_assert (hb_set_has (s, 601));
  g_assert (!hb_set_has (s, 602));
  g_assert (!hb_set_has (s, 3000));
  g_assert (!hb_set_has (s, 4000));
  g_assert (hb_set_has (s, 4001));

  g_assert (!hb_set_is_empty (s));
  g_assert_cmpint (hb_set_get_population (s), ==, max_set_elements - 3436);
  g_assert_cmpint (hb_set_get_min (s), ==, 0);
  g_assert_cmpint (hb_set_get_max (s), ==, max_set_elements - 1);

  hb_set_del_range (s, 0, 37);
  g_assert (!hb_set_has (s, 0));
  g_assert (!hb_set_has (s, 37));
  g_assert (hb_set_has (s, 38));
  g_assert (!hb_set_is_empty (s));
  g_assert_cmpint (hb_set_get_population (s), ==,
                   max_set_elements - 3436 - 38);
  g_assert_cmpint (hb_set_get_min (s), ==, 38);
  g_assert_cmpint (hb_set_get_max (s), ==, max_set_elements - 1);

  hb_set_del_range (s, max_set_elements - 13, max_set_elements - 1);
  g_assert (!hb_set_has (s, max_set_elements - 1));
  g_assert (!hb_set_has (s, max_set_elements - 13));
  g_assert (hb_set_has (s, max_set_elements - 14));

  g_assert (!hb_set_is_empty (s));
  g_assert_cmpint (hb_set_get_population (s), ==,
                   max_set_elements - 3436 - 38 - 13);
  g_assert_cmpint (hb_set_get_min (s), ==, 38);
  g_assert_cmpint (hb_set_get_max (s), ==, max_set_elements - 14);

  hb_set_destroy (s);
}

static void
test_set_inverted_iteration_next (void)
{
  // Tests:
  // next, next_range
  hb_set_t *s = hb_set_create ();
  hb_set_invert (s);

  hb_set_del_range (s, 41, 4000);
  hb_set_add_range (s, 78, 601);

  hb_codepoint_t cp = HB_SET_VALUE_INVALID;
  hb_codepoint_t start = 0;
  hb_codepoint_t end = 0;
  g_assert (hb_set_next (s, &cp));
  g_assert_cmpint (cp, ==, 0);
  g_assert (hb_set_next (s, &cp));
  g_assert_cmpint (cp, ==, 1);

  g_assert (hb_set_next_range (s, &start, &end));
  g_assert_cmpint (start, ==, 1);
  g_assert_cmpint (end, ==, 40);

  start = 40;
  end = 40;
  g_assert (hb_set_next_range (s, &start, &end));
  g_assert_cmpint (start, ==, 78);
  g_assert_cmpint (end, ==, 601);

  start = 40;
  end = 57;
  g_assert (hb_set_next_range (s, &start, &end));
  g_assert_cmpint (start, ==, 78);
  g_assert_cmpint (end, ==, 601);

  cp = 39;
  g_assert (hb_set_next (s, &cp));
  g_assert_cmpint (cp, ==, 40);

  g_assert (hb_set_next (s, &cp));
  g_assert_cmpint (cp, ==, 78);

  cp = 56;
  g_assert (hb_set_next (s, &cp));
  g_assert_cmpint (cp, ==, 78);

  cp = 78;
  g_assert (hb_set_next (s, &cp));
  g_assert_cmpint (cp, ==, 79);

  cp = 601;
  g_assert (hb_set_next (s, &cp));
  g_assert_cmpint (cp, ==, 4001);

  cp = HB_SET_VALUE_INVALID;
  hb_set_del (s, 0);
  g_assert (hb_set_next (s, &cp));
  g_assert_cmpint (cp, ==, 1);

  start = 0;
  end = 0;
  g_assert (hb_set_next_range (s, &start, &end));
  g_assert_cmpint (start, ==, 1);
  g_assert_cmpint (end, ==, 40);

  cp = max_set_elements - 1;
  g_assert (!hb_set_next (s, &cp));
  g_assert_cmpint (cp, ==, HB_SET_VALUE_INVALID);

  start = 4000;
  end = 4000;
  g_assert (hb_set_next_range (s, &start, &end));
  g_assert_cmpint (start, ==, 4001);
  g_assert_cmpint (end, ==, max_set_elements - 1);

  start = max_set_elements - 1;
  end = max_set_elements - 1;
  g_assert (!hb_set_next_range (s, &start, &end));
  g_assert_cmpint (start, ==, HB_SET_VALUE_INVALID);
  g_assert_cmpint (end, ==, HB_SET_VALUE_INVALID);

  cp = max_set_elements - 3;
  hb_set_del (s, max_set_elements - 1);
  g_assert (hb_set_next (s, &cp));
  g_assert_cmpint (cp, ==, max_set_elements - 2);
  g_assert (!hb_set_next (s, &cp));
  g_assert_cmpint (cp, ==, HB_SET_VALUE_INVALID);


  start = max_set_elements - 2;
  end = max_set_elements - 2;
  g_assert (!hb_set_next_range (s, &start, &end));
  g_assert_cmpint (start, ==, HB_SET_VALUE_INVALID);
  g_assert_cmpint (end, ==, HB_SET_VALUE_INVALID);

  start = max_set_elements - 3;
  end = max_set_elements - 3;
  g_assert (hb_set_next_range (s, &start, &end));
  g_assert_cmpint (start, ==, max_set_elements - 2);
  g_assert_cmpint (end, ==, max_set_elements - 2);

  hb_set_destroy (s);
}

static void
test_set_inverted_iteration_prev (void)
{
  // Tests:
  // previous, previous_range
  hb_set_t *s = hb_set_create ();
  hb_set_invert (s);

  hb_set_del_range (s, 41, 4000);
  hb_set_add_range (s, 78, 601);

  hb_codepoint_t cp = HB_SET_VALUE_INVALID;
  hb_codepoint_t start = max_set_elements - 1;
  hb_codepoint_t end = max_set_elements - 1;
  g_assert (hb_set_previous (s, &cp));
  g_assert_cmpint (cp, ==, max_set_elements - 1);
  g_assert (hb_set_previous (s, &cp));
  g_assert_cmpint (cp, ==, max_set_elements - 2);

  g_assert (hb_set_previous_range (s, &start, &end));
  g_assert_cmpint (start, ==, 4001);
  g_assert_cmpint (end, ==, max_set_elements - 2);

  start = 4001;
  end = 4001;
  g_assert (hb_set_previous_range (s, &start, &end));
  g_assert_cmpint (start, ==, 78);
  g_assert_cmpint (end, ==, 601);

  start = 2500;
  end = 3000;
  g_assert (hb_set_previous_range (s, &start, &end));
  g_assert_cmpint (start, ==, 78);
  g_assert_cmpint (end, ==, 601);

  cp = 4002;
  g_assert (hb_set_previous (s, &cp));
  g_assert_cmpint (cp, ==, 4001);

  g_assert (hb_set_previous (s, &cp));
  g_assert_cmpint (cp, ==, 601);

  cp = 3500;
  g_assert (hb_set_previous (s, &cp));
  g_assert_cmpint (cp, ==, 601);

  cp = 601;
  g_assert (hb_set_previous (s, &cp));
  g_assert_cmpint (cp, ==, 600);

  cp = 78;
  g_assert (hb_set_previous (s, &cp));
  g_assert_cmpint (cp, ==, 40);

  cp = HB_SET_VALUE_INVALID;
  hb_set_del (s, max_set_elements - 1);
  g_assert (hb_set_previous (s, &cp));
  g_assert_cmpint (cp, ==, max_set_elements - 2);

  start = max_set_elements - 1;
  end = max_set_elements - 1;
  g_assert (hb_set_previous_range (s, &start, &end));
  g_assert_cmpint (start, ==, 4001);
  g_assert_cmpint (end, ==, max_set_elements - 2);

  cp = 0;
  g_assert (!hb_set_previous (s, &cp));
  g_assert_cmpint (cp, ==, HB_SET_VALUE_INVALID);

  cp = 40;
  g_assert (hb_set_previous (s, &cp));
  g_assert_cmpint (cp, ==, 39);

  start = 40;
  end = 40;
  g_assert (hb_set_previous_range (s, &start, &end));
  g_assert_cmpint (start, ==, 0);
  g_assert_cmpint (end, ==, 39);

  start = 0;
  end = 0;
  g_assert (!hb_set_previous_range (s, &start, &end));
  g_assert_cmpint (start, ==, HB_SET_VALUE_INVALID);
  g_assert_cmpint (end, ==, HB_SET_VALUE_INVALID);


  cp = 2;
  hb_set_del (s, 0);
  g_assert (hb_set_previous (s, &cp));
  g_assert_cmpint (cp, ==, 1);
  g_assert (!hb_set_previous (s, &cp));
  g_assert_cmpint (cp, ==, HB_SET_VALUE_INVALID);

  start = 1;
  end = 1;
  g_assert (!hb_set_previous_range (s, &start, &end));
  g_assert_cmpint (start, ==, HB_SET_VALUE_INVALID);
  g_assert_cmpint (end, ==, HB_SET_VALUE_INVALID);

  start = 2;
  end = 2;
  g_assert (hb_set_previous_range (s, &start, &end));
  g_assert_cmpint (start, ==, 1);
  g_assert_cmpint (end, ==, 1);

  hb_set_destroy (s);
}


static void
test_set_inverted_equality (void)
{
  hb_set_t *a = hb_set_create ();
  hb_set_t *b = hb_set_create ();
  hb_set_invert (a);
  hb_set_invert (b);

  g_assert (hb_set_is_equal (a, b));
  g_assert (hb_set_is_equal (b, a));

  hb_set_add (a, 10);
  g_assert (hb_set_is_equal (a, b));
  g_assert (hb_set_is_equal (b, a));

  hb_set_del (a, 42);
  g_assert (!hb_set_is_equal (a, b));
  g_assert (!hb_set_is_equal (b, a));

  hb_set_del (b, 42);
  g_assert (hb_set_is_equal (a, b));
  g_assert (hb_set_is_equal (b, a));

  hb_set_del_range (a, 43, 50);
  hb_set_del_range (a, 51, 76);
  hb_set_del_range (b, 43, 76);
  g_assert (hb_set_is_equal (a, b));
  g_assert (hb_set_is_equal (b, a));

  hb_set_del (a, 0);
  g_assert (!hb_set_is_equal (a, b));
  g_assert (!hb_set_is_equal (b, a));

  hb_set_del (b, 0);
  g_assert (hb_set_is_equal (a, b));
  g_assert (hb_set_is_equal (b, a));

  hb_set_del (a, max_set_elements - 1);
  g_assert (!hb_set_is_equal (a, b));
  g_assert (!hb_set_is_equal (b, a));

  hb_set_del (b, max_set_elements - 1);
  g_assert (hb_set_is_equal (a, b));
  g_assert (hb_set_is_equal (b, a));

  hb_set_invert (a);
  g_assert (!hb_set_is_equal (a, b));
  g_assert (!hb_set_is_equal (b, a));

  hb_set_invert (b);
  g_assert (hb_set_is_equal (a, b));
  g_assert (hb_set_is_equal (b, a));

  hb_set_destroy (a);
  hb_set_destroy (b);
}

typedef enum {
  UNION = 0,
  INTERSECT,
  SUBTRACT,
  SYM_DIFF,
  LAST,
} set_operation;

static hb_set_t* prepare_set(hb_bool_t has_x,
                             hb_bool_t inverted,
                             hb_bool_t has_page,
                             hb_bool_t is_null)
{
  static const hb_codepoint_t x = 13;
  if (is_null)
    return hb_set_get_empty ();

  hb_set_t* s = hb_set_create ();
  if (inverted) hb_set_invert (s);
  if (has_page)
  {
    // Ensure a page exists for x.
    inverted ? hb_set_del (s, x) : hb_set_add (s, x);
  }
  if (has_x)
    hb_set_add (s, x);
  else
    hb_set_del (s, x);

  return s;
}

static hb_bool_t
check_set_operations(hb_bool_t a_has_x,
                     hb_bool_t a_inverted,
                     hb_bool_t a_has_page,
                     hb_bool_t a_is_null,
                     hb_bool_t b_has_x,
                     hb_bool_t b_inverted,
                     hb_bool_t b_has_page,
                     hb_bool_t b_is_null,
                     set_operation op)
{
  hb_codepoint_t x = 13;
  hb_set_t* a = prepare_set (a_has_x, a_inverted, a_has_page, a_is_null);
  hb_set_t* b = prepare_set (b_has_x, b_inverted, b_has_page, b_is_null);

  const char* op_name;
  hb_bool_t has_expected;
  hb_bool_t should_have_x;
  switch (op) {
  default:
  case LAST:
  case UNION:
    op_name = "union";
    should_have_x = (a_has_x || b_has_x) && !a_is_null;
    hb_set_union (a, b);
    has_expected = (hb_set_has (a, x) == should_have_x);
    break;
  case INTERSECT:
    op_name = "intersect";
    should_have_x = (a_has_x && b_has_x) && !a_is_null;
    hb_set_intersect (a, b);
    has_expected = (hb_set_has (a, x) == should_have_x);
    break;
  case SUBTRACT:
    op_name = "subtract";
    should_have_x = (a_has_x && !b_has_x) && !a_is_null;
    hb_set_subtract (a, b);
    has_expected = (hb_set_has (a, x) == should_have_x);
    break;
  case SYM_DIFF:
    op_name = "sym_diff";
    should_have_x = (a_has_x ^ b_has_x) && !a_is_null;
    hb_set_symmetric_difference (a, b);
    has_expected = (hb_set_has (a, x) == should_have_x);
    break;
  }

  printf ("%s%s%s%s %-9s %s%s%s%s == %s  [%s]\n",
          a_inverted ? "i" : " ",
          a_has_page ? "p" : " ",
          a_is_null ? "n" : " ",
          a_has_x ? "{13}" : "{}  ",
          op_name,
          b_inverted ? "i" : " ",
          b_has_page ? "p" : " ",
          b_is_null ? "n" : " ",
          b_has_x ? "{13}" : "{}  ",
          should_have_x ? "{13}" : "{}  ",
          has_expected ? "succeeded" : "failed");

  hb_set_destroy (a);
  hb_set_destroy (b);

  return has_expected;
}

static void
test_set_inverted_operations (void)
{
  hb_bool_t all_succeeded = 1;
  for (hb_bool_t a_has_x = 0; a_has_x <= 1; a_has_x++) {
    for (hb_bool_t a_inverted = 0; a_inverted <= 1; a_inverted++) {
      for (hb_bool_t b_has_x = 0; b_has_x <= 1; b_has_x++) {
        for (hb_bool_t b_inverted = 0; b_inverted <= 1; b_inverted++) {
          for (hb_bool_t a_has_page = 0; a_has_page <= !(a_has_x ^ a_inverted); a_has_page++) {
            for (hb_bool_t b_has_page = 0; b_has_page <= !(b_has_x ^ b_inverted); b_has_page++) {
              for (hb_bool_t a_is_null = 0; a_is_null <= (!a_has_x && !a_has_page && !a_inverted); a_is_null++) {
                for (hb_bool_t b_is_null = 0; b_is_null <= (!b_has_x && !b_has_page && !b_inverted); b_is_null++) {
                  for (set_operation op = UNION; op < LAST; op++) {
                    all_succeeded = check_set_operations (a_has_x, a_inverted, a_has_page, a_is_null,
                                                          b_has_x, b_inverted, b_has_page, b_is_null,
                                                          op)
                                    && all_succeeded;
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  g_assert (all_succeeded);
}

static void
test_hb_set_add_sorted_array (void)
{
  hb_set_t *set = hb_set_create ();
  hb_codepoint_t array[7] = {1, 2, 3, 1000, 2000, 2001, 2002};
  hb_set_add_sorted_array (set, array, 7);
  g_assert_cmpint (hb_set_get_population (set), ==, 7);
  g_assert (hb_set_has (set, 1));
  g_assert (hb_set_has (set, 2));
  g_assert (hb_set_has (set, 3));
  g_assert (hb_set_has (set, 1000));
  g_assert (hb_set_has (set, 2000));
  g_assert (hb_set_has (set, 2001));
  g_assert (hb_set_has (set, 2002));
  hb_set_destroy (set);
}

static void
test_set_next_many (void)
{
  hb_set_t *set = hb_set_create ();
  for (unsigned i=0; i<600; i++)
    hb_set_add (set, i);
  for (unsigned i=6000; i<6100; i++)
    hb_set_add (set, i);
  g_assert (hb_set_get_population (set) == 700);
  hb_codepoint_t array[700];

  unsigned int n = hb_set_next_many (set, HB_SET_VALUE_INVALID, array, 700);

  g_assert_cmpint(n, ==, 700);
  for (unsigned i=0; i<600; i++)
    g_assert_cmpint (array[i], ==, i);
  for (unsigned i=0; i<100; i++)
    g_assert (array[600 + i] == 6000u + i);

  // Try skipping initial values.
  for (unsigned i = 0; i < 700; i++)
    array[i] = 0;

  n = hb_set_next_many (set, 42, array, 700);

  g_assert_cmpint (n, ==, 657);
  g_assert_cmpint (array[0], ==, 43);
  g_assert_cmpint (array[n - 1], ==, 6099);

  hb_set_destroy (set);
}

static void
test_set_next_many_restricted (void)
{
  hb_set_t *set = hb_set_create ();
  for (int i=0; i<600; i++)
    hb_set_add (set, i);
  for (int i=6000; i<6100; i++)
    hb_set_add (set, i);
  g_assert (hb_set_get_population (set) == 700);
  hb_codepoint_t array[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  hb_set_next_many (set, HB_SET_VALUE_INVALID, array, 9);

  for (int i=0; i<9; i++)
    g_assert_cmpint (array[i], ==, i);
  g_assert_cmpint (array[9], ==, 0);
  hb_set_destroy (set);
}

static void
test_set_next_many_inverted (void)
{
  hb_set_t *set = hb_set_create ();
  hb_set_add (set, 1);
  hb_set_add (set, 3);
  hb_set_invert (set);

  hb_codepoint_t array[] = {0, 0, 0, 0, 0, 999};

  // Single page.
  hb_set_next_many (set, HB_SET_VALUE_INVALID, array, 5);

  g_assert_cmpint (array[0], ==, 0);
  g_assert_cmpint (array[1], ==, 2);
  g_assert_cmpint (array[2], ==, 4);
  g_assert_cmpint (array[3], ==, 5);
  g_assert_cmpint (array[4], ==, 6);
  g_assert_cmpint (array[5], ==, 999);

  // Multiple pages.
  hb_set_invert (set);
  hb_set_add (set, 1000);
  hb_set_invert (set);

  hb_codepoint_t array2[1000];
  hb_set_next_many (set, HB_SET_VALUE_INVALID, array2, 1000);
  g_assert_cmpint (array2[0], ==, 0);
  g_assert_cmpint (array2[1], ==, 2);
  g_assert_cmpint (array2[2], ==, 4);
  g_assert_cmpint (array2[3], ==, 5);
  for (int i=4; i<997; i++)
  {
    g_assert_cmpint (array2[i], ==, i + 2);
  }
  g_assert_cmpint (array2[997], ==, 999);
  // Value 1000 skipped.
  g_assert_cmpint (array2[998], ==, 1001);
  g_assert_cmpint (array2[999], ==, 1002);

  hb_set_destroy (set);
}

static void
test_set_next_many_out_of_order_pages (void) {
  hb_set_t* set = hb_set_create();
  hb_set_add(set, 1957);
  hb_set_add(set, 69);
  hb_codepoint_t results[2];
  unsigned int result_size = hb_set_next_many(set, HB_SET_VALUE_INVALID, results, 2);
  g_assert_cmpint(result_size, == , 2);
  g_assert_cmpint(results[0], == , 69);
  g_assert_cmpint(results[1], == , 1957);
  hb_set_destroy(set);
}

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  hb_test_add (test_set_basic);
  hb_test_add (test_set_subsets);
  hb_test_add (test_set_algebra);
  hb_test_add (test_set_iter);
  hb_test_add (test_set_empty);
  hb_test_add (test_set_delrange);

  hb_test_add (test_set_intersect_empty);
  hb_test_add (test_set_intersect_page_reduction);
  hb_test_add (test_set_union);

  hb_test_add (test_set_inverted_basics);
  hb_test_add (test_set_inverted_ranges);
  hb_test_add (test_set_inverted_iteration_next);
  hb_test_add (test_set_inverted_iteration_prev);
  hb_test_add (test_set_inverted_equality);
  hb_test_add (test_set_inverted_operations);

  hb_test_add (test_hb_set_add_sorted_array);
  hb_test_add (test_set_next_many);
  hb_test_add (test_set_next_many_restricted);
  hb_test_add (test_set_next_many_inverted);
  hb_test_add (test_set_next_many_out_of_order_pages);

  return hb_test_run();
}
