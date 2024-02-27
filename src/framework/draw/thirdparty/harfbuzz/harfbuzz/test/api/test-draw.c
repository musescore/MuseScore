/*
 * Copyright © 2020  Ebrahim Byagowi
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
#include <math.h>

#include <hb.h>
#ifdef HAVE_FREETYPE
#include <hb-ft.h>
#endif

typedef struct draw_data_t
{
  char *str;
  unsigned size;
  unsigned consumed;
} draw_data_t;

/* Our modified itoa, why not using libc's? it is going to be used
   in harfbuzzjs where libc isn't available */
static void _hb_reverse (char *buf, unsigned int len)
{
  unsigned start = 0, end = len - 1;
  while (start < end)
  {
    char c = buf[end];
    buf[end] = buf[start];
    buf[start] = c;
    start++; end--;
  }
}
static unsigned _hb_itoa (float fnum, char *buf)
{
  int32_t num = (int32_t) floorf (fnum + .5f);
  unsigned int i = 0;
  hb_bool_t is_negative = num < 0;
  if (is_negative) num = -num;
  do
  {
    buf[i++] = '0' + num % 10;
    num /= 10;
  } while (num);
  if (is_negative) buf[i++] = '-';
  _hb_reverse (buf, i);
  buf[i] = '\0';
  return i;
}

#define ITOA_BUF_SIZE 12 // 10 digits in int32, 1 for negative sign, 1 for \0

static void
test_itoa (void)
{
  char s[] = "12345";
  _hb_reverse (s, 5);
  g_assert_cmpmem (s, 5, "54321", 5);

  {
    unsigned num = 12345;
    char buf[ITOA_BUF_SIZE];
    unsigned len = _hb_itoa (num, buf);
    g_assert_cmpmem (buf, len, "12345", 5);
  }

  {
    unsigned num = 3152;
    char buf[ITOA_BUF_SIZE];
    unsigned len = _hb_itoa (num, buf);
    g_assert_cmpmem (buf, len, "3152", 4);
  }

  {
    int num = -6457;
    char buf[ITOA_BUF_SIZE];
    unsigned len = _hb_itoa (num, buf);
    g_assert_cmpmem (buf, len, "-6457", 5);
  }
}

static void
move_to (hb_draw_funcs_t *dfuncs, draw_data_t *draw_data,
	 hb_draw_state_t *st,
	 float to_x, float to_y,
	 void *user_data)
{
  /* 4 = command character space + comma + array starts with 0 index + nul character space */
  if (draw_data->consumed + 2 * ITOA_BUF_SIZE + 4 > draw_data->size) return;
  draw_data->str[draw_data->consumed++] = 'M';
  draw_data->consumed += _hb_itoa (to_x, draw_data->str + draw_data->consumed);
  draw_data->str[draw_data->consumed++] = ',';
  draw_data->consumed += _hb_itoa (to_y, draw_data->str + draw_data->consumed);
}

static void
line_to (hb_draw_funcs_t *dfuncs, draw_data_t *draw_data,
	 hb_draw_state_t *st,
	 float to_x, float to_y,
	 void *user_data)
{
  if (draw_data->consumed + 2 * ITOA_BUF_SIZE + 4 > draw_data->size) return;
  draw_data->str[draw_data->consumed++] = 'L';
  draw_data->consumed += _hb_itoa (to_x, draw_data->str + draw_data->consumed);
  draw_data->str[draw_data->consumed++] = ',';
  draw_data->consumed += _hb_itoa (to_y, draw_data->str + draw_data->consumed);
}

static void
quadratic_to (hb_draw_funcs_t *dfuncs, draw_data_t *draw_data,
	      hb_draw_state_t *st,
	      float control_x, float control_y,
	      float to_x, float to_y,
	      void *user_data)
{

  if (draw_data->consumed + 4 * ITOA_BUF_SIZE + 6 > draw_data->size) return;
  draw_data->str[draw_data->consumed++] = 'Q';
  draw_data->consumed += _hb_itoa (control_x, draw_data->str + draw_data->consumed);
  draw_data->str[draw_data->consumed++] = ',';
  draw_data->consumed += _hb_itoa (control_y, draw_data->str + draw_data->consumed);
  draw_data->str[draw_data->consumed++] = ' ';
  draw_data->consumed += _hb_itoa (to_x, draw_data->str + draw_data->consumed);
  draw_data->str[draw_data->consumed++] = ',';
  draw_data->consumed += _hb_itoa (to_y, draw_data->str + draw_data->consumed);
}

static void
cubic_to (hb_draw_funcs_t *dfuncs, draw_data_t *draw_data,
	  hb_draw_state_t *st,
	  float control1_x, float control1_y,
	  float control2_x, float control2_y,
	  float to_x, float to_y,
	  void *user_data)
{
  if (draw_data->consumed + 6 * ITOA_BUF_SIZE + 8 > draw_data->size) return;
  draw_data->str[draw_data->consumed++] = 'C';
  draw_data->consumed += _hb_itoa (control1_x, draw_data->str + draw_data->consumed);
  draw_data->str[draw_data->consumed++] = ',';
  draw_data->consumed += _hb_itoa (control1_y, draw_data->str + draw_data->consumed);
  draw_data->str[draw_data->consumed++] = ' ';
  draw_data->consumed += _hb_itoa (control2_x, draw_data->str + draw_data->consumed);
  draw_data->str[draw_data->consumed++] = ',';
  draw_data->consumed += _hb_itoa (control2_y, draw_data->str + draw_data->consumed);
  draw_data->str[draw_data->consumed++] = ' ';
  draw_data->consumed += _hb_itoa (to_x, draw_data->str + draw_data->consumed);
  draw_data->str[draw_data->consumed++] = ',';
  draw_data->consumed += _hb_itoa (to_y, draw_data->str + draw_data->consumed);
}

static void
close_path (hb_draw_funcs_t *dfuncs, draw_data_t *draw_data,
	    hb_draw_state_t *st,
	    void *user_data)
{
  if (draw_data->consumed + 2 > draw_data->size) return;
  draw_data->str[draw_data->consumed++] = 'Z';
}

static hb_draw_funcs_t *funcs;
static hb_draw_funcs_t *funcs2; /* this one translates quadratic calls to cubic ones */

static void
test_hb_draw_empty (void)
{
  hb_font_draw_glyph (hb_font_get_empty (), 3, funcs, NULL);
}

