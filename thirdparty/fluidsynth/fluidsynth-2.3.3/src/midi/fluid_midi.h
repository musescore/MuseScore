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

#ifndef _FLUID_MIDI_H
#define _FLUID_MIDI_H

#include "fluidsynth_priv.h"
#include "fluid_sys.h"
#include "fluid_list.h"

typedef struct _fluid_midi_parser_t fluid_midi_parser_t;

fluid_midi_parser_t *new_fluid_midi_parser(void);
void delete_fluid_midi_parser(fluid_midi_parser_t *parser);
fluid_midi_event_t *fluid_midi_parser_parse(fluid_midi_parser_t *parser, unsigned char c);


/***************************************************************
 *
 *                   CONSTANTS & ENUM
 */


#define MAX_NUMBER_OF_TRACKS 128
#define MAX_NUMBER_OF_CHANNELS 16

enum fluid_midi_event_type
{
    /* channel messages */
    NOTE_OFF = 0x80,
    NOTE_ON = 0x90,
    KEY_PRESSURE = 0xa0,
    CONTROL_CHANGE = 0xb0,
    PROGRAM_CHANGE = 0xc0,
    CHANNEL_PRESSURE = 0xd0,
    PITCH_BEND = 0xe0,
    /* system exclusive */
    MIDI_SYSEX = 0xf0,
    /* system common - never in midi files */
    MIDI_TIME_CODE = 0xf1,
    MIDI_SONG_POSITION = 0xf2,
    MIDI_SONG_SELECT = 0xf3,
    MIDI_TUNE_REQUEST = 0xf6,
    MIDI_EOX = 0xf7,
    /* system real-time - never in midi files */
    MIDI_SYNC = 0xf8,
    MIDI_TICK = 0xf9,
    MIDI_START = 0xfa,
    MIDI_CONTINUE = 0xfb,
    MIDI_STOP = 0xfc,
    MIDI_ACTIVE_SENSING = 0xfe,
    MIDI_SYSTEM_RESET = 0xff,
    /* meta event - for midi files only */
    MIDI_META_EVENT = 0xff
};

enum fluid_midi_control_change
{
    BANK_SELECT_MSB = 0x00,
    MODULATION_MSB = 0x01,
    BREATH_MSB = 0x02,
    FOOT_MSB = 0x04,
    PORTAMENTO_TIME_MSB = 0x05,
    DATA_ENTRY_MSB = 0x06,
    VOLUME_MSB = 0x07,
    BALANCE_MSB = 0x08,
    PAN_MSB = 0x0A,
    EXPRESSION_MSB = 0x0B,
    EFFECTS1_MSB = 0x0C,
    EFFECTS2_MSB = 0x0D,
    GPC1_MSB = 0x10, /* general purpose controller */
    GPC2_MSB = 0x11,
    GPC3_MSB = 0x12,
    GPC4_MSB = 0x13,
    BANK_SELECT_LSB = 0x20,
    MODULATION_WHEEL_LSB = 0x21,
    BREATH_LSB = 0x22,
    FOOT_LSB = 0x24,
    PORTAMENTO_TIME_LSB = 0x25,
    DATA_ENTRY_LSB = 0x26,
    VOLUME_LSB = 0x27,
    BALANCE_LSB = 0x28,
    PAN_LSB = 0x2A,
    EXPRESSION_LSB = 0x2B,
    EFFECTS1_LSB = 0x2C,
    EFFECTS2_LSB = 0x2D,
    GPC1_LSB = 0x30,
    GPC2_LSB = 0x31,
    GPC3_LSB = 0x32,
    GPC4_LSB = 0x33,
    SUSTAIN_SWITCH = 0x40,
    PORTAMENTO_SWITCH = 0x41,
    SOSTENUTO_SWITCH = 0x42,
    SOFT_PEDAL_SWITCH = 0x43,
    LEGATO_SWITCH = 0x44,
    HOLD2_SWITCH = 0x45,
    SOUND_CTRL1 = 0x46,
    SOUND_CTRL2 = 0x47,
    SOUND_CTRL3 = 0x48,
    SOUND_CTRL4 = 0x49,
    SOUND_CTRL5 = 0x4A,
    SOUND_CTRL6 = 0x4B,
    SOUND_CTRL7 = 0x4C,
    SOUND_CTRL8 = 0x4D,
    SOUND_CTRL9 = 0x4E,
    SOUND_CTRL10 = 0x4F,
    GPC5 = 0x50,
    GPC6 = 0x51,
    GPC7 = 0x52,
    GPC8 = 0x53,
    PORTAMENTO_CTRL = 0x54,
    EFFECTS_DEPTH1 = 0x5B,
    EFFECTS_DEPTH2 = 0x5C,
    EFFECTS_DEPTH3 = 0x5D,
    EFFECTS_DEPTH4 = 0x5E,
    EFFECTS_DEPTH5 = 0x5F,
    DATA_ENTRY_INCR = 0x60,
    DATA_ENTRY_DECR = 0x61,
    NRPN_LSB = 0x62,
    NRPN_MSB = 0x63,
    RPN_LSB = 0x64,
    RPN_MSB = 0x65,
    ALL_SOUND_OFF = 0x78,
    ALL_CTRL_OFF = 0x79,
    LOCAL_CONTROL = 0x7A,
    ALL_NOTES_OFF = 0x7B,
    OMNI_OFF = 0x7C,
    OMNI_ON = 0x7D,
    POLY_OFF = 0x7E,
    POLY_ON = 0x7F
};

