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

static void
test_ot_layout_get_ligature_carets_ot_gdef (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/NotoNastaliqUrdu-Regular.ttf");
  hb_font_t *font = hb_font_create (face);
  hb_font_set_scale (font, hb_face_get_upem (face) * 2, hb_face_get_upem (face) * 4);

  hb_position_t caret_array[16];

  /* call with no result */
  {
    unsigned caret_count = 16;
    g_assert_cmpuint (0, ==, hb_ot_layout_get_ligature_carets (font, HB_DIRECTION_LTR,
							       188, 0, &caret_count,
							       caret_array));

    g_assert_cmpuint (0, ==, caret_count);
  }

  /* call with no result and some offset */
  {
    unsigned caret_count = 16;
    g_assert_cmpuint (0, ==, hb_ot_layout_get_ligature_carets (font, HB_DIRECTION_LTR,
							       188, 10, &caret_count,
							       caret_array));

    g_assert_cmpuint (0, ==, caret_count);
  }

  /* a glyph with 3 ligature carets */
  {
    unsigned caret_count = 16;
    g_assert_cmpuint (3, ==, hb_ot_layout_get_ligature_carets (font, HB_DIRECTION_LTR,
							       1020, 0, &caret_count,
							       caret_array));

    g_assert_cmpuint (3, ==, caret_count);
    g_assert_cmpuint (2718, ==, caret_array[0]);
    g_assert_cmpuint (5438, ==, caret_array[1]);
    g_assert_cmpuint (8156, ==, caret_array[2]);
  }

  /* the same glyph as above but with offset */
  {
    caret_array[2] = 123;

    unsigned caret_count = 16;
    g_assert_cmpuint (3, ==, hb_ot_layout_get_ligature_carets (font, HB_DIRECTION_LTR,
							       1020, 1, &caret_count,
							       caret_array));

    g_assert_cmpuint (2, ==, caret_count);
    g_assert_cmpuint (5438, ==, caret_array[0]);
    g_assert_cmpuint (8156, ==, caret_array[1]);

    g_assert_cmpuint (123, ==, caret_array[2]);
  }

  /* the same glyph as above but with another offset */
  {
    unsigned caret_count = 16;
    g_assert_cmpuint (3, ==, hb_ot_layout_get_ligature_carets (font, HB_DIRECTION_LTR,
							       1020, 2, &caret_count,
							       caret_array));

    g_assert_cmpuint (1, ==, caret_count);
    g_assert_cmpuint (8156, ==, caret_array[0]);
  }

  /* call with no result */
  {
    unsigned caret_count = 16;
    g_assert_cmpuint (0, ==, hb_ot_layout_get_ligature_carets (font, HB_DIRECTION_LTR,
							       1021, 0, &caret_count,
							       caret_array));

    g_assert_cmpuint (0, ==, caret_count);
  }

  /* a glyph with 1 ligature caret */
  {
    unsigned caret_count = 16;
    g_assert_cmpuint (1, ==, hb_ot_layout_get_ligature_carets (font, HB_DIRECTION_LTR,
							       1022, 0, &caret_count,
							       caret_array));

    g_assert_cmpuint (1, ==, caret_count);
    g_assert_cmpuint (3530, ==, caret_array[0]);
  }

  /* the same glyph as above but with offset */
  {
    unsigned caret_count = 16;
    g_assert_cmpuint (1, ==, hb_ot_layout_get_ligature_carets (font, HB_DIRECTION_LTR,
							       1022, 1, &caret_count,
							       caret_array));

    g_assert_cmpuint (0, ==, caret_count);
  }

  /* a glyph with 2 ligature carets */
  {
    unsigned caret_count = 16;
    g_assert_cmpuint (2, ==, hb_ot_layout_get_ligature_carets (font, HB_DIRECTION_LTR,
							       1023, 0, &caret_count,
							       caret_array));

    g_assert_cmpuint (2, ==, caret_count);
    g_assert_cmpuint (2352, ==, caret_array[0]);
    g_assert_cmpuint (4706, ==, caret_array[1]);
  }

  hb_font_destroy (font);
  hb_face_destroy (face);
}

static void
test_ot_layout_get_ligature_carets_empty (void)
{
  hb_face_t *face = hb_face_get_empty ();
  hb_font_t *font = hb_font_create (face);
  hb_font_set_scale (font, hb_face_get_upem (face) * 2, hb_face_get_upem (face) * 4);

  hb_position_t caret_array[3];
  unsigned int caret_count = 3;
  g_assert_cmpuint (0, ==, hb_ot_layout_get_ligature_carets (font, HB_DIRECTION_RTL,
							     1024, 0, &caret_count,
							     caret_array));

  g_assert_cmpuint (0, ==, caret_count);

  hb_font_destroy (font);
  hb_face_destroy (face);
}

int
main (int argc, char **argv)
{
  g_test_init (&argc, &argv, NULL);

  hb_test_add (test_ot_layout_get_ligature_carets_ot_gdef);
  hb_test_add (test_ot_layout_get_ligature_carets_empty);

  return hb_test_run ();
}
