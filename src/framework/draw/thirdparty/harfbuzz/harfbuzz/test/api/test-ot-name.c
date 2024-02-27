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
 *
 */

#include "hb-test.h"

#include <hb-ot.h>

static hb_face_t *face;

static void
test_ot_layout_feature_get_name_ids_and_characters (void)
{
  hb_tag_t cv01 = HB_TAG ('c','v','0','1');
  unsigned int feature_index;
  unsigned int num_named_parameters;
  hb_ot_name_id_t label_id;
  hb_ot_name_id_t tooltip_id;
  hb_ot_name_id_t sample_id;
  hb_ot_name_id_t first_param_id;
  hb_codepoint_t characters[100];
  unsigned int char_count = 100;
  unsigned int all_chars;
  if (!hb_ot_layout_language_find_feature (face,
					   HB_OT_TAG_GSUB,
					   0,
					   HB_OT_LAYOUT_DEFAULT_LANGUAGE_INDEX,
					   cv01,
					   &feature_index))
     g_error ("Failed to find feature index");

  if (!hb_ot_layout_feature_get_name_ids (face, HB_OT_TAG_GSUB, feature_index,
					  &label_id, &tooltip_id, &sample_id,
					  &num_named_parameters, &first_param_id))
    g_error ("Failed to get name ids");

  g_assert_cmpint (label_id, ==, 256);
  g_assert_cmpint (tooltip_id, ==, 257);
  g_assert_cmpint (sample_id, ==, 258);
  g_assert_cmpint (num_named_parameters, ==, 2);
  g_assert_cmpint (first_param_id, ==, 259);

  all_chars = hb_ot_layout_feature_get_characters (face, HB_OT_TAG_GSUB, feature_index,
						   0, &char_count, characters);

  g_assert_cmpint (all_chars, ==, 2);
  g_assert_cmpint (char_count, ==, 2);
  g_assert_cmpint (characters[0], ==, 10);
  g_assert_cmpint (characters[1], ==, 24030);

  char_count = 100;
  characters[1] = 1234;
  all_chars = hb_ot_layout_feature_get_characters (face, HB_OT_TAG_GSUB, feature_index,
						   1, &char_count, characters);
  g_assert_cmpint (all_chars, ==, 2);
  g_assert_cmpint (char_count, ==, 1);
  g_assert_cmpint (characters[0], ==, 24030);
  g_assert_cmpint (characters[1], ==, 1234);
}

static void
test_ot_name (void)
{
  unsigned int num_entries;
  const hb_ot_name_entry_t *entries;
  hb_ot_name_id_t name_id;
  hb_language_t lang;
  char text[10];
  unsigned int text_size = 10;
  entries = hb_ot_name_list_names (face, &num_entries);
  g_assert_cmpuint (12, ==, num_entries);
  name_id = entries[3].name_id;
  g_assert_cmpuint (3, ==, name_id);
  lang = entries[3].language;

  g_assert_cmpstr (hb_language_to_string (lang), ==, "en");
  g_assert_cmpuint (27, ==, hb_ot_name_get_utf8 (face, name_id, lang, &text_size, text));
  g_assert_cmpuint (9, ==, text_size);
  g_assert_cmpstr (text, ==, "FontForge");

  g_assert_cmpuint (27, ==, hb_ot_name_get_utf8 (face, name_id, hb_language_from_string ("en_US", -1), &text_size, text));
  g_assert_cmpuint (8, ==, text_size);
  g_assert_cmpstr (text, ==, "FontForg");

  g_assert_cmpuint (0, ==, hb_ot_name_get_utf8 (face, name_id, hb_language_from_string ("fa_IR", -1), &text_size, text));
}

int
main (int argc, char **argv)
{
  unsigned int status;
  g_test_init (&argc, &argv, NULL);

  hb_test_add (test_ot_layout_feature_get_name_ids_and_characters);
  hb_test_add (test_ot_name);

  face = hb_test_open_font_file ("fonts/cv01.otf");
  status = hb_test_run ();
  hb_face_destroy (face);
  return status;
}
