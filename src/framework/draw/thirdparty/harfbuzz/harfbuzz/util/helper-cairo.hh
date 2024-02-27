/*
 * Copyright © 2011  Google, Inc.
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
 * Google Author(s): Behdad Esfahbod
 */

#ifndef HELPER_CAIRO_HH
#define HELPER_CAIRO_HH

#include "view-options.hh"
#include "output-options.hh"
#ifdef HAVE_CAIRO_FT
#  include "helper-cairo-ft.hh"
#endif

#include <cairo.h>
#include <hb.h>
#include <hb-cairo.h>

#include "helper-cairo-ansi.hh"
#ifdef CAIRO_HAS_SVG_SURFACE
#  include <cairo-svg.h>
#endif
#ifdef CAIRO_HAS_PDF_SURFACE
#  include <cairo-pdf.h>
#endif
#ifdef CAIRO_HAS_PS_SURFACE
#  include <cairo-ps.h>
#  if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1,6,0)
#    define HAS_EPS 1

static cairo_surface_t *
_cairo_eps_surface_create_for_stream (cairo_write_func_t  write_func,
				      void               *closure,
				      double              width,
				      double              height)
{
  cairo_surface_t *surface;

  surface = cairo_ps_surface_create_for_stream (write_func, closure, width, height);
  cairo_ps_surface_set_eps (surface, true);

  return surface;
}

#  else
#    undef HAS_EPS
#  endif
#endif

static inline bool
helper_cairo_use_hb_draw (const font_options_t *font_opts)
{
  const char *env = getenv ("HB_DRAW");
  if (!env)
    /* Older cairo had a bug in rendering COLRv0 fonts in
     * right-to-left direction as well as clipping issue
     * with user-fonts.
     *
     * https://github.com/harfbuzz/harfbuzz/issues/4051 */
    return cairo_version () >= CAIRO_VERSION_ENCODE (1, 17, 5);

  return atoi (env);
}

static inline cairo_scaled_font_t *
helper_cairo_create_scaled_font (const font_options_t *font_opts,
				 const view_options_t *view_opts)
{
  hb_font_t *font = font_opts->font;
  bool use_hb_draw = true;

#ifdef HAVE_CAIRO_FT
  use_hb_draw = helper_cairo_use_hb_draw (font_opts);
#endif


  cairo_font_face_t *cairo_face = nullptr;
  if (use_hb_draw)
  {
    cairo_face = hb_cairo_font_face_create_for_font (font);
    hb_cairo_font_face_set_scale_factor (cairo_face, 1 << font_opts->subpixel_bits);
  }
#ifdef HAVE_CAIRO_FT
  else
    cairo_face = helper_cairo_create_ft_font_face (font_opts);
#endif

  cairo_matrix_t ctm, font_matrix;
  cairo_font_options_t *font_options;

  cairo_matrix_init_identity (&ctm);
  cairo_matrix_init_scale (&font_matrix,
			   font_opts->font_size_x,
			   font_opts->font_size_y);
  if (!use_hb_draw)
    font_matrix.xy = -font_opts->slant * font_opts->font_size_x;

  font_options = cairo_font_options_create ();
  cairo_font_options_set_hint_style (font_options, CAIRO_HINT_STYLE_NONE);
  cairo_font_options_set_hint_metrics (font_options, CAIRO_HINT_METRICS_OFF);
#ifdef CAIRO_COLOR_PALETTE_DEFAULT
  cairo_font_options_set_color_palette (font_options, view_opts->palette);
#endif
#ifdef HAVE_CAIRO_FONT_OPTIONS_GET_CUSTOM_PALETTE_COLOR
  if (view_opts->custom_palette)
  {
    char **entries = g_strsplit (view_opts->custom_palette, ",", -1);
    unsigned idx = 0;
    for (unsigned i = 0; entries[i]; i++)
    {
      const char *p = strchr (entries[i], '=');
      if (!p)
        p = entries[i];
      else
      {
	sscanf (entries[i], "%u", &idx);
        p++;
      }

      unsigned fr, fg, fb, fa;
      fr = fg = fb = fa = 0;
      if (parse_color (p, fr, fg,fb, fa))
	cairo_font_options_set_custom_palette_color (font_options, idx, fr / 255., fg / 255., fb / 255., fa / 255.);

      idx++;
    }
    g_strfreev (entries);
  }
#endif

  cairo_scaled_font_t *scaled_font = cairo_scaled_font_create (cairo_face,
							       &font_matrix,
							       &ctm,
							       font_options);

  cairo_font_options_destroy (font_options);
  cairo_font_face_destroy (cairo_face);

  return scaled_font;
}

