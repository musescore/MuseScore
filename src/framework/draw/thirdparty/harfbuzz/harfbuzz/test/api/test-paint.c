/*
 * Copyright © 2022 Matthias Clasen
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

#include <hb-features.h>
#include <hb-ot.h>

#ifdef HB_HAS_FREETYPE
#include <hb-ft.h>

#if (FREETYPE_MAJOR*10000 + FREETYPE_MINOR*100 + FREETYPE_PATCH) >= 21300
#include FT_COLOR_H
#endif
#endif

static inline hb_bool_t
have_ft_colrv1 (void)
{
#if (FREETYPE_MAJOR*10000 + FREETYPE_MINOR*100 + FREETYPE_PATCH) >= 21300
  return TRUE;
#else
  return FALSE;
#endif
}

/* Unit tests for hb-paint.h */

/* ---- */

typedef struct {
  int level;
  GString *string;
} paint_data_t;

static void print (paint_data_t *data, const char *format, ...) G_GNUC_PRINTF (2, 3);

static void
print (paint_data_t *data,
       const char *format,
       ...)
{
  va_list args;

  g_string_append_printf (data->string, "%*s", 2 * data->level, "");

  va_start (args, format);
  g_string_append_vprintf (data->string, format, args);
  va_end (args);

  g_string_append (data->string, "\n");
}

static void
push_transform (hb_paint_funcs_t *funcs,
                void *paint_data,
                float xx, float yx,
                float xy, float yy,
                float dx, float dy,
                void *user_data)
{
  paint_data_t *data = paint_data;

  print (data, "start transform %.3g %.3g %.3g %.3g %.3g %.3g", xx, yx, xy, yy, dx, dy);
  data->level++;
}

static void
pop_transform (hb_paint_funcs_t *funcs,
               void *paint_data,
               void *user_data)
{
  paint_data_t *data = paint_data;

  data->level--;
  print (data, "end transform");
}

static void
push_clip_glyph (hb_paint_funcs_t *funcs,
                 void *paint_data,
                 hb_codepoint_t glyph,
                 hb_font_t *font,
                 void *user_data)
{
  paint_data_t *data = paint_data;

  print (data, "start clip glyph %u", glyph);
  data->level++;
}

static void
push_clip_rectangle (hb_paint_funcs_t *funcs,
                     void *paint_data,
                     float xmin, float ymin, float xmax, float ymax,
                     void *user_data)
{
  paint_data_t *data = paint_data;

  print (data, "start clip rectangle %.3g %.3g %.3g %.3g", xmin, ymin, xmax, ymax);
  data->level++;
}

static void
pop_clip (hb_paint_funcs_t *funcs,
          void *paint_data,
          void *user_data)
{
  paint_data_t *data = paint_data;

  data->level--;
  print (data, "end clip");
}

static void
paint_color (hb_paint_funcs_t *funcs,
             void *paint_data,
             hb_bool_t use_foreground,
             hb_color_t color,
             void *user_data)
{
  paint_data_t *data = paint_data;

  print (data, "solid %d %d %d %d",
         hb_color_get_red (color),
         hb_color_get_green (color),
         hb_color_get_blue (color),
         hb_color_get_alpha (color));
}

static hb_bool_t
paint_image (hb_paint_funcs_t *funcs,
             void *paint_data,
             hb_blob_t *blob,
             unsigned int width,
             unsigned int height,
             hb_tag_t format,
             float slant,
             hb_glyph_extents_t *extents,
             void *user_data)
{
  paint_data_t *data = paint_data;
  char buf[5] = { 0, };

  hb_tag_to_string (format, buf);
  print (data, "image type %s size %u %u slant %.3g extents %d %d %d %d\n",
         buf, width, height, slant,
         extents->x_bearing, extents->y_bearing, extents->width, extents->height);

  return TRUE;
}

