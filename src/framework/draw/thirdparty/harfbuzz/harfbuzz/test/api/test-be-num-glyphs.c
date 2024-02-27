/*
 * Copyright © 2022  Behdad Esfahbod
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

static void
test_maxp_and_loca (void)
{
  hb_face_t *face;

  const char maxp_data[] = "\x00\x00\x50\x00" // version
			   "\x00\x05" // numGlyphs
			   ;
#ifndef HB_NO_BEYOND_64K
  const char loca_data[18] = "";
#endif

  face = hb_face_builder_create ();
  g_assert_cmpuint (hb_face_get_glyph_count (face), ==, 0);
  hb_face_destroy (face);

  face = hb_face_builder_create ();
  HB_FACE_ADD_TABLE (face, "maxp", maxp_data);
  g_assert_cmpuint (hb_face_get_glyph_count (face), ==, 5);
  hb_face_destroy (face);

#ifndef HB_NO_BEYOND_64K
  face = hb_face_builder_create ();
  HB_FACE_ADD_TABLE (face, "maxp", maxp_data);
  HB_FACE_ADD_TABLE (face, "loca", loca_data);
  g_assert_cmpuint (hb_face_get_glyph_count (face), ==, 8);
  hb_face_destroy (face);

  face = hb_face_builder_create ();
  HB_FACE_ADD_TABLE (face, "loca", loca_data);
  g_assert_cmpuint (hb_face_get_glyph_count (face), ==, 8);
  hb_face_destroy (face);
#endif
}


int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  hb_test_add (test_maxp_and_loca);

  return hb_test_run();
}
