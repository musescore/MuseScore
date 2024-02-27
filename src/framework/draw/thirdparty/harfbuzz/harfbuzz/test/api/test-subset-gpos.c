/*
 * Copyright © 2020 Adobe Inc.
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

/* Unit tests for GPOS subsetting */

static void
test_subset_gpos_lookup_subtable (void)
{
  hb_face_t *face_pwa = hb_test_open_font_file ("fonts/Roboto-Regular-gpos-.aw.ttf");
  hb_face_t *face_wa = hb_test_open_font_file ("fonts/Roboto-Regular-gpos-aw.ttf");

  hb_set_t *codepoints = hb_set_create ();
  hb_face_t *face_pwa_subset;
  hb_set_add (codepoints, 'a');
  hb_set_add (codepoints, 'w');

  hb_subset_input_t *input = hb_subset_test_create_input (codepoints);

  hb_set_del (hb_subset_input_set (input, HB_SUBSET_SETS_DROP_TABLE_TAG),
              HB_TAG ('G', 'P', 'O', 'S'));

  face_pwa_subset = hb_subset_test_create_subset (face_pwa, input);
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_wa, face_pwa_subset, HB_TAG ('G','P','O','S'));

  hb_face_destroy (face_pwa_subset);
  hb_face_destroy (face_pwa);
  hb_face_destroy (face_wa);
}

static void
test_subset_gpos_pairpos1_vf (void)
{
  hb_face_t *face_wav = hb_test_open_font_file ("fonts/AdobeVFPrototype.WAV.gpos.otf");
  hb_face_t *face_wa = hb_test_open_font_file ("fonts/AdobeVFPrototype.WA.gpos.otf");

  hb_set_t *codepoints = hb_set_create ();
  hb_face_t *face_wav_subset;
  hb_set_add (codepoints, 'W');
  hb_set_add (codepoints, 'A');

  hb_subset_input_t *input = hb_subset_test_create_input (codepoints);

  hb_set_del (hb_subset_input_set (input, HB_SUBSET_SETS_DROP_TABLE_TAG),
              HB_TAG ('G', 'P', 'O', 'S'));

  face_wav_subset = hb_subset_test_create_subset (face_wav, input);
  hb_set_destroy (codepoints);

  hb_subset_test_check (face_wa, face_wav_subset, HB_TAG ('G','P','O','S'));

  hb_face_destroy (face_wav_subset);
  hb_face_destroy (face_wav);
  hb_face_destroy (face_wa);
}

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  hb_test_add (test_subset_gpos_lookup_subtable);
  hb_test_add (test_subset_gpos_pairpos1_vf);

  return hb_test_run ();
}
