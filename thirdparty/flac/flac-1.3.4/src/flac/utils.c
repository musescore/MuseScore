/* flac - Command-line FLAC encoder/decoder
 * Copyright (C) 2002-2009  Josh Coalson
 * Copyright (C) 2011-2016  Xiph.Org Foundation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "FLAC/assert.h"
#include "FLAC/metadata.h"
#include "share/compat.h"
#ifndef _WIN32
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <wchar.h>
#ifdef HAVE_TERMIOS_H
# include <termios.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif
#endif

const char *CHANNEL_MASK_TAG = "WAVEFORMATEXTENSIBLE_CHANNEL_MASK";

int flac__utils_verbosity_ = 2;

static FLAC__bool local__parse_uint64_(const char *s, FLAC__uint64 *value)
{
	FLAC__uint64 ret = 0;
	char c;

	if(*s == '\0')
		return false;

	while('\0' != (c = *s++))
		if(c >= '0' && c <= '9')
			ret = ret * 10 + (c - '0');
		else
			return false;

	*value = ret;
	return true;
}

static FLAC__bool local__parse_timecode_(const char *s, double *value)
{
	double ret;
	uint32_t i;
	char c, *endptr;

	/* parse [0-9][0-9]*: */
	c = *s++;
	if(c >= '0' && c <= '9')
		i = (c - '0');
	else
		return false;
	while(':' != (c = *s++)) {
		if(c >= '0' && c <= '9')
			i = i * 10 + (c - '0');
		else
			return false;
	}
	ret = (double)i * 60.;

	/* parse [0-9]*[.,]?[0-9]* i.e. a sign-less rational number (. or , OK for fractional seconds, to support different locales) */
	if(strspn(s, "1234567890.,") != strlen(s))
		return false;
	ret += strtod(s, &endptr);
	if (endptr == s || *endptr)
		return false;

	*value = ret;
	return true;
}

static FLAC__bool local__parse_cue_(const char *s, const char *end, uint32_t *track, uint32_t *indx)
{
	FLAC__bool got_track = false, got_index = false;
	uint32_t t = 0, i = 0;
	char c;

	while(end? s < end : *s != '\0') {
		c = *s++;
		if(c >= '0' && c <= '9') {
			t = t * 10 + (c - '0');
			got_track = true;
		}
		else if(c == '.')
			break;
		else
			return false;
	}
	while(end? s < end : *s != '\0') {
		c = *s++;
		if(c >= '0' && c <= '9') {
			i = i * 10 + (c - '0');
			got_index = true;
		}
		else
			return false;
	}
	*track = t;
	*indx = i;
	return got_track && got_index;
}

/*
 * this only works with sorted cuesheets (the spec strongly recommends but
 * does not require sorted cuesheets).  but if it's not sorted, picking a
 * nearest cue point has no significance.
 */
static FLAC__uint64 local__find_closest_cue_(const FLAC__StreamMetadata_CueSheet *cuesheet, uint32_t track, uint32_t indx, FLAC__uint64 total_samples, FLAC__bool look_forward)
{
	int t, i;
	if(look_forward) {
		for(t = 0; t < (int)cuesheet->num_tracks; t++)
			for(i = 0; i < (int)cuesheet->tracks[t].num_indices; i++)
				if(cuesheet->tracks[t].number > track || (cuesheet->tracks[t].number == track && cuesheet->tracks[t].indices[i].number >= indx))
					return cuesheet->tracks[t].offset + cuesheet->tracks[t].indices[i].offset;
		return total_samples;
	}
	else {
		for(t = (int)cuesheet->num_tracks - 1; t >= 0; t--)
			for(i = (int)cuesheet->tracks[t].num_indices - 1; i >= 0; i--)
				if(cuesheet->tracks[t].number < track || (cuesheet->tracks[t].number == track && cuesheet->tracks[t].indices[i].number <= indx))
					return cuesheet->tracks[t].offset + cuesheet->tracks[t].indices[i].offset;
		return 0;
	}
}