static void
print_color_line (paint_data_t *data,
                  hb_color_line_t *color_line)
{
  hb_color_stop_t *stops;
  unsigned int len;

  len = hb_color_line_get_color_stops (color_line, 0, NULL, NULL);
  stops = alloca (len * sizeof (hb_color_stop_t));
  hb_color_line_get_color_stops (color_line, 0, &len, stops);

  print (data, "colors %d", hb_color_line_get_extend (color_line));
  data->level += 1;
  for (unsigned int i = 0; i < len; i++)
    print (data, "%.3g %d %d %d %d",
           stops[i].offset,
           hb_color_get_red (stops[i].color),
           hb_color_get_green (stops[i].color),
           hb_color_get_blue (stops[i].color),
           hb_color_get_alpha (stops[i].color));
  data->level -= 1;
}

static void
paint_linear_gradient (hb_paint_funcs_t *funcs,
                       void *paint_data,
                       hb_color_line_t *color_line,
                       float x0, float y0,
                       float x1, float y1,
                       float x2, float y2,
                       void *user_data)
{
  paint_data_t *data = paint_data;

  print (data, "linear gradient");
  data->level += 1;
  print (data, "p0 %.3g %.3g", x0, y0);
  print (data, "p1 %.3g %.3g", x1, y1);
  print (data, "p2 %.3g %.3g", x2, y2);

  print_color_line (data, color_line);
  data->level -= 1;
}

static void
paint_radial_gradient (hb_paint_funcs_t *funcs,
                       void *paint_data,
                       hb_color_line_t *color_line,
                       float x0, float y0, float r0,
                       float x1, float y1, float r1,
                       void *user_data)
{
  paint_data_t *data = paint_data;

  print (data, "radial gradient");
  data->level += 1;
  print (data, "p0 %.3g %.3g radius %.3g", x0, y0, r0);
  print (data, "p1 %.3g %.3g radius %.3g", x1, y1, r1);

  print_color_line (data, color_line);
  data->level -= 1;
}

static void
paint_sweep_gradient (hb_paint_funcs_t *funcs,
                      void *paint_data,
                      hb_color_line_t *color_line,
                      float cx, float cy,
                      float start_angle,
                      float end_angle,
                      void *user_data)
{
  paint_data_t *data = paint_data;

  print (data, "sweep gradient");
  data->level++;
  print (data, "center %.3g %.3g", cx, cy);
  print (data, "angles %.3g %.3g", start_angle, end_angle);

  print_color_line (data, color_line);
  data->level -= 1;
}

static void
push_group (hb_paint_funcs_t *funcs,
            void *paint_data,
            void *user_data)
{
  paint_data_t *data = paint_data;
  print (data, "push group");
  data->level++;
}

static void
pop_group (hb_paint_funcs_t *funcs,
           void *paint_data,
           hb_paint_composite_mode_t mode,
           void *user_data)
{
  paint_data_t *data = paint_data;
  data->level--;
  print (data, "pop group mode %d", mode);
}

static hb_paint_funcs_t *
get_test_paint_funcs (void)
{
  static hb_paint_funcs_t *funcs = NULL;

  if (!funcs)
  {
    funcs = hb_paint_funcs_create ();

    hb_paint_funcs_set_push_transform_func (funcs, push_transform, NULL, NULL);
    hb_paint_funcs_set_pop_transform_func (funcs, pop_transform, NULL, NULL);
    hb_paint_funcs_set_push_clip_glyph_func (funcs, push_clip_glyph, NULL, NULL);
    hb_paint_funcs_set_push_clip_rectangle_func (funcs, push_clip_rectangle, NULL, NULL);
    hb_paint_funcs_set_pop_clip_func (funcs, pop_clip, NULL, NULL);
    hb_paint_funcs_set_push_group_func (funcs, push_group, NULL, NULL);
    hb_paint_funcs_set_pop_group_func (funcs, pop_group, NULL, NULL);
    hb_paint_funcs_set_color_func (funcs, paint_color, NULL, NULL);
    hb_paint_funcs_set_image_func (funcs, paint_image, NULL, NULL);
    hb_paint_funcs_set_linear_gradient_func (funcs, paint_linear_gradient, NULL, NULL);
    hb_paint_funcs_set_radial_gradient_func (funcs, paint_radial_gradient, NULL, NULL);
    hb_paint_funcs_set_sweep_gradient_func (funcs, paint_sweep_gradient, NULL, NULL);

    hb_paint_funcs_make_immutable (funcs);
  }

  return funcs;
}

