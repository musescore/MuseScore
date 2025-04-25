[+ AutoGen5 template c +]
/*
** Copyright (C) 1999-2012 Erik de Castro Lopo <erikd@mega-nerd.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "sfconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#else
#include "sf_unistd.h"
#endif

#include <sndfile.h>

#include "utils.h"

#ifndef		M_PI
#define		M_PI	3.14159265358979323846264338
#endif

#define	HALF_BUFFER_SIZE	(1 << 12)
#define	BUFFER_SIZE			(2 * HALF_BUFFER_SIZE)

#define	SINE_AMP		1.1
#define	MAX_ERROR		0.0202

[+ FOR float_type +]
[+ FOR data_type
+]static void	[+ (get "float_short_name") +]_scale_clip_test_[+ (get "name") +] (const char *filename, int filetype, float maxval) ;
[+ ENDFOR data_type
+][+ ENDFOR float_type +]

[+ FOR float_type +]
[+ FOR int_type
+]static void [+ (get "float_short_name") +]_[+ (get "int_type_name") +]_clip_read_test (const char *filename, int filetype) ;
[+ ENDFOR int_type
+][+ ENDFOR float_type +]

[+ FOR int_type +]
[+ FOR float_type
+]static void [+ (get "int_type_name") +]_[+ (get "float_short_name") +]_scale_write_test (const char *filename, int filetype) ;
[+ ENDFOR float_type
+][+ ENDFOR int_type +]

typedef union
{	double	dbl [BUFFER_SIZE] ;
	float	flt [BUFFER_SIZE] ;
	int		i [BUFFER_SIZE] ;
	short	s [BUFFER_SIZE] ;
} BUFFER ;

/* Data buffer. */
static	BUFFER	buffer_out ;
static	BUFFER	buffer_in ;

int
main (void)
{
	flt_scale_clip_test_08 ("scale_clip_s8.au", SF_FORMAT_AU | SF_FORMAT_PCM_S8, 1.0 * 0x80) ;
	flt_scale_clip_test_08 ("scale_clip_u8.wav", SF_FORMAT_WAV | SF_FORMAT_PCM_U8, 1.0 * 0x80) ;

	dbl_scale_clip_test_08 ("scale_clip_s8.au", SF_FORMAT_AU | SF_FORMAT_PCM_S8, 1.0 * 0x80) ;
	dbl_scale_clip_test_08 ("scale_clip_u8.wav", SF_FORMAT_WAV | SF_FORMAT_PCM_U8, 1.0 * 0x80) ;

	/*
	**	Now use SF_FORMAT_AU where possible because it allows both
	**	big and little endian files.
	*/

	flt_scale_clip_test_16 ("scale_clip_be16.au", SF_ENDIAN_BIG	| SF_FORMAT_AU | SF_FORMAT_PCM_16, 1.0 * 0x8000) ;
	flt_scale_clip_test_16 ("scale_clip_le16.au", SF_ENDIAN_LITTLE | SF_FORMAT_AU | SF_FORMAT_PCM_16, 1.0 * 0x8000) ;
	flt_scale_clip_test_24 ("scale_clip_be24.au", SF_ENDIAN_BIG	| SF_FORMAT_AU | SF_FORMAT_PCM_24, 1.0 * 0x800000) ;
	flt_scale_clip_test_24 ("scale_clip_le24.au", SF_ENDIAN_LITTLE	| SF_FORMAT_AU | SF_FORMAT_PCM_24, 1.0 * 0x800000) ;
	flt_scale_clip_test_32 ("scale_clip_be32.au", SF_ENDIAN_BIG	| SF_FORMAT_AU | SF_FORMAT_PCM_32, 1.0 * 0x80000000) ;
	flt_scale_clip_test_32 ("scale_clip_le32.au", SF_ENDIAN_LITTLE	| SF_FORMAT_AU | SF_FORMAT_PCM_32, 1.0 * 0x80000000) ;

	dbl_scale_clip_test_16 ("scale_clip_be16.au", SF_ENDIAN_BIG	| SF_FORMAT_AU | SF_FORMAT_PCM_16, 1.0 * 0x8000) ;
	dbl_scale_clip_test_16 ("scale_clip_le16.au", SF_ENDIAN_LITTLE	| SF_FORMAT_AU | SF_FORMAT_PCM_16, 1.0 * 0x8000) ;
	dbl_scale_clip_test_24 ("scale_clip_be24.au", SF_ENDIAN_BIG	| SF_FORMAT_AU | SF_FORMAT_PCM_24, 1.0 * 0x800000) ;
	dbl_scale_clip_test_24 ("scale_clip_le24.au", SF_ENDIAN_LITTLE	| SF_FORMAT_AU | SF_FORMAT_PCM_24, 1.0 * 0x800000) ;
	dbl_scale_clip_test_32 ("scale_clip_be32.au", SF_ENDIAN_BIG	| SF_FORMAT_AU | SF_FORMAT_PCM_32, 1.0 * 0x80000000) ;
	dbl_scale_clip_test_32 ("scale_clip_le32.au", SF_ENDIAN_LITTLE	| SF_FORMAT_AU | SF_FORMAT_PCM_32, 1.0 * 0x80000000) ;

	flt_short_clip_read_test	("flt_short.au"	, SF_ENDIAN_BIG		| SF_FORMAT_AU | SF_FORMAT_FLOAT) ;
	flt_int_clip_read_test		("flt_int.au"	, SF_ENDIAN_LITTLE	| SF_FORMAT_AU | SF_FORMAT_FLOAT) ;
	dbl_short_clip_read_test	("dbl_short.au"	, SF_ENDIAN_BIG		| SF_FORMAT_AU | SF_FORMAT_DOUBLE) ;
	dbl_int_clip_read_test		("dbl_int.au"	, SF_ENDIAN_LITTLE	| SF_FORMAT_AU | SF_FORMAT_DOUBLE) ;

	short_flt_scale_write_test	("short_flt.au"	, SF_ENDIAN_BIG		| SF_FORMAT_AU | SF_FORMAT_FLOAT) ;
	int_flt_scale_write_test	("int_flt.au"	, SF_ENDIAN_LITTLE	| SF_FORMAT_AU | SF_FORMAT_FLOAT) ;
	short_dbl_scale_write_test	("short_dbl.au"	, SF_ENDIAN_BIG		| SF_FORMAT_AU | SF_FORMAT_DOUBLE) ;
	int_dbl_scale_write_test	("int_dbl.au"	, SF_ENDIAN_LITTLE	| SF_FORMAT_AU | SF_FORMAT_DOUBLE) ;

	return 0 ;
} /* main */

