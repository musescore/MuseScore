/* flac - Command-line FLAC encoder/decoder
 * Copyright (C) 2000-2009  Josh Coalson
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

#ifndef flac__encode_h
#define flac__encode_h

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "FLAC/metadata.h"
#include "foreign_metadata.h"
#include "utils.h"
#include "share/compat.h"

extern const int FLAC_ENCODE__DEFAULT_PADDING;

typedef enum {
	CST_BLOCKSIZE,
	CST_COMPRESSION_LEVEL,
	CST_DO_MID_SIDE,
	CST_LOOSE_MID_SIDE,
	CST_APODIZATION,
	CST_MAX_LPC_ORDER,
	CST_QLP_COEFF_PRECISION,
	CST_DO_QLP_COEFF_PREC_SEARCH,
	CST_DO_ESCAPE_CODING,
	CST_DO_EXHAUSTIVE_MODEL_SEARCH,
	CST_MIN_RESIDUAL_PARTITION_ORDER,
	CST_MAX_RESIDUAL_PARTITION_ORDER,
	CST_RICE_PARAMETER_SEARCH_DIST
} compression_setting_type_t;

typedef struct {
	compression_setting_type_t type;
	union {
		FLAC__bool t_bool;
		unsigned t_unsigned;
		const char *t_string;
	} value;
} compression_setting_t;

typedef struct {
	utils__SkipUntilSpecification skip_specification;
	utils__SkipUntilSpecification until_specification;
	FLAC__bool verify;
#if FLAC__HAS_OGG
	FLAC__bool use_ogg;
	long serial_number;
#endif
	FLAC__bool lax;
	int padding;
	size_t num_compression_settings;
	compression_setting_t compression_settings[64];
	char *requested_seek_points;
	int num_requested_seek_points;
	const char *cuesheet_filename;
	FLAC__bool treat_warnings_as_errors;
	FLAC__bool continue_through_decode_errors; /* currently only obeyed when encoding from FLAC or Ogg FLAC */
	FLAC__bool cued_seekpoints;
	FLAC__bool channel_map_none; /* --channel-map=none specified, eventually will expand to take actual channel map */

	/* options related to --replay-gain and --sector-align */
	FLAC__bool is_first_file;
	FLAC__bool is_last_file;
	FLAC__int32 **align_reservoir;
	unsigned *align_reservoir_samples;
	FLAC__bool replay_gain;
	FLAC__bool ignore_chunk_sizes;
	FLAC__bool sector_align;
	FLAC__bool error_on_compression_fail;

	FLAC__StreamMetadata *vorbis_comment;
	FLAC__StreamMetadata *pictures[64];
	unsigned num_pictures;

	FileFormat format;
	union {
		struct {
			FLAC__bool is_big_endian;
			FLAC__bool is_unsigned_samples;
			unsigned channels;
			unsigned bps;
			unsigned sample_rate;
		} raw;
		struct {
			foreign_metadata_t *foreign_metadata; /* NULL unless --keep-foreign-metadata requested */
		} iff;
	} format_options;

	struct {
		FLAC__bool disable_constant_subframes;
		FLAC__bool disable_fixed_subframes;
		FLAC__bool disable_verbatim_subframes;
		FLAC__bool do_md5;
	} debug;
} encode_options_t;

int flac__encode_file(FILE *infile, FLAC__off_t infilesize, const char *infilename, const char *outfilename, const FLAC__byte *lookahead, uint32_t lookahead_length, encode_options_t options);

#endif
