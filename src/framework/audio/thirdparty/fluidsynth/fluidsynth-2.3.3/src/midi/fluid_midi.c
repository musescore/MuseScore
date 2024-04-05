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

#include "fluid_midi.h"
#include "fluid_sys.h"
#include "fluid_synth.h"
#include "fluid_settings.h"


static int fluid_midi_event_length(unsigned char event);
static int fluid_isasciistring(char *s);
static long fluid_getlength(const unsigned char *s);


/* Read the entire contents of a file into memory, allocating enough memory
 * for the file, and returning the length and the buffer.
 * Note: This rewinds the file to the start before reading.
 * Returns NULL if there was an error reading or allocating memory.
 */
typedef FILE  *fluid_file;
static char *fluid_file_read_full(fluid_file fp, size_t *length);
static void fluid_midi_event_set_sysex_LOCAL(fluid_midi_event_t *evt, int type, void *data, int size, int dynamic);
static void fluid_midi_event_get_sysex_LOCAL(fluid_midi_event_t *evt, void **data, int *size);
#define READ_FULL_INITIAL_BUFLEN 1024

static fluid_track_t *new_fluid_track(int num);
static void delete_fluid_track(fluid_track_t *track);
static int fluid_track_set_name(fluid_track_t *track, char *name);
static int fluid_track_add_event(fluid_track_t *track, fluid_midi_event_t *evt);
static fluid_midi_event_t *fluid_track_next_event(fluid_track_t *track);
static int fluid_track_get_duration(fluid_track_t *track);
static int fluid_track_reset(fluid_track_t *track);

static int fluid_player_add_track(fluid_player_t *player, fluid_track_t *track);
static int fluid_player_callback(void *data, unsigned int msec);
static int fluid_player_reset(fluid_player_t *player);
static int fluid_player_load(fluid_player_t *player, fluid_playlist_item *item);
static void fluid_player_advancefile(fluid_player_t *player);
static void fluid_player_playlist_load(fluid_player_t *player, unsigned int msec);
static void fluid_player_update_tempo(fluid_player_t *player);

static fluid_midi_file *new_fluid_midi_file(const char *buffer, size_t length);
static void delete_fluid_midi_file(fluid_midi_file *mf);
static int fluid_midi_file_read_mthd(fluid_midi_file *midifile);
static int fluid_midi_file_load_tracks(fluid_midi_file *midifile, fluid_player_t *player);
static int fluid_midi_file_read_track(fluid_midi_file *mf, fluid_player_t *player, int num);
static int fluid_midi_file_read_event(fluid_midi_file *mf, fluid_track_t *track);
static int fluid_midi_file_read_varlen(fluid_midi_file *mf);
static int fluid_midi_file_getc(fluid_midi_file *mf);
static int fluid_midi_file_push(fluid_midi_file *mf, int c);
static int fluid_midi_file_read(fluid_midi_file *mf, void *buf, int len);
static int fluid_midi_file_skip(fluid_midi_file *mf, int len);
static int fluid_midi_file_eof(fluid_midi_file *mf);
static int fluid_midi_file_read_tracklen(fluid_midi_file *mf);
static int fluid_midi_file_eot(fluid_midi_file *mf);
static int fluid_midi_file_get_division(fluid_midi_file *midifile);


/***************************************************************
 *
 *                      MIDIFILE
 */

/**
 * Check if a file is a MIDI file.
 * @param filename Path to the file to check
 * @return TRUE if it could be a MIDI file, FALSE otherwise
 *
 * The current implementation only checks for the "MThd" header in the file.
 * It is useful only to distinguish between SoundFont and MIDI files.
 */
int fluid_is_midifile(const char *filename)
{
    FILE    *fp;
    uint32_t id;
    int      retcode = FALSE;

    do
    {
        if((fp = fluid_file_open(filename, NULL)) == NULL)
        {
            return retcode;
        }
        
        if(FLUID_FREAD(&id, sizeof(id), 1, fp) != 1)
        {
            break;
        }

        retcode = (id == FLUID_FOURCC('M', 'T', 'h', 'd'));
    }
    while(0);

    FLUID_FCLOSE(fp);

    return retcode;
}

/**
 * Return a new MIDI file handle for parsing an already-loaded MIDI file.
 * @internal
 * @param buffer Pointer to full contents of MIDI file (borrows the pointer).
 *  The caller must not free buffer until after the fluid_midi_file is deleted.
 * @param length Size of the buffer in bytes.
 * @return New MIDI file handle or NULL on error.
 */