static inline bool
helper_cairo_scaled_font_has_color (cairo_scaled_font_t *scaled_font)
{
  hb_font_t *font = hb_cairo_font_face_get_font (cairo_scaled_font_get_font_face (scaled_font));

#ifdef HAVE_CAIRO_FT
  if (!font)
    return helper_cairo_ft_scaled_font_has_color (scaled_font);
#endif

  hb_face_t *face = hb_font_get_face (font);

  return hb_ot_color_has_png (face) ||
         hb_ot_color_has_layers (face) ||
         hb_ot_color_has_paint (face);
}


enum class image_protocol_t {
  NONE = 0,
  ITERM2,
  KITTY,
};

struct finalize_closure_t {
  void (*callback)(finalize_closure_t *);
  cairo_surface_t *surface;
  cairo_write_func_t write_func;
  void *closure;
  image_protocol_t protocol;
};
static cairo_user_data_key_t finalize_closure_key;


static void
finalize_ansi (finalize_closure_t *closure)
{
  cairo_status_t status;
  status = helper_cairo_surface_write_to_ansi_stream (closure->surface,
						      closure->write_func,
						      closure->closure);
  if (status != CAIRO_STATUS_SUCCESS)
    fail (false, "Failed to write output: %s",
	  cairo_status_to_string (status));
}

static cairo_surface_t *
_cairo_ansi_surface_create_for_stream (cairo_write_func_t write_func,
				       void *closure,
				       double width,
				       double height,
				       cairo_content_t content,
				       image_protocol_t protocol HB_UNUSED)
{
  cairo_surface_t *surface;
  int w = ceil (width);
  int h = ceil (height);

  switch (content) {
    case CAIRO_CONTENT_ALPHA:
      surface = cairo_image_surface_create (CAIRO_FORMAT_A8, w, h);
      break;
    default:
    case CAIRO_CONTENT_COLOR:
      surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, w, h);
      break;
    case CAIRO_CONTENT_COLOR_ALPHA:
      surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, w, h);
      break;
  }
  cairo_status_t status = cairo_surface_status (surface);
  if (status != CAIRO_STATUS_SUCCESS)
    fail (false, "Failed to create cairo surface: %s",
	  cairo_status_to_string (status));

  finalize_closure_t *ansi_closure = g_new0 (finalize_closure_t, 1);
  ansi_closure->callback = finalize_ansi;
  ansi_closure->surface = surface;
  ansi_closure->write_func = write_func;
  ansi_closure->closure = closure;

  if (cairo_surface_set_user_data (surface,
				   &finalize_closure_key,
				   (void *) ansi_closure,
				   (cairo_destroy_func_t) g_free))
    g_free ((void *) closure);

  return surface;
}


#ifdef CAIRO_HAS_PNG_FUNCTIONS

static cairo_status_t
byte_array_write_func (void                *closure,
		       const unsigned char *data,
		       unsigned int         size)
{
  g_byte_array_append ((GByteArray *) closure, data, size);
  return CAIRO_STATUS_SUCCESS;
}

