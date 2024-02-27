/*
 * Copyright © 2018 Adobe Inc.
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
#include "hb-subset-test.h"

/* Unit tests for CFF subsetting */

static void
test_subset_cff1_noop (void)
{
  hb_face_t *face_abc = hb_test_open_font_file("fonts/SourceSansPro-Regular.abc.otf");

  hb_set_t *codepoints = hb_set_create ();
  hb_face_t *face_abc_subset;
  hb_set_add (codepoints, 'a');
  hb_set_add (codepoints, 'b');
  hb_set_add (codepoints, 'c');
  face_abc_subset = hb_subset_test_create_subset (face_abc, hb_subset_test_create_input (codepoints));
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_abc, face_abc_subset, HB_TAG ('C','F','F',' '));

  hb_face_destroy (face_abc_subset);
  hb_face_destroy (face_abc);
}

static void
test_subset_cff1 (void)
{
  hb_face_t *face_abc = hb_test_open_font_file ("fonts/SourceSansPro-Regular.abc.otf");
  hb_face_t *face_ac = hb_test_open_font_file ("fonts/SourceSansPro-Regular.ac.otf");

  hb_set_t *codepoints = hb_set_create ();
  hb_face_t *face_abc_subset;
  hb_set_add (codepoints, 'a');
  hb_set_add (codepoints, 'c');
  face_abc_subset = hb_subset_test_create_subset (face_abc, hb_subset_test_create_input (codepoints));
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_ac, face_abc_subset, HB_TAG ('C','F','F',' '));

  hb_face_destroy (face_abc_subset);
  hb_face_destroy (face_abc);
  hb_face_destroy (face_ac);
}

static void
test_subset_cff1_strip_hints (void)
{
  hb_face_t *face_abc = hb_test_open_font_file ("fonts/SourceSansPro-Regular.abc.otf");
  hb_face_t *face_ac = hb_test_open_font_file ("fonts/SourceSansPro-Regular.ac.nohints.otf");

  hb_set_t *codepoints = hb_set_create ();
  hb_subset_input_t *input;
  hb_face_t *face_abc_subset;
  hb_set_add (codepoints, 'a');
  hb_set_add (codepoints, 'c');
  input = hb_subset_test_create_input (codepoints);
  hb_subset_input_set_flags (input, HB_SUBSET_FLAGS_NO_HINTING);
  face_abc_subset = hb_subset_test_create_subset (face_abc, input);
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_ac, face_abc_subset, HB_TAG ('C', 'F', 'F', ' '));

  hb_face_destroy (face_abc_subset);
  hb_face_destroy (face_abc);
  hb_face_destroy (face_ac);
}

static void
test_subset_cff1_desubr (void)
{
  hb_face_t *face_abc = hb_test_open_font_file ("fonts/SourceSansPro-Regular.abc.otf");
  hb_face_t *face_ac = hb_test_open_font_file ("fonts/SourceSansPro-Regular.ac.nosubrs.otf");

  hb_set_t *codepoints = hb_set_create ();
  hb_subset_input_t *input;
  hb_face_t *face_abc_subset;
  hb_set_add (codepoints, 'a');
  hb_set_add (codepoints, 'c');
  input = hb_subset_test_create_input (codepoints);
  hb_subset_input_set_flags (input, HB_SUBSET_FLAGS_DESUBROUTINIZE);
  face_abc_subset = hb_subset_test_create_subset (face_abc, input);
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_ac, face_abc_subset, HB_TAG ('C','F','F',' '));

  hb_face_destroy (face_abc_subset);
  hb_face_destroy (face_abc);
  hb_face_destroy (face_ac);
}

static void
test_subset_cff1_desubr_strip_hints (void)
{
  hb_face_t *face_abc = hb_test_open_font_file ("fonts/SourceSansPro-Regular.abc.otf");
  hb_face_t *face_ac = hb_test_open_font_file ("fonts/SourceSansPro-Regular.ac.nosubrs.nohints.otf");

  hb_set_t *codepoints = hb_set_create ();
  hb_subset_input_t *input;
  hb_face_t *face_abc_subset;
  hb_set_add (codepoints, 'a');
  hb_set_add (codepoints, 'c');
  input = hb_subset_test_create_input (codepoints);
  hb_subset_input_set_flags (input,
                             HB_SUBSET_FLAGS_NO_HINTING | HB_SUBSET_FLAGS_DESUBROUTINIZE);
  face_abc_subset = hb_subset_test_create_subset (face_abc, input);
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_ac, face_abc_subset, HB_TAG ('C', 'F', 'F', ' '));

  hb_face_destroy (face_abc_subset);
  hb_face_destroy (face_abc);
  hb_face_destroy (face_ac);
}

