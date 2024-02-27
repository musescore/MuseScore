/*
 * Copyright © 2021  Khaled Hosny
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

#define BEGIN(tag, idx) \
  hb_ot_layout_lookup_collect_glyphs (face, tag, idx, before, input, after, output) \

#define END() \
  hb_set_clear (before); \
  hb_set_clear (input); \
  hb_set_clear (after); \
  hb_set_clear (output)

static void
test_ot_layout_lookup_collect_glyphs_source_sans (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/SourceSansPro-Regular.otf");

  hb_set_t *before = hb_set_create();
  hb_set_t *input = hb_set_create();
  hb_set_t *after = hb_set_create();
  hb_set_t *output = hb_set_create();
  hb_codepoint_t g = HB_SET_VALUE_INVALID;

  /* SingleSubst */
  BEGIN(HB_OT_TAG_GSUB, 0);
  g_assert_cmpuint (0, ==, hb_set_get_population (before));
  g_assert_cmpuint (684, ==, hb_set_get_population (input));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (54, ==, g);
  g_assert_cmpuint (0, ==, hb_set_get_population (after));
  g_assert_cmpuint (372, ==, hb_set_get_population (output));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (574, ==, g);
  END();

  /* AlternateSubst */
  BEGIN(HB_OT_TAG_GSUB, 1);
  g_assert_cmpuint (0, ==, hb_set_get_population (before));

  g_assert_cmpuint (143, ==, hb_set_get_population (input));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (2, ==, g);

  g_assert_cmpuint (0, ==, hb_set_get_population (after));

  g_assert_cmpuint (319, ==, hb_set_get_population (output));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (519, ==, g);
  END();

  /* MultipleSubst */
  BEGIN(HB_OT_TAG_GSUB, 7);
  g_assert_cmpuint (0, ==, hb_set_get_population (before));

  g_assert_cmpuint (10, ==, hb_set_get_population (input));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (92, ==, g);

  g_assert_cmpuint (0, ==, hb_set_get_population (after));

  g_assert_cmpuint (9, ==, hb_set_get_population (output));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (6, ==, g);
  END();

  /* LigatureSubst */
  BEGIN(HB_OT_TAG_GSUB, 10);
  g_assert_cmpuint (0, ==, hb_set_get_population (before));

  g_assert_cmpuint (14, ==, hb_set_get_population (input));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (1823, ==, g);

  g_assert_cmpuint (0, ==, hb_set_get_population (after));

  g_assert_cmpuint (22, ==, hb_set_get_population (output));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (1897, ==, g);
  END();

  /* ChainContextSubstFormat3 */
  BEGIN(HB_OT_TAG_GSUB, 8);
  g_assert_cmpuint (0, ==, hb_set_get_population (before));

  g_assert_cmpuint (10, ==, hb_set_get_population (input));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (92, ==, g);

  g_assert_cmpuint (2, ==, hb_set_get_population (after));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (after, &g));
  g_assert_cmpuint (1826, ==, g);
  g_assert (hb_set_next (after, &g));
  g_assert_cmpuint (1837, ==, g);

  g_assert_cmpuint (9, ==, hb_set_get_population (output));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (6, ==, g);
  END();

  /* ChainContextSubstFormat3 */
  BEGIN(HB_OT_TAG_GSUB, 13);
  g_assert_cmpuint (771, ==, hb_set_get_population (before));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (before, &g));
  g_assert_cmpuint (2, ==, g);

  g_assert_cmpuint (28, ==, hb_set_get_population (input));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (1823, ==, g);

  g_assert_cmpuint (0, ==, hb_set_get_population (after));

  g_assert_cmpuint (48, ==, hb_set_get_population (output));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (325, ==, g);
  END();

  /* MarkBasePos */
  BEGIN(HB_OT_TAG_GPOS, 0);
  g_assert_cmpuint (0, ==, hb_set_get_population (before));

  g_assert_cmpuint (179, ==, hb_set_get_population (input));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (28, ==, g);

  g_assert_cmpuint (0, ==, hb_set_get_population (after));
  g_assert_cmpuint (0, ==, hb_set_get_population (output));
  END();

  /* MarkMarkPos */
  BEGIN(HB_OT_TAG_GPOS, 9);
  g_assert_cmpuint (0, ==, hb_set_get_population (before));

  g_assert_cmpuint (48, ==, hb_set_get_population (input));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (1823, ==, g);

  g_assert_cmpuint (0, ==, hb_set_get_population (after));
  g_assert_cmpuint (0, ==, hb_set_get_population (output));
  END();

  /* PairPos */
  BEGIN(HB_OT_TAG_GPOS, 10);
  g_assert_cmpuint (0, ==, hb_set_get_population (before));

  g_assert_cmpuint (1426, ==, hb_set_get_population (input));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (2, ==, g);

  g_assert_cmpuint (0, ==, hb_set_get_population (after));
  g_assert_cmpuint (0, ==, hb_set_get_population (output));
  END();

  hb_face_destroy (face);
  face = hb_test_open_font_file ("fonts/SourceHanSans-Regular.41,3041,4C2E.otf");

  /* SinglePosFormat2 */
  BEGIN(HB_OT_TAG_GPOS, 1);
  g_assert_cmpuint (0, ==, hb_set_get_population (before));

  g_assert_cmpuint (1, ==, hb_set_get_population (input));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (4, ==, g);

  g_assert_cmpuint (0, ==, hb_set_get_population (after));
  g_assert_cmpuint (0, ==, hb_set_get_population (output));
  END();

  hb_face_destroy (face);
  hb_set_destroy (before);
  hb_set_destroy (input);
  hb_set_destroy (after);
  hb_set_destroy (output);
}