typedef struct {
  const char *font_file;
  float slant;
  hb_codepoint_t glyph;
  unsigned int palette;
  const char *output;
} paint_test_t;

#define NOTO_HAND   "fonts/noto_handwriting-cff2_colr_1.otf"
#define TEST_GLYPHS "fonts/test_glyphs-glyf_colr_1.ttf"
#define TEST_GLYPHS_VF "fonts/test_glyphs-glyf_colr_1_variable.ttf"
#define BAD_COLRV1  "fonts/bad_colrv1.ttf"
#define ROCHER_ABC  "fonts/RocherColorGX.abc.ttf"

/* To verify the rendering visually, use
 *
 * hb-view --font-slant SLANT --font-palette PALETTE FONT --glyphs [gidGID=0+1000]
 *
 * where GID is the glyph value of the test.
 */
static paint_test_t paint_tests[] = {
  /* COLRv1 */
  { NOTO_HAND,   0.,  10,   0, "hand-10" },
  { NOTO_HAND,   0.2f,10,   0, "hand-10.2" },

  { TEST_GLYPHS, 0,    6,   0, "test-6" },   // linear gradient
  { TEST_GLYPHS, 0,   10,   0, "test-10" },  // sweep gradient
  { TEST_GLYPHS, 0,   92,   0, "test-92" },  // radial gradient
  { TEST_GLYPHS, 0,  106,   0, "test-106" },
  { TEST_GLYPHS, 0,  116,   0, "test-116" }, // compositing
  { TEST_GLYPHS, 0,  123,   0, "test-123" },
  { TEST_GLYPHS, 0,  154,   0, "test-154" },
  { TEST_GLYPHS, 0,  165,   0, "test-165" }, // linear gradient
  { TEST_GLYPHS, 0,  175,   0, "test-175" }, // layers

  { TEST_GLYPHS_VF, 0,    6,   0, "testvf-6" },
  { TEST_GLYPHS_VF, 0,   10,   0, "testvf-10" },
  { TEST_GLYPHS_VF, 0,   92,   0, "testvf-92" },
  { TEST_GLYPHS_VF, 0,  106,   0, "testvf-106" },
  { TEST_GLYPHS_VF, 0,  116,   0, "testvf-116" },
  { TEST_GLYPHS_VF, 0,  123,   0, "testvf-123" },
  { TEST_GLYPHS_VF, 0,  154,   0, "testvf-154" },
  { TEST_GLYPHS_VF, 0,  165,   0, "testvf-165" },
  { TEST_GLYPHS_VF, 0,  175,   0, "testvf-175" },

  { BAD_COLRV1,  0,  154,   0, "bad-154" },  // recursion

  /* COLRv0 */
  { ROCHER_ABC, 0.3f, 1,   0, "rocher-1" },
  { ROCHER_ABC, 0.3f, 2,   2, "rocher-2" },
  { ROCHER_ABC, 0,    3, 200, "rocher-3" },
};

