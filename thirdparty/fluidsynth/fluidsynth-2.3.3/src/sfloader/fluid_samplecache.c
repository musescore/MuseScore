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

/* CACHED SAMPLE DATA LOADER
 *
 * This is a wrapper around fluid_sffile_read_sample_data that attempts to cache the read
 * data across all FluidSynth instances in a global (process-wide) list.
 */

#include "fluid_samplecache.h"
#include "fluid_sys.h"
#include "fluid_list.h"


typedef struct _fluid_samplecache_entry_t fluid_samplecache_entry_t;

struct _fluid_samplecache_entry_t
{
    /* The following members all form the cache key */
    char *filename;
    time_t modification_time;
    unsigned int sf_samplepos;
    unsigned int sf_samplesize;
    unsigned int sf_sample24pos;
    unsigned int sf_sample24size;
    unsigned int sample_start;
    unsigned int sample_end;
    int sample_type;
    /*  End of cache key members */

    short *sample_data;
    char *sample_data24;
    int sample_count;

    int num_references;
    int mlocked;
};

static fluid_list_t *samplecache_list = NULL;
static fluid_mutex_t samplecache_mutex = FLUID_MUTEX_INIT;

static fluid_samplecache_entry_t *new_samplecache_entry(SFData *sf, unsigned int sample_start,
        unsigned int sample_end, int sample_type, time_t mtime);
static fluid_samplecache_entry_t *get_samplecache_entry(SFData *sf, unsigned int sample_start,
        unsigned int sample_end, int sample_type, time_t mtime);
static void delete_samplecache_entry(fluid_samplecache_entry_t *entry);

static int fluid_get_file_modification_time(char *filename, time_t *modification_time);


/* PUBLIC INTERFACE */

int fluid_samplecache_load(SFData *sf,
                           unsigned int sample_start, unsigned int sample_end, int sample_type,
                           int try_mlock, short **sample_data, char **sample_data24)
{
    fluid_samplecache_entry_t *entry;
    int ret;
    time_t mtime;

    fluid_mutex_lock(samplecache_mutex);

    if(fluid_get_file_modification_time(sf->fname, &mtime) == FLUID_FAILED)
    {
        mtime = 0;
    }

    entry = get_samplecache_entry(sf, sample_start, sample_end, sample_type, mtime);

    if(entry == NULL)
    {
        fluid_mutex_unlock(samplecache_mutex);
        entry = new_samplecache_entry(sf, sample_start, sample_end, sample_type, mtime);

        if(entry == NULL)
        {
            ret = -1;
            goto unlock_exit;
        }

        fluid_mutex_lock(samplecache_mutex);
        samplecache_list = fluid_list_prepend(samplecache_list, entry);
    }
        fluid_mutex_unlock(samplecache_mutex);

    if(try_mlock && !entry->mlocked)
    {
        /* Lock the memory to disable paging. It's okay if this fails. It
         * probably means that the user doesn't have the required permission. */
        if(fluid_mlock(entry->sample_data, entry->sample_count * sizeof(short)) == 0)
        {
            if(entry->sample_data24 != NULL)
            {
                entry->mlocked = (fluid_mlock(entry->sample_data24, entry->sample_count) == 0);
            }
            else
            {
                entry->mlocked = TRUE;
            }

            if(!entry->mlocked)
            {
                fluid_munlock(entry->sample_data, entry->sample_count * sizeof(short));
                FLUID_LOG(FLUID_WARN, "Failed to pin the sample data to RAM; swapping is possible.");
            }
        }
    }

    entry->num_references++;
    *sample_data = entry->sample_data;
    *sample_data24 = entry->sample_data24;
    ret = entry->sample_count;

unlock_exit:
    return ret;
}

