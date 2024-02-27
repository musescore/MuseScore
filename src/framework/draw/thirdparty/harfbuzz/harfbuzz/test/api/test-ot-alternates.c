/*
 * Copyright © 2020  Ebrahim Byagowi
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
 */

#include "hb-test.h"

#include <hb.h>
#include <hb-ot.h>

static void
test_ot_layout_lookup_get_glyph_alternates (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/SourceSansPro-Regular.otf");

  hb_codepoint_t alternates[3];
  unsigned alternates_count = 3;
  g_assert_cmpuint (7, ==, hb_ot_layout_lookup_get_glyph_alternates (face, 1, 1091, 2, &alternates_count, alternates));

  g_assert_cmpuint (3, ==, alternates_count);
  g_assert_cmpuint (1606, ==, alternates[0]);
  g_assert_cmpuint (1578, ==, alternates[1]);
  g_assert_cmpuint (1592, ==, alternates[2]);

  hb_face_destroy (face);
}

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);
  hb_test_add (test_ot_layout_lookup_get_glyph_alternates);
  return hb_test_run ();
}
