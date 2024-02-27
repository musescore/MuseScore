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
#include "hb-subset-test.h"

/* Unit tests for HVAR subsetting */

static void
test_subset_map_HVAR_noop (void)
{
  hb_face_t *face_abc = hb_test_open_font_file("fonts/AdobeVFPrototype.abc.otf");

  hb_set_t *codepoints = hb_set_create ();
  hb_face_t *face_abc_subset;
  hb_set_add (codepoints, 'a');
  hb_set_add (codepoints, 'b');
  hb_set_add (codepoints, 'c');
  face_abc_subset = hb_subset_test_create_subset (face_abc, hb_subset_test_create_input (codepoints));
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_abc, face_abc_subset, HB_TAG ('H','V','A','R'));

  hb_face_destroy (face_abc_subset);
  hb_face_destroy (face_abc);
}

static void
test_subset_map_HVAR (void)
{
  hb_face_t *face_abc = hb_test_open_font_file ("fonts/AdobeVFPrototype.abc.otf");
  hb_face_t *face_ac = hb_test_open_font_file ("fonts/AdobeVFPrototype.ac.otf");

  hb_set_t *codepoints = hb_set_create ();
  hb_face_t *face_abc_subset;
  hb_set_add (codepoints, 'a');
  hb_set_add (codepoints, 'c');
  face_abc_subset = hb_subset_test_create_subset (face_abc, hb_subset_test_create_input (codepoints));
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_ac, face_abc_subset, HB_TAG ('H','V','A','R'));

  hb_face_destroy (face_abc_subset);
  hb_face_destroy (face_abc);
  hb_face_destroy (face_ac);
}

static void
test_subset_map_HVAR_retaingids (void)
{
  hb_face_t *face_abc = hb_test_open_font_file ("fonts/AdobeVFPrototype.abc.otf");
  hb_face_t *face_ac = hb_test_open_font_file ("fonts/AdobeVFPrototype.ac.retaingids.otf");

  hb_set_t *codepoints = hb_set_create ();
  hb_face_t *face_abc_subset;
  hb_set_add (codepoints, 'a');
  hb_set_add (codepoints, 'c');
  hb_subset_input_t *input = hb_subset_test_create_input (codepoints);
  hb_subset_input_set_flags (input, HB_SUBSET_FLAGS_RETAIN_GIDS);
  face_abc_subset = hb_subset_test_create_subset (face_abc, input);
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_ac, face_abc_subset, HB_TAG ('H','V','A','R'));

  hb_face_destroy (face_abc_subset);
  hb_face_destroy (face_abc);
  hb_face_destroy (face_ac);
}

static void
test_subset_map_modHVAR (void)
{
  hb_face_t *face_abc = hb_test_open_font_file ("fonts/SourceSansVariable-Roman-modHVAR.abc.ttf");
  hb_face_t *face_ac = hb_test_open_font_file ("fonts/SourceSansVariable-Roman-modHVAR.ac.ttf");

  hb_set_t *codepoints = hb_set_create ();
  hb_face_t *face_abc_subset;
  hb_set_add (codepoints, 'a');
  hb_set_add (codepoints, 'c');
  face_abc_subset = hb_subset_test_create_subset (face_abc, hb_subset_test_create_input (codepoints));
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_ac, face_abc_subset, HB_TAG ('H','V','A','R'));

  hb_face_destroy (face_abc_subset);
  hb_face_destroy (face_abc);
  hb_face_destroy (face_ac);
}

static void
test_subset_identity_HVAR_noop (void)
{
  hb_face_t *face_abc = hb_test_open_font_file("fonts/SourceSansVariable-Roman.abc.ttf");

  hb_set_t *codepoints = hb_set_create ();
  hb_face_t *face_abc_subset;
  hb_set_add (codepoints, 'a');
  hb_set_add (codepoints, 'b');
  hb_set_add (codepoints, 'c');
  face_abc_subset = hb_subset_test_create_subset (face_abc, hb_subset_test_create_input (codepoints));
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_abc, face_abc_subset, HB_TAG ('H','V','A','R'));

  hb_face_destroy (face_abc_subset);
  hb_face_destroy (face_abc);
}

static void
test_subset_identity_HVAR (void)
{
  hb_face_t *face_abc = hb_test_open_font_file ("fonts/SourceSansVariable-Roman.abc.ttf");
  hb_face_t *face_ac = hb_test_open_font_file ("fonts/SourceSansVariable-Roman.ac.ttf");

  hb_set_t *codepoints = hb_set_create ();
  hb_face_t *face_abc_subset;
  hb_set_add (codepoints, 'a');
  hb_set_add (codepoints, 'c');
  face_abc_subset = hb_subset_test_create_subset (face_abc, hb_subset_test_create_input (codepoints));
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_ac, face_abc_subset, HB_TAG ('H','V','A','R'));

  hb_face_destroy (face_abc_subset);
  hb_face_destroy (face_abc);
  hb_face_destroy (face_ac);
}

static void
test_subset_identity_HVAR_retaingids (void)
{
  hb_face_t *face_abc = hb_test_open_font_file ("fonts/SourceSansVariable-Roman.abc.ttf");
  hb_face_t *face_ac = hb_test_open_font_file ("fonts/SourceSansVariable-Roman.ac.retaingids.ttf");

  hb_set_t *codepoints = hb_set_create ();
  hb_face_t *face_abc_subset;
  hb_set_add (codepoints, 'a');
  hb_set_add (codepoints, 'c');
  hb_subset_input_t *input = hb_subset_test_create_input (codepoints);
  hb_subset_input_set_flags (input, HB_SUBSET_FLAGS_RETAIN_GIDS);
  face_abc_subset = hb_subset_test_create_subset (face_abc, input);
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_ac, face_abc_subset, HB_TAG ('H','V','A','R'));

  hb_face_destroy (face_abc_subset);
  hb_face_destroy (face_abc);
  hb_face_destroy (face_ac);
}


int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  hb_test_add (test_subset_map_HVAR_noop);
  hb_test_add (test_subset_map_HVAR);
  hb_test_add (test_subset_map_HVAR_retaingids);
  hb_test_add (test_subset_map_modHVAR);
  hb_test_add (test_subset_identity_HVAR_noop);
  hb_test_add (test_subset_identity_HVAR);
  hb_test_add (test_subset_identity_HVAR_retaingids);

  return hb_test_run ();
}
