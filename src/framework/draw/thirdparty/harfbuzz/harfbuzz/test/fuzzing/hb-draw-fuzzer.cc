#include <assert.h>
#include <stdlib.h>

#include <hb-ot.h>

#include "hb-fuzzer.hh"

struct _draw_data_t
{
  unsigned path_len;
  float path_start_x;
  float path_start_y;
  float path_last_x;
  float path_last_y;
};

#include <cstdio>
static void
_move_to (hb_draw_funcs_t *dfuncs, void *draw_data_,
	  hb_draw_state_t *st,
	  float to_x, float to_y,
	  void *user_data)
{
  _draw_data_t *draw_data = (_draw_data_t *) draw_data_;
  assert (!st->path_open);
  draw_data->path_start_x = draw_data->path_last_x = to_x;
  draw_data->path_start_y = draw_data->path_last_y = to_y;
}

static void
_line_to (hb_draw_funcs_t *dfuncs, void *draw_data_,
	  hb_draw_state_t *st,
	  float to_x, float to_y,
	  void *user_data)
{
  _draw_data_t *draw_data = (_draw_data_t *) draw_data_;
  assert (st->path_open);
  ++draw_data->path_len;
  draw_data->path_last_x = to_x;
  draw_data->path_last_y = to_y;
}

static void
_quadratic_to (hb_draw_funcs_t *dfuncs, void *draw_data_,
	       hb_draw_state_t *st,
	       float control_x, float control_y,
	       float to_x, float to_y,
	       void *user_data)
{
  _draw_data_t *draw_data = (_draw_data_t *) draw_data_;
  assert (st->path_open);
  ++draw_data->path_len;
  draw_data->path_last_x = to_x;
  draw_data->path_last_y = to_y;
}

static void
_cubic_to (hb_draw_funcs_t *dfuncs, void *draw_data_,
	   hb_draw_state_t *st,
	   float control1_x, float control1_y,
	   float control2_x, float control2_y,
	   float to_x, float to_y,
	   void *user_data)
{
  _draw_data_t *draw_data = (_draw_data_t *) draw_data_;
  assert (st->path_open);
  ++draw_data->path_len;
  draw_data->path_last_x = to_x;
  draw_data->path_last_y = to_y;
}

static void
_close_path (hb_draw_funcs_t *dfuncs, void *draw_data_,
	     hb_draw_state_t *st,
	     void *user_data)
{
  _draw_data_t *draw_data = (_draw_data_t *) draw_data_;
  assert (st->path_open && draw_data->path_len != 0);
  draw_data->path_len = 0;
  assert (draw_data->path_start_x == draw_data->path_last_x &&
	  draw_data->path_start_y == draw_data->path_last_y);
}

/* Similar to test-ot-face.c's #test_font() */
static void misc_calls_for_gid (hb_face_t *face, hb_font_t *font, hb_set_t *set, hb_codepoint_t cp)
{
  /* Other gid specific misc calls */
  hb_face_collect_variation_unicodes (face, cp, set);

  hb_codepoint_t g;
  hb_font_get_nominal_glyph (font, cp, &g);
  hb_font_get_variation_glyph (font, cp, cp, &g);
  hb_font_get_glyph_h_advance (font, cp);
  hb_font_get_glyph_v_advance (font, cp);
  hb_position_t x, y;
  hb_font_get_glyph_h_origin (font, cp, &x, &y);
  hb_font_get_glyph_v_origin (font, cp, &x, &y);
  hb_font_get_glyph_contour_point (font, cp, 0, &x, &y);
  char buf[64];
  hb_font_get_glyph_name (font, cp, buf, sizeof (buf));

  hb_ot_color_palette_get_name_id (face, cp);
  hb_ot_color_palette_color_get_name_id (face, cp);
  hb_ot_color_palette_get_flags (face, cp);
  hb_ot_color_palette_get_colors (face, cp, 0, nullptr, nullptr);
  hb_ot_color_glyph_get_layers (face, cp, 0, nullptr, nullptr);
  hb_blob_destroy (hb_ot_color_glyph_reference_svg (face, cp));
  hb_blob_destroy (hb_ot_color_glyph_reference_png (font, cp));

  hb_ot_layout_get_ligature_carets (font, HB_DIRECTION_LTR, cp, 0, nullptr, nullptr);

  hb_ot_math_get_glyph_italics_correction (font, cp);
  hb_ot_math_get_glyph_top_accent_attachment (font, cp);
  hb_ot_math_is_glyph_extended_shape (face, cp);
  hb_ot_math_get_glyph_kerning (font, cp, HB_OT_MATH_KERN_BOTTOM_RIGHT, 0);
  hb_ot_math_get_glyph_variants (font, cp, HB_DIRECTION_TTB, 0, nullptr, nullptr);
  hb_ot_math_get_glyph_assembly (font, cp, HB_DIRECTION_BTT, 0, nullptr, nullptr, nullptr);
}

