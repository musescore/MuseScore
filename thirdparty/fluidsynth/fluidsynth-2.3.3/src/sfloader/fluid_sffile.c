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


#include "fluid_sffile.h"
#include "fluid_sfont.h"
#include "fluid_sys.h"

#if LIBSNDFILE_SUPPORT
#include <sndfile.h>
#endif

#if LIBINSTPATCH_SUPPORT
#include <libinstpatch/libinstpatch.h>
#endif

/*=================================sfload.c========================
  Borrowed from Smurf SoundFont Editor by Josh Green
  =================================================================*/

/* FOURCC definitions */
#define RIFF_FCC    FLUID_FOURCC('R','I','F','F')
#define LIST_FCC    FLUID_FOURCC('L','I','S','T')
#define SFBK_FCC    FLUID_FOURCC('s','f','b','k')
#define INFO_FCC    FLUID_FOURCC('I','N','F','O')
#define SDTA_FCC    FLUID_FOURCC('s','d','t','a')
#define PDTA_FCC    FLUID_FOURCC('p','d','t','a') /* info/sample/preset */

#define IFIL_FCC    FLUID_FOURCC('i','f','i','l')
#define ISNG_FCC    FLUID_FOURCC('i','s','n','g')
#define INAM_FCC    FLUID_FOURCC('I','N','A','M')
#define IROM_FCC    FLUID_FOURCC('i','r','o','m') /* info ids (1st byte of info strings) */
#define IVER_FCC    FLUID_FOURCC('i','v','e','r')
#define ICRD_FCC    FLUID_FOURCC('I','C','R','D')
#define IENG_FCC    FLUID_FOURCC('I','E','N','G')
#define IPRD_FCC    FLUID_FOURCC('I','P','R','D') /* more info ids */
#define ICOP_FCC    FLUID_FOURCC('I','C','O','P')
#define ICMT_FCC    FLUID_FOURCC('I','C','M','T')
#define ISFT_FCC    FLUID_FOURCC('I','S','F','T') /* and yet more info ids */

#define SNAM_FCC    FLUID_FOURCC('s','n','a','m')
#define SMPL_FCC    FLUID_FOURCC('s','m','p','l') /* sample ids */
#define PHDR_FCC    FLUID_FOURCC('p','h','d','r')
#define PBAG_FCC    FLUID_FOURCC('p','b','a','g')
#define PMOD_FCC    FLUID_FOURCC('p','m','o','d')
#define PGEN_FCC    FLUID_FOURCC('p','g','e','n') /* preset ids */
#define IHDR_FCC    FLUID_FOURCC('i','n','s','t')
#define IBAG_FCC    FLUID_FOURCC('i','b','a','g')
#define IMOD_FCC    FLUID_FOURCC('i','m','o','d')
#define IGEN_FCC    FLUID_FOURCC('i','g','e','n') /* instrument ids */
#define SHDR_FCC    FLUID_FOURCC('s','h','d','r') /* sample info */
#define SM24_FCC    FLUID_FOURCC('s','m','2','4')

/* Set when the FCC code is unknown */
#define UNKN_ID     FLUID_N_ELEMENTS(idlist)

/*
 * This declares a uint32_t array containing the SF2 chunk identifiers.
 */
static const uint32_t idlist[] =
{
    RIFF_FCC,
    LIST_FCC,
    SFBK_FCC,
    INFO_FCC,
    SDTA_FCC,
    PDTA_FCC,

    IFIL_FCC,
    ISNG_FCC,
    INAM_FCC,
    IROM_FCC,
    IVER_FCC,
    ICRD_FCC,
    IENG_FCC,
    IPRD_FCC,
    ICOP_FCC,
    ICMT_FCC,
    ISFT_FCC,

    SNAM_FCC,
    SMPL_FCC,
    PHDR_FCC,
    PBAG_FCC,
    PMOD_FCC,
    PGEN_FCC,
    IHDR_FCC,
    IBAG_FCC,
    IMOD_FCC,
    IGEN_FCC,
    SHDR_FCC,
    SM24_FCC
};

static const unsigned short invalid_inst_gen[] =
{
    GEN_UNUSED1,
    GEN_UNUSED2,
    GEN_UNUSED3,
    GEN_UNUSED4,
    GEN_RESERVED1,
    GEN_RESERVED2,
    GEN_RESERVED3,
    GEN_INSTRUMENT,
};

static const unsigned short invalid_preset_gen[] =
{
    GEN_STARTADDROFS,
    GEN_ENDADDROFS,
    GEN_STARTLOOPADDROFS,
    GEN_ENDLOOPADDROFS,
    GEN_STARTADDRCOARSEOFS,
    GEN_ENDADDRCOARSEOFS,
    GEN_STARTLOOPADDRCOARSEOFS,
    GEN_KEYNUM,
    GEN_VELOCITY,
    GEN_ENDLOOPADDRCOARSEOFS,
    GEN_SAMPLEMODE,
    GEN_EXCLUSIVECLASS,
    GEN_OVERRIDEROOTKEY,
    GEN_SAMPLEID,
};


/* sfont file chunk sizes */
#define SF_PHDR_SIZE (38)
#define SF_BAG_SIZE  (4)
#define SF_MOD_SIZE  (10)
#define SF_GEN_SIZE  (4)
#define SF_IHDR_SIZE (22)
#define SF_SHDR_SIZE (46)


#define READCHUNK(sf, var)                                                  \
    do                                                                      \
    {                                                                       \
        if (sf->fcbs->fread(var, 8, sf->sffd) == FLUID_FAILED)              \
            return FALSE;                                                   \
        ((SFChunk *)(var))->size = FLUID_LE32TOH(((SFChunk *)(var))->size); \
    } while (0)

#define READD(sf, var)                                            \
    do                                                            \
    {                                                             \
        uint32_t _temp;                                           \
        if (sf->fcbs->fread(&_temp, 4, sf->sffd) == FLUID_FAILED) \
            return FALSE;                                         \
        var = FLUID_LE32TOH(_temp);                               \
    } while (0)

#define READW(sf, var)                                            \
    do                                                            \
    {                                                             \
        uint16_t _temp;                                           \
        if (sf->fcbs->fread(&_temp, 2, sf->sffd) == FLUID_FAILED) \
            return FALSE;                                         \
        var = FLUID_LE16TOH(_temp);                               \
    } while (0)

#define READID(sf, var)                                        \
    do                                                         \
    {                                                          \
        if (sf->fcbs->fread(var, 4, sf->sffd) == FLUID_FAILED) \
            return FALSE;                                      \
    } while (0)

#define READSTR(sf, var)                                        \
    do                                                          \
    {                                                           \
        if (sf->fcbs->fread(var, 20, sf->sffd) == FLUID_FAILED) \
            return FALSE;                                       \
        (*var)[20] = '\0';                                      \
    } while (0)

#define READB(sf, var)                                          \
    do                                                          \
    {                                                           \
        if (sf->fcbs->fread(&var, 1, sf->sffd) == FLUID_FAILED) \
            return FALSE;                                       \
    } while (0)

#define FSKIP(sf, size)                                                \
    do                                                                 \
    {                                                                  \
        if (sf->fcbs->fseek(sf->sffd, size, SEEK_CUR) == FLUID_FAILED) \
            return FALSE;                                              \
    } while (0)

#define FSKIPW(sf)                                                  \
    do                                                              \
    {                                                               \
        if (sf->fcbs->fseek(sf->sffd, 2, SEEK_CUR) == FLUID_FAILED) \
            return FALSE;                                           \
    } while (0)

/* removes and advances a fluid_list_t pointer */
#define SLADVREM(list, item)                        \
    do                                              \
    {                                               \
        fluid_list_t *_temp = item;                 \
        item = fluid_list_next(item);               \
        list = fluid_list_remove_link(list, _temp); \
        delete1_fluid_list(_temp);                  \
    } while (0)


static int load_header(SFData *sf);
static int load_body(SFData *sf);
static int process_info(SFData *sf, int size);
static int process_sdta(SFData *sf, unsigned int size);
static int process_pdta(SFData *sf, int size);
static int load_phdr(SFData *sf, unsigned int size);
static int load_pbag(SFData *sf, int size);
static int load_pmod(SFData *sf, int size);
static int load_ihdr(SFData *sf, unsigned int size);
static int load_ibag(SFData *sf, int size);
static int load_imod(SFData *sf, int size);
static int load_shdr(SFData *sf, unsigned int size);