/* General MIDI RPN event numbers (LSB, MSB = 0) */
enum midi_rpn_event
{
    RPN_PITCH_BEND_RANGE = 0x00,
    RPN_CHANNEL_FINE_TUNE = 0x01,
    RPN_CHANNEL_COARSE_TUNE = 0x02,
    RPN_TUNING_PROGRAM_CHANGE = 0x03,
    RPN_TUNING_BANK_SELECT = 0x04,
    RPN_MODULATION_DEPTH_RANGE = 0x05
};

enum midi_meta_event
{
    MIDI_TEXT = 0x01,
    MIDI_COPYRIGHT = 0x02,
    MIDI_TRACK_NAME = 0x03,
    MIDI_INST_NAME = 0x04,
    MIDI_LYRIC = 0x05,
    MIDI_MARKER = 0x06,
    MIDI_CUE_POINT = 0x07,
    MIDI_EOT = 0x2f,
    MIDI_SET_TEMPO = 0x51,
    MIDI_SMPTE_OFFSET = 0x54,
    MIDI_TIME_SIGNATURE = 0x58,
    MIDI_KEY_SIGNATURE = 0x59,
    MIDI_SEQUENCER_EVENT = 0x7f
};

/* MIDI SYSEX useful manufacturer values */
enum midi_sysex_manuf
{
    MIDI_SYSEX_MANUF_ROLAND       = 0x41,         /**< Roland manufacturer ID */
    MIDI_SYSEX_MANUF_YAMAHA       = 0x43,
    MIDI_SYSEX_UNIV_NON_REALTIME  = 0x7E,         /**< Universal non realtime message */
    MIDI_SYSEX_UNIV_REALTIME      = 0x7F          /**< Universal realtime message */
};

#define MIDI_SYSEX_DEVICE_ID_ALL        0x7F    /**< Device ID used in SYSEX messages to indicate all devices */

/* SYSEX sub-ID #1 which follows device ID */
#define MIDI_SYSEX_MIDI_TUNING_ID       0x08    /**< Sysex sub-ID #1 for MIDI tuning messages */
#define MIDI_SYSEX_GM_ID                0x09    /**< Sysex sub-ID #1 for General MIDI messages */
#define MIDI_SYSEX_GS_ID                0x42    /**< Model ID (GS) serving as sub-ID #1 for GS messages*/
#define MIDI_SYSEX_XG_ID                0x4C    /**< Model ID (XG) serving as sub-ID #1 for XG messages*/

/**
 * SYSEX tuning message IDs.
 */
