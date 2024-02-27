/*
 * Copyright © 2019  Ebrahim Byagowi
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

#include <hb-ot.h>

/* Unit tests for hb_font_[gs]et_var_coords_ */

static void
test_get_var_coords (void)
{
#ifndef G_APPROX_VALUE
#define G_APPROX_VALUE(a, b, epsilon) \
  (((a) > (b) ? (a) - (b) : (b) - (a)) < (epsilon))
#endif

#define EPSILON 0.05f
	
  hb_face_t *face = hb_test_open_font_file ("fonts/TestCFF2VF.otf");
  hb_font_t *font = hb_font_create (face);

  /* Normalized coords as input */
  int normalized_coords[] = {100, 0};
  hb_font_set_var_coords_normalized (font, normalized_coords, 2);
  g_assert_cmpint ((int) hb_font_get_var_coords_design (font, NULL)[0], ==, 403);
  g_assert_cmpint ((int) hb_font_get_var_coords_normalized (font, NULL)[0], ==, 100);

  /* Design coords as input */
  float design_coords[] = {206.f, 0};
  hb_font_set_var_coords_design (font, design_coords, 2);
  g_assert_cmpint ((int) hb_font_get_var_coords_normalized (font, NULL)[0], ==, -16117);
  g_assert_cmpint ((int) hb_font_get_var_coords_design (font, NULL)[0], ==, 206);

  for (float weight = 200; weight < 901; ++weight)
  {
    int normalized;
    hb_ot_var_normalize_coords (face, 1, &weight, &normalized);
    hb_font_set_var_coords_normalized (font, &normalized, 1);
    float converted_back = hb_font_get_var_coords_design (font, NULL)[0];
    // fprintf (stderr, "%f: %d => %f\n", weight, normalized, converted_back);
    g_assert_true (G_APPROX_VALUE (converted_back, weight, EPSILON));
  }

  hb_font_destroy (font);
  hb_face_destroy (face);
}

static void
test_get_var_get_axis_infos (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/Estedad-VF.ttf");

  g_assert_cmpint (hb_ot_var_get_axis_count (face), ==, 2);

  hb_ot_var_axis_info_t info;
  unsigned c = 1;

  g_assert_cmpint (hb_ot_var_get_axis_infos (face, 0, &c, &info), ==, 2);
  g_assert (info.tag == HB_TAG ('w','g','h','t'));
  g_assert_cmpint (c, ==, 1);

  hb_ot_var_get_axis_infos (face, 1, &c, &info);
  g_assert (info.tag == HB_TAG ('w','d','t','h'));
  g_assert_cmpint (c, ==, 1);

  hb_ot_var_get_axis_infos (face, 2, &c, &info);
  g_assert_cmpint (c, ==, 0);

  hb_face_destroy (face);
}

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);
  hb_test_add (test_get_var_coords);
  hb_test_add (test_get_var_get_axis_infos);
  return hb_test_run ();
}
