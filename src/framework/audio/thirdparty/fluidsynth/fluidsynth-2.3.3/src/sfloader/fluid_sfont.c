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

#include "fluid_sfont.h"
#include "fluid_sys.h"


void *default_fopen(const char *path)
{
    const char* msg;
    FILE* handle = fluid_file_open(path, &msg);

    if(handle == NULL)
    {
        FLUID_LOG(FLUID_ERR, "fluid_sfloader_load(): Failed to open '%s': %s", path, msg);
    }

    return handle;
}

int default_fclose(void *handle)
{
    return FLUID_FCLOSE((FILE *)handle) == 0 ? FLUID_OK : FLUID_FAILED;
}

fluid_long_long_t default_ftell(void *handle)
{
    return FLUID_FTELL((FILE *)handle);
}

#ifdef _WIN32
#define FLUID_PRIi64 "I64d"
#else
#define FLUID_PRIi64 "lld"
#endif

int safe_fread(void *buf, fluid_long_long_t count, void *fd)
{
    if(FLUID_FREAD(buf, (size_t)count, 1, (FILE *)fd) != 1)
    {
        if(feof((FILE *)fd))
        {
            FLUID_LOG(FLUID_ERR, "EOF while attempting to read %" FLUID_PRIi64 " bytes", count);
        }
        else
        {
            FLUID_LOG(FLUID_ERR, "File read failed");
        }

        return FLUID_FAILED;
    }

    return FLUID_OK;
}

int safe_fseek(void *fd, fluid_long_long_t ofs, int whence)
{
    if(FLUID_FSEEK((FILE *)fd, ofs, whence) != 0)
    {
        FLUID_LOG(FLUID_ERR, "File seek failed with offset = %" FLUID_PRIi64 " and whence = %d", ofs, whence);
        return FLUID_FAILED;
    }

    return FLUID_OK;
}

#undef FLUID_PRIi64

/**
 * Creates a new SoundFont loader.
 *
 * @param load Pointer to a function that provides a #fluid_sfont_t (see #fluid_sfloader_load_t).
 * @param free Pointer to a function that destroys this instance (see #fluid_sfloader_free_t).
 * Unless any private data needs to be freed it is sufficient to set this to delete_fluid_sfloader().
 *
 * @return the SoundFont loader instance on success, NULL otherwise.
 */