static int chunkid(uint32_t id);
static int read_listchunk(SFData *sf, SFChunk *chunk);
static int pdtahelper(SFData *sf, unsigned int expid, unsigned int reclen, SFChunk *chunk, int *size);
static int preset_compare_func(const void *a, const void *b);
static fluid_list_t *find_gen_by_id(int gen, fluid_list_t *genlist);
static int valid_inst_genid(unsigned short genid);
static int valid_preset_genid(unsigned short genid);

static int fluid_sffile_read_vorbis(SFData *sf, unsigned int start_byte, unsigned int end_byte, short **data);
static int fluid_sffile_read_wav(SFData *sf, unsigned int start, unsigned int end, short **data, char **data24);

/**
 * Check if a file is a SoundFont file.
 *
 * @param filename Path to the file to check
 * @return TRUE if it could be a SF2, SF3 or DLS file, FALSE otherwise
 *
 * If fluidsynth was built with DLS support, this function will also identify DLS files.
 *
 * @note This function only checks whether header(s) in the RIFF chunk are present.
 * A call to fluid_synth_sfload() might still fail.
 */
int fluid_is_soundfont(const char *filename)
{
    FILE    *fp;
    uint32_t fcc;
    int      retcode = FALSE;
    const char* err_msg;

    do
    {
        if((fp = fluid_file_open(filename, &err_msg)) == NULL)
        {
            FLUID_LOG(FLUID_ERR, "fluid_is_soundfont(): fopen() failed: '%s'", err_msg);
            return retcode;
        }

        if(FLUID_FREAD(&fcc, sizeof(fcc), 1, fp) != 1)
        {
            FLUID_LOG(FLUID_ERR, "fluid_is_soundfont(): failed to read RIFF chunk id.");
            break;
        }

        if(fcc != RIFF_FCC)
        {
            FLUID_LOG(FLUID_ERR, "fluid_is_soundfont(): expected RIFF chunk id '0x%04X' but got '0x%04X'.", (unsigned int) RIFF_FCC, (unsigned int)fcc);
            break;
        }

        if(FLUID_FSEEK(fp, 4, SEEK_CUR))
        {
            FLUID_LOG(FLUID_ERR, "fluid_is_soundfont(): cannot seek +4 bytes.");
            break;
        }

        if(FLUID_FREAD(&fcc, sizeof(fcc), 1, fp) != 1)
        {
            FLUID_LOG(FLUID_ERR, "fluid_is_soundfont(): failed to read SFBK chunk id.");
            break;
        }

        retcode = (fcc == SFBK_FCC);
        if(retcode)
        {
            break;  // seems to be SF2, stop here
        }
#ifdef LIBINSTPATCH_SUPPORT
        else
        {
            IpatchFileHandle *fhandle = ipatch_file_identify_open(filename, NULL);
            if(fhandle != NULL)
            {
                retcode = (ipatch_file_identify(fhandle->file, NULL) == IPATCH_TYPE_DLS_FILE);
                ipatch_file_close(fhandle);
            }
        }
#endif
    }
    while(0);

    FLUID_FCLOSE(fp);

    return retcode;
}

/*
 * Open a SoundFont file and parse it's contents into a SFData structure.
 *
 * @param fname filename
 * @param fcbs file callback structure
 * @return the partially parsed SoundFont as SFData structure or NULL on error
 */
SFData *fluid_sffile_open(const char *fname, const fluid_file_callbacks_t *fcbs)
{
    SFData *sf;
    fluid_long_long_t fsize = 0;

    if(!(sf = FLUID_NEW(SFData)))
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(sf, 0, sizeof(SFData));

    fluid_rec_mutex_init(sf->mtx);
    sf->fcbs = fcbs;

    if((sf->sffd = fcbs->fopen(fname)) == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Unable to open file '%s'", fname);
        goto error_exit;
    }

    sf->fname = FLUID_STRDUP(fname);

    if(sf->fname == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        goto error_exit;
    }

    /* get size of file by seeking to end */
    if(fcbs->fseek(sf->sffd, 0L, SEEK_END) == FLUID_FAILED)
    {
        FLUID_LOG(FLUID_ERR, "Seek to end of file failed");
        goto error_exit;
    }

    if((fsize = fcbs->ftell(sf->sffd)) == FLUID_FAILED)
    {
        FLUID_LOG(FLUID_ERR, "Get end of file position failed");
        goto error_exit;
    }

    sf->filesize = fsize;

    if(fcbs->fseek(sf->sffd, 0, SEEK_SET) == FLUID_FAILED)
    {
        FLUID_LOG(FLUID_ERR, "Rewind to start of file failed");
        goto error_exit;
    }

    if(!load_header(sf))
    {
        goto error_exit;
    }

    return sf;

error_exit:
    fluid_sffile_close(sf);
    return NULL;
}

/*
 * Parse all preset information from the soundfont
 *
 * @return FLUID_OK on success, otherwise FLUID_FAILED
 */
int fluid_sffile_parse_presets(SFData *sf)
{
    if(!load_body(sf))
    {
        return FLUID_FAILED;
    }

    return FLUID_OK;
}

/* Load sample data from the soundfont file
 *
 * This function will always return the sample data in WAV format. If the sample_type specifies an
 * Ogg Vorbis compressed sample, it will be decompressed automatically before returning.
 *
 * @param sf SFData instance
 * @param sample_start index of first sample point in Soundfont sample chunk
 * @param sample_end index of last sample point in Soundfont sample chunk
 * @param sample_type type of the sample in Soundfont
 * @param data pointer to sample data pointer, will point to loaded sample data on success
 * @param data24 pointer to 24-bit sample data pointer if 24-bit data present, will point to loaded
 *               24-bit sample data on success or NULL if no 24-bit data is present in file
 *
 * @return The number of sample words in returned buffers or -1 on failure
 */
int fluid_sffile_read_sample_data(SFData *sf, unsigned int sample_start, unsigned int sample_end,
                                  int sample_type, short **data, char **data24)
{
    int num_samples;

    if(sample_type & FLUID_SAMPLETYPE_OGG_VORBIS)
    {
        num_samples = fluid_sffile_read_vorbis(sf, sample_start, sample_end, data);
    }
    else
    {
        num_samples = fluid_sffile_read_wav(sf, sample_start, sample_end, data, data24);
    }

    return num_samples;
}

/*
 * Close a SoundFont file and free the SFData structure.
 *
 * @param sf pointer to SFData structure
 * @param fcbs file callback structure
 */
void fluid_sffile_close(SFData *sf)
{
    fluid_list_t *entry;
    SFPreset *preset;
    SFInst *inst;

    fluid_rec_mutex_destroy(sf->mtx);
    if(sf->sffd)
    {
        sf->fcbs->fclose(sf->sffd);
    }

    FLUID_FREE(sf->fname);

    entry = sf->info;

    while(entry)
    {
        FLUID_FREE(fluid_list_get(entry));
        entry = fluid_list_next(entry);
    }

    delete_fluid_list(sf->info);

    entry = sf->preset;

    while(entry)
    {
        preset = (SFPreset *)fluid_list_get(entry);
        delete_preset(preset);
        entry = fluid_list_next(entry);
    }

    delete_fluid_list(sf->preset);

    entry = sf->inst;

    while(entry)
    {
        inst = (SFInst *)fluid_list_get(entry);
        delete_inst(inst);
        entry = fluid_list_next(entry);
    }

    delete_fluid_list(sf->inst);

    entry = sf->sample;

    while(entry)
    {
        FLUID_FREE(fluid_list_get(entry));
        entry = fluid_list_next(entry);
    }

    delete_fluid_list(sf->sample);

    FLUID_FREE(sf);
}


/*
 * Private functions
 */

/* sound font file load functions */
static int chunkid(uint32_t id)
{
    unsigned int i;

    for(i = 0; i < FLUID_N_ELEMENTS(idlist); i++)
    {
        if(idlist[i] == id)
        {
            break;
        }
    }

    /* Return chunk id or UNKN_ID if not found */
    return i;
}

