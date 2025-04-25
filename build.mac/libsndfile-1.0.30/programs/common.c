/*
** Copyright (C) 1999-2019 Erik de Castro Lopo <erikd@mega-nerd.com>
** Copyright (C) 2008 George Blood Audio
**
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in
**       the documentation and/or other materials provided with the
**       distribution.
**     * Neither the author nor the names of any contributors may be used
**       to endorse or promote products derived from this software without
**       specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
** TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
** CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
** EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
** PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
** OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
** WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
** OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
** ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <math.h>

#include <sndfile.h>

#include "common.h"

#define	BUFFER_LEN	4096

#define	MIN(x, y)	((x) < (y) ? (x) : (y))

int
sfe_copy_data_fp (SNDFILE *outfile, SNDFILE *infile, int channels, int normalize)
{	static double	data [BUFFER_LEN], max ;
	sf_count_t		frames, readcount, k ;

	frames = BUFFER_LEN / channels ;
	readcount = frames ;

	sf_command (infile, SFC_CALC_SIGNAL_MAX, &max, sizeof (max)) ;
	if (!isnormal (max)) /* neither zero, subnormal, infinite, nor NaN */
		return 1 ;

	if (!normalize && max < 1.0)
	{	while (readcount > 0)
		{	readcount = sf_readf_double (infile, data, frames) ;
			sf_writef_double (outfile, data, readcount) ;
			} ;
		}
	else
	{	sf_command (infile, SFC_SET_NORM_DOUBLE, NULL, SF_FALSE) ;

		while (readcount > 0)
		{	readcount = sf_readf_double (infile, data, frames) ;
			for (k = 0 ; k < readcount * channels ; k++)
			{	data [k] /= max ;

				if (!isfinite (data [k])) /* infinite or NaN */
					return 1;
				}
			sf_writef_double (outfile, data, readcount) ;
			} ;
		} ;

	return 0 ;
} /* sfe_copy_data_fp */

void
sfe_copy_data_int (SNDFILE *outfile, SNDFILE *infile, int channels)
{	static int	data [BUFFER_LEN] ;
	int		frames, readcount ;

	frames = BUFFER_LEN / channels ;
	readcount = frames ;

	while (readcount > 0)
	{	readcount = sf_readf_int (infile, data, frames) ;
		sf_writef_int (outfile, data, readcount) ;
		} ;

	return ;
} /* sfe_copy_data_int */

/*==============================================================================
*/

