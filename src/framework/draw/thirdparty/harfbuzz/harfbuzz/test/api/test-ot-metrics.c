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
 */

#include "hb-test.h"

#include <hb-ot.h>

#include <math.h>

/* Unit tests for hb-ot-metrics.h */

static void
test_ot_metrics_get_no_var (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/cpal-v0.ttf");
  hb_font_t *font = hb_font_create (face);
  hb_position_t value;
  g_assert (hb_ot_metrics_get_position (font, HB_OT_METRICS_TAG_HORIZONTAL_ASCENDER, &value));
  g_assert_cmpint (value, ==, 1000);
  g_assert_cmpint (hb_ot_metrics_get_x_variation (font, HB_OT_METRICS_TAG_HORIZONTAL_ASCENDER), ==, 0);
  g_assert_cmpint (hb_ot_metrics_get_y_variation (font, HB_OT_METRICS_TAG_HORIZONTAL_ASCENDER), ==, 0);
  g_assert_cmpint (hb_ot_metrics_get_x_variation (font, HB_OT_METRICS_TAG_X_HEIGHT), ==, 0);
  // g_assert_cmpint ((int) hb_ot_metrics_get_variation (font, HB_OT_METRICS_TAG_HORIZONTAL_ASCENDER), ==, 0);
  hb_font_destroy (font);
  hb_face_destroy (face);
}

static void
test_ot_metrics_get_var (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/TestCFF2VF.otf");
  hb_font_t *font = hb_font_create (face);
  hb_position_t value;
  g_assert (hb_ot_metrics_get_position (font, HB_OT_METRICS_TAG_X_HEIGHT, &value));
  g_assert_cmpint (value, ==, 486);
  g_assert_cmpint (hb_ot_metrics_get_x_variation (font, HB_OT_METRICS_TAG_HORIZONTAL_ASCENDER), ==, 0);
  g_assert_cmpint (hb_ot_metrics_get_y_variation (font, HB_OT_METRICS_TAG_HORIZONTAL_ASCENDER), ==, 0);
  g_assert_cmpint (hb_ot_metrics_get_x_variation (font, HB_OT_METRICS_TAG_X_HEIGHT), ==, 0);
  float coords[] = {100.f};
  hb_font_set_var_coords_design (font, coords, 1);
  g_assert (hb_ot_metrics_get_position (font, HB_OT_METRICS_TAG_X_HEIGHT, &value));
  g_assert_cmpint (value, ==,  478);
  g_assert_cmpint (hb_ot_metrics_get_x_variation (font, HB_OT_METRICS_TAG_HORIZONTAL_ASCENDER), ==, 0);
  g_assert_cmpint (hb_ot_metrics_get_y_variation (font, HB_OT_METRICS_TAG_HORIZONTAL_ASCENDER), ==, 0);
  g_assert_cmpint (hb_ot_metrics_get_x_variation (font, HB_OT_METRICS_TAG_X_HEIGHT), ==, -8);
  hb_font_destroy (font);
  hb_face_destroy (face);
}

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);
  hb_test_add (test_ot_metrics_get_no_var);
  hb_test_add (test_ot_metrics_get_var);
  return hb_test_run ();
}