static void
test_hb_draw_glyf (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/SourceSerifVariable-Roman-VVAR.abc.ttf");
  hb_font_t *font = hb_font_create (face);
  hb_face_destroy (face);

  char str[1024];
  draw_data_t draw_data = {
    .str = str,
    .size = sizeof (str),
    .consumed = 0
  };

  draw_data.consumed = 0;
  hb_font_draw_glyph (font, 4, funcs, &draw_data);

  draw_data.consumed = 0;
  hb_font_draw_glyph (font, 3, funcs, &draw_data);
  char expected[] = "M275,442Q232,442 198,420Q164,397 145,353Q126,309 126,245"
		    "Q126,182 147,139Q167,95 204,73Q240,50 287,50Q330,50 367,70"
		    "Q404,90 427,128L451,116Q431,54 384,21Q336,-13 266,-13"
		    "Q198,-13 148,18Q97,48 70,104Q43,160 43,236Q43,314 76,371"
		    "Q108,427 160,457Q212,487 272,487Q316,487 354,470Q392,453 417,424"
		    "Q442,395 448,358Q441,321 403,321Q378,321 367,334"
		    "Q355,347 350,366L325,454L371,417Q346,430 321,436Q296,442 275,442Z";
  g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);

  /* Test translating quadratic calls to cubic by a _draw_funcs_t that doesn't set the callback */
  draw_data.consumed = 0;
  hb_font_draw_glyph (font, 3, funcs2, &draw_data);
  char expected2[] = "M275,442C246,442 221,435 198,420C175,405 158,382 145,353"
		     "C132,324 126,288 126,245C126,203 133,168 147,139C160,110 179,88 204,73"
		     "C228,58 256,50 287,50C316,50 342,57 367,70C392,83 412,103 427,128"
		     "L451,116C438,75 415,43 384,21C352,-2 313,-13 266,-13C221,-13 181,-3 148,18"
		     "C114,38 88,67 70,104C52,141 43,185 43,236C43,288 54,333 76,371"
		     "C97,408 125,437 160,457C195,477 232,487 272,487C301,487 329,481 354,470"
		     "C379,459 400,443 417,424C434,405 444,383 448,358C443,333 428,321 403,321"
		     "C386,321 374,325 367,334C359,343 353,353 350,366L325,454L371,417"
		     "C354,426 338,432 321,436C304,440 289,442 275,442Z";
  g_assert_cmpmem (str, draw_data.consumed, expected2, sizeof (expected2) - 1);

  hb_variation_t var;
  var.tag = HB_TAG ('w','g','h','t');
  var.value = 800;
  hb_font_set_variations (font, &var, 1);

  draw_data.consumed = 0;
  hb_font_draw_glyph (font, 3, funcs, &draw_data);
  char expected3[] = "M323,448Q297,448 271,430Q244,412 226,371Q209,330 209,261"
		     "Q209,204 225,166Q242,127 272,107Q303,86 344,86Q378,86 404,101"
		     "Q430,115 451,137L488,103Q458,42 404,13Q350,-16 279,-16"
		     "Q211,-16 153,13Q95,41 60,98Q25,156 25,241Q25,323 62,382"
		     "Q99,440 163,470Q226,501 303,501Q357,501 399,480Q440,460 464,426"
		     "Q488,392 492,352Q475,297 420,297Q390,297 366,319Q342,342 339,401"
		     "L333,469L411,427Q387,438 367,443Q348,448 323,448Z";
  g_assert_cmpmem (str, draw_data.consumed, expected3, sizeof (expected3) - 1);

  hb_font_destroy (font);
}

static void
test_hb_draw_cff1 (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/cff1_seac.otf");
  hb_font_t *font = hb_font_create (face);
  hb_face_destroy (face);

  char str[1024];
  draw_data_t draw_data = {
    .str = str,
    .size = sizeof (str),
    .consumed = 0
  };
  hb_font_draw_glyph (font, 3, funcs, &draw_data);
  char expected[] = "M203,367C227,440 248,512 268,588L272,588C293,512 314,440 338,367L369,267L172,267L203,367Z"
		    "M3,0L88,0L151,200L390,200L452,0L541,0L319,656L225,656L3,0Z"
		    "M300,653L342,694L201,861L143,806L300,653Z";
  g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);

  hb_font_destroy (font);
}

static void
test_hb_draw_cff1_rline (void)
{
  /* https://github.com/harfbuzz/harfbuzz/pull/2053 */
  hb_face_t *face = hb_test_open_font_file ("fonts/RanaKufi-Regular.subset.otf");
  hb_font_t *font = hb_font_create (face);
  hb_face_destroy (face);

  char str[1024];
  draw_data_t draw_data = {
    .str = str,
    .size = sizeof (str),
    .consumed = 0
  };
  hb_font_draw_glyph (font, 1, funcs, &draw_data);
  char expected[] = "M775,400C705,400 650,343 650,274L650,250L391,250L713,572L392,893"
		    "L287,1000C311,942 296,869 250,823C250,823 286,858 321,823L571,572"
		    "L150,150L750,150L750,276C750,289 761,300 775,300C789,300 800,289 800,276"
		    "L800,100L150,100C100,100 100,150 100,150C100,85 58,23 0,0L900,0L900,274"
		    "C900,343 844,400 775,400Z";
  g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);

  hb_font_destroy (font);
}

static void
test_hb_draw_cff2 (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/AdobeVFPrototype.abc.otf");
  hb_font_t *font = hb_font_create (face);
  hb_face_destroy (face);

  char str[1024];
  draw_data_t draw_data = {
    .str = str,
    .size = sizeof (str)
  };

  draw_data.consumed = 0;
  hb_font_draw_glyph (font, 3, funcs, &draw_data);
  char expected[] = "M275,442C303,442 337,435 371,417L325,454L350,366"
		    "C357,341 370,321 403,321C428,321 443,333 448,358"
		    "C435,432 361,487 272,487C153,487 43,393 43,236"
		    "C43,83 129,-13 266,-13C360,-13 424,33 451,116L427,128"
		    "C396,78 345,50 287,50C193,50 126,119 126,245C126,373 188,442 275,442Z";
  g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);

  hb_variation_t var;
  var.tag = HB_TAG ('w','g','h','t');
  var.value = 800;
  hb_font_set_variations (font, &var, 1);

  draw_data.consumed = 0;
  hb_font_draw_glyph (font, 3, funcs, &draw_data);
  char expected2[] = "M323,448C356,448 380,441 411,427L333,469L339,401"
		     "C343,322 379,297 420,297C458,297 480,314 492,352"
		     "C486,433 412,501 303,501C148,501 25,406 25,241"
		     "C25,70 143,-16 279,-16C374,-16 447,22 488,103L451,137"
		     "C423,107 390,86 344,86C262,86 209,148 209,261C209,398 271,448 323,448Z";
  g_assert_cmpmem (str, draw_data.consumed, expected2, sizeof (expected2) - 1);

  hb_font_destroy (font);
}

static void
test_hb_draw_ttf_parser_tests (void)
{
  /* https://github.com/RazrFalcon/ttf-parser/blob/337e7d1c/tests/tests.rs#L50-L133 */
  char str[1024];
  draw_data_t draw_data = {
    .str = str,
    .size = sizeof (str)
  };
  {
    hb_face_t *face = hb_test_open_font_file ("fonts/glyphs.ttf");
    hb_font_t *font = hb_font_create (face);
    hb_face_destroy (face);
    {
      draw_data.consumed = 0;
      hb_font_draw_glyph (font, 0, funcs, &draw_data);
      char expected[] = "M50,0L50,750L450,750L450,0L50,0Z";
      g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);
    }
    {
      draw_data.consumed = 0;
      hb_font_draw_glyph (font, 1, funcs, &draw_data);
      char expected[] = "M56,416L56,487L514,487L514,416L56,416ZM56,217L56,288L514,288L514,217L56,217Z";
      g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);
    }
    {
      draw_data.consumed = 0;
      hb_font_draw_glyph (font, 4, funcs, &draw_data);
      char expected[] = "M332,468L197,468L197,0L109,0L109,468L15,468L15,509L109,539"
			"L109,570Q109,674 155,720Q201,765 283,765Q315,765 342,760"
			"Q368,754 387,747L364,678Q348,683 327,688Q306,693 284,693"
			"Q240,693 219,664Q197,634 197,571L197,536L332,536L332,468Z"
			"M474,737Q494,737 510,724Q525,710 525,681Q525,653 510,639"
			"Q494,625 474,625Q452,625 437,639Q422,653 422,681"
			"Q422,710 437,724Q452,737 474,737ZM517,536L517,0L429,0L429,536L517,536Z";
      g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);
    }
    {
      draw_data.consumed = 0;
      hb_font_draw_glyph (font, 5, funcs, &draw_data);
      char expected[] = "M15,0Q15,0 15,0Z";
      g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);
    }
    {
      draw_data.consumed = 0;
      hb_font_draw_glyph (font, 6, funcs, &draw_data);
      char expected[] = "M346,468L211,468L211,0L123,0L123,468L29,468L29,509L123,539"
			"L123,570Q123,674 169,720Q215,765 297,765Q329,765 356,760"
			"Q382,754 401,747L378,678Q362,683 341,688Q320,693 298,693"
			"Q254,693 233,664Q211,634 211,571L211,536L346,536L346,468Z"
			"M15,0Q15,0 15,0Z";
      g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);
    }

    hb_font_destroy (font);
  }
  {
    hb_face_t *face = hb_test_open_font_file ("fonts/cff1_flex.otf");
    hb_font_t *font = hb_font_create (face);
    hb_face_destroy (face);

    draw_data.consumed = 0;
    hb_font_draw_glyph (font, 1, funcs, &draw_data);
    char expected[] = "M0,0C100,0 150,-20 250,-20C350,-20 400,0 500,0C500,100 520,150 520,250"
		      "C520,350 500,400 500,500C400,500 350,520 250,520C150,520 100,500 0,500"
		      "C0,400 -20,350 -20,250C-20,150 0,100 0,0ZM50,50C50,130 34,170 34,250"
		      "C34,330 50,370 50,450C130,450 170,466 250,466C330,466 370,450 450,450"
		      "C450,370 466,330 466,250C466,170 450,130 450,50C370,50 330,34 250,34"
		      "C170,34 130,50 50,50Z";
    g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);

    hb_font_destroy (font);
  }
  {
    hb_face_t *face = hb_test_open_font_file ("fonts/cff1_dotsect.nohints.otf");
    hb_font_t *font = hb_font_create (face);
    hb_face_destroy (face);

    draw_data.consumed = 0;
    hb_font_draw_glyph (font, 1, funcs, &draw_data);
    char expected[] = "M82,0L164,0L164,486L82,486L82,0Z"
		      "M124,586C156,586 181,608 181,639C181,671 156,692 124,692"
		      "C92,692 67,671 67,639C67,608 92,586 124,586Z";
    g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);

    hb_font_destroy (font);
  }
}

