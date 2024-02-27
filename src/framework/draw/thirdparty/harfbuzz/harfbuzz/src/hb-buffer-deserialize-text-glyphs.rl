/*
 * Copyright © 2013  Google, Inc.
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

#ifndef HB_BUFFER_DESERIALIZE_TEXT_GLYPHS_HH
#define HB_BUFFER_DESERIALIZE_TEXT_GLYPHS_HH

#include "hb.hh"

%%{

machine deserialize_text_glyphs;
alphtype unsigned char;
write data;

action clear_item {
	hb_memset (&info, 0, sizeof (info));
	hb_memset (&pos , 0, sizeof (pos ));
}

action add_item {
	buffer->add_info (info);
	if (unlikely (!buffer->successful))
	  return false;
	buffer->pos[buffer->len - 1] = pos;
	*end_ptr = p;
}

action tok {
	tok = p;
}

action parse_glyph {
	/* TODO Unescape delimiters. */
	if (!hb_font_glyph_from_string (font,
					tok, p - tok,
					&info.codepoint))
	  return false;
}

action parse_cluster	{ if (!parse_uint (tok, p, &info.cluster )) return false; }
action parse_x_offset	{ if (!parse_int  (tok, p, &pos.x_offset )) return false; }
action parse_y_offset	{ if (!parse_int  (tok, p, &pos.y_offset )) return false; }
action parse_x_advance	{ if (!parse_int  (tok, p, &pos.x_advance)) return false; }
action parse_y_advance	{ if (!parse_int  (tok, p, &pos.y_advance)) return false; }
action parse_glyph_flags{ if (!parse_uint (tok, p, &info.mask    )) return false; }

unum  = '0' | [1-9] digit*;
num	= '-'? unum;

glyph_id = unum;
glyph_name = ([^\\\]=@+,#|] | '\\' [\\\]=@+,|]) *;

glyph	= (glyph_id | glyph_name) >tok %parse_glyph;
cluster	= '=' (unum >tok %parse_cluster);
offsets	= '@' (num >tok %parse_x_offset)   ',' (num >tok %parse_y_offset );
advances= '+' (num >tok %parse_x_advance) (',' (num >tok %parse_y_advance))?;
glyphflags= '#' (unum >tok %parse_glyph_flags);

glyph_item	=
	(
		glyph
		cluster?
		offsets?
		advances?
		glyphflags?
	)
	>clear_item
	%add_item
	;

glyphs = glyph_item (space* '|' space* glyph_item)* space*;

main := space* glyphs;

}%%

static hb_bool_t
_hb_buffer_deserialize_text_glyphs (hb_buffer_t *buffer,
				    const char *buf,
				    unsigned int buf_len,
				    const char **end_ptr,
				    hb_font_t *font)
{
  const char *p = buf, *pe = buf + buf_len, *eof = pe, *orig_pe = pe;

  /* Ensure we have positions. */
  (void) hb_buffer_get_glyph_positions (buffer, nullptr);

  while (p < pe && ISSPACE (*p))
    p++;
  if (p < pe && *p == (buffer->len ? '|' : '['))
    *end_ptr = ++p;

  const char *end = strchr ((char *) p, ']');
  if (end)
    pe = eof = end;
  else
  {
    end = strrchr ((char *) p, '|');
    if (end)
      pe = eof = end;
    else
      pe = eof = p;
  }

  const char *tok = nullptr;
  int cs;
  hb_glyph_info_t info = {0};
  hb_glyph_position_t pos = {0};
  %%{
    write init;
    write exec;
  }%%

  if (pe < orig_pe && *pe == ']')
  {
    pe++;
    if (p == pe)
      p++;
  }

  *end_ptr = p;

  return p == pe;
}

#endif /* HB_BUFFER_DESERIALIZE_TEXT_GLYPHS_HH */