static int load_header(SFData *sf)
{
    SFChunk chunk;

    READCHUNK(sf, &chunk); /* load RIFF chunk */

    if(chunk.id != RIFF_FCC)
    {
        /* error if not RIFF */
        FLUID_LOG(FLUID_ERR, "Not a RIFF file");
        return FALSE;
    }

    READID(sf, &chunk.id); /* load file ID */

    if(chunk.id != SFBK_FCC)
    {
        /* error if not SFBK_ID */
        FLUID_LOG(FLUID_ERR, "Not a SoundFont file");
        return FALSE;
    }

    if(chunk.size != sf->filesize - 8)
    {
        FLUID_LOG(FLUID_ERR, "SoundFont file size mismatch");
        return FALSE;
    }

    /* Process INFO block */
    if(!read_listchunk(sf, &chunk))
    {
        return FALSE;
    }

    if(chunk.id != INFO_FCC)
    {
        FLUID_LOG(FLUID_ERR, "Invalid ID found when expecting INFO chunk");
        return FALSE;
    }

    if(!process_info(sf, chunk.size))
    {
        return FALSE;
    }

    /* Process sample chunk */
    if(!read_listchunk(sf, &chunk))
    {
        return FALSE;
    }

    if(chunk.id != SDTA_FCC)
    {
        FLUID_LOG(FLUID_ERR, "Invalid ID found when expecting SAMPLE chunk");
        return FALSE;
    }

    if(!process_sdta(sf, chunk.size))
    {
        return FALSE;
    }

    /* process HYDRA chunk */
    if(!read_listchunk(sf, &chunk))
    {
        return FALSE;
    }

    if(chunk.id != PDTA_FCC)
    {
        FLUID_LOG(FLUID_ERR, "Invalid ID found when expecting HYDRA chunk");
        return FALSE;
    }

    sf->hydrapos = sf->fcbs->ftell(sf->sffd);
    sf->hydrasize = chunk.size;

    return TRUE;
}

static int load_body(SFData *sf)
{
    if(sf->fcbs->fseek(sf->sffd, sf->hydrapos, SEEK_SET) == FLUID_FAILED)
    {
        FLUID_LOG(FLUID_ERR, "Failed to seek to HYDRA position");
        return FALSE;
    }

    if(!process_pdta(sf, sf->hydrasize))
    {
        return FALSE;
    }

    /* sort preset list by bank, preset # */
    sf->preset = fluid_list_sort(sf->preset, preset_compare_func);

    return TRUE;
}

static int read_listchunk(SFData *sf, SFChunk *chunk)
{
    READCHUNK(sf, chunk); /* read list chunk */

    if(chunk->id != LIST_FCC)  /* error if ! list chunk */
    {
        FLUID_LOG(FLUID_ERR, "Invalid chunk id in level 0 parse");
        return FALSE;
    }

    READID(sf, &chunk->id); /* read id string */
    chunk->size -= 4;
    return TRUE;
}

static int process_info(SFData *sf, int size)
{
    SFChunk chunk;
    union
    {
        char *chr;
        uint32_t *fcc;
    } item;
    unsigned short ver;

    while(size > 0)
    {
        READCHUNK(sf, &chunk);
        size -= 8;

        if(chunk.id == IFIL_FCC)
        {
            /* sound font version chunk? */
            if(chunk.size != 4)
            {
                FLUID_LOG(FLUID_ERR, "Sound font version info chunk has invalid size");
                return FALSE;
            }

            READW(sf, ver);
            sf->version.major = ver;
            READW(sf, ver);
            sf->version.minor = ver;

            if(sf->version.major < 2)
            {
                FLUID_LOG(FLUID_ERR, "Sound font version is %d.%d which is not"
                          " supported, convert to version 2.0x",
                          sf->version.major, sf->version.minor);
                return FALSE;
            }

            if(sf->version.major == 3)
            {
#if !LIBSNDFILE_SUPPORT
                FLUID_LOG(FLUID_WARN,
                          "Sound font version is %d.%d but fluidsynth was compiled without"
                          " support for (v3.x)",
                          sf->version.major, sf->version.minor);
                return FALSE;
#endif
            }
            else if(sf->version.major > 2)
            {
                FLUID_LOG(FLUID_WARN,
                          "Sound font version is %d.%d which is newer than"
                          " what this version of fluidsynth was designed for (v2.0x)",
                          sf->version.major, sf->version.minor);
                return FALSE;
            }
        }
        else if(chunk.id == IVER_FCC)
        {
            /* ROM version chunk? */
            if(chunk.size != 4)
            {
                FLUID_LOG(FLUID_ERR, "ROM version info chunk has invalid size");
                return FALSE;
            }

            READW(sf, ver);
            sf->romver.major = ver;
            READW(sf, ver);
            sf->romver.minor = ver;
        }
        else if(chunkid(chunk.id) != UNKN_ID)
        {
            if((chunk.id != ICMT_FCC && chunk.size > 256) || (chunk.size > 65536) || (chunk.size % 2))
            {
                FLUID_LOG(FLUID_ERR, "INFO sub chunk %.4s has invalid chunk size of %d bytes",
                          (char*)&chunk.id, chunk.size);
                return FALSE;
            }

            /* alloc for chunk fcc and da chunk */
            if(!(item.fcc = FLUID_MALLOC(chunk.size + sizeof(uint32_t) + 1)))
            {
                FLUID_LOG(FLUID_ERR, "Out of memory");
                return FALSE;
            }

            /* attach to INFO list, fluid_sffile_close will cleanup if FAIL occurs */
            sf->info = fluid_list_append(sf->info, item.fcc);

            /* save chunk fcc and update pointer to data value */
            *item.fcc++ = chunk.id;

            if(sf->fcbs->fread(item.chr, chunk.size, sf->sffd) == FLUID_FAILED)
            {
                return FALSE;
            }

            /* force terminate info item */
            item.chr[chunk.size] = '\0';
        }
        else
        {
            FLUID_LOG(FLUID_ERR, "Invalid chunk id in INFO chunk");
            return FALSE;
        }

        size -= chunk.size;
    }

    if(size < 0)
    {
        FLUID_LOG(FLUID_ERR, "INFO chunk size mismatch");
        return FALSE;
    }

    return TRUE;
}

static int process_sdta(SFData *sf, unsigned int size)
{
    SFChunk chunk;

    if(size == 0)
    {
        return TRUE;    /* no sample data? */
    }

    /* read sub chunk */
    READCHUNK(sf, &chunk);
    size -= 8;

    if(chunk.id != SMPL_FCC)
    {
        FLUID_LOG(FLUID_ERR, "Expected SMPL chunk found invalid id instead");
        return FALSE;
    }

    /* SDTA chunk may also contain sm24 chunk for 24 bit samples
     * (not yet supported), only an error if SMPL chunk size is
     * greater than SDTA. */
    if(chunk.size > size)
    {
        FLUID_LOG(FLUID_ERR, "SDTA chunk size mismatch");
        return FALSE;
    }

    /* sample data follows */
    sf->samplepos = sf->fcbs->ftell(sf->sffd);

    /* used to check validity of sample headers */
    sf->samplesize = chunk.size;

    FSKIP(sf, chunk.size);
    size -= chunk.size;

    if(sf->version.major >= 2 && sf->version.minor >= 4)
    {
        /* any chance to find another chunk here? */
        if(size > 8)
        {
            /* read sub chunk */
            READCHUNK(sf, &chunk);
            size -= 8;

            if(chunk.id == SM24_FCC)
            {
                int sm24size, sdtahalfsize;

                FLUID_LOG(FLUID_DBG, "Found SM24 chunk");

                if(chunk.size > size)
                {
                    FLUID_LOG(FLUID_WARN, "SM24 exceeds SDTA chunk, ignoring SM24");
                    goto ret; // no error
                }

                sdtahalfsize = sf->samplesize / 2;
                /* + 1 byte in the case that half the size of smpl chunk is an odd value */
                sdtahalfsize += sdtahalfsize % 2;
                sm24size = chunk.size;

                if(sdtahalfsize != sm24size)
                {
                    FLUID_LOG(FLUID_WARN, "SM24 not equal to half the size of SMPL chunk (0x%X != "
                              "0x%X), ignoring SM24",
                              sm24size, sdtahalfsize);
                    goto ret; // no error
                }

                /* sample data24 follows */
                sf->sample24pos = sf->fcbs->ftell(sf->sffd);
                sf->sample24size = sm24size;
            }
        }
    }

ret:
    FSKIP(sf, size);

    return TRUE;
}

static int pdtahelper(SFData *sf, unsigned int expid, unsigned int reclen, SFChunk *chunk, int *size)
{
    READCHUNK(sf, chunk);
    *size -= 8;

    if(chunk->id != expid)
    {
        FLUID_LOG(FLUID_ERR, "Expected PDTA sub-chunk '%.4s' found invalid id instead", (char*)&expid);
        return FALSE;
    }

    if(chunk->size % reclen)  /* valid chunk size? */
    {
        FLUID_LOG(FLUID_ERR, "'%.4s' chunk size is not a multiple of %d bytes", (char*)&expid, reclen);
        return FALSE;
    }

    if((*size -= chunk->size) < 0)
    {
        FLUID_LOG(FLUID_ERR, "'%.4s' chunk size exceeds remaining PDTA chunk size", (char*)&expid);
        return FALSE;
    }

    return TRUE;
}

