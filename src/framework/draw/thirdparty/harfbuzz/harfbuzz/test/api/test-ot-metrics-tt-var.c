/*
 * Copyright © 2019 Adobe Inc.
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

/* Unit tests for glyph advance widths and extents of TrueType variable fonts */

static void
test_extents_tt_var (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/SourceSansVariable-Roman-nohvar-41,C1.ttf");
  g_assert (face);
  hb_font_t *font = hb_font_create (face);
  hb_face_destroy (face);
  g_assert (font);
  hb_ot_font_set_funcs (font);

  hb_glyph_extents_t  extents;
  hb_bool_t result = hb_font_get_glyph_extents (font, 2, &extents);
  g_assert (result);

  g_assert_cmpint (extents.x_bearing, ==, 10);
  g_assert_cmpint (extents.y_bearing, ==, 846);
  g_assert_cmpint (extents.width, ==, 500);
  g_assert_cmpint (extents.height, ==, -846);

  float coords[1] = { 500.0f };
  hb_font_set_var_coords_design (font, coords, 1);
  result = hb_font_get_glyph_extents (font, 2, &extents);
  g_assert (result);

  g_assert_cmpint (extents.x_bearing, ==, 0);
  g_assert_cmpint (extents.y_bearing, ==, 874);
  g_assert_cmpint (extents.width, ==, 551);
  g_assert_cmpint (extents.height, ==, -874);

  hb_font_destroy (font);
}

static void
test_advance_tt_var_nohvar (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/SourceSansVariable-Roman-nohvar-41,C1.ttf");
  g_assert (face);
  hb_font_t *font = hb_font_create (face);
  hb_face_destroy (face);
  g_assert (font);
  hb_ot_font_set_funcs (font);

  hb_position_t x, y;
  hb_font_get_glyph_advance_for_direction(font, 2, HB_DIRECTION_LTR, &x, &y);

  g_assert_cmpint (x, ==, 520);
  g_assert_cmpint (y, ==, 0);

  hb_font_get_glyph_advance_for_direction(font, 2, HB_DIRECTION_TTB, &x, &y);

  g_assert_cmpint (x, ==, 0);
  g_assert_cmpint (y, ==, -1257);

  float coords[1] = { 500.0f };
  hb_font_set_var_coords_design (font, coords, 1);
  hb_font_get_glyph_advance_for_direction(font, 2, HB_DIRECTION_LTR, &x, &y);

  g_assert_cmpint (x, ==, 551);
  g_assert_cmpint (y, ==, 0);

  hb_font_get_glyph_advance_for_direction(font, 2, HB_DIRECTION_TTB, &x, &y);

  g_assert_cmpint (x, ==, 0);
  g_assert_cmpint (y, ==, -1257);

  hb_font_destroy (font);
}

static void
test_advance_tt_var_hvarvvar (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/SourceSerifVariable-Roman-VVAR.abc.ttf");
  g_assert (face);
  hb_font_t *font = hb_font_create (face);
  hb_face_destroy (face);
  g_assert (font);
  hb_ot_font_set_funcs (font);

  hb_position_t x, y;
  hb_font_get_glyph_advance_for_direction(font, 1, HB_DIRECTION_LTR, &x, &y);

  g_assert_cmpint (x, ==, 508);
  g_assert_cmpint (y, ==, 0);

  hb_font_get_glyph_advance_for_direction(font, 1, HB_DIRECTION_TTB, &x, &y);

  g_assert_cmpint (x, ==, 0);
  g_assert_cmpint (y, ==, -1000);

  float coords[1] = { 700.0f };
  hb_font_set_var_coords_design (font, coords, 1);
  hb_font_get_glyph_advance_for_direction(font, 1, HB_DIRECTION_LTR, &x, &y);

  g_assert_cmpint (x, ==, 531);
  g_assert_cmpint (y, ==, 0);

  hb_font_get_glyph_advance_for_direction(font, 1, HB_DIRECTION_TTB, &x, &y);

  g_assert_cmpint (x, ==, 0);
  g_assert_cmpint (y, ==, -1012);

  hb_font_destroy (font);
}

