/*
 * Copyright © 2018 Adobe Inc.
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
 * Adobe Author(s): Michiharu Ariza
 */

#include "hb-test.h"
#include <hb-ot.h>

/* Unit tests for CFF/CFF2 glyph extents */

static void
test_extents_cff1 (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/SourceSansPro-Regular.abc.otf");
  g_assert (face);
  hb_font_t *font = hb_font_create (face);
  hb_face_destroy (face);
  g_assert (font);
  hb_ot_font_set_funcs (font);

  hb_glyph_extents_t  extents;
  hb_bool_t result = hb_font_get_glyph_extents (font, 1, &extents);
  g_assert (result);

  g_assert_cmpint (extents.x_bearing, ==, 52);
  g_assert_cmpint (extents.y_bearing, ==, 498);
  g_assert_cmpint (extents.width, ==, 381);
  g_assert_cmpint (extents.height, ==, -510);

  hb_font_destroy (font);

  hb_face_t *face_j = hb_test_open_font_file ("fonts/SourceHanSans-Regular.41,3041,4C2E.otf");
  g_assert (face_j);
  hb_font_t *font_j = hb_font_create (face_j);
  hb_face_destroy (face_j);
  g_assert (font_j);
  hb_ot_font_set_funcs (font_j);

  hb_bool_t result_j = hb_font_get_glyph_extents (font_j, 3, &extents);
  g_assert (result_j);

  g_assert_cmpint (extents.x_bearing, ==, 34);
  g_assert_cmpint (extents.y_bearing, ==, 840);
  g_assert_cmpint (extents.width, ==, 920);
  g_assert_cmpint (extents.height, ==, -907);

  hb_font_destroy (font_j);
}

static void
test_extents_cff1_flex (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/cff1_flex.otf");
  g_assert (face);
  hb_font_t *font = hb_font_create (face);
  hb_face_destroy (face);
  g_assert (font);
  hb_ot_font_set_funcs (font);

  hb_glyph_extents_t  extents;
  hb_bool_t result = hb_font_get_glyph_extents (font, 1, &extents);
  g_assert (result);

  g_assert_cmpint (extents.x_bearing, ==, -20);
  g_assert_cmpint (extents.y_bearing, ==, 520);
  g_assert_cmpint (extents.width, ==, 540);
  g_assert_cmpint (extents.height, ==, -540);

  hb_font_destroy (font);
}

static void
test_extents_cff1_seac (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/cff1_seac.otf");
  g_assert (face);
  hb_font_t *font = hb_font_create (face);
  hb_face_destroy (face);
  g_assert (font);
  hb_ot_font_set_funcs (font);

  hb_glyph_extents_t  extents;
  hb_bool_t result = hb_font_get_glyph_extents (font, 3, &extents); /* Agrave */
  g_assert (result);

  g_assert_cmpint (extents.x_bearing, ==, 3);
  g_assert_cmpint (extents.y_bearing, ==, 861);
  g_assert_cmpint (extents.width, ==, 538);
  g_assert_cmpint (extents.height, ==, -861);

  result = hb_font_get_glyph_extents (font, 4, &extents); /* Udieresis */
  g_assert (result);

  g_assert_cmpint (extents.x_bearing, ==, 87);
  g_assert_cmpint (extents.y_bearing, ==, 827);
  g_assert_cmpint (extents.width, ==, 471);
  g_assert_cmpint (extents.height, ==, -839);

  hb_font_destroy (font);
}

static void
test_extents_cff2 (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/AdobeVFPrototype.abc.otf");
  g_assert (face);
  hb_font_t *font = hb_font_create (face);
  hb_face_destroy (face);
  g_assert (font);
  hb_ot_font_set_funcs (font);

  hb_glyph_extents_t  extents;
  hb_bool_t result = hb_font_get_glyph_extents (font, 1, &extents);
  g_assert (result);

  g_assert_cmpint (extents.x_bearing, ==, 46);
  g_assert_cmpint (extents.y_bearing, ==, 487);
  g_assert_cmpint (extents.width, ==, 455);
  g_assert_cmpint (extents.height, ==, -500);

  float coords[2] = { 600.0f, 50.0f };
  hb_font_set_var_coords_design (font, coords, 2);
  result = hb_font_get_glyph_extents (font, 1, &extents);
  g_assert (result);

  g_assert_cmpint (extents.x_bearing, ==, 38);
  g_assert_cmpint (extents.y_bearing, ==, 493);
  g_assert_cmpint (extents.width, ==, 480);
  g_assert_cmpint (extents.height, ==, -507);

  hb_font_destroy (font);
}

static void
test_extents_cff2_vsindex (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/AdobeVFPrototype_vsindex.otf");
  g_assert (face);
  hb_font_t *font = hb_font_create (face);
  hb_face_destroy (face);
  g_assert (font);
  hb_ot_font_set_funcs (font);

  hb_glyph_extents_t  extents;
  float coords[2] = { 800.0f, 50.0f };
  hb_font_set_var_coords_design (font, coords, 2);
  hb_bool_t result = hb_font_get_glyph_extents (font, 1, &extents);
  g_assert (result);

  g_assert_cmpint (extents.x_bearing, ==, 12);
  g_assert_cmpint (extents.y_bearing, ==, 655);
  g_assert_cmpint (extents.width, ==, 651);
  g_assert_cmpint (extents.height, ==, -655);

  result = hb_font_get_glyph_extents (font, 2, &extents);
  g_assert (result);

  g_assert_cmpint (extents.x_bearing, ==, 8);
  g_assert_cmpint (extents.y_bearing, ==, 669);
  g_assert_cmpint (extents.width, ==, 648);
  g_assert_cmpint (extents.height, ==, -669);

  hb_font_destroy (font);
}

static void
test_extents_cff2_vsindex_named_instance (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/AdobeVFPrototype_vsindex.otf");
  g_assert (face);
  hb_font_t *font = hb_font_create (face);
  hb_face_destroy (face);
  g_assert (font);
  hb_ot_font_set_funcs (font);

  hb_font_set_var_named_instance (font, 6); // 6 (BlackMediumContrast): 900, 50
  hb_glyph_extents_t  extents;
  hb_bool_t result = hb_font_get_glyph_extents (font, 1, &extents);
  g_assert (result);

  g_assert_cmpint (extents.x_bearing, ==, 13);
  g_assert_cmpint (extents.y_bearing, ==, 652);
  g_assert_cmpint (extents.width, ==, 652);
  g_assert_cmpint (extents.height, ==, -652);

  result = hb_font_get_glyph_extents (font, 2, &extents);
  g_assert (result);

  g_assert_cmpint (extents.x_bearing, ==, 6);
  g_assert_cmpint (extents.y_bearing, ==, 675);
  g_assert_cmpint (extents.width, ==, 647);
  g_assert_cmpint (extents.height, ==, -675);

  hb_font_destroy (font);
}

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  hb_test_add (test_extents_cff1);
  hb_test_add (test_extents_cff1_flex);
  hb_test_add (test_extents_cff1_seac);
  hb_test_add (test_extents_cff2);
  hb_test_add (test_extents_cff2_vsindex);
  hb_test_add (test_extents_cff2_vsindex_named_instance);

  return hb_test_run ();
}