static int process_pdta(SFData *sf, int size)
{
    SFChunk chunk;

    if(!pdtahelper(sf, PHDR_FCC, SF_PHDR_SIZE, &chunk, &size))
    {
        return FALSE;
    }

    if(!load_phdr(sf, chunk.size))
    {
        return FALSE;
    }

    if(!pdtahelper(sf, PBAG_FCC, SF_BAG_SIZE, &chunk, &size))
    {
        return FALSE;
    }

    if(!load_pbag(sf, chunk.size))
    {
        return FALSE;
    }

    if(!pdtahelper(sf, PMOD_FCC, SF_MOD_SIZE, &chunk, &size))
    {
        return FALSE;
    }

    if(!load_pmod(sf, chunk.size))
    {
        return FALSE;
    }

    if(!pdtahelper(sf, PGEN_FCC, SF_GEN_SIZE, &chunk, &size))
    {
        return FALSE;
    }

    if(!load_pgen(sf, chunk.size))
    {
        return FALSE;
    }

    if(!pdtahelper(sf, IHDR_FCC, SF_IHDR_SIZE, &chunk, &size))
    {
        return FALSE;
    }

    if(!load_ihdr(sf, chunk.size))
    {
        return FALSE;
    }

    if(!pdtahelper(sf, IBAG_FCC, SF_BAG_SIZE, &chunk, &size))
    {
        return FALSE;
    }

    if(!load_ibag(sf, chunk.size))
    {
        return FALSE;
    }

    if(!pdtahelper(sf, IMOD_FCC, SF_MOD_SIZE, &chunk, &size))
    {
        return FALSE;
    }

    if(!load_imod(sf, chunk.size))
    {
        return FALSE;
    }

    if(!pdtahelper(sf, IGEN_FCC, SF_GEN_SIZE, &chunk, &size))
    {
        return FALSE;
    }

    if(!load_igen(sf, chunk.size))
    {
        return FALSE;
    }

    if(!pdtahelper(sf, SHDR_FCC, SF_SHDR_SIZE, &chunk, &size))
    {
        return FALSE;
    }

    if(!load_shdr(sf, chunk.size))
    {
        return FALSE;
    }

    return TRUE;
}

/* preset header loader */
static int load_phdr(SFData *sf, unsigned int size)
{
    unsigned int i;
    int i2;
    SFPreset *preset, *prev_preset = NULL;
    unsigned short pbag_idx, prev_pbag_idx = 0;

    if(size % SF_PHDR_SIZE || size == 0)
    {
        FLUID_LOG(FLUID_ERR, "Preset header chunk size is invalid");
        return FALSE;
    }

    i = size / SF_PHDR_SIZE - 1;

    if(i == 0)
    {
        /* at least one preset + term record */
        FLUID_LOG(FLUID_WARN, "File contains no presets");
        FSKIP(sf, SF_PHDR_SIZE);
        return TRUE;
    }

    for(; i > 0; i--)
    {
        /* load all preset headers */
        if((preset = FLUID_NEW(SFPreset)) == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            return FALSE;
        }

        sf->preset = fluid_list_append(sf->preset, preset);
        preset->zone = NULL; /* In case of failure, fluid_sffile_close can cleanup */
        READSTR(sf, &preset->name); /* possible read failure ^ */
        READW(sf, preset->prenum);
        READW(sf, preset->bank);
        READW(sf, pbag_idx);
        FSKIP(sf, 4); /* library ignored */
        FSKIP(sf, 4); /* genre ignored */
        FSKIP(sf, 4); /* morphology ignored */

        if(prev_preset)
        {
            /* not first preset? */
            if(pbag_idx < prev_pbag_idx)
            {
                FLUID_LOG(FLUID_ERR, "Preset header indices not monotonic");
                return FALSE;
            }

            i2 = pbag_idx - prev_pbag_idx;

            while(i2--)
            {
                prev_preset->zone = fluid_list_prepend(prev_preset->zone, NULL);
            }
        }
        else if(pbag_idx > 0)  /* 1st preset, warn if ofs >0 */
        {
            FLUID_LOG(FLUID_WARN, "%d preset zones not referenced, discarding", pbag_idx);
        }

        prev_preset = preset; /* update preset ptr */
        prev_pbag_idx = pbag_idx;
    }

    FSKIP(sf, 24);
    READW(sf, pbag_idx); /* Read terminal generator index */
    FSKIP(sf, 12);

    if(pbag_idx < prev_pbag_idx)
    {
        FLUID_LOG(FLUID_ERR, "Preset header indices not monotonic");
        return FALSE;
    }

    i2 = pbag_idx - prev_pbag_idx;

    while(i2--)
    {
        prev_preset->zone = fluid_list_prepend(prev_preset->zone, NULL);
    }

    return TRUE;
}

/* preset bag loader */
static int load_pbag(SFData *sf, int size)
{
    fluid_list_t *preset_list;
    fluid_list_t *zone_list;
    SFZone *z, *pz = NULL;
    unsigned short genndx, modndx;
    unsigned short pgenndx = 0, pmodndx = 0;
    unsigned short i;

    if(size % SF_BAG_SIZE || size == 0)  /* size is multiple of SF_BAG_SIZE? */
    {
        FLUID_LOG(FLUID_ERR, "Preset bag chunk size is invalid");
        return FALSE;
    }

    preset_list = sf->preset;

    /* traverse through presets */
    while(preset_list)
    {
        zone_list = ((SFPreset *)(preset_list->data))->zone;

        /* traverse preset's zones */
        while(zone_list)
        {
            if((size -= SF_BAG_SIZE) < 0)
            {
                FLUID_LOG(FLUID_ERR, "Preset bag chunk size mismatch");
                return FALSE;
            }

            if((z = FLUID_NEW(SFZone)) == NULL)
            {
                FLUID_LOG(FLUID_ERR, "Out of memory");
                return FALSE;
            }

            zone_list->data = z;
            z->gen = NULL; /* Init gen and mod before possible failure, */
            z->mod = NULL; /* to ensure proper cleanup (fluid_sffile_close) */
            READW(sf, genndx); /* possible read failure ^ */
            READW(sf, modndx);

            if(pz)
            {
                /* if not first zone */
                if(genndx < pgenndx)
                {
                    FLUID_LOG(FLUID_ERR, "Preset bag generator indices not monotonic");
                    return FALSE;
                }

                if(modndx < pmodndx)
                {
                    FLUID_LOG(FLUID_ERR, "Preset bag modulator indices not monotonic");
                    return FALSE;
                }

                i = genndx - pgenndx;

                while(i--)
                {
                    pz->gen = fluid_list_prepend(pz->gen, NULL);
                }

                i = modndx - pmodndx;

                while(i--)
                {
                    pz->mod = fluid_list_prepend(pz->mod, NULL);
                }
            }

            pz = z; /* update previous zone ptr */
            pgenndx = genndx; /* update previous zone gen index */
            pmodndx = modndx; /* update previous zone mod index */
            zone_list = fluid_list_next(zone_list);
        }

        preset_list = fluid_list_next(preset_list);
    }

    size -= SF_BAG_SIZE;

    if(size != 0)
    {
        FLUID_LOG(FLUID_ERR, "Preset bag chunk size mismatch");
        return FALSE;
    }

    READW(sf, genndx);
    READW(sf, modndx);

    if(!pz)
    {
        if(genndx > 0)
        {
            FLUID_LOG(FLUID_WARN, "No preset generators and terminal index not 0");
        }

        if(modndx > 0)
        {
            FLUID_LOG(FLUID_WARN, "No preset modulators and terminal index not 0");
        }

        return TRUE;
    }

    if(genndx < pgenndx)
    {
        FLUID_LOG(FLUID_ERR, "Preset bag generator indices not monotonic");
        return FALSE;
    }

    if(modndx < pmodndx)
    {
        FLUID_LOG(FLUID_ERR, "Preset bag modulator indices not monotonic");
        return FALSE;
    }

    i = genndx - pgenndx;

    while(i--)
    {
        pz->gen = fluid_list_prepend(pz->gen, NULL);
    }

    i = modndx - pmodndx;

    while(i--)
    {
        pz->mod = fluid_list_prepend(pz->mod, NULL);
    }

    return TRUE;
}