static void
test_hb_draw_font_kit_glyphs_tests (void)
{
  /* https://github.com/foliojs/fontkit/blob/master/test/glyphs.js */
  char str[2048];
  draw_data_t draw_data = {
    .str = str,
    .size = sizeof (str)
  };
  /* truetype glyphs */
  {
    hb_face_t *face = hb_test_open_font_file ("fonts/OpenSans-Regular.ttf");
    hb_font_t *font = hb_font_create (face);
    hb_face_destroy (face);

    /* should get a path for the glyph */
    draw_data.consumed = 0;
    hb_font_draw_glyph (font, 37, funcs, &draw_data);
    char expected[] = "M201,1462L614,1462Q905,1462 1035,1375Q1165,1288 1165,1100"
		      "Q1165,970 1093,886Q1020,801 881,776L881,766Q1214,709 1214,416"
		      "Q1214,220 1082,110Q949,0 711,0L201,0L201,1462ZM371,836L651,836"
		      "Q831,836 910,893Q989,949 989,1083Q989,1206 901,1261"
		      "Q813,1315 621,1315L371,1315L371,836ZM371,692L371,145L676,145"
		      "Q853,145 943,214Q1032,282 1032,428Q1032,564 941,628Q849,692 662,692L371,692Z";
    g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);

    /* should get a path for the glyph */
    draw_data.consumed = 0;
    hb_font_draw_glyph (font, 171, funcs, &draw_data);
    char expected2[] = "M639,-20Q396,-20 256,128Q115,276 115,539Q115,804 246,960Q376,1116 596,1116"
		       "Q802,1116 922,981Q1042,845 1042,623L1042,518L287,518Q292,325 385,225"
		       "Q477,125 645,125Q822,125 995,199L995,51Q907,13 829,-3Q750,-20 639,-20Z"
		       "M594,977Q462,977 384,891Q305,805 291,653L864,653Q864,810 794,894"
		       "Q724,977 594,977ZM471,1266Q519,1328 575,1416Q630,1504 662,1569"
		       "L864,1569L864,1548Q820,1483 733,1388Q646,1293 582,1241L471,1241L471,1266Z";
    g_assert_cmpmem (str, draw_data.consumed, expected2, sizeof (expected2) - 1);

    hb_font_destroy (font);
  }
  {
    hb_face_t *face = hb_test_open_font_file ("fonts/Mada-VF.ttf");
    hb_font_t *font = hb_font_create (face);
    hb_face_destroy (face);

    hb_buffer_t *buffer = hb_buffer_create ();
    hb_codepoint_t codepoint = 1610; /* ي */
    hb_buffer_add_codepoints (buffer, &codepoint, 1, 0, -1);
    hb_buffer_set_direction (buffer, HB_DIRECTION_RTL);
    hb_shape (font, buffer, NULL, 0);
    codepoint = hb_buffer_get_glyph_infos (buffer, NULL)[0].codepoint;
    hb_buffer_destroy (buffer);

    /* should resolve composite glyphs recursively */
    draw_data.consumed = 0;
    hb_font_draw_glyph (font, codepoint, funcs, &draw_data);
    char expected[] = "M581,274L443,274Q409,274 384,259Q359,243 348,219Q336,194 340,166"
		      "Q343,138 365,111L468,-13Q470,-10 473,-6Q475,-3 477,0L253,0Q225,0 203,8"
		      "Q180,15 168,32Q155,48 155,73L155,269L50,269L50,73Q50,24 69,-10"
		      "Q88,-44 118,-64Q147,-85 181,-94Q214,-104 243,-104L473,-104"
		      "Q501,-104 525,-91Q549,-78 564,-56Q578,-34 578,-8Q578,18 557,43"
		      "L442,182Q439,179 437,176Q435,173 432,170L581,170L581,274ZM184,-194"
		      "Q184,-216 199,-231Q214,-246 236,-246Q258,-246 273,-231Q288,-216 288,-194"
		      "Q288,-172 273,-157Q258,-142 236,-142Q214,-142 199,-157Q184,-172 184,-194Z"
		      "M360,-194Q360,-216 375,-231Q390,-246 412,-246Q434,-246 449,-231"
		      "Q464,-216 464,-194Q464,-172 449,-157Q434,-142 412,-142"
		      "Q390,-142 375,-157Q360,-172 360,-194Z";
    g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);

    /* should transform points of a composite glyph */
    draw_data.consumed = 0;
    hb_font_draw_glyph (font, 2, funcs, &draw_data); /* 2 == arAlef.fina */
    char expected2[] = "M155,624L155,84Q150,90 146,95Q141,99 136,105"
		       "L292,105L292,0L156,0Q128,0 104,14Q79,27 65,51"
		       "Q50,74 50,104L50,624L155,624ZM282,105L312,105"
		       "L312,0L282,0L282,105Z";
    g_assert_cmpmem (str, draw_data.consumed, expected2, sizeof (expected2) - 1);

    hb_font_destroy (font);
  }
  /* CFF glyphs, should get a path for the glyph */
  {
    hb_face_t *face = hb_test_open_font_file ("fonts/SourceSansPro-Regular.otf");
    hb_font_t *font = hb_font_create (face);
    hb_face_destroy (face);

    draw_data.consumed = 0;
    hb_font_draw_glyph (font, 5, funcs, &draw_data);
    char expected[] = "M90,0L258,0C456,0 564,122 564,331C564,539 456,656 254,656L90,656L90,0Z"
		      "M173,68L173,588L248,588C401,588 478,496 478,331C478,165 401,68 248,68L173,68Z";
    g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);

    hb_font_destroy (font);
  }
  /* CFF glyphs (CID font) */
  {
    /* replaced with a subset as the original one was 15MB */
    hb_face_t *face = hb_test_open_font_file ("fonts/NotoSansCJKkr-Regular-subset-colon.ttf");
    hb_font_t *font = hb_font_create (face);
    hb_face_destroy (face);

    draw_data.consumed = 0;
    hb_font_draw_glyph (font, 1, funcs, &draw_data);
    char expected[] = "M139,390C175,390 205,419 205,459C205,501 175,530 139,530C103,530 73,501 73,459"
		      "C73,419 103,390 139,390ZM139,-13C175,-13 205,15 205,56C205,97 175,127 139,127"
		      "C103,127 73,97 73,56C73,15 103,-13 139,-13Z";
    g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);

    hb_font_destroy (font);
  }
  /* Skip SBIX glyphs (empty path), COLR glyphs (empty path), WOFF ttf glyphs, WOFF2 ttf glyph */
}

