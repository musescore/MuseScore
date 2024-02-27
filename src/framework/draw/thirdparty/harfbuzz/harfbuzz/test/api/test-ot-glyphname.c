/*
 * Copyright © 2019  Adobe, Inc.
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
#include "hb-ot.h"

static void
test_one_glyph (hb_font_t *font,  hb_codepoint_t gid, const char *name)
{
  char			buf[64];
  hb_codepoint_t	glyph;

  g_assert(hb_font_get_glyph_name (font, gid, buf, sizeof (buf)));
  g_assert_cmpstr(buf, ==, name);
  g_assert(hb_font_get_glyph_from_name (font, name, -1, &glyph));
  g_assert_cmpint(glyph, ==, gid);
}

/* Unit tests for CFF glyph names  */

static void
test_standard_names (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/MathTestFontFull.otf");
  hb_font_t *font = hb_font_create (face);

  test_one_glyph (font, 0,   ".notdef");
  test_one_glyph (font, 27,  "Z");

  hb_font_destroy (font);
  hb_face_destroy (face);
}

static void
test_non_standard_names (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/MathTestFontFull.otf");
  hb_font_t *font = hb_font_create (face);

  test_one_glyph (font, 46,  "arrowdblright");
  test_one_glyph (font, 138, "uni21E7_size5");

  hb_font_destroy (font);
  hb_face_destroy (face);
}

static void
test_predef_charset_names (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/cff1_expert.otf");
  hb_font_t *font = hb_font_create (face);

  test_one_glyph (font, 0,   ".notdef");
  test_one_glyph (font, 29,  "centsuperior");
  test_one_glyph (font, 86,  "commainferior");

  hb_font_destroy (font);
  hb_face_destroy (face);
}

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  hb_test_add (test_standard_names);
  hb_test_add (test_non_standard_names);
  hb_test_add (test_predef_charset_names);

  return hb_test_run();
}