static int
merge_broadcast_info (SNDFILE * infile, SNDFILE * outfile, int format, const METADATA_INFO * info)
{	SF_BROADCAST_INFO_2K binfo ;
	int infileminor ;

	memset (&binfo, 0, sizeof (binfo)) ;

	if ((SF_FORMAT_TYPEMASK & format) != SF_FORMAT_WAV)
	{	printf ("Error : This is not a WAV file and hence broadcast info cannot be added to it.\n\n") ;
		return 1 ;
		} ;

	infileminor = SF_FORMAT_SUBMASK & format ;

	switch (infileminor)
	{	case SF_FORMAT_PCM_16 :
		case SF_FORMAT_PCM_24 :
		case SF_FORMAT_PCM_32 :
			break ;

		default :
			printf (
				"Warning : The EBU Technical Recommendation R68-2000 states that the only\n"
				"          allowed encodings are Linear PCM and MPEG3. This file is not in\n"
				"          the right format.\n\n"
				) ;
			break ;
		} ;

	if (sf_command (infile, SFC_GET_BROADCAST_INFO, &binfo, sizeof (binfo)) == 0)
	{	if (infile == outfile)
		{	printf (
				"Error : Attempting in-place broadcast info update, but file does not\n"
				"        have a 'bext' chunk to modify. The solution is to specify both\n"
				"        input and output files on the command line.\n\n"
				) ;
			return 1 ;
			} ;
		} ;

#define REPLACE_IF_NEW(x) \
		if (info->x != NULL) \
		{	memset (binfo.x, 0, sizeof (binfo.x)) ; \
			memcpy (binfo.x, info->x, MIN (strlen (info->x), sizeof (binfo.x))) ; \
			} ;

	REPLACE_IF_NEW (description) ;
	REPLACE_IF_NEW (originator) ;
	REPLACE_IF_NEW (originator_reference) ;
	REPLACE_IF_NEW (origination_date) ;
	REPLACE_IF_NEW (origination_time) ;
	REPLACE_IF_NEW (umid) ;

	/* Special case loudness values */
#define REPLACE_IF_NEW_INT(x) \
		if (info->x != NULL) \
		{	binfo.x = round (atof (info->x) * 100.0) ; \
			} ;

	REPLACE_IF_NEW_INT (loudness_value) ;
	REPLACE_IF_NEW_INT (loudness_range) ;
	REPLACE_IF_NEW_INT (max_true_peak_level) ;
	REPLACE_IF_NEW_INT (max_momentary_loudness) ;
	REPLACE_IF_NEW_INT (max_shortterm_loudness) ;

	/* Special case for Time Ref. */
	if (info->time_ref != NULL)
	{	uint64_t ts = atoll (info->time_ref) ;

		binfo.time_reference_high = (ts >> 32) ;
		binfo.time_reference_low = (ts & 0xffffffff) ;
		} ;

	/* Special case for coding_history because we may want to append. */
	if (info->coding_history != NULL)
	{	if (info->coding_hist_append)
		{	int slen = strlen (binfo.coding_history) ;

			while (slen > 1 && isspace (binfo.coding_history [slen - 1]))
				slen -- ;

			memcpy (binfo.coding_history + slen, info->coding_history, sizeof (binfo.coding_history) - slen) ;
			}
		else
		{	size_t slen = MIN (strlen (info->coding_history), sizeof (binfo.coding_history)) ;

			memset (binfo.coding_history, 0, sizeof (binfo.coding_history)) ;
			memcpy (binfo.coding_history, info->coding_history, slen) ;
			binfo.coding_history_size = slen ;
			} ;
		} ;

	if (sf_command (outfile, SFC_SET_BROADCAST_INFO, &binfo, sizeof (binfo)) == 0)
	{	printf ("Error : Setting of broadcast info chunks failed.\n\n") ;
		return 1 ;
		} ;

	return 0 ;
} /* merge_broadcast_info*/

static void
update_strings (SNDFILE * outfile, const METADATA_INFO * info)
{
	if (info->title != NULL)
		sf_set_string (outfile, SF_STR_TITLE, info->title) ;

	if (info->copyright != NULL)
		sf_set_string (outfile, SF_STR_COPYRIGHT, info->copyright) ;

	if (info->artist != NULL)
		sf_set_string (outfile, SF_STR_ARTIST, info->artist) ;

	if (info->comment != NULL)
		sf_set_string (outfile, SF_STR_COMMENT, info->comment) ;

	if (info->date != NULL)
		sf_set_string (outfile, SF_STR_DATE, info->date) ;

	if (info->album != NULL)
		sf_set_string (outfile, SF_STR_ALBUM, info->album) ;

	if (info->license != NULL)
		sf_set_string (outfile, SF_STR_LICENSE, info->license) ;

} /* update_strings */



