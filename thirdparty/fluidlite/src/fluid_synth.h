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


#ifndef _FLUID_SYNTH_H
#define _FLUID_SYNTH_H


/***************************************************************
 *
 *                         INCLUDES
 */

#include "fluid_config.h"
#include "fluidsynth_priv.h"
#include "fluid_list.h"
#include "fluid_rev.h"
#include "fluid_voice.h"
#include "fluid_chorus.h"
#include "fluid_sys.h"

/***************************************************************
 *
 *                         DEFINES
 */
#define FLUID_NUM_PROGRAMS      128
#define DRUM_INST_BANK		128

#if defined(WITH_FLOAT)
#define FLUID_SAMPLE_FORMAT     FLUID_SAMPLE_FLOAT
#else
#define FLUID_SAMPLE_FORMAT     FLUID_SAMPLE_DOUBLE
#endif


/***************************************************************
 *
 *                         ENUM
 */
enum fluid_loop {
  FLUID_UNLOOPED = 0,
  FLUID_LOOP_DURING_RELEASE = 1,
  FLUID_NOTUSED = 2,
  FLUID_LOOP_UNTIL_RELEASE = 3
};

enum fluid_synth_status
{
  FLUID_SYNTH_CLEAN,
  FLUID_SYNTH_PLAYING,
  FLUID_SYNTH_QUIET,
  FLUID_SYNTH_STOPPED
};


typedef struct _fluid_bank_offset_t fluid_bank_offset_t;

struct _fluid_bank_offset_t {
	int sfont_id;
	int offset;
};


/*
 * fluid_synth_t
 */

struct _fluid_synth_t
{
  /* fluid_settings_old_t settings_old;  the old synthesizer settings */
  fluid_settings_t* settings;         /** the synthesizer settings */
  int polyphony;                     /** maximum polyphony */
  char with_reverb;                  /** Should the synth use the built-in reverb unit? */
  char with_chorus;                  /** Should the synth use the built-in chorus unit? */
  char verbose;                      /** Turn verbose mode on? */
  char dump;                         /** Dump events to stdout to hook up a user interface? */
  double sample_rate;                /** The sample rate */
  int midi_channels;                 /** the number of MIDI channels (>= 16) */
  int audio_channels;                /** the number of audio channels (1 channel=left+right) */
  int audio_groups;                  /** the number of (stereo) 'sub'groups from the synth.
					 Typically equal to audio_channels. */
  int effects_channels;              /** the number of effects channels (= 2) */
  unsigned int state;                /** the synthesizer state */
  unsigned int ticks;                /** the number of audio samples since the start */

  fluid_list_t *loaders;              /** the soundfont loaders */
  fluid_list_t* sfont;                /** the loaded soundfont */
  unsigned int sfont_id;
  fluid_list_t* bank_offsets;       /** the offsets of the soundfont banks */

#if defined(MACOS9)
  fluid_list_t* unloading;            /** the soundfonts that need to be unloaded */
#endif

  double gain;                        /** master gain */
  fluid_channel_t** channel;          /** the channels */
  int num_channels;                   /** the number of channels */
  int nvoice;                         /** the length of the synthesis process array */
  fluid_voice_t** voice;              /** the synthesis processes */
  unsigned int noteid;                /** the id is incremented for every new note. it's used for noteoff's  */
  unsigned int storeid;
  int nbuf;                           /** How many audio buffers are used? (depends on nr of audio channels / groups)*/

  fluid_real_t** left_buf;
  fluid_real_t** right_buf;
  fluid_real_t** fx_left_buf;
  fluid_real_t** fx_right_buf;

  fluid_revmodel_t* reverb;
  fluid_chorus_t* chorus;
  int cur;                           /** the current sample in the audio buffers to be output */
  int dither_index;		/* current index in random dither value buffer: fluid_synth_(write_s16|dither_s16) */

  char outbuf[256];                  /** buffer for message output */

  fluid_tuning_t*** tuning;           /** 128 banks of 128 programs for the tunings */
  fluid_tuning_t* cur_tuning;         /** current tuning in the iteration */

  unsigned int min_note_length_ticks; /**< If note-offs are triggered just after a note-on, they will be delayed */
};

/** returns 1 if the value has been set, 0 otherwise */
int fluid_synth_setstr(fluid_synth_t* synth, char* name, char* str);

/** returns 1 if the value exists, 0 otherwise */
int fluid_synth_getstr(fluid_synth_t* synth, char* name, char** str);

/** returns 1 if the value has been set, 0 otherwise */
int fluid_synth_setnum(fluid_synth_t* synth, char* name, double val);

/** returns 1 if the value exists, 0 otherwise */
int fluid_synth_getnum(fluid_synth_t* synth, char* name, double* val);

/** returns 1 if the value has been set, 0 otherwise */
int fluid_synth_setint(fluid_synth_t* synth, char* name, int val);

/** returns 1 if the value exists, 0 otherwise */
int fluid_synth_getint(fluid_synth_t* synth, char* name, int* val);


int fluid_synth_set_reverb_preset(fluid_synth_t* synth, int num);

int fluid_synth_one_block(fluid_synth_t* synth, int do_not_mix_fx_to_out);

fluid_preset_t* fluid_synth_get_preset(fluid_synth_t* synth,
				     unsigned int sfontnum,
				     unsigned int banknum,
				     unsigned int prognum);

fluid_preset_t* fluid_synth_find_preset(fluid_synth_t* synth,
				      unsigned int banknum,
				      unsigned int prognum);

//int fluid_synth_all_notes_off(fluid_synth_t* synth, int chan);
//int fluid_synth_all_sounds_off(fluid_synth_t* synth, int chan);
int fluid_synth_modulate_voices(fluid_synth_t* synth, int chan, int is_cc, int ctrl);
int fluid_synth_modulate_voices_all(fluid_synth_t* synth, int chan);
int fluid_synth_damp_voices(fluid_synth_t* synth, int chan);
int fluid_synth_kill_voice(fluid_synth_t* synth, fluid_voice_t * voice);
void fluid_synth_kill_by_exclusive_class(fluid_synth_t* synth, fluid_voice_t* voice);
void fluid_synth_release_voice_on_same_note(fluid_synth_t* synth, int chan, int key);
void fluid_synth_sfunload_macos9(fluid_synth_t* synth);

void fluid_synth_print_voice(fluid_synth_t* synth);

/** This function assures that every MIDI channels has a valid preset
 *  (NULL is okay). This function is called after a SoundFont is
 *  unloaded or reloaded. */
void fluid_synth_update_presets(fluid_synth_t* synth);


int fluid_synth_update_gain(fluid_synth_t* synth, char* name, double value);
int fluid_synth_update_polyphony(fluid_synth_t* synth, char* name, int value);

fluid_bank_offset_t* fluid_synth_get_bank_offset0(fluid_synth_t* synth, int sfont_id);
void fluid_synth_remove_bank_offset(fluid_synth_t* synth, int sfont_id);

void fluid_synth_dither_s16(int *dither_index, int len, float* lin, float* rin,
			    void* lout, int loff, int lincr,
			    void* rout, int roff, int rincr);
/*
 * misc
 */

void fluid_synth_settings(fluid_settings_t* settings);

#endif  /* _FLUID_SYNTH_H */
