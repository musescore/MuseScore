/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

/*
 * Low-level routines for file output.
 */

#include "fluid_sys.h"
#include "fluid_synth.h"
#include "fluid_settings.h"

#if LIBSNDFILE_SUPPORT
#include <sndfile.h>
#endif

struct _fluid_file_renderer_t
{
    fluid_synth_t *synth;

#if LIBSNDFILE_SUPPORT
    SNDFILE *sndfile;
    float *buf;
#else
    FILE *file;
    short *buf;
#endif

    int period_size;
    int buf_size;
};

#if LIBSNDFILE_SUPPORT

/* Default file type used, if none specified and auto extension search fails */
#define FLUID_FILE_RENDERER_DEFAULT_FILE_TYPE   SF_FORMAT_WAV

/* File audio format names.
 * !! Keep in sync with format_ids[] */
static const char *const format_names[] =
{
    "s8",
    "s16",
    "s24",
    "s32",
    "u8",
    "float",
    "double"
};


/* File audio format IDs.
 * !! Keep in sync with format_names[] */
static const int format_ids[] =
{
    SF_FORMAT_PCM_S8,
    SF_FORMAT_PCM_16,
    SF_FORMAT_PCM_24,
    SF_FORMAT_PCM_32,
    SF_FORMAT_PCM_U8,
    SF_FORMAT_FLOAT,
    SF_FORMAT_DOUBLE
};

/* File endian byte order names.
 * !! Keep in sync with endian_ids[] */
static const char *const endian_names[] =
{
    "auto",
    "little",
    "big",
    "cpu"
};

/* File endian byte order ids.
 * !! Keep in sync with endian_names[] */
static const int endian_ids[] =
{
    SF_ENDIAN_FILE,
    SF_ENDIAN_LITTLE,
    SF_ENDIAN_BIG,
    SF_ENDIAN_CPU
};

static int fluid_file_renderer_parse_options(char *filetype, char *format,
        char *endian, char *filename, SF_INFO *info);
static int fluid_file_renderer_find_file_type(char *extension, int *type);
static int fluid_file_renderer_find_valid_format(SF_INFO *info);

#endif


void
fluid_file_renderer_settings(fluid_settings_t *settings)
{
#if LIBSNDFILE_SUPPORT
    SF_FORMAT_INFO finfo, cmpinfo;
    int major_count;
    int i, i2;
    unsigned int n;

    fluid_settings_register_str(settings, "audio.file.name", "fluidsynth.wav", 0);
    fluid_settings_register_str(settings, "audio.file.type", "auto", 0);
    fluid_settings_register_str(settings, "audio.file.format", "s16", 0);
    fluid_settings_register_str(settings, "audio.file.endian", "auto", 0);

    fluid_settings_add_option(settings, "audio.file.type", "auto");

    sf_command(NULL, SFC_GET_FORMAT_MAJOR_COUNT, &major_count, sizeof(int));

    for(i = 0; i < major_count; i++)
    {
        finfo.format = i;
        sf_command(NULL, SFC_GET_FORMAT_MAJOR, &finfo, sizeof(finfo));

        /* Check for duplicates */
        for(i2 = 0; i2 < i; i2++)
        {
            cmpinfo.format = i2;
            sf_command(NULL, SFC_GET_FORMAT_MAJOR, &cmpinfo, sizeof(cmpinfo));

            if(FLUID_STRCMP(cmpinfo.extension, finfo.extension) == 0)
            {
                break;
            }
        }

        if(i2 == i)
        {
            fluid_settings_add_option(settings, "audio.file.type", finfo.extension);
        }
    }

    for(n = 0; n < FLUID_N_ELEMENTS(format_names); n++)
    {
        fluid_settings_add_option(settings, "audio.file.format", format_names[n]);
    }

    for(n = 0; n < FLUID_N_ELEMENTS(endian_names); n++)
    {
        fluid_settings_add_option(settings, "audio.file.endian", endian_names[n]);
    }

#else

    fluid_settings_register_str(settings, "audio.file.name", "fluidsynth.raw", 0);
    fluid_settings_register_str(settings, "audio.file.type", "raw", 0);
    fluid_settings_add_option(settings, "audio.file.type", "raw");
    fluid_settings_register_str(settings, "audio.file.format", "s16", 0);
    fluid_settings_add_option(settings, "audio.file.format", "s16");
    fluid_settings_register_str(settings, "audio.file.endian", "cpu", 0);
    fluid_settings_add_option(settings, "audio.file.endian", "cpu");
#endif
}