static void
test_hb_draw_font_kit_variations_tests (void)
{
  /* https://github.com/foliojs/fontkit/blob/b310db5/test/variations.js */
  char str[2048];
  draw_data_t draw_data = {
    .str = str,
    .size = sizeof (str)
  };
  /* Skia */
  {
    /* Skipping Skia tests for now even the fact we can actually do platform specific tests using our CIs */
  }
  /* truetype variations */
  /* should support sharing all points */
  {
    hb_face_t *face = hb_test_open_font_file ("fonts/TestGVAROne.ttf");
    hb_font_t *font = hb_font_create (face);
    hb_face_destroy (face);

    hb_variation_t var;
    var.tag = HB_TAG ('w','g','h','t');
    var.value = 300;
    hb_font_set_variations (font, &var, 1);

    hb_buffer_t *buffer = hb_buffer_create ();
    hb_codepoint_t codepoint = 24396; /* 彌 */
    hb_buffer_add_codepoints (buffer, &codepoint, 1, 0, -1);
    hb_buffer_set_direction (buffer, HB_DIRECTION_LTR);
    hb_shape (font, buffer, NULL, 0);
    codepoint = hb_buffer_get_glyph_infos (buffer, NULL)[0].codepoint;
    hb_buffer_destroy (buffer);

    draw_data.consumed = 0;
    hb_font_draw_glyph (font, codepoint, funcs, &draw_data);
    char expected[] = "M371,-102L371,539L914,539L914,-27Q914,-102 840,-102"
		      "Q796,-102 755,-98L742,-59Q790,-66 836,-66Q871,-66 871,-31L871,504"
		      "L414,504L414,-102L371,-102ZM203,-94Q138,-94 86,-90L74,-52"
		      "Q137,-59 188,-59Q211,-59 222,-46Q233,-34 236,12Q238,58 240,135"
		      "Q242,211 242,262L74,262L94,527L242,527L242,719L63,719L63,754"
		      "L285,754L285,492L133,492L117,297L285,297Q285,241 284,185"
		      "Q284,104 281,46Q278,-20 269,-49Q260,-78 242,-86Q223,-94 203,-94Z"
		      "M461,12L434,43Q473,73 503,115Q478,150 441,188L469,211Q501,179 525,147"
		      "Q538,172 559,230L594,211Q571,152 551,117Q577,84 602,43L566,20"
		      "Q544,64 528,86Q500,44 461,12ZM465,258L438,285Q474,316 501,351"
		      "Q474,388 445,418L473,441Q500,414 523,381Q546,413 563,453L598,434"
		      "Q571,382 549,352Q576,320 598,285L563,262Q546,294 525,322Q491,280 465,258Z"
		      "M707,12L680,43Q717,68 753,115Q731,147 691,188L719,211Q739,190 754,172"
		      "Q769,154 774,147Q793,185 809,230L844,211Q822,155 801,117Q828,82 852,43"
		      "L820,20Q798,58 778,87Q747,43 707,12ZM621,-94L621,730L664,730L664,-94"
		      "L621,-94ZM348,570L324,605Q425,629 527,688L555,656Q491,621 439,601"
		      "Q386,581 348,570ZM715,258L688,285Q727,318 753,351Q733,378 695,418L723,441"
		      "Q754,410 775,381Q794,407 813,453L848,434Q826,387 801,352Q823,321 848,281"
		      "L813,262Q791,301 775,323Q749,288 715,258ZM348,719L348,754L941,754L941,719"
		      "L348,719ZM936,570Q870,602 817,622Q764,641 727,652L749,688Q852,655 957,605L936,570Z";
    g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);

    hb_font_destroy (font);
  }
  /* should support sharing enumerated points */
  {
    hb_face_t *face = hb_test_open_font_file ("fonts/TestGVARTwo.ttf");
    hb_font_t *font = hb_font_create (face);
    hb_face_destroy (face);

    hb_variation_t var;
    var.tag = HB_TAG ('w','g','h','t');
    var.value = 300;
    hb_font_set_variations (font, &var, 1);

    hb_buffer_t *buffer = hb_buffer_create ();
    hb_codepoint_t codepoint = 24396; /* 彌 */
    hb_buffer_add_codepoints (buffer, &codepoint, 1, 0, -1);
    hb_buffer_set_direction (buffer, HB_DIRECTION_LTR);
    hb_shape (font, buffer, NULL, 0);
    codepoint = hb_buffer_get_glyph_infos (buffer, NULL)[0].codepoint;
    hb_buffer_destroy (buffer);

    draw_data.consumed = 0;
    hb_font_draw_glyph (font, codepoint, funcs, &draw_data);
    char expected[] = "M371,-102L371,539L914,539L914,-27Q914,-102 840,-102Q796,-102 755,-98"
		      "L742,-59Q790,-66 836,-66Q871,-66 871,-31L871,504L414,504L414,-102"
		      "L371,-102ZM203,-94Q138,-94 86,-90L74,-52Q137,-59 188,-59Q211,-59 222,-46"
		      "Q233,-34 236,12Q238,58 240,135Q242,211 242,262L74,262L94,527L242,527"
		      "L242,719L63,719L63,754L285,754L285,492L133,492L117,297L285,297"
		      "Q285,241 284,185Q284,104 281,46Q278,-20 269,-49Q260,-78 242,-86Q223,-94 203,-94Z"
		      "M461,12L434,43Q473,73 503,115Q478,150 441,188L469,211Q501,179 525,147"
		      "Q538,172 559,230L594,211Q571,152 551,117Q577,84 602,43L566,20"
		      "Q544,64 528,86Q500,44 461,12ZM465,258L438,285Q474,316 501,351"
		      "Q474,388 445,418L473,441Q500,414 523,381Q546,413 563,453L598,434"
		      "Q571,382 549,352Q576,320 598,285L563,262Q546,294 525,322Q491,280 465,258Z"
		      "M707,12L680,43Q717,68 753,115Q731,147 691,188L719,211Q739,190 754,172"
		      "Q769,154 774,147Q793,185 809,230L844,211Q822,155 801,117Q828,82 852,43L820,20"
		      "Q798,58 778,87Q747,43 707,12ZM621,-94L621,730L664,730L664,-94L621,-94ZM348,570"
		      "L324,605Q425,629 527,688L555,656Q491,621 439,601Q386,581 348,570ZM715,258L688,285"
		      "Q727,318 753,351Q733,378 695,418L723,441Q754,410 775,381Q794,407 813,453"
		      "L848,434Q826,387 801,352Q823,321 848,281L813,262Q791,301 775,323Q749,288 715,258Z"
		      "M348,719L348,754L941,754L941,719L348,719ZM936,570Q870,602 817,622"
		      "Q764,641 727,652L749,688Q852,655 957,605L936,570Z";
    g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);

    hb_font_destroy (font);
  }
  /* should support sharing no points */
  {
    hb_face_t *face = hb_test_open_font_file ("fonts/TestGVARThree.ttf");
    hb_font_t *font = hb_font_create (face);
    hb_face_destroy (face);

    hb_variation_t var;
    var.tag = HB_TAG ('w','g','h','t');
    var.value = 300;
    hb_font_set_variations (font, &var, 1);

    hb_buffer_t *buffer = hb_buffer_create ();
    hb_codepoint_t codepoint = 24396; /* 彌 */
    hb_buffer_add_codepoints (buffer, &codepoint, 1, 0, -1);
    hb_buffer_set_direction (buffer, HB_DIRECTION_LTR);
    hb_shape (font, buffer, NULL, 0);
    codepoint = hb_buffer_get_glyph_infos (buffer, NULL)[0].codepoint;
    hb_buffer_destroy (buffer);

    draw_data.consumed = 0;
    hb_font_draw_glyph (font, codepoint, funcs, &draw_data);
    char expected[] = "M371,-102L371,539L914,539L914,-27Q914,-102 840,-102Q796,-102 755,-98"
		      "L742,-59Q790,-66 836,-66Q871,-66 871,-31L871,504L414,504L414,-102"
		      "L371,-102ZM203,-94Q138,-94 86,-90L74,-52Q137,-59 188,-59Q211,-59 222,-46"
		      "Q233,-34 236,12Q238,58 240,135Q242,211 242,262L74,262L94,527L242,527L242,719"
		      "L63,719L63,754L285,754L285,492L133,492L117,297L285,297Q285,241 284,185"
		      "Q284,104 281,46Q278,-20 269,-49Q260,-78 242,-86Q223,-94 203,-94ZM461,12"
		      "L434,43Q473,73 503,115Q478,150 441,188L469,211Q501,179 525,147"
		      "Q538,172 559,230L594,211Q571,152 551,117Q577,84 602,43L566,20Q544,64 528,86"
		      "Q500,44 461,12ZM465,258L438,285Q474,316 501,351Q474,388 445,418L473,441"
		      "Q500,414 523,381Q546,413 563,453L598,434Q571,382 549,352Q576,320 598,285"
		      "L563,262Q546,294 525,322Q491,280 465,258ZM707,12L680,43Q717,68 753,115"
		      "Q731,147 691,188L719,211Q739,190 754,172Q769,154 774,147Q793,185 809,230"
		      "L844,211Q822,155 801,117Q828,82 852,43L820,20Q798,58 778,87Q747,43 707,12Z"
		      "M621,-94L621,730L664,730L664,-94L621,-94ZM348,570L324,605Q425,629 527,688"
		      "L555,656Q491,621 439,601Q386,581 348,570ZM715,258L688,285Q727,318 753,351"
		      "Q733,378 695,418L723,441Q754,410 775,381Q794,407 813,453L848,434Q826,387 801,352"
		      "Q823,321 848,281L813,262Q791,301 775,323Q749,288 715,258ZM348,719L348,754"
		      "L941,754L941,719L348,719ZM936,570Q870,602 817,622"
		      "Q764,641 727,652L749,688Q852,655 957,605L936,570Z";
    g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);

    hb_font_destroy (font);
  }

  /* CFF2 variations */
  {
    hb_face_t *face = hb_test_open_font_file ("fonts/AdobeVFPrototype-Subset.otf");
    hb_font_t *font = hb_font_create (face);
    hb_face_destroy (face);

    hb_variation_t var;
    var.tag = HB_TAG ('w','g','h','t');
    /* applies variations to CFF2 glyphs */
    {
      var.value = 100;
      hb_font_set_variations (font, &var, 1);

      hb_buffer_t *buffer = hb_buffer_create ();
      hb_codepoint_t codepoint = '$';
      hb_buffer_add_codepoints (buffer, &codepoint, 1, 0, -1);
      hb_buffer_set_direction (buffer, HB_DIRECTION_LTR);
      hb_shape (font, buffer, NULL, 0);
      codepoint = hb_buffer_get_glyph_infos (buffer, NULL)[0].codepoint;
      hb_buffer_destroy (buffer);

      draw_data.consumed = 0;
      hb_font_draw_glyph (font, codepoint, funcs, &draw_data);
      char expected[] = "M246,15C188,15 147,27 101,68L142,23L117,117C111,143 96,149 81,149"
		        "C65,149 56,141 52,126C71,40 137,-13 244,-13C348,-13 436,46 436,156"
		        "C436,229 405,295 271,349L247,359C160,393 119,439 119,506"
		        "C119,592 178,637 262,637C311,637 346,626 390,585L348,629L373,535"
		        "C380,510 394,503 408,503C424,503 434,510 437,526C418,614 348,665 259,665"
		        "C161,665 78,606 78,500C78,414 128,361 224,321L261,305C367,259 395,217 395,152"
		        "C395,65 334,15 246,15ZM267,331L267,759L240,759L240,331L267,331ZM240,-115"
		        "L267,-115L267,331L240,331L240,-115Z";
      g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);
    }
    {
      var.value = 500;
      hb_font_set_variations (font, &var, 1);

      hb_buffer_t *buffer = hb_buffer_create ();
      hb_codepoint_t codepoint = '$';
      hb_buffer_add_codepoints (buffer, &codepoint, 1, 0, -1);
      hb_buffer_set_direction (buffer, HB_DIRECTION_LTR);
      hb_shape (font, buffer, NULL, 0);
      codepoint = hb_buffer_get_glyph_infos (buffer, NULL)[0].codepoint;
      hb_buffer_destroy (buffer);

      draw_data.consumed = 0;
      hb_font_draw_glyph (font, codepoint, funcs, &draw_data);
      char expected[] = "M251,36C206,36 165,42 118,61L176,21L161,99C151,152 129,167 101,167"
			"C78,167 61,155 51,131C54,43 133,-14 247,-14C388,-14 474,64 474,171"
			"C474,258 430,321 294,370L257,383C188,406 150,438 150,499"
			"C150,571 204,606 276,606C308,606 342,601 386,582L327,621"
			"L343,546C355,490 382,476 408,476C428,476 448,487 455,512"
			"C450,597 370,656 264,656C140,656 57,576 57,474C57,373 119,318 227,279"
			"L263,266C345,236 379,208 379,145C379,76 329,36 251,36ZM289,320"
			"L289,746L242,746L242,320L289,320ZM240,-115L286,-115L286,320L240,320L240,-115Z";
      g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);
    }
    /* substitutes GSUB features depending on variations */
    {
      var.value = 900;
      hb_font_set_variations (font, &var, 1);

      hb_buffer_t *buffer = hb_buffer_create ();
      hb_codepoint_t codepoint = '$';
      hb_buffer_add_codepoints (buffer, &codepoint, 1, 0, -1);
      hb_buffer_set_direction (buffer, HB_DIRECTION_LTR);
      hb_shape (font, buffer, NULL, 0);
      codepoint = hb_buffer_get_glyph_infos (buffer, NULL)[0].codepoint;
      hb_buffer_destroy (buffer);

      draw_data.consumed = 0;
      hb_font_draw_glyph (font, codepoint, funcs, &draw_data);
      char expected[] = "M258,38C197,38 167,48 118,71L192,19L183,103C177,155 155,174 115,174"
			"C89,174 64,161 51,125C52,36 124,-16 258,-16C417,-16 513,67 513,175"
			"C513,278 457,328 322,388L289,403C232,429 203,452 203,500C203,562 244,589 301,589"
			"C342,589 370,585 420,562L341,607L352,539C363,468 398,454 434,454C459,454 486,468 492,506"
			"C491,590 408,643 290,643C141,643 57,563 57,460C57,357 122,307 233,256L265,241"
			"C334,209 363,186 363,130C363,77 320,38 258,38ZM318,616L318,734L252,734L252,616"
			"L318,616ZM253,-115L319,-115L319,14L253,14L253,-115Z";
      g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);
    }

    hb_font_destroy (font);
  }
}