static void
test_ot_layout_lookup_collect_glyphs_noto_nastaliq (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/NotoNastaliqUrdu-Regular.ttf");

  hb_set_t *before = hb_set_create();
  hb_set_t *input = hb_set_create();
  hb_set_t *after = hb_set_create();
  hb_set_t *output = hb_set_create();
  hb_codepoint_t g = HB_SET_VALUE_INVALID;

  /* ExtensionSubst -> ContextSubstFormat1 */
  BEGIN(HB_OT_TAG_GSUB, 9);
  g_assert_cmpuint (0, ==, hb_set_get_population (before));

  g_assert_cmpuint (3, ==, hb_set_get_population (input));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (228, ==, g);
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (416, ==, g);
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (441, ==, g);

  g_assert_cmpuint (0, ==, hb_set_get_population (after));

  g_assert_cmpuint (3, ==, hb_set_get_population (output));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (267, ==, g);
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (268, ==, g);
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (279, ==, g);
  END();

  /* ExtensionSubst -> SingleSubst */
  BEGIN(HB_OT_TAG_GSUB, 10);
  g_assert_cmpuint (0, ==, hb_set_get_population (before));

  g_assert_cmpuint (3, ==, hb_set_get_population (input));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (228, ==, g);
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (416, ==, g);
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (441, ==, g);

  g_assert_cmpuint (0, ==, hb_set_get_population (after));

  g_assert_cmpuint (3, ==, hb_set_get_population (output));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (267, ==, g);
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (268, ==, g);
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (279, ==, g);
  END();

  /* ExtensionSubst -> ChainContextSubstFormat2 */
  BEGIN(HB_OT_TAG_GSUB, 16);
  g_assert_cmpuint (16, ==, hb_set_get_population (before));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (before, &g));
  g_assert_cmpuint (74, ==, g);

  g_assert_cmpuint (27, ==, hb_set_get_population (input));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (276, ==, g);

  g_assert_cmpuint (14, ==, hb_set_get_population (after));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (after, &g));
  g_assert_cmpuint (252, ==, g);

  g_assert_cmpuint (43, ==, hb_set_get_population (output));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (74, ==, g);
  END();

  /* ExtensionSubst -> ContextSubstFormat2 */
  BEGIN(HB_OT_TAG_GSUB, 39);
  g_assert_cmpuint (0, ==, hb_set_get_population (before));

  g_assert_cmpuint (246, ==, hb_set_get_population (input));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (252, ==, g);

  g_assert_cmpuint (0, ==, hb_set_get_population (after));

  g_assert_cmpuint (258, ==, hb_set_get_population (output));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (74, ==, g);
  END();


  /* CursivePos */
  BEGIN(HB_OT_TAG_GPOS, 0);
  g_assert_cmpuint (0, ==, hb_set_get_population (before));

  g_assert_cmpuint (616, ==, hb_set_get_population (input));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (228, ==, g);

  g_assert_cmpuint (0, ==, hb_set_get_population (after));
  g_assert_cmpuint (0, ==, hb_set_get_population (output));
  END();

  /* ExtensionSubst -> MarkLigPos */
  BEGIN(HB_OT_TAG_GPOS, 13);
  g_assert_cmpuint (0, ==, hb_set_get_population (before));

  g_assert_cmpuint (46, ==, hb_set_get_population (input));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (1004, ==, g);

  g_assert_cmpuint (0, ==, hb_set_get_population (after));
  g_assert_cmpuint (0, ==, hb_set_get_population (output));
  END();

  /* ExtensionSubst -> SinglePosFormat1 */
  BEGIN(HB_OT_TAG_GPOS, 17);
  g_assert_cmpuint (0, ==, hb_set_get_population (before));

  g_assert_cmpuint (242, ==, hb_set_get_population (input));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (257, ==, g);

  g_assert_cmpuint (0, ==, hb_set_get_population (after));
  g_assert_cmpuint (0, ==, hb_set_get_population (output));
  END();

  hb_face_destroy (face);
  hb_set_destroy (before);
  hb_set_destroy (input);
  hb_set_destroy (after);
  hb_set_destroy (output);
}