/**
 * Create a new file renderer and open the file.
 *
 * @param synth The synth that creates audio data.
 * @return the new object, or NULL on failure
 *
 * @note Available file types and formats depends on if libfluidsynth was
 * built with libsndfile support or not.  If not then only RAW 16 bit output is
 * supported.
 *
 * Uses the following settings from the synth object:
 *   - \ref settings_audio_file_name : Output filename
 *   - \ref settings_audio_file_type : File type, "auto" tries to determine type from filename
 *     extension with fallback to "wav".
 *   - \ref settings_audio_file_format : Audio format
 *   - \ref settings_audio_file_endian : Endian byte order, "auto" for file type's default byte order
 *   - \ref settings_audio_period-size : Size of audio blocks to process
 *   - \ref settings_synth_sample-rate : Sample rate to use
 *
 * @since 1.1.0
 */
fluid_file_renderer_t *
new_fluid_file_renderer(fluid_synth_t *synth)
{
#if LIBSNDFILE_SUPPORT
    char *type, *format, *endian;
    SF_INFO info;
    double samplerate;
    int retval;
#endif
    int audio_channels;
    char *filename = NULL;
    fluid_file_renderer_t *dev;

    fluid_return_val_if_fail(synth != NULL, NULL);
    fluid_return_val_if_fail(synth->settings != NULL, NULL);

    dev = FLUID_NEW(fluid_file_renderer_t);

    if(dev == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(dev, 0, sizeof(fluid_file_renderer_t));

    dev->synth = synth;
    fluid_settings_getint(synth->settings, "audio.period-size", &dev->period_size);

#if LIBSNDFILE_SUPPORT
    dev->buf_size = 2 * dev->period_size * sizeof(float);
    dev->buf = FLUID_ARRAY(float, 2 * dev->period_size);
#else
    dev->buf_size = 2 * dev->period_size * sizeof(short);
    dev->buf = FLUID_ARRAY(short, 2 * dev->period_size);
#endif

    if(dev->buf == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        goto error_recovery;
    }

    fluid_settings_dupstr(synth->settings, "audio.file.name", &filename);
    fluid_settings_getint(synth->settings, "synth.audio-channels", &audio_channels);

    if(filename == NULL)
    {
        FLUID_LOG(FLUID_ERR, "No file name specified");
        goto error_recovery;
    }

#if LIBSNDFILE_SUPPORT
    memset(&info, 0, sizeof(info));

    info.format = FLUID_FILE_RENDERER_DEFAULT_FILE_TYPE | SF_FORMAT_PCM_16;

    fluid_settings_dupstr(synth->settings, "audio.file.type", &type);
    fluid_settings_dupstr(synth->settings, "audio.file.format", &format);
    fluid_settings_dupstr(synth->settings, "audio.file.endian", &endian);

    retval = fluid_file_renderer_parse_options(type, format, endian, filename, &info);

    if(type)
    {
        FLUID_FREE(type);
    }

    if(format)
    {
        FLUID_FREE(format);
    }

    if(endian)
    {
        FLUID_FREE(endian);
    }

    if(!retval)
    {
        goto error_recovery;
    }

    fluid_settings_getnum(synth->settings, "synth.sample-rate", &samplerate);
    info.samplerate = samplerate + 0.5;
    info.channels = 2;

    /* Search for valid format for given file type, if invalid and no format was specified.
     * To handle Ogg/Vorbis and possibly future file types with new formats.
     * Checking if format is SF_FORMAT_PCM_16 isn't a fool proof way to check if
     * format was specified or not (if user specifies "s16" itself), but should suffice. */
    if(!sf_format_check(&info)
            && ((info.format & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16
                || !fluid_file_renderer_find_valid_format(&info)))
    {
        FLUID_LOG(FLUID_ERR, "Invalid or unsupported audio file format settings");
        goto error_recovery;
    }

    dev->sndfile = sf_open(filename, SFM_WRITE, &info);

    if(!dev->sndfile)
    {
        FLUID_LOG(FLUID_ERR, "Failed to open audio file '%s' for writing", filename);
        goto error_recovery;
    }

    /* Turn on clipping and normalization of floats (-1.0 - 1.0) */
    sf_command(dev->sndfile, SFC_SET_CLIPPING, NULL, SF_TRUE);
    sf_command(dev->sndfile, SFC_SET_NORM_FLOAT, NULL, SF_TRUE);

#else
    dev->file = FLUID_FOPEN(filename, "wb");

    if(dev->file == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Failed to open the file '%s'", filename);
        goto error_recovery;
    }

#endif

    if(audio_channels != 1)
    {
        FLUID_LOG(FLUID_WARN, "The file-renderer currently only supports a single stereo channel. You have provided %d stereo channels. Audio may sound strange or incomplete.", audio_channels);
    }

    FLUID_FREE(filename);
    return dev;

error_recovery:

    FLUID_FREE(filename);
    delete_fluid_file_renderer(dev);
    return NULL;
}

/**
 * Set vbr encoding quality (only available with libsndfile support)
 * @param dev File renderer object.
 * @param q The encoding quality, see libsndfile documentation of \c SFC_SET_VBR_ENCODING_QUALITY
 * @return #FLUID_OK if the quality has been successfully set, #FLUID_FAILED otherwise
 * @since 1.1.7
 */
int
fluid_file_set_encoding_quality(fluid_file_renderer_t *dev, double q)
{
#if LIBSNDFILE_SUPPORT

    if(sf_command(dev->sndfile, SFC_SET_VBR_ENCODING_QUALITY, &q, sizeof(double)) == SF_TRUE)
    {
        return FLUID_OK;
    }
    else
#endif
    {
        return FLUID_FAILED;
    }
}

/**
 * Close file and destroy a file renderer object.
 * @param dev File renderer object.
 * @since 1.1.0
 */
void delete_fluid_file_renderer(fluid_file_renderer_t *dev)
{
    fluid_return_if_fail(dev != NULL);

#if LIBSNDFILE_SUPPORT

    if(dev->sndfile != NULL)
    {
        int retval = sf_close(dev->sndfile);

        if(retval != 0)
        {
            FLUID_LOG(FLUID_WARN, "Error closing audio file: %s", sf_error_number(retval));
        }
    }

#else

    if(dev->file != NULL)
    {
        fclose(dev->file);
    }

#endif

    FLUID_FREE(dev->buf);
    FLUID_FREE(dev);
}

/**
 * Write period_size samples to file.
 * @param dev File renderer instance
 * @return #FLUID_OK or #FLUID_FAILED if an error occurred
 * @since 1.1.0
 */
int
fluid_file_renderer_process_block(fluid_file_renderer_t *dev)
{
#if LIBSNDFILE_SUPPORT
    int n;

    fluid_synth_write_float(dev->synth, dev->period_size, dev->buf, 0, 2, dev->buf, 1, 2);

    n = sf_writef_float(dev->sndfile, dev->buf, dev->period_size);

    if(n != dev->period_size)
    {
        FLUID_LOG(FLUID_ERR, "Audio file write error: %s",
                  sf_strerror(dev->sndfile));
        return FLUID_FAILED;
    }

    return FLUID_OK;

#else   /* No libsndfile support */

    size_t res, nmemb = dev->buf_size;

    fluid_synth_write_s16(dev->synth, dev->period_size, dev->buf, 0, 2, dev->buf, 1, 2);

    res = fwrite(dev->buf, 1, nmemb, dev->file);

    if(res < nmemb)
    {
        FLUID_LOG(FLUID_ERR, "Audio output file write error: %s",
                  strerror(errno));
        return FLUID_FAILED;
    }

    return FLUID_OK;
#endif
}


#if LIBSNDFILE_SUPPORT

/**
 * Parse a colon separated format string and configure an SF_INFO structure accordingly.
 * @param filetype File type string (NULL or "auto" to attempt to identify format
 *   by filename extension, with fallback to "wav")
 * @param format File audio format string or NULL to use "s16"
 * @param endian File endian string or NULL to use "auto" which uses the file type's
 *   default endian byte order.
 * @param filename File name (used by "auto" type to determine type, based on extension)
 * @param info Audio file info structure to configure
 * @return TRUE on success, FALSE otherwise
 */
static int
fluid_file_renderer_parse_options(char *filetype, char *format, char *endian,
                                  char *filename, SF_INFO *info)
{
    int type = -1;        /* -1 indicates "auto" type */
    char *s;
    unsigned int i;

    /* If "auto" type, then use extension to search for a match */
    if(!filetype || FLUID_STRCMP(filetype, "auto") == 0)
    {
        type = FLUID_FILE_RENDERER_DEFAULT_FILE_TYPE;
        s = FLUID_STRRCHR(filename, '.');

        if(s && s[1] != '\0')
        {
            if(!fluid_file_renderer_find_file_type(s + 1, &type))
            {
                FLUID_LOG(FLUID_WARN, "Failed to determine audio file type from filename, defaulting to WAV");
            }
        }
    }
    else if(!fluid_file_renderer_find_file_type(filetype, &type))
    {
        FLUID_LOG(FLUID_ERR, "Invalid or unsupported audio file type '%s'", filetype);
        return FALSE;
    }


    info->format = (info->format & ~SF_FORMAT_TYPEMASK) | type;

    /* Look for subtype */
    if(format)
    {
        for(i = 0; i < FLUID_N_ELEMENTS(format_names); i++)
        {
            if(FLUID_STRCMP(format, format_names[i]) == 0)
            {
                break;
            }
        }

        if(i >= FLUID_N_ELEMENTS(format_names))
        {
            FLUID_LOG(FLUID_ERR, "Invalid or unsupported file audio format '%s'", format);
            return FALSE;
        }

        info->format = (info->format & ~SF_FORMAT_SUBMASK) | format_ids[i];
    }

#if LIBSNDFILE_HASVORBIS

    /* Force subformat to vorbis as nothing else would make sense currently */
    if((info->format & SF_FORMAT_TYPEMASK) == SF_FORMAT_OGG)
    {
        info->format = (info->format & ~SF_FORMAT_SUBMASK) | SF_FORMAT_VORBIS;
    }

#endif

    /* Look for endian */
    if(endian)
    {
        for(i = 0; i < FLUID_N_ELEMENTS(endian_names); i++)
        {
            if(FLUID_STRCMP(endian, endian_names[i]) == 0)
            {
                break;
            }
        }

        if(i >= FLUID_N_ELEMENTS(endian_names))
        {
            FLUID_LOG(FLUID_ERR, "Invalid or unsupported endian byte order '%s'", endian);
            return FALSE;
        }

        info->format = (info->format & ~SF_FORMAT_ENDMASK) | endian_ids[i];
    }

    return TRUE;
}

/**
 * Searches for a supported libsndfile file type by extension.
 * @param extension The extension string
 * @param type Location to store the type (unmodified if not found)
 * @return TRUE if found, FALSE otherwise
 */
static int
fluid_file_renderer_find_file_type(char *extension, int *type)
{
    SF_FORMAT_INFO finfo;
    int major_count;
    int i;

    sf_command(NULL, SFC_GET_FORMAT_MAJOR_COUNT, &major_count, sizeof(int));

    for(i = 0; i < major_count; i++)
    {
        finfo.format = i;
        sf_command(NULL, SFC_GET_FORMAT_MAJOR, &finfo, sizeof(finfo));

        if(FLUID_STRCMP(extension, finfo.extension) == 0)
        {
            break;
        }
    }

    if(i < major_count)
    {
        *type = finfo.format;
        return TRUE;
    }

    return FALSE;
}

/* Search for a valid audio format for a given file type */
static int
fluid_file_renderer_find_valid_format(SF_INFO *info)
{
    SF_FORMAT_INFO format_info;
    int count, i;

    sf_command(NULL, SFC_GET_FORMAT_SUBTYPE_COUNT, &count, sizeof(int));

    for(i = 0; i < count; i++)
    {
        format_info.format = i;

        sf_command(NULL, SFC_GET_FORMAT_SUBTYPE, &format_info, sizeof(format_info));

        info->format = (info->format & ~SF_FORMAT_SUBMASK) | format_info.format;

        if(sf_format_check(info))
        {
            return TRUE;
        }
    }

    return FALSE;
}

#endif