static void
test_advance_tt_var_anchor (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/SourceSansVariable-Roman.anchor.ttf");
  g_assert (face);
  hb_font_t *font = hb_font_create (face);
  hb_face_destroy (face);
  g_assert (font);
  hb_ot_font_set_funcs (font);

  hb_glyph_extents_t  extents;
  hb_bool_t result = hb_font_get_glyph_extents (font, 2, &extents);
  g_assert (result);

  g_assert_cmpint (extents.x_bearing, ==, 56);
  g_assert_cmpint (extents.y_bearing, ==, 672);
  g_assert_cmpint (extents.width, ==, 556);
  g_assert_cmpint (extents.height, ==, -684);

  float coords[1] = { 500.0f };
  hb_font_set_var_coords_design (font, coords, 1);
  result = hb_font_get_glyph_extents (font, 2, &extents);
  g_assert (result);

  g_assert_cmpint (extents.x_bearing, ==, 50);
  g_assert_cmpint (extents.y_bearing, ==, 667);
  g_assert_cmpint (extents.width, ==, 592);
  g_assert_cmpint (extents.height, ==, -679);

  hb_font_destroy (font);
}

static void
test_extents_tt_var_comp (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/SourceSansVariable-Roman.modcomp.ttf");
  g_assert (face);
  hb_font_t *font = hb_font_create (face);
  hb_face_destroy (face);
  g_assert (font);
  hb_ot_font_set_funcs (font);

  hb_glyph_extents_t  extents;
  float coords[1] = { 800.0f };
  hb_font_set_var_coords_design (font, coords, 1);

  hb_bool_t result;
  result = hb_font_get_glyph_extents (font, 2, &extents);	/* Ccedilla, cedilla y-scaled by 0.8, with unscaled component offset */
  g_assert (result);

  g_assert_cmpint (extents.x_bearing, ==, 19);
  g_assert_cmpint (extents.y_bearing, ==, 663);
  g_assert_cmpint (extents.width, ==, 519);
  g_assert_cmpint (extents.height, ==, -895);

  result = hb_font_get_glyph_extents (font, 3, &extents);	/* Cacute, acute y-scaled by 0.8, with unscaled component offset (default) */
  g_assert (result);

  g_assert_cmpint (extents.x_bearing, ==, 19);
  g_assert_cmpint (extents.y_bearing, ==, 909);
  g_assert_cmpint (extents.width, ==, 519);
  g_assert_cmpint (extents.height, ==, -921);

  result = hb_font_get_glyph_extents (font, 4, &extents);	/* Ccaron, caron y-scaled by 0.8, with scaled component offset */
  g_assert (result);

  g_assert_cmpint (extents.x_bearing, ==, 19);
  g_assert_cmpint (extents.y_bearing, ==, 866);
  g_assert_cmpint (extents.width, ==, 519);
  g_assert_cmpint (extents.height, ==, -878);

  hb_font_destroy (font);
}

static void
test_advance_tt_var_comp_v (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/SourceSansVariable-Roman.modcomp.ttf");
  g_assert (face);
  hb_font_t *font = hb_font_create (face);
  hb_face_destroy (face);
  g_assert (font);
  hb_ot_font_set_funcs (font);

  float coords[1] = { 800.0f };
  hb_font_set_var_coords_design (font, coords, 1);

  hb_position_t x, y;
  hb_font_get_glyph_advance_for_direction(font, 2, HB_DIRECTION_TTB, &x, &y);	/* No VVAR; 'C' in composite Ccedilla determines metrics */

  g_assert_cmpint (x, ==, 0);
  g_assert_cmpint (y, ==, -991);

  hb_font_get_glyph_origin_for_direction(font, 2, HB_DIRECTION_TTB, &x, &y);

  g_assert_cmpint (x, ==, 292);
  g_assert_cmpint (y, ==, 1013);

  hb_font_destroy (font);
}

static void
test_advance_tt_var_gvar_infer (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/TestGVAREight.ttf");
  hb_font_t *font = hb_font_create (face);
  hb_ot_font_set_funcs (font);
  hb_face_destroy (face);

  int coords[6] = {100};
  hb_font_set_var_coords_normalized (font, coords, 6);

  hb_glyph_extents_t extents = {0};
  g_assert (hb_font_get_glyph_extents (font, 4, &extents));

  hb_font_destroy (font);
}

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  hb_test_add (test_extents_tt_var);
  hb_test_add (test_advance_tt_var_nohvar);
  hb_test_add (test_advance_tt_var_hvarvvar);
  hb_test_add (test_advance_tt_var_anchor);
  hb_test_add (test_extents_tt_var_comp);
  hb_test_add (test_advance_tt_var_comp_v);
  hb_test_add (test_advance_tt_var_gvar_infer);

  return hb_test_run ();
}
