/*
 * Copyright © 2019  Google, Inc.
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
 * Google Author(s): Garret Rieger
 */

#include "hb-test.h"
#include "hb-subset-test.h"

/* Unit tests for hb-subset.cc drop tables functionality */

static void
test_subset_drop_tables (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/Roboto-Regular.abc.ttf");

  hb_set_t *codepoints = hb_set_create();
  hb_set_add (codepoints, 97);
  hb_set_add (codepoints, 99);
  hb_subset_input_t *input = hb_subset_test_create_input (codepoints);
  hb_set_add (hb_subset_input_set (input, HB_SUBSET_SETS_DROP_TABLE_TAG), HB_TAG ('h', 'd', 'm', 'x'));
  hb_set_add (hb_subset_input_set (input, HB_SUBSET_SETS_DROP_TABLE_TAG), HB_TAG ('h', 'm', 't', 'x'));
  hb_set_destroy (codepoints);

  hb_face_t* subset = hb_subset_or_fail (face, input);
  g_assert (subset);

  hb_blob_t *hdmx = hb_face_reference_table (subset, HB_TAG ('h', 'd', 'm', 'x'));
  hb_blob_t *hmtx = hb_face_reference_table (subset, HB_TAG ('h', 'm', 't', 'x'));
  hb_blob_t *cmap = hb_face_reference_table (subset, HB_TAG ('c', 'm', 'a', 'p'));
  g_assert (!hb_blob_get_length (hdmx));
  g_assert (!hb_blob_get_length (hmtx));
  g_assert ( hb_blob_get_length (cmap));
  hb_blob_destroy (hdmx);
  hb_blob_destroy (hmtx);
  hb_blob_destroy (cmap);

  hb_face_destroy (subset);
  hb_subset_input_destroy (input);
  hb_face_destroy (face);
}


int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  hb_test_add (test_subset_drop_tables);

  return hb_test_run();
}