void flac__utils_printf(FILE *stream, int level, const char *format, ...)
{
	if(flac__utils_verbosity_ >= level) {
		va_list args;

		FLAC__ASSERT(0 != format);

		va_start(args, format);

		(void) flac_vfprintf(stream, format, args);

		va_end(args);

#ifdef _MSC_VER
		if(stream == stderr)
			fflush(stream); /* for some reason stderr is buffered in at least some if not all MSC libs */
#endif
	}
}

/* variables and functions for console status output */
static FLAC__bool is_name_printed;
static int stats_char_count = 0;
static int console_width;
static int console_chars_left;

int get_console_width(void)
{
	int width = 0;
#if defined _WIN32
	width = win_get_console_width();
#elif defined __EMX__
	int s[2];
	_scrsize (s);
	width = s[0];
#elif defined TIOCGWINSZ
	struct winsize w;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1)
		width = w.ws_col;
#endif
	if (width <= 0)
		width = 80;
	return width;
}

size_t strlen_console(const char *text)
{
#ifdef _WIN32
	return strlen_utf8(text);
#elif defined(__DJGPP__) /* workaround for DJGPP missing wcswidth() */
	return strlen(text);
#else
	size_t len;
	wchar_t *wtmp;

	len = strlen(text)+1;
	wtmp = (wchar_t *)malloc(len*sizeof(wchar_t));
	if (wtmp == NULL) return len-1;
	mbstowcs(wtmp, text, len);
	len = wcswidth(wtmp, len);
	free(wtmp);

	return len;
#endif
}

void stats_new_file(void)
{
	is_name_printed = false;
}

void stats_clear(void)
{
	while (stats_char_count > 0 && stats_char_count--)
		fprintf(stderr, "\b");
}

void stats_print_name(int level, const char *name)
{
	int len;

	if (flac__utils_verbosity_ >= level) {
		stats_clear();
		if(is_name_printed) return;

		console_width = get_console_width();
		len = strlen_console(name)+2;
		console_chars_left = console_width  - (len % console_width);
		flac_fprintf(stderr, "%s: ", name);
		is_name_printed = true;
	}
}

void stats_print_info(int level, const char *format, ...)
{
	char tmp[80];
	int len, clear_len;

	if (flac__utils_verbosity_ >= level) {
		va_list args;
		va_start(args, format);
		len = flac_vsnprintf(tmp, sizeof(tmp), format, args);
		va_end(args);
		stats_clear();
		if (len >= console_chars_left) {
			clear_len = console_chars_left;
			while (clear_len > 0 && clear_len--) fprintf(stderr, " ");
			fprintf(stderr, "\n");
			console_chars_left = console_width;
		}
		stats_char_count = fprintf(stderr, "%s", tmp);
		fflush(stderr);
	}
}

#ifdef FLAC__VALGRIND_TESTING
size_t flac__utils_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t ret = fwrite(ptr, size, nmemb, stream);
	if(!ferror(stream))
		fflush(stream);
	return ret;
}
#endif

FLAC__bool flac__utils_parse_skip_until_specification(const char *s, utils__SkipUntilSpecification *spec)
{
	FLAC__uint64 val;
	FLAC__bool is_negative = false;

	FLAC__ASSERT(0 != spec);

	spec->is_relative = false;
	spec->value_is_samples = true;
	spec->value.samples = 0;

	if(0 != s) {
		if(s[0] == '-') {
			is_negative = true;
			spec->is_relative = true;
			s++;
		}
		else if(s[0] == '+') {
			spec->is_relative = true;
			s++;
		}

		if(local__parse_uint64_(s, &val)) {
			spec->value_is_samples = true;
			spec->value.samples = (FLAC__int64)val;
			if(is_negative)
				spec->value.samples = -(spec->value.samples);
		}
		else {
			double d;
			if(!local__parse_timecode_(s, &d))
				return false;
			spec->value_is_samples = false;
			spec->value.seconds = d;
			if(is_negative)
				spec->value.seconds = -(spec->value.seconds);
		}
	}

	return true;
}

void flac__utils_canonicalize_skip_until_specification(utils__SkipUntilSpecification *spec, uint32_t sample_rate)
{
	FLAC__ASSERT(0 != spec);
	if(!spec->value_is_samples) {
		spec->value.samples = (FLAC__int64)(spec->value.seconds * (double)sample_rate);
		spec->value_is_samples = true;
	}
}