/*============================================================================================
**	Here are the test functions.
*/

[+ FOR float_type +]
[+ FOR data_type
+]static void
[+ (get "float_short_name") +]_scale_clip_test_[+ (get "name") +] (const char *filename, int filetype, float maxval)
{	SNDFILE		*file ;
	SF_INFO		sfinfo ;
	int			k ;
	[+ (get "float_type_name") +]		*data_out, *data_in ;
	double		diff, clip_max_diff ;

	print_test_name ("[+ (get "float_short_name") +]_scale_clip_test_[+ (get "name") +]", filename) ;

	data_out = buffer_out.[+ (get "float_short_name") +] ;
	data_in = buffer_in.[+ (get "float_short_name") +] ;

	for (k = 0 ; k < HALF_BUFFER_SIZE ; k++)
	{	data_out [k] = 1.2 * sin (2 * M_PI * k / HALF_BUFFER_SIZE) ;
		data_out [k + HALF_BUFFER_SIZE] = data_out [k] * maxval ;
		} ;

	sfinfo.samplerate	= 44100 ;
	sfinfo.frames		= 123456789 ; /* Wrong length. Library should correct this on sf_close. */
	sfinfo.channels		= 1 ;
	sfinfo.format		= filetype ;

	/*
	**	Write two versions of the data:
	**		normalized and clipped
	**		un-normalized and clipped.
	*/

	file = test_open_file_or_die (filename, SFM_WRITE, &sfinfo, SF_TRUE, __LINE__) ;
	sf_command (file, SFC_SET_CLIPPING, NULL, SF_TRUE) ;
	test_write_[+ (get "float_type_name") +]_or_die (file, 0, data_out, HALF_BUFFER_SIZE, __LINE__) ;
	sf_command (file, SFC_SET_NORM_[+ (get "float_upper_name") +], NULL, SF_FALSE) ;
	test_write_[+ (get "float_type_name") +]_or_die (file, 0, data_out + HALF_BUFFER_SIZE, HALF_BUFFER_SIZE, __LINE__) ;
	sf_close (file) ;

	memset (&buffer_in, 0, sizeof (buffer_in)) ;

	file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_TRUE, __LINE__) ;

	sfinfo.format &= (SF_FORMAT_TYPEMASK | SF_FORMAT_SUBMASK) ;

	if (sfinfo.format != (filetype & (SF_FORMAT_TYPEMASK | SF_FORMAT_SUBMASK)))
	{	printf ("\n\nLine %d: Returned format incorrect (0x%08X => 0x%08X).\n\n", __LINE__, filetype, sfinfo.format) ;
		exit (1) ;
		} ;

	if (sfinfo.frames != BUFFER_SIZE)
	{	printf ("\n\nLine %d: Incorrect number of frames in file (%d => %" PRId64 ").\n\n", __LINE__, BUFFER_SIZE, sfinfo.frames) ;
		exit (1) ;
		} ;

	if (sfinfo.channels != 1)
	{	printf ("\n\nLine %d: Incorrect number of channels in file.\n\n", __LINE__) ;
		exit (1) ;
		} ;

	check_log_buffer_or_die (file, __LINE__) ;

	test_read_[+ (get "float_type_name") +]_or_die (file, 0, data_in, HALF_BUFFER_SIZE, __LINE__) ;
	sf_command (file, SFC_SET_NORM_[+ (get "float_upper_name") +], NULL, SF_FALSE) ;
	test_read_[+ (get "float_type_name") +]_or_die (file, 0, data_in + HALF_BUFFER_SIZE, HALF_BUFFER_SIZE, __LINE__) ;
	sf_close (file) ;

	/* Check normalized version. */
	clip_max_diff = 0.0 ;
	for (k = 0 ; k < HALF_BUFFER_SIZE ; k++)
	{	if (fabs (data_in [k]) > 1.0)
		{	printf ("\n\nLine %d: Input sample %d/%d (%f) has not been clipped.\n\n", __LINE__, k, BUFFER_SIZE, data_in [k]) ;
			exit (1) ;
			} ;

		if (data_out [k] * data_in [k] < 0.0)
		{	printf ("\n\nLine %d: Data wrap around at index %d/%d.\n\n", __LINE__, k, BUFFER_SIZE) ;
			exit (1) ;
			} ;

		if (fabs (data_out [k]) > 1.0)
			continue ;

		diff = fabs (data_out [k] - data_in [k]) ;
		if (diff > clip_max_diff)
			clip_max_diff = diff ;
		} ;

	if (clip_max_diff < 1e-20)
	{	printf ("\n\nLine %d: Clipping difference (%e) too small (normalized).\n\n", __LINE__, clip_max_diff) ;
		exit (1) ;
		} ;

	if (clip_max_diff > [+ (get "error_val") +])
	{	printf ("\n\nLine %d: Clipping difference (%e) too large (normalized).\n\n", __LINE__, clip_max_diff) ;
		exit (1) ;
		} ;

	/* Check the un-normalized data. */
	clip_max_diff = 0.0 ;
	for (k = HALF_BUFFER_SIZE ; k < BUFFER_SIZE ; k++)
	{	if (fabs (data_in [k]) > maxval)
		{	printf ("\n\nLine %d: Input sample %d/%d (%f) has not been clipped.\n\n", __LINE__, k, BUFFER_SIZE, data_in [k]) ;
			exit (1) ;
			} ;

		if (data_out [k] * data_in [k] < 0.0)
		{	printf ("\n\nLine %d: Data wrap around at index %d/%d.\n\n", __LINE__, k, BUFFER_SIZE) ;
			exit (1) ;
			} ;

		if (fabs (data_out [k]) > maxval)
			continue ;

		diff = fabs (data_out [k] - data_in [k]) ;
		if (diff > clip_max_diff)
			clip_max_diff = diff ;
		} ;

	if (clip_max_diff < 1e-20)
	{	printf ("\n\nLine %d: Clipping difference (%e) too small (un-normalized).\n\n", __LINE__, clip_max_diff) ;
		exit (1) ;
		} ;

	if (clip_max_diff > 1.0)
	{	printf ("\n\nLine %d: Clipping difference (%e) too large (un-normalised).\n\n", __LINE__, clip_max_diff) ;
		exit (1) ;
		} ;

	printf ("ok\n") ;
	unlink (filename) ;
} /* [+ (get "float_short_name") +]_scale_clip_test_[+ (get "name") +] */