/* preset modulator loader */
static int load_pmod(SFData *sf, int size)
{
    fluid_list_t *preset_list;
    fluid_list_t *zone_list;
    fluid_list_t *mod_list;
    SFMod *m;

    preset_list = sf->preset;

    while(preset_list)
    {
        /* traverse through all presets */
        zone_list = ((SFPreset *)(preset_list->data))->zone;

        while(zone_list)
        {
            /* traverse this preset's zones */
            mod_list = ((SFZone *)(zone_list->data))->mod;

            while(mod_list)
            {
                /* load zone's modulators */
                if((size -= SF_MOD_SIZE) < 0)
                {
                    FLUID_LOG(FLUID_ERR, "Preset modulator chunk size mismatch");
                    return FALSE;
                }

                if((m = FLUID_NEW(SFMod)) == NULL)
                {
                    FLUID_LOG(FLUID_ERR, "Out of memory");
                    return FALSE;
                }

                mod_list->data = m;
                READW(sf, m->src);
                READW(sf, m->dest);
                READW(sf, m->amount);
                READW(sf, m->amtsrc);
                READW(sf, m->trans);
                mod_list = fluid_list_next(mod_list);
            }

            zone_list = fluid_list_next(zone_list);
        }

        preset_list = fluid_list_next(preset_list);
    }

    /*
       If there isn't even a terminal record
       Hmmm, the specs say there should be one, but..
     */
    if(size == 0)
    {
        return TRUE;
    }

    size -= SF_MOD_SIZE;

    if(size != 0)
    {
        FLUID_LOG(FLUID_ERR, "Preset modulator chunk size mismatch");
        return FALSE;
    }

    FSKIP(sf, SF_MOD_SIZE); /* terminal mod */

    return TRUE;
}

/* -------------------------------------------------------------------
 * preset generator loader
 * generator (per preset) loading rules:
 * Zones with no generators or modulators shall be annihilated
 * Global zone must be 1st zone, discard additional ones (instrumentless zones)
 *
 * generator (per zone) loading rules (in order of decreasing precedence):
 * KeyRange is 1st in list (if exists), else discard
 * if a VelRange exists only preceded by a KeyRange, else discard
 * if a generator follows an instrument discard it
 * if a duplicate generator exists replace previous one
 * ------------------------------------------------------------------- */
int load_pgen(SFData *sf, int size)
{
    fluid_list_t *dup;
    fluid_list_t *preset_list;
    fluid_list_t *zone_list;
    fluid_list_t *gen_list;
    SFZone *zone;
    SFGen *g;
    SFPreset *preset;
    SFGenAmount genval;
    unsigned short genid;
    int level, skip, drop, discarded;

    preset_list = sf->preset;

    while(preset_list)
    {
        preset = fluid_list_get(preset_list);

        /* traverse through all presets */
        discarded = FALSE;
        zone_list = preset->zone;

        /* traverse preset's zones */
        while(zone_list)
        {
            zone = fluid_list_get(zone_list);
            level = 0;
            gen_list = zone->gen;

            while(gen_list)
            {
                /* load zone's generators */
                dup = NULL;
                skip = FALSE;
                drop = FALSE;

                if((size -= SF_GEN_SIZE) < 0)
                {
                    FLUID_LOG(FLUID_ERR, "Preset generator chunk size mismatch");
                    return FALSE;
                }

                READW(sf, genid);

                if(genid == GEN_KEYRANGE)
                {
                    /* nothing precedes */
                    if(level == 0)
                    {
                        level = 1;
                        READB(sf, genval.range.lo);
                        READB(sf, genval.range.hi);
                    }
                    else
                    {
                        skip = TRUE;
                    }
                }
                else if(genid == GEN_VELRANGE)
                {
                    /* only KeyRange precedes */
                    if(level <= 1)
                    {
                        level = 2;
                        READB(sf, genval.range.lo);
                        READB(sf, genval.range.hi);
                    }
                    else
                    {
                        skip = TRUE;
                    }
                }
                else if(genid == GEN_INSTRUMENT)
                {
                    /* inst is last gen */
                    level = 3;
                    READW(sf, genval.uword);
                }
                else
                {
                    level = 2;

                    if(valid_preset_genid(genid))
                    {
                        /* generator valid? */
                        READW(sf, genval.sword);
                        dup = find_gen_by_id(genid, zone->gen);
                    }
                    else
                    {
                        skip = TRUE;
                    }
                }

                if(!skip)
                {
                    if(!dup)
                    {
                        /* if gen ! dup alloc new */
                        if((g = FLUID_NEW(SFGen)) == NULL)
                        {
                            FLUID_LOG(FLUID_ERR, "Out of memory");
                            return FALSE;
                        }

                        gen_list->data = g;
                        g->id = genid;
                    }
                    else
                    {
                        g = (SFGen *)(dup->data); /* ptr to orig gen */
                        drop = TRUE;
                    }

                    g->amount = genval;
                }
                else
                {
                    /* Skip this generator */
                    discarded = TRUE;
                    drop = TRUE;
                    FSKIPW(sf);
                }

                if(!drop)
                {
                    gen_list = fluid_list_next(gen_list);    /* next gen */
                }
                else
                {
                    SLADVREM(zone->gen, gen_list);    /* drop place holder */
                }

                /* GEN_INSTRUMENT should be the last generator */
                if (level == 3)
                {
                    break;
                }

            } /* generator loop */

            /* Anything below level 3 means it's a global zone. The global zone
             * should always be the first zone in the list, so discard any
             * other global zones we encounter */
            if(level < 3 && (zone_list != preset->zone))
            {
                /* advance to next zone before deleting the current list element */
                zone_list = fluid_list_next(zone_list);

                FLUID_LOG(FLUID_WARN, "Preset '%s': Discarding invalid global zone",
                            preset->name);
                preset->zone = fluid_list_remove(preset->zone, zone);
                delete_zone(zone);

                /* we have already advanced the zone_list pointer, so continue with next zone */
                continue;
            }

            /* All remaining generators are invalid and should be discarded
             * (because they come after an instrument generator) */
            while(gen_list)
            {
                discarded = TRUE;

                if((size -= SF_GEN_SIZE) < 0)
                {
                    FLUID_LOG(FLUID_ERR, "Preset generator chunk size mismatch");
                    return FALSE;
                }

                FSKIP(sf, SF_GEN_SIZE);
                SLADVREM(zone->gen, gen_list);
            }

            zone_list = fluid_list_next(zone_list);
        }

        if(discarded)
        {
            FLUID_LOG(FLUID_WARN,
                      "Preset '%s': Some invalid generators were discarded",
                      preset->name);
        }

        preset_list = fluid_list_next(preset_list);
    }

    /* in case there isn't a terminal record */
    if(size == 0)
    {
        return TRUE;
    }

    size -= SF_GEN_SIZE;

    if(size != 0)
    {
        FLUID_LOG(FLUID_ERR, "Preset generator chunk size mismatch");
        return FALSE;
    }

    FSKIP(sf, SF_GEN_SIZE); /* terminal gen */

    return TRUE;
}

/* instrument header loader */
static int load_ihdr(SFData *sf, unsigned int size)
{
    unsigned int i;
    int i2;
    SFInst *inst, *prev_inst = NULL; /* ptr to current & previous instrument */
    unsigned short zndx, pzndx = 0;

    if(size % SF_IHDR_SIZE || size == 0)  /* chunk size is valid? */
    {
        FLUID_LOG(FLUID_ERR, "Instrument header has invalid size");
        return FALSE;
    }

    size = size / SF_IHDR_SIZE - 1;

    if(size == 0)
    {
        /* at least one preset + term record */
        FLUID_LOG(FLUID_WARN, "File contains no instruments");
        FSKIP(sf, SF_IHDR_SIZE);
        return TRUE;
    }

    for(i = 0; i < size; i++)
    {
        /* load all instrument headers */
        if((inst = FLUID_NEW(SFInst)) == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            return FALSE;
        }

        sf->inst = fluid_list_append(sf->inst, inst);
        inst->zone = NULL; /* For proper cleanup if fail (fluid_sffile_close) */
        inst->idx = i;
        READSTR(sf, &inst->name); /* Possible read failure ^ */
        READW(sf, zndx);

        if(prev_inst)
        {
            /* not first instrument? */
            if(zndx < pzndx)
            {
                FLUID_LOG(FLUID_ERR, "Instrument header indices not monotonic");
                return FALSE;
            }

            i2 = zndx - pzndx;

            while(i2--)
            {
                prev_inst->zone = fluid_list_prepend(prev_inst->zone, NULL);
            }
        }
        else if(zndx > 0)  /* 1st inst, warn if ofs >0 */
        {
            FLUID_LOG(FLUID_WARN, "%d instrument zones not referenced, discarding", zndx);
        }

        pzndx = zndx;
        prev_inst = inst; /* update instrument ptr */
    }

    FSKIP(sf, 20);
    READW(sf, zndx);

    if(zndx < pzndx)
    {
        FLUID_LOG(FLUID_ERR, "Instrument header indices not monotonic");
        return FALSE;
    }

    i2 = zndx - pzndx;

    while(i2--)
    {
        prev_inst->zone = fluid_list_prepend(prev_inst->zone, NULL);
    }

    return TRUE;
}