static void
test_hb_paint (gconstpointer d,
               hb_bool_t     use_ft)
{
  const paint_test_t *test = d;
  hb_face_t *face;
  hb_font_t *font;
  hb_paint_funcs_t *funcs;
  paint_data_t data;
  char *file;
  char *buffer;
  gsize len;
  GError *error = NULL;

  face = hb_test_open_font_file (test->font_file);
  font = hb_font_create (face);

  hb_font_set_synthetic_slant (font, test->slant);

#ifdef HB_HAS_FREETYPE
  if (use_ft)
    hb_ft_font_set_funcs (font);
#endif

  funcs = get_test_paint_funcs ();

  data.string = g_string_new ("");
  data.level = 0;

  hb_font_paint_glyph (font, test->glyph, funcs, &data, 0, HB_COLOR (0, 0, 0, 255));

  /* Run
   *
   * GENERATE_DATA=1 G_TEST_SRCDIR=./test/api ./build/test/api/test-ot-color -p TESTCASE > test/api/results/OUTPUT
   *
   * to produce the expected results file.
   */
  if (getenv ("GENERATE_DATA"))
    {
      g_print ("%s", data.string->str);
      exit (0);
    }

  file = g_test_build_filename (G_TEST_DIST, "results", test->output, NULL);
  if (!g_file_get_contents (file, &buffer, &len, &error))
  {
    g_test_message ("File %s not found.", file);
    g_test_fail ();
    return;
  }

  char **lines = g_strsplit (data.string->str, "\n", 0);
  char **expected;
  if (strstr (buffer, "\r\n"))
    expected = g_strsplit (buffer, "\r\n", 0);
  else
    expected = g_strsplit (buffer, "\n", 0);

  /* Strip initial comments */
  int i;
  for (i = 0; expected[i]; i++)
    {
      if (expected[i][0] != '#')
        {
          if (i > 0)
            {
              char **tmp = g_strdupv (expected + i);
              g_strfreev (expected);
              expected = tmp;
            }
          break;
        }
    }

  if (g_strv_length (lines) != g_strv_length (expected))
  {
    g_test_message ("Unexpected number of lines in output (%d instead of %d):\n%s", g_strv_length (lines), g_strv_length (expected), data.string->str);
    g_test_fail ();
  }
  else
  {
    unsigned int length = g_strv_length (lines);
    for (unsigned int i = 0; i < length; i++)
    {
      if (strcmp (lines[i], expected[i]) != 0)
      {
        int pos;
        for (pos = 0; lines[i][pos]; pos++)
          if (lines[i][pos] != expected[i][pos])
            break;

        g_test_message ("Unexpected output at %d:%d (%c instead of %c):\n%s", i, pos, lines[i][pos], expected[i][pos], data.string->str);
        g_test_fail ();
      }
    }
  }

  g_strfreev (lines);
  g_strfreev (expected);

  g_free (buffer);
  g_free (file);

  g_string_free (data.string, TRUE);

  hb_font_destroy (font);
  hb_face_destroy (face);
}

static void
test_compare_ot_ft (const char *file, hb_codepoint_t glyph)
{
  hb_face_t *face;
  hb_font_t *font;
  hb_paint_funcs_t *funcs;
  GString *ot_str;
  paint_data_t data;

  face = hb_test_open_font_file (file);
  font = hb_font_create (face);

  funcs = get_test_paint_funcs ();

  data.string = g_string_new ("");
  data.level = 0;

  hb_font_paint_glyph (font, glyph, funcs, &data, 0, HB_COLOR (0, 0, 0, 255));

  g_assert_true (data.level == 0);

  ot_str = data.string;

#ifdef HB_HAS_FREETYPE
  hb_ft_font_set_funcs (font);
#endif

  data.string = g_string_new ("");
  data.level = 0;

  hb_font_paint_glyph (font, glyph, funcs, &data, 0, HB_COLOR (0, 0, 0, 255));

  g_assert_true (data.level == 0);

  g_assert_cmpstr (ot_str->str, ==, data.string->str);

  g_string_free (data.string, TRUE);
  hb_font_destroy (font);
  hb_face_destroy (face);

  g_string_free (ot_str, TRUE);
}

static void
test_hb_paint_ot (gconstpointer data)
{
  test_hb_paint (data, 0);
}

static void
test_hb_paint_ft (gconstpointer data)
{
  if (have_ft_colrv1 ())
    test_hb_paint (data, 1);
  else
    g_test_skip ("FreeType COLRv1 support not present");
}

static void
test_compare_ot_ft_novf (gconstpointer d)
{
  if (have_ft_colrv1 ())
    test_compare_ot_ft (TEST_GLYPHS, GPOINTER_TO_UINT (d));
  else
    g_test_skip ("FreeType COLRv1 support not present");
}