[+ ENDFOR data_type
+]
[+ ENDFOR float_type +]

/*==============================================================================
*/

[+ FOR float_type +]
[+ FOR int_type
+]static void [+ (get "float_short_name") +]_[+ (get "int_type_name") +]_clip_read_test (const char *filename, int filetype)
{	SNDFILE		*file ;
	SF_INFO		sfinfo ;
	[+ (get "float_type_name") +]		*data_out ;
	[+ (get "int_type_name") +]			*data_in, max_value ;
	int			k ;

	print_test_name ("[+ (get "float_short_name") +]_[+ (get "int_type_name") +]_clip_read_test", filename) ;

	data_out = buffer_out.[+ (get "float_short_name") +] ;
	data_in = buffer_in.[+ (get "int_short_name") +] ;

	for (k = 0 ; k < BUFFER_SIZE ; k++)
		data_out [k] = 0.995 * sin (4 * M_PI * k / BUFFER_SIZE) ;
	data_out [BUFFER_SIZE / 8] = 1.0 ;
	data_out [3 * BUFFER_SIZE / 8] = -1.000000001 ;
	data_out [5 * BUFFER_SIZE / 8] = 1.0 ;
	data_out [7 * BUFFER_SIZE / 8] = -1.000000001 ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;
	sfinfo.samplerate	= 44100 ;
	sfinfo.frames		= 123456789 ; /* Wrong length. Library should correct this on sf_close. */
	sfinfo.channels		= 1 ;
	sfinfo.format		= filetype ;

	/* Save unclipped data to the file. */
	file = test_open_file_or_die (filename, SFM_WRITE, &sfinfo, SF_TRUE, __LINE__) ;
	test_write_[+ (get "float_type_name") +]_or_die (file, 0, data_out, BUFFER_SIZE, __LINE__) ;
	sf_close (file) ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;

	file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_TRUE, __LINE__) ;
	sf_command (file, SFC_SET_SCALE_FLOAT_INT_READ, NULL, SF_TRUE) ;

	sfinfo.format &= (SF_FORMAT_TYPEMASK | SF_FORMAT_SUBMASK) ;

	if (sfinfo.format != (filetype & (SF_FORMAT_TYPEMASK | SF_FORMAT_SUBMASK)))
	{	printf ("\n\nLine %d: Returned format incorrect (0x%08X => 0x%08X).\n\n", __LINE__, filetype, sfinfo.format) ;
		exit (1) ;
		} ;

	if (sfinfo.frames != BUFFER_SIZE)
	{	printf ("\n\nLine %d: Incorrect number of frames in file (%d => %" PRId64 ").\n\n", __LINE__, BUFFER_SIZE, sfinfo.frames) ;
		exit (1) ;
		} ;

	if (sfinfo.channels != 1)
	{	printf ("\n\nLine %d: Incorrect number of channels in file.\n\n", __LINE__) ;
		exit (1) ;
		} ;

	check_log_buffer_or_die (file, __LINE__) ;

	sf_command (file, SFC_SET_CLIPPING, NULL, SF_TRUE) ;
	test_read_[+ (get "int_type_name") +]_or_die (file, 0, data_in, BUFFER_SIZE, __LINE__) ;
	/*-sf_command (file, SFC_SET_NORM_[+ (get "float_upper_name") +], NULL, SF_FALSE) ;-*/
	sf_close (file) ;

	/* Check the first half. */
	max_value = 0 ;
	for (k = 0 ; k < sfinfo.frames ; k++)
	{	/* Check if data_out has different sign from data_in. */
		if ((data_out [k] < 0.0 && data_in [k] > 0) || (data_out [k] > 0.0 && data_in [k] < 0))
		{	printf ("\n\nLine %d: Data wrap around at index %d/%d  (%f -> %d).\n\n", __LINE__, k, BUFFER_SIZE, data_out [k], data_in [k]) ;
			exit (1) ;
			} ;
		max_value = (max_value > abs (data_in [k])) ? max_value : abs (data_in [k]) ;
		} ;

	unlink (filename) ;
	puts ("ok") ;
} /* [+ (get "float_short_name") +]_[+ (get "int_type_name") +]_clip_read_test */
[+ ENDFOR int_type
+][+ ENDFOR float_type +]