/* instrument bag loader */
static int load_ibag(SFData *sf, int size)
{
    fluid_list_t *inst_list;
    fluid_list_t *zone_list;
    SFZone *z, *pz = NULL;
    unsigned short genndx, modndx, pgenndx = 0, pmodndx = 0;
    int i;

    if(size % SF_BAG_SIZE || size == 0)  /* size is multiple of SF_BAG_SIZE? */
    {
        FLUID_LOG(FLUID_ERR, "Instrument bag chunk size is invalid");
        return FALSE;
    }

    inst_list = sf->inst;

    while(inst_list)
    {
        /* traverse through inst */
        zone_list = ((SFInst *)(inst_list->data))->zone;

        while(zone_list)
        {
            /* load this inst's zones */
            if((size -= SF_BAG_SIZE) < 0)
            {
                FLUID_LOG(FLUID_ERR, "Instrument bag chunk size mismatch");
                return FALSE;
            }

            if((z = FLUID_NEW(SFZone)) == NULL)
            {
                FLUID_LOG(FLUID_ERR, "Out of memory");
                return FALSE;
            }

            zone_list->data = z;
            z->gen = NULL; /* In case of failure, */
            z->mod = NULL; /* fluid_sffile_close can clean up */
            READW(sf, genndx); /* READW = possible read failure */
            READW(sf, modndx);

            if(pz)
            {
                /* if not first zone */
                if(genndx < pgenndx)
                {
                    FLUID_LOG(FLUID_ERR, "Instrument generator indices not monotonic");
                    return FALSE;
                }

                if(modndx < pmodndx)
                {
                    FLUID_LOG(FLUID_ERR, "Instrument modulator indices not monotonic");
                    return FALSE;
                }

                i = genndx - pgenndx;

                while(i--)
                {
                    pz->gen = fluid_list_prepend(pz->gen, NULL);
                }

                i = modndx - pmodndx;

                while(i--)
                {
                    pz->mod = fluid_list_prepend(pz->mod, NULL);
                }
            }

            pz = z; /* update previous zone ptr */
            pgenndx = genndx;
            pmodndx = modndx;
            zone_list = fluid_list_next(zone_list);
        }

        inst_list = fluid_list_next(inst_list);
    }

    size -= SF_BAG_SIZE;

    if(size != 0)
    {
        FLUID_LOG(FLUID_ERR, "Instrument chunk size mismatch");
        return FALSE;
    }

    READW(sf, genndx);
    READW(sf, modndx);

    if(!pz)
    {
        /* in case that all are no zoners */
        if(genndx > 0)
        {
            FLUID_LOG(FLUID_WARN, "No instrument generators and terminal index not 0");
        }

        if(modndx > 0)
        {
            FLUID_LOG(FLUID_WARN, "No instrument modulators and terminal index not 0");
        }

        return TRUE;
    }

    if(genndx < pgenndx)
    {
        FLUID_LOG(FLUID_ERR, "Instrument generator indices not monotonic");
        return FALSE;
    }

    if(modndx < pmodndx)
    {
        FLUID_LOG(FLUID_ERR, "Instrument modulator indices not monotonic");
        return FALSE;
    }

    i = genndx - pgenndx;

    while(i--)
    {
        pz->gen = fluid_list_prepend(pz->gen, NULL);
    }

    i = modndx - pmodndx;

    while(i--)
    {
        pz->mod = fluid_list_prepend(pz->mod, NULL);
    }

    return TRUE;
}

/* instrument modulator loader */
static int load_imod(SFData *sf, int size)
{
    fluid_list_t *inst_list;
    fluid_list_t *zone_list;
    fluid_list_t *mod_list;
    SFMod *m;

    inst_list = sf->inst;

    while(inst_list)
    {
        /* traverse through all inst */
        zone_list = ((SFInst *)(inst_list->data))->zone;

        while(zone_list)
        {
            /* traverse this inst's zones */
            mod_list = ((SFZone *)(zone_list->data))->mod;

            while(mod_list)
            {
                /* load zone's modulators */
                if((size -= SF_MOD_SIZE) < 0)
                {
                    FLUID_LOG(FLUID_ERR, "Instrument modulator chunk size mismatch");
                    return FALSE;
                }

                if((m = FLUID_NEW(SFMod)) == NULL)
                {
                    FLUID_LOG(FLUID_ERR, "Out of memory");
                    return FALSE;
                }

                mod_list->data = m;
                READW(sf, m->src);
                READW(sf, m->dest);
                READW(sf, m->amount);
                READW(sf, m->amtsrc);
                READW(sf, m->trans);
                mod_list = fluid_list_next(mod_list);
            }

            zone_list = fluid_list_next(zone_list);
        }

        inst_list = fluid_list_next(inst_list);
    }

    /*
       If there isn't even a terminal record
       Hmmm, the specs say there should be one, but..
     */
    if(size == 0)
    {
        return TRUE;
    }

    size -= SF_MOD_SIZE;

    if(size != 0)
    {
        FLUID_LOG(FLUID_ERR, "Instrument modulator chunk size mismatch");
        return FALSE;
    }

    FSKIP(sf, SF_MOD_SIZE); /* terminal mod */

    return TRUE;
}

/* load instrument generators (see load_pgen for loading rules) */
int load_igen(SFData *sf, int size)
{
    fluid_list_t *dup;
    fluid_list_t *inst_list;
    fluid_list_t *zone_list;
    fluid_list_t *gen_list;
    SFZone *zone;
    SFGen *g;
    SFInst *inst;
    SFGenAmount genval;
    unsigned short genid;
    int level, skip, drop, discarded;

    inst_list = sf->inst;

    /* traverse through all instruments */
    while(inst_list)
    {
        inst = fluid_list_get(inst_list);

        discarded = FALSE;
        zone_list = inst->zone;

        /* traverse this instrument's zones */
        while(zone_list)
        {
            zone = fluid_list_get(zone_list);

            level = 0;
            gen_list = zone->gen;

            while(gen_list)
            {
                /* load zone's generators */
                dup = NULL;
                skip = FALSE;
                drop = FALSE;

                if((size -= SF_GEN_SIZE) < 0)
                {
                    FLUID_LOG(FLUID_ERR, "IGEN chunk size mismatch");
                    return FALSE;
                }

                READW(sf, genid);

                if(genid == GEN_KEYRANGE)
                {
                    /* nothing precedes */
                    if(level == 0)
                    {
                        level = 1;
                        READB(sf, genval.range.lo);
                        READB(sf, genval.range.hi);
                    }
                    else
                    {
                        skip = TRUE;
                    }
                }
                else if(genid == GEN_VELRANGE)
                {
                    /* only KeyRange precedes */
                    if(level <= 1)
                    {
                        level = 2;
                        READB(sf, genval.range.lo);
                        READB(sf, genval.range.hi);
                    }
                    else
                    {
                        skip = TRUE;
                    }
                }
                else if(genid == GEN_SAMPLEID)
                {
                    /* sample is last gen */
                    level = 3;
                    READW(sf, genval.uword);
                }
                else
                {
                    level = 2;

                    if(valid_inst_genid(genid))
                    {
                        /* gen valid? */
                        READW(sf, genval.sword);
                        dup = find_gen_by_id(genid, zone->gen);
                    }
                    else
                    {
                        skip = TRUE;
                    }
                }

                if(!skip)
                {
                    if(!dup)
                    {
                        /* if gen ! dup alloc new */
                        if((g = FLUID_NEW(SFGen)) == NULL)
                        {
                            FLUID_LOG(FLUID_ERR, "Out of memory");
                            return FALSE;
                        }

                        gen_list->data = g;
                        g->id = genid;
                    }
                    else
                    {
                        g = (SFGen *)(dup->data);
                        drop = TRUE;
                    }

                    g->amount = genval;
                }
                else
                {
                    /* skip this generator */
                    discarded = TRUE;
                    drop = TRUE;
                    FSKIPW(sf);
                }

                if(!drop)
                {
                    gen_list = fluid_list_next(gen_list);    /* next gen */
                }
                else
                {
                    SLADVREM(zone->gen, gen_list);
                }

                /* GEN_SAMPLEID should be last generator */
                if (level == 3)
                {
                    break;
                }

            } /* generator loop */

            /* Anything below level 3 means it's a global zone. The global zone
             * should always be the first zone in the list, so discard any
             * other global zones we encounter */
            if(level < 3 && (zone_list != inst->zone))
            {
                /* advance to next zone before deleting the current list element */
                zone_list = fluid_list_next(zone_list);

                FLUID_LOG(FLUID_WARN, "Instrument '%s': Discarding invalid global zone",
                            inst->name);
                inst->zone = fluid_list_remove(inst->zone, zone);
                delete_zone(zone);

                /* we have already advanced the zone_list pointer, so continue with next zone */
                continue;
            }

            /* All remaining generators must be invalid and should be discarded
             * (because they come after a sampleid generator) */
            while(gen_list)
            {
                discarded = TRUE;

                if((size -= SF_GEN_SIZE) < 0)
                {
                    FLUID_LOG(FLUID_ERR, "Instrument generator chunk size mismatch");
                    return FALSE;
                }

                FSKIP(sf, SF_GEN_SIZE);
                SLADVREM(zone->gen, gen_list);
            }

            zone_list = fluid_list_next(zone_list); /* next zone */
        }

        if(discarded)
        {
            FLUID_LOG(FLUID_WARN,
                      "Instrument '%s': Some invalid generators were discarded",
                      inst->name);
        }

        inst_list = fluid_list_next(inst_list);
    }

    /* for those non-terminal record cases, grr! */
    if(size == 0)
    {
        return TRUE;
    }

    size -= SF_GEN_SIZE;

    if(size != 0)
    {
        FLUID_LOG(FLUID_ERR, "IGEN chunk size mismatch");
        return FALSE;
    }

    FSKIP(sf, SF_GEN_SIZE); /* terminal gen */

    return TRUE;
}