static void
test_subset_cff1_j (void)
{
  hb_face_t *face_41_3041_4c2e = hb_test_open_font_file ("fonts/SourceHanSans-Regular.41,3041,4C2E.otf");
  hb_face_t *face_41_4c2e = hb_test_open_font_file ("fonts/SourceHanSans-Regular.41,4C2E.otf");

  hb_set_t *codepoints = hb_set_create ();
  hb_face_t *face_41_3041_4c2e_subset;
  hb_set_add (codepoints, 0x41);
  hb_set_add (codepoints, 0x4C2E);
  face_41_3041_4c2e_subset = hb_subset_test_create_subset (face_41_3041_4c2e, hb_subset_test_create_input (codepoints));
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_41_4c2e, face_41_3041_4c2e_subset, HB_TAG ('C','F','F',' '));

  hb_face_destroy (face_41_3041_4c2e_subset);
  hb_face_destroy (face_41_3041_4c2e);
  hb_face_destroy (face_41_4c2e);
}

static void
test_subset_cff1_j_strip_hints (void)
{
  hb_face_t *face_41_3041_4c2e = hb_test_open_font_file ("fonts/SourceHanSans-Regular.41,3041,4C2E.otf");
  hb_face_t *face_41_4c2e = hb_test_open_font_file ("fonts/SourceHanSans-Regular.41,4C2E.nohints.otf");

  hb_set_t *codepoints = hb_set_create ();
  hb_face_t *face_41_3041_4c2e_subset;
  hb_subset_input_t *input;
  hb_set_add (codepoints, 0x41);
  hb_set_add (codepoints, 0x4C2E);
  input = hb_subset_test_create_input (codepoints);
  hb_subset_input_set_flags (input, HB_SUBSET_FLAGS_NO_HINTING);
  face_41_3041_4c2e_subset = hb_subset_test_create_subset (face_41_3041_4c2e, input);
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_41_4c2e, face_41_3041_4c2e_subset, HB_TAG ('C','F','F',' '));

  hb_face_destroy (face_41_3041_4c2e_subset);
  hb_face_destroy (face_41_3041_4c2e);
  hb_face_destroy (face_41_4c2e);
}

static void
test_subset_cff1_j_desubr (void)
{
  hb_face_t *face_41_3041_4c2e = hb_test_open_font_file ("fonts/SourceHanSans-Regular.41,3041,4C2E.otf");
  hb_face_t *face_41_4c2e = hb_test_open_font_file ("fonts/SourceHanSans-Regular.41,4C2E.nosubrs.otf");

  hb_set_t *codepoints = hb_set_create ();
  hb_face_t *face_41_3041_4c2e_subset;
  hb_subset_input_t *input;
  hb_set_add (codepoints, 0x41);
  hb_set_add (codepoints, 0x4C2E);
  input = hb_subset_test_create_input (codepoints);
  hb_subset_input_set_flags (input, HB_SUBSET_FLAGS_DESUBROUTINIZE);
  face_41_3041_4c2e_subset = hb_subset_test_create_subset (face_41_3041_4c2e, input);
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_41_4c2e, face_41_3041_4c2e_subset, HB_TAG ('C','F','F',' '));

  hb_face_destroy (face_41_3041_4c2e_subset);
  hb_face_destroy (face_41_3041_4c2e);
  hb_face_destroy (face_41_4c2e);
}

static void
test_subset_cff1_j_desubr_strip_hints (void)
{
  hb_face_t *face_41_3041_4c2e = hb_test_open_font_file ("fonts/SourceHanSans-Regular.41,3041,4C2E.otf");
  hb_face_t *face_41_4c2e = hb_test_open_font_file ("fonts/SourceHanSans-Regular.41,4C2E.nosubrs.nohints.otf");

  hb_set_t *codepoints = hb_set_create ();
  hb_face_t *face_41_3041_4c2e_subset;
  hb_subset_input_t *input;
  hb_set_add (codepoints, 0x41);
  hb_set_add (codepoints, 0x4C2E);
  input = hb_subset_test_create_input (codepoints);
  hb_subset_input_set_flags (input,
                             HB_SUBSET_FLAGS_NO_HINTING | HB_SUBSET_FLAGS_DESUBROUTINIZE);
  face_41_3041_4c2e_subset = hb_subset_test_create_subset (face_41_3041_4c2e, input);
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_41_4c2e, face_41_3041_4c2e_subset, HB_TAG ('C','F','F',' '));

  hb_face_destroy (face_41_3041_4c2e_subset);
  hb_face_destroy (face_41_3041_4c2e);
  hb_face_destroy (face_41_4c2e);
}