static void
finalize_png (finalize_closure_t *closure)
{
  cairo_status_t status;
  GByteArray *bytes = nullptr;
  GString *string;
  gchar *base64;
  size_t base64_len;

  if (closure->protocol == image_protocol_t::NONE)
  {
    status = cairo_surface_write_to_png_stream (closure->surface,
						closure->write_func,
						closure->closure);
  }
  else
  {
    bytes = g_byte_array_new ();
    status = cairo_surface_write_to_png_stream (closure->surface,
						byte_array_write_func,
						bytes);
  }

  if (status != CAIRO_STATUS_SUCCESS)
    fail (false, "Failed to write output: %s",
	  cairo_status_to_string (status));

  if (closure->protocol == image_protocol_t::NONE)
    return;

  base64 = g_base64_encode (bytes->data, bytes->len);
  base64_len = strlen (base64);

  string = g_string_new (NULL);
  if (closure->protocol == image_protocol_t::ITERM2)
  {
    /* https://iterm2.com/documentation-images.html */
    g_string_printf (string, "\033]1337;File=inline=1;size=%zu:%s\a\n",
		     base64_len, base64);
  }
  else if (closure->protocol == image_protocol_t::KITTY)
  {
#define CHUNK_SIZE 4096
    /* https://sw.kovidgoyal.net/kitty/graphics-protocol.html */
    for (size_t pos = 0; pos < base64_len; pos += CHUNK_SIZE)
    {
      size_t len = base64_len - pos;

      if (pos == 0)
	g_string_append (string, "\033_Ga=T,f=100,m=");
      else
	g_string_append (string, "\033_Gm=");

      if (len > CHUNK_SIZE)
      {
	g_string_append (string, "1;");
	g_string_append_len (string, base64 + pos, CHUNK_SIZE);
      }
      else
      {
	g_string_append (string, "0;");
	g_string_append_len (string, base64 + pos, len);
      }

      g_string_append (string, "\033\\");
    }
    g_string_append (string, "\n");
#undef CHUNK_SIZE
  }

  closure->write_func (closure->closure, (unsigned char *) string->str, string->len);

  g_byte_array_unref (bytes);
  g_free (base64);
  g_string_free (string, TRUE);
}

static cairo_surface_t *
_cairo_png_surface_create_for_stream (cairo_write_func_t write_func,
				      void *closure,
				      double width,
				      double height,
				      cairo_content_t content,
				      image_protocol_t protocol)
{
  cairo_surface_t *surface;
  int w = ceil (width);
  int h = ceil (height);

  switch (content) {
    case CAIRO_CONTENT_ALPHA:
      surface = cairo_image_surface_create (CAIRO_FORMAT_A8, w, h);
      break;
    default:
    case CAIRO_CONTENT_COLOR:
      surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, w, h);
      break;
    case CAIRO_CONTENT_COLOR_ALPHA:
      surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, w, h);
      break;
  }
  cairo_status_t status = cairo_surface_status (surface);
  if (status != CAIRO_STATUS_SUCCESS)
    fail (false, "Failed to create cairo surface: %s",
	  cairo_status_to_string (status));

  finalize_closure_t *png_closure = g_new0 (finalize_closure_t, 1);
  png_closure->callback = finalize_png;
  png_closure->surface = surface;
  png_closure->write_func = write_func;
  png_closure->closure = closure;
  png_closure->protocol = protocol;

  if (cairo_surface_set_user_data (surface,
				   &finalize_closure_key,
				   (void *) png_closure,
				   (cairo_destroy_func_t) g_free))
    g_free ((void *) closure);

  return surface;
}

#endif

static cairo_status_t
stdio_write_func (void                *closure,
		  const unsigned char *data,
		  unsigned int         size)
{
  FILE *fp = (FILE *) closure;

  while (size) {
    size_t ret = fwrite (data, 1, size, fp);
    size -= ret;
    data += ret;
    if (size && ferror (fp))
      fail (false, "Failed to write output: %s", strerror (errno));
  }

  return CAIRO_STATUS_SUCCESS;
}

