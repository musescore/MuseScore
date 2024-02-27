/*
 * Copyright © 2020  Google, Inc.
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
 * Google Author(s): Calder Kitagawa
 */

#include "hb-test.h"
#include "hb-subset-test.h"

/* Unit tests for COLR subsetting */

static void
test_subset_colr_noop (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/TwemojiMozilla.subset.ttf");

  hb_set_t *codepoints = hb_set_create ();
  hb_face_t *face_subset;
  hb_set_add (codepoints, '2');
  hb_set_add (codepoints, 0x3297);
  hb_set_add (codepoints, 0x3299);
  face_subset = hb_subset_test_create_subset (face, hb_subset_test_create_input (codepoints));
  hb_set_destroy (codepoints);

  hb_subset_test_check (face, face_subset, HB_TAG ('C','O','L','R'));

  hb_face_destroy (face_subset);
  hb_face_destroy (face);
}

static void
test_subset_colr_keep_one_colr_glyph (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/TwemojiMozilla.subset.ttf");
  hb_face_t *face_expected = hb_test_open_font_file ("fonts/TwemojiMozilla.subset.default.3297.ttf");

  hb_set_t *codepoints = hb_set_create ();
  hb_face_t *face_subset;
  hb_set_add (codepoints, 0x3297);
  face_subset = hb_subset_test_create_subset (face, hb_subset_test_create_input (codepoints));
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_expected, face_subset, HB_TAG ('C','O','L','R'));

  hb_face_destroy (face_subset);
  hb_face_destroy (face_expected);
  hb_face_destroy (face);
}

static void
test_subset_colr_keep_mixed_glyph (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/TwemojiMozilla.subset.ttf");
  hb_face_t *face_expected = hb_test_open_font_file ("fonts/TwemojiMozilla.subset.default.32,3299.ttf");

  hb_set_t *codepoints = hb_set_create ();
  hb_face_t *face_subset;
  hb_set_add (codepoints, '2');
  hb_set_add (codepoints, 0x3299);
  face_subset = hb_subset_test_create_subset (face, hb_subset_test_create_input (codepoints));
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_expected, face_subset, HB_TAG ('C','O','L','R'));

  hb_face_destroy (face_subset);
  hb_face_destroy (face_expected);
  hb_face_destroy (face);
}

static void
test_subset_colr_keep_no_colr_glyph (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/TwemojiMozilla.subset.ttf");
  hb_face_t *face_expected = hb_test_open_font_file ("fonts/TwemojiMozilla.subset.default.32.ttf");

  hb_set_t *codepoints = hb_set_create ();
  hb_face_t *face_subset;
  hb_set_add (codepoints, '2');
  face_subset = hb_subset_test_create_subset (face, hb_subset_test_create_input (codepoints));
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_expected, face_subset, HB_TAG ('C','O','L','R'));

  hb_face_destroy (face_subset);
  hb_face_destroy (face_expected);
  hb_face_destroy (face);
}

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  hb_test_add (test_subset_colr_noop);
  hb_test_add (test_subset_colr_keep_one_colr_glyph);
  hb_test_add (test_subset_colr_keep_mixed_glyph);
  hb_test_add (test_subset_colr_keep_no_colr_glyph);

  return hb_test_run();
}
