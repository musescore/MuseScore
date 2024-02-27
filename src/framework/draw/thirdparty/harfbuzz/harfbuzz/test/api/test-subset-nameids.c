/*
 * Copyright © 2018  Google, Inc.
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
 * Google Author(s): Garret Rieger
 */

#include "hb-test.h"
#include "hb-subset-test.h"

static void
test_subset_nameids (void)
{
  hb_face_t *face_origin = hb_test_open_font_file ("fonts/nameID.origin.ttf");
  hb_face_t *face_expected = hb_test_open_font_file ("fonts/nameID.expected.ttf");

  hb_set_t *name_ids = hb_set_create();
  hb_face_t *face_subset;
  hb_set_add (name_ids, 0);
  hb_set_add (name_ids, 9);
  face_subset = hb_subset_test_create_subset (face_origin, hb_subset_test_create_input_from_nameids (name_ids));
  hb_set_destroy (name_ids);

  hb_subset_test_check (face_expected, face_subset, HB_TAG ('n','a','m','e'));

  hb_face_destroy (face_subset);
  hb_face_destroy (face_origin);
  hb_face_destroy (face_expected);
}

static void
test_subset_nameids_with_dup_strs (void)
{
  hb_face_t *face_origin = hb_test_open_font_file ("fonts/nameID.dup.origin.ttf");
  hb_face_t *face_expected = hb_test_open_font_file ("fonts/nameID.dup.expected.ttf");

  hb_set_t *name_ids = hb_set_create();
  hb_face_t *face_subset;
  hb_set_add (name_ids, 1);
  hb_set_add (name_ids, 3);
  face_subset = hb_subset_test_create_subset (face_origin, hb_subset_test_create_input_from_nameids (name_ids));
  hb_set_destroy (name_ids);

  hb_subset_test_check (face_expected, face_subset, HB_TAG ('n','a','m','e'));

  hb_face_destroy (face_subset);
  hb_face_destroy (face_origin);
  hb_face_destroy (face_expected);
}

#ifdef HB_EXPERIMENTAL_API
static void
test_subset_name_overrides (void)
{
  hb_face_t *face_origin = hb_test_open_font_file ("fonts/nameID.origin.ttf");
  hb_face_t *face_expected = hb_test_open_font_file ("fonts/nameID.override.expected.ttf");

  char str1[] = "Roboto Test";
  char str1_3[] = "Roboto Test unicode platform";
  char str2[] = "Bold";
  char str6[] = "Roboto-Bold";
  char str12[] = "Non ascii test Ü";
  char str16[] = "Roboto-test-inserting";
 
  hb_set_t *name_ids = hb_set_create();
  hb_face_t *face_subset;
  hb_set_add_range (name_ids, 0, 13);

  hb_subset_input_t *subset_input = hb_subset_test_create_input_from_nameids (name_ids);
  hb_subset_input_override_name_table (subset_input, 1, 1, 0, 0, str1, -1);
  hb_subset_input_override_name_table (subset_input, 1, 3, 1, 0x409, str1_3, -1);
  hb_subset_input_override_name_table (subset_input, 2, 1, 0, 0, str2, 4);
  hb_subset_input_override_name_table (subset_input, 6, 1, 0, 0, str6, -1);
  hb_subset_input_override_name_table (subset_input, 12, 1, 0, 0, str12, -1);
  hb_subset_input_override_name_table (subset_input, 14, 1, 0, 0, NULL, -1);
  hb_subset_input_override_name_table (subset_input, 16, 1, 0, 0, str16, -1);

  face_subset = hb_subset_test_create_subset (face_origin, subset_input);
  hb_set_destroy (name_ids);

  hb_subset_test_check (face_expected, face_subset, HB_TAG ('n','a','m','e'));

  hb_face_destroy (face_subset);
  hb_face_destroy (face_origin);
  hb_face_destroy (face_expected);
}
#endif

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  hb_test_add (test_subset_nameids);
  hb_test_add (test_subset_nameids_with_dup_strs);
#ifdef HB_EXPERIMENTAL_API
  hb_test_add (test_subset_name_overrides);
#endif

  return hb_test_run();
}