static const char *helper_cairo_supported_formats[] =
{
  "ansi",
  #ifdef CAIRO_HAS_PNG_FUNCTIONS
  "png",
  #endif
  #ifdef CAIRO_HAS_SVG_SURFACE
  "svg",
  #endif
  #ifdef CAIRO_HAS_PDF_SURFACE
  "pdf",
  #endif
  #ifdef CAIRO_HAS_PS_SURFACE
  "ps",
   #ifdef HAS_EPS
    "eps",
   #endif
  #endif
  nullptr
};

template <typename view_options_t,
	 typename output_options_type>
static inline cairo_t *
helper_cairo_create_context (double w, double h,
			     view_options_t *view_opts,
			     output_options_type *out_opts,
			     cairo_content_t content)
{
  cairo_surface_t *(*constructor) (cairo_write_func_t write_func,
				   void *closure,
				   double width,
				   double height) = nullptr;
  cairo_surface_t *(*constructor2) (cairo_write_func_t write_func,
				    void *closure,
				    double width,
				    double height,
				    cairo_content_t content,
				    image_protocol_t protocol) = nullptr;

  image_protocol_t protocol = image_protocol_t::NONE;
  const char *extension = out_opts->output_format;
  if (!extension) {
#if HAVE_ISATTY
    if (isatty (fileno (out_opts->out_fp)))
    {
#ifdef CAIRO_HAS_PNG_FUNCTIONS
      const char *name;
      /* https://gitlab.com/gnachman/iterm2/-/issues/7154 */
      if ((name = getenv ("LC_TERMINAL")) != nullptr &&
	  0 == g_ascii_strcasecmp (name, "iTerm2"))
      {
	extension = "png";
	protocol = image_protocol_t::ITERM2;
      }
      else if ((name = getenv ("TERM_PROGRAM")) != nullptr &&
	  0 == g_ascii_strcasecmp (name, "WezTerm"))
      {
	extension = "png";
	protocol = image_protocol_t::ITERM2;
      }
      else if ((name = getenv ("TERM")) != nullptr &&
	       0 == g_ascii_strcasecmp (name, "xterm-kitty"))
      {
	extension = "png";
	protocol = image_protocol_t::KITTY;
      }
      else
	extension = "ansi";
#else
      extension = "ansi";
#endif
    }
    else
#endif
    {
#ifdef CAIRO_HAS_PNG_FUNCTIONS
      extension = "png";
#else
      extension = "ansi";
#endif
    }
  }
  if (0)
    ;
    else if (0 == g_ascii_strcasecmp (extension, "ansi"))
      constructor2 = _cairo_ansi_surface_create_for_stream;
  #ifdef CAIRO_HAS_PNG_FUNCTIONS
    else if (0 == g_ascii_strcasecmp (extension, "png"))
      constructor2 = _cairo_png_surface_create_for_stream;
  #endif
  #ifdef CAIRO_HAS_SVG_SURFACE
    else if (0 == g_ascii_strcasecmp (extension, "svg"))
      constructor = cairo_svg_surface_create_for_stream;
  #endif
  #ifdef CAIRO_HAS_PDF_SURFACE
    else if (0 == g_ascii_strcasecmp (extension, "pdf"))
      constructor = cairo_pdf_surface_create_for_stream;
  #endif
  #ifdef CAIRO_HAS_PS_SURFACE
    else if (0 == g_ascii_strcasecmp (extension, "ps"))
      constructor = cairo_ps_surface_create_for_stream;
   #ifdef HAS_EPS
    else if (0 == g_ascii_strcasecmp (extension, "eps"))
      constructor = _cairo_eps_surface_create_for_stream;
   #endif
  #endif


  unsigned int fr, fg, fb, fa, br, bg, bb, ba;
  const char *color;
  br = bg = bb = ba = 255;
  color = view_opts->back ? view_opts->back : DEFAULT_BACK;
  parse_color (color, br, bg, bb, ba);
  fr = fg = fb = 0; fa = 255;
  color = view_opts->fore ? view_opts->fore : DEFAULT_FORE;
  parse_color (color, fr, fg, fb, fa);

  if (content == CAIRO_CONTENT_ALPHA)
  {
    if (view_opts->show_extents ||
	br != bg || bg != bb ||
	fr != fg || fg != fb)
      content = CAIRO_CONTENT_COLOR;
  }
  if (ba != 255)
    content = CAIRO_CONTENT_COLOR_ALPHA;

  cairo_surface_t *surface;
  FILE *f = out_opts->out_fp;
  if (constructor)
    surface = constructor (stdio_write_func, f, w, h);
  else if (constructor2)
    surface = constructor2 (stdio_write_func, f, w, h, content, protocol);
  else
    fail (false, "Unknown output format `%s'; supported formats are: %s%s",
	  extension,
	  g_strjoinv ("/", const_cast<char**> (helper_cairo_supported_formats)),
	  out_opts->explicit_output_format ? "" :
	  "\nTry setting format using --output-format");

  cairo_t *cr = cairo_create (surface);
  content = cairo_surface_get_content (surface);

  switch (content) {
    case CAIRO_CONTENT_ALPHA:
      cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
      cairo_set_source_rgba (cr, 1., 1., 1., br / 255.);
      cairo_paint (cr);
      cairo_set_source_rgba (cr, 1., 1., 1.,
			     (fr / 255.) * (fa / 255.) + (br / 255) * (1 - (fa / 255.)));
      break;
    default:
    case CAIRO_CONTENT_COLOR:
    case CAIRO_CONTENT_COLOR_ALPHA:
      cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
      cairo_set_source_rgba (cr, br / 255., bg / 255., bb / 255., ba / 255.);
      cairo_paint (cr);
      cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
      cairo_set_source_rgba (cr, fr / 255., fg / 255., fb / 255., fa / 255.);
      break;
  }

  cairo_surface_destroy (surface);
  return cr;
}

