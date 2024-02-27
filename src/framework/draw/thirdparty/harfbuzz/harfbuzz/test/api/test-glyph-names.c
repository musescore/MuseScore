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
test_glyph_names_post (void)
{
  hb_face_t *face;
  hb_font_t *font;
  hb_bool_t ret;
  char name [64];

  face = hb_test_open_font_file ("fonts/adwaita.ttf");
  font = hb_font_create (face);

  ret = hb_font_get_glyph_name (font, 0, name, 64);
  g_assert_true (ret);
  g_assert_cmpstr (name, ==, ".notdef");

  ret = hb_font_get_glyph_name (font, 1, name, 64);
  g_assert_true (ret);
  g_assert_cmpstr (name, ==, ".space");

  ret = hb_font_get_glyph_name (font, 2, name, 64);
  g_assert_true (ret);
  g_assert_cmpstr (name, ==, "icon0");

  ret = hb_font_get_glyph_name (font, 3, name, 64);
  g_assert_true (ret);
  g_assert_cmpstr (name, ==, "icon0.0");

  ret = hb_font_get_glyph_name (font, 4, name, 64);
  g_assert_true (ret);
  g_assert_cmpstr (name, ==, "icon0.1");

  ret = hb_font_get_glyph_name (font, 5, name, 64);
  g_assert_true (ret);
  g_assert_cmpstr (name, ==, "icon0.2");

  /* beyond last glyph */
  ret = hb_font_get_glyph_name (font, 100, name, 64);
  g_assert_false (ret);

  hb_font_destroy (font);
  hb_face_destroy (face);
}

static void
test_glyph_names_cff (void)
{
  hb_face_t *face;
  hb_font_t *font;
  hb_bool_t ret;
  char name [64];

  face = hb_test_open_font_file ("fonts/SourceSansPro-Regular.otf");
  font = hb_font_create (face);

  ret = hb_font_get_glyph_name (font, 0, name, 64);
  g_assert_true (ret);
  g_assert_cmpstr (name, ==, ".notdef");

  ret = hb_font_get_glyph_name (font, 1, name, 64);
  g_assert_true (ret);
  g_assert_cmpstr (name, ==, "space");

  ret = hb_font_get_glyph_name (font, 2, name, 64);
  g_assert_true (ret);
  g_assert_cmpstr (name, ==, "A");

  /* beyond last glyph */
  ret = hb_font_get_glyph_name (font, 2000, name, 64);
  g_assert_false (ret);

  hb_font_destroy (font);
  hb_face_destroy (face);
}

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  hb_test_add (test_glyph_names_post);
  hb_test_add (test_glyph_names_cff);

  return hb_test_run();
}