enum midi_sysex_tuning_msg_id
{
    MIDI_SYSEX_TUNING_BULK_DUMP_REQ       = 0x00, /**< Bulk tuning dump request (non-realtime) */
    MIDI_SYSEX_TUNING_BULK_DUMP           = 0x01, /**< Bulk tuning dump response (non-realtime) */
    MIDI_SYSEX_TUNING_NOTE_TUNE           = 0x02, /**< Tuning note change message (realtime) */
    MIDI_SYSEX_TUNING_BULK_DUMP_REQ_BANK  = 0x03, /**< Bulk tuning dump request (with bank, non-realtime) */
    MIDI_SYSEX_TUNING_BULK_DUMP_BANK      = 0x04, /**< Bulk tuning dump response (with bank, non-realtime) */
    MIDI_SYSEX_TUNING_OCTAVE_DUMP_1BYTE   = 0x05, /**< Octave tuning dump using 1 byte values (non-realtime) */
    MIDI_SYSEX_TUNING_OCTAVE_DUMP_2BYTE   = 0x06, /**< Octave tuning dump using 2 byte values (non-realtime) */
    MIDI_SYSEX_TUNING_NOTE_TUNE_BANK      = 0x07, /**< Tuning note change message (with bank, realtime/non-realtime) */
    MIDI_SYSEX_TUNING_OCTAVE_TUNE_1BYTE   = 0x08, /**< Octave tuning message using 1 byte values (realtime/non-realtime) */
    MIDI_SYSEX_TUNING_OCTAVE_TUNE_2BYTE   = 0x09  /**< Octave tuning message using 2 byte values (realtime/non-realtime) */
};

/* General MIDI sub-ID #2 */
#define MIDI_SYSEX_GM_ON                0x01    /**< Enable GM mode */
#define MIDI_SYSEX_GM_OFF               0x02    /**< Disable GM mode */
#define MIDI_SYSEX_GM2_ON               0x03    /**< Enable GM2 mode */
#define MIDI_SYSEX_GS_DT1               0x12    /**< GS DT1 command */

enum fluid_driver_status
{
    FLUID_MIDI_READY,
    FLUID_MIDI_LISTENING,
    FLUID_MIDI_DONE
};

/***************************************************************
 *
 *         TYPE DEFINITIONS & FUNCTION DECLARATIONS
 */

/*
 * fluid_midi_event_t
 */
struct _fluid_midi_event_t
{
    fluid_midi_event_t *next; /* Link to next event */
    void *paramptr;           /* Pointer parameter (for SYSEX data), size is stored to param1, param2 indicates if pointer should be freed (dynamic if TRUE) */
    unsigned int dtime;       /* Delay (ticks) between this and previous event. midi tracks. */
    unsigned int param1;      /* First parameter */
    unsigned int param2;      /* Second parameter */
    unsigned char type;       /* MIDI event type */
    unsigned char channel;    /* MIDI channel */
};


/*
 * fluid_track_t
 */
struct _fluid_track_t
{
    char *name;
    int num;
    fluid_midi_event_t *first;
    fluid_midi_event_t *cur;
    fluid_midi_event_t *last;
    unsigned int ticks;
};

typedef struct _fluid_track_t fluid_track_t;

#define fluid_track_eot(track)  ((track)->cur == NULL)


/*
 * fluid_playlist_item
 * Used as the `data' elements of the fluid_player.playlist.
 * Represents either a filename or a pre-loaded memory buffer.
 * Exactly one of `filename' and `buffer' is non-NULL.
 */
typedef struct
{
    char *filename;     /** Name of file (owned); NULL if data pre-loaded */
    void *buffer;       /** The MIDI file data (owned); NULL if filename */
    size_t buffer_len;  /** Number of bytes in buffer; 0 if filename */
} fluid_playlist_item;

/* range of tempo values */
#define MIN_TEMPO_VALUE (1.0f)
#define MAX_TEMPO_VALUE (60000000.0f)
/* range of tempo multiplier values */
#define MIN_TEMPO_MULTIPLIER (0.001f)
#define MAX_TEMPO_MULTIPLIER (1000.0f)

/*
 * fluid_player
 */
struct _fluid_player_t
{
    fluid_atomic_int_t status;
    fluid_atomic_int_t stopping; /* Flag for sending all_notes_off when player is stopped */
    int ntracks;
    fluid_track_t *track[MAX_NUMBER_OF_TRACKS];
    fluid_synth_t *synth;
    fluid_timer_t *system_timer;
    fluid_sample_timer_t *sample_timer;