static void
test_subset_cff1_expert (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/cff1_expert.otf");
  hb_face_t *face_subset = hb_test_open_font_file ("fonts/cff1_expert.2D,F6E9,FB00.otf");

  hb_set_t *codepoints = hb_set_create ();
  hb_face_t *face_test;
  hb_set_add (codepoints, 0x2D);
  hb_set_add (codepoints, 0xF6E9);
  hb_set_add (codepoints, 0xFB00);
  face_test = hb_subset_test_create_subset (face, hb_subset_test_create_input (codepoints));
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_subset, face_test, HB_TAG ('C','F','F',' '));

  hb_face_destroy (face_test);
  hb_face_destroy (face_subset);
  hb_face_destroy (face);
}

static void
test_subset_cff1_seac (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/cff1_seac.otf");
  hb_face_t *face_subset = hb_test_open_font_file ("fonts/cff1_seac.C0.otf");
  hb_face_t *face_test;

  hb_set_t *codepoints = hb_set_create ();
  hb_set_add (codepoints, 0xC0);  /* Agrave */
  face_test = hb_subset_test_create_subset (face, hb_subset_test_create_input (codepoints));
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_subset, face_test, HB_TAG ('C','F','F',' '));

  hb_face_destroy (face_test);
  hb_face_destroy (face_subset);
  hb_face_destroy (face);
}

static void
test_subset_cff1_dotsection (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/cff1_dotsect.otf");
  hb_face_t *face_subset = hb_test_open_font_file ("fonts/cff1_dotsect.nohints.otf");

  hb_set_t *codepoints = hb_set_create ();
  hb_subset_input_t *input;
  hb_face_t *face_test;
  hb_set_add (codepoints, 0x69);  /* i */
  input = hb_subset_test_create_input (codepoints);
  hb_subset_input_set_flags (input, HB_SUBSET_FLAGS_NO_HINTING);
  face_test = hb_subset_test_create_subset (face, input);
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_subset, face_test, HB_TAG ('C','F','F',' '));

  hb_face_destroy (face_test);
  hb_face_destroy (face_subset);
  hb_face_destroy (face);
}

static void
test_subset_cff1_retaingids (void)
{
  hb_face_t *face_abc = hb_test_open_font_file ("fonts/SourceSansPro-Regular.abc.otf");
  hb_face_t *face_ac = hb_test_open_font_file ("fonts/SourceSansPro-Regular.ac.retaingids.otf");

  hb_set_t *codepoints = hb_set_create ();
  hb_subset_input_t *input;
  hb_face_t *face_abc_subset;
  hb_set_add (codepoints, 'a');
  hb_set_add (codepoints, 'c');
  input = hb_subset_test_create_input (codepoints);
  hb_subset_input_set_flags (input, HB_SUBSET_FLAGS_RETAIN_GIDS);
  face_abc_subset = hb_subset_test_create_subset (face_abc, input);
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_ac, face_abc_subset, HB_TAG ('C','F','F',' '));

  hb_face_destroy (face_abc_subset);
  hb_face_destroy (face_abc);
  hb_face_destroy (face_ac);
}

static void
test_subset_cff1_j_retaingids (void)
{
  hb_face_t *face_41_3041_4c2e = hb_test_open_font_file ("fonts/SourceHanSans-Regular.41,3041,4C2E.otf");
  hb_face_t *face_41_4c2e = hb_test_open_font_file ("fonts/SourceHanSans-Regular.41,4C2E.retaingids.otf");

  hb_set_t *codepoints = hb_set_create ();
  hb_subset_input_t *input;
  hb_face_t *face_41_3041_4c2e_subset;
  hb_set_add (codepoints, 0x41);
  hb_set_add (codepoints, 0x4C2E);
  input = hb_subset_test_create_input (codepoints);
  hb_subset_input_set_flags (input, HB_SUBSET_FLAGS_RETAIN_GIDS);
  face_41_3041_4c2e_subset = hb_subset_test_create_subset (face_41_3041_4c2e, input);
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_41_4c2e, face_41_3041_4c2e_subset, HB_TAG ('C','F','F',' '));

  hb_face_destroy (face_41_3041_4c2e_subset);
  hb_face_destroy (face_41_3041_4c2e);
  hb_face_destroy (face_41_4c2e);
}

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  hb_test_add (test_subset_cff1_noop);
  hb_test_add (test_subset_cff1);
  hb_test_add (test_subset_cff1_strip_hints);
  hb_test_add (test_subset_cff1_desubr);
  hb_test_add (test_subset_cff1_desubr_strip_hints);
  hb_test_add (test_subset_cff1_j);
  hb_test_add (test_subset_cff1_j_strip_hints);
  hb_test_add (test_subset_cff1_j_desubr);
  hb_test_add (test_subset_cff1_j_desubr_strip_hints);
  hb_test_add (test_subset_cff1_expert);
  hb_test_add (test_subset_cff1_seac);
  hb_test_add (test_subset_cff1_dotsection);
  hb_test_add (test_subset_cff1_retaingids);
  hb_test_add (test_subset_cff1_j_retaingids);

  return hb_test_run ();
}