static void
test_hb_draw_estedad_vf (void)
{
  /* https://github.com/harfbuzz/harfbuzz/issues/2215 */
  char str[2048];
  draw_data_t draw_data = {
    .str = str,
    .size = sizeof (str)
  };
  {
    /* See https://github.com/google/skia/blob/d38f00a1/gm/stroketext.cpp#L115-L124 */
    hb_face_t *face = hb_test_open_font_file ("fonts/Estedad-VF.ttf");
    hb_font_t *font = hb_font_create (face);
    hb_face_destroy (face);

    hb_variation_t var;
    hb_variation_from_string ("wght=100", -1, &var);
    hb_font_set_variations (font, &var, 1);

    draw_data.consumed = 0;
    hb_font_draw_glyph (font, 156, funcs, &draw_data);
    /* Skip empty path where all the points of a path are equal */
    char expected[] = "M150,1158L182,1158Q256,1158 317,1170Q377,1182 421,1213L421,430L521,430"
		      "L521,1490L421,1490L421,1320Q393,1279 344,1262Q294,1244 182,1244L150,1244"
		      "L150,1158ZM1815,-122L1669,-122L1669,642L1552,642L1055,-117L1055,-206"
		      "L1569,-206L1569,-458L1669,-458L1669,-206L1815,-206L1815,-122ZM1569,-122"
		      "L1166,-122L1569,494L1569,-122ZM609,-79L1639,1288L1555,1334L525,-33L609,-79Z";
    g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);

    draw_data.consumed = 0;
    hb_font_draw_glyph (font, 180, funcs, &draw_data);
    /* Skip empty path where all the points of a path are equal */
    char expected2[] = "M120,693Q120,545 177,414Q233,282 333,182Q433,81 567,24"
		       "Q701,-33 856,-33Q1010,-33 1144,24Q1277,81 1377,182Q1477,282 1534,414"
		       "Q1590,545 1590,693Q1590,842 1534,973Q1477,1104 1377,1205"
		       "Q1277,1305 1144,1362Q1010,1419 856,1419Q701,1419 567,1362"
		       "Q433,1305 333,1205Q233,1104 177,973Q120,842 120,693Z"
		       "M220,693Q220,828 270,945Q320,1061 409,1148Q497,1235 612,1284"
		       "Q726,1333 855,1333Q984,1333 1099,1284Q1213,1235 1302,1148"
		       "Q1390,1061 1440,945Q1490,828 1490,693Q1490,558 1440,442"
		       "Q1390,325 1302,237Q1213,149 1099,100Q984,51 855,51"
		       "Q726,51 611,100Q497,149 408,237Q320,325 270,442"
		       "Q220,558 220,693ZM690,643L690,997L886,997Q970,997 1029,949"
		       "Q1087,901 1087,819Q1087,737 1028,690Q969,643 886,643L690,643Z"
		       "M1165,334L973,568Q1065,591 1126,658Q1187,725 1187,819"
		       "Q1187,896 1147,956Q1106,1015 1037,1049Q969,1083 886,1083"
		       "L590,1083L590,310L690,310L690,557L860,557L1083,286L1165,334Z";
    g_assert_cmpmem (str, draw_data.consumed, expected2, sizeof (expected2) - 1);

    draw_data.consumed = 0;
    hb_font_draw_glyph (font, 262, funcs, &draw_data);
    /* Skip empty path where all the points of a path are equal */
    char expected3[] = "M422,598Q495,598 545,548Q595,498 595,426Q595,353 545,303Q494,252 422,252"
		       "Q350,252 300,303Q250,353 250,426Q250,499 300,549Q349,598 422,598ZM422,698"
		       "Q347,698 285,662Q223,625 187,564Q150,502 150,426Q150,351 187,289"
		       "Q223,226 285,189Q346,152 422,152Q498,152 560,189Q622,226 658,288"
		       "Q695,351 695,426Q695,502 658,563Q621,625 559,661Q498,698 422,698Z";
    g_assert_cmpmem (str, draw_data.consumed, expected3, sizeof (expected3) - 1);

    hb_font_destroy (font);
  }
}