void
sfe_apply_metadata_changes (const char * filenames [2], const METADATA_INFO * info)
{	SNDFILE *infile = NULL, *outfile = NULL ;
	SF_INFO sfinfo ;
	METADATA_INFO tmpinfo ;
	int error_code = 0 ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;
	memset (&tmpinfo, 0, sizeof (tmpinfo)) ;

	if (filenames [1] == NULL)
		infile = outfile = sf_open (filenames [0], SFM_RDWR, &sfinfo) ;
	else
	{	infile = sf_open (filenames [0], SFM_READ, &sfinfo) ;

		/* Output must be WAV. */
		sfinfo.format = SF_FORMAT_WAV | (SF_FORMAT_SUBMASK & sfinfo.format) ;
		outfile = sf_open (filenames [1], SFM_WRITE, &sfinfo) ;
		} ;

	if (infile == NULL)
	{	printf ("Error : Not able to open input file '%s' : %s\n", filenames [0], sf_strerror (infile)) ;
		error_code = 1 ;
		goto cleanup_exit ;
		} ;

	if (outfile == NULL)
	{	printf ("Error : Not able to open output file '%s' : %s\n", filenames [1], sf_strerror (outfile)) ;
		error_code = 1 ;
		goto cleanup_exit ;
		} ;

	if (info->has_bext_fields && merge_broadcast_info (infile, outfile, sfinfo.format, info))
	{	error_code = 1 ;
		goto cleanup_exit ;
		} ;

	if (infile != outfile)
	{	int infileminor = SF_FORMAT_SUBMASK & sfinfo.format ;

		/* If the input file is not the same as the output file, copy the data. */
		if ((infileminor == SF_FORMAT_DOUBLE) || (infileminor == SF_FORMAT_FLOAT))
		{	if (sfe_copy_data_fp (outfile, infile, sfinfo.channels, SF_FALSE) != 0)
			{	printf ("Error : Not able to decode input file '%s'\n", filenames [0]) ;
				error_code = 1 ;
				goto cleanup_exit ;
				} ;
			}
		else
			sfe_copy_data_int (outfile, infile, sfinfo.channels) ;
		} ;

	update_strings (outfile, info) ;

cleanup_exit :

	if (outfile != NULL && outfile != infile)
		sf_close (outfile) ;

	if (infile != NULL)
		sf_close (infile) ;

	if (error_code)
		exit (error_code) ;

	return ;
} /* sfe_apply_metadata_changes */

/*==============================================================================
*/

typedef struct
{	const char	*ext ;
	int			len ;
	int			format ;
} OUTPUT_FORMAT_MAP ;

/* Map a file name extension to a container format. */
static OUTPUT_FORMAT_MAP format_map [] =
{
	{	"wav", 		0,	SF_FORMAT_WAV	},
	{	"aif",		3,	SF_FORMAT_AIFF	},
	{	"au",		0,	SF_FORMAT_AU	},
	{	"snd",		0,	SF_FORMAT_AU	},
	{	"raw",		0,	SF_FORMAT_RAW	},
	{	"gsm",		0,	SF_FORMAT_RAW | SF_FORMAT_GSM610 },
	{	"vox",		0, 	SF_FORMAT_RAW | SF_FORMAT_VOX_ADPCM },
	{	"paf",		0,	SF_FORMAT_PAF | SF_ENDIAN_BIG },
	{	"fap",		0,	SF_FORMAT_PAF | SF_ENDIAN_LITTLE },
	{	"svx",		0,	SF_FORMAT_SVX	},
	{	"nist", 	0,	SF_FORMAT_NIST	},
	{	"sph",		0,	SF_FORMAT_NIST	},
	{	"voc",		0, 	SF_FORMAT_VOC	},
	{	"ircam",	0,	SF_FORMAT_IRCAM	},
	{	"sf",		0, 	SF_FORMAT_IRCAM	},
	{	"w64", 		0, 	SF_FORMAT_W64	},
	{	"mat",		0, 	SF_FORMAT_MAT4 	},
	{	"mat4", 	0,	SF_FORMAT_MAT4	},
	{	"mat5", 	0, 	SF_FORMAT_MAT5 	},
	{	"pvf",		0, 	SF_FORMAT_PVF 	},
	{	"xi",		0, 	SF_FORMAT_XI 	},
	{	"htk",		0,	SF_FORMAT_HTK	},
	{	"sds",		0, 	SF_FORMAT_SDS 	},
	{	"avr",		0, 	SF_FORMAT_AVR 	},
	{	"wavex",	0, 	SF_FORMAT_WAVEX },
	{	"sd2",		0, 	SF_FORMAT_SD2 	},
	{	"flac",		0,	SF_FORMAT_FLAC	},
	{	"caf",		0,	SF_FORMAT_CAF	},
	{	"wve",		0,	SF_FORMAT_WVE	},
	{	"prc",		0,	SF_FORMAT_WVE	},
	{	"ogg",		0,	SF_FORMAT_OGG	},
	{	"oga",		0,	SF_FORMAT_OGG	},
	{	"opus",		0,	SF_FORMAT_OGG | SF_FORMAT_OPUS },
	{	"mpc",		0,	SF_FORMAT_MPC2K	},
	{	"rf64",		0,	SF_FORMAT_RF64	},
} ; /* format_map */