    int loop; /* -1 = loop infinitely, otherwise times left to loop the playlist */
    fluid_list_t *playlist; /* List of fluid_playlist_item* objects */
    fluid_list_t *currentfile; /* points to an item in files, or NULL if not playing */

    char use_system_timer;   /* if zero, use sample timers, otherwise use system clock timer */
    char reset_synth_between_songs; /* 1 if system reset should be sent to the synth between songs. */
    fluid_atomic_int_t seek_ticks; /* new position in tempo ticks (midi ticks) for seeking */
    int start_ticks;          /* the number of tempo ticks passed at the last tempo change */
    int cur_ticks;            /* the number of tempo ticks passed */
    int last_callback_ticks;  /* the last tick number that was passed to player->tick_callback */
    int begin_msec;           /* the time (msec) of the beginning of the file */
    int start_msec;           /* the start time of the last tempo change */
    unsigned int cur_msec;    /* the current time */
    int end_msec;             /* when >=0, playback is extended until this time (for, e.g., reverb) */
    char end_pedals_disabled; /* 1 once the pedals have been released after the last midi event, 0 otherwise */
    /* sync mode: indicates the tempo mode the player is driven by (see fluid_player_set_tempo()):
       1, the player is driven by internal tempo (miditempo). This is the default.
       0, the player is driven by external tempo (exttempo)
    */
    int sync_mode;
    /* miditempo: internal tempo coming from MIDI file tempo change events
      (in micro seconds per quarter note)
    */
    int miditempo;     /* as indicated by MIDI SetTempo: n 24th of a usec per midi-clock. bravo! */
    /* exttempo: external tempo set by fluid_player_set_tempo() (in micro seconds per quarter note) */
    int exttempo;
    /* multempo: tempo multiplier set by fluid_player_set_tempo() */
    float multempo;
    float deltatime;   /* milliseconds per midi tick. depends on current tempo mode (see sync_mode) */
    unsigned int division;

    handle_midi_event_func_t playback_callback; /* function fired on each midi event as it is played */
    void *playback_userdata; /* pointer to user-defined data passed to playback_callback function */
    handle_midi_tick_func_t tick_callback; /* function fired on each tick change */
    void *tick_userdata; /* pointer to user-defined data passed to tick_callback function */

    int channel_isplaying[MAX_NUMBER_OF_CHANNELS]; /* flags indicating channels on which notes have played */
};

#define FLUID_PLAYER_STOP_GRACE_MS 2000

void fluid_player_settings(fluid_settings_t *settings);


/*
 * fluid_midi_file
 */
typedef struct
{
    const char *buffer;           /* Entire contents of MIDI file (borrowed) */
    int buf_len;                  /* Length of buffer, in bytes */
    int buf_pos;                  /* Current read position in contents buffer */
    int eof;                      /* The "end of file" condition */
    int running_status;
    int c;
    int type;
    int ntracks;
    int uses_smpte;
    unsigned int smpte_fps;
    unsigned int smpte_res;
    unsigned int division;       /* If uses_SMPTE == 0 then division is
				  ticks per beat (quarter-note) */
    double tempo;                /* Beats per second (SI rules =) */
    int tracklen;
    int trackpos;
    int eot;
    int varlen;
    int dtime;
} fluid_midi_file;



#define FLUID_MIDI_PARSER_MAX_DATA_SIZE 1024    /**< Maximum size of MIDI parameters/data (largest is SYSEX data) */

/*
 * fluid_midi_parser_t
 */
struct _fluid_midi_parser_t
{
    unsigned char status;           /* Identifies the type of event, that is currently received ('Noteon', 'Pitch Bend' etc). */
    unsigned char channel;          /* The channel of the event that is received (in case of a channel event) */
    unsigned int nr_bytes;          /* How many bytes have been read for the current event? */
    unsigned int nr_bytes_total;    /* How many bytes does the current event type include? */
    unsigned char data[FLUID_MIDI_PARSER_MAX_DATA_SIZE]; /* The parameters or SYSEX data */
    fluid_midi_event_t event;        /* The event, that is returned to the MIDI driver. */
};


#endif /* _FLUID_MIDI_H */
