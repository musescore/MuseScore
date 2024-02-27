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

/* Unit tests for hb-subset-glyf.h */

static void
test_subset_32_tables (void)
{
  hb_face_t *face = hb_test_open_font_file ("../fuzzing/fonts/oom-6ef8c96d3710262511bcc730dce9c00e722cb653");

  hb_subset_input_t *input = hb_subset_input_create_or_fail ();
  hb_set_t *codepoints = hb_subset_input_unicode_set (input);
  hb_face_t *subset;

  hb_set_add (codepoints, 'a');
  hb_set_add (codepoints, 'b');
  hb_set_add (codepoints, 'c');

  subset = hb_subset_or_fail (face, input);
  g_assert (subset);
  g_assert (subset != hb_face_get_empty ());

  hb_subset_input_destroy (input);
  hb_face_destroy (subset);
  hb_face_destroy (face);
}

static void
test_subset_no_inf_loop (void)
{
  hb_face_t *face = hb_test_open_font_file ("../fuzzing/fonts/clusterfuzz-testcase-minimized-hb-subset-fuzzer-5521982557782016");

  hb_subset_input_t *input = hb_subset_input_create_or_fail ();
  hb_set_t *codepoints = hb_subset_input_unicode_set (input);
  hb_face_t *subset;

  hb_set_add (codepoints, 'a');
  hb_set_add (codepoints, 'b');
  hb_set_add (codepoints, 'c');

  subset = hb_subset_or_fail (face, input);
  g_assert (!subset);

  hb_subset_input_destroy (input);
  hb_face_destroy (subset);
  hb_face_destroy (face);
}

static void
test_subset_crash (void)
{
  hb_face_t *face = hb_test_open_font_file ("../fuzzing/fonts/crash-4b60576767ee4d9fe1cc10959d89baf73d4e8249");

  hb_subset_input_t *input = hb_subset_input_create_or_fail ();
  hb_set_t *codepoints = hb_subset_input_unicode_set (input);
  hb_face_t *subset;

  hb_set_add (codepoints, 'a');
  hb_set_add (codepoints, 'b');
  hb_set_add (codepoints, 'c');

  subset = hb_subset_or_fail (face, input);
  g_assert (!subset);

  hb_subset_input_destroy (input);
  hb_face_destroy (subset);
  hb_face_destroy (face);
}

static void
test_subset_set_flags (void)
{
  hb_subset_input_t *input = hb_subset_input_create_or_fail ();

  g_assert (hb_subset_input_get_flags (input) == HB_SUBSET_FLAGS_DEFAULT);

  hb_subset_input_set_flags (input,
                             HB_SUBSET_FLAGS_NAME_LEGACY |
                             HB_SUBSET_FLAGS_NOTDEF_OUTLINE |
                             HB_SUBSET_FLAGS_GLYPH_NAMES);

  g_assert (hb_subset_input_get_flags (input) ==
            (hb_subset_flags_t) (
            HB_SUBSET_FLAGS_NAME_LEGACY |
            HB_SUBSET_FLAGS_NOTDEF_OUTLINE |
            HB_SUBSET_FLAGS_GLYPH_NAMES));

  hb_subset_input_set_flags (input,
                             HB_SUBSET_FLAGS_NAME_LEGACY |
                             HB_SUBSET_FLAGS_NOTDEF_OUTLINE |
                             HB_SUBSET_FLAGS_NO_PRUNE_UNICODE_RANGES);

  g_assert (hb_subset_input_get_flags (input) ==
            (hb_subset_flags_t) (
            HB_SUBSET_FLAGS_NAME_LEGACY |
            HB_SUBSET_FLAGS_NOTDEF_OUTLINE |
            HB_SUBSET_FLAGS_NO_PRUNE_UNICODE_RANGES));


  hb_subset_input_destroy (input);
}