static void
test_hb_draw_stroking (void)
{
  /* https://skia-review.googlesource.com/c/skia/+/266945
     https://savannah.nongnu.org/bugs/index.php?57701 */
  char str[2048];
  draw_data_t draw_data = {
    .str = str,
    .size = sizeof (str)
  };
  {
    /* See https://github.com/google/skia/blob/d38f00a1/gm/stroketext.cpp#L115-L124 */
    hb_face_t *face = hb_test_open_font_file ("fonts/Stroking.ttf");
    hb_font_t *font = hb_font_create (face);
    hb_face_destroy (face);

    draw_data.consumed = 0;
    hb_font_draw_glyph (font, 6, funcs, &draw_data);
    /* Skip empty path where all the points of a path are equal */
    char expected[] = "M436,1522Q436,1280 531,1060Q625,839 784,680Q943,521 1164,427Q1384,332 1626,332"
		      "Q1868,332 2089,427Q2309,521 2468,680Q2627,839 2722,1060Q2816,1280 2816,1522"
		      "Q2816,1764 2722,1985Q2627,2205 2468,2364Q2309,2523 2089,2618Q1868,2712 1626,2712"
		      "Q1384,2712 1164,2618Q943,2523 784,2364Q625,2205 531,1985Q436,1764 436,1522ZM256,1528"
		      "Q256,1714 306,1892Q355,2069 443,2220Q531,2370 658,2497Q784,2623 935,2711"
		      "Q1085,2799 1263,2849Q1440,2898 1626,2898Q1812,2898 1990,2849Q2167,2799 2318,2711"
		      "Q2468,2623 2595,2497Q2721,2370 2809,2220Q2897,2069 2947,1892Q2996,1714 2996,1528"
		      "Q2996,1342 2947,1165Q2897,987 2809,837Q2721,686 2595,560Q2468,433 2318,345"
		      "Q2167,257 1990,208Q1812,158 1626,158Q1440,158 1263,208Q1085,257 935,345"
		      "Q784,433 658,560Q531,686 443,837Q355,987 306,1165Q256,1342 256,1528Z";
    g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);

    draw_data.consumed = 0;
    hb_font_draw_glyph (font, 7, funcs, &draw_data);
    char expected2[] = "M436,1522Q436,1280 531,1060Q625,839 784,680Q943,521 1164,427"
		       "Q1384,332 1626,332Q1868,332 2089,427Q2309,521 2468,680"
		       "Q2627,839 2722,1060Q2816,1280 2816,1522Q2816,1764 2722,1985"
		       "Q2627,2205 2468,2364Q2309,2523 2089,2618Q1868,2712 1626,2712"
		       "Q1384,2712 1164,2618Q943,2523 784,2364Q625,2205 531,1985"
		       "Q436,1764 436,1522ZM256,1528Q256,1714 306,1892Q355,2069 443,2220"
		       "Q531,2370 658,2497Q784,2623 935,2711Q1085,2799 1263,2849"
		       "Q1440,2898 1626,2898Q1812,2898 1990,2849Q2167,2799 2318,2711"
		       "Q2468,2623 2595,2497Q2721,2370 2809,2220Q2897,2069 2947,1892"
		       "Q2996,1714 2996,1528Q2996,1342 2947,1165Q2897,987 2809,837"
		       "Q2721,686 2595,560Q2468,433 2318,345Q2167,257 1990,208"
		       "Q1812,158 1626,158Q1440,158 1263,208Q1085,257 935,345"
		       "Q784,433 658,560Q531,686 443,837Q355,987 306,1165"
		       "Q256,1342 256,1528Z";
    g_assert_cmpmem (str, draw_data.consumed, expected2, sizeof (expected2) - 1);

    hb_font_destroy (font);
  }
  {
    /* https://github.com/google/skia/blob/d38f00a1/gm/stroketext.cpp#L131-L138 */
    hb_face_t *face = hb_test_open_font_file ("fonts/Stroking.otf");
    hb_font_t *font = hb_font_create (face);
    hb_face_destroy (face);

    draw_data.consumed = 0;
    hb_font_draw_glyph (font, 4, funcs, &draw_data);
    /* Skip empty path in CFF */
    char expected[] = "M106,372C106,532 237,662 397,662C557,662 688,532 688,372C688,212 557,81 397,81C237,81 106,212 106,372Z"
		      "M62,373C62,188 212,39 397,39C582,39 731,188 731,373C731,558 582,708 397,708C212,708 62,558 62,373Z";
    g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);

    draw_data.consumed = 0;
    hb_font_draw_glyph (font, 5, funcs, &draw_data);
    /* Fold consequent move-to commands */
    char expected2[] = "M106,372C106,532 237,662 397,662C557,662 688,532 688,372"
		       "C688,212 557,81 397,81C237,81 106,212 106,372ZM62,373"
		       "C62,188 212,39 397,39C582,39 731,188 731,373"
		       "C731,558 582,708 397,708C212,708 62,558 62,373Z";
    g_assert_cmpmem (str, draw_data.consumed, expected2, sizeof (expected2) - 1);

    hb_font_destroy (font);
  }
}