static inline void
helper_cairo_destroy_context (cairo_t *cr)
{
  finalize_closure_t *closure = (finalize_closure_t *)
				cairo_surface_get_user_data (cairo_get_target (cr),
							     &finalize_closure_key);
  if (closure)
    closure->callback (closure);

  cairo_status_t status = cairo_status (cr);
  if (status != CAIRO_STATUS_SUCCESS)
    fail (false, "Failed: %s",
	  cairo_status_to_string (status));
  cairo_destroy (cr);
}


struct helper_cairo_line_t {
  cairo_glyph_t *glyphs = nullptr;
  unsigned int num_glyphs = 0;
  char *utf8 = nullptr;
  unsigned int utf8_len = 0;
  cairo_text_cluster_t *clusters = nullptr;
  unsigned int num_clusters = 0;
  cairo_text_cluster_flags_t cluster_flags = (cairo_text_cluster_flags_t) 0;

  helper_cairo_line_t (const char          *utf8_,
		       unsigned             utf8_len_,
		       hb_buffer_t         *buffer,
		       hb_bool_t            utf8_clusters,
		       unsigned             subpixel_bits) :
    utf8 (utf8_ ? g_strndup (utf8_, utf8_len_) : nullptr),
    utf8_len (utf8_len_)
  {
    hb_cairo_glyphs_from_buffer (buffer,
				 utf8_clusters,
				 1 << subpixel_bits, 1 << subpixel_bits,
				 0., 0.,
				 utf8, utf8_len,
				 &glyphs, &num_glyphs,
				 &clusters, &num_clusters,
				 &cluster_flags);
  }

  void finish ()
  {
    if (glyphs)
      cairo_glyph_free (glyphs);
    if (clusters)
      cairo_text_cluster_free (clusters);
    g_free (utf8);
  }

  void get_advance (double *x_advance, double *y_advance)
  {
    *x_advance = glyphs[num_glyphs].x;
    *y_advance = glyphs[num_glyphs].y;
  }
};

#endif