static void
test_compare_ot_ft_vf (gconstpointer d)
{
  if (have_ft_colrv1 ())
    test_compare_ot_ft (TEST_GLYPHS_VF, GPOINTER_TO_UINT (d));
  else
    g_test_skip ("FreeType COLRv1 support not present");
}

static void
scrutinize_linear_gradient (hb_paint_funcs_t *funcs,
                            void *paint_data,
                            hb_color_line_t *color_line,
                            float x0, float y0,
                            float x1, float y1,
                            float x2, float y2,
                            void *user_data)
{
  hb_bool_t *result = paint_data;
  hb_color_stop_t *stops;
  unsigned int len;
  hb_color_stop_t *stops2;
  unsigned int len2;

  *result = FALSE;

  len = hb_color_line_get_color_stops (color_line, 0, NULL, NULL);
  if (len == 0)
    return;

  stops = malloc (len * sizeof (hb_color_stop_t));
  stops2 = malloc (len * sizeof (hb_color_stop_t));

  hb_color_line_get_color_stops (color_line, 0, &len, stops);
  hb_color_line_get_color_stops (color_line, 0, &len, stops2);

  // check that we can get stops twice
  if (memcmp (stops, stops2, len * sizeof (hb_color_stop_t)) != 0)
  {
    free (stops);
    free (stops2);
    return;
  }

  // check that we can get a single stop in the middle
  len2 = 1;
  hb_color_line_get_color_stops (color_line, len - 1, &len2, stops2);
  if (memcmp (&stops[len - 1], stops2, sizeof (hb_color_stop_t)) != 0)
  {
    free (stops);
    free (stops2);
    return;
  }

  free (stops);
  free (stops2);

  *result = TRUE;
}

static void
test_color_stops (hb_bool_t use_ft)
{
  hb_face_t *face;
  hb_font_t *font;
  hb_paint_funcs_t *funcs;
  hb_bool_t result = FALSE;

  face = hb_test_open_font_file (NOTO_HAND);
  font = hb_font_create (face);

#ifdef HB_HAS_FREETYPE
  if (use_ft)
    hb_ft_font_set_funcs (font);
#endif

  funcs = hb_paint_funcs_create ();
  hb_paint_funcs_set_linear_gradient_func (funcs, scrutinize_linear_gradient, NULL, NULL);

  hb_font_paint_glyph (font, 10, funcs, &result, 0, HB_COLOR (0, 0, 0, 255));

  g_assert_true (result);

  hb_paint_funcs_destroy (funcs);
  hb_font_destroy (font);
  hb_face_destroy (face);
}

static void
test_color_stops_ot (void)
{
  test_color_stops (0);
}

static void
test_color_stops_ft (void)
{
  if (have_ft_colrv1 ())
    test_color_stops (1);
  else
    g_test_skip ("FreeType COLRv1 support not present");
}

int
main (int argc, char **argv)
{
  int status = 0;

  hb_test_init (&argc, &argv);
  for (unsigned int i = 0; i < G_N_ELEMENTS (paint_tests); i++)
  {
    hb_test_add_data_flavor (&paint_tests[i], paint_tests[i].output, test_hb_paint_ot);
    hb_test_add_data_flavor (&paint_tests[i], paint_tests[i].output, test_hb_paint_ft);
  }

  hb_face_t *face = hb_test_open_font_file (TEST_GLYPHS);
  unsigned glyph_count = hb_face_get_glyph_count (face);
  for (unsigned int i = 1; i < glyph_count; i++)
  {
    char buf[20];
    snprintf (buf, 20, "test-%u", i);
    hb_test_add_data_flavor (GUINT_TO_POINTER (i), buf, test_compare_ot_ft_novf);
    hb_test_add_data_flavor (GUINT_TO_POINTER (i), buf, test_compare_ot_ft_vf);
  }
  hb_face_destroy (face);

  hb_test_add (test_color_stops_ot);
  hb_test_add (test_color_stops_ft);

  status = hb_test_run();

  return status;
}