static void
test_hb_draw_drawing_funcs (void)
{
  char str[2048];
  draw_data_t draw_data = {
    .str = str,
    .size = sizeof (str)
  };

  {
    hb_draw_state_t st = HB_DRAW_STATE_DEFAULT;
    draw_data.consumed = 0;
    hb_draw_move_to (funcs, &draw_data, &st, 90.f, 0.f);
    hb_draw_line_to (funcs, &draw_data, &st, 258.f, 0.f);
    hb_draw_cubic_to (funcs, &draw_data, &st, 456.f, 0.f, 564.f, 122.f, 564.f, 331.f);
    hb_draw_cubic_to (funcs, &draw_data, &st, 564.f, 539.f, 456.f, 656.f, 254.f, 656.f);
    hb_draw_line_to (funcs, &draw_data, &st, 90.f, 656.f);
    hb_draw_line_to (funcs, &draw_data, &st, 90.f, 0.f);
    hb_draw_close_path (funcs, &draw_data, &st);

    char expected[] = "M90,0L258,0C456,0 564,122 564,331C564,539 456,656 254,656L90,656L90,0Z";
    g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);
  }

  {
    hb_draw_state_t st = HB_DRAW_STATE_DEFAULT;
    draw_data.consumed = 0;
    hb_draw_move_to (funcs, &draw_data, &st, 155.f, 624.f);
    hb_draw_line_to (funcs, &draw_data, &st, 155.f, 84.f);
    hb_draw_quadratic_to (funcs, &draw_data, &st, 150.f, 90.f, 146.f, 95.f);
    hb_draw_quadratic_to (funcs, &draw_data, &st, 141.f, 99.f, 136.f, 105.f);
    hb_draw_line_to (funcs, &draw_data, &st, 292.f, 105.f);
    hb_draw_line_to (funcs, &draw_data, &st, 292.f, 0.f);
    hb_draw_line_to (funcs, &draw_data, &st, 156.f, 0.f);
    hb_draw_quadratic_to (funcs, &draw_data, &st, 128.f, 0.f, 104.f, 14.f);
    hb_draw_quadratic_to (funcs, &draw_data, &st, 79.f, 27.f, 65.f, 51.f);
    hb_draw_quadratic_to (funcs, &draw_data, &st, 50.f, 74.f, 50.f, 104.f);
    hb_draw_line_to (funcs, &draw_data, &st, 50.f, 624.f);
    hb_draw_line_to (funcs, &draw_data, &st, 155.f, 624.f);
    hb_draw_close_path (funcs, &draw_data, &st);

    char expected[] = "M155,624L155,84Q150,90 146,95Q141,99 136,105"
		       "L292,105L292,0L156,0Q128,0 104,14Q79,27 65,51"
		       "Q50,74 50,104L50,624L155,624Z";
    g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);
  }
}

static void
test_hb_draw_synthetic_slant (void)
{
  char str[2048];
  draw_data_t draw_data = {
    .str = str,
    .size = sizeof (str)
  };
  {
    hb_face_t *face = hb_test_open_font_file ("fonts/OpenSans-Regular.ttf");
    hb_font_t *font = hb_font_create (face);
    hb_face_destroy (face);
    hb_font_set_synthetic_slant (font, 0.2f);

    draw_data.consumed = 0;
    hb_font_draw_glyph (font, 37, funcs, &draw_data);
    char expected[] = "M493,1462L906,1462Q1197,1462 1310,1375Q1423,1288 1385,1100"
		      "Q1359,970 1270,886Q1180,801 1036,776L1034,766Q1356,709 1297,416"
		      "Q1258,220 1104,110Q949,0 711,0L201,0L493,1462ZM538,836L818,836"
		      "Q998,836 1089,893Q1179,949 1206,1083Q1230,1206 1153,1261"
		      "Q1076,1315 884,1315L634,1315L538,836ZM509,692L400,145L705,145"
		      "Q882,145 985,214Q1088,282 1118,428Q1145,564 1066,628Q987,692 800,692L509,692Z";
    g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);

    hb_font_destroy (font);
  }
  {
    hb_face_t *face = hb_test_open_font_file ("fonts/SourceSansPro-Regular.otf");
    hb_font_t *font = hb_font_create (face);
    hb_face_destroy (face);
    hb_font_set_synthetic_slant (font, 0.2f);

    draw_data.consumed = 0;
    hb_font_draw_glyph (font, 5, funcs, &draw_data);
    char expected[] = "M90,0L258,0C456,0 588,122 630,331C672,539 587,656 385,656L221,656L90,0Z"
		      "M187,68L291,588L366,588C519,588 577,496 544,331C511,165 415,68 262,68L187,68Z";
    g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);

    hb_font_destroy (font);
  }
}

static void
test_hb_draw_subfont_scale (void)
{
  char str[2048];
  draw_data_t draw_data = {
    .str = str,
    .size = sizeof (str)
  };
  signed x, y;
  {
    hb_face_t *face = hb_test_open_font_file ("fonts/OpenSans-Regular.ttf");
    hb_font_t *font1 = hb_font_create (face);
    hb_font_t *font2 = hb_font_create_sub_font (font1);

    hb_font_get_scale (font1, &x, &y);
    hb_font_set_scale (font2, x*2, y*2);

    draw_data.consumed = 0;
    hb_font_draw_glyph (font1, 37, funcs, &draw_data);
    char expected1[] = "M201,1462L614,1462Q905,1462 1035,1375Q1165,1288 1165,1100"
		       "Q1165,970 1093,886Q1020,801 881,776L881,766Q1214,709 1214,416"
		       "Q1214,220 1082,110Q949,0 711,0L201,0L201,1462ZM371,836L651,836"
		       "Q831,836 910,893Q989,949 989,1083Q989,1206 901,1261"
		       "Q813,1315 621,1315L371,1315L371,836ZM371,692L371,145L676,145"
		       "Q853,145 943,214Q1032,282 1032,428Q1032,564 941,628Q849,692 662,692L371,692Z";
    g_assert_cmpmem (str, draw_data.consumed, expected1, sizeof (expected1) - 1);

    draw_data.consumed = 0;
    hb_font_draw_glyph (font2, 37, funcs, &draw_data);
    char expected2[] = "M402,2924L1228,2924Q1810,2924 2070,2750Q2330,2576 2330,2200"
		       "Q2330,1940 2185,1771Q2040,1602 1762,1552L1762,1532Q2428,1418 2428,832"
		       "Q2428,440 2163,220Q1898,0 1422,0L402,0L402,2924ZM742,1672L1302,1672"
		       "Q1662,1672 1820,1785Q1978,1898 1978,2166Q1978,2412 1802,2521"
		       "Q1626,2630 1242,2630L742,2630L742,1672ZM742,1384L742,290L1352,290"
		       "Q1706,290 1885,427Q2064,564 2064,856Q2064,1128 1881,1256Q1698,1384 1324,1384L742,1384Z";
    g_assert_cmpmem (str, draw_data.consumed, expected2, sizeof (expected2) - 1);

    hb_font_destroy (font1);
    hb_font_destroy (font2);
    hb_face_destroy (face);
  }
  {
    hb_face_t *face = hb_test_open_font_file ("fonts/SourceSansPro-Regular.otf");
    hb_font_t *font1 = hb_font_create (face);
    hb_font_t *font2 = hb_font_create_sub_font (font1);

    hb_font_get_scale (font1, &x, &y);
    hb_font_set_scale (font2, x*2, y*2);

    draw_data.consumed = 0;
    hb_font_draw_glyph (font1, 5, funcs, &draw_data);
    char expected1[] = "M90,0L258,0C456,0 564,122 564,331C564,539 456,656 254,656L90,656L90,0Z"
		       "M173,68L173,588L248,588C401,588 478,496 478,331C478,165 401,68 248,68L173,68Z";
    g_assert_cmpmem (str, draw_data.consumed, expected1, sizeof (expected1) - 1);

    draw_data.consumed = 0;
    hb_font_draw_glyph (font2, 5, funcs, &draw_data);
    char expected2[] = "M180,0L516,0C912,0 1128,244 1128,662C1128,1078 912,1312 508,1312L180,1312L180,0Z"
		       "M346,136L346,1176L496,1176C802,1176 956,992 956,662C956,330 802,136 496,136L346,136Z";
    g_assert_cmpmem (str, draw_data.consumed, expected2, sizeof (expected2) - 1);

    hb_font_destroy (font1);
    hb_font_destroy (font2);
    hb_face_destroy (face);
  }
}