extern "C" int LLVMFuzzerTestOneInput (const uint8_t *data, size_t size)
{
  alloc_state = _fuzzing_alloc_state (data, size);

  hb_blob_t *blob = hb_blob_create ((const char *) data, size,
				    HB_MEMORY_MODE_READONLY, nullptr, nullptr);
  hb_face_t *face = hb_face_create (blob, 0);
  hb_font_t *font = hb_font_create (face);

  unsigned num_coords = 0;
  if (size) num_coords = data[size - 1];
  num_coords = hb_ot_var_get_axis_count (face) > num_coords ? num_coords : hb_ot_var_get_axis_count (face);
  int *coords = (int *) calloc (num_coords, sizeof (int));
  if (size > num_coords + 1)
    for (unsigned i = 0; i < num_coords; ++i)
      coords[i] = ((int) data[size - num_coords + i - 1] - 128) * 10;
  hb_font_set_var_coords_normalized (font, coords, num_coords);
  free (coords);

  unsigned glyph_count = hb_face_get_glyph_count (face);
  glyph_count = glyph_count > 16 ? 16 : glyph_count;

  _draw_data_t draw_data = {0, 0, 0, 0, 0};

  hb_draw_funcs_t *funcs = hb_draw_funcs_create ();
  hb_draw_funcs_set_move_to_func (funcs, (hb_draw_move_to_func_t) _move_to, nullptr, nullptr);
  hb_draw_funcs_set_line_to_func (funcs, (hb_draw_line_to_func_t) _line_to, nullptr, nullptr);
  hb_draw_funcs_set_quadratic_to_func (funcs, (hb_draw_quadratic_to_func_t) _quadratic_to, nullptr, nullptr);
  hb_draw_funcs_set_cubic_to_func (funcs, (hb_draw_cubic_to_func_t) _cubic_to, nullptr, nullptr);
  hb_draw_funcs_set_close_path_func (funcs, (hb_draw_close_path_func_t) _close_path, nullptr, nullptr);
  volatile unsigned counter = !glyph_count;
  hb_set_t *set = hb_set_create ();
  for (unsigned gid = 0; gid < glyph_count; ++gid)
  {
    hb_font_draw_glyph (font, gid, funcs, &draw_data);

    /* Glyph extents also may practices the similar path, call it now that is related */
    hb_glyph_extents_t extents;
    if (hb_font_get_glyph_extents (font, gid, &extents))
      counter += !!extents.width + !!extents.height + !!extents.x_bearing + !!extents.y_bearing;

    if (!counter) counter += 1;

    /* other misc calls */
    misc_calls_for_gid (face, font, set, gid);
  }
  hb_set_destroy (set);
  assert (counter);
  hb_draw_funcs_destroy (funcs);

  hb_font_destroy (font);
  hb_face_destroy (face);
  hb_blob_destroy (blob);
  return 0;
}