int
sfe_file_type_of_ext (const char *str, int format)
{	char	buffer [16], *cptr ;
	int		k ;

	format &= SF_FORMAT_SUBMASK ;

	if ((cptr = strrchr (str, '.')) == NULL)
		return 0 ;

	strncpy (buffer, cptr + 1, 15) ;
	buffer [15] = 0 ;

	for (k = 0 ; buffer [k] ; k++)
		buffer [k] = tolower ((buffer [k])) ;

	for (k = 0 ; k < (int) (sizeof (format_map) / sizeof (format_map [0])) ; k++)
	{	if ((format_map [k].len > 0 && strncmp (buffer, format_map [k].ext, format_map [k].len) == 0) ||
			(strcmp (buffer, format_map [k].ext) == 0))
		{	if (format_map [k].format & SF_FORMAT_SUBMASK)
				return format_map [k].format ;
			else
				return format_map [k].format | format ;
			} ;
		} ;

	/* Default if all the above fails. */
	return (SF_FORMAT_WAV | SF_FORMAT_PCM_24) ;
} /* sfe_file_type_of_ext */

void
sfe_dump_format_map (void)
{	SF_FORMAT_INFO	info ;
	int k ;

	for (k = 0 ; k < ARRAY_LEN (format_map) ; k++)
	{	info.format = format_map [k].format ;
		sf_command (NULL, SFC_GET_FORMAT_INFO, &info, sizeof (info)) ;
		printf ("        %-10s : %s", format_map [k].ext, info.name == NULL ? "????" : info.name) ;
		if (format_map [k].format & SF_FORMAT_SUBMASK)
		{	info.format = format_map [k].format & SF_FORMAT_SUBMASK ;
			sf_command (NULL, SFC_GET_FORMAT_INFO, &info, sizeof (info)) ;
			printf (" %s", info.name == NULL ? "????" : info.name) ;
			} ;
		putchar ('\n') ;

		} ;

} /* sfe_dump_format_map */

const char *
program_name (const char * argv0)
{	const char * tmp ;

	tmp = strrchr (argv0, '/') ;
	argv0 = tmp ? tmp + 1 : argv0 ;

	/* Remove leading libtool name mangling. */
	if (strstr (argv0, "lt-") == argv0)
		return argv0 + 3 ;

	return argv0 ;
} /* program_name */

const char *
sfe_endian_name (int format)
{
	switch (format & SF_FORMAT_ENDMASK)
	{	case SF_ENDIAN_FILE : return "file" ;
		case SF_ENDIAN_LITTLE : return "little" ;
		case SF_ENDIAN_BIG : return "big" ;
		case SF_ENDIAN_CPU : return "cpu" ;
		default : break ;
		} ;

	return "unknown" ;
} /* sfe_endian_name */