/* sample header loader */
static int load_shdr(SFData *sf, unsigned int size)
{
    unsigned int i;
    SFSample *p;

    if(size % SF_SHDR_SIZE || size == 0)  /* size is multiple of SHDR size? */
    {
        FLUID_LOG(FLUID_ERR, "Sample header has invalid size");
        return FALSE;
    }

    size = size / SF_SHDR_SIZE - 1;

    if(size == 0)
    {
        /* at least one sample + term record? */
        FLUID_LOG(FLUID_WARN, "File contains no samples");
        FSKIP(sf, SF_SHDR_SIZE);
        return TRUE;
    }

    /* load all sample headers */
    for(i = 0; i < size; i++)
    {
        if((p = FLUID_NEW(SFSample)) == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            return FALSE;
        }
        p->idx = i;

        sf->sample = fluid_list_prepend(sf->sample, p);
        READSTR(sf, &p->name);
        READD(sf, p->start);
        READD(sf, p->end);
        READD(sf, p->loopstart);
        READD(sf, p->loopend);
        READD(sf, p->samplerate);
        READB(sf, p->origpitch);
        READB(sf, p->pitchadj);
        FSKIPW(sf); /* skip sample link */
        READW(sf, p->sampletype);
    }

    FSKIP(sf, SF_SHDR_SIZE); /* skip terminal shdr */

    return TRUE;
}

void delete_preset(SFPreset *preset)
{
    fluid_list_t *entry;
    SFZone *zone;

    if(!preset)
    {
        return;
    }

    entry = preset->zone;

    while(entry)
    {
        zone = (SFZone *)fluid_list_get(entry);
        delete_zone(zone);
        entry = fluid_list_next(entry);
    }

    delete_fluid_list(preset->zone);

    FLUID_FREE(preset);
}

void delete_inst(SFInst *inst)
{
    fluid_list_t *entry;
    SFZone *zone;

    if(!inst)
    {
        return;
    }

    entry = inst->zone;

    while(entry)
    {
        zone = (SFZone *)fluid_list_get(entry);
        delete_zone(zone);
        entry = fluid_list_next(entry);
    }

    delete_fluid_list(inst->zone);

    FLUID_FREE(inst);
}


/* Free all elements of a zone (Preset or Instrument) */
void delete_zone(SFZone *zone)
{
    fluid_list_t *entry;

    if(!zone)
    {
        return;
    }

    entry = zone->gen;

    while(entry)
    {
        FLUID_FREE(fluid_list_get(entry));
        entry = fluid_list_next(entry);
    }

    delete_fluid_list(zone->gen);

    entry = zone->mod;

    while(entry)
    {
        FLUID_FREE(fluid_list_get(entry));
        entry = fluid_list_next(entry);
    }

    delete_fluid_list(zone->mod);

    FLUID_FREE(zone);
}

/* preset sort function, first by bank, then by preset # */
static int preset_compare_func(const void *a, const void *b)
{
    int aval, bval;

    aval = (int)(((const SFPreset *)a)->bank) << 16 | ((const SFPreset *)a)->prenum;
    bval = (int)(((const SFPreset *)b)->bank) << 16 | ((const SFPreset *)b)->prenum;

    return (aval - bval);
}

/* Find a generator by its id in the passed in list.
 *
 * @return pointer to SFGen if found, otherwise NULL
 */
static fluid_list_t *find_gen_by_id(int gen, fluid_list_t *genlist)
{
    /* is generator in gen list? */
    fluid_list_t *p;

    p = genlist;

    while(p)
    {
        if(p->data == NULL)
        {
            return NULL;
        }

        if(gen == ((SFGen *)p->data)->id)
        {
            break;
        }

        p = fluid_list_next(p);
    }

    return p;
}

/* check validity of instrument generator */
static int valid_inst_genid(unsigned short genid)
{
    size_t i;

    /* OVERRIDEROOTKEY is the last official generator, everything
     * following it are generators internal to FluidSynth and will
     * never appear in a SoundFont file. */
    if(genid > GEN_OVERRIDEROOTKEY)
    {
        return FALSE;
    }

    for(i = 0; i < FLUID_N_ELEMENTS(invalid_inst_gen); i++)
    {
        if (invalid_inst_gen[i] == genid)
        {
            return FALSE;
        }
    }

    return TRUE;
}

/* check validity of preset generator */
static int valid_preset_genid(unsigned short genid)
{
    size_t i;

    if(!valid_inst_genid(genid))
    {
        return FALSE;
    }

    for(i = 0; i < FLUID_N_ELEMENTS(invalid_preset_gen); i++)
    {
        if (invalid_preset_gen[i] == genid)
        {
            return FALSE;
        }
    }

    return TRUE;
}


