/*
 * Copyright 2021 Red Hat, Inc
 *
 * This is part of HarfBuzz, a text shaping library.
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

#include <hb.h>

/* test for https://github.com/harfbuzz/harfbuzz/issues/3274 */

static void
test_hb_scale (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/Cantarell.A.otf");
  hb_font_t *font1 = hb_font_create (face);
  hb_font_t *font2 = hb_font_create (face);

  double scale = 64 * 1024 * 96./72.;
  hb_font_set_scale (font1, scale, scale);
  hb_font_set_scale (font2, - scale, - scale);

  g_assert_cmpint (hb_font_get_glyph_h_advance (font1, 1), ==, - hb_font_get_glyph_h_advance (font2, 1));

  hb_font_destroy (font1);
  hb_font_destroy (font2);
  hb_face_destroy (face);
}

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);
  hb_test_add (test_hb_scale);
  return hb_test_run ();
}