const char *
sfe_container_name (int format)
{
	switch (format & SF_FORMAT_TYPEMASK)
	{	case SF_FORMAT_WAV : return "WAV" ;
		case SF_FORMAT_AIFF : return "AIFF" ;
		case SF_FORMAT_AU : return "AU" ;
		case SF_FORMAT_RAW : return "RAW" ;
		case SF_FORMAT_PAF : return "PAF" ;
		case SF_FORMAT_SVX : return "SVX" ;
		case SF_FORMAT_NIST : return "NIST" ;
		case SF_FORMAT_VOC : return "VOC" ;
		case SF_FORMAT_IRCAM : return "IRCAM" ;
		case SF_FORMAT_W64 : return "W64" ;
		case SF_FORMAT_MAT4 : return "MAT4" ;
		case SF_FORMAT_MAT5 : return "MAT5" ;
		case SF_FORMAT_PVF : return "PVF" ;
		case SF_FORMAT_XI : return "XI" ;
		case SF_FORMAT_HTK : return "HTK" ;
		case SF_FORMAT_SDS : return "SDS" ;
		case SF_FORMAT_AVR : return "AVR" ;
		case SF_FORMAT_WAVEX : return "WAVEX" ;
		case SF_FORMAT_SD2 : return "SD2" ;
		case SF_FORMAT_FLAC : return "FLAC" ;
		case SF_FORMAT_CAF : return "CAF" ;
		case SF_FORMAT_WVE : return "WVE" ;
		case SF_FORMAT_OGG : return "OGG" ;
		case SF_FORMAT_MPC2K : return "MPC2K" ;
		case SF_FORMAT_RF64 : return "RF64" ;
		default : break ;
		} ;

	return "unknown" ;
} /* sfe_container_name */

const char *
sfe_codec_name (int format)
{
	switch (format & SF_FORMAT_SUBMASK)
	{	case SF_FORMAT_PCM_S8 : return "signed 8 bit PCM" ;
		case SF_FORMAT_PCM_16 : return "16 bit PCM" ;
		case SF_FORMAT_PCM_24 : return "24 bit PCM" ;
		case SF_FORMAT_PCM_32 : return "32 bit PCM" ;
		case SF_FORMAT_PCM_U8 : return "unsigned 8 bit PCM" ;
		case SF_FORMAT_FLOAT : return "32 bit float" ;
		case SF_FORMAT_DOUBLE : return "64 bit double" ;
		case SF_FORMAT_ULAW : return "u-law" ;
		case SF_FORMAT_ALAW : return "a-law" ;
		case SF_FORMAT_IMA_ADPCM : return "IMA ADPCM" ;
		case SF_FORMAT_MS_ADPCM : return "MS ADPCM" ;
		case SF_FORMAT_GSM610 : return "gsm610" ;
		case SF_FORMAT_VOX_ADPCM : return "Vox ADPCM" ;
		case SF_FORMAT_G721_32 : return "g721 32kbps" ;
		case SF_FORMAT_G723_24 : return "g723 24kbps" ;
		case SF_FORMAT_G723_40 : return "g723 40kbps" ;
		case SF_FORMAT_DWVW_12 : return "12 bit DWVW" ;
		case SF_FORMAT_DWVW_16 : return "16 bit DWVW" ;
		case SF_FORMAT_DWVW_24 : return "14 bit DWVW" ;
		case SF_FORMAT_DWVW_N : return "DWVW" ;
		case SF_FORMAT_DPCM_8 : return "8 bit DPCM" ;
		case SF_FORMAT_DPCM_16 : return "16 bit DPCM" ;
		case SF_FORMAT_VORBIS : return "Vorbis" ;
		case SF_FORMAT_ALAC_16 : return "16 bit ALAC" ;
		case SF_FORMAT_ALAC_20 : return "20 bit ALAC" ;
		case SF_FORMAT_ALAC_24 : return "24 bit ALAC" ;
		case SF_FORMAT_ALAC_32 : return "32 bit ALAC" ;
		case SF_FORMAT_OPUS : return "Opus" ;
		default : break ;
		} ;
	return "unknown" ;
} /* sfe_codec_name */