static int fluid_sffile_read_wav(SFData *sf, unsigned int start, unsigned int end, short **data, char **data24)
{
    short *loaded_data = NULL;
    char *loaded_data24 = NULL;
    unsigned int num_samples;

    fluid_return_val_if_fail((end + 1) > start , -1);

    num_samples = (end + 1) - start;

    if((start * sizeof(short) > sf->samplesize) || (end * sizeof(short) > sf->samplesize))
    {
        FLUID_LOG(FLUID_ERR, "Sample offsets exceed sample data chunk");
        goto error_exit;
    }

    /* Load 16-bit sample data */
    if(sf->fcbs->fseek(sf->sffd, sf->samplepos + (start * sizeof(short)), SEEK_SET) == FLUID_FAILED)
    {
        FLUID_LOG(FLUID_ERR, "Failed to seek to sample position");
        goto error_exit;
    }

    loaded_data = FLUID_ARRAY(short, num_samples);

    if(loaded_data == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        goto error_exit;
    }

    if(sf->fcbs->fread(loaded_data, num_samples * sizeof(short), sf->sffd) == FLUID_FAILED)
    {
#if FLUID_VERSION_CHECK(FLUIDSYNTH_VERSION_MAJOR, FLUIDSYNTH_VERSION_MINOR, FLUIDSYNTH_VERSION_MICRO) < FLUID_VERSION_CHECK(2,2,0)
        if((int)(num_samples * sizeof(short)) < 0)
        {
            FLUID_LOG(FLUID_INFO,
                      "This SoundFont seems to be bigger than 2GB, which is not supported in this version of fluidsynth. "
                      "You need to use at least fluidsynth 2.2.0");
        }
#endif
        FLUID_LOG(FLUID_ERR, "Failed to read sample data");
        goto error_exit;
    }

    /* If this machine is big endian, byte swap the 16 bit samples */
    if(FLUID_IS_BIG_ENDIAN)
    {
        unsigned int i;

        for(i = 0; i < num_samples; i++)
        {
            loaded_data[i] = FLUID_LE16TOH(loaded_data[i]);
        }
    }

    *data = loaded_data;

    /* Optionally load additional 8 bit sample data for 24-bit support. Any failures while loading
     * the 24-bit sample data will be logged as errors but won't prevent the sample reading to
     * fail, as sound output is still possible with the 16-bit sample data. */
    if(sf->sample24pos)
    {
        if((start > sf->sample24size) || (end > sf->sample24size))
        {
            FLUID_LOG(FLUID_ERR, "Sample offsets exceed 24-bit sample data chunk");
            goto error24_exit;
        }

        if(sf->fcbs->fseek(sf->sffd, sf->sample24pos + start, SEEK_SET) == FLUID_FAILED)
        {
            FLUID_LOG(FLUID_ERR, "Failed to seek position for 24-bit sample data in data file");
            goto error24_exit;
        }

        loaded_data24 = FLUID_ARRAY(char, num_samples);

        if(loaded_data24 == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory reading 24-bit sample data");
            goto error24_exit;
        }

        if(sf->fcbs->fread(loaded_data24, num_samples, sf->sffd) == FLUID_FAILED)
        {
            FLUID_LOG(FLUID_ERR, "Failed to read 24-bit sample data");
            goto error24_exit;
        }
    }

    *data24 = loaded_data24;

    return num_samples;

error24_exit:
    FLUID_LOG(FLUID_WARN, "Ignoring 24-bit sample data, sound quality might suffer");
    FLUID_FREE(loaded_data24);
    *data24 = NULL;
    return num_samples;

error_exit:
    FLUID_FREE(loaded_data);
    FLUID_FREE(loaded_data24);
    return -1;
}


/* Ogg Vorbis loading and decompression */
#if LIBSNDFILE_SUPPORT

/* Virtual file access routines to allow loading individually compressed
 * samples from the Soundfont sample data chunk using the file callbacks
 * passing in during opening of the file */
typedef struct _sfvio_data_t
{
    SFData *sffile;
    sf_count_t start;  /* start byte offset of compressed data */
    sf_count_t end;    /* end byte offset of compressed data */
    sf_count_t offset; /* current virtual file offset from start byte offset */

} sfvio_data_t;

static sf_count_t sfvio_get_filelen(void *user_data)
{
    sfvio_data_t *data = user_data;

    return (data->end + 1) - data->start;
}

static sf_count_t sfvio_seek(sf_count_t offset, int whence, void *user_data)
{
    sfvio_data_t *data = user_data;
    SFData *sf = data->sffile;
    sf_count_t new_offset;

    switch(whence)
    {
    case SEEK_SET:
        new_offset = offset;
        break;

    case SEEK_CUR:
        new_offset = data->offset + offset;
        break;

    case SEEK_END:
        new_offset = sfvio_get_filelen(user_data) + offset;
        break;

    default:
        goto fail; /* proper error handling not possible?? */
    }

    new_offset += data->start;
    fluid_rec_mutex_lock(sf->mtx);
    if (data->start <= new_offset && new_offset <= data->end &&
        sf->fcbs->fseek(sf->sffd, new_offset, SEEK_SET) != FLUID_FAILED)
    {
        data->offset = new_offset - data->start;
    }
    fluid_rec_mutex_unlock(sf->mtx);

fail:
    return data->offset;
}

static sf_count_t sfvio_read(void *ptr, sf_count_t count, void *user_data)
{
    sfvio_data_t *data = user_data;
    SFData *sf = data->sffile;
    sf_count_t remain;

    remain = sfvio_get_filelen(user_data) - data->offset;

    if(count > remain)
    {
        count = remain;
    }

    if(count == 0)
    {
        return count;
    }

    fluid_rec_mutex_lock(sf->mtx);
    if (sf->fcbs->fseek(sf->sffd, data->start + data->offset, SEEK_SET) == FLUID_FAILED)
    {
        FLUID_LOG(FLUID_ERR, "This should never happen: fseek failed in sfvoid_read()");
        count = 0;
    }
    else
    {
        if (sf->fcbs->fread(ptr, count, sf->sffd) == FLUID_FAILED)
        {
            FLUID_LOG(FLUID_ERR, "Failed to read compressed sample data");
            count = 0;
        }
    }
    fluid_rec_mutex_unlock(sf->mtx);

    data->offset += count;

    return count;
}

static sf_count_t sfvio_tell(void *user_data)
{
    sfvio_data_t *data = user_data;

    return data->offset;
}

/**
 * Read Ogg Vorbis compressed data from the Soundfont and decompress it, returning the number of samples
 * in the decompressed WAV. Only 16-bit mono samples are supported.
 *
 * Note that this function takes byte indices for start and end source data. The sample headers in SF3
 * files use byte indices, so those pointers can be passed directly to this function.
 *
 * This function uses a virtual file structure in order to read the Ogg Vorbis
 * data from arbitrary locations in the source file.
 */
static int fluid_sffile_read_vorbis(SFData *sf, unsigned int start_byte, unsigned int end_byte, short **data)
{
    SNDFILE *sndfile;
    SF_INFO sfinfo;
    SF_VIRTUAL_IO sfvio =
    {
        sfvio_get_filelen,
        sfvio_seek,
        sfvio_read,
        NULL,
        sfvio_tell
    };
    sfvio_data_t sfdata;
    short *wav_data = NULL;

    if((start_byte > sf->samplesize) || (end_byte > sf->samplesize))
    {
        FLUID_LOG(FLUID_ERR, "Ogg Vorbis data offsets exceed sample data chunk");
        return -1;
    }

    // Initialize file position indicator and SF_INFO structure
    sfdata.sffile = sf;
    sfdata.start = sf->samplepos + start_byte;
    sfdata.end = sf->samplepos + end_byte;
    sfdata.offset = -1;

    /* Seek to sfdata.start, the beginning of Ogg Vorbis data in Soundfont */
    sfvio_seek(0, SEEK_SET, &sfdata);
    if (sfdata.offset != 0)
    {
        FLUID_LOG(FLUID_ERR, "Failed to seek to compressed sample position");
        return -1;
    }

    FLUID_MEMSET(&sfinfo, 0, sizeof(sfinfo));

    // Open sample as a virtual file
    sndfile = sf_open_virtual(&sfvio, SFM_READ, &sfinfo, &sfdata);

    if(!sndfile)
    {
        FLUID_LOG(FLUID_ERR, "sf_open_virtual(): %s", sf_strerror(sndfile));
        return -1;
    }

    // Empty sample
    if(sfinfo.frames <= 0 || sfinfo.channels <= 0)
    {
        FLUID_LOG(FLUID_DBG, "Empty decompressed sample");
        *data = NULL;
        sf_close(sndfile);
        return 0;
    }

    // Mono sample
    if(sfinfo.channels != 1)
    {
        FLUID_LOG(FLUID_DBG, "Unsupported channel count %d in ogg sample", sfinfo.channels);
        goto error_exit;
    }

    if((sfinfo.format & SF_FORMAT_OGG) == 0)
    {
        FLUID_LOG(FLUID_WARN, "OGG sample is not OGG compressed, this is not officially supported");
    }

    wav_data = FLUID_ARRAY(short, sfinfo.frames * sfinfo.channels);

    if(!wav_data)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        goto error_exit;
    }

    /* Automatically decompresses the Ogg Vorbis data to 16-bit PCM */
    if(sf_readf_short(sndfile, wav_data, sfinfo.frames) < sfinfo.frames)
    {
        FLUID_LOG(FLUID_DBG, "Decompression failed!");
        FLUID_LOG(FLUID_ERR, "sf_readf_short(): %s", sf_strerror(sndfile));
        goto error_exit;
    }

    sf_close(sndfile);

    *data = wav_data;

    return sfinfo.frames;

error_exit:
    FLUID_FREE(wav_data);
    sf_close(sndfile);
    return -1;
}
#else
static int fluid_sffile_read_vorbis(SFData *sf, unsigned int start_byte, unsigned int end_byte, short **data)
{
    return -1;
}
#endif
