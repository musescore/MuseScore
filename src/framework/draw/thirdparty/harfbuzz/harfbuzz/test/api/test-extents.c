/*
 * Copyright © 2023 Red Hat, Inc.
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
 * Author(s): Matthias Clasen
 */

#include "hb-test.h"


static void
test_glyph_extents (void)
{
  hb_face_t *face;
  hb_font_t *font;
  hb_glyph_extents_t extents;
  hb_bool_t ret;

  /* This font contains a COLRv1 glyph with a ClipBox,
   * and various components without. The main thing
   * we test here is that glyphs with no paint return
   * 0,0,0,0 and not meaningless numbers.
   */

  face = hb_test_open_font_file ("fonts/adwaita.ttf");
  font = hb_font_create (face);

  ret = hb_font_get_glyph_extents (font, 0, &extents);
  g_assert_true (ret);
  g_assert_true (extents.x_bearing == 0 &&
                 extents.y_bearing == 0 &&
                 extents.width     == 0 &&
                 extents.height    == 0);

  ret = hb_font_get_glyph_extents (font, 1, &extents);
  g_assert_true (ret);
  g_assert_true (extents.x_bearing == 0 &&
                 extents.y_bearing == 0 &&
                 extents.width     == 0 &&
                 extents.height    == 0);

  ret = hb_font_get_glyph_extents (font, 2, &extents);
  g_assert_true (ret);
  g_assert_true (extents.x_bearing ==   180 &&
                 extents.y_bearing ==   960 &&
                 extents.width     ==  1060 &&
                 extents.height    == -1220);

  ret = hb_font_get_glyph_extents (font, 3, &extents);
  g_assert_true (ret);
  g_assert_true (extents.x_bearing ==   188 &&
                 extents.y_bearing ==   950 &&
                 extents.width     ==   900 &&
                 extents.height    == -1200);

  ret = hb_font_get_glyph_extents (font, 4, &extents);
  g_assert_true (ret);
  g_assert_true (extents.x_bearing ==   413 &&
                 extents.y_bearing ==    50 &&
                 extents.width     ==   150 &&
                 extents.height    ==   -75);

  ret = hb_font_get_glyph_extents (font, 5, &extents);
  g_assert_true (ret);
  g_assert_true (extents.x_bearing ==   638 &&
                 extents.y_bearing ==   350 &&
                 extents.width     ==   600 &&
                 extents.height    ==  -600);

  ret = hb_font_get_glyph_extents (font, 1000, &extents);
  g_assert_false (ret);

  hb_font_destroy (font);
  hb_face_destroy (face);
}

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  hb_test_add (test_glyph_extents);

  return hb_test_run();
}