static void
test_subset_sets (void)
{
  hb_subset_input_t *input = hb_subset_input_create_or_fail ();
  hb_set_t* set = hb_set_create ();

  hb_set_add (hb_subset_input_set (input, HB_SUBSET_SETS_GLYPH_INDEX), 83);
  hb_set_add (hb_subset_input_set (input, HB_SUBSET_SETS_UNICODE), 85);

  hb_set_clear (hb_subset_input_set (input, HB_SUBSET_SETS_LAYOUT_FEATURE_TAG));
  hb_set_add (hb_subset_input_set (input, HB_SUBSET_SETS_LAYOUT_FEATURE_TAG), 87);

  hb_set_add (set, 83);
  g_assert (hb_set_is_equal (hb_subset_input_glyph_set (input), set));
  hb_set_clear (set);

  hb_set_add (set, 85);
  g_assert (hb_set_is_equal (hb_subset_input_unicode_set (input), set));
  hb_set_clear (set);

  hb_set_add (set, 87);
  g_assert (hb_set_is_equal (hb_subset_input_set (input, HB_SUBSET_SETS_LAYOUT_FEATURE_TAG), set));
  hb_set_clear (set);

  hb_set_destroy (set);
  hb_subset_input_destroy (input);
}

static void
test_subset_plan (void)
{
  hb_face_t *face_abc = hb_test_open_font_file ("fonts/Roboto-Regular.abc.ttf");
  hb_face_t *face_ac = hb_test_open_font_file ("fonts/Roboto-Regular.ac.ttf");

  hb_set_t *codepoints = hb_set_create();
  hb_set_add (codepoints, 97);
  hb_set_add (codepoints, 99);
  hb_subset_input_t* input = hb_subset_test_create_input (codepoints);
  hb_set_destroy (codepoints);

  hb_subset_plan_t* plan = hb_subset_plan_create_or_fail (face_abc, input);
  g_assert (plan);

  const hb_map_t* mapping = hb_subset_plan_old_to_new_glyph_mapping (plan);
  g_assert (hb_map_get (mapping, 1) == 1);
  g_assert (hb_map_get (mapping, 3) == 2);

  mapping = hb_subset_plan_new_to_old_glyph_mapping (plan);
  g_assert (hb_map_get (mapping, 1) == 1);
  g_assert (hb_map_get (mapping, 2) == 3);

  mapping = hb_subset_plan_unicode_to_old_glyph_mapping (plan);
  g_assert (hb_map_get (mapping, 0x63) == 3);

  hb_face_t* face_abc_subset = hb_subset_plan_execute_or_fail (plan);

  hb_subset_test_check (face_ac, face_abc_subset, HB_TAG ('l','o','c', 'a'));
  hb_subset_test_check (face_ac, face_abc_subset, HB_TAG ('g','l','y','f'));

  hb_subset_input_destroy (input);
  hb_subset_plan_destroy (plan);
  hb_face_destroy (face_abc_subset);
  hb_face_destroy (face_abc);
  hb_face_destroy (face_ac);
}

static hb_blob_t*
_ref_table (hb_face_t *face, hb_tag_t tag, void *user_data)
{
  return hb_face_reference_table ((hb_face_t*) user_data, tag);
}

static void
test_subset_create_for_tables_face (void)
{
  hb_face_t *face_abc = hb_test_open_font_file ("fonts/Roboto-Regular.abc.ttf");
  hb_face_t *face_ac = hb_test_open_font_file ("fonts/Roboto-Regular.ac.ttf");
  hb_face_t *face_create_for_tables = hb_face_create_for_tables (
      _ref_table,
      face_abc,
      NULL);

  hb_set_t *codepoints = hb_set_create();
  hb_set_add (codepoints, 97);
  hb_set_add (codepoints, 99);

  hb_subset_input_t* input = hb_subset_test_create_input (codepoints);
  hb_set_destroy (codepoints);

  hb_face_t* face_abc_subset = hb_subset_or_fail (face_create_for_tables, input);

  hb_subset_test_check (face_ac, face_abc_subset, HB_TAG ('l','o','c', 'a'));
  hb_subset_test_check (face_ac, face_abc_subset, HB_TAG ('g','l','y','f'));
  hb_subset_test_check (face_ac, face_abc_subset, HB_TAG ('g','a','s','p'));

  hb_subset_input_destroy (input);
  hb_face_destroy (face_abc_subset);
  hb_face_destroy (face_create_for_tables);
  hb_face_destroy (face_abc);
  hb_face_destroy (face_ac);
}

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  hb_test_add (test_subset_32_tables);
  hb_test_add (test_subset_no_inf_loop);
  hb_test_add (test_subset_crash);
  hb_test_add (test_subset_set_flags);
  hb_test_add (test_subset_sets);
  hb_test_add (test_subset_plan);
  hb_test_add (test_subset_create_for_tables_face);

  return hb_test_run();
}