static void
test_hb_draw_immutable (void)
{
  hb_draw_funcs_t *draw_funcs = hb_draw_funcs_create ();
  g_assert (!hb_draw_funcs_is_immutable (draw_funcs));
  hb_draw_funcs_make_immutable (draw_funcs);
  g_assert (hb_draw_funcs_is_immutable (draw_funcs));
  hb_draw_funcs_destroy (draw_funcs);
}

#ifdef HAVE_FREETYPE
static void test_hb_draw_ft (void)
{
  char str[1024];
  draw_data_t draw_data = {
    .str = str,
    .size = sizeof (str)
  };
  {
    hb_face_t *face = hb_test_open_font_file ("fonts/glyphs.ttf");
    hb_font_t *font = hb_font_create (face);
    hb_ft_font_set_funcs (font);
    hb_face_destroy (face);
    {
      draw_data.consumed = 0;
      hb_font_draw_glyph (font, 0, funcs, &draw_data);
      char expected[] = "M50,0L50,750L450,750L450,0L50,0Z";
      g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);
    }
    {
      draw_data.consumed = 0;
      hb_font_draw_glyph (font, 5, funcs, &draw_data);
      char expected[] = "M15,0Q15,0 15,0Z";
      g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);
    }
    hb_font_destroy (font);
  }
  {
    hb_face_t *face = hb_test_open_font_file ("fonts/cff1_flex.otf");
    hb_font_t *font = hb_font_create (face);
    hb_ft_font_set_funcs (font);
    hb_face_destroy (face);

    draw_data.consumed = 0;
    hb_font_draw_glyph (font, 1, funcs, &draw_data);
    char expected[] = "M0,0C100,0 150,-20 250,-20C350,-20 400,0 500,0C500,100 520,150 520,250"
		      "C520,350 500,400 500,500C400,500 350,520 250,520C150,520 100,500 0,500"
		      "C0,400 -20,350 -20,250C-20,150 0,100 0,0ZM50,50C50,130 34,170 34,250"
		      "C34,330 50,370 50,450C130,450 170,466 250,466C330,466 370,450 450,450"
		      "C450,370 466,330 466,250C466,170 450,130 450,50C370,50 330,34 250,34"
		      "C170,34 130,50 50,50Z";
    g_assert_cmpmem (str, draw_data.consumed, expected, sizeof (expected) - 1);

    hb_font_destroy (font);
  }
}

static void
test_hb_draw_compare_ot_ft (void)
{
  char str[1024];
  draw_data_t draw_data = {
    .str = str,
    .size = sizeof (str),
    .consumed = 0
  };
  char str2[1024];
  draw_data_t draw_data2 = {
    .str = str2,
    .size = sizeof (str2),
    .consumed = 0
  };

  hb_face_t *face = hb_test_open_font_file ("fonts/cff1_flex.otf");
  hb_font_t *font = hb_font_create (face);

  hb_font_set_scale (font, 100, 100);

  hb_font_draw_glyph (font, 1, funcs, &draw_data);
  draw_data.str[draw_data.consumed] = '\0';

  hb_ft_font_set_funcs (font);

  hb_font_draw_glyph (font, 1, funcs, &draw_data2);
  draw_data2.str[draw_data2.consumed] = '\0';

  g_assert_cmpstr (draw_data.str, ==, draw_data2.str);

  hb_font_destroy (font);
  hb_face_destroy (face);
}
#endif

int
main (int argc, char **argv)
{
  funcs = hb_draw_funcs_create ();
  hb_draw_funcs_set_move_to_func (funcs, (hb_draw_move_to_func_t) move_to, NULL, NULL);
  hb_draw_funcs_set_line_to_func (funcs, (hb_draw_line_to_func_t) line_to, NULL, NULL);
  hb_draw_funcs_set_quadratic_to_func (funcs, (hb_draw_quadratic_to_func_t) quadratic_to, NULL, NULL);
  hb_draw_funcs_set_cubic_to_func (funcs, (hb_draw_cubic_to_func_t) cubic_to, NULL, NULL);
  hb_draw_funcs_set_close_path_func (funcs, (hb_draw_close_path_func_t) close_path, NULL, NULL);
  hb_draw_funcs_make_immutable (funcs);

  funcs2 = hb_draw_funcs_create ();
  hb_draw_funcs_set_move_to_func (funcs2, (hb_draw_move_to_func_t) move_to, NULL, NULL);
  hb_draw_funcs_set_line_to_func (funcs2, (hb_draw_line_to_func_t) line_to, NULL, NULL);
  hb_draw_funcs_set_cubic_to_func (funcs2, (hb_draw_cubic_to_func_t) cubic_to, NULL, NULL);
  hb_draw_funcs_set_close_path_func (funcs2, (hb_draw_close_path_func_t) close_path, NULL, NULL);
  hb_draw_funcs_make_immutable (funcs2);

  hb_test_init (&argc, &argv);
  hb_test_add (test_itoa);
  hb_test_add (test_hb_draw_empty);
  hb_test_add (test_hb_draw_glyf);
  hb_test_add (test_hb_draw_cff1);
  hb_test_add (test_hb_draw_cff1_rline);
  hb_test_add (test_hb_draw_cff2);
  hb_test_add (test_hb_draw_ttf_parser_tests);
  hb_test_add (test_hb_draw_font_kit_glyphs_tests);
  hb_test_add (test_hb_draw_font_kit_variations_tests);
  hb_test_add (test_hb_draw_estedad_vf);
 if(0) hb_test_add (test_hb_draw_stroking);
  hb_test_add (test_hb_draw_drawing_funcs);
  hb_test_add (test_hb_draw_synthetic_slant);
  hb_test_add (test_hb_draw_subfont_scale);
  hb_test_add (test_hb_draw_immutable);
#ifdef HAVE_FREETYPE
  hb_test_add (test_hb_draw_ft);
  hb_test_add (test_hb_draw_compare_ot_ft);
#endif
  unsigned result = hb_test_run ();

  hb_draw_funcs_destroy (funcs);
  hb_draw_funcs_destroy (funcs2);
  return result;
}