static void
test_ot_layout_lookup_collect_glyphs_qahiri (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/Qahiri-Regular.ttf");

  hb_set_t *before = hb_set_create();
  hb_set_t *input = hb_set_create();
  hb_set_t *after = hb_set_create();
  hb_set_t *output = hb_set_create();
  hb_codepoint_t g = HB_SET_VALUE_INVALID;

  /* SingleSubstFormat1 */
  BEGIN(HB_OT_TAG_GSUB, 0);
  g_assert_cmpuint (0, ==, hb_set_get_population (before));

  g_assert_cmpuint (4, ==, hb_set_get_population (input));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (52, ==, g);
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (60, ==, g);
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (62, ==, g);
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (159, ==, g);

  g_assert_cmpuint (0, ==, hb_set_get_population (after));

  g_assert_cmpuint (4, ==, hb_set_get_population (output));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (53, ==, g);
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (61, ==, g);
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (63, ==, g);
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (160, ==, g);
  END();

  /* ChainContextSubstFormat1 */
  BEGIN(HB_OT_TAG_GSUB, 11);
  g_assert_cmpuint (1, ==, hb_set_get_population (before));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (before, &g));
  g_assert_cmpuint (39, ==, g);

  g_assert_cmpuint (2, ==, hb_set_get_population (input));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (154, ==, g);
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (159, ==, g);

  g_assert_cmpuint (1, ==, hb_set_get_population (after));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (after, &g));
  g_assert_cmpuint (179, ==, g);

  g_assert_cmpuint (2, ==, hb_set_get_population (output));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (155, ==, g);
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (162, ==, g);
  END();

  /* ContextSubstFormat3 */
  BEGIN(HB_OT_TAG_GSUB, 31);
  g_assert_cmpuint (0, ==, hb_set_get_population (before));

  g_assert_cmpuint (4, ==, hb_set_get_population (input));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (53, ==, g);

  g_assert_cmpuint (0, ==, hb_set_get_population (after));

  g_assert_cmpuint (4, ==, hb_set_get_population (output));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (52, ==, g);
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (60, ==, g);
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (62, ==, g);
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (159, ==, g);
  END();

  /* ReverseChainSingleSubst */
  BEGIN(HB_OT_TAG_GSUB, 32);
  g_assert_cmpuint (0, ==, hb_set_get_population (before));

  g_assert_cmpuint (42, ==, hb_set_get_population (input));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (input, &g));
  g_assert_cmpuint (47, ==, g);

  g_assert_cmpuint (46, ==, hb_set_get_population (after));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (after, &g));
  g_assert_cmpuint (61, ==, g);

  g_assert_cmpuint (42, ==, hb_set_get_population (output));
  g = HB_SET_VALUE_INVALID;
  g_assert (hb_set_next (output, &g));
  g_assert_cmpuint (463, ==, g);
  END();

  hb_face_destroy (face);
  hb_set_destroy (before);
  hb_set_destroy (input);
  hb_set_destroy (after);
  hb_set_destroy (output);
}

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);
  hb_test_add (test_ot_layout_lookup_collect_glyphs_source_sans);
  hb_test_add (test_ot_layout_lookup_collect_glyphs_noto_nastaliq);
  hb_test_add (test_ot_layout_lookup_collect_glyphs_qahiri);
  return hb_test_run ();
}