fluid_midi_file *
new_fluid_midi_file(const char *buffer, size_t length)
{
    fluid_midi_file *mf;

    if(length > INT_MAX)
    {
        FLUID_LOG(FLUID_ERR, "Refusing to open a MIDI file which is bigger than 2GiB");
        return NULL;
    }

    mf = FLUID_NEW(fluid_midi_file);
    if(mf == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(mf, 0, sizeof(fluid_midi_file));

    mf->c = -1;
    mf->running_status = -1;

    mf->buffer = buffer;
    mf->buf_len = (int)length;
    mf->buf_pos = 0;
    mf->eof = FALSE;

    if(fluid_midi_file_read_mthd(mf) != FLUID_OK)
    {
        FLUID_FREE(mf);
        return NULL;
    }

    return mf;
}

static char *
fluid_file_read_full(fluid_file fp, size_t *length)
{
    size_t buflen;
    char *buffer;
    size_t n;

    /* Work out the length of the file in advance */
    if(FLUID_FSEEK(fp, 0, SEEK_END) != 0)
    {
        FLUID_LOG(FLUID_ERR, "File load: Could not seek within file");
        return NULL;
    }

    buflen = ftell(fp);

    if(FLUID_FSEEK(fp, 0, SEEK_SET) != 0)
    {
        FLUID_LOG(FLUID_ERR, "File load: Could not seek within file");
        return NULL;
    }

    FLUID_LOG(FLUID_DBG, "File load: Allocating %lu bytes", (unsigned long)buflen);
    buffer = FLUID_MALLOC(buflen);

    if(buffer == NULL)
    {
        FLUID_LOG(FLUID_PANIC, "Out of memory");
        return NULL;
    }

    n = FLUID_FREAD(buffer, 1, buflen, fp);

    if(n != buflen)
    {
        FLUID_LOG(FLUID_ERR, "Only read %lu bytes; expected %lu", (unsigned long)n,
                  (unsigned long)buflen);
        FLUID_FREE(buffer);
        return NULL;
    };

    *length = n;

    return buffer;
}

/**
 * Delete a MIDI file handle.
 * @internal
 * @param mf MIDI file handle to close and free.
 */
void
delete_fluid_midi_file(fluid_midi_file *mf)
{
    fluid_return_if_fail(mf != NULL);

    FLUID_FREE(mf);
}

/*
 * Gets the next byte in a MIDI file, taking into account previous running status.
 *
 * returns -1 if EOF or read error
 */
int
fluid_midi_file_getc(fluid_midi_file *mf)
{
    unsigned char c;

    if(mf->c >= 0)
    {
        c = mf->c;
        mf->c = -1;
    }
    else
    {
        if(mf->buf_pos >= mf->buf_len)
        {
            mf->eof = TRUE;
            return -1;
        }

        c = mf->buffer[mf->buf_pos++];
        mf->trackpos++;
    }

    return (int) c;
}

/*
 * Saves a byte to be returned the next time fluid_midi_file_getc() is called,
 * when it is necessary according to running status.
 */
int
fluid_midi_file_push(fluid_midi_file *mf, int c)
{
    mf->c = c;
    return FLUID_OK;
}

/*
 * fluid_midi_file_read
 */
int
fluid_midi_file_read(fluid_midi_file *mf, void *buf, int len)
{
    int num = len < mf->buf_len - mf->buf_pos
              ? len : mf->buf_len - mf->buf_pos;

    if(num != len)
    {
        mf->eof = TRUE;
    }

    if(num < 0)
    {
        num = 0;
    }

    /* Note: Read bytes, even if there aren't enough, but only increment
     * trackpos if successful (emulates old behaviour of fluid_midi_file_read)
     */
    FLUID_MEMCPY(buf, mf->buffer + mf->buf_pos, num);
    mf->buf_pos += num;

    if(num == len)
    {
        mf->trackpos += num;
    }

#if DEBUG
    else
    {
        FLUID_LOG(FLUID_DBG, "Could not read the requested number of bytes");
    }

#endif
    return (num != len) ? FLUID_FAILED : FLUID_OK;
}

/*
 * fluid_midi_file_skip
 */
int
fluid_midi_file_skip(fluid_midi_file *mf, int skip)
{
    int new_pos = mf->buf_pos + skip;

    /* Mimic the behaviour of fseek: Error to seek past the start of file, but
     * OK to seek past end (this just puts it into the EOF state). */
    if(new_pos < 0)
    {
        FLUID_LOG(FLUID_ERR, "Failed to seek position in file");
        return FLUID_FAILED;
    }

    /* Clear the EOF flag, even if moved past the end of the file (this is
     * consistent with the behaviour of fseek). */
    mf->eof = FALSE;
    mf->buf_pos = new_pos;
    return FLUID_OK;
}

/*
 * fluid_midi_file_eof
 */
int fluid_midi_file_eof(fluid_midi_file *mf)
{
    /* Note: This does not simply test whether the file read pointer is past
     * the end of the file. It mimics the behaviour of feof by actually
     * testing the stateful EOF condition, which is set to TRUE if getc or
     * fread have attempted to read past the end (but not if they have
     * precisely reached the end), but reset to FALSE upon a successful seek.
     */
    return mf->eof;
}

/*
 * fluid_midi_file_read_mthd
 */
int
fluid_midi_file_read_mthd(fluid_midi_file *mf)
{
    char mthd[14];

    if(fluid_midi_file_read(mf, mthd, sizeof(mthd)) != FLUID_OK)
    {
        return FLUID_FAILED;
    }

    if((FLUID_STRNCMP(mthd, "MThd", 4) != 0) || (mthd[7] != 6)
            || (mthd[9] > 2))
    {
        FLUID_LOG(FLUID_ERR,
                  "Doesn't look like a MIDI file: invalid MThd header");
        return FLUID_FAILED;
    }

    mf->type = mthd[9];
    mf->ntracks = (unsigned) mthd[11];
    mf->ntracks += (unsigned int)(mthd[10]) << 16;

    if((signed char)mthd[12] < 0)
    {
        mf->uses_smpte = 1;
        mf->smpte_fps = -(signed char)mthd[12];
        mf->smpte_res = (unsigned) mthd[13];
        FLUID_LOG(FLUID_ERR, "File uses SMPTE timing -- Not implemented yet");
        return FLUID_FAILED;
    }
    else
    {
        mf->uses_smpte = 0;
        mf->division = ((unsigned)mthd[12] << 8) | ((unsigned)mthd[13] & 0xff);
        FLUID_LOG(FLUID_DBG, "Division=%d", mf->division);
    }

    return FLUID_OK;
}

/*
 * fluid_midi_file_load_tracks
 */
int
fluid_midi_file_load_tracks(fluid_midi_file *mf, fluid_player_t *player)
{
    int i;

    for(i = 0; i < mf->ntracks; i++)
    {
        if(fluid_midi_file_read_track(mf, player, i) != FLUID_OK)
        {
            return FLUID_FAILED;
        }
    }

    return FLUID_OK;
}

/*
 * fluid_isasciistring
 */
int
fluid_isasciistring(char *s)
{
    /* From ctype.h */
#define fluid_isascii(c)    (((c) & ~0x7f) == 0)

    size_t i, len = FLUID_STRLEN(s);

    for(i = 0; i < len; i++)
    {
        if(!fluid_isascii(s[i]))
        {
            return 0;
        }
    }

    return 1;

#undef fluid_isascii
}

/*
 * fluid_getlength
 */
long
fluid_getlength(const unsigned char *s)
{
    long i = 0;
    i = s[3] | (s[2] << 8) | (s[1] << 16) | (s[0] << 24);
    return i;
}

/*
 * fluid_midi_file_read_tracklen
 */
int
fluid_midi_file_read_tracklen(fluid_midi_file *mf)
{
    unsigned char length[5];

    if(fluid_midi_file_read(mf, length, 4) != FLUID_OK)
    {
        return FLUID_FAILED;
    }

    mf->tracklen = fluid_getlength(length);
    mf->trackpos = 0;
    mf->eot = 0;
    return FLUID_OK;
}

/*
 * fluid_midi_file_eot
 */
int
fluid_midi_file_eot(fluid_midi_file *mf)
{
#if DEBUG

    if(mf->trackpos > mf->tracklen)
    {
        printf("track overrun: %d > %d\n", mf->trackpos, mf->tracklen);
    }

#endif
    return mf->eot || (mf->trackpos >= mf->tracklen);
}

/*
 * fluid_midi_file_read_track
 */
int
fluid_midi_file_read_track(fluid_midi_file *mf, fluid_player_t *player, int num)
{
    fluid_track_t *track;
    unsigned char id[5], length[5];
    int found_track = 0;
    int skip;

    if(fluid_midi_file_read(mf, id, 4) != FLUID_OK)
    {
        return FLUID_FAILED;
    }

    id[4] = '\0';
    mf->dtime = 0;

    while(!found_track)
    {

        if(fluid_isasciistring((char *) id) == 0)
        {
            FLUID_LOG(FLUID_ERR,
                      "A non-ascii track header found, corrupt file");
            return FLUID_FAILED;

        }
        else if(FLUID_STRCMP((char *) id, "MTrk") == 0)
        {

            found_track = 1;

            if(fluid_midi_file_read_tracklen(mf) != FLUID_OK)
            {
                return FLUID_FAILED;
            }

            track = new_fluid_track(num);

            if(track == NULL)
            {
                FLUID_LOG(FLUID_ERR, "Out of memory");
                return FLUID_FAILED;
            }

            while(!fluid_midi_file_eot(mf))
            {
                if(fluid_midi_file_read_event(mf, track) != FLUID_OK)
                {
                    delete_fluid_track(track);
                    return FLUID_FAILED;
                }
            }

            /* Skip remaining track data, if any */
            if(mf->trackpos < mf->tracklen)
            {
                if(fluid_midi_file_skip(mf, mf->tracklen - mf->trackpos) != FLUID_OK)
                {
                    delete_fluid_track(track);
                    return FLUID_FAILED;
                }
            }

            if(fluid_player_add_track(player, track) != FLUID_OK)
            {
                delete_fluid_track(track);
                return FLUID_FAILED;
            }

        }
        else
        {
            found_track = 0;

            if(fluid_midi_file_read(mf, length, 4) != FLUID_OK)
            {
                return FLUID_FAILED;
            }

            skip = fluid_getlength(length);

            /* fseek(mf->fp, skip, SEEK_CUR); */
            if(fluid_midi_file_skip(mf, skip) != FLUID_OK)
            {
                return FLUID_FAILED;
            }
        }
    }

    if(fluid_midi_file_eof(mf))
    {
        FLUID_LOG(FLUID_ERR, "Unexpected end of file");
        return FLUID_FAILED;
    }

    return FLUID_OK;
}

/*
 * fluid_midi_file_read_varlen
 */
int
fluid_midi_file_read_varlen(fluid_midi_file *mf)
{
    int i;
    int c;
    mf->varlen = 0;

    for(i = 0;; i++)
    {
        if(i == 4)
        {
            FLUID_LOG(FLUID_ERR, "Invalid variable length number");
            return FLUID_FAILED;
        }

        c = fluid_midi_file_getc(mf);

        if(c < 0)
        {
            FLUID_LOG(FLUID_ERR, "Unexpected end of file");
            return FLUID_FAILED;
        }

        if(c & 0x80)
        {
            mf->varlen |= (int)(c & 0x7F);
            mf->varlen <<= 7;
        }
        else
        {
            mf->varlen += c;
            break;
        }
    }

    return FLUID_OK;
}

/*
 * fluid_midi_file_read_event
 */
int
fluid_midi_file_read_event(fluid_midi_file *mf, fluid_track_t *track)
{
    int status;
    int type;
    int tempo;
    unsigned char *metadata = NULL;
    unsigned char *dyn_buf = NULL;
    unsigned char static_buf[256];
    int nominator, denominator, clocks, notes;
    fluid_midi_event_t *evt;
    int channel = 0;
    int param1 = 0;
    int param2 = 0;
    int size;

    /* read the delta-time of the event */
    if(fluid_midi_file_read_varlen(mf) != FLUID_OK)
    {
        return FLUID_FAILED;
    }

    mf->dtime += mf->varlen;

    /* read the status byte */
    status = fluid_midi_file_getc(mf);

    if(status < 0)
    {
        FLUID_LOG(FLUID_ERR, "Unexpected end of file");
        return FLUID_FAILED;
    }

    /* not a valid status byte: use the running status instead */
    if((status & 0x80) == 0)
    {
        if((mf->running_status & 0x80) == 0)
        {
            FLUID_LOG(FLUID_ERR, "Undefined status and invalid running status");
            return FLUID_FAILED;
        }

        fluid_midi_file_push(mf, status);
        status = mf->running_status;
    }

    /* check what message we have */

    mf->running_status = status;

    if(status == MIDI_SYSEX)    /* system exclusif */
    {
        /* read the length of the message */
        if(fluid_midi_file_read_varlen(mf) != FLUID_OK)
        {
            return FLUID_FAILED;
        }

        if(mf->varlen)
        {
            FLUID_LOG(FLUID_DBG, "%s: %d: alloc metadata, len = %d", __FILE__,
                      __LINE__, mf->varlen);
            metadata = FLUID_MALLOC(mf->varlen + 1);

            if(metadata == NULL)
            {
                FLUID_LOG(FLUID_PANIC, "Out of memory");
                return FLUID_FAILED;
            }

            /* read the data of the message */
            if(fluid_midi_file_read(mf, metadata, mf->varlen) != FLUID_OK)
            {
                FLUID_FREE(metadata);
                return FLUID_FAILED;
            }

            evt = new_fluid_midi_event();

            if(evt == NULL)
            {
                FLUID_LOG(FLUID_ERR, "Out of memory");
                FLUID_FREE(metadata);
                return FLUID_FAILED;
            }

            evt->dtime = mf->dtime;
            size = mf->varlen;

            if(metadata[mf->varlen - 1] == MIDI_EOX)
            {
                size--;
            }

            /* Add SYSEX event and indicate that its dynamically allocated and should be freed with event */
            fluid_midi_event_set_sysex(evt, metadata, size, TRUE);
            fluid_track_add_event(track, evt);
            mf->dtime = 0;
        }

        return FLUID_OK;

    }
    else if(status == MIDI_META_EVENT)      /* meta events */
    {

        int result = FLUID_OK;

        /* get the type of the meta message */
        type = fluid_midi_file_getc(mf);

        if(type < 0)
        {
            FLUID_LOG(FLUID_ERR, "Unexpected end of file");
            return FLUID_FAILED;
        }

        /* get the length of the data part */
        if(fluid_midi_file_read_varlen(mf) != FLUID_OK)
        {
            return FLUID_FAILED;
        }

        if(mf->varlen < 255)
        {
            metadata = &static_buf[0];
        }
        else
        {
            FLUID_LOG(FLUID_DBG, "%s: %d: alloc metadata, len = %d", __FILE__,
                      __LINE__, mf->varlen);
            dyn_buf = FLUID_MALLOC(mf->varlen + 1);

            if(dyn_buf == NULL)
            {
                FLUID_LOG(FLUID_PANIC, "Out of memory");
                return FLUID_FAILED;
            }

            metadata = dyn_buf;
        }

        /* read the data */
        if(mf->varlen)
        {
            if(fluid_midi_file_read(mf, metadata, mf->varlen) != FLUID_OK)
            {
                if(dyn_buf)
                {
                    FLUID_FREE(dyn_buf);
                }

                return FLUID_FAILED;
            }
        }

        /* handle meta data */
        switch(type)
        {

        case MIDI_COPYRIGHT:
            metadata[mf->varlen] = 0;
            break;

        case MIDI_TRACK_NAME:
            metadata[mf->varlen] = 0;
            fluid_track_set_name(track, (char *) metadata);
            break;

        case MIDI_INST_NAME:
            metadata[mf->varlen] = 0;
            break;

        case MIDI_LYRIC:
        case MIDI_TEXT:
        {
            void *tmp;
            int size = mf->varlen + 1;

            /* NULL terminate strings for safety */
            metadata[size - 1] = '\0';

            evt = new_fluid_midi_event();

            if(evt == NULL)
            {
                FLUID_LOG(FLUID_ERR, "Out of memory");
                result = FLUID_FAILED;
                break;
            }

            evt->dtime = mf->dtime;

            tmp = FLUID_MALLOC(size);

            if(tmp == NULL)
            {
                FLUID_LOG(FLUID_PANIC, "Out of memory");
                delete_fluid_midi_event(evt);
                evt = NULL;
                result = FLUID_FAILED;
                break;
            }

            FLUID_MEMCPY(tmp, metadata, size);

            fluid_midi_event_set_sysex_LOCAL(evt, type, tmp, size, TRUE);
            fluid_track_add_event(track, evt);
            mf->dtime = 0;
        }
        break;

        case MIDI_MARKER:
            break;

        case MIDI_CUE_POINT:
            break; /* don't care much for text events */

        case MIDI_EOT:
            if(mf->varlen != 0)
            {
                FLUID_LOG(FLUID_ERR, "Invalid length for EndOfTrack event");
                result = FLUID_FAILED;
                break;
            }

            mf->eot = 1;
            evt = new_fluid_midi_event();

            if(evt == NULL)
            {
                FLUID_LOG(FLUID_ERR, "Out of memory");
                result = FLUID_FAILED;
                break;
            }

            evt->dtime = mf->dtime;
            evt->type = MIDI_EOT;
            fluid_track_add_event(track, evt);
            mf->dtime = 0;
            break;

        case MIDI_SET_TEMPO:
            if(mf->varlen != 3)
            {
                FLUID_LOG(FLUID_ERR,
                          "Invalid length for SetTempo meta event");
                result = FLUID_FAILED;
                break;
            }

            tempo = (metadata[0] << 16) + (metadata[1] << 8) + metadata[2];
            evt = new_fluid_midi_event();

            if(evt == NULL)
            {
                FLUID_LOG(FLUID_ERR, "Out of memory");
                result = FLUID_FAILED;
                break;
            }

            evt->dtime = mf->dtime;
            evt->type = MIDI_SET_TEMPO;
            evt->channel = 0;
            evt->param1 = tempo;
            evt->param2 = 0;
            fluid_track_add_event(track, evt);
            mf->dtime = 0;
            break;

        case MIDI_SMPTE_OFFSET:
            if(mf->varlen != 5)
            {
                FLUID_LOG(FLUID_ERR,
                          "Invalid length for SMPTE Offset meta event");
                result = FLUID_FAILED;
                break;
            }

            break; /* we don't use smtp */

        case MIDI_TIME_SIGNATURE:
            if(mf->varlen != 4)
            {
                FLUID_LOG(FLUID_ERR,
                          "Invalid length for TimeSignature meta event");
                result = FLUID_FAILED;
                break;
            }

            nominator = metadata[0];
            denominator = pow(2.0, (double) metadata[1]);
            clocks = metadata[2];
            notes = metadata[3];

            FLUID_LOG(FLUID_DBG,
                      "signature=%d/%d, metronome=%d, 32nd-notes=%d",
                      nominator, denominator, clocks, notes);

            break;

        case MIDI_KEY_SIGNATURE:
            if(mf->varlen != 2)
            {
                FLUID_LOG(FLUID_ERR,
                          "Invalid length for KeySignature meta event");
                result = FLUID_FAILED;
                break;
            }

            /* We don't care about key signatures anyway */
            /* sf = metadata[0];
            mi = metadata[1]; */
            break;

        case MIDI_SEQUENCER_EVENT:
            break;

        default:
            break;
        }

        if(dyn_buf)
        {
            FLUID_LOG(FLUID_DBG, "%s: %d: free metadata", __FILE__, __LINE__);
            FLUID_FREE(dyn_buf);
        }

        return result;

    }
    else     /* channel messages */
    {

        type = status & 0xf0;
        channel = status & 0x0f;

        /* all channel message have at least 1 byte of associated data */
        if((param1 = fluid_midi_file_getc(mf)) < 0)
        {
            FLUID_LOG(FLUID_ERR, "Unexpected end of file");
            return FLUID_FAILED;
        }

        switch(type)
        {

        case NOTE_ON:
            if((param2 = fluid_midi_file_getc(mf)) < 0)
            {
                FLUID_LOG(FLUID_ERR, "Unexpected end of file");
                return FLUID_FAILED;
            }

            break;

        case NOTE_OFF:
            if((param2 = fluid_midi_file_getc(mf)) < 0)
            {
                FLUID_LOG(FLUID_ERR, "Unexpected end of file");
                return FLUID_FAILED;
            }

            break;

        case KEY_PRESSURE:
            if((param2 = fluid_midi_file_getc(mf)) < 0)
            {
                FLUID_LOG(FLUID_ERR, "Unexpected end of file");
                return FLUID_FAILED;
            }

            break;

        case CONTROL_CHANGE:
            if((param2 = fluid_midi_file_getc(mf)) < 0)
            {
                FLUID_LOG(FLUID_ERR, "Unexpected end of file");
                return FLUID_FAILED;
            }

            break;

        case PROGRAM_CHANGE:
            break;

        case CHANNEL_PRESSURE:
            break;

        case PITCH_BEND:
            if((param2 = fluid_midi_file_getc(mf)) < 0)
            {
                FLUID_LOG(FLUID_ERR, "Unexpected end of file");
                return FLUID_FAILED;
            }

            param1 = ((param2 & 0x7f) << 7) | (param1 & 0x7f);
            param2 = 0;
            break;

        default:
            /* Can't possibly happen !? */
            FLUID_LOG(FLUID_ERR, "Unrecognized MIDI event");
            return FLUID_FAILED;
        }

        evt = new_fluid_midi_event();

        if(evt == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            return FLUID_FAILED;
        }

        evt->dtime = mf->dtime;
        evt->type = type;
        evt->channel = channel;
        evt->param1 = param1;
        evt->param2 = param2;
        fluid_track_add_event(track, evt);
        mf->dtime = 0;
    }

    return FLUID_OK;
}

/*
 * fluid_midi_file_get_division
 */
int
fluid_midi_file_get_division(fluid_midi_file *midifile)
{
    return midifile->division;
}

/******************************************************
 *
 *     fluid_track_t
 */

/**
 * Create a MIDI event structure.
 * @return New MIDI event structure or NULL when out of memory.
 */
fluid_midi_event_t *
new_fluid_midi_event()
{
    fluid_midi_event_t *evt;
    evt = FLUID_NEW(fluid_midi_event_t);

    if(evt == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    evt->dtime = 0;
    evt->type = 0;
    evt->channel = 0;
    evt->param1 = 0;
    evt->param2 = 0;
    evt->next = NULL;
    evt->paramptr = NULL;
    return evt;
}

/**
 * Delete MIDI event structure.
 * @param evt MIDI event structure
 */
void
delete_fluid_midi_event(fluid_midi_event_t *evt)
{
    fluid_midi_event_t *temp;
    fluid_return_if_fail(evt != NULL);

    while(evt)
    {
        temp = evt->next;

        /* Dynamic SYSEX event? - free (param2 indicates if dynamic) */
        if((evt->type == MIDI_SYSEX || (evt-> type == MIDI_TEXT) || (evt->type == MIDI_LYRIC)) &&
                evt->paramptr && evt->param2)
        {
            FLUID_FREE(evt->paramptr);
        }

        FLUID_FREE(evt);
        evt = temp;
    }
}

/**
 * Get the event type field of a MIDI event structure.
 * @param evt MIDI event structure
 * @return Event type field (MIDI status byte without channel)
 */
int
fluid_midi_event_get_type(const fluid_midi_event_t *evt)
{
    return evt->type;
}

/**
 * Set the event type field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param type Event type field (MIDI status byte without channel)
 * @return Always returns #FLUID_OK
 */
int
fluid_midi_event_set_type(fluid_midi_event_t *evt, int type)
{
    evt->type = type;
    return FLUID_OK;
}

/**
 * Get the channel field of a MIDI event structure.
 * @param evt MIDI event structure
 * @return Channel field
 */
int
fluid_midi_event_get_channel(const fluid_midi_event_t *evt)
{
    return evt->channel;
}

/**
 * Set the channel field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param chan MIDI channel field
 * @return Always returns #FLUID_OK
 */
int
fluid_midi_event_set_channel(fluid_midi_event_t *evt, int chan)
{
    evt->channel = chan;
    return FLUID_OK;
}

/**
 * Get the key field of a MIDI event structure.
 * @param evt MIDI event structure
 * @return MIDI note number (0-127)
 */
int
fluid_midi_event_get_key(const fluid_midi_event_t *evt)
{
    return evt->param1;
}

/**
 * Set the key field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param v MIDI note number (0-127)
 * @return Always returns #FLUID_OK
 */
int
fluid_midi_event_set_key(fluid_midi_event_t *evt, int v)
{
    evt->param1 = v;
    return FLUID_OK;
}

/**
 * Get the velocity field of a MIDI event structure.
 * @param evt MIDI event structure
 * @return MIDI velocity number (0-127)
 */
int
fluid_midi_event_get_velocity(const fluid_midi_event_t *evt)
{
    return evt->param2;
}

/**
 * Set the velocity field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param v MIDI velocity value
 * @return Always returns #FLUID_OK
 */
int
fluid_midi_event_set_velocity(fluid_midi_event_t *evt, int v)
{
    evt->param2 = v;
    return FLUID_OK;
}

/**
 * Get the control number of a MIDI event structure.
 * @param evt MIDI event structure
 * @return MIDI control number
 */
int
fluid_midi_event_get_control(const fluid_midi_event_t *evt)
{
    return evt->param1;
}

/**
 * Set the control field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param v MIDI control number
 * @return Always returns #FLUID_OK
 */
int
fluid_midi_event_set_control(fluid_midi_event_t *evt, int v)
{
    evt->param1 = v;
    return FLUID_OK;
}

/**
 * Get the value field from a MIDI event structure.
 * @param evt MIDI event structure
 * @return Value field
 */
int
fluid_midi_event_get_value(const fluid_midi_event_t *evt)
{
    return evt->param2;
}

/**
 * Set the value field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param v Value to assign
 * @return Always returns #FLUID_OK
 */
int
fluid_midi_event_set_value(fluid_midi_event_t *evt, int v)
{
    evt->param2 = v;
    return FLUID_OK;
}

/**
 * Get the program field of a MIDI event structure.
 * @param evt MIDI event structure
 * @return MIDI program number (0-127)
 */
int
fluid_midi_event_get_program(const fluid_midi_event_t *evt)
{
    return evt->param1;
}

/**
 * Set the program field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param val MIDI program number (0-127)
 * @return Always returns #FLUID_OK
 */
int
fluid_midi_event_set_program(fluid_midi_event_t *evt, int val)
{
    evt->param1 = val;
    return FLUID_OK;
}

/**
 * Get the pitch field of a MIDI event structure.
 * @param evt MIDI event structure
 * @return Pitch value (14 bit value, 0-16383, 8192 is center)
 */
int
fluid_midi_event_get_pitch(const fluid_midi_event_t *evt)
{
    return evt->param1;
}

/**
 * Set the pitch field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param val Pitch value (14 bit value, 0-16383, 8192 is center)
 * @return Always returns FLUID_OK
 */
int
fluid_midi_event_set_pitch(fluid_midi_event_t *evt, int val)
{
    evt->param1 = val;
    return FLUID_OK;
}

/**
 * Assign sysex data to a MIDI event structure.
 * @param evt MIDI event structure
 * @param data Pointer to SYSEX data
 * @param size Size of SYSEX data in bytes
 * @param dynamic TRUE if the SYSEX data has been dynamically allocated and
 *   should be freed when the event is freed (only applies if event gets destroyed
 *   with delete_fluid_midi_event())
 * @return Always returns #FLUID_OK
 */
int
fluid_midi_event_set_sysex(fluid_midi_event_t *evt, void *data, int size, int dynamic)
{
    fluid_midi_event_set_sysex_LOCAL(evt, MIDI_SYSEX, data, size, dynamic);
    return FLUID_OK;
}

/**
 * Assign text data to a MIDI event structure.
 * @param evt MIDI event structure
 * @param data Pointer to text data
 * @param size Size of text data in bytes
 * @param dynamic TRUE if the data has been dynamically allocated and
 *   should be freed when the event is freed via delete_fluid_midi_event()
 * @return Always returns #FLUID_OK
 *
 * @since 2.0.0
 */
int
fluid_midi_event_set_text(fluid_midi_event_t *evt, void *data, int size, int dynamic)
{
    fluid_midi_event_set_sysex_LOCAL(evt, MIDI_TEXT, data, size, dynamic);
    return FLUID_OK;
}

/**
 * Get the text of a MIDI event structure.
 * @param evt MIDI event structure
 * @param data Pointer to return text data on.
 * @param size Pointer to return text size on.
 * @return Returns #FLUID_OK if \p data and \p size previously set by
 * fluid_midi_event_set_text() have been successfully retrieved.
 * Else #FLUID_FAILED is returned and \p data and \p size are not changed.
 * @since 2.0.3
 */
int fluid_midi_event_get_text(fluid_midi_event_t *evt, void **data, int *size)
{
    fluid_return_val_if_fail(evt != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(evt->type == MIDI_TEXT, FLUID_FAILED);

    fluid_midi_event_get_sysex_LOCAL(evt, data, size);
    return FLUID_OK;
}

/**
 * Assign lyric data to a MIDI event structure.
 * @param evt MIDI event structure
 * @param data Pointer to lyric data
 * @param size Size of lyric data in bytes
 * @param dynamic TRUE if the data has been dynamically allocated and
 *   should be freed when the event is freed via delete_fluid_midi_event()
 * @return Always returns #FLUID_OK
 *
 * @since 2.0.0
 */
int
fluid_midi_event_set_lyrics(fluid_midi_event_t *evt, void *data, int size, int dynamic)
{
    fluid_midi_event_set_sysex_LOCAL(evt, MIDI_LYRIC, data, size, dynamic);
    return FLUID_OK;
}

/**
 * Get the lyric of a MIDI event structure.
 * @param evt MIDI event structure
 * @param data Pointer to return lyric data on.
 * @param size Pointer to return lyric size on.
 * @return Returns #FLUID_OK if \p data and \p size previously set by
 * fluid_midi_event_set_lyrics() have been successfully retrieved.
 * Else #FLUID_FAILED is returned and \p data and \p size are not changed.
 * @since 2.0.3
 */
int fluid_midi_event_get_lyrics(fluid_midi_event_t *evt, void **data, int *size)
{
    fluid_return_val_if_fail(evt != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(evt->type == MIDI_LYRIC, FLUID_FAILED);

    fluid_midi_event_get_sysex_LOCAL(evt, data, size);
    return FLUID_OK;
}

static void fluid_midi_event_set_sysex_LOCAL(fluid_midi_event_t *evt, int type, void *data, int size, int dynamic)
{
    evt->type = type;
    evt->paramptr = data;
    evt->param1 = size;
    evt->param2 = dynamic;
}

static void fluid_midi_event_get_sysex_LOCAL(fluid_midi_event_t *evt, void **data, int *size)
{
    if(data)
    {
        *data = evt->paramptr;
    }

    if(size)
    {
        *size = evt->param1;
    }
}

/******************************************************
 *
 *     fluid_track_t
 */

/*
 * new_fluid_track
 */
fluid_track_t *
new_fluid_track(int num)
{
    fluid_track_t *track;
    track = FLUID_NEW(fluid_track_t);

    if(track == NULL)
    {
        return NULL;
    }

    track->name = NULL;
    track->num = num;
    track->first = NULL;
    track->cur = NULL;
    track->last = NULL;
    track->ticks = 0;
    return track;
}

/*
 * delete_fluid_track
 */
void
delete_fluid_track(fluid_track_t *track)
{
    fluid_return_if_fail(track != NULL);

    FLUID_FREE(track->name);
    delete_fluid_midi_event(track->first);
    FLUID_FREE(track);
}

/*
 * fluid_track_set_name
 */
int
fluid_track_set_name(fluid_track_t *track, char *name)
{
    size_t len;

    if(track->name != NULL)
    {
        FLUID_FREE(track->name);
    }

    if(name == NULL)
    {
        track->name = NULL;
        return FLUID_OK;
    }

    len = FLUID_STRLEN(name);
    track->name = FLUID_MALLOC(len + 1);

    if(track->name == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return FLUID_FAILED;
    }

    FLUID_STRCPY(track->name, name);
    return FLUID_OK;
}

/*
 * fluid_track_get_duration
 */
int
fluid_track_get_duration(fluid_track_t *track)
{
    int time = 0;
    fluid_midi_event_t *evt = track->first;

    while(evt != NULL)
    {
        time += evt->dtime;
        evt = evt->next;
    }

    return time;
}

/*
 * fluid_track_add_event
 */
int
fluid_track_add_event(fluid_track_t *track, fluid_midi_event_t *evt)
{
    evt->next = NULL;

    if(track->first == NULL)
    {
        track->first = evt;
        track->cur = evt;
        track->last = evt;
    }
    else
    {
        track->last->next = evt;
        track->last = evt;
    }

    return FLUID_OK;
}

/*
 * fluid_track_next_event
 */
fluid_midi_event_t *
fluid_track_next_event(fluid_track_t *track)
{
    if(track->cur != NULL)
    {
        track->cur = track->cur->next;
    }

    return track->cur;
}

/*
 * fluid_track_reset
 */
int
fluid_track_reset(fluid_track_t *track)
{
    track->ticks = 0;
    track->cur = track->first;
    return FLUID_OK;
}

/*
 * fluid_track_send_events
 */
static void
fluid_track_send_events(fluid_track_t *track,
                        fluid_synth_t *synth,
                        fluid_player_t *player,
                        unsigned int ticks,
                        int seek_ticks
                       )
{
    fluid_midi_event_t *event;
    int seeking = seek_ticks >= 0;

    if(seeking)
    {
        ticks = seek_ticks; /* update target ticks */

        if(track->ticks > ticks)
        {
            fluid_track_reset(track);    /* reset track if seeking backwards */
        }
    }

    while(1)
    {

        event = track->cur;

        if(event == NULL)
        {
            return;
        }

        /*         printf("track=%02d\tticks=%05u\ttrack=%05u\tdtime=%05u\tnext=%05u\n", */
        /*                track->num, */
        /*                ticks, */
        /*                track->ticks, */
        /*                event->dtime, */
        /*                track->ticks + event->dtime); */

        if(track->ticks + event->dtime > ticks)
        {
            return;
        }

        track->ticks += event->dtime;

        if(!player || event->type == MIDI_EOT)
        {
            /* don't send EOT events to the callback */
        }
        else if(seeking && track->ticks != ticks && (event->type == NOTE_ON || event->type == NOTE_OFF))
        {
            /* skip on/off messages */
        }
        else
        {
            if(player->playback_callback)
            {
                player->playback_callback(player->playback_userdata, event);
                if(event->type == NOTE_ON && event->param2 != 0 && !player->channel_isplaying[event->channel])
                {
                    player->channel_isplaying[event->channel] = TRUE;
                }
            }
        }

        if(event->type == MIDI_SET_TEMPO && player != NULL)
        {
            /* memorize the tempo change value coming from the MIDI file */
            fluid_atomic_int_set(&player->miditempo, event->param1);
            fluid_player_update_tempo(player);
        }

        fluid_track_next_event(track);

    }
}

/******************************************************
 *
 *     fluid_player
 */
static void
fluid_player_handle_reset_synth(void *data, const char *name, int value)
{
    fluid_player_t *player = data;
    fluid_return_if_fail(player != NULL);

    player->reset_synth_between_songs = value;
}

/**
 * Create a new MIDI player.
 * @param synth Fluid synthesizer instance to create player for
 * @return New MIDI player instance or NULL on error (out of memory)
 */
fluid_player_t *
new_fluid_player(fluid_synth_t *synth)
{
    int i;
    fluid_player_t *player;
    player = FLUID_NEW(fluid_player_t);

    if(player == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    fluid_atomic_int_set(&player->status, FLUID_PLAYER_READY);
    fluid_atomic_int_set(&player->stopping, 0);
    player->loop = 1;
    player->ntracks = 0;

    for(i = 0; i < MAX_NUMBER_OF_TRACKS; i++)
    {
        player->track[i] = NULL;
    }

    player->synth = synth;
    player->system_timer = NULL;
    player->sample_timer = NULL;
    player->playlist = NULL;
    player->currentfile = NULL;
    player->division = 0;

    /* internal tempo (from MIDI file) in micro seconds per quarter note */
    player->sync_mode = 1; /* the player follows internal tempo change */
    player->miditempo = 500000;
    /* external tempo in micro seconds per quarter note */
    player->exttempo = 500000;
    /* tempo multiplier */
    player->multempo = 1.0F;

    player->deltatime = 4.0;
    player->cur_msec = 0;
    player->cur_ticks = 0;
    player->end_msec = -1;
    player->end_pedals_disabled = 0;
    player->last_callback_ticks = -1;
    fluid_atomic_int_set(&player->seek_ticks, -1);
    fluid_player_set_playback_callback(player, fluid_synth_handle_midi_event, synth);
    fluid_player_set_tick_callback(player, NULL, NULL);
    player->use_system_timer = fluid_settings_str_equal(synth->settings,
                               "player.timing-source", "system");
    if(player->use_system_timer)
    {
        player->system_timer = new_fluid_timer((int) player->deltatime,
                                               fluid_player_callback, player, TRUE, FALSE, TRUE);

        if(player->system_timer == NULL)
        {
            goto err;
        }
    }
    else
    {
        player->sample_timer = new_fluid_sample_timer(player->synth,
                               fluid_player_callback, player);

        if(player->sample_timer == NULL)
        {
            goto err;
        }
    }

    fluid_settings_getint(synth->settings, "player.reset-synth", &i);
    fluid_player_handle_reset_synth(player, NULL, i);

    fluid_settings_callback_int(synth->settings, "player.reset-synth",
                                fluid_player_handle_reset_synth, player);

    return player;

err:
    delete_fluid_player(player);
    return NULL;
}

/**
 * Delete a MIDI player instance.
 * @param player MIDI player instance
 * @warning Do not call when the synthesizer associated to this \p player renders audio,
 * i.e. an audio driver is running or any other synthesizer thread concurrently calls
 * fluid_synth_process() or fluid_synth_nwrite_float() or fluid_synth_write_*() !
 */
void
delete_fluid_player(fluid_player_t *player)
{
    fluid_list_t *q;
    fluid_playlist_item *pi;

    fluid_return_if_fail(player != NULL);

    fluid_settings_callback_int(player->synth->settings, "player.reset-synth",
                                NULL, NULL);

    fluid_player_stop(player);
    fluid_player_reset(player);

    delete_fluid_timer(player->system_timer);
    delete_fluid_sample_timer(player->synth, player->sample_timer);

    while(player->playlist != NULL)
    {
        q = player->playlist->next;
        pi = (fluid_playlist_item *) player->playlist->data;
        FLUID_FREE(pi->filename);
        FLUID_FREE(pi->buffer);
        FLUID_FREE(pi);
        delete1_fluid_list(player->playlist);
        player->playlist = q;
    }

    FLUID_FREE(player);
}

/**
 * Registers settings related to the MIDI player
 */
void
fluid_player_settings(fluid_settings_t *settings)
{
    /* player.timing-source can be either "system" (use system timer)
     or "sample" (use timer based on number of written samples) */
    fluid_settings_register_str(settings, "player.timing-source", "sample", 0);
    fluid_settings_add_option(settings, "player.timing-source", "sample");
    fluid_settings_add_option(settings, "player.timing-source", "system");

    /* Selects whether the player should reset the synth between songs, or not. */
    fluid_settings_register_int(settings, "player.reset-synth", 1, 0, 1, FLUID_HINT_TOGGLED);
}


int
fluid_player_reset(fluid_player_t *player)
{
    int i;

    for(i = 0; i < MAX_NUMBER_OF_TRACKS; i++)
    {
        if(player->track[i] != NULL)
        {
            delete_fluid_track(player->track[i]);
            player->track[i] = NULL;
        }
    }

    for(i = 0; i < MAX_NUMBER_OF_CHANNELS; i++)
    {
        player->channel_isplaying[i] = FALSE;
    }

    /*    player->current_file = NULL; */
    /*    player->status = FLUID_PLAYER_READY; */
    /*    player->loop = 1; */
    player->ntracks = 0;
    player->division = 0;
    player->miditempo = 500000;
    player->deltatime = 4.0;
    return 0;
}

/*
 * fluid_player_add_track
 */
int
fluid_player_add_track(fluid_player_t *player, fluid_track_t *track)
{
    if(player->ntracks < MAX_NUMBER_OF_TRACKS)
    {
        player->track[player->ntracks++] = track;
        return FLUID_OK;
    }
    else
    {
        return FLUID_FAILED;
    }
}

/**
 * Change the MIDI callback function.
 *
 * @param player MIDI player instance
 * @param handler Pointer to callback function
 * @param handler_data Parameter sent to the callback function
 * @returns FLUID_OK
 *
 * This is usually set to fluid_synth_handle_midi_event(), but can optionally
 * be changed to a user-defined function instead, for intercepting all MIDI
 * messages sent to the synth. You can also use a midi router as the callback
 * function to modify the MIDI messages before sending them to the synth.
 *
 * @since 1.1.4
 */
int
fluid_player_set_playback_callback(fluid_player_t *player,
                                   handle_midi_event_func_t handler, void *handler_data)
{
    player->playback_callback = handler;
    player->playback_userdata = handler_data;
    return FLUID_OK;
}

/**
 * Add a listener function for every MIDI tick change.
 *
 * @param player MIDI player instance
 * @param handler Pointer to callback function
 * @param handler_data Opaque parameter to be sent to the callback function
 * @returns #FLUID_OK
 *
 * This callback is not set by default, but can optionally
 * be changed to a user-defined function for intercepting all MIDI
 * tick changes and react to them with precision.
 *
 * @since 2.2.0
 */
int
fluid_player_set_tick_callback(fluid_player_t *player, handle_midi_tick_func_t handler, void *handler_data)
{
    player->tick_callback = handler;
    player->tick_userdata = handler_data;
    return FLUID_OK;
}

/**
 * Add a MIDI file to a player queue.
 * @param player MIDI player instance
 * @param midifile File name of the MIDI file to add
 * @return #FLUID_OK or #FLUID_FAILED
 */
int
fluid_player_add(fluid_player_t *player, const char *midifile)
{
    fluid_playlist_item *pi = FLUID_MALLOC(sizeof(fluid_playlist_item));
    char *f = FLUID_STRDUP(midifile);

    if(!pi || !f)
    {
        FLUID_FREE(pi);
        FLUID_FREE(f);
        FLUID_LOG(FLUID_PANIC, "Out of memory");
        return FLUID_FAILED;
    }

    pi->filename = f;
    pi->buffer = NULL;
    pi->buffer_len = 0;
    player->playlist = fluid_list_append(player->playlist, pi);
    return FLUID_OK;
}

/**
 * Add a MIDI file to a player queue, from a buffer in memory.
 * @param player MIDI player instance
 * @param buffer Pointer to memory containing the bytes of a complete MIDI
 *   file. The data is copied, so the caller may free or modify it immediately
 *   without affecting the playlist.
 * @param len Length of the buffer, in bytes.
 * @return #FLUID_OK or #FLUID_FAILED
 */
int
fluid_player_add_mem(fluid_player_t *player, const void *buffer, size_t len)
{
    /* Take a copy of the buffer, so the caller can free immediately. */
    fluid_playlist_item *pi = FLUID_MALLOC(sizeof(fluid_playlist_item));
    void *buf_copy = FLUID_MALLOC(len);

    if(!pi || !buf_copy)
    {
        FLUID_FREE(pi);
        FLUID_FREE(buf_copy);
        FLUID_LOG(FLUID_PANIC, "Out of memory");
        return FLUID_FAILED;
    }

    FLUID_MEMCPY(buf_copy, buffer, len);
    pi->filename = NULL;
    pi->buffer = buf_copy;
    pi->buffer_len = len;
    player->playlist = fluid_list_append(player->playlist, pi);
    return FLUID_OK;
}

/*
 * fluid_player_load
 */
int
fluid_player_load(fluid_player_t *player, fluid_playlist_item *item)
{
    fluid_midi_file *midifile;
    char *buffer;
    size_t buffer_length;
    int buffer_owned;

    if(item->filename != NULL)
    {
        fluid_file fp;
        /* This file is specified by filename; load the file from disk */
        FLUID_LOG(FLUID_DBG, "%s: %d: Loading midifile %s", __FILE__, __LINE__,
                  item->filename);
        /* Read the entire contents of the file into the buffer */
        fp = FLUID_FOPEN(item->filename, "rb");

        if(fp == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Couldn't open the MIDI file");
            return FLUID_FAILED;
        }

        buffer = fluid_file_read_full(fp, &buffer_length);

        FLUID_FCLOSE(fp);

        if(buffer == NULL)
        {
            return FLUID_FAILED;
        }

        buffer_owned = 1;
    }
    else
    {
        /* This file is specified by a pre-loaded buffer; load from memory */
        FLUID_LOG(FLUID_DBG, "%s: %d: Loading midifile from memory (%p)",
                  __FILE__, __LINE__, item->buffer);
        buffer = (char *) item->buffer;
        buffer_length = item->buffer_len;
        /* Do not free the buffer (it is owned by the playlist) */
        buffer_owned = 0;
    }

    midifile = new_fluid_midi_file(buffer, buffer_length);

    if(midifile == NULL)
    {
        if(buffer_owned)
        {
            FLUID_FREE(buffer);
        }

        return FLUID_FAILED;
    }

    player->division = fluid_midi_file_get_division(midifile);
    fluid_player_update_tempo(player);  // Update deltatime
    /*FLUID_LOG(FLUID_DBG, "quarter note division=%d\n", player->division); */

    if(fluid_midi_file_load_tracks(midifile, player) != FLUID_OK)
    {
        if(buffer_owned)
        {
            FLUID_FREE(buffer);
        }

        delete_fluid_midi_file(midifile);
        return FLUID_FAILED;
    }

    delete_fluid_midi_file(midifile);

    if(buffer_owned)
    {
        FLUID_FREE(buffer);
    }

    return FLUID_OK;
}

void
fluid_player_advancefile(fluid_player_t *player)
{
    if(player->playlist == NULL)
    {
        return; /* No files to play */
    }

    if(player->currentfile != NULL)
    {
        player->currentfile = fluid_list_next(player->currentfile);
    }

    if(player->currentfile == NULL)
    {
        if(player->loop == 0)
        {
            return; /* We're done playing */
        }

        if(player->loop > 0)
        {
            player->loop--;
        }

        player->currentfile = player->playlist;
    }
}

void
fluid_player_playlist_load(fluid_player_t *player, unsigned int msec)
{
    fluid_playlist_item *current_playitem;
    int i;

    do
    {
        fluid_player_advancefile(player);

        if(player->currentfile == NULL)
        {
            /* Failed to find next song, probably since we're finished */
            fluid_atomic_int_set(&player->status, FLUID_PLAYER_DONE);
            return;
        }

        fluid_player_reset(player);
        current_playitem = (fluid_playlist_item *) player->currentfile->data;
    }
    while(fluid_player_load(player, current_playitem) != FLUID_OK);

    /* Successfully loaded midi file */

    player->begin_msec = msec;
    player->start_msec = msec;
    player->start_ticks = 0;
    player->cur_ticks = 0;

    for(i = 0; i < player->ntracks; i++)
    {
        if(player->track[i] != NULL)
        {
            fluid_track_reset(player->track[i]);
        }
    }
}

/*
 * fluid_player_callback
 */
int
fluid_player_callback(void *data, unsigned int msec)
{
    int i;
    int loadnextfile;
    int status = FLUID_PLAYER_DONE;
    fluid_midi_event_t mute_event;
    fluid_player_t *player;
    fluid_synth_t *synth;
    player = (fluid_player_t *) data;
    synth = player->synth;

    loadnextfile = player->currentfile == NULL ? 1 : 0;
    
    fluid_midi_event_set_type(&mute_event, CONTROL_CHANGE);
    mute_event.param1 = ALL_SOUND_OFF;
    mute_event.param2 = 1;

    if(fluid_player_get_status(player) != FLUID_PLAYER_PLAYING)
    {
        if(fluid_atomic_int_get(&player->stopping))
        {
            for(i = 0; i < synth->midi_channels; i++)
            {
                if(player->channel_isplaying[i])
                {
                    fluid_midi_event_set_channel(&mute_event, i);
                    player->playback_callback(player->playback_userdata, &mute_event);
                    player->channel_isplaying[i] = FALSE;
                }
            }
            fluid_atomic_int_set(&player->stopping, 0);
        }
        return 1;
    }
    do
    {
        float deltatime;
        int seek_ticks;

        if(loadnextfile)
        {
            loadnextfile = 0;
            fluid_player_playlist_load(player, msec);

            if(player->currentfile == NULL)
            {
                return 0;
            }
        }

        if(msec < player->cur_msec)
        {
            // overflow of fluid_synth_get_ticks()
            FLUID_LOG(FLUID_ERR, "The maximum playback duration has been reached. Terminating player!");
            fluid_player_stop(player);
            fluid_player_seek(player, 0);
            player->cur_ticks = 0;
            return 0;
        }

        player->cur_msec = msec;
        deltatime = fluid_atomic_float_get(&player->deltatime);
        player->cur_ticks = (player->start_ticks
                             + (int)((double)(player->cur_msec - player->start_msec)
                                     / deltatime + 0.5)); /* 0.5 to average overall error when casting */

        seek_ticks = fluid_atomic_int_get(&player->seek_ticks);
        if(seek_ticks >= 0)
        {
            for(i = 0; i < synth->midi_channels; i++)
            {
                if(player->channel_isplaying[i])
                {
                    fluid_midi_event_set_channel(&mute_event, i);
                    player->playback_callback(player->playback_userdata, &mute_event);
                    player->channel_isplaying[i] = FALSE;
                }
            }
        }

        for(i = 0; i < player->ntracks; i++)
        {
            fluid_track_send_events(player->track[i], synth, player, player->cur_ticks, seek_ticks);
            if(!fluid_track_eot(player->track[i]))
            {
                status = FLUID_PLAYER_PLAYING;
            }
        }

        if(seek_ticks >= 0)
        {
            player->start_ticks = seek_ticks;   /* tick position of last tempo value (which is now) */
            player->cur_ticks = seek_ticks;
            player->begin_msec = msec;      /* only used to calculate the duration of playing */
            player->start_msec = msec;      /* should be the (synth)-time of the last tempo change */
            fluid_atomic_int_set(&player->seek_ticks, -1); /* clear seek_ticks */
        }
        
        if(fluid_list_next(player->currentfile) == NULL && player->loop == 0)
        {
            /* Once we've run out of MIDI events, keep playing until no voices are active */
            if(status == FLUID_PLAYER_DONE && fluid_synth_get_active_voice_count(player->synth) > 0)
            {
                /* The first time we notice we've run out of MIDI events but there are still active voices, disable all hold pedals */
                if(!player->end_pedals_disabled)
                {
                    for(i = 0; i < synth->midi_channels; i++)
                    {
                        fluid_synth_cc(player->synth, i, SUSTAIN_SWITCH, 0);
                        fluid_synth_cc(player->synth, i, SOSTENUTO_SWITCH, 0);
                    }

                    player->end_pedals_disabled = 1;
                }

                status = FLUID_PLAYER_PLAYING;
            }

            /* Once no voices are active, if end_msec hasn't been scheduled, schedule it so we wait for reverb, etc to finish */
            if(status == FLUID_PLAYER_DONE && player->end_msec < 0)
            {
                player->end_msec = msec + FLUID_PLAYER_STOP_GRACE_MS;
            }
            /* If end_msec has been scheduled and is in the future, keep playing */
            if (player->end_msec >= 0 && msec < (unsigned int) player->end_msec)
            {
                status = FLUID_PLAYER_PLAYING;
            }
        }

        /* Once there's no reason to keep playing, we're actually done */
        if(status == FLUID_PLAYER_DONE)
        {
            FLUID_LOG(FLUID_DBG, "%s: %d: Duration=%.3f sec", __FILE__,
                      __LINE__, (msec - player->begin_msec) / 1000.0);

            if(player->reset_synth_between_songs)
            {
                fluid_synth_system_reset(player->synth);
            }

            loadnextfile = 1;
        }

        if (player->tick_callback != NULL && player->last_callback_ticks != player->cur_ticks) {
            player->tick_callback(player->tick_userdata, player->cur_ticks);
            player->last_callback_ticks = player->cur_ticks;
        }
    }
    while(loadnextfile);

    /* do not update the status if the player has been stopped already */
    fluid_atomic_int_compare_and_exchange(&player->status, FLUID_PLAYER_PLAYING, status);

    return 1;
}

/**
 * Activates play mode for a MIDI player if not already playing.
 * @param player MIDI player instance
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 *
 * If the list of files added to the player has completed its requested number of loops,
 * the playlist will be restarted from the beginning with a loop count of 1.
 */
int
fluid_player_play(fluid_player_t *player)
{
    if(fluid_player_get_status(player) == FLUID_PLAYER_PLAYING ||
       player->playlist == NULL)
    {
        return FLUID_OK;
    }

    if(!player->use_system_timer)
    {
        fluid_sample_timer_reset(player->synth, player->sample_timer);
    }

    /* If we're at the end of the playlist and there are no loops left, loop once */
    if(player->currentfile == NULL && player->loop == 0)
    {
      player->loop = 1;
    }

    player->end_msec = -1;
    player->end_pedals_disabled = 0;

    fluid_atomic_int_set(&player->status, FLUID_PLAYER_PLAYING);

    return FLUID_OK;
}

/**
 * Pauses the MIDI playback.
 *
 * @param player MIDI player instance
 * @return Always returns #FLUID_OK
 *
 * It will not rewind to the beginning of the file, use fluid_player_seek() for this purpose.
 */
int
fluid_player_stop(fluid_player_t *player)
{
    fluid_atomic_int_set(&player->status, FLUID_PLAYER_DONE);
    fluid_atomic_int_set(&player->stopping, 1);
    fluid_player_seek(player, fluid_player_get_current_tick(player));
    return FLUID_OK;
}

/**
 * Get MIDI player status.
 * @param player MIDI player instance
 * @return Player status (#fluid_player_status)
 * @since 1.1.0
 */
int
fluid_player_get_status(fluid_player_t *player)
{
    return fluid_atomic_int_get(&player->status);
}

/**
 * Seek in the currently playing file.
 *
 * @param player MIDI player instance
 * @param ticks the position to seek to in the current file
 * @return #FLUID_FAILED if ticks is negative or after the latest tick of the file
 * [or, since 2.1.3, if another seek operation is currently in progress],
 * #FLUID_OK otherwise.
 *
 * The actual seek will be performed when the synth calls back the player (i.e. a few
 * levels above the player's callback set with fluid_player_set_playback_callback()).
 * If the player's status is #FLUID_PLAYER_PLAYING and a previous seek operation has
 * not been completed yet, #FLUID_FAILED is returned.
 *
 * @since 2.0.0
 */
int fluid_player_seek(fluid_player_t *player, int ticks)
{
    if(ticks < 0 || (fluid_player_get_status(player) != FLUID_PLAYER_READY && ticks > fluid_player_get_total_ticks(player)))
    {
        return FLUID_FAILED;
    }

    if(fluid_player_get_status(player) == FLUID_PLAYER_PLAYING)
    {
        if(fluid_atomic_int_compare_and_exchange(&player->seek_ticks, -1, ticks))
        {
            // new seek position has been set, as no previous seek was in progress
            return FLUID_OK;
        }
    }
    else
    {
        // If the player is not currently playing, a new seek position can be set at any time. This allows
        // the user to do:
        // fluid_player_stop();
        // fluid_player_seek(0); // to beginning
        fluid_atomic_int_set(&player->seek_ticks, ticks);
        return FLUID_OK;
    }

    // a previous seek is still in progress or hasn't been processed yet
    return FLUID_FAILED;
}


/**
 * Enable looping of a MIDI player
 *
 * @param player MIDI player instance
 * @param loop Times left to loop the playlist. -1 means loop infinitely.
 * @return Always returns #FLUID_OK
 *
 * For example, if you want to loop the playlist twice, set loop to 2
 * and call this function before you start the player.
 *
 * @since 1.1.0
 */
int fluid_player_set_loop(fluid_player_t *player, int loop)
{
    player->loop = loop;
    return FLUID_OK;
}

/**
 * update the MIDI player internal deltatime dependent of actual tempo.
 * @param player MIDI player instance
 */
static void fluid_player_update_tempo(fluid_player_t *player)
{
    int tempo; /* tempo in micro seconds by quarter note */
    float deltatime;

    /* do nothing if the division is still unknown to avoid a div by zero */
    if(player->division == 0)
    {
        return;
    }

    if(fluid_atomic_int_get(&player->sync_mode))
    {
        /* take internal tempo from MIDI file */
        tempo = fluid_atomic_int_get(&player->miditempo);
        /* compute deltattime (in ms) from current tempo and apply tempo multiplier */
        deltatime = (float)tempo / (float)player->division / (float)1000.0;
        deltatime /= fluid_atomic_float_get(&player->multempo); /* multiply tempo */
    }
    else
    {
        /* take  external tempo */
        tempo = fluid_atomic_int_get(&player->exttempo);
        /* compute deltattime (in ms) from current tempo */
        deltatime = (float)tempo / (float)player->division / (float)1000.0;
    }

    fluid_atomic_float_set(&player->deltatime, deltatime);

    player->start_msec = player->cur_msec;
    player->start_ticks = player->cur_ticks;

    FLUID_LOG(FLUID_DBG,
              "tempo=%d, tick time=%f msec, cur time=%d msec, cur tick=%d",
              tempo, player->deltatime, player->cur_msec, player->cur_ticks);

}

/**
 * Set the tempo of a MIDI player.
 * The player can be controlled by internal tempo coming from MIDI file tempo
 * change or controlled by external tempo expressed in BPM or in micro seconds
 * per quarter note.
 *
 * @param player MIDI player instance. Must be a valid pointer.
 * @param tempo_type Must a be value of #fluid_player_set_tempo_type and indicates the
 *  meaning of tempo value and how the player will be controlled, see below.
 * @param tempo Tempo value or multiplier.
 * 
 *  #FLUID_PLAYER_TEMPO_INTERNAL, the player will be controlled by internal
 *  MIDI file tempo changes. If there are no tempo change in the MIDI file a default
 *  value of 120 bpm is used. The @c tempo parameter is used as a multiplier factor
 *  that must be in the range (0.001 to 1000).
 *  For example, if the current MIDI file tempo is 120 bpm and the multiplier
 *  value is 0.5 then this tempo will be slowed down to 60 bpm.
 *  At creation, the player is set to be controlled by internal tempo with a
 *  multiplier factor set to 1.0.
 *
 *  #FLUID_PLAYER_TEMPO_EXTERNAL_BPM, the player will be controlled by the
 *  external tempo value provided  by the tempo parameter in bpm
 *  (i.e in quarter notes per minute) which must be in the range (1 to 60000000).
 *
 *  #FLUID_PLAYER_TEMPO_EXTERNAL_MIDI, similar as FLUID_PLAYER_TEMPO_EXTERNAL_BPM,
 *  but the tempo parameter value is in  micro seconds per quarter note which
 *  must be in the range (1 to 60000000).
 *  Using micro seconds per quarter note is convenient when the tempo value is
 *  derived from MIDI clock realtime messages.
 *
 * @note When the player is controlled by an external tempo
 * (#FLUID_PLAYER_TEMPO_EXTERNAL_BPM or #FLUID_PLAYER_TEMPO_EXTERNAL_MIDI) it
 * continues to memorize the most recent internal tempo change coming from the
 * MIDI file so that next call to fluid_player_set_tempo() with
 * #FLUID_PLAYER_TEMPO_INTERNAL will set the player to follow this internal
 * tempo.
 *
 * @warning If the function is called when no MIDI file is loaded or currently playing, it
 * would have caused a division by zero in fluidsynth 2.2.7 and earlier. Starting with 2.2.8, the
 * new tempo change will be stashed and applied later.
 *
 * @return #FLUID_OK if success or #FLUID_FAILED otherwise (incorrect parameters).
 * @since 2.2.0
 */
int fluid_player_set_tempo(fluid_player_t *player, int tempo_type, double tempo)
{
    fluid_return_val_if_fail(player != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(tempo_type >= FLUID_PLAYER_TEMPO_INTERNAL, FLUID_FAILED);
    fluid_return_val_if_fail(tempo_type < FLUID_PLAYER_TEMPO_NBR, FLUID_FAILED);

    switch(tempo_type)
    {
        /* set the player to be driven by internal tempo coming from MIDI file */
        case FLUID_PLAYER_TEMPO_INTERNAL:
            /* check if the multiplier is in correct range */
            fluid_return_val_if_fail(tempo >= MIN_TEMPO_MULTIPLIER, FLUID_FAILED);
            fluid_return_val_if_fail(tempo <= MAX_TEMPO_MULTIPLIER, FLUID_FAILED);

            /* set the tempo multiplier */
            fluid_atomic_float_set(&player->multempo, (float)tempo);
            fluid_atomic_int_set(&player->sync_mode, 1); /* set internal mode */
            break;

        /* set the player to be driven by external tempo */
        case FLUID_PLAYER_TEMPO_EXTERNAL_BPM:  /* value in bpm */
        case FLUID_PLAYER_TEMPO_EXTERNAL_MIDI: /* value in us/quarter note */
            /* check if tempo is in correct range */
            fluid_return_val_if_fail(tempo >= MIN_TEMPO_VALUE, FLUID_FAILED);
            fluid_return_val_if_fail(tempo <= MAX_TEMPO_VALUE, FLUID_FAILED);

            /* set the tempo value */
            if(tempo_type == FLUID_PLAYER_TEMPO_EXTERNAL_BPM)
            {
                tempo = 60000000L / tempo; /* convert tempo in us/quarter note */
            }
            fluid_atomic_int_set(&player->exttempo, (int)tempo);
            fluid_atomic_int_set(&player->sync_mode, 0); /* set external mode */
            break;

        default: /* shouldn't happen */
            break;
    }

    /* update deltatime depending of current tempo */
    fluid_player_update_tempo(player);

    return FLUID_OK;
}

/**
 * Set the tempo of a MIDI player.
 * @param player MIDI player instance
 * @param tempo Tempo to set playback speed to (in microseconds per quarter note, as per MIDI file spec)
 * @return Always returns #FLUID_OK
 * @note Tempo change events contained in the MIDI file can override the specified tempo at any time!
 * @deprecated Use fluid_player_set_tempo() instead.
 */
int fluid_player_set_midi_tempo(fluid_player_t *player, int tempo)
{
    player->miditempo = tempo;

    fluid_player_update_tempo(player);
    return FLUID_OK;
}

/**
 * Set the tempo of a MIDI player in beats per minute.
 * @param player MIDI player instance
 * @param bpm Tempo in beats per minute
 * @return Always returns #FLUID_OK
 * @note Tempo change events contained in the MIDI file can override the specified BPM at any time!
 * @deprecated Use fluid_player_set_tempo() instead.
 */
int fluid_player_set_bpm(fluid_player_t *player, int bpm)
{
    if(bpm <= 0)
    {
        return FLUID_FAILED; /* to avoid a division by 0 */
    }

    return fluid_player_set_midi_tempo(player, 60000000L / bpm);
}

/**
 * Wait for a MIDI player until the playback has been stopped.
 * @param player MIDI player instance
 * @return Always #FLUID_OK
 *
 * @warning The player may still be used by a concurrently running synth context. Hence it is
 * not safe to immediately delete the player afterwards. Also refer to delete_fluid_player().
 */
int
fluid_player_join(fluid_player_t *player)
{
    while(fluid_player_get_status(player) != FLUID_PLAYER_DONE)
    {
        fluid_msleep(10);
    }
    return FLUID_OK;
}

/**
 * Get the number of tempo ticks passed.
 * @param player MIDI player instance
 * @return The number of tempo ticks passed
 * @since 1.1.7
 */
int fluid_player_get_current_tick(fluid_player_t *player)
{
    return player->cur_ticks;
}

/**
 * Looks through all available MIDI tracks and gets the absolute tick of the very last event to play.
 * @param player MIDI player instance
 * @return Total tick count of the sequence
 * @since 1.1.7
 */
int fluid_player_get_total_ticks(fluid_player_t *player)
{
    int i;
    int maxTicks = 0;

    for(i = 0; i < player->ntracks; i++)
    {
        if(player->track[i] != NULL)
        {
            int ticks = fluid_track_get_duration(player->track[i]);

            if(ticks > maxTicks)
            {
                maxTicks = ticks;
            }
        }
    }

    return maxTicks;
}

/**
 * Get the tempo currently used by a MIDI player.
 * The player can be controlled by internal tempo coming from MIDI file tempo
 * change or controlled by external tempo see fluid_player_set_tempo().
 * @param player MIDI player instance. Must be a valid pointer.
 * @return MIDI player tempo in BPM or FLUID_FAILED if error.
 * @since 1.1.7
 */
int fluid_player_get_bpm(fluid_player_t *player)
{

    int midi_tempo = fluid_player_get_midi_tempo(player);

    if(midi_tempo > 0)
    {
        midi_tempo = 60000000L / midi_tempo; /* convert in bpm */
    }

    return midi_tempo;
}

/**
 * Get the division currently used by a MIDI player.
 * The player can be controlled by internal tempo coming from MIDI file tempo
 * change or controlled by external tempo see fluid_player_set_tempo().
 * @param player MIDI player instance. Must be a valid pointer.
 * @return MIDI player division or FLUID_FAILED if error.
 * @since 2.3.2
 */
int fluid_player_get_division(fluid_player_t *player)
{
    return player->division;
}

/**
 * Get the tempo currently used by a MIDI player.
 * The player can be controlled by internal tempo coming from MIDI file tempo
 * change or controlled by external tempo see fluid_player_set_tempo().

 * @param player MIDI player instance. Must be a valid pointer.
 * @return Tempo of the MIDI player (in microseconds per quarter note, as per
 * MIDI file spec) or FLUID_FAILED if error.
 * @since 1.1.7
 */
int fluid_player_get_midi_tempo(fluid_player_t *player)
{
    int midi_tempo; /* value to return */

    fluid_return_val_if_fail(player != NULL, FLUID_FAILED);

    midi_tempo = fluid_atomic_int_get(&player->exttempo);
    /* look if the player is internally synced */
    if(fluid_atomic_int_get(&player->sync_mode))
    {
        midi_tempo = (int)((float)fluid_atomic_int_get(&player->miditempo)/
                           fluid_atomic_float_get(&player->multempo));
    }

    return midi_tempo;
}

/************************************************************************
 *       MIDI PARSER
 *
 */

/*
 * new_fluid_midi_parser
 */
fluid_midi_parser_t *
new_fluid_midi_parser()
{
    fluid_midi_parser_t *parser;
    parser = FLUID_NEW(fluid_midi_parser_t);

    if(parser == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    parser->status = 0; /* As long as the status is 0, the parser won't do anything -> no need to initialize all the fields. */
    return parser;
}

/*
 * delete_fluid_midi_parser
 */
void
delete_fluid_midi_parser(fluid_midi_parser_t *parser)
{
    fluid_return_if_fail(parser != NULL);

    FLUID_FREE(parser);
}

/**
 * Parse a MIDI stream one character at a time.
 * @param parser Parser instance
 * @param c Next character in MIDI stream
 * @return A parsed MIDI event or NULL if none.  Event is internal and should
 *   not be modified or freed and is only valid until next call to this function.
 * @internal Do not expose this function to the public API. It would allow downstream
 * apps to abuse fluidsynth as midi parser, e.g. feeding it with rawmidi and pull out
 * the needed midi information using the getter functions of fluid_midi_event_t.
 * This parser however is incomplete as it e.g. only provides a limited buffer to
 * store and process SYSEX data (i.e. doesn't allow arbitrary lengths)
 */
fluid_midi_event_t *
fluid_midi_parser_parse(fluid_midi_parser_t *parser, unsigned char c)
{
    fluid_midi_event_t *event;

    /* Real-time messages (0xF8-0xFF) can occur anywhere, even in the middle
     * of another message. */
    if(c >= 0xF8)
    {
        parser->event.type = c;
        parser->status = 0; /* clear the status */
        return &parser->event;
    }

    /* Status byte? - If previous message not yet complete, it is discarded (re-sync). */
    if(c & 0x80)
    {
        /* Any status byte terminates SYSEX messages (not just 0xF7) */
        if(parser->status == MIDI_SYSEX && parser->nr_bytes > 0)
        {
            event = &parser->event;
            fluid_midi_event_set_sysex(event, parser->data, parser->nr_bytes,
                                       FALSE);
        }
        else
        {
            event = NULL;
        }

        if(c < 0xF0)  /* Voice category message? */
        {
            parser->channel = c & 0x0F;
            parser->status = c & 0xF0;

            /* The event consumes x bytes of data... (subtract 1 for the status byte) */
            parser->nr_bytes_total = fluid_midi_event_length(parser->status)
                                     - 1;

            parser->nr_bytes = 0; /* 0  bytes read so far */
        }
        else if(c == MIDI_SYSEX)
        {
            parser->status = MIDI_SYSEX;
            parser->nr_bytes = 0;
        }
        else
        {
            parser->status = 0;    /* Discard other system messages (0xF1-0xF7) */
        }

        return event; /* Return SYSEX event or NULL */
    }

    /* Data/parameter byte */

    /* Discard data bytes for events we don't care about */
    if(parser->status == 0)
    {
        return NULL;
    }

    /* Max data size exceeded? (SYSEX messages only really) */
    if(parser->nr_bytes == FLUID_MIDI_PARSER_MAX_DATA_SIZE)
    {
        parser->status = 0; /* Discard the rest of the message */
        return NULL;
    }

    /* Store next byte */
    parser->data[parser->nr_bytes++] = c;

    /* Do we still need more data to get this event complete? */
    if(parser->status == MIDI_SYSEX || parser->nr_bytes < parser->nr_bytes_total)
    {
        return NULL;
    }

    /* Event is complete, return it.
     * Running status byte MIDI feature is also handled here. */
    parser->event.type = parser->status;
    parser->event.channel = parser->channel;
    parser->nr_bytes = 0; /* Reset data size, in case there are additional running status messages */

    switch(parser->status)
    {
    case NOTE_OFF:
    case NOTE_ON:
    case KEY_PRESSURE:
    case CONTROL_CHANGE:
    case PROGRAM_CHANGE:
    case CHANNEL_PRESSURE:
        parser->event.param1 = parser->data[0]; /* For example key number */
        parser->event.param2 = parser->data[1]; /* For example velocity */
        break;

    case PITCH_BEND:
        /* Pitch-bend is transmitted with 14-bit precision. */
        parser->event.param1 = (parser->data[1] << 7) | parser->data[0];
        break;

    default: /* Unlikely */
        return NULL;
    }

    return &parser->event;
}

/* Purpose:
 * Returns the length of a MIDI message. */
static int
fluid_midi_event_length(unsigned char event)
{
    switch(event & 0xF0)
    {
    case NOTE_OFF:
    case NOTE_ON:
    case KEY_PRESSURE:
    case CONTROL_CHANGE:
    case PITCH_BEND:
        return 3;

    case PROGRAM_CHANGE:
    case CHANNEL_PRESSURE:
        return 2;
    }

    switch(event)
    {
    case MIDI_TIME_CODE:
    case MIDI_SONG_SELECT:
    case 0xF4:
    case 0xF5:
        return 2;

    case MIDI_TUNE_REQUEST:
        return 1;

    case MIDI_SONG_POSITION:
        return 3;
    }

    return 1;
}