int fluid_samplecache_unload(const short *sample_data)
{
    fluid_list_t *entry_list;
    fluid_samplecache_entry_t *entry;
    int ret;

    fluid_mutex_lock(samplecache_mutex);

    entry_list = samplecache_list;

    while(entry_list)
    {
        entry = (fluid_samplecache_entry_t *)fluid_list_get(entry_list);

        if(sample_data == entry->sample_data)
        {
            entry->num_references--;

            if(entry->num_references == 0)
            {
                if(entry->mlocked)
                {
                    fluid_munlock(entry->sample_data, entry->sample_count * sizeof(short));

                    if(entry->sample_data24 != NULL)
                    {
                        fluid_munlock(entry->sample_data24, entry->sample_count);
                    }
                }

                samplecache_list = fluid_list_remove(samplecache_list, entry);
                delete_samplecache_entry(entry);
            }

            ret = FLUID_OK;
            goto unlock_exit;
        }

        entry_list = fluid_list_next(entry_list);
    }

    FLUID_LOG(FLUID_ERR, "Trying to free sample data not found in cache.");
    ret = FLUID_FAILED;

unlock_exit:
    fluid_mutex_unlock(samplecache_mutex);
    return ret;
}


/* Private functions */
static fluid_samplecache_entry_t *new_samplecache_entry(SFData *sf,
        unsigned int sample_start,
        unsigned int sample_end,
        int sample_type,
        time_t mtime)
{
    fluid_samplecache_entry_t *entry;

    entry = FLUID_NEW(fluid_samplecache_entry_t);

    if(entry == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(entry, 0, sizeof(*entry));

    entry->filename = FLUID_STRDUP(sf->fname);

    if(entry->filename == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        goto error_exit;
    }

    entry->sf_samplepos = sf->samplepos;
    entry->sf_samplesize = sf->samplesize;
    entry->sf_sample24pos = sf->sample24pos;
    entry->sf_sample24size = sf->sample24size;
    entry->sample_start = sample_start;
    entry->sample_end = sample_end;
    entry->sample_type = sample_type;
    entry->modification_time = mtime;

    entry->sample_count = fluid_sffile_read_sample_data(sf, sample_start, sample_end, sample_type,
                          &entry->sample_data, &entry->sample_data24);

    if(entry->sample_count < 0)
    {
        goto error_exit;
    }

    return entry;

error_exit:
    delete_samplecache_entry(entry);
    return NULL;
}

static void delete_samplecache_entry(fluid_samplecache_entry_t *entry)
{
    fluid_return_if_fail(entry != NULL);

    FLUID_FREE(entry->filename);
    FLUID_FREE(entry->sample_data);
    FLUID_FREE(entry->sample_data24);
    FLUID_FREE(entry);
}

static fluid_samplecache_entry_t *get_samplecache_entry(SFData *sf,
        unsigned int sample_start,
        unsigned int sample_end,
        int sample_type,
        time_t mtime)
{
    fluid_list_t *entry_list;
    fluid_samplecache_entry_t *entry;

    entry_list = samplecache_list;

    while(entry_list)
    {
        entry = (fluid_samplecache_entry_t *)fluid_list_get(entry_list);

        if((FLUID_STRCMP(sf->fname, entry->filename) == 0) &&
                (mtime == entry->modification_time) &&
                (sf->samplepos == entry->sf_samplepos) &&
                (sf->samplesize == entry->sf_samplesize) &&
                (sf->sample24pos == entry->sf_sample24pos) &&
                (sf->sample24size == entry->sf_sample24size) &&
                (sample_start == entry->sample_start) &&
                (sample_end == entry->sample_end) &&
                (sample_type == entry->sample_type))
        {
            return entry;
        }

        entry_list = fluid_list_next(entry_list);
    }

    return NULL;
}

static int fluid_get_file_modification_time(char *filename, time_t *modification_time)
{
    fluid_stat_buf_t buf;

    if(fluid_stat(filename, &buf))
    {
        return FLUID_FAILED;
    }

    *modification_time = buf.st_mtime;
    return FLUID_OK;
}


/* Only used for tests */
int fluid_samplecache_count_entries(void)
{
    fluid_list_t *entry;
    int count = 0;

    fluid_mutex_lock(samplecache_mutex);

    for(entry = samplecache_list; entry != NULL; entry = fluid_list_next(entry))
    {
        count++;
    }

    fluid_mutex_unlock(samplecache_mutex);

    return count;
}
