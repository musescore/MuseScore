/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * SoundFont file loading code borrowed from Smurf SoundFont Editor
 * Copyright (C) 1999-2001 Josh Green
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


#include "fluid_defsfont.h"
#include "fluid_sfont.h"
#include "fluid_sys.h"
#include "fluid_synth.h"
#include "fluid_samplecache.h"
#include "fluid_chan.h"

/* EMU8k/10k hardware applies this factor to initial attenuation generator values set at preset and
 * instrument level in a soundfont. We apply this factor when loading the generator values to stay
 * compatible as most existing soundfonts expect exactly this (strange, non-standard) behaviour. */
#define EMU_ATTENUATION_FACTOR (0.4f)

/* Dynamic sample loading functions */
static int pin_preset_samples(fluid_defsfont_t *defsfont, fluid_preset_t *preset);
static int unpin_preset_samples(fluid_defsfont_t *defsfont, fluid_preset_t *preset);
static int load_preset_samples(fluid_defsfont_t *defsfont, fluid_preset_t *preset);
static int unload_preset_samples(fluid_defsfont_t *defsfont, fluid_preset_t *preset);
static void unload_sample(fluid_sample_t *sample);
static int dynamic_samples_preset_notify(fluid_preset_t *preset, int reason, int chan);
static int dynamic_samples_sample_notify(fluid_sample_t *sample, int reason);
static int fluid_preset_zone_create_voice_zones(fluid_preset_zone_t *preset_zone);
static fluid_inst_t *find_inst_by_idx(fluid_defsfont_t *defsfont, int idx);


/***************************************************************
 *
 *                           SFONT LOADER
 */

/**
 * Creates a default soundfont2 loader that can be used with fluid_synth_add_sfloader().
 * By default every synth instance has an initial default soundfont loader instance.
 * Calling this function is usually only necessary to load a soundfont from memory, by providing custom callback functions via fluid_sfloader_set_callbacks().
 *
 * @param settings A settings instance obtained by new_fluid_settings()
 * @return A default soundfont2 loader struct
 */