/*==============================================================================
*/

[+ FOR int_type +]
[+ FOR float_type
+]static void [+ (get "int_type_name") +]_[+ (get "float_short_name") +]_scale_write_test (const char *filename, int filetype)
{	SNDFILE		*file ;
	SF_INFO		sfinfo ;
	[+ (get "int_type_name") +]		*data_out ;
	[+ (get "float_type_name") +]	*data_in, max_value ;
	int			k ;

	print_test_name ("[+ (get "int_type_name") +]_[+ (get "float_short_name") +]_clip_write_test", filename) ;

	data_out = buffer_out.[+ (get "int_short_name") +] ;
	data_in = buffer_in.[+ (get "float_short_name") +] ;

	for (k = 0 ; k < BUFFER_SIZE ; k++)
		data_out [k] = [+ (get "float_to_int") +] ([+ (get "int_max_value") +] * 0.995 * sin (4 * M_PI * k / BUFFER_SIZE)) ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;
	sfinfo.samplerate	= 44100 ;
	sfinfo.frames		= 123456789 ; /* Wrong length. Library should correct this on sf_close. */
	sfinfo.channels		= 1 ;
	sfinfo.format		= filetype ;

	file = test_open_file_or_die (filename, SFM_WRITE, &sfinfo, SF_TRUE, __LINE__) ;
	test_write_[+ (get "int_type_name") +]_or_die (file, 0, data_out, BUFFER_SIZE, __LINE__) ;
	sf_command (file, SFC_SET_SCALE_INT_FLOAT_WRITE, NULL, SF_TRUE) ;
	test_write_[+ (get "int_type_name") +]_or_die (file, 0, data_out, BUFFER_SIZE, __LINE__) ;
	sf_command (file, SFC_SET_SCALE_INT_FLOAT_WRITE, NULL, SF_FALSE) ;
	test_write_[+ (get "int_type_name") +]_or_die (file, 0, data_out, BUFFER_SIZE, __LINE__) ;
	sf_close (file) ;

	memset (&sfinfo, 0, sizeof (sfinfo)) ;

	file = test_open_file_or_die (filename, SFM_READ, &sfinfo, SF_TRUE, __LINE__) ;

	sfinfo.format &= (SF_FORMAT_TYPEMASK | SF_FORMAT_SUBMASK) ;

	if (sfinfo.format != (filetype & (SF_FORMAT_TYPEMASK | SF_FORMAT_SUBMASK)))
	{	printf ("\n\nLine %d: Returned format incorrect (0x%08X => 0x%08X).\n\n", __LINE__, filetype, sfinfo.format) ;
		exit (1) ;
		} ;

	if (sfinfo.frames != 3 * BUFFER_SIZE)
	{	printf ("\n\nLine %d: Incorrect number of frames in file (%d => %" PRId64 ").\n\n", __LINE__, 3 * BUFFER_SIZE, sfinfo.frames) ;
		exit (1) ;
		} ;

	if (sfinfo.channels != 1)
	{	printf ("\n\nLine %d: Incorrect number of channels in file.\n\n", __LINE__) ;
		exit (1) ;
		} ;

	check_log_buffer_or_die (file, __LINE__) ;

	/* Check the first section. */
	test_read_[+ (get "float_type_name") +]_or_die (file, 0, data_in, BUFFER_SIZE, __LINE__) ;

	max_value = 0.0 ;
	for (k = 0 ; k < BUFFER_SIZE ; k++)
		max_value = (max_value > fabs (data_in [k])) ? max_value : fabs (data_in [k]) ;

	if (max_value < 1000.0)
	{	printf ("\n\nLine %d: Max value (%f) < 1000.0.\n\n", __LINE__, max_value) ;
		exit (1) ;
		} ;

	/* Check the second section. */
	test_read_[+ (get "float_type_name") +]_or_die (file, 0, data_in, BUFFER_SIZE, __LINE__) ;

	max_value = 0.0 ;
	for (k = 0 ; k < BUFFER_SIZE ; k++)
		max_value = (max_value > fabs (data_in [k])) ? max_value : fabs (data_in [k]) ;

	if (max_value > 1.0)
	{	printf ("\n\nLine %d: Max value (%f) > 1.0.\n\n", __LINE__, max_value) ;
		exit (1) ;
		} ;

	/* Check the third section. */
	test_read_[+ (get "float_type_name") +]_or_die (file, 0, data_in, BUFFER_SIZE, __LINE__) ;

	max_value = 0.0 ;
	for (k = 0 ; k < BUFFER_SIZE ; k++)
		max_value = (max_value > fabs (data_in [k])) ? max_value : fabs (data_in [k]) ;

	if (max_value < 1000.0)
	{	printf ("\n\nLine %d: Max value (%f) < 1000.0.\n\n", __LINE__, max_value) ;
		exit (1) ;
		} ;

	sf_close (file) ;

	unlink (filename) ;
	puts ("ok") ;
} /* [+ (get "int_type_name") +]_[+ (get "float_short_name") +]_scale_write_test */
[+ ENDFOR float_type
+][+ ENDFOR int_type +]


