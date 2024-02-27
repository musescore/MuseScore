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

#include "hb-ft.h"

#include FT_FONT_FORMATS_H

static FT_Library ft_library;

static void
init_freetype (void)
{
  FT_Error ft_error;
  if ((ft_error = FT_Init_FreeType (&ft_library)))
    abort ();
}

static void
cleanup_freetype (void)
{
  FT_Done_FreeType (ft_library);
}

static FT_Face
get_ft_face (const char *file)
{
  FT_Face ft_face;

#if GLIB_CHECK_VERSION(2,37,2)
  char* path = g_test_build_filename (G_TEST_DIST, file, NULL);
#else
  char* path = g_strdup (file);
#endif

  FT_Error ft_error;
  if ((ft_error = FT_New_Face (ft_library, path, 0, &ft_face))) {
    g_free (path);
    abort();
  }
  g_free (path);

  if ((ft_error = FT_Set_Char_Size (ft_face, 2000, 1000, 0, 0)))
    abort ();

  return ft_face;
}

static void
test_native_ft_basic (void)
{
  FT_Face ft_face;
  hb_font_t *font;
  FT_Face ft_face2;

  init_freetype ();

  ft_face = get_ft_face ("fonts/Cantarell.A.otf");

  g_assert_nonnull (ft_face);
  g_assert_nonnull (FT_Get_Font_Format (ft_face));

  font = hb_ft_font_create_referenced (ft_face);

  ft_face2 = hb_ft_font_get_face (font);

  g_assert_true (ft_face2 == ft_face);

  ft_face2 = hb_ft_font_lock_face (font);

  g_assert_true (ft_face2 == ft_face);

  hb_ft_font_unlock_face (font);

  hb_ft_font_set_load_flags (font, FT_LOAD_NO_SCALE | FT_LOAD_NO_AUTOHINT);
  int load_flags = hb_ft_font_get_load_flags (font);

  g_assert_true (load_flags == (FT_LOAD_NO_SCALE | FT_LOAD_NO_AUTOHINT));

  hb_font_destroy (font);

  FT_Done_Face (ft_face);

  cleanup_freetype ();
}

int
main (int argc, char **argv)
{
  hb_test_init (&argc, &argv);

  hb_test_add (test_native_ft_basic);

  return hb_test_run ();
}
