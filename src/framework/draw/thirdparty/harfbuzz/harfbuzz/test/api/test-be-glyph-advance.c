/*
 * Copyright © 2022  Behdad Esfahbod
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

static void
test_maxp_and_hmtx (void)
{
  hb_face_t *face;
  hb_font_t *font;

  const char maxp_data[] = "\x00\x00\x50\x00" // version
			   "\x00\x05" // numGlyphs
			   ;
  const char loca_data[18] = "";
  const char hhea_data[36] =
    "\x00\x01\x00\x00" /* FixedVersion<>version;	 * 0x00010000u for version 1.0. */
    "\x02\x00" /* FWORD		ascender;	 * Typographic ascent. */
    "\x00\x10" /* FWORD		descender;	 * Typographic descent. */
    "\x00\x00" /* FWORD		lineGap;	 * Typographic line gap. */
    "\x00\x00" /* UFWORD	advanceMax;	 * Maximum advance width/height value in metrics table. */
    "\x00\x00" /* FWORD		minLeadingBearing;  * Minimum left/top sidebearing value in metrics table. */
    "\x00\x00" /* FWORD		minTrailingBearing;  * Minimum right/bottom sidebearing value; */
    "\x01\x00" /* FWORD		maxExtent;	 * horizontal: Max(lsb + (xMax - xMin)), */
    "\x00\x00" /* HBINT16	caretSlopeRise;	 * Used to calculate the slope of the,*/
    "\x00\x00" /* HBINT16	caretSlopeRun;	 * 0 for vertical caret, 1 for horizontal. */
    "\x00\x00" /* HBINT16	caretOffset;	 * The amount by which a slanted */
    "\x00\x00" /* HBINT16	reserved1;	 * Set to 0. */
    "\x00\x00" /* HBINT16	reserved2;	 * Set to 0. */
    "\x00\x00" /* HBINT16	reserved3;	 * Set to 0. */
    "\x00\x00" /* HBINT16	reserved4;	 * Set to 0. */
    "\x00\x00" /* HBINT16	metricDataFormat; * 0 for current format. */
    "\x00\x02" /* HBUINT16	numberOfLongMetrics;  * Number of LongMetric entries in metric table. */
    ;
  const char hmtx_data[18] =
    "\x00\x01\x00\x02"	/* glyph 0 advance lsb */
    "\x00\x03\x00\x04"	/* glyph 1 advance lsb */
    "\x00\x05"		/* glyph 2         lsb */
    "\x00\x06"		/* glyph 3         lsb */
    "\x00\x07"		/* glyph 4         lsb */
    "\x00\x08"		/* glyph 5 advance */
    "\x00\x09"		/* glyph 6 advance */
    ;

  face = hb_face_builder_create ();
  HB_FACE_ADD_TABLE (face, "maxp", maxp_data);
  HB_FACE_ADD_TABLE (face, "loca", loca_data);
  HB_FACE_ADD_TABLE (face, "hhea", hhea_data);
  HB_FACE_ADD_TABLE (face, "hmtx", hmtx_data);
  font = hb_font_create (face);
  hb_face_destroy (face);
  g_assert_cmpuint (hb_font_get_glyph_h_advance (font, 0), ==, 1);
  g_assert_cmpuint (hb_font_get_glyph_h_advance (font, 1), ==, 3);
  g_assert_cmpuint (hb_font_get_glyph_h_advance (font, 2), ==, 3);
  g_assert_cmpuint (hb_font_get_glyph_h_advance (font, 3), ==, 3);
  g_assert_cmpuint (hb_font_get_glyph_h_advance (font, 4), ==, 3);
#ifndef HB_NO_BEYOND_64K
  g_assert_cmpuint (hb_font_get_glyph_h_advance (font, 5), ==, 8);
  g_assert_cmpuint (hb_font_get_glyph_h_advance (font, 6), ==, 9);
  g_assert_cmpuint (hb_font_get_glyph_h_advance (font, 7), ==, 9);
#endif
  g_assert_cmpuint (hb_font_get_glyph_h_advance (font, 8), ==, 0);
  g_assert_cmpuint (hb_font_get_glyph_h_advance (font, 9), ==, 0);
  g_assert_cmpuint (hb_font_get_glyph_h_advance (font,10), ==, 0);
  g_assert_cmpuint (hb_font_get_glyph_h_advance (font,11), ==, 0);
  hb_font_destroy (font);
}


int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  hb_test_add (test_maxp_and_hmtx);

  return hb_test_run();
}
