/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */

#ifndef _FLUID_MIDI_H
#define _FLUID_MIDI_H

#include "fluidsynth_priv.h"
#include "fluid_sys.h"
#include "fluid_list.h"

typedef struct _fluid_midi_parser_t fluid_midi_parser_t;

fluid_midi_parser_t* new_fluid_midi_parser(void);
int delete_fluid_midi_parser(fluid_midi_parser_t* parser);
fluid_midi_event_t* fluid_midi_parser_parse(fluid_midi_parser_t* parser, unsigned char c);

int fluid_midi_send_event(fluid_synth_t* synth, fluid_player_t* player, fluid_midi_event_t* evt);


/***************************************************************
 *
 *                   CONSTANTS & ENUM
 */


#define MAX_NUMBER_OF_TRACKS 128

enum fluid_midi_event_type {
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

enum fluid_midi_control_change {
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
  LEGATO_SWITCH = 0x45,
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
enum midi_rpn_event {
  RPN_PITCH_BEND_RANGE = 0x00,
  RPN_CHANNEL_FINE_TUNE = 0x01,
  RPN_CHANNEL_COARSE_TUNE = 0x02,
  RPN_TUNING_PROGRAM_CHANGE = 0x03,
  RPN_TUNING_BANK_SELECT = 0x04,
  RPN_MODULATION_DEPTH_RANGE = 0x05
};

enum midi_meta_event {
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
enum midi_sysex_manuf {
  MIDI_SYSEX_MANUF_ROLAND       = 0x41,         /**< Roland manufacturer ID */
  MIDI_SYSEX_UNIV_NON_REALTIME  = 0x7E,         /**< Universal non realtime message */
  MIDI_SYSEX_UNIV_REALTIME      = 0x7F          /**< Universal realtime message */
};

#define MIDI_SYSEX_DEVICE_ID_ALL        0x7F    /**< Device ID used in SYSEX messages to indicate all devices */

/* SYSEX sub-ID #1 which follows device ID */
#define MIDI_SYSEX_MIDI_TUNING_ID       0x08    /**< Sysex sub-ID #1 for MIDI tuning messages */
#define MIDI_SYSEX_GM_ID                0x09    /**< Sysex sub-ID #1 for General MIDI messages */

/**
 * SYSEX tuning message IDs.
 */
enum midi_sysex_tuning_msg_id {
  MIDI_SYSEX_TUNING_BULK_DUMP_REQ       = 0x00, /**< Bulk tuning dump request (non-realtime) */
  MIDI_SYSEX_TUNING_BULK_DUMP           = 0x01, /**< Bulk tuning dump response (non-realtime) */
  MIDI_SYSEX_TUNING_NOTE_TUNE           = 0x02, /**< Tuning note change message (realtime) */
  MIDI_SYSEX_TUNING_BULK_DUMP_REQ_BANK  = 0x03, /**< Bulk tuning dump request (with bank, non-realtime) */
  MIDI_SYSEX_TUNING_BULK_DUMP_BANK      = 0x04, /**< Bulk tuning dump resonse (with bank, non-realtime) */
  MIDI_SYSEX_TUNING_OCTAVE_DUMP_1BYTE   = 0x05, /**< Octave tuning dump using 1 byte values (non-realtime) */
  MIDI_SYSEX_TUNING_OCTAVE_DUMP_2BYTE   = 0x06, /**< Octave tuning dump using 2 byte values (non-realtime) */
  MIDI_SYSEX_TUNING_NOTE_TUNE_BANK      = 0x07, /**< Tuning note change message (with bank, realtime/non-realtime) */
  MIDI_SYSEX_TUNING_OCTAVE_TUNE_1BYTE   = 0x08, /**< Octave tuning message using 1 byte values (realtime/non-realtime) */
  MIDI_SYSEX_TUNING_OCTAVE_TUNE_2BYTE   = 0x09  /**< Octave tuning message using 2 byte values (realtime/non-realtime) */
};

/* General MIDI sub-ID #2 */
#define MIDI_SYSEX_GM_ON                0x01    /**< Enable GM mode */
#define MIDI_SYSEX_GM_OFF               0x02    /**< Disable GM mode */


enum fluid_player_status
{
  FLUID_PLAYER_READY,
  FLUID_PLAYER_PLAYING,
  FLUID_PLAYER_DONE
};

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

/* From ctype.h */
#define fluid_isascii(c)    (((c) & ~0x7f) == 0)



/*
 * fluid_midi_event_t
 */
struct _fluid_midi_event_t {
  fluid_midi_event_t* next;  /* Don't use it, it will dissappear. Used in midi tracks.  */
  unsigned int dtime;       /* Delay (ticks) between this and previous event. midi tracks. */
  unsigned char type;       /* MIDI event type */
  unsigned char channel;    /* MIDI channel */
  unsigned int param1;      /* First parameter */
  unsigned int param2;      /* Second parameter */
};




#endif /* _FLUID_MIDI_H */