FLAC__bool flac__utils_parse_cue_specification(const char *s, utils__CueSpecification *spec)
{
	const char *start = s, *end = 0;

	FLAC__ASSERT(0 != spec);

	spec->has_start_point = spec->has_end_point = false;

	s = strchr(s, '-');

	if(0 != s) {
		if(s == start)
			start = 0;
		end = s+1;
		if(*end == '\0')
			end = 0;
	}

	if(start) {
		if(!local__parse_cue_(start, s, &spec->start_track, &spec->start_index))
			return false;
		spec->has_start_point = true;
	}

	if(end) {
		if(!local__parse_cue_(end, 0, &spec->end_track, &spec->end_index))
			return false;
		spec->has_end_point = true;
	}

	return true;
}

void flac__utils_canonicalize_cue_specification(const utils__CueSpecification *cue_spec, const FLAC__StreamMetadata_CueSheet *cuesheet, FLAC__uint64 total_samples, utils__SkipUntilSpecification *skip_spec, utils__SkipUntilSpecification *until_spec)
{
	FLAC__ASSERT(0 != cue_spec);
	FLAC__ASSERT(0 != cuesheet);
	FLAC__ASSERT(0 != total_samples);
	FLAC__ASSERT(0 != skip_spec);
	FLAC__ASSERT(0 != until_spec);

	skip_spec->is_relative = false;
	skip_spec->value_is_samples = true;

	until_spec->is_relative = false;
	until_spec->value_is_samples = true;

	if(cue_spec->has_start_point)
		skip_spec->value.samples = local__find_closest_cue_(cuesheet, cue_spec->start_track, cue_spec->start_index, total_samples, /*look_forward=*/false);
	else
		skip_spec->value.samples = 0;

	if(cue_spec->has_end_point)
		until_spec->value.samples = local__find_closest_cue_(cuesheet, cue_spec->end_track, cue_spec->end_index, total_samples, /*look_forward=*/true);
	else
		until_spec->value.samples = total_samples;
}

FLAC__bool flac__utils_set_channel_mask_tag(FLAC__StreamMetadata *object, FLAC__uint32 channel_mask)
{
	FLAC__StreamMetadata_VorbisComment_Entry entry = { 0, 0 };
	char tag[128];

	FLAC__ASSERT(object);
	FLAC__ASSERT(object->type == FLAC__METADATA_TYPE_VORBIS_COMMENT);
	FLAC__ASSERT(strlen(CHANNEL_MASK_TAG)+1+2+16+1 <= sizeof(tag)); /* +1 for =, +2 for 0x, +16 for digits, +1 for NUL */
	entry.entry = (FLAC__byte*)tag;
	if((entry.length = flac_snprintf(tag, sizeof(tag), "%s=0x%04X", CHANNEL_MASK_TAG, (uint32_t)channel_mask)) >= sizeof(tag))
		return false;
	if(!FLAC__metadata_object_vorbiscomment_replace_comment(object, entry, /*all=*/true, /*copy=*/true))
		return false;
	return true;
}

FLAC__bool flac__utils_get_channel_mask_tag(const FLAC__StreamMetadata *object, FLAC__uint32 *channel_mask)
{
	int offset;
	uint32_t val;
	char *p;
	FLAC__ASSERT(object);
	FLAC__ASSERT(object->type == FLAC__METADATA_TYPE_VORBIS_COMMENT);
	if(0 > (offset = FLAC__metadata_object_vorbiscomment_find_entry_from(object, /*offset=*/0, CHANNEL_MASK_TAG)))
		return false;
	if(object->data.vorbis_comment.comments[offset].length < strlen(CHANNEL_MASK_TAG)+4)
		return false;
	if(0 == (p = strchr((const char *)object->data.vorbis_comment.comments[offset].entry, '='))) /* should never happen, but just in case */
		return false;
	if(FLAC__STRNCASECMP(p, "=0x", 3))
		return false;
	if(sscanf(p+3, "%x", &val) != 1)
		return false;
	*channel_mask = val;
	return true;
}
