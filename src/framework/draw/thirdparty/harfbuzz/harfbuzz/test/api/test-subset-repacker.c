/*
 * Copyright © 2022  Google, Inc.
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
 */

#include "hb-test.h"
#include "hb-subset-test.h"

#ifdef HB_EXPERIMENTAL_API
#include "hb-subset-repacker.h"

char test_gsub_data[106] = "\x0\x1\x0\x0\x0\xa\x0\x1e\x0\x2c\x0\x1\x6c\x61\x74\x6e\x0\x8\x0\x4\x0\x0\x0\x0\xff\xff\x0\x1\x0\x0\x0\x1\x74\x65\x73\x74\x0\x8\x0\x0\x0\x1\x0\x1\x0\x2\x0\x2a\x0\x6\x0\x5\x0\x0\x0\x1\x0\x8\x0\x1\x0\x8\x0\x1\x0\xe\x0\x1\x0\x1\x0\x1\x0\x1\x0\x4\x0\x2\x0\x1\x0\x2\x0\x1\x0\x0\x0\x1\x0\x0\x0\x1\x0\x8\x0\x1\x0\x6\x0\x1\x0\x1\x0\x1\x0\x2";

static void
test_hb_repack_with_cy_struct (void)
{
  hb_object_t *hb_objs = calloc (15, sizeof (hb_object_t));

  hb_objs[0].head = &(test_gsub_data[100]);
  hb_objs[0].tail = &(test_gsub_data[105]) + 1;
  hb_objs[0].num_real_links = 0;
  hb_objs[0].num_virtual_links = 0;
  hb_objs[0].real_links = NULL;
  hb_objs[0].virtual_links = NULL;

  hb_objs[1].head = &(test_gsub_data[94]);
  hb_objs[1].tail = &(test_gsub_data[100]);
  hb_objs[1].num_real_links = 1;
  hb_objs[1].num_virtual_links = 0;
  hb_objs[1].real_links = malloc (sizeof (hb_link_t));
  hb_objs[1].real_links[0].width = 2;
  hb_objs[1].real_links[0].position = 2;
  hb_objs[1].real_links[0].objidx = 1;
  hb_objs[1].virtual_links = NULL;


  hb_objs[2].head = &(test_gsub_data[86]);
  hb_objs[2].tail = &(test_gsub_data[94]);
  hb_objs[2].num_real_links = 1;
  hb_objs[2].num_virtual_links = 0;
  hb_objs[2].real_links = malloc (sizeof (hb_link_t));
  hb_objs[2].real_links[0].width = 2;
  hb_objs[2].real_links[0].position = 6;
  hb_objs[2].real_links[0].objidx = 2;
  hb_objs[2].virtual_links = NULL;

  hb_objs[3].head = &(test_gsub_data[76]);
  hb_objs[3].tail = &(test_gsub_data[86]);
  hb_objs[3].num_real_links = 0;
  hb_objs[3].num_virtual_links = 0;
  hb_objs[3].real_links = NULL;
  hb_objs[3].virtual_links = NULL;

  hb_objs[4].head = &(test_gsub_data[72]);
  hb_objs[4].tail = &(test_gsub_data[76]);
  hb_objs[4].num_real_links = 1;
  hb_objs[4].num_virtual_links = 0;
  hb_objs[4].real_links = malloc (sizeof (hb_link_t));
  hb_objs[4].real_links[0].width = 2;
  hb_objs[4].real_links[0].position = 2;
  hb_objs[4].real_links[0].objidx = 4;
  hb_objs[4].virtual_links = NULL;

  hb_objs[5].head = &(test_gsub_data[66]);
  hb_objs[5].tail = &(test_gsub_data[72]);
  hb_objs[5].num_real_links = 0;
  hb_objs[5].num_virtual_links = 0;
  hb_objs[5].real_links = NULL;
  hb_objs[5].virtual_links = NULL;

  hb_objs[6].head = &(test_gsub_data[58]);
  hb_objs[6].tail = &(test_gsub_data[66]);
  hb_objs[6].num_real_links = 2;
  hb_objs[6].num_virtual_links = 0;
  hb_objs[6].real_links = calloc (2, sizeof (hb_link_t));
  hb_objs[6].real_links[0].width = 2;
  hb_objs[6].real_links[0].position = 6;
  hb_objs[6].real_links[0].objidx = 5;
  hb_objs[6].real_links[1].width = 2;
  hb_objs[6].real_links[1].position = 2;
  hb_objs[6].real_links[1].objidx = 6;
  hb_objs[6].virtual_links = NULL;

  hb_objs[7].head = &(test_gsub_data[50]);
  hb_objs[7].tail = &(test_gsub_data[58]);
  hb_objs[7].num_real_links = 1;
  hb_objs[7].num_virtual_links = 0;
  hb_objs[7].real_links = malloc (sizeof (hb_link_t));
  hb_objs[7].real_links[0].width = 2;
  hb_objs[7].real_links[0].position = 6;
  hb_objs[7].real_links[0].objidx = 7;
  hb_objs[7].virtual_links = NULL;

  hb_objs[8].head = &(test_gsub_data[44]);
  hb_objs[8].tail = &(test_gsub_data[50]);
  hb_objs[8].num_real_links = 2;
  hb_objs[8].num_virtual_links = 0;
  hb_objs[8].real_links = calloc (2, sizeof (hb_link_t));
  hb_objs[8].real_links[0].width = 2;
  hb_objs[8].real_links[0].position = 2;
  hb_objs[8].real_links[0].objidx = 3;
  hb_objs[8].real_links[1].width = 2;
  hb_objs[8].real_links[1].position = 4;
  hb_objs[8].real_links[1].objidx = 8;
  hb_objs[8].virtual_links = NULL;

  hb_objs[9].head = &(test_gsub_data[38]);
  hb_objs[9].tail = &(test_gsub_data[44]);
  hb_objs[9].num_real_links = 0;
  hb_objs[9].num_virtual_links = 0;
  hb_objs[9].real_links = NULL;
  hb_objs[9].virtual_links = NULL;

  hb_objs[10].head = &(test_gsub_data[30]);
  hb_objs[10].tail = &(test_gsub_data[38]);
  hb_objs[10].num_real_links = 1;
  hb_objs[10].num_virtual_links = 0;
  hb_objs[10].real_links = malloc (sizeof (hb_link_t));
  hb_objs[10].real_links[0].width = 2;
  hb_objs[10].real_links[0].position = 6;
  hb_objs[10].real_links[0].objidx = 10;
  hb_objs[10].virtual_links = NULL;

  hb_objs[11].head = &(test_gsub_data[22]);
  hb_objs[11].tail = &(test_gsub_data[30]);
  hb_objs[11].num_real_links = 0;
  hb_objs[11].num_virtual_links = 0;
  hb_objs[11].real_links = NULL;
  hb_objs[11].virtual_links = NULL;

  hb_objs[12].head = &(test_gsub_data[18]);
  hb_objs[12].tail = &(test_gsub_data[22]);
  hb_objs[12].num_real_links = 1;
  hb_objs[12].num_virtual_links = 0;
  hb_objs[12].real_links = malloc (sizeof (hb_link_t));
  hb_objs[12].real_links[0].width = 2;
  hb_objs[12].real_links[0].position = 0;
  hb_objs[12].real_links[0].objidx = 12;
  hb_objs[12].virtual_links = NULL;

  hb_objs[13].head = &(test_gsub_data[10]);
  hb_objs[13].tail = &(test_gsub_data[18]);
  hb_objs[13].num_real_links = 1;
  hb_objs[13].num_virtual_links = 0;
  hb_objs[13].real_links = malloc (sizeof (hb_link_t));
  hb_objs[13].real_links[0].width = 2;
  hb_objs[13].real_links[0].position = 6;
  hb_objs[13].real_links[0].objidx = 13;
  hb_objs[13].virtual_links = NULL;

  hb_objs[14].head = &(test_gsub_data[0]);
  hb_objs[14].tail = &(test_gsub_data[10]);
  hb_objs[14].num_real_links = 3;
  hb_objs[14].num_virtual_links = 0;
  hb_objs[14].real_links = calloc (3, sizeof (hb_link_t));
  hb_objs[14].real_links[0].width = 2;
  hb_objs[14].real_links[0].position = 8;
  hb_objs[14].real_links[0].objidx = 9;
  hb_objs[14].real_links[1].width = 2;
  hb_objs[14].real_links[1].position = 6;
  hb_objs[14].real_links[1].objidx = 11;
  hb_objs[14].real_links[2].width = 2;
  hb_objs[14].real_links[2].position = 4;
  hb_objs[14].real_links[2].objidx = 14;
  hb_objs[14].virtual_links = NULL;

  hb_blob_t *result = hb_subset_repack_or_fail (HB_TAG_NONE, hb_objs, 15);

  hb_face_t *face_expected = hb_test_open_font_file ("fonts/repacker_expected.otf");
  hb_blob_t *expected_blob = hb_face_reference_table (face_expected, HB_TAG ('G','S','U','B'));
  fprintf(stderr, "expected %d bytes, actual %d bytes\n", hb_blob_get_length(expected_blob), hb_blob_get_length (result));

  if (hb_blob_get_length (expected_blob) != 0 ||
      hb_blob_get_length (result) != 0)
    hb_test_assert_blobs_equal (expected_blob, result);

  hb_face_destroy (face_expected);
  hb_blob_destroy (expected_blob);
  hb_blob_destroy (result);

  for (unsigned i = 0 ; i < 15; i++)
  {
    if (hb_objs[i].real_links != NULL)
      free (hb_objs[i].real_links);
  }

  free (hb_objs);
}


int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  hb_test_add (test_hb_repack_with_cy_struct);

  return hb_test_run();
}
#else
int main (int argc HB_UNUSED, char **argv HB_UNUSED)
{
  return 0;
}
#endif