fluid_sfloader_t *new_fluid_defsfloader(fluid_settings_t *settings)
{
    fluid_sfloader_t *loader;
    fluid_return_val_if_fail(settings != NULL, NULL);

    loader = new_fluid_sfloader(fluid_defsfloader_load, delete_fluid_sfloader);

    if(loader == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    fluid_sfloader_set_data(loader, settings);

    return loader;
}

fluid_sfont_t *fluid_defsfloader_load(fluid_sfloader_t *loader, const char *filename)
{
    fluid_defsfont_t *defsfont;
    fluid_sfont_t *sfont;

    defsfont = new_fluid_defsfont(fluid_sfloader_get_data(loader));

    if(defsfont == NULL)
    {
        return NULL;
    }

    sfont = new_fluid_sfont(fluid_defsfont_sfont_get_name,
                            fluid_defsfont_sfont_get_preset,
                            fluid_defsfont_sfont_iteration_start,
                            fluid_defsfont_sfont_iteration_next,
                            fluid_defsfont_sfont_delete);

    if(sfont == NULL)
    {
        delete_fluid_defsfont(defsfont);
        return NULL;
    }

    fluid_sfont_set_data(sfont, defsfont);

    defsfont->sfont = sfont;

    if(fluid_defsfont_load(defsfont, &loader->file_callbacks, filename) == FLUID_FAILED)
    {
        fluid_defsfont_sfont_delete(sfont);
        return NULL;
    }

    return sfont;
}



/***************************************************************
 *
 *                           PUBLIC INTERFACE
 */

int fluid_defsfont_sfont_delete(fluid_sfont_t *sfont)
{
    if(delete_fluid_defsfont(fluid_sfont_get_data(sfont)) != FLUID_OK)
    {
        return -1;
    }

    delete_fluid_sfont(sfont);
    return 0;
}

const char *fluid_defsfont_sfont_get_name(fluid_sfont_t *sfont)
{
    return fluid_defsfont_get_name(fluid_sfont_get_data(sfont));
}

fluid_preset_t *
fluid_defsfont_sfont_get_preset(fluid_sfont_t *sfont, int bank, int prenum)
{
    return fluid_defsfont_get_preset(fluid_sfont_get_data(sfont), bank, prenum);
}

void fluid_defsfont_sfont_iteration_start(fluid_sfont_t *sfont)
{
    fluid_defsfont_iteration_start(fluid_sfont_get_data(sfont));
}

fluid_preset_t *fluid_defsfont_sfont_iteration_next(fluid_sfont_t *sfont)
{
    return fluid_defsfont_iteration_next(fluid_sfont_get_data(sfont));
}

void fluid_defpreset_preset_delete(fluid_preset_t *preset)
{
    fluid_defsfont_t *defsfont;
    fluid_defpreset_t *defpreset;

    defsfont = fluid_sfont_get_data(preset->sfont);
    defpreset = fluid_preset_get_data(preset);

    if(defsfont)
    {
        defsfont->preset = fluid_list_remove(defsfont->preset, defpreset);
    }

    delete_fluid_defpreset(defpreset);
    delete_fluid_preset(preset);
}

const char *fluid_defpreset_preset_get_name(fluid_preset_t *preset)
{
    return fluid_defpreset_get_name(fluid_preset_get_data(preset));
}

int fluid_defpreset_preset_get_banknum(fluid_preset_t *preset)
{
    return fluid_defpreset_get_banknum(fluid_preset_get_data(preset));
}

int fluid_defpreset_preset_get_num(fluid_preset_t *preset)
{
    return fluid_defpreset_get_num(fluid_preset_get_data(preset));
}

int fluid_defpreset_preset_noteon(fluid_preset_t *preset, fluid_synth_t *synth,
                                  int chan, int key, int vel)
{
    return fluid_defpreset_noteon(fluid_preset_get_data(preset), synth, chan, key, vel);
}


/***************************************************************
 *
 *                           SFONT
 */

/*
 * new_fluid_defsfont
 */
fluid_defsfont_t *new_fluid_defsfont(fluid_settings_t *settings)
{
    fluid_defsfont_t *defsfont;

    defsfont = FLUID_NEW(fluid_defsfont_t);

    if(defsfont == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(defsfont, 0, sizeof(*defsfont));

    fluid_settings_getint(settings, "synth.lock-memory", &defsfont->mlock);
    fluid_settings_getint(settings, "synth.dynamic-sample-loading", &defsfont->dynamic_samples);

    return defsfont;
}

/*
 * delete_fluid_defsfont
 */
int delete_fluid_defsfont(fluid_defsfont_t *defsfont)
{
    fluid_list_t *list;
    fluid_preset_t *preset;
    fluid_sample_t *sample;

    fluid_return_val_if_fail(defsfont != NULL, FLUID_OK);

    /* If we use dynamic sample loading, make sure we unpin any
     * pinned presets before removing this soundfont */
    if(defsfont->dynamic_samples)
    {
        for(list = defsfont->preset; list; list = fluid_list_next(list))
        {
            preset = (fluid_preset_t *)fluid_list_get(list);
            unpin_preset_samples(defsfont, preset);
        }
    }

    /* Check that no samples are currently used */
    for(list = defsfont->sample; list; list = fluid_list_next(list))
    {
        sample = (fluid_sample_t *) fluid_list_get(list);

        if(sample->refcount != 0)
        {
            return FLUID_FAILED;
        }
    }

    if(defsfont->filename != NULL)
    {
        FLUID_FREE(defsfont->filename);
    }

    for(list = defsfont->sample; list; list = fluid_list_next(list))
    {
        sample = (fluid_sample_t *) fluid_list_get(list);

        /* If the sample data pointer is different to the sampledata chunk of
         * the soundfont, then the sample has been loaded individually (SF3)
         * and needs to be unloaded explicitly. This is safe even if using
         * dynamic sample loading, as the sample_unload mechanism sets
         * sample->data to NULL after unload. */
        if ((sample->data != NULL) && (sample->data != defsfont->sampledata))
        {
            fluid_samplecache_unload(sample->data);
        }
        delete_fluid_sample(sample);
    }

    if(defsfont->sample)
    {
        delete_fluid_list(defsfont->sample);
    }

    if(defsfont->sampledata != NULL)
    {
        fluid_samplecache_unload(defsfont->sampledata);
    }

    for(list = defsfont->preset; list; list = fluid_list_next(list))
    {
        preset = (fluid_preset_t *)fluid_list_get(list);
        fluid_defpreset_preset_delete(preset);
    }

    delete_fluid_list(defsfont->preset);

    for(list = defsfont->inst; list; list = fluid_list_next(list))
    {
        delete_fluid_inst(fluid_list_get(list));
    }

    delete_fluid_list(defsfont->inst);

    FLUID_FREE(defsfont);
    return FLUID_OK;
}

/*
 * fluid_defsfont_get_name
 */
const char *fluid_defsfont_get_name(fluid_defsfont_t *defsfont)
{
    return defsfont->filename;
}

/* Load sample data for a single sample from the Soundfont file.
 * Returns FLUID_OK on error, otherwise FLUID_FAILED
 */
int fluid_defsfont_load_sampledata(fluid_defsfont_t *defsfont, SFData *sfdata, fluid_sample_t *sample)
{
    int num_samples;
    unsigned int source_end = sample->source_end;

    /* For uncompressed samples we want to include the 46 zero sample word area following each sample
     * in the Soundfont. Otherwise samples with loopend > end, which we have decided not to correct, would
     * be corrected after all in fluid_sample_sanitize_loop */
    if(!(sample->sampletype & FLUID_SAMPLETYPE_OGG_VORBIS))
    {
        source_end += 46;  /* Length of zero sample word after each sample, according to SF specs */

        /* Safeguard against Soundfonts that are not quite valid and don't include 46 sample words after the
         * last sample */
        if(source_end >= (defsfont->samplesize  / sizeof(short)))
        {
            source_end = defsfont->samplesize  / sizeof(short);
        }
    }

    num_samples = fluid_samplecache_load(
                      sfdata, sample->source_start, source_end, sample->sampletype,
                      defsfont->mlock, &sample->data, &sample->data24);

    if(num_samples < 0)
    {
        return FLUID_FAILED;
    }

    if(num_samples == 0)
    {
        sample->start = sample->end = 0;
        sample->loopstart = sample->loopend = 0;
        return FLUID_OK;
    }

    /* Ogg Vorbis samples already have loop pointers relative to the individual decompressed sample,
     * but SF2 samples are relative to sample chunk start, so they need to be adjusted */
    if(!(sample->sampletype & FLUID_SAMPLETYPE_OGG_VORBIS))
    {
        sample->loopstart = sample->source_loopstart - sample->source_start;
        sample->loopend = sample->source_loopend - sample->source_start;
    }

    /* As we've just loaded an individual sample into it's own buffer, we need to adjust the start
     * and end pointers */
    sample->start = 0;
    sample->end = num_samples - 1;

    return FLUID_OK;
}

/* Loads the sample data for all samples from the Soundfont file. For SF2 files, it loads the data in
 * one large block. For SF3 files, each compressed sample gets loaded individually.
 * Returns FLUID_OK on success, otherwise FLUID_FAILED
 */
int fluid_defsfont_load_all_sampledata(fluid_defsfont_t *defsfont, SFData *sfdata)
{
    fluid_list_t *list;
    fluid_sample_t *sample;
    int sf3_file = (sfdata->version.major == 3);
    int sample_parsing_result = FLUID_OK;
    int invalid_loops_were_sanitized = FALSE;

    /* For SF2 files, we load the sample data in one large block */
    if(!sf3_file)
    {
        int read_samples;
        int num_samples = sfdata->samplesize / sizeof(short);

        read_samples = fluid_samplecache_load(sfdata, 0, num_samples - 1, 0, defsfont->mlock,
                                              &defsfont->sampledata, &defsfont->sample24data);

        if(read_samples != num_samples)
        {
            FLUID_LOG(FLUID_ERR, "Attempted to read %d words of sample data, but got %d instead",
                      num_samples, read_samples);
            return FLUID_FAILED;
        }
    }

    #pragma omp parallel
    #pragma omp single
    for(list = defsfont->sample; list; list = fluid_list_next(list))
    {
        sample = fluid_list_get(list);

        if(sf3_file)
        {
            /* SF3 samples get loaded individually, as most (or all) of them are in Ogg Vorbis format
             * anyway */
            #pragma omp task firstprivate(sample,sfdata,defsfont) shared(sample_parsing_result, invalid_loops_were_sanitized) default(none)
            {
                if(fluid_defsfont_load_sampledata(defsfont, sfdata, sample) == FLUID_FAILED)
                {
                    #pragma omp critical
                    {
                        FLUID_LOG(FLUID_ERR, "Failed to load sample '%s'", sample->name);
                        sample_parsing_result = FLUID_FAILED;
                    }
                }
                else
                {
                    int modified = fluid_sample_sanitize_loop(sample, (sample->end + 1) * sizeof(short));
                    if(modified)
                    {
                        #pragma omp critical
                        {
                            invalid_loops_were_sanitized = TRUE;
                        }
                    }
                    fluid_voice_optimize_sample(sample);
                }
            }
        }
        else
        {
            #pragma omp task firstprivate(sample, defsfont) shared(invalid_loops_were_sanitized) default(none)
            {
                int modified;
                /* Data pointers of SF2 samples point to large sample data block loaded above */
                sample->data = defsfont->sampledata;
                sample->data24 = defsfont->sample24data;
                modified = fluid_sample_sanitize_loop(sample, defsfont->samplesize);
                if(modified)
                {
                    #pragma omp critical
                    {
                        invalid_loops_were_sanitized = TRUE;
                    }
                }
                fluid_voice_optimize_sample(sample);
            }
        }
    }

    if(invalid_loops_were_sanitized)
    {
        FLUID_LOG(FLUID_WARN,
                  "Some invalid sample loops were sanitized! If you experience audible glitches, "
                  "start fluidsynth in verbose mode for detailed information.");
    }

    return sample_parsing_result;
}

/*
 * fluid_defsfont_load
 */
int fluid_defsfont_load(fluid_defsfont_t *defsfont, const fluid_file_callbacks_t *fcbs, const char *file)
{
    SFData *sfdata;
    fluid_list_t *p;
    SFPreset *sfpreset;
    SFSample *sfsample;
    fluid_sample_t *sample;
    fluid_defpreset_t *defpreset = NULL;

    defsfont->filename = FLUID_STRDUP(file);

    if(defsfont->filename == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return FLUID_FAILED;
    }

    defsfont->fcbs = fcbs;

    /* The actual loading is done in the sfont and sffile files */
    sfdata = fluid_sffile_open(file, fcbs);

    if(sfdata == NULL)
    {
        /* error message already printed */
        return FLUID_FAILED;
    }

    if(fluid_sffile_parse_presets(sfdata) == FLUID_FAILED)
    {
        FLUID_LOG(FLUID_ERR, "Couldn't parse presets from soundfont file");
        goto err_exit;
    }

    /* Keep track of the position and size of the sample data because
       it's loaded separately (and might be unoaded/reloaded in future) */
    defsfont->samplepos = sfdata->samplepos;
    defsfont->samplesize = sfdata->samplesize;
    defsfont->sample24pos = sfdata->sample24pos;
    defsfont->sample24size = sfdata->sample24size;

    /* Create all samples from sample headers */
    p = sfdata->sample;

    while(p != NULL)
    {
        sfsample = (SFSample *)fluid_list_get(p);

        sample = new_fluid_sample();

        if(sample == NULL)
        {
            goto err_exit;
        }

        if(fluid_sample_import_sfont(sample, sfsample, defsfont) == FLUID_OK)
        {
            fluid_defsfont_add_sample(defsfont, sample);
        }
        else
        {
            delete_fluid_sample(sample);
            sample = NULL;
        }

        /* Store reference to FluidSynth sample in SFSample for later IZone fixups */
        sfsample->fluid_sample = sample;

        p = fluid_list_next(p);
    }

    /* If dynamic sample loading is disabled, load all samples in the Soundfont */
    if(!defsfont->dynamic_samples)
    {
        if(fluid_defsfont_load_all_sampledata(defsfont, sfdata) == FLUID_FAILED)
        {
            FLUID_LOG(FLUID_ERR, "Unable to load all sample data");
            goto err_exit;
        }
    }

    /* Load all the presets */
    p = sfdata->preset;

    while(p != NULL)
    {
        sfpreset = (SFPreset *)fluid_list_get(p);
        defpreset = new_fluid_defpreset();

        if(defpreset == NULL)
        {
            goto err_exit;
        }

        if(fluid_defpreset_import_sfont(defpreset, sfpreset, defsfont, sfdata) != FLUID_OK)
        {
            goto err_exit;
        }

        if(fluid_defsfont_add_preset(defsfont, defpreset) == FLUID_FAILED)
        {
            goto err_exit;
        }

        p = fluid_list_next(p);
    }

    fluid_sffile_close(sfdata);

    return FLUID_OK;

err_exit:
    fluid_sffile_close(sfdata);
    delete_fluid_defpreset(defpreset);
    return FLUID_FAILED;
}

/* fluid_defsfont_add_sample
 *
 * Add a sample to the SoundFont
 */
int fluid_defsfont_add_sample(fluid_defsfont_t *defsfont, fluid_sample_t *sample)
{
    defsfont->sample = fluid_list_prepend(defsfont->sample, sample);
    return FLUID_OK;
}

/* fluid_defsfont_add_preset
 *
 * Add a preset to the SoundFont
 */
int fluid_defsfont_add_preset(fluid_defsfont_t *defsfont, fluid_defpreset_t *defpreset)
{
    fluid_preset_t *preset;

    preset = new_fluid_preset(defsfont->sfont,
                              fluid_defpreset_preset_get_name,
                              fluid_defpreset_preset_get_banknum,
                              fluid_defpreset_preset_get_num,
                              fluid_defpreset_preset_noteon,
                              fluid_defpreset_preset_delete);

    if(preset == NULL)
    {
        return FLUID_FAILED;
    }

    if(defsfont->dynamic_samples)
    {
        preset->notify = dynamic_samples_preset_notify;
    }

    fluid_preset_set_data(preset, defpreset);

    defsfont->preset = fluid_list_append(defsfont->preset, preset);

    return FLUID_OK;
}

/*
 * fluid_defsfont_get_preset
 */
fluid_preset_t *fluid_defsfont_get_preset(fluid_defsfont_t *defsfont, int bank, int num)
{
    fluid_preset_t *preset;
    fluid_list_t *list;

    for(list = defsfont->preset; list != NULL; list = fluid_list_next(list))
    {
        preset = (fluid_preset_t *)fluid_list_get(list);

        if((fluid_preset_get_banknum(preset) == bank) && (fluid_preset_get_num(preset) == num))
        {
            return preset;
        }
    }

    return NULL;
}

/*
 * fluid_defsfont_iteration_start
 */
void fluid_defsfont_iteration_start(fluid_defsfont_t *defsfont)
{
    defsfont->preset_iter_cur = defsfont->preset;
}

/*
 * fluid_defsfont_iteration_next
 */
fluid_preset_t *fluid_defsfont_iteration_next(fluid_defsfont_t *defsfont)
{
    fluid_preset_t *preset = (fluid_preset_t *)fluid_list_get(defsfont->preset_iter_cur);

    defsfont->preset_iter_cur = fluid_list_next(defsfont->preset_iter_cur);

    return preset;
}

/***************************************************************
 *
 *                           PRESET
 */

/*
 * new_fluid_defpreset
 */
fluid_defpreset_t *
new_fluid_defpreset(void)
{
    fluid_defpreset_t *defpreset = FLUID_NEW(fluid_defpreset_t);

    if(defpreset == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    defpreset->next = NULL;
    defpreset->name[0] = 0;
    defpreset->bank = 0;
    defpreset->num = 0;
    defpreset->global_zone = NULL;
    defpreset->zone = NULL;
    defpreset->pinned = FALSE;
    return defpreset;
}

/*
 * delete_fluid_defpreset
 */
void
delete_fluid_defpreset(fluid_defpreset_t *defpreset)
{
    fluid_preset_zone_t *zone;

    fluid_return_if_fail(defpreset != NULL);

    delete_fluid_preset_zone(defpreset->global_zone);
    defpreset->global_zone = NULL;

    zone = defpreset->zone;

    while(zone != NULL)
    {
        defpreset->zone = zone->next;
        delete_fluid_preset_zone(zone);
        zone = defpreset->zone;
    }

    FLUID_FREE(defpreset);
}

int
fluid_defpreset_get_banknum(fluid_defpreset_t *defpreset)
{
    return defpreset->bank;
}

int
fluid_defpreset_get_num(fluid_defpreset_t *defpreset)
{
    return defpreset->num;
}

const char *
fluid_defpreset_get_name(fluid_defpreset_t *defpreset)
{
    return defpreset->name;
}

/*
 * fluid_defpreset_next
 */
fluid_defpreset_t *
fluid_defpreset_next(fluid_defpreset_t *defpreset)
{
    return defpreset->next;
}

/*
 * Adds global and local modulators list to the voice. This is done in 2 steps:
 * - Step 1: Local modulators replace identical global modulators.
 * - Step 2: global + local modulators are added to the voice using mode.
 *
 * Instrument zone list (local/global) must be added using FLUID_VOICE_OVERWRITE.
 * Preset zone list (local/global) must be added using FLUID_VOICE_ADD.
 *
 * @param voice voice instance.
 * @param global_mod global list of modulators.
 * @param local_mod local list of modulators.
 * @param mode Determines how to handle an existing identical modulator.
 *   #FLUID_VOICE_ADD to add (offset) the modulator amounts,
 *   #FLUID_VOICE_OVERWRITE to replace the modulator,
*/
static void
fluid_defpreset_noteon_add_mod_to_voice(fluid_voice_t *voice,
                                        fluid_mod_t *global_mod, fluid_mod_t *local_mod,
                                        int mode)
{
    fluid_mod_t *mod;
    /* list for 'sorting' global/local modulators */
    fluid_mod_t *mod_list[FLUID_NUM_MOD];
    int mod_list_count, i;

    /* identity_limit_count is the modulator upper limit number to handle with
     * existing identical modulators.
     * When identity_limit_count is below the actual number of modulators, this
     * will restrict identity check to this upper limit,
     * This is useful when we know by advance that there is no duplicate with
     * modulators at index above this limit. This avoid wasting cpu cycles at
     * noteon.
     */
    int identity_limit_count;

    /* Step 1: Local modulators replace identical global modulators. */

    /* local (instrument zone/preset zone), modulators: Put them all into a list. */
    mod_list_count = 0;

    while(local_mod)
    {
        /* As modulators number in local_mod list was limited to FLUID_NUM_MOD at
           soundfont loading time (fluid_limit_mod_list()), here we don't need
           to check if mod_list is full.
         */
        mod_list[mod_list_count++] = local_mod;
        local_mod = local_mod->next;
    }

    /* global (instrument zone/preset zone), modulators.
     * Replace modulators with the same definition in the global list:
     * (Instrument zone: SF 2.01 page 69, 'bullet' 8)
     * (Preset zone:     SF 2.01 page 69, second-last bullet).
     *
     * mod_list contains local modulators. Now we know that there
     * is no global modulator identical to another global modulator (this has
     * been checked at soundfont loading time). So global modulators
     * are only checked against local modulators number.
     */

    /* Restrict identity check to the number of local modulators */
    identity_limit_count = mod_list_count;

    while(global_mod)
    {
        /* 'Identical' global modulators are ignored.
         *  SF2.01 section 9.5.1
         *  page 69, 'bullet' 3 defines 'identical'.  */

        for(i = 0; i < identity_limit_count; i++)
        {
            if(fluid_mod_test_identity(global_mod, mod_list[i]))
            {
                break;
            }
        }

        /* Finally add the new modulator to the list. */
        if(i >= identity_limit_count)
        {
            /* Although local_mod and global_mod lists was limited to
               FLUID_NUM_MOD at soundfont loading time, it is possible that
               local + global modulators exceeds FLUID_NUM_MOD.
               So, checks if mod_list_count reaches the limit.
            */
            if(mod_list_count >= FLUID_NUM_MOD)
            {
                /* mod_list is full, we silently forget this modulator and
                   next global modulators. When mod_list will be added to the
                   voice, a warning will be displayed if the voice list is full.
                   (see fluid_voice_add_mod_local()).
                */
                break;
            }

            mod_list[mod_list_count++] = global_mod;
        }

        global_mod = global_mod->next;
    }

    /* Step 2: global + local modulators are added to the voice using mode. */

    /*
     * mod_list contains local and global modulators, we know that:
     * - there is no global modulator identical to another global modulator,
     * - there is no local modulator identical to another local modulator,
     * So these local/global modulators are only checked against
     * actual number of voice modulators.
     */

    /* Restrict identity check to the actual number of voice modulators */
    /* Actual number of voice modulators : defaults + [instruments] */
    identity_limit_count = voice->mod_count;

    for(i = 0; i < mod_list_count; i++)
    {

        mod = mod_list[i];
        /* in mode FLUID_VOICE_OVERWRITE disabled instruments modulators CANNOT be skipped. */
        /* in mode FLUID_VOICE_ADD disabled preset modulators can be skipped. */

        if((mode == FLUID_VOICE_OVERWRITE) || (mod->amount != 0))
        {
            /* Instrument modulators -supersede- existing (default) modulators.
               SF 2.01 page 69, 'bullet' 6 */

            /* Preset modulators -add- to existing instrument modulators.
               SF2.01 page 70 first bullet on page */
            fluid_voice_add_mod_local(voice, mod, mode, identity_limit_count);
        }
    }
}

/*
 * fluid_defpreset_noteon
 */
int
fluid_defpreset_noteon(fluid_defpreset_t *defpreset, fluid_synth_t *synth, int chan, int key, int vel)
{
    fluid_preset_zone_t *preset_zone, *global_preset_zone;
    fluid_inst_t *inst;
    fluid_inst_zone_t *inst_zone, *global_inst_zone;
    fluid_voice_zone_t *voice_zone;
    fluid_list_t *list;
    fluid_voice_t *voice;
    int tuned_key;
    int i;

    /* For detuned channels it might be better to use another key for Soundfont sample selection
     * giving better approximations for the pitch than the original key.
     * Example: play key 60 on 6370 Hz => use tuned key 64 for sample selection
     *
     * This feature is only enabled for melodic channels.
     * For drum channels we always select Soundfont samples by key numbers.
     */

    if(synth->channel[chan]->channel_type == CHANNEL_TYPE_MELODIC)
    {
        tuned_key = (int)(fluid_channel_get_key_pitch(synth->channel[chan], key) / 100.0f + 0.5f);
    }
    else
    {
        tuned_key = key;
    }

    global_preset_zone = fluid_defpreset_get_global_zone(defpreset);

    /* run thru all the zones of this preset */
    preset_zone = fluid_defpreset_get_zone(defpreset);

    while(preset_zone != NULL)
    {

        /* check if the note falls into the key and velocity range of this
           preset */
        if(fluid_zone_inside_range(&preset_zone->range, tuned_key, vel))
        {

            inst = fluid_preset_zone_get_inst(preset_zone);
            global_inst_zone = fluid_inst_get_global_zone(inst);

            /* run thru all the zones of this instrument that could start a voice */
            for(list = preset_zone->voice_zone; list != NULL; list = fluid_list_next(list))
            {
                voice_zone = fluid_list_get(list);

                /* check if the instrument zone is ignored and the note falls into
                   the key and velocity range of this  instrument zone.
                   An instrument zone must be ignored when its voice is already running
                   played by a legato passage (see fluid_synth_noteon_monopoly_legato()) */
                if(fluid_zone_inside_range(&voice_zone->range, tuned_key, vel))
                {

                    inst_zone = voice_zone->inst_zone;

                    /* this is a good zone. allocate a new synthesis process and initialize it */
                    voice = fluid_synth_alloc_voice_LOCAL(synth, inst_zone->sample, chan, key, vel, &voice_zone->range);

                    if(voice == NULL)
                    {
                        return FLUID_FAILED;
                    }


                    /* Instrument level, generators */

                    for(i = 0; i < GEN_LAST; i++)
                    {

                        /* SF 2.01 section 9.4 'bullet' 4:
                         *
                         * A generator in a local instrument zone supersedes a
                         * global instrument zone generator.  Both cases supersede
                         * the default generator -> voice_gen_set */

                        if(inst_zone->gen[i].flags)
                        {
                            fluid_voice_gen_set(voice, i, inst_zone->gen[i].val);

                        }
                        else if((global_inst_zone != NULL) && (global_inst_zone->gen[i].flags))
                        {
                            fluid_voice_gen_set(voice, i, global_inst_zone->gen[i].val);

                        }
                        else
                        {
                            /* The generator has not been defined in this instrument.
                             * Do nothing, leave it at the default.
                             */
                        }

                    } /* for all generators */

                    /* Adds instrument zone modulators (global and local) to the voice.*/
                    fluid_defpreset_noteon_add_mod_to_voice(voice,
                                                            /* global instrument modulators */
                                                            global_inst_zone ? global_inst_zone->mod : NULL,
                                                            inst_zone->mod, /* local instrument modulators */
                                                            FLUID_VOICE_OVERWRITE); /* mode */

                    /* Preset level, generators */

                    for(i = 0; i < GEN_LAST; i++)
                    {

                        /* SF 2.01 section 8.5 page 58: If some generators are
                         encountered at preset level, they should be ignored.
                         However this check is not necessary when the soundfont
                         loader has ignored invalid preset generators.
                         Actually load_pgen()has ignored these invalid preset
                         generators:
                           GEN_STARTADDROFS,      GEN_ENDADDROFS,
                           GEN_STARTLOOPADDROFS,  GEN_ENDLOOPADDROFS,
                           GEN_STARTADDRCOARSEOFS,GEN_ENDADDRCOARSEOFS,
                           GEN_STARTLOOPADDRCOARSEOFS,
                           GEN_KEYNUM, GEN_VELOCITY,
                           GEN_ENDLOOPADDRCOARSEOFS,
                           GEN_SAMPLEMODE, GEN_EXCLUSIVECLASS,GEN_OVERRIDEROOTKEY
                        */

                        /* SF 2.01 section 9.4 'bullet' 9: A generator in a
                         * local preset zone supersedes a global preset zone
                         * generator.  The effect is -added- to the destination
                         * summing node -> voice_gen_incr */

                        if(preset_zone->gen[i].flags)
                        {
                            fluid_voice_gen_incr(voice, i, preset_zone->gen[i].val);
                        }
                        else if((global_preset_zone != NULL) && global_preset_zone->gen[i].flags)
                        {
                            fluid_voice_gen_incr(voice, i, global_preset_zone->gen[i].val);
                        }
                        else
                        {
                            /* The generator has not been defined in this preset
                             * Do nothing, leave it unchanged.
                             */
                        }
                    } /* for all generators */

                    /* Adds preset zone modulators (global and local) to the voice.*/
                    fluid_defpreset_noteon_add_mod_to_voice(voice,
                                                            /* global preset modulators */
                                                            global_preset_zone ? global_preset_zone->mod : NULL,
                                                            preset_zone->mod, /* local preset modulators */
                                                            FLUID_VOICE_ADD); /* mode */

                    /* add the synthesis process to the synthesis loop. */
                    fluid_synth_start_voice(synth, voice);

                    /* Store the ID of the first voice that was created by this noteon event.
                     * Exclusive class may only terminate older voices.
                     * That avoids killing voices, which have just been created.
                     * (a noteon event can create several voice processes with the same exclusive
                     * class - for example when using stereo samples)
                     */
                }
            }
        }

        preset_zone = fluid_preset_zone_next(preset_zone);
    }

    return FLUID_OK;
}

/*
 * fluid_defpreset_set_global_zone
 */
int
fluid_defpreset_set_global_zone(fluid_defpreset_t *defpreset, fluid_preset_zone_t *zone)
{
    defpreset->global_zone = zone;
    return FLUID_OK;
}

/*
 * fluid_defpreset_import_sfont
 */
int
fluid_defpreset_import_sfont(fluid_defpreset_t *defpreset,
                             SFPreset *sfpreset,
                             fluid_defsfont_t *defsfont,
                             SFData *sfdata)
{
    fluid_list_t *p;
    SFZone *sfzone;
    fluid_preset_zone_t *zone;
    int count;
    char zone_name[256];

    if(FLUID_STRLEN(sfpreset->name) > 0)
    {
        FLUID_STRCPY(defpreset->name, sfpreset->name);
    }
    else
    {
        FLUID_SNPRINTF(defpreset->name, sizeof(defpreset->name), "Bank%d,Pre%d", sfpreset->bank, sfpreset->prenum);
    }

    defpreset->bank = sfpreset->bank;
    defpreset->num = sfpreset->prenum;
    p = sfpreset->zone;
    count = 0;

    while(p != NULL)
    {
        sfzone = (SFZone *)fluid_list_get(p);
        FLUID_SNPRINTF(zone_name, sizeof(zone_name), "pz:%s/%d", defpreset->name, count);
        zone = new_fluid_preset_zone(zone_name);

        if(zone == NULL)
        {
            return FLUID_FAILED;
        }

        if(fluid_preset_zone_import_sfont(zone, defpreset->global_zone, sfzone, defsfont, sfdata) != FLUID_OK)
        {
            delete_fluid_preset_zone(zone);
            return FLUID_FAILED;
        }

        if((count == 0) && (fluid_preset_zone_get_inst(zone) == NULL))
        {
            fluid_defpreset_set_global_zone(defpreset, zone);
        }
        else if(fluid_defpreset_add_zone(defpreset, zone) != FLUID_OK)
        {
            delete_fluid_preset_zone(zone);
            return FLUID_FAILED;
        }

        p = fluid_list_next(p);
        count++;
    }

    return FLUID_OK;
}

/*
 * fluid_defpreset_add_zone
 */
int
fluid_defpreset_add_zone(fluid_defpreset_t *defpreset, fluid_preset_zone_t *zone)
{
    if(defpreset->zone == NULL)
    {
        zone->next = NULL;
        defpreset->zone = zone;
    }
    else
    {
        zone->next = defpreset->zone;
        defpreset->zone = zone;
    }

    return FLUID_OK;
}

/*
 * fluid_defpreset_get_zone
 */
fluid_preset_zone_t *
fluid_defpreset_get_zone(fluid_defpreset_t *defpreset)
{
    return defpreset->zone;
}

/*
 * fluid_defpreset_get_global_zone
 */
fluid_preset_zone_t *
fluid_defpreset_get_global_zone(fluid_defpreset_t *defpreset)
{
    return defpreset->global_zone;
}

/***************************************************************
 *
 *                           PRESET_ZONE
 */

/*
 * fluid_preset_zone_next
 */
fluid_preset_zone_t *
fluid_preset_zone_next(fluid_preset_zone_t *zone)
{
    return zone->next;
}

/*
 * new_fluid_preset_zone
 */
fluid_preset_zone_t *
new_fluid_preset_zone(char *name)
{
    fluid_preset_zone_t *zone = NULL;
    zone = FLUID_NEW(fluid_preset_zone_t);

    if(zone == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    zone->next = NULL;
    zone->voice_zone = NULL;
    zone->name = FLUID_STRDUP(name);

    if(zone->name == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        FLUID_FREE(zone);
        return NULL;
    }

    zone->inst = NULL;
    zone->range.keylo = 0;
    zone->range.keyhi = 128;
    zone->range.vello = 0;
    zone->range.velhi = 128;
    zone->range.ignore = FALSE;

    /* Flag all generators as unused (default, they will be set when they are found
     * in the sound font).
     * This also sets the generator values to default, but that is of no concern here.*/
    fluid_gen_init(&zone->gen[0], NULL);
    zone->mod = NULL; /* list of modulators */
    return zone;
}

/*
 * delete list of modulators.
 */
void delete_fluid_list_mod(fluid_mod_t *mod)
{
    fluid_mod_t *tmp;

    while(mod)	/* delete the modulators */
    {
        tmp = mod;
        mod = mod->next;
        delete_fluid_mod(tmp);
    }
}

/*
 * delete_fluid_preset_zone
 */
void
delete_fluid_preset_zone(fluid_preset_zone_t *zone)
{
    fluid_list_t *list;

    fluid_return_if_fail(zone != NULL);

    delete_fluid_list_mod(zone->mod);

    for(list = zone->voice_zone; list != NULL; list = fluid_list_next(list))
    {
        FLUID_FREE(fluid_list_get(list));
    }

    delete_fluid_list(zone->voice_zone);

    FLUID_FREE(zone->name);
    FLUID_FREE(zone);
}

static int fluid_preset_zone_create_voice_zones(fluid_preset_zone_t *preset_zone)
{
    fluid_inst_zone_t *inst_zone;
    fluid_sample_t *sample;
    fluid_voice_zone_t *voice_zone;
    fluid_zone_range_t *irange;
    fluid_zone_range_t *prange = &preset_zone->range;

    fluid_return_val_if_fail(preset_zone->inst != NULL, FLUID_FAILED);

    inst_zone = fluid_inst_get_zone(preset_zone->inst);

    while(inst_zone != NULL)
    {

        /* We only create voice ranges for zones that could actually start a voice,
         * i.e. that have a sample and don't point to ROM */
        sample = fluid_inst_zone_get_sample(inst_zone);

        if((sample == NULL) || fluid_sample_in_rom(sample))
        {
            inst_zone = fluid_inst_zone_next(inst_zone);
            continue;
        }

        voice_zone = FLUID_NEW(fluid_voice_zone_t);

        if(voice_zone == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            return FLUID_FAILED;
        }

        voice_zone->inst_zone = inst_zone;

        irange = &inst_zone->range;

        voice_zone->range.keylo = (prange->keylo > irange->keylo) ? prange->keylo : irange->keylo;
        voice_zone->range.keyhi = (prange->keyhi < irange->keyhi) ? prange->keyhi : irange->keyhi;
        voice_zone->range.vello = (prange->vello > irange->vello) ? prange->vello : irange->vello;
        voice_zone->range.velhi = (prange->velhi < irange->velhi) ? prange->velhi : irange->velhi;
        voice_zone->range.ignore = FALSE;

        preset_zone->voice_zone = fluid_list_append(preset_zone->voice_zone, voice_zone);

        inst_zone = fluid_inst_zone_next(inst_zone);
    }

    return FLUID_OK;
}

/**
 * Checks if modulator mod is identical to another modulator in the list
 * (specs SF 2.0X  7.4, 7.8).
 * @param mod, modulator list.
 * @param name, if not NULL, pointer on a string displayed as warning.
 * @return TRUE if mod is identical to another modulator, FALSE otherwise.
 */
static int
fluid_zone_is_mod_identical(fluid_mod_t *mod, char *name)
{
    fluid_mod_t *next = mod->next;

    while(next)
    {
        /* is mod identical to next ? */
        if(fluid_mod_test_identity(mod, next))
        {
            if(name)
            {
                FLUID_LOG(FLUID_WARN, "Ignoring identical modulator %s", name);
            }

            return TRUE;
        }

        next = next->next;
    }

    return FALSE;
}

/**
 * Limits the number of modulators in a modulator list.
 * This is appropriate to internal synthesizer modulators tables
 * which have a fixed size (FLUID_NUM_MOD).
 *
 * @param zone_name, zone name
 * @param list_mod, address of pointer on modulator list.
 */
static void fluid_limit_mod_list(char *zone_name, fluid_mod_t **list_mod)
{
    int mod_idx = 0; /* modulator index */
    fluid_mod_t *prev_mod = NULL; /* previous modulator in list_mod */
    fluid_mod_t *mod = *list_mod; /* first modulator in list_mod */

    while(mod)
    {
        if((mod_idx + 1) > FLUID_NUM_MOD)
        {
            /* truncation of list_mod */
            if(mod_idx)
            {
                prev_mod->next = NULL;
            }
            else
            {
                *list_mod = NULL;
            }

            delete_fluid_list_mod(mod);
            FLUID_LOG(FLUID_WARN, "%s, modulators count limited to %d", zone_name,
                      FLUID_NUM_MOD);
            break;
        }

        mod_idx++;
        prev_mod = mod;
        mod = mod->next;
    }
}

/**
 * Checks and remove invalid modulators from a zone modulators list.
 * - checks valid modulator sources (specs SF 2.01  7.4, 7.8, 8.2.1).
 * - checks identical modulators in the list (specs SF 2.01  7.4, 7.8).
 * @param zone_name, zone name.
 * @param list_mod, address of pointer on modulators list.
 */
static void
fluid_zone_check_mod(char *zone_name, fluid_mod_t **list_mod)
{
    fluid_mod_t *prev_mod = NULL; /* previous modulator in list_mod */
    fluid_mod_t *mod = *list_mod; /* first modulator in list_mod */
    int mod_idx = 0; /* modulator index */

    while(mod)
    {
        char zone_mod_name[256];
        fluid_mod_t *next = mod->next;

        /* prepare modulator name: zonename/#modulator */
        FLUID_SNPRINTF(zone_mod_name, sizeof(zone_mod_name), "%s/mod%d", zone_name, mod_idx);

        /* has mod invalid sources ? */
        if(!fluid_mod_check_sources(mod,  zone_mod_name)
                /* or is mod identical to any following modulator ? */
                || fluid_zone_is_mod_identical(mod, zone_mod_name))
        {
            /* the modulator is useless so we remove it */
            if(prev_mod)
            {
                prev_mod->next = next;
            }
            else
            {
                *list_mod = next;
            }

            delete_fluid_mod(mod); /* freeing */
        }
        else
        {
            prev_mod = mod;
        }

        mod = next;
        mod_idx++;
    }

    /* limits the size of modulators list */
    fluid_limit_mod_list(zone_name, list_mod);
}

/*
 * fluid_zone_gen_import_sfont
 * Imports generators from sfzone to gen and range.
 * @param gen, pointer on destination generators table.
 * @param range, pointer on destination range generators.
 * @param sfzone, pointer on soundfont zone generators.
 */
static void
fluid_zone_gen_import_sfont(fluid_gen_t *gen, fluid_zone_range_t *range, fluid_zone_range_t *global_range, SFZone *sfzone)
{
    fluid_list_t *r;
    SFGen *sfgen;

    if(global_range != NULL)
    {
        // All zones are initialized with the default range of 0-127. However, local zones should be superseded by
        // the range of their global zone in case that local zone lacks a GEN_KEYRANGE or GEN_VELRANGE
        // (see issue #1250).
        range->keylo = global_range->keylo;
        range->keyhi = global_range->keyhi;
        range->vello = global_range->vello;
        range->velhi = global_range->velhi;
    }

    for(r = sfzone->gen; r != NULL;)
    {
        sfgen = (SFGen *)fluid_list_get(r);

        switch(sfgen->id)
        {
        case GEN_KEYRANGE:
            range->keylo = sfgen->amount.range.lo;
            range->keyhi = sfgen->amount.range.hi;
            break;

        case GEN_VELRANGE:
            range->vello = sfgen->amount.range.lo;
            range->velhi = sfgen->amount.range.hi;
            break;

        case GEN_ATTENUATION:
            /* EMU8k/10k hardware applies a scale factor to initial attenuation generator values set at
             * preset and instrument level */
            gen[sfgen->id].val = (fluid_real_t) sfgen->amount.sword * EMU_ATTENUATION_FACTOR;
            gen[sfgen->id].flags = GEN_SET;
            break;

        case GEN_INSTRUMENT:
        case GEN_SAMPLEID:
            gen[sfgen->id].val = (fluid_real_t) sfgen->amount.uword;
            gen[sfgen->id].flags = GEN_SET;
            break;

        default:
            gen[sfgen->id].val = (fluid_real_t) sfgen->amount.sword;
            gen[sfgen->id].flags = GEN_SET;
            break;
        }

        r = fluid_list_next(r);
    }
}

/*
 * fluid_zone_mod_source_import_sfont
 * Imports source information from sf_source to src and flags.
 * @param src, pointer on destination modulator source.
 * @param flags, pointer on destination modulator flags.
 * @param sf_source, soundfont modulator source.
 * @return return TRUE if success, FALSE if source type is unknown.
 */
static int
fluid_zone_mod_source_import_sfont(unsigned char *src, unsigned char *flags, unsigned short sf_source)
{
    int type;
    unsigned char flags_dest; /* destination flags */

    /* sources */
    *src = sf_source & 127; /* index of source, seven-bit value, SF2.01 section 8.2, page 50 */

    /* Bit 7: CC flag SF 2.01 section 8.2.1 page 50*/
    flags_dest = 0;

    if(sf_source & (1 << 7))
    {
        flags_dest |= FLUID_MOD_CC;
    }
    else
    {
        flags_dest |= FLUID_MOD_GC;
    }

    /* Bit 8: D flag SF 2.01 section 8.2.2 page 51*/
    if(sf_source & (1 << 8))
    {
        flags_dest |= FLUID_MOD_NEGATIVE;
    }
    else
    {
        flags_dest |= FLUID_MOD_POSITIVE;
    }

    /* Bit 9: P flag SF 2.01 section 8.2.3 page 51*/
    if(sf_source & (1 << 9))
    {
        flags_dest |= FLUID_MOD_BIPOLAR;
    }
    else
    {
        flags_dest |= FLUID_MOD_UNIPOLAR;
    }

    /* modulator source types: SF2.01 section 8.2.1 page 52 */
    type = sf_source >> 10;
    type &= 63; /* type is a 6-bit value */

    if(type == 0)
    {
        flags_dest |= FLUID_MOD_LINEAR;
    }
    else if(type == 1)
    {
        flags_dest |= FLUID_MOD_CONCAVE;
    }
    else if(type == 2)
    {
        flags_dest |= FLUID_MOD_CONVEX;
    }
    else if(type == 3)
    {
        flags_dest |= FLUID_MOD_SWITCH;
    }
    else
    {
        *flags = flags_dest;
        /* This shouldn't happen - unknown type! */
        return FALSE;
    }

    *flags = flags_dest;
    return TRUE;
}

/*
 * fluid_zone_mod_import_sfont
 * Imports modulators from sfzone to modulators list mod.
 * @param zone_name, zone name.
 * @param mod, address of pointer on modulators list to return.
 * @param sfzone, pointer on soundfont zone.
 * @return FLUID_OK if success, FLUID_FAILED otherwise.
 */
static int
fluid_zone_mod_import_sfont(char *zone_name, fluid_mod_t **mod, SFZone *sfzone)
{
    fluid_list_t *r;
    int count;

    /* Import the modulators (only SF2.1 and higher) */
    for(count = 0, r = sfzone->mod; r != NULL; count++)
    {

        SFMod *mod_src = (SFMod *)fluid_list_get(r);
        fluid_mod_t *mod_dest = new_fluid_mod();

        if(mod_dest == NULL)
        {
            return FLUID_FAILED;
        }

        mod_dest->next = NULL; /* pointer to next modulator, this is the end of the list now.*/

        /* *** Amount *** */
        mod_dest->amount = mod_src->amount;

        /* *** Source *** */
        if(!fluid_zone_mod_source_import_sfont(&mod_dest->src1, &mod_dest->flags1, mod_src->src))
        {
            /* This shouldn't happen - unknown type!
             * Deactivate the modulator by setting the amount to 0. */
            mod_dest->amount = 0;
        }  

        /* Note: When primary source input (src1) is set to General Controller 'No Controller',
           output will be forced to 0.0 at synthesis time (see fluid_mod_get_value()).
           That means that the minimum value of the modulator will be always 0.0.
           We need to force amount value to 0 to ensure a correct evaluation of the minimum
           value later (see fluid_voice_get_lower_boundary_for_attenuation()).
        */
        if(((mod_dest->flags1 & FLUID_MOD_CC) == FLUID_MOD_GC) && 
            (mod_dest->src1 == FLUID_MOD_NONE))
        {
            mod_dest->amount = 0;
        }

        /* *** Dest *** */
        mod_dest->dest = mod_src->dest; /* index of controlled generator */

        /* *** Amount source *** */
        if(!fluid_zone_mod_source_import_sfont(&mod_dest->src2, &mod_dest->flags2, mod_src->amtsrc))
        {
            /* This shouldn't happen - unknown type!
             * Deactivate the modulator by setting the amount to 0. */
            mod_dest->amount = 0;
        }  
        /* Note: When secondary source input (src2) is set to General Controller 'No Controller',
           output will be forced to +1.0 at synthesis time (see fluid_mod_get_value()).
           That means that this source will behave unipolar only. We need to force the
           unipolar flag to ensure to ensure a correct evaluation of the minimum
           value later (see fluid_voice_get_lower_boundary_for_attenuation()).
        */
        if(((mod_dest->flags2 & FLUID_MOD_CC) == FLUID_MOD_GC) && 
            (mod_dest->src2 == FLUID_MOD_NONE))
        {
            mod_dest->flags2 &= ~FLUID_MOD_BIPOLAR;
        }

        /* *** Transform *** */
        /* SF2.01 only uses the 'linear' transform (0).
         * Deactivate the modulator by setting the amount to 0 in any other case.
         */
        if(mod_src->trans != 0)
        {
            mod_dest->amount = 0;
        }

        /* Store the new modulator in the zone The order of modulators
         * will make a difference, at least in an instrument context: The
         * second modulator overwrites the first one, if they only differ
         * in amount. */
        if(count == 0)
        {
            *mod = mod_dest;
        }
        else
        {
            fluid_mod_t *last_mod = *mod;

            /* Find the end of the list */
            while(last_mod->next != NULL)
            {
                last_mod = last_mod->next;
            }

            last_mod->next = mod_dest;
        }

        r = fluid_list_next(r);
    } /* foreach modulator */

    /* checks and removes invalid modulators in modulators list*/
    fluid_zone_check_mod(zone_name, mod);
    return FLUID_OK;
}

/*
 * fluid_preset_zone_import_sfont
 */
int
fluid_preset_zone_import_sfont(fluid_preset_zone_t *zone, fluid_preset_zone_t *global_zone, SFZone *sfzone, fluid_defsfont_t *defsfont, SFData *sfdata)
{
    /* import the generators */
    fluid_zone_gen_import_sfont(zone->gen, &zone->range, global_zone ? &global_zone->range : NULL, sfzone);

    if(zone->gen[GEN_INSTRUMENT].flags == GEN_SET)
    {
        int inst_idx = (int) zone->gen[GEN_INSTRUMENT].val;

        zone->inst = find_inst_by_idx(defsfont, inst_idx);

        if(zone->inst == NULL)
        {
            zone->inst = fluid_inst_import_sfont(inst_idx, defsfont, sfdata);
        }

        if(zone->inst == NULL)
        {

            FLUID_LOG(FLUID_ERR, "Preset zone %s: Invalid instrument reference",
                    zone->name);
            return FLUID_FAILED;
        }

        if(fluid_preset_zone_create_voice_zones(zone) == FLUID_FAILED)
        {
            return FLUID_FAILED;
        }

        /* We don't need this generator anymore */
        zone->gen[GEN_INSTRUMENT].flags = GEN_UNUSED;
    }

    /* Import the modulators (only SF2.1 and higher) */
    return fluid_zone_mod_import_sfont(zone->name, &zone->mod, sfzone);
}

/*
 * fluid_preset_zone_get_inst
 */
fluid_inst_t *
fluid_preset_zone_get_inst(fluid_preset_zone_t *zone)
{
    return zone->inst;
}


/***************************************************************
 *
 *                           INST
 */

/*
 * new_fluid_inst
 */
fluid_inst_t *
new_fluid_inst()
{
    fluid_inst_t *inst = FLUID_NEW(fluid_inst_t);

    if(inst == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    inst->name[0] = 0;
    inst->global_zone = NULL;
    inst->zone = NULL;
    return inst;
}

/*
 * delete_fluid_inst
 */
void
delete_fluid_inst(fluid_inst_t *inst)
{
    fluid_inst_zone_t *zone;

    fluid_return_if_fail(inst != NULL);

    delete_fluid_inst_zone(inst->global_zone);
    inst->global_zone = NULL;

    zone = inst->zone;

    while(zone != NULL)
    {
        inst->zone = zone->next;
        delete_fluid_inst_zone(zone);
        zone = inst->zone;
    }

    FLUID_FREE(inst);
}

/*
 * fluid_inst_set_global_zone
 */
int
fluid_inst_set_global_zone(fluid_inst_t *inst, fluid_inst_zone_t *zone)
{
    inst->global_zone = zone;
    return FLUID_OK;
}

/*
 * fluid_inst_import_sfont
 */
fluid_inst_t *
fluid_inst_import_sfont(int inst_idx, fluid_defsfont_t *defsfont, SFData *sfdata)
{
    fluid_list_t *p;
    fluid_list_t *inst_list;
    fluid_inst_t *inst;
    SFZone *sfzone;
    SFInst *sfinst;
    fluid_inst_zone_t *inst_zone;
    char zone_name[256];
    int count;

    for (inst_list = sfdata->inst; inst_list; inst_list = fluid_list_next(inst_list))
    {
        sfinst = fluid_list_get(inst_list);
        if (sfinst->idx == inst_idx)
        {
            break;
        }
    }
    if (inst_list == NULL)
    {
        return NULL;
    }

    inst = (fluid_inst_t *) new_fluid_inst();

    if(inst == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    inst->source_idx = sfinst->idx;

    p = sfinst->zone;

    if(FLUID_STRLEN(sfinst->name) > 0)
    {
        FLUID_STRCPY(inst->name, sfinst->name);
    }
    else
    {
        FLUID_STRCPY(inst->name, "<untitled>");
    }

    count = 0;

    while(p != NULL)
    {

        sfzone = (SFZone *)fluid_list_get(p);
        /* instrument zone name */
        FLUID_SNPRINTF(zone_name, sizeof(zone_name), "iz:%s/%d", inst->name, count);

        inst_zone = new_fluid_inst_zone(zone_name);
        if(inst_zone == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            goto error;
        }

        if(fluid_inst_zone_import_sfont(inst_zone, inst->global_zone, sfzone, defsfont, sfdata) != FLUID_OK)
        {
            FLUID_LOG(FLUID_ERR, "fluid_inst_zone_import_sfont() failed for instrument %s", inst->name);
            delete_fluid_inst_zone(inst_zone);
            goto error;
        }

        if((count == 0) && (fluid_inst_zone_get_sample(inst_zone) == NULL))
        {
            fluid_inst_set_global_zone(inst, inst_zone);

        }
        else if(fluid_inst_add_zone(inst, inst_zone) != FLUID_OK)
        {
            FLUID_LOG(FLUID_ERR, "fluid_inst_add_zone() failed for instrument %s", inst->name);
            delete_fluid_inst_zone(inst_zone);
            goto error;
        }

        p = fluid_list_next(p);
        count++;
    }

    defsfont->inst = fluid_list_append(defsfont->inst, inst);
    return inst;

error:
    delete_fluid_inst(inst);
    return NULL;
}

/*
 * fluid_inst_add_zone
 */
int
fluid_inst_add_zone(fluid_inst_t *inst, fluid_inst_zone_t *zone)
{
    if(inst->zone == NULL)
    {
        zone->next = NULL;
        inst->zone = zone;
    }
    else
    {
        zone->next = inst->zone;
        inst->zone = zone;
    }

    return FLUID_OK;
}

/*
 * fluid_inst_get_zone
 */
fluid_inst_zone_t *
fluid_inst_get_zone(fluid_inst_t *inst)
{
    return inst->zone;
}

/*
 * fluid_inst_get_global_zone
 */
fluid_inst_zone_t *
fluid_inst_get_global_zone(fluid_inst_t *inst)
{
    return inst->global_zone;
}

/***************************************************************
 *
 *                           INST_ZONE
 */

/*
 * new_fluid_inst_zone
 */
fluid_inst_zone_t *
new_fluid_inst_zone(char *name)
{
    fluid_inst_zone_t *zone = NULL;
    zone = FLUID_NEW(fluid_inst_zone_t);

    if(zone == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    zone->next = NULL;
    zone->name = FLUID_STRDUP(name);

    if(zone->name == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        FLUID_FREE(zone);
        return NULL;
    }

    zone->sample = NULL;
    zone->range.keylo = 0;
    zone->range.keyhi = 128;
    zone->range.vello = 0;
    zone->range.velhi = 128;
    zone->range.ignore = FALSE;
    /* Flag the generators as unused.
     * This also sets the generator values to default, but they will be overwritten anyway, if used.*/
    fluid_gen_init(&zone->gen[0], NULL);
    zone->mod = NULL; /* list of modulators */
    return zone;
}

/*
 * delete_fluid_inst_zone
 */
void
delete_fluid_inst_zone(fluid_inst_zone_t *zone)
{
    fluid_return_if_fail(zone != NULL);

    delete_fluid_list_mod(zone->mod);

    FLUID_FREE(zone->name);
    FLUID_FREE(zone);
}

/*
 * fluid_inst_zone_next
 */
fluid_inst_zone_t *
fluid_inst_zone_next(fluid_inst_zone_t *zone)
{
    return zone->next;
}

/*
 * fluid_inst_zone_import_sfont
 */
int
fluid_inst_zone_import_sfont(fluid_inst_zone_t *inst_zone, fluid_inst_zone_t *global_inst_zone, SFZone *sfzone, fluid_defsfont_t *defsfont, SFData *sfdata)
{
    /* import the generators */
    fluid_zone_gen_import_sfont(inst_zone->gen, &inst_zone->range, global_inst_zone ? &global_inst_zone->range : NULL, sfzone);

    /* FIXME */
    /*    if (zone->gen[GEN_EXCLUSIVECLASS].flags == GEN_SET) { */
    /*      FLUID_LOG(FLUID_DBG, "ExclusiveClass=%d\n", (int) zone->gen[GEN_EXCLUSIVECLASS].val); */
    /*    } */

    if (inst_zone->gen[GEN_SAMPLEID].flags == GEN_SET)
    {
        fluid_list_t *list;
        SFSample *sfsample;
        int sample_idx = (int) inst_zone->gen[GEN_SAMPLEID].val;

        /* find the SFSample by index */
        for(list = sfdata->sample; list; list = fluid_list_next(list))
        {
            sfsample = fluid_list_get(list);
            if (sfsample->idx == sample_idx)
            {
                break;
            }
        }
        if (list == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Instrument zone '%s': Invalid sample reference",
                      inst_zone->name);
            return FLUID_FAILED;
        }

        inst_zone->sample = sfsample->fluid_sample;

        /* we don't need this generator anymore, mark it as unused */
        inst_zone->gen[GEN_SAMPLEID].flags = GEN_UNUSED;
    }

    /* Import the modulators (only SF2.1 and higher) */
    return fluid_zone_mod_import_sfont(inst_zone->name, &inst_zone->mod, sfzone);
}

/*
 * fluid_inst_zone_get_sample
 */
fluid_sample_t *
fluid_inst_zone_get_sample(fluid_inst_zone_t *zone)
{
    return zone->sample;
}


int
fluid_zone_inside_range(fluid_zone_range_t *range, int key, int vel)
{
    /* ignoreInstrumentZone is set in mono legato playing */
    int ignore_zone = range->ignore;

    /* Reset the 'ignore' request */
    range->ignore = FALSE;

    return !ignore_zone && ((range->keylo <= key) &&
                            (range->keyhi >= key) &&
                            (range->vello <= vel) &&
                            (range->velhi >= vel));
}

/***************************************************************
 *
 *                           SAMPLE
 */

/*
 * fluid_sample_in_rom
 */
int
fluid_sample_in_rom(fluid_sample_t *sample)
{
    return (sample->sampletype & FLUID_SAMPLETYPE_ROM);
}


/*
 * fluid_sample_import_sfont
 */
int
fluid_sample_import_sfont(fluid_sample_t *sample, SFSample *sfsample, fluid_defsfont_t *defsfont)
{
    FLUID_STRCPY(sample->name, sfsample->name);

    sample->source_start = sfsample->start;
    sample->source_end = (sfsample->end > 0) ? sfsample->end - 1 : 0; /* marks last sample, contrary to SF spec. */
    sample->source_loopstart = sfsample->loopstart;
    sample->source_loopend = sfsample->loopend;

    sample->start = sample->source_start;
    sample->end = sample->source_end;
    sample->loopstart = sample->source_loopstart;
    sample->loopend = sample->source_loopend;
    sample->samplerate = sfsample->samplerate;
    sample->origpitch = sfsample->origpitch;
    sample->pitchadj = sfsample->pitchadj;
    sample->sampletype = sfsample->sampletype;

    if(defsfont->dynamic_samples)
    {
        sample->notify = dynamic_samples_sample_notify;
    }

    if(fluid_sample_validate(sample, defsfont->samplesize) == FLUID_FAILED)
    {
        return FLUID_FAILED;
    }

    return FLUID_OK;
}

/* Called if a sample is no longer used by a voice. Used by dynamic sample loading
 * to unload a sample that is not used by any loaded presets anymore but couldn't
 * be unloaded straight away because it was still in use by a voice. */
static int dynamic_samples_sample_notify(fluid_sample_t *sample, int reason)
{
    if(reason == FLUID_SAMPLE_DONE && sample->preset_count == 0)
    {
        unload_sample(sample);
    }

    return FLUID_OK;
}

/* Called if a preset has been selected for or unselected from a channel. Used by
 * dynamic sample loading to load and unload samples on demand. */
static int dynamic_samples_preset_notify(fluid_preset_t *preset, int reason, int chan)
{
    fluid_defsfont_t *defsfont;

    if(reason == FLUID_PRESET_SELECTED)
    {
        FLUID_LOG(FLUID_DBG, "Selected preset '%s' on channel %d", fluid_preset_get_name(preset), chan);
        defsfont = fluid_sfont_get_data(preset->sfont);
        return load_preset_samples(defsfont, preset);
    }

    if(reason == FLUID_PRESET_UNSELECTED)
    {
        FLUID_LOG(FLUID_DBG, "Deselected preset '%s' from channel %d", fluid_preset_get_name(preset), chan);
        defsfont = fluid_sfont_get_data(preset->sfont);
        return unload_preset_samples(defsfont, preset);
    }

    if(reason == FLUID_PRESET_PIN)
    {
        defsfont = fluid_sfont_get_data(preset->sfont);
        return pin_preset_samples(defsfont, preset);
    }

    if(reason == FLUID_PRESET_UNPIN)
    {
        defsfont = fluid_sfont_get_data(preset->sfont);
        return unpin_preset_samples(defsfont, preset);
    }

    return FLUID_OK;
}


static int pin_preset_samples(fluid_defsfont_t *defsfont, fluid_preset_t *preset)
{
    fluid_defpreset_t *defpreset;

    defpreset = fluid_preset_get_data(preset);
    if (defpreset->pinned)
    {
        return FLUID_OK;
    }

    FLUID_LOG(FLUID_DBG, "Pinning preset '%s'", fluid_preset_get_name(preset));

    if(load_preset_samples(defsfont, preset) == FLUID_FAILED)
    {
        return FLUID_FAILED;
    }

    defpreset->pinned = TRUE;

    return FLUID_OK;
}


static int unpin_preset_samples(fluid_defsfont_t *defsfont, fluid_preset_t *preset)
{
    fluid_defpreset_t *defpreset;

    defpreset = fluid_preset_get_data(preset);
    if (!defpreset->pinned)
    {
        return FLUID_OK;
    }

    FLUID_LOG(FLUID_DBG, "Unpinning preset '%s'", fluid_preset_get_name(preset));

    if(unload_preset_samples(defsfont, preset) == FLUID_FAILED)
    {
        return FLUID_FAILED;
    }

    defpreset->pinned = FALSE;

    return FLUID_OK;
}


/* Walk through all samples used by the passed in preset and make sure that the
 * sample data is loaded for each sample. Used by dynamic sample loading. */
static int load_preset_samples(fluid_defsfont_t *defsfont, fluid_preset_t *preset)
{
    fluid_defpreset_t *defpreset;
    fluid_preset_zone_t *preset_zone;
    fluid_inst_t *inst;
    fluid_inst_zone_t *inst_zone;
    fluid_sample_t *sample;
    SFData *sffile = NULL;

    defpreset = fluid_preset_get_data(preset);
    preset_zone = fluid_defpreset_get_zone(defpreset);

    while(preset_zone != NULL)
    {
        inst = fluid_preset_zone_get_inst(preset_zone);
        inst_zone = fluid_inst_get_zone(inst);

        while(inst_zone != NULL)
        {
            sample = fluid_inst_zone_get_sample(inst_zone);

            if((sample != NULL) && (sample->start != sample->end))
            {
                sample->preset_count++;

                /* If this is the first time this sample has been selected,
                 * load the sampledata */
                if(sample->preset_count == 1)
                {
                    /* Make sure we have an open Soundfont file. Do this here
                     * to avoid having to open the file if no loading is necessary
                     * for a preset */
                    if(sffile == NULL)
                    {
                        sffile = fluid_sffile_open(defsfont->filename, defsfont->fcbs);

                        if(sffile == NULL)
                        {
                            FLUID_LOG(FLUID_ERR, "Unable to open Soundfont file");
                            return FLUID_FAILED;
                        }
                    }

                    if(fluid_defsfont_load_sampledata(defsfont, sffile, sample) == FLUID_OK)
                    {
                        fluid_sample_sanitize_loop(sample, (sample->end + 1) * sizeof(short));
                        fluid_voice_optimize_sample(sample);
                    }
                    else
                    {
                        FLUID_LOG(FLUID_ERR, "Unable to load sample '%s', disabling", sample->name);
                        sample->start = sample->end = 0;
                    }
                }
            }

            inst_zone = fluid_inst_zone_next(inst_zone);
        }

        preset_zone = fluid_preset_zone_next(preset_zone);
    }

    if(sffile != NULL)
    {
        fluid_sffile_close(sffile);
    }

    return FLUID_OK;
}

/* Walk through all samples used by the passed in preset and unload the sample data
 * of each sample that is not used by any selected preset anymore. Used by dynamic
 * sample loading. */
static int unload_preset_samples(fluid_defsfont_t *defsfont, fluid_preset_t *preset)
{
    fluid_defpreset_t *defpreset;
    fluid_preset_zone_t *preset_zone;
    fluid_inst_t *inst;
    fluid_inst_zone_t *inst_zone;
    fluid_sample_t *sample;

    defpreset = fluid_preset_get_data(preset);
    preset_zone = fluid_defpreset_get_zone(defpreset);

    while(preset_zone != NULL)
    {
        inst = fluid_preset_zone_get_inst(preset_zone);
        inst_zone = fluid_inst_get_zone(inst);

        while(inst_zone != NULL)
        {
            sample = fluid_inst_zone_get_sample(inst_zone);

            if((sample != NULL) && (sample->preset_count > 0))
            {
                sample->preset_count--;

                /* If the sample is not used by any preset or used by a
                 * sounding voice, unload it from the sample cache. If it's
                 * still in use by a voice, dynamic_samples_sample_notify will
                 * take care of unloading the sample as soon as the voice is
                 * finished with it (but only on the next API call). */
                if(sample->preset_count == 0 && sample->refcount == 0)
                {
                    unload_sample(sample);
                }
            }

            inst_zone = fluid_inst_zone_next(inst_zone);
        }

        preset_zone = fluid_preset_zone_next(preset_zone);
    }

    return FLUID_OK;
}

/* Unload an unused sample from the samplecache */
static void unload_sample(fluid_sample_t *sample)
{
    fluid_return_if_fail(sample != NULL);
    fluid_return_if_fail(sample->data != NULL);
    fluid_return_if_fail(sample->preset_count == 0);
    fluid_return_if_fail(sample->refcount == 0);

    FLUID_LOG(FLUID_DBG, "Unloading sample '%s'", sample->name);

    if(fluid_samplecache_unload(sample->data) == FLUID_FAILED)
    {
        FLUID_LOG(FLUID_ERR, "Unable to unload sample '%s'", sample->name);
    }
    else
    {
        sample->data = NULL;
        sample->data24 = NULL;
    }
}

static fluid_inst_t *find_inst_by_idx(fluid_defsfont_t *defsfont, int idx)
{
    fluid_list_t *list;
    fluid_inst_t *inst;

    for(list = defsfont->inst; list != NULL; list = fluid_list_next(list))
    {
        inst = fluid_list_get(list);

        if(inst->source_idx == idx)
        {
            return inst;
        }
    }

    return NULL;
}