fluid_sfloader_t *new_fluid_sfloader(fluid_sfloader_load_t load, fluid_sfloader_free_t free)
{
    fluid_sfloader_t *loader;

    fluid_return_val_if_fail(load != NULL, NULL);
    fluid_return_val_if_fail(free != NULL, NULL);

    loader = FLUID_NEW(fluid_sfloader_t);

    if(loader == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(loader, 0, sizeof(*loader));

    loader->load = load;
    loader->free = free;
    fluid_sfloader_set_callbacks(loader,
                                 default_fopen,
                                 safe_fread,
                                 safe_fseek,
                                 default_ftell,
                                 default_fclose);

    return loader;
}

/**
 * Frees a SoundFont loader created with new_fluid_sfloader().
 *
 * @param loader The SoundFont loader instance to free.
 */
void delete_fluid_sfloader(fluid_sfloader_t *loader)
{
    fluid_return_if_fail(loader != NULL);

    FLUID_FREE(loader);
}

/**
 * Specify private data to be used by #fluid_sfloader_load_t.
 *
 * @param loader The SoundFont loader instance.
 * @param data The private data to store.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int fluid_sfloader_set_data(fluid_sfloader_t *loader, void *data)
{
    fluid_return_val_if_fail(loader != NULL, FLUID_FAILED);

    loader->data = data;
    return FLUID_OK;
}

/**
 * Obtain private data previously set with fluid_sfloader_set_data().
 *
 * @param loader The SoundFont loader instance.
 * @return The private data or NULL if none explicitly set before.
 */
void *fluid_sfloader_get_data(fluid_sfloader_t *loader)
{
    fluid_return_val_if_fail(loader != NULL, NULL);

    return loader->data;
}

/**
 * Set custom callbacks to be used upon soundfont loading.
 *
 * @param loader The SoundFont loader instance.
 * @param open A function implementing #fluid_sfloader_callback_open_t.
 * @param read A function implementing #fluid_sfloader_callback_read_t.
 * @param seek A function implementing #fluid_sfloader_callback_seek_t.
 * @param tell A function implementing #fluid_sfloader_callback_tell_t.
 * @param close A function implementing #fluid_sfloader_callback_close_t.
 * @return #FLUID_OK if the callbacks have been successfully set, #FLUID_FAILED otherwise.
 *
 * Useful for loading a soundfont from memory, see \a doc/fluidsynth_sfload_mem.c as an example.
 *
 */
int fluid_sfloader_set_callbacks(fluid_sfloader_t *loader,
                                 fluid_sfloader_callback_open_t open,
                                 fluid_sfloader_callback_read_t read,
                                 fluid_sfloader_callback_seek_t seek,
                                 fluid_sfloader_callback_tell_t tell,
                                 fluid_sfloader_callback_close_t close)
{
    fluid_file_callbacks_t *cb;

    fluid_return_val_if_fail(loader != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(open != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(read != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(seek != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(tell != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(close != NULL, FLUID_FAILED);

    cb = &loader->file_callbacks;

    cb->fopen = open;
    cb->fread = read;
    cb->fseek = seek;
    cb->ftell = tell;
    cb->fclose = close;

    // NOTE: if we ever make the instpatch loader public, this may return FLUID_FAILED
    return FLUID_OK;
}

/**
 * Creates a new virtual SoundFont instance structure.
 *
 * @param get_name A function implementing #fluid_sfont_get_name_t.
 * @param get_preset A function implementing #fluid_sfont_get_preset_t.
 * @param iter_start A function implementing #fluid_sfont_iteration_start_t, or NULL if preset iteration not needed.
 * @param iter_next A function implementing #fluid_sfont_iteration_next_t, or NULL if preset iteration not needed.
 * @param free A function implementing #fluid_sfont_free_t.
 * @return The soundfont instance on success or NULL otherwise.
 */
fluid_sfont_t *new_fluid_sfont(fluid_sfont_get_name_t get_name,
                               fluid_sfont_get_preset_t get_preset,
                               fluid_sfont_iteration_start_t iter_start,
                               fluid_sfont_iteration_next_t iter_next,
                               fluid_sfont_free_t free)
{
    fluid_sfont_t *sfont;

    fluid_return_val_if_fail(get_name != NULL, NULL);
    fluid_return_val_if_fail(get_preset != NULL, NULL);
    fluid_return_val_if_fail(free != NULL, NULL);

    sfont = FLUID_NEW(fluid_sfont_t);

    if(sfont == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(sfont, 0, sizeof(*sfont));

    sfont->get_name = get_name;
    sfont->get_preset = get_preset;
    sfont->iteration_start = iter_start;
    sfont->iteration_next = iter_next;
    sfont->free = free;

    return sfont;
}

/**
 * Set private data to use with a SoundFont instance.
 *
 * @param sfont The SoundFont instance.
 * @param data The private data to store.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int fluid_sfont_set_data(fluid_sfont_t *sfont, void *data)
{
    fluid_return_val_if_fail(sfont != NULL, FLUID_FAILED);

    sfont->data = data;
    return FLUID_OK;
}

/**
 * Retrieve the private data of a SoundFont instance.
 *
 * @param sfont The SoundFont instance.
 * @return The private data or NULL if none explicitly set before.
 */
void *fluid_sfont_get_data(fluid_sfont_t *sfont)
{
    fluid_return_val_if_fail(sfont != NULL, NULL);

    return sfont->data;
}

/**
 * Retrieve the unique ID of a SoundFont instance.
 *
 * @param sfont The SoundFont instance.
 * @return The SoundFont ID.
 */
int fluid_sfont_get_id(fluid_sfont_t *sfont)
{
    return sfont->id;
}

/**
 * Retrieve the name of a SoundFont instance.
 *
 * @param sfont The SoundFont instance.
 * @return The name of the SoundFont.
 */
const char *fluid_sfont_get_name(fluid_sfont_t *sfont)
{
    return sfont->get_name(sfont);
}

/**
 * Retrieve the preset assigned the a SoundFont instance for the given bank and preset number.
 *
 * @param sfont The SoundFont instance.
 * @param bank bank number of the preset
 * @param prenum program number of the preset
 * @return The preset instance or NULL if none found.
 */
fluid_preset_t *fluid_sfont_get_preset(fluid_sfont_t *sfont, int bank, int prenum)
{
    return sfont->get_preset(sfont, bank, prenum);
}


/**
 * Starts / re-starts virtual preset iteration in a SoundFont.
 *
 * @param sfont Virtual SoundFont instance
 */
void fluid_sfont_iteration_start(fluid_sfont_t *sfont)
{
    fluid_return_if_fail(sfont != NULL);
    fluid_return_if_fail(sfont->iteration_start != NULL);

    sfont->iteration_start(sfont);
}

/**
 * Virtual SoundFont preset iteration function.
 *
 * Returns preset information to the caller and advances the
 * internal iteration state to the next preset for subsequent calls.
 * @param sfont The SoundFont instance.
 * @return NULL when no more presets are available, otherwise the a pointer to the current preset
 */
fluid_preset_t *fluid_sfont_iteration_next(fluid_sfont_t *sfont)
{
    fluid_return_val_if_fail(sfont != NULL, NULL);
    fluid_return_val_if_fail(sfont->iteration_next != NULL, NULL);

    return sfont->iteration_next(sfont);
}

/**
 * Destroys a SoundFont instance created with new_fluid_sfont().
 *
 * @param sfont The SoundFont instance to destroy.
 * @return Always returns 0.
 *
 * Implements #fluid_sfont_free_t.
 *
 */
int delete_fluid_sfont(fluid_sfont_t *sfont)
{
    fluid_return_val_if_fail(sfont != NULL, 0);

    FLUID_FREE(sfont);
    return 0;
}

/**
 * Create a virtual SoundFont preset instance.
 *
 * @param parent_sfont The SoundFont instance this preset shall belong to
 * @param get_name A function implementing #fluid_preset_get_name_t
 * @param get_bank A function implementing #fluid_preset_get_banknum_t
 * @param get_num A function implementing #fluid_preset_get_num_t
 * @param noteon A function implementing #fluid_preset_noteon_t
 * @param free A function implementing #fluid_preset_free_t
 * @return The preset instance on success, NULL otherwise.
 */
fluid_preset_t *new_fluid_preset(fluid_sfont_t *parent_sfont,
                                 fluid_preset_get_name_t get_name,
                                 fluid_preset_get_banknum_t get_bank,
                                 fluid_preset_get_num_t get_num,
                                 fluid_preset_noteon_t noteon,
                                 fluid_preset_free_t free)
{
    fluid_preset_t *preset;

    fluid_return_val_if_fail(parent_sfont != NULL, NULL);
    fluid_return_val_if_fail(get_name != NULL, NULL);
    fluid_return_val_if_fail(get_bank != NULL, NULL);
    fluid_return_val_if_fail(get_num != NULL, NULL);
    fluid_return_val_if_fail(noteon != NULL, NULL);
    fluid_return_val_if_fail(free != NULL, NULL);

    preset = FLUID_NEW(fluid_preset_t);

    if(preset == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(preset, 0, sizeof(*preset));

    preset->sfont = parent_sfont;
    preset->get_name = get_name;
    preset->get_banknum = get_bank;
    preset->get_num = get_num;
    preset->noteon = noteon;
    preset->free = free;

    return preset;
}

/**
 * Set private data to use with a SoundFont preset instance.
 *
 * @param preset The SoundFont preset instance.
 * @param data The private data to store.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int fluid_preset_set_data(fluid_preset_t *preset, void *data)
{
    fluid_return_val_if_fail(preset != NULL, FLUID_FAILED);

    preset->data = data;
    return FLUID_OK;
}

/**
 * Retrieve the private data of a SoundFont preset instance.
 *
 * @param preset The SoundFont preset instance.
 * @return The private data or NULL if none explicitly set before.
 */
void *fluid_preset_get_data(fluid_preset_t *preset)
{
    fluid_return_val_if_fail(preset != NULL, NULL);

    return preset->data;
}

/**
 * Retrieves the presets name by executing the \p get_name function
 * provided on its creation.
 *
 * @param preset The SoundFont preset instance.
 * @return Pointer to a NULL-terminated string containing the presets name.
 */
const char *fluid_preset_get_name(fluid_preset_t *preset)
{
    return preset->get_name(preset);
}

/**
 * Retrieves the presets bank number by executing the \p get_bank function
 * provided on its creation.
 *
 * @param preset The SoundFont preset instance.
 * @return The bank number of \p preset.
 */
int fluid_preset_get_banknum(fluid_preset_t *preset)
{
    return preset->get_banknum(preset);
}

/**
 * Retrieves the presets (instrument) number by executing the \p get_num function
 * provided on its creation.
 *
 * @param preset The SoundFont preset instance.
 * @return The number of \p preset.
 */
int fluid_preset_get_num(fluid_preset_t *preset)
{
    return preset->get_num(preset);
}

/**
 * Retrieves the presets parent SoundFont instance.
 *
 * @param preset The SoundFont preset instance.
 * @return The parent SoundFont of \p preset.
 */
fluid_sfont_t *fluid_preset_get_sfont(fluid_preset_t *preset)
{
    return preset->sfont;
}

/**
 * Destroys a SoundFont preset instance created with new_fluid_preset().
 *
 * @param preset The SoundFont preset instance to destroy.
 *
 * Implements #fluid_preset_free_t.
 *
 */
void delete_fluid_preset(fluid_preset_t *preset)
{
    fluid_return_if_fail(preset != NULL);

    FLUID_FREE(preset);
}

/**
 * Create a new sample instance.
 *
 * @return  The sample on success, NULL otherwise.
 */
fluid_sample_t *
new_fluid_sample()
{
    fluid_sample_t *sample = NULL;

    sample = FLUID_NEW(fluid_sample_t);

    if(sample == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(sample, 0, sizeof(*sample));

    return sample;
}

/**
 * Destroy a sample instance previously created with new_fluid_sample().
 *
 * @param sample The sample to destroy.
 */
void
delete_fluid_sample(fluid_sample_t *sample)
{
    fluid_return_if_fail(sample != NULL);

    if(sample->auto_free)
    {
        FLUID_FREE(sample->data);
        FLUID_FREE(sample->data24);
    }

    FLUID_FREE(sample);
}

/**
 * Returns the size of the fluid_sample_t structure.
 *
 * @return Size of fluid_sample_t in bytes
 *
 * Useful in low latency scenarios e.g. to allocate a pool of samples.
 *
 * @note It is recommend to zero initialize the memory before using the object.
 *
 * @warning Do NOT allocate samples on the stack and assign them to a voice!
 */
size_t fluid_sample_sizeof()
{
    return sizeof(fluid_sample_t);
}

/**
 * Set the name of a SoundFont sample.
 *
 * @param sample SoundFont sample
 * @param name Name to assign to sample (20 chars in length + zero terminator)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int fluid_sample_set_name(fluid_sample_t *sample, const char *name)
{
    fluid_return_val_if_fail(sample != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(name != NULL, FLUID_FAILED);

    FLUID_STRNCPY(sample->name, name, sizeof(sample->name));
    return FLUID_OK;
}

/**
 * Assign sample data to a SoundFont sample.
 *
 * @param sample SoundFont sample
 * @param data Buffer containing 16 bit (mono-)audio sample data
 * @param data24 If not NULL, pointer to the least significant byte counterparts of each sample data point in order to create 24 bit audio samples
 * @param nbframes Number of samples in \a data
 * @param sample_rate Sampling rate of the sample data
 * @param copy_data TRUE to copy the sample data (and automatically free it upon delete_fluid_sample()), FALSE to use it directly (and not free it)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 *
 * @note If \a copy_data is FALSE, data should have 8 unused frames at start
 * and 8 unused frames at the end and \a nbframes should be >=48
 */
int
fluid_sample_set_sound_data(fluid_sample_t *sample,
                            short *data,
                            char *data24,
                            unsigned int nbframes,
                            unsigned int sample_rate,
                            short copy_data
                           )
{
    /* the number of samples before the start and after the end */
#define SAMPLE_LOOP_MARGIN 8U

    fluid_return_val_if_fail(sample != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(data != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(nbframes != 0, FLUID_FAILED);

    /* in case we already have some data */
    if((sample->data != NULL || sample->data24 != NULL) && sample->auto_free)
    {
        FLUID_FREE(sample->data);
        FLUID_FREE(sample->data24);
    }

    sample->data = NULL;
    sample->data24 = NULL;

    if(copy_data)
    {
        unsigned int storedNbFrames;

        /* nbframes should be >= 48 (SoundFont specs) */
        storedNbFrames = nbframes;

        if(storedNbFrames < 48)
        {
            storedNbFrames = 48;
        }

        storedNbFrames += 2 * SAMPLE_LOOP_MARGIN;

        sample->data = FLUID_ARRAY(short, storedNbFrames);

        if(sample->data == NULL)
        {
            goto error_rec;
        }

        FLUID_MEMSET(sample->data, 0, storedNbFrames * sizeof(short));
        FLUID_MEMCPY(sample->data + SAMPLE_LOOP_MARGIN, data, nbframes * sizeof(short));

        if(data24 != NULL)
        {
            sample->data24 = FLUID_ARRAY(char, storedNbFrames);

            if(sample->data24 == NULL)
            {
                goto error_rec;
            }

            FLUID_MEMSET(sample->data24, 0, storedNbFrames);
            FLUID_MEMCPY(sample->data24 + SAMPLE_LOOP_MARGIN, data24, nbframes * sizeof(char));
        }

        /* pointers */
        /* all from the start of data */
        sample->start = SAMPLE_LOOP_MARGIN;
        sample->end = SAMPLE_LOOP_MARGIN + nbframes - 1;
    }
    else
    {
        /* we cannot assure the SAMPLE_LOOP_MARGIN */
        sample->data = data;
        sample->data24 = data24;
        sample->start = 0;
        sample->end = nbframes - 1;
    }

    sample->samplerate = sample_rate;
    sample->sampletype = FLUID_SAMPLETYPE_MONO;
    sample->auto_free = copy_data;

    return FLUID_OK;

error_rec:
    FLUID_LOG(FLUID_ERR, "Out of memory");
    FLUID_FREE(sample->data);
    FLUID_FREE(sample->data24);
    sample->data = NULL;
    sample->data24 = NULL;
    return FLUID_FAILED;

#undef SAMPLE_LOOP_MARGIN
}

/**
 * Set the loop of a sample.
 *
 * @param sample SoundFont sample
 * @param loop_start Start sample index of the loop.
 * @param loop_end End index of the loop (must be a valid sample as it marks the last sample to be played).
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int fluid_sample_set_loop(fluid_sample_t *sample, unsigned int loop_start, unsigned int loop_end)
{
    fluid_return_val_if_fail(sample != NULL, FLUID_FAILED);

    sample->loopstart = loop_start;
    sample->loopend = loop_end;

    return FLUID_OK;
}

/**
 * Set the pitch of a sample.
 *
 * @param sample SoundFont sample
 * @param root_key Root MIDI note of sample (0-127)
 * @param fine_tune Fine tune in cents
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int fluid_sample_set_pitch(fluid_sample_t *sample, int root_key, int fine_tune)
{
    fluid_return_val_if_fail(sample != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(0 <= root_key && root_key <= 127, FLUID_FAILED);

    sample->origpitch = root_key;
    sample->pitchadj = fine_tune;

    return FLUID_OK;
}


/**
 * Validate parameters of a sample
 *
 */
int fluid_sample_validate(fluid_sample_t *sample, unsigned int buffer_size)
{
#define EXCLUSIVE_FLAGS (FLUID_SAMPLETYPE_MONO | FLUID_SAMPLETYPE_RIGHT | FLUID_SAMPLETYPE_LEFT)
    static const unsigned int supported_flags = EXCLUSIVE_FLAGS | FLUID_SAMPLETYPE_LINKED | FLUID_SAMPLETYPE_OGG_VORBIS | FLUID_SAMPLETYPE_ROM;

    /* ROM samples are unusable for us by definition */
    if(sample->sampletype & FLUID_SAMPLETYPE_ROM)
    {
        FLUID_LOG(FLUID_WARN, "Sample '%s': ROM sample ignored", sample->name);
        return FLUID_FAILED;
    }

    if(sample->sampletype & ~supported_flags)
    {
        FLUID_LOG(FLUID_WARN, "Sample '%s' has unknown flags, possibly using an unsupported compression; sample ignored", sample->name);
        return FLUID_FAILED;
    }

    if((sample->sampletype & EXCLUSIVE_FLAGS) & ((sample->sampletype & EXCLUSIVE_FLAGS) - 1))
    {
        FLUID_LOG(FLUID_INFO, "Sample '%s' should be either mono or left or right; using it anyway", sample->name);
    }

    if((sample->sampletype & FLUID_SAMPLETYPE_LINKED) && (sample->sampletype & EXCLUSIVE_FLAGS))
    {
        FLUID_LOG(FLUID_INFO, "Linked sample '%s' should not be mono, left or right at the same time; using it anyway", sample->name);
    }

    if((sample->sampletype & EXCLUSIVE_FLAGS) == 0)
    {
        FLUID_LOG(FLUID_INFO, "Sample '%s' has no flags set, assuming mono", sample->name);
        sample->sampletype = FLUID_SAMPLETYPE_MONO;
    }

    /* Ogg vorbis compressed samples in the SF3 format use byte indices for
     * sample start and end pointers before decompression. Standard SF2 samples
     * use sample word indices for all pointers, so use half the buffer_size
     * for validation. */
    if(!(sample->sampletype & FLUID_SAMPLETYPE_OGG_VORBIS))
    {
        if(buffer_size % 2)
        {
            FLUID_LOG(FLUID_WARN, "Sample '%s': invalid buffer size", sample->name);
            return FLUID_FAILED;
        }

        buffer_size /= 2;
    }

    if((sample->end > buffer_size) || (sample->start >= sample->end))
    {
        FLUID_LOG(FLUID_WARN, "Sample '%s': invalid start/end file positions", sample->name);
        return FLUID_FAILED;
    }

    return FLUID_OK;
#undef EXCLUSIVE_FLAGS
}

/* Check the sample loop pointers and optionally convert them to something
 * usable in case they are broken. Return a boolean indicating if the pointers
 * have been modified, so the user can be notified of possible audio glitches.
 */
int fluid_sample_sanitize_loop(fluid_sample_t *sample, unsigned int buffer_size)
{
    int modified = FALSE;
    unsigned int max_end = buffer_size / 2;
    /* In fluid_sample_t the sample end pointer points to the last sample, not
     * to the data word after the last sample. FIXME: why? */
    unsigned int sample_end = sample->end + 1;

    if(sample->loopstart == sample->loopend)
    {
        /* Some SoundFonts disable loops by setting loopstart = loopend. While
         * technically invalid, we decided to accept those samples anyway.
         * Before fluidsynth 2.2.5 we've set those indices to zero, as this
         * change was believed to be inaudible. This turned out to be an
         * incorrect assumption, as the loop points may still be modified by
         * loop offset modulators afterwards.
         */
        if(sample->loopstart != sample->start)
        {
            // Many soundfonts set loopstart == loopend == sample->start to disabled to loop.
            // Only report cases where it's not equal to the sample->start, to avoid spam.
            FLUID_LOG(FLUID_DBG, "Sample '%s': zero length loop detected: loopstart == loopend == '%d', sample start '%d', using it anyway",
                    sample->name, sample->loopstart, sample->start);
        }
    }
    else if(sample->loopstart > sample->loopend)
    {
        unsigned int tmp;

        /* If loop start and end are reversed, try to swap them around and
         * continue validation */
        FLUID_LOG(FLUID_DBG, "Sample '%s': reversed loop pointers '%d' - '%d', trying to fix",
                  sample->name, sample->loopstart, sample->loopend);
        tmp = sample->loopstart;
        sample->loopstart = sample->loopend;
        sample->loopend = tmp;
        modified = TRUE;
    }

    /* The SoundFont 2.4 spec defines the loopstart index as the first sample
     * point of the loop while loopend is the first point AFTER the last sample
     * of the loop. However we cannot be sure whether any of loopend or end is
     * correct. Hours of thinking through this have concluded that it would be
     * best practice to mangle with loops as little as necessary by only making
     * sure the pointers are within sample->start to max_end. Incorrect
     * soundfont shall preferably fail loudly. */
    if((sample->loopstart < sample->start) || (sample->loopstart > max_end))
    {
        FLUID_LOG(FLUID_DBG, "Sample '%s': invalid loop start '%d', setting to sample start '%d'",
                  sample->name, sample->loopstart, sample->start);
        sample->loopstart = sample->start;
        modified = TRUE;
    }

    if((sample->loopend < sample->start) || (sample->loopend > max_end))
    {
        FLUID_LOG(FLUID_DBG, "Sample '%s': invalid loop end '%d', setting to sample end '%d'",
                  sample->name, sample->loopend, sample_end);
        sample->loopend = sample_end;
        modified = TRUE;
    }

    if((sample->loopstart > sample_end) || (sample->loopend > sample_end))
    {
        FLUID_LOG(FLUID_DBG, "Sample '%s': loop range '%d - %d' after sample end '%d', using it anyway",
                  sample->name, sample->loopstart, sample->loopend, sample_end);
    }

    return modified;
}
