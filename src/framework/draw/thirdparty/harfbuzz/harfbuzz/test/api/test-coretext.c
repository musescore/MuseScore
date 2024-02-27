/*
 * Copyright © 2022 Red Hat, Inc.
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
 * Author: Matthias Clasen
 */


#include "hb-test.h"

#include "hb-coretext.h"

CTFontRef
get_ctfont (void)
{
  CTFontCollectionRef collection;
  CFArrayRef ctfaces;
  CTFontDescriptorRef desc;
  CTFontRef ctfont;

  collection = CTFontCollectionCreateFromAvailableFonts (0);
  ctfaces = CTFontCollectionCreateMatchingFontDescriptors (collection);

  desc = CFArrayGetValueAtIndex (ctfaces, 0);
  ctfont = CTFontCreateWithFontDescriptor (desc, 0.0, NULL);

  return ctfont;
}

static void
test_native_coretext_basic (void)
{
  CTFontRef ctfont;
  hb_font_t *font;
  CTFontRef ctfont2;

  ctfont = get_ctfont ();

  g_assert_nonnull (ctfont);

  font = hb_coretext_font_create (ctfont);

  ctfont2 = hb_coretext_font_get_ct_font (font);

  g_assert_true (ctfont2 == ctfont);

  hb_font_destroy (font);

  CFRelease (ctfont);
}

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  hb_test_add (test_native_coretext_basic);

  return hb_test_run ();
}
