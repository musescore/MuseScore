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

#include "fluid_synth.h"
#include "fluid_sys.h"
#include "fluid_chan.h"
#include "fluid_tuning.h"
#include "fluid_settings.h"
#include "fluid_sfont.h"
#include "fluid_defsfont.h"
#include "fluid_instpatch.h"

#ifdef TRAP_ON_FPE
#define _GNU_SOURCE
#include <fenv.h>

/* seems to not be declared in fenv.h */
extern int feenableexcept(int excepts);
#endif

#define FLUID_API_RETURN(return_value) \
  do { fluid_synth_api_exit(synth); \
  return return_value; } while (0)

#define FLUID_API_RETURN_IF_CHAN_DISABLED(return_value) \
  do { if (FLUID_LIKELY(synth->channel[chan]->mode & FLUID_CHANNEL_ENABLED)) \
       {} \
       else \
       { FLUID_API_RETURN(return_value); } \
  } while (0)

#define FLUID_API_ENTRY_CHAN(fail_value)  \
  fluid_return_val_if_fail (synth != NULL, fail_value); \
  fluid_return_val_if_fail (chan >= 0, fail_value); \
  fluid_synth_api_enter(synth); \
  if (chan >= synth->midi_channels) { \
    FLUID_API_RETURN(fail_value); \
  } \

static void fluid_synth_init(void);
static void fluid_synth_api_enter(fluid_synth_t *synth);
static void fluid_synth_api_exit(fluid_synth_t *synth);

static int fluid_synth_noteon_LOCAL(fluid_synth_t *synth, int chan, int key,
                                    int vel);
static int fluid_synth_noteoff_LOCAL(fluid_synth_t *synth, int chan, int key);
static int fluid_synth_cc_LOCAL(fluid_synth_t *synth, int channum, int num);
static int fluid_synth_sysex_midi_tuning(fluid_synth_t *synth, const char *data,
        int len, char *response,
        int *response_len, int avail_response,
        int *handled, int dryrun);
static int fluid_synth_sysex_gs_dt1(fluid_synth_t *synth, const char *data,
        int len, char *response,
        int *response_len, int avail_response,
        int *handled, int dryrun);
static int fluid_synth_sysex_xg(fluid_synth_t *synth, const char *data,
        int len, char *response,
        int *response_len, int avail_response,
        int *handled, int dryrun);
int fluid_synth_all_notes_off_LOCAL(fluid_synth_t *synth, int chan);
static int fluid_synth_all_sounds_off_LOCAL(fluid_synth_t *synth, int chan);
static int fluid_synth_system_reset_LOCAL(fluid_synth_t *synth);
static int fluid_synth_modulate_voices_LOCAL(fluid_synth_t *synth, int chan,
        int is_cc, int ctrl);
static int fluid_synth_modulate_voices_all_LOCAL(fluid_synth_t *synth, int chan);
static int fluid_synth_update_channel_pressure_LOCAL(fluid_synth_t *synth, int channum);
static int fluid_synth_update_key_pressure_LOCAL(fluid_synth_t *synth, int chan, int key);
static int fluid_synth_update_pitch_bend_LOCAL(fluid_synth_t *synth, int chan);
static int fluid_synth_update_pitch_wheel_sens_LOCAL(fluid_synth_t *synth, int chan);
static int fluid_synth_set_preset(fluid_synth_t *synth, int chan,
                                  fluid_preset_t *preset);
static int fluid_synth_reverb_get_param(fluid_synth_t *synth, int fx_group,
                                        int param, double *value);
static int fluid_synth_chorus_get_param(fluid_synth_t *synth, int fx_group,
                                        int param, double *value);

static fluid_preset_t *
fluid_synth_get_preset(fluid_synth_t *synth, int sfontnum,
                       int banknum, int prognum);
static fluid_preset_t *
fluid_synth_get_preset_by_sfont_name(fluid_synth_t *synth, const char *sfontname,
                                     int banknum, int prognum);

static void fluid_synth_update_presets(fluid_synth_t *synth);
static void fluid_synth_update_gain_LOCAL(fluid_synth_t *synth);
static int fluid_synth_update_polyphony_LOCAL(fluid_synth_t *synth, int new_polyphony);
static void init_dither(void);
static FLUID_INLINE int16_t round_clip_to_i16(float x);
static int fluid_synth_render_blocks(fluid_synth_t *synth, int blockcount);

static fluid_voice_t *fluid_synth_free_voice_by_kill_LOCAL(fluid_synth_t *synth);
static void fluid_synth_kill_by_exclusive_class_LOCAL(fluid_synth_t *synth,
        fluid_voice_t *new_voice);
static int fluid_synth_sfunload_callback(void *data, unsigned int msec);
static fluid_tuning_t *fluid_synth_get_tuning(fluid_synth_t *synth,
        int bank, int prog);
static int fluid_synth_replace_tuning_LOCK(fluid_synth_t *synth,
        fluid_tuning_t *tuning,
        int bank, int prog, int apply);
static void fluid_synth_replace_tuning_LOCAL(fluid_synth_t *synth,
        fluid_tuning_t *old_tuning,
        fluid_tuning_t *new_tuning,
        int apply, int unref_new);
static void fluid_synth_update_voice_tuning_LOCAL(fluid_synth_t *synth,
        fluid_channel_t *channel);
static int fluid_synth_set_tuning_LOCAL(fluid_synth_t *synth, int chan,
                                        fluid_tuning_t *tuning, int apply);
static void fluid_synth_set_gen_LOCAL(fluid_synth_t *synth, int chan,
                                      int param, float value);
static void fluid_synth_stop_LOCAL(fluid_synth_t *synth, unsigned int id);


static int fluid_synth_set_important_channels(fluid_synth_t *synth, const char *channels);


/* Callback handlers for real-time settings */
static void fluid_synth_handle_gain(void *data, const char *name, double value);
static void fluid_synth_handle_polyphony(void *data, const char *name, int value);
static void fluid_synth_handle_device_id(void *data, const char *name, int value);
static void fluid_synth_handle_overflow(void *data, const char *name, double value);
static void fluid_synth_handle_important_channels(void *data, const char *name,
        const char *value);
static void fluid_synth_handle_reverb_chorus_num(void *data, const char *name, double value);
static void fluid_synth_handle_reverb_chorus_int(void *data, const char *name, int value);


static void fluid_synth_reset_basic_channel_LOCAL(fluid_synth_t *synth, int chan, int nbr_chan);
static int fluid_synth_check_next_basic_channel(fluid_synth_t *synth, int basicchan, int mode, int val);
static void fluid_synth_set_basic_channel_LOCAL(fluid_synth_t *synth, int basicchan, int mode, int val);

/***************************************************************
 *
 *                         GLOBAL
 */

/* has the synth module been initialized? */
/* fluid_atomic_int_t may be anything, so init with {0} to catch most cases */
static fluid_atomic_int_t fluid_synth_initialized = {0};

/* default modulators
 * SF2.01 page 52 ff:
 *
 * There is a set of predefined default modulators. They have to be
 * explicitly overridden by the sound font in order to turn them off.
 */

static fluid_mod_t default_vel2att_mod;        /* SF2.01 section 8.4.1  */
/*not static */ fluid_mod_t default_vel2filter_mod;     /* SF2.01 section 8.4.2  */
static fluid_mod_t default_at2viblfo_mod;      /* SF2.01 section 8.4.3  */
static fluid_mod_t default_mod2viblfo_mod;     /* SF2.01 section 8.4.4  */
static fluid_mod_t default_att_mod;            /* SF2.01 section 8.4.5  */
static fluid_mod_t default_pan_mod;            /* SF2.01 section 8.4.6  */
static fluid_mod_t default_expr_mod;           /* SF2.01 section 8.4.7  */
static fluid_mod_t default_reverb_mod;         /* SF2.01 section 8.4.8  */
static fluid_mod_t default_chorus_mod;         /* SF2.01 section 8.4.9  */
static fluid_mod_t default_pitch_bend_mod;     /* SF2.01 section 8.4.10 */
static fluid_mod_t custom_balance_mod;         /* Non-standard modulator */


/* custom_breath2att_modulator is not a default modulator specified in SF
it is intended to replace default_vel2att_mod on demand using
API fluid_set_breath_mode() or command shell setbreathmode.
*/
static fluid_mod_t custom_breath2att_mod;

/* reverb presets */
static const fluid_revmodel_presets_t revmodel_preset[] =
{
    /* name */    /* roomsize */ /* damp */ /* width */ /* level */
    { "Test 1",          0.2f,      0.0f,       0.5f,       0.9f },
    { "Test 2",          0.4f,      0.2f,       0.5f,       0.8f },
    { "Test 3",          0.6f,      0.4f,       0.5f,       0.7f },
    { "Test 4",          0.8f,      0.7f,       0.5f,       0.6f },
    { "Test 5",          0.8f,      1.0f,       0.5f,       0.5f },
};


/***************************************************************
 *
 *               INITIALIZATION & UTILITIES
 */

void fluid_synth_settings(fluid_settings_t *settings)
{
    fluid_settings_register_int(settings, "synth.verbose", 0, 0, 1, FLUID_HINT_TOGGLED);

    fluid_settings_register_int(settings, "synth.reverb.active", 1, 0, 1, FLUID_HINT_TOGGLED);
    fluid_settings_register_num(settings, "synth.reverb.room-size", FLUID_REVERB_DEFAULT_ROOMSIZE, 0.0f, 1.0f, 0);
    fluid_settings_register_num(settings, "synth.reverb.damp", FLUID_REVERB_DEFAULT_DAMP, 0.0f, 1.0f, 0);
    fluid_settings_register_num(settings, "synth.reverb.width", FLUID_REVERB_DEFAULT_WIDTH, 0.0f, 100.0f, 0);
    fluid_settings_register_num(settings, "synth.reverb.level", FLUID_REVERB_DEFAULT_LEVEL, 0.0f, 1.0f, 0);

    fluid_settings_register_int(settings, "synth.chorus.active", 1, 0, 1, FLUID_HINT_TOGGLED);
    fluid_settings_register_int(settings, "synth.chorus.nr", FLUID_CHORUS_DEFAULT_N, 0, 99, 0);
    fluid_settings_register_num(settings, "synth.chorus.level", FLUID_CHORUS_DEFAULT_LEVEL, 0.0f, 10.0f, 0);
    fluid_settings_register_num(settings, "synth.chorus.speed", FLUID_CHORUS_DEFAULT_SPEED, 0.1f, 5.0f, 0);
    fluid_settings_register_num(settings, "synth.chorus.depth", FLUID_CHORUS_DEFAULT_DEPTH, 0.0f, 256.0f, 0);

    fluid_settings_register_int(settings, "synth.ladspa.active", 0, 0, 1, FLUID_HINT_TOGGLED);
    fluid_settings_register_int(settings, "synth.lock-memory", 1, 0, 1, FLUID_HINT_TOGGLED);
    fluid_settings_register_str(settings, "midi.portname", "", 0);

#ifdef DEFAULT_SOUNDFONT
    fluid_settings_register_str(settings, "synth.default-soundfont", DEFAULT_SOUNDFONT, 0);
#endif

    fluid_settings_register_int(settings, "synth.polyphony", 256, 1, 65535, 0);
    fluid_settings_register_int(settings, "synth.midi-channels", 16, 16, 256, 0);
    fluid_settings_register_num(settings, "synth.gain", 0.2f, 0.0f, 10.0f, 0);
    fluid_settings_register_int(settings, "synth.audio-channels", 1, 1, 128, 0);
    fluid_settings_register_int(settings, "synth.audio-groups", 1, 1, 128, 0);
    fluid_settings_register_int(settings, "synth.effects-channels", 2, 2, 2, 0);
    fluid_settings_register_int(settings, "synth.effects-groups", 1, 1, 128, 0);
    fluid_settings_register_num(settings, "synth.sample-rate", 44100.0f, 8000.0f, 384000.0f, 0);
    fluid_settings_register_int(settings, "synth.device-id", 0, 0, 127, 0);
#ifdef ENABLE_MIXER_THREADS
    fluid_settings_register_int(settings, "synth.cpu-cores", 1, 1, 256, 0);
#else
    fluid_settings_register_int(settings, "synth.cpu-cores", 1, 1, 1, 0);
#endif

    fluid_settings_register_int(settings, "synth.min-note-length", 10, 0, 65535, 0);

    fluid_settings_register_int(settings, "synth.threadsafe-api", 1, 0, 1, FLUID_HINT_TOGGLED);

    fluid_settings_register_num(settings, "synth.overflow.percussion", 4000, -10000, 10000, 0);
    fluid_settings_register_num(settings, "synth.overflow.sustained", -1000, -10000, 10000, 0);
    fluid_settings_register_num(settings, "synth.overflow.released", -2000, -10000, 10000, 0);
    fluid_settings_register_num(settings, "synth.overflow.age", 1000, -10000, 10000, 0);
    fluid_settings_register_num(settings, "synth.overflow.volume", 500, -10000, 10000, 0);
    fluid_settings_register_num(settings, "synth.overflow.important", 5000, -50000, 50000, 0);
    fluid_settings_register_str(settings, "synth.overflow.important-channels", "", 0);

    fluid_settings_register_str(settings, "synth.midi-bank-select", "gs", 0);
    fluid_settings_add_option(settings, "synth.midi-bank-select", "gm");
    fluid_settings_add_option(settings, "synth.midi-bank-select", "gs");
    fluid_settings_add_option(settings, "synth.midi-bank-select", "xg");
    fluid_settings_add_option(settings, "synth.midi-bank-select", "mma");

    fluid_settings_register_int(settings, "synth.dynamic-sample-loading", 0, 0, 1, FLUID_HINT_TOGGLED);
}

/**
 * Get FluidSynth runtime version.
 * @param major Location to store major number
 * @param minor Location to store minor number
 * @param micro Location to store micro number
 */
void fluid_version(int *major, int *minor, int *micro)
{
    *major = FLUIDSYNTH_VERSION_MAJOR;
    *minor = FLUIDSYNTH_VERSION_MINOR;
    *micro = FLUIDSYNTH_VERSION_MICRO;
}

/**
 * Get FluidSynth runtime version as a string.
 * @return FluidSynth version string, which is internal and should not be
 *   modified or freed.
 */
char *
fluid_version_str(void)
{
    return FLUIDSYNTH_VERSION;
}

/*
 * void fluid_synth_init
 *
 * Does all the initialization for this module.
 */
static void
fluid_synth_init(void)
{
#ifdef TRAP_ON_FPE
  #if !defined(__GLIBC__) && defined(__linux__)
    #warning "Trap on FPE is only supported when using glibc!"
  #else
    /* Turn on floating point exception traps */
    feenableexcept(FE_DIVBYZERO | FE_OVERFLOW | FE_INVALID);
  #endif
#endif

    init_dither();

    /* custom_breath2att_mod is not a default modulator specified in SF2.01.
     it is intended to replace default_vel2att_mod on demand using
     API fluid_set_breath_mode() or command shell setbreathmode.
     */
    fluid_mod_set_source1(&custom_breath2att_mod, /* The modulator we are programming here */
                          BREATH_MSB,    /* Source. breath MSB corresponds to 2. */
                          FLUID_MOD_CC           /* MIDI continuous controller */
                          | FLUID_MOD_CONCAVE    /* Curve shape. Corresponds to 'type=1' */
                          | FLUID_MOD_UNIPOLAR   /* Polarity. Corresponds to 'P=0' */
                          | FLUID_MOD_NEGATIVE   /* Direction. Corresponds to 'D=1' */
                         );
    fluid_mod_set_source2(&custom_breath2att_mod, 0, 0); /* No 2nd source */
    fluid_mod_set_dest(&custom_breath2att_mod, GEN_ATTENUATION);  /* Target: Initial attenuation */
    fluid_mod_set_amount(&custom_breath2att_mod, FLUID_PEAK_ATTENUATION); /* Modulation amount: 960 */

    /* SF2.01 page 53 section 8.4.1: MIDI Note-On Velocity to Initial Attenuation */
    fluid_mod_set_source1(&default_vel2att_mod, /* The modulator we are programming here */
                          FLUID_MOD_VELOCITY,    /* Source. VELOCITY corresponds to 'index=2'. */
                          FLUID_MOD_GC           /* Not a MIDI continuous controller */
                          | FLUID_MOD_CONCAVE    /* Curve shape. Corresponds to 'type=1' */
                          | FLUID_MOD_UNIPOLAR   /* Polarity. Corresponds to 'P=0' */
                          | FLUID_MOD_NEGATIVE   /* Direction. Corresponds to 'D=1' */
                         );
    fluid_mod_set_source2(&default_vel2att_mod, 0, 0); /* No 2nd source */
    fluid_mod_set_dest(&default_vel2att_mod, GEN_ATTENUATION);  /* Target: Initial attenuation */
    fluid_mod_set_amount(&default_vel2att_mod, FLUID_PEAK_ATTENUATION); /* Modulation amount: 960 */



    /* SF2.01 page 53 section 8.4.2: MIDI Note-On Velocity to Filter Cutoff
     * Have to make a design decision here. The specs don't make any sense this way or another.
     * One sound font, 'Kingston Piano', which has been praised for its quality, tries to
     * override this modulator with an amount of 0 and positive polarity (instead of what
     * the specs say, D=1) for the secondary source.
     * So if we change the polarity to 'positive', one of the best free sound fonts works...
     */
    fluid_mod_set_source1(&default_vel2filter_mod, FLUID_MOD_VELOCITY, /* Index=2 */
                          FLUID_MOD_GC                        /* CC=0 */
                          | FLUID_MOD_LINEAR                  /* type=0 */
                          | FLUID_MOD_UNIPOLAR                /* P=0 */
                          | FLUID_MOD_NEGATIVE                /* D=1 */
                         );
    fluid_mod_set_source2(&default_vel2filter_mod, FLUID_MOD_VELOCITY, /* Index=2 */
                          FLUID_MOD_GC                                 /* CC=0 */
                          | FLUID_MOD_SWITCH                           /* type=3 */
                          | FLUID_MOD_UNIPOLAR                         /* P=0 */
                          // do not remove       | FLUID_MOD_NEGATIVE                         /* D=1 */
                          | FLUID_MOD_POSITIVE                         /* D=0 */
                         );
    fluid_mod_set_dest(&default_vel2filter_mod, GEN_FILTERFC);        /* Target: Initial filter cutoff */
    fluid_mod_set_amount(&default_vel2filter_mod, -2400);



    /* SF2.01 page 53 section 8.4.3: MIDI Channel pressure to Vibrato LFO pitch depth */
    fluid_mod_set_source1(&default_at2viblfo_mod, FLUID_MOD_CHANNELPRESSURE, /* Index=13 */
                          FLUID_MOD_GC                        /* CC=0 */
                          | FLUID_MOD_LINEAR                  /* type=0 */
                          | FLUID_MOD_UNIPOLAR                /* P=0 */
                          | FLUID_MOD_POSITIVE                /* D=0 */
                         );
    fluid_mod_set_source2(&default_at2viblfo_mod, 0, 0); /* no second source */
    fluid_mod_set_dest(&default_at2viblfo_mod, GEN_VIBLFOTOPITCH);        /* Target: Vib. LFO => pitch */
    fluid_mod_set_amount(&default_at2viblfo_mod, 50);



    /* SF2.01 page 53 section 8.4.4: Mod wheel (Controller 1) to Vibrato LFO pitch depth */
    fluid_mod_set_source1(&default_mod2viblfo_mod, MODULATION_MSB, /* Index=1 */
                          FLUID_MOD_CC                        /* CC=1 */
                          | FLUID_MOD_LINEAR                  /* type=0 */
                          | FLUID_MOD_UNIPOLAR                /* P=0 */
                          | FLUID_MOD_POSITIVE                /* D=0 */
                         );
    fluid_mod_set_source2(&default_mod2viblfo_mod, 0, 0); /* no second source */
    fluid_mod_set_dest(&default_mod2viblfo_mod, GEN_VIBLFOTOPITCH);        /* Target: Vib. LFO => pitch */
    fluid_mod_set_amount(&default_mod2viblfo_mod, 50);



    /* SF2.01 page 55 section 8.4.5: MIDI continuous controller 7 to initial attenuation*/
    fluid_mod_set_source1(&default_att_mod, VOLUME_MSB,    /* index=7 */
                          FLUID_MOD_CC                              /* CC=1 */
                          | FLUID_MOD_CONCAVE                       /* type=1 */
                          | FLUID_MOD_UNIPOLAR                      /* P=0 */
                          | FLUID_MOD_NEGATIVE                      /* D=1 */
                         );
    fluid_mod_set_source2(&default_att_mod, 0, 0);                 /* No second source */
    fluid_mod_set_dest(&default_att_mod, GEN_ATTENUATION);         /* Target: Initial attenuation */
    fluid_mod_set_amount(&default_att_mod, FLUID_PEAK_ATTENUATION);  /* Amount: 960 */



    /* SF2.01 page 55 section 8.4.6 MIDI continuous controller 10 to Pan Position */
    fluid_mod_set_source1(&default_pan_mod, PAN_MSB,       /* index=10 */
                          FLUID_MOD_CC                              /* CC=1 */
                          | FLUID_MOD_LINEAR                        /* type=0 */
                          | FLUID_MOD_BIPOLAR                       /* P=1 */
                          | FLUID_MOD_POSITIVE                      /* D=0 */
                         );
    fluid_mod_set_source2(&default_pan_mod, 0, 0);                 /* No second source */
    fluid_mod_set_dest(&default_pan_mod, GEN_PAN);                 /* Target: pan */
    /* Amount: 500. The SF specs $8.4.6, p. 55 says: "Amount = 1000
       tenths of a percent". The center value (64) corresponds to 50%,
       so it follows that amount = 50% x 1000/% = 500. */
    fluid_mod_set_amount(&default_pan_mod, 500.0);


    /* SF2.01 page 55 section 8.4.7: MIDI continuous controller 11 to initial attenuation*/
    fluid_mod_set_source1(&default_expr_mod, EXPRESSION_MSB, /* index=11 */
                          FLUID_MOD_CC                              /* CC=1 */
                          | FLUID_MOD_CONCAVE                       /* type=1 */
                          | FLUID_MOD_UNIPOLAR                      /* P=0 */
                          | FLUID_MOD_NEGATIVE                      /* D=1 */
                         );
    fluid_mod_set_source2(&default_expr_mod, 0, 0);                 /* No second source */
    fluid_mod_set_dest(&default_expr_mod, GEN_ATTENUATION);         /* Target: Initial attenuation */
    fluid_mod_set_amount(&default_expr_mod, FLUID_PEAK_ATTENUATION);  /* Amount: 960 */



    /* SF2.01 page 55 section 8.4.8: MIDI continuous controller 91 to Reverb send */
    fluid_mod_set_source1(&default_reverb_mod, EFFECTS_DEPTH1, /* index=91 */
                          FLUID_MOD_CC                              /* CC=1 */
                          | FLUID_MOD_LINEAR                        /* type=0 */
                          | FLUID_MOD_UNIPOLAR                      /* P=0 */
                          | FLUID_MOD_POSITIVE                      /* D=0 */
                         );
    fluid_mod_set_source2(&default_reverb_mod, 0, 0);              /* No second source */
    fluid_mod_set_dest(&default_reverb_mod, GEN_REVERBSEND);       /* Target: Reverb send */
    fluid_mod_set_amount(&default_reverb_mod, 200);                /* Amount: 200 ('tenths of a percent') */



    /* SF2.01 page 55 section 8.4.9: MIDI continuous controller 93 to Chorus send */
    fluid_mod_set_source1(&default_chorus_mod, EFFECTS_DEPTH3, /* index=93 */
                          FLUID_MOD_CC                              /* CC=1 */
                          | FLUID_MOD_LINEAR                        /* type=0 */
                          | FLUID_MOD_UNIPOLAR                      /* P=0 */
                          | FLUID_MOD_POSITIVE                      /* D=0 */
                         );
    fluid_mod_set_source2(&default_chorus_mod, 0, 0);              /* No second source */
    fluid_mod_set_dest(&default_chorus_mod, GEN_CHORUSSEND);       /* Target: Chorus */
    fluid_mod_set_amount(&default_chorus_mod, 200);                /* Amount: 200 ('tenths of a percent') */



    /* SF2.01 page 57 section 8.4.10 MIDI Pitch Wheel to Initial Pitch ... */
    /* Initial Pitch is not a "standard" generator, because it isn't mentioned in the
       list of generators in the SF2 specifications. That's why destination Initial Pitch
       is replaced here by fine tune generator.
     */
    fluid_mod_set_source1(&default_pitch_bend_mod, FLUID_MOD_PITCHWHEEL, /* Index=14 */
                          FLUID_MOD_GC                              /* CC =0 */
                          | FLUID_MOD_LINEAR                        /* type=0 */
                          | FLUID_MOD_BIPOLAR                       /* P=1 */
                          | FLUID_MOD_POSITIVE                      /* D=0 */
                         );
    fluid_mod_set_source2(&default_pitch_bend_mod, FLUID_MOD_PITCHWHEELSENS,  /* Index = 16 */
                          FLUID_MOD_GC                                        /* CC=0 */
                          | FLUID_MOD_LINEAR                                  /* type=0 */
                          | FLUID_MOD_UNIPOLAR                                /* P=0 */
                          | FLUID_MOD_POSITIVE                                /* D=0 */
                         );
    /* Also see the comment in gen.h about GEN_PITCH */
    fluid_mod_set_dest(&default_pitch_bend_mod, GEN_FINETUNE);              /* Destination: Fine Tune */
    fluid_mod_set_amount(&default_pitch_bend_mod, 12700.0);                 /* Amount: 12700 cents */


    /* Non-standard MIDI continuous controller 8 to channel stereo balance */
    fluid_mod_set_source1(&custom_balance_mod, BALANCE_MSB, /* Index=8 */
                          FLUID_MOD_CC                              /* CC=1 */
                          | FLUID_MOD_CONCAVE                       /* type=1 */
                          | FLUID_MOD_BIPOLAR                       /* P=1 */
                          | FLUID_MOD_POSITIVE                      /* D=0 */
                         );
    fluid_mod_set_source2(&custom_balance_mod, 0, 0);
    fluid_mod_set_dest(&custom_balance_mod, GEN_CUSTOM_BALANCE);     /* Destination: stereo balance */
    /* Amount: 96 dB of attenuation (on the opposite channel) */
    fluid_mod_set_amount(&custom_balance_mod, FLUID_PEAK_ATTENUATION); /* Amount: 960 */

#if defined(LIBINSTPATCH_SUPPORT)
    /* defer libinstpatch init to fluid_instpatch.c to avoid #include "libinstpatch.h" */
    if(!fluid_instpatch_supports_multi_init())
    {
        fluid_instpatch_init();
    }
#endif
}

static FLUID_INLINE unsigned int fluid_synth_get_ticks(fluid_synth_t *synth)
{
    return fluid_atomic_int_get(&synth->ticks_since_start);
}

static FLUID_INLINE void fluid_synth_add_ticks(fluid_synth_t *synth, int val)
{
    fluid_atomic_uint_add(&synth->ticks_since_start, val);
}


/***************************************************************
 *                    FLUID SAMPLE TIMERS
 *    Timers that use written audio data as timing reference
 */
struct _fluid_sample_timer_t
{
    fluid_sample_timer_t *next; /* Single linked list of timers */
    unsigned long starttick;
    fluid_timer_callback_t callback;
    void *data;
    int isfinished;
};

/*
 * fluid_sample_timer_process - called when synth->ticks is updated
 */
static void fluid_sample_timer_process(fluid_synth_t *synth)
{
    fluid_sample_timer_t *st;
    long msec;
    int cont;
    unsigned int ticks = fluid_synth_get_ticks(synth);

    for(st = synth->sample_timers; st; st = st->next)
    {
        if(st->isfinished)
        {
            continue;
        }

        msec = (long)(1000.0 * ((double)(ticks - st->starttick)) / synth->sample_rate);
        cont = (*st->callback)(st->data, msec);

        if(cont == 0)
        {
            st->isfinished = 1;
        }
    }
}

fluid_sample_timer_t *new_fluid_sample_timer(fluid_synth_t *synth, fluid_timer_callback_t callback, void *data)
{
    fluid_sample_timer_t *result = FLUID_NEW(fluid_sample_timer_t);

    if(result == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    fluid_sample_timer_reset(synth, result);
    result->data = data;
    result->callback = callback;
    result->next = synth->sample_timers;
    synth->sample_timers = result;
    return result;
}

void delete_fluid_sample_timer(fluid_synth_t *synth, fluid_sample_timer_t *timer)
{
    fluid_sample_timer_t **ptr;
    fluid_return_if_fail(synth != NULL);
    fluid_return_if_fail(timer != NULL);

    ptr = &synth->sample_timers;

    while(*ptr)
    {
        if(*ptr == timer)
        {
            *ptr = timer->next;
            FLUID_FREE(timer);
            return;
        }

        ptr = &((*ptr)->next);
    }
}

void fluid_sample_timer_reset(fluid_synth_t *synth, fluid_sample_timer_t *timer)
{
    timer->starttick = fluid_synth_get_ticks(synth);
    timer->isfinished = 0;
}

/***************************************************************
 *
 *                      FLUID SYNTH
 */

static FLUID_INLINE void
fluid_synth_update_mixer(fluid_synth_t *synth, fluid_rvoice_function_t method, int intparam,
                         fluid_real_t realparam)
{
    fluid_return_if_fail(synth != NULL && synth->eventhandler != NULL);
    fluid_return_if_fail(synth->eventhandler->mixer != NULL);
    fluid_rvoice_eventhandler_push_int_real(synth->eventhandler, method,
                                            synth->eventhandler->mixer,
                                            intparam, realparam);
}

static FLUID_INLINE unsigned int fluid_synth_get_min_note_length_LOCAL(fluid_synth_t *synth)
{
    int i;
    fluid_settings_getint(synth->settings, "synth.min-note-length", &i);
    return (unsigned int)(i * synth->sample_rate / 1000.0f);
}

/**
 * Create new FluidSynth instance.
 * @param settings Configuration parameters to use (used directly).
 * @return New FluidSynth instance or NULL on error
 *
 * @note The @p settings parameter is used directly, but the synth does not take ownership of it.
 * Hence, the caller is responsible for freeing it, when no longer needed.
 * Further note that you may modify FluidSettings of the
 * @p settings instance. However, only those FluidSettings marked as 'realtime' will
 * affect the synth immediately. See the \ref fluidsettings for more details.
 *
 * @warning The @p settings object should only be used by a single synth at a time. I.e. creating
 * multiple synth instances with a single @p settings object causes undefined behavior. Once the
 * "single synth" has been deleted, you may use the @p settings object again for another synth.
 */
fluid_synth_t *
new_fluid_synth(fluid_settings_t *settings)
{
    fluid_synth_t *synth;
    fluid_sfloader_t *loader;
    char *important_channels;
    int i, prio_level = 0;
    int with_ladspa = 0;
    double sample_rate_min, sample_rate_max;

    /* initialize all the conversion tables and other stuff */
    if(fluid_atomic_int_compare_and_exchange(&fluid_synth_initialized, 0, 1))
    {
        fluid_synth_init();
    }

    /* allocate a new synthesizer object */
    synth = FLUID_NEW(fluid_synth_t);

    if(synth == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(synth, 0, sizeof(fluid_synth_t));

#if defined(LIBINSTPATCH_SUPPORT)
    if(fluid_instpatch_supports_multi_init())
    {
        fluid_instpatch_init();
    }
#endif

    fluid_rec_mutex_init(synth->mutex);
    fluid_settings_getint(settings, "synth.threadsafe-api", &synth->use_mutex);
    synth->public_api_count = 0;

    synth->settings = settings;

    fluid_settings_getint(settings, "synth.reverb.active", &synth->with_reverb);
    fluid_settings_getint(settings, "synth.chorus.active", &synth->with_chorus);
    fluid_settings_getint(settings, "synth.verbose", &synth->verbose);

    fluid_settings_getint(settings, "synth.polyphony", &synth->polyphony);
    fluid_settings_getnum(settings, "synth.sample-rate", &synth->sample_rate);
    fluid_settings_getnum_range(settings, "synth.sample-rate", &sample_rate_min, &sample_rate_max);
    fluid_settings_getint(settings, "synth.midi-channels", &synth->midi_channels);
    fluid_settings_getint(settings, "synth.audio-channels", &synth->audio_channels);
    fluid_settings_getint(settings, "synth.audio-groups", &synth->audio_groups);
    fluid_settings_getint(settings, "synth.effects-channels", &synth->effects_channels);
    fluid_settings_getint(settings, "synth.effects-groups", &synth->effects_groups);
    fluid_settings_getnum_float(settings, "synth.gain", &synth->gain);
    fluid_settings_getint(settings, "synth.device-id", &synth->device_id);
    fluid_settings_getint(settings, "synth.cpu-cores", &synth->cores);

    fluid_settings_getnum_float(settings, "synth.overflow.percussion", &synth->overflow.percussion);
    fluid_settings_getnum_float(settings, "synth.overflow.released", &synth->overflow.released);
    fluid_settings_getnum_float(settings, "synth.overflow.sustained", &synth->overflow.sustained);
    fluid_settings_getnum_float(settings, "synth.overflow.volume", &synth->overflow.volume);
    fluid_settings_getnum_float(settings, "synth.overflow.age", &synth->overflow.age);
    fluid_settings_getnum_float(settings, "synth.overflow.important", &synth->overflow.important);

    /* register the callbacks */
    fluid_settings_callback_num(settings, "synth.gain",
                                fluid_synth_handle_gain, synth);
    fluid_settings_callback_int(settings, "synth.polyphony",
                                fluid_synth_handle_polyphony, synth);
    fluid_settings_callback_int(settings, "synth.device-id",
                                fluid_synth_handle_device_id, synth);
    fluid_settings_callback_num(settings, "synth.overflow.percussion",
                                fluid_synth_handle_overflow, synth);
    fluid_settings_callback_num(settings, "synth.overflow.sustained",
                                fluid_synth_handle_overflow, synth);
    fluid_settings_callback_num(settings, "synth.overflow.released",
                                fluid_synth_handle_overflow, synth);
    fluid_settings_callback_num(settings, "synth.overflow.age",
                                fluid_synth_handle_overflow, synth);
    fluid_settings_callback_num(settings, "synth.overflow.volume",
                                fluid_synth_handle_overflow, synth);
    fluid_settings_callback_num(settings, "synth.overflow.important",
                                fluid_synth_handle_overflow, synth);
    fluid_settings_callback_str(settings, "synth.overflow.important-channels",
                                fluid_synth_handle_important_channels, synth);
    fluid_settings_callback_num(settings, "synth.reverb.room-size",
                                fluid_synth_handle_reverb_chorus_num, synth);
    fluid_settings_callback_num(settings, "synth.reverb.damp",
                                fluid_synth_handle_reverb_chorus_num, synth);
    fluid_settings_callback_num(settings, "synth.reverb.width",
                                fluid_synth_handle_reverb_chorus_num, synth);
    fluid_settings_callback_num(settings, "synth.reverb.level",
                                fluid_synth_handle_reverb_chorus_num, synth);
    fluid_settings_callback_int(settings, "synth.reverb.active",
                                fluid_synth_handle_reverb_chorus_int, synth);
    fluid_settings_callback_int(settings, "synth.chorus.active",
                                fluid_synth_handle_reverb_chorus_int, synth);
    fluid_settings_callback_int(settings, "synth.chorus.nr",
                                fluid_synth_handle_reverb_chorus_int, synth);
    fluid_settings_callback_num(settings, "synth.chorus.level",
                                fluid_synth_handle_reverb_chorus_num, synth);
    fluid_settings_callback_num(settings, "synth.chorus.depth",
                                fluid_synth_handle_reverb_chorus_num, synth);
    fluid_settings_callback_num(settings, "synth.chorus.speed",
                                fluid_synth_handle_reverb_chorus_num, synth);

    /* do some basic sanity checking on the settings */

    if(synth->midi_channels % 16 != 0)
    {
        int n = synth->midi_channels / 16;
        synth->midi_channels = (n + 1) * 16;
        fluid_settings_setint(settings, "synth.midi-channels", synth->midi_channels);
        FLUID_LOG(FLUID_WARN, "Requested number of MIDI channels is not a multiple of 16. "
                  "I'll increase the number of channels to the next multiple.");
    }

    if(synth->audio_channels < 1)
    {
        FLUID_LOG(FLUID_WARN, "Requested number of audio channels is smaller than 1. "
                  "Changing this setting to 1.");
        synth->audio_channels = 1;
    }
    else if(synth->audio_channels > 128)
    {
        FLUID_LOG(FLUID_WARN, "Requested number of audio channels is too big (%d). "
                  "Limiting this setting to 128.", synth->audio_channels);
        synth->audio_channels = 128;
    }

    if(synth->audio_groups < 1)
    {
        FLUID_LOG(FLUID_WARN, "Requested number of audio groups is smaller than 1. "
                  "Changing this setting to 1.");
        synth->audio_groups = 1;
    }
    else if(synth->audio_groups > 128)
    {
        FLUID_LOG(FLUID_WARN, "Requested number of audio groups is too big (%d). "
                  "Limiting this setting to 128.", synth->audio_groups);
        synth->audio_groups = 128;
    }

    if(synth->effects_channels < 2)
    {
        FLUID_LOG(FLUID_WARN, "Invalid number of effects channels (%d)."
                  "Setting effects channels to 2.", synth->effects_channels);
        synth->effects_channels = 2;
    }

    /*
     number of buffers rendered by the mixer is determined by synth->audio_groups.
     audio from MIDI channel is rendered, mapped and mixed in these buffers.

     Typically synth->audio_channels is only used by audio driver and should be set
     to the same value that synth->audio_groups. In some situation using LADSPA,
     it is best to diminish audio-channels so that the driver will be able to pass
     the audio to audio devices in the case these devices have a limited number of
     audio channels.

     audio-channels must not be greater then audio-groups, otherwise these
     audio output above audio-groups will not be rendered by the mixeur.
    */
    if(synth->audio_channels > synth->audio_groups)
    {
        synth->audio_channels = synth->audio_groups;
        fluid_settings_setint(settings, "synth.audio-channels", synth->audio_channels);
                       FLUID_LOG(FLUID_WARN, "Requested audio-channels to high. "
                       "Limiting this setting to audio-groups.");
    }

    if(fluid_settings_dupstr(settings, "synth.overflow.important-channels",
                             &important_channels) == FLUID_OK)
    {
        if(fluid_synth_set_important_channels(synth, important_channels) != FLUID_OK)
        {
            FLUID_LOG(FLUID_WARN, "Failed to set overflow important channels");
        }

        FLUID_FREE(important_channels);
    }

    /* as soon as the synth is created it starts playing. */
    synth->state = FLUID_SYNTH_PLAYING;

    synth->fromkey_portamento = INVALID_NOTE;		/* disable portamento */

    fluid_atomic_int_set(&synth->ticks_since_start, 0);
    synth->tuning = NULL;
    fluid_private_init(synth->tuning_iter);

    /* Initialize multi-core variables if multiple cores enabled */
    if(synth->cores > 1)
    {
        fluid_settings_getint(synth->settings, "audio.realtime-prio", &prio_level);
    }

    /* Allocate event queue for rvoice mixer */
    /* In an overflow situation, a new voice takes about 50 spaces in the queue! */
    synth->eventhandler = new_fluid_rvoice_eventhandler(synth->polyphony * 64,
                          synth->polyphony, synth->audio_groups,
                          synth->effects_channels, synth->effects_groups,
                          (fluid_real_t)sample_rate_max, synth->sample_rate,
                          synth->cores - 1, prio_level);

    if(synth->eventhandler == NULL)
    {
        goto error_recovery;
    }

    /* Setup the list of default modulators.
     * Needs to happen after eventhandler has been set up, as fluid_synth_enter_api is called in the process */
    synth->default_mod = NULL;
    fluid_synth_add_default_mod(synth, &default_vel2att_mod, FLUID_SYNTH_ADD);
    fluid_synth_add_default_mod(synth, &default_vel2filter_mod, FLUID_SYNTH_ADD);
    fluid_synth_add_default_mod(synth, &default_at2viblfo_mod, FLUID_SYNTH_ADD);
    fluid_synth_add_default_mod(synth, &default_mod2viblfo_mod, FLUID_SYNTH_ADD);
    fluid_synth_add_default_mod(synth, &default_att_mod, FLUID_SYNTH_ADD);
    fluid_synth_add_default_mod(synth, &default_pan_mod, FLUID_SYNTH_ADD);
    fluid_synth_add_default_mod(synth, &default_expr_mod, FLUID_SYNTH_ADD);
    fluid_synth_add_default_mod(synth, &default_reverb_mod, FLUID_SYNTH_ADD);
    fluid_synth_add_default_mod(synth, &default_chorus_mod, FLUID_SYNTH_ADD);
    fluid_synth_add_default_mod(synth, &default_pitch_bend_mod, FLUID_SYNTH_ADD);
    fluid_synth_add_default_mod(synth, &custom_balance_mod, FLUID_SYNTH_ADD);

    /* Create and initialize the Fx unit.*/
    fluid_settings_getint(settings, "synth.ladspa.active", &with_ladspa);

    if(with_ladspa)
    {
#ifdef LADSPA
        synth->ladspa_fx = new_fluid_ladspa_fx(synth->sample_rate,
                                               FLUID_MIXER_MAX_BUFFERS_DEFAULT * FLUID_BUFSIZE);

        if(synth->ladspa_fx == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            goto error_recovery;
        }

        fluid_rvoice_mixer_set_ladspa(synth->eventhandler->mixer, synth->ladspa_fx,
                                      synth->audio_groups);
#else /* LADSPA */
        FLUID_LOG(FLUID_WARN, "FluidSynth has not been compiled with LADSPA support");
#endif /* LADSPA */
    }

    /* allocate and add the dls sfont loader */
#ifdef LIBINSTPATCH_SUPPORT
    loader = new_fluid_instpatch_loader(settings);

    if(loader == NULL)
    {
        FLUID_LOG(FLUID_WARN, "Failed to create the instpatch SoundFont loader");
    }
    else
    {
        fluid_synth_add_sfloader(synth, loader);
    }
#endif

    /* allocate and add the default sfont loader */
    loader = new_fluid_defsfloader(settings);

    if(loader == NULL)
    {
        FLUID_LOG(FLUID_WARN, "Failed to create the default SoundFont loader");
    }
    else
    {
        fluid_synth_add_sfloader(synth, loader);
    }

    /* allocate all channel objects */
    synth->channel = FLUID_ARRAY(fluid_channel_t *, synth->midi_channels);

    if(synth->channel == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        goto error_recovery;
    }

    FLUID_MEMSET(synth->channel, 0, synth->midi_channels * sizeof(*synth->channel));
    for(i = 0; i < synth->midi_channels; i++)
    {
        synth->channel[i] = new_fluid_channel(synth, i);

        if(synth->channel[i] == NULL)
        {
            goto error_recovery;
        }
    }

    /* allocate all synthesis processes */
    synth->nvoice = synth->polyphony;
    synth->voice = FLUID_ARRAY(fluid_voice_t *, synth->nvoice);

    if(synth->voice == NULL)
    {
        goto error_recovery;
    }

    FLUID_MEMSET(synth->voice, 0, synth->nvoice * sizeof(*synth->voice));
    for(i = 0; i < synth->nvoice; i++)
    {
        synth->voice[i] = new_fluid_voice(synth->eventhandler, synth->sample_rate);

        if(synth->voice[i] == NULL)
        {
            goto error_recovery;
        }
    }

    /* sets a default basic channel */
    /* Sets one basic channel: basic channel 0, mode 0 (Omni On - Poly) */
    /* (i.e all channels are polyphonic) */
    /* Must be called after channel objects allocation */
    fluid_synth_set_basic_channel_LOCAL(synth, 0, FLUID_CHANNEL_MODE_OMNION_POLY,
                                        synth->midi_channels);

    synth->min_note_length_ticks = fluid_synth_get_min_note_length_LOCAL(synth);


    fluid_synth_update_mixer(synth, fluid_rvoice_mixer_set_polyphony,
                             synth->polyphony, 0.0f);
    fluid_synth_reverb_on(synth, -1, synth->with_reverb);
    fluid_synth_chorus_on(synth, -1, synth->with_chorus);

    synth->cur = FLUID_BUFSIZE;
    synth->curmax = 0;
    synth->dither_index = 0;

    {
        double values[FLUID_REVERB_PARAM_LAST];

        fluid_settings_getnum(settings, "synth.reverb.room-size", &values[FLUID_REVERB_ROOMSIZE]);
        fluid_settings_getnum(settings, "synth.reverb.damp", &values[FLUID_REVERB_DAMP]);
        fluid_settings_getnum(settings, "synth.reverb.width", &values[FLUID_REVERB_WIDTH]);
        fluid_settings_getnum(settings, "synth.reverb.level", &values[FLUID_REVERB_LEVEL]);

        fluid_synth_set_reverb_full(synth, -1, FLUID_REVMODEL_SET_ALL, values);
    }

    {
        double values[FLUID_CHORUS_PARAM_LAST];

        fluid_settings_getint(settings, "synth.chorus.nr", &i);
        values[FLUID_CHORUS_NR] = (double)i;
        fluid_settings_getnum(settings, "synth.chorus.level", &values[FLUID_CHORUS_LEVEL]);
        fluid_settings_getnum(settings, "synth.chorus.speed", &values[FLUID_CHORUS_SPEED]);
        fluid_settings_getnum(settings, "synth.chorus.depth", &values[FLUID_CHORUS_DEPTH]);
        values[FLUID_CHORUS_TYPE] = (double)FLUID_CHORUS_DEFAULT_TYPE;

        fluid_synth_set_chorus_full(synth, -1, FLUID_CHORUS_SET_ALL, values);
    }


    synth->bank_select = FLUID_BANK_STYLE_GS;

    if(fluid_settings_str_equal(settings, "synth.midi-bank-select", "gm"))
    {
        synth->bank_select = FLUID_BANK_STYLE_GM;
    }
    else if(fluid_settings_str_equal(settings, "synth.midi-bank-select", "gs"))
    {
        synth->bank_select = FLUID_BANK_STYLE_GS;
    }
    else if(fluid_settings_str_equal(settings, "synth.midi-bank-select", "xg"))
    {
        synth->bank_select = FLUID_BANK_STYLE_XG;
    }
    else if(fluid_settings_str_equal(settings, "synth.midi-bank-select", "mma"))
    {
        synth->bank_select = FLUID_BANK_STYLE_MMA;
    }

    fluid_synth_process_event_queue(synth);

    /* FIXME */
    synth->start = fluid_curtime();

    return synth;

error_recovery:
    delete_fluid_synth(synth);
    return NULL;
}


/**
 * Delete a FluidSynth instance.
 * @param synth FluidSynth instance to delete
 *
 * @note Other users of a synthesizer instance, such as audio and MIDI drivers,
 * should be deleted prior to freeing the FluidSynth instance.
 */
void
delete_fluid_synth(fluid_synth_t *synth)
{
    int i, k;
    fluid_list_t *list;
    fluid_sfont_t *sfont;
    fluid_sfloader_t *loader;

    fluid_return_if_fail(synth != NULL);

    fluid_profiling_print();

    /* unregister all real-time settings callback, to avoid a use-after-free when changing those settings after
     * this synth has been deleted*/

    fluid_settings_callback_num(synth->settings, "synth.gain",
                                NULL, NULL);
    fluid_settings_callback_int(synth->settings, "synth.polyphony",
                                NULL, NULL);
    fluid_settings_callback_int(synth->settings, "synth.device-id",
                                NULL, NULL);
    fluid_settings_callback_num(synth->settings, "synth.overflow.percussion",
                                NULL, NULL);
    fluid_settings_callback_num(synth->settings, "synth.overflow.sustained",
                                NULL, NULL);
    fluid_settings_callback_num(synth->settings, "synth.overflow.released",
                                NULL, NULL);
    fluid_settings_callback_num(synth->settings, "synth.overflow.age",
                                NULL, NULL);
    fluid_settings_callback_num(synth->settings, "synth.overflow.volume",
                                NULL, NULL);
    fluid_settings_callback_num(synth->settings, "synth.overflow.important",
                                NULL, NULL);
    fluid_settings_callback_str(synth->settings, "synth.overflow.important-channels",
                                NULL, NULL);
    fluid_settings_callback_num(synth->settings, "synth.reverb.room-size",
                                NULL, NULL);
    fluid_settings_callback_num(synth->settings, "synth.reverb.damp",
                                NULL, NULL);
    fluid_settings_callback_num(synth->settings, "synth.reverb.width",
                                NULL, NULL);
    fluid_settings_callback_num(synth->settings, "synth.reverb.level",
                                NULL, NULL);
    fluid_settings_callback_int(synth->settings, "synth.reverb.active",
                                NULL, NULL);
    fluid_settings_callback_int(synth->settings, "synth.chorus.active",
                                NULL, NULL);
    fluid_settings_callback_int(synth->settings, "synth.chorus.nr",
                                NULL, NULL);
    fluid_settings_callback_num(synth->settings, "synth.chorus.level",
                                NULL, NULL);
    fluid_settings_callback_num(synth->settings, "synth.chorus.depth",
                                NULL, NULL);
    fluid_settings_callback_num(synth->settings, "synth.chorus.speed",
                                NULL, NULL);

    /* turn off all voices, needed to unload SoundFont data */
    if(synth->voice != NULL)
    {
        for(i = 0; i < synth->nvoice; i++)
        {
            fluid_voice_t *voice = synth->voice[i];

            if(!voice)
            {
                continue;
            }

            /* WARNING: A this point we must ensure that the reference counter
               of any soundfont sample owned by any rvoice belonging to the voice
               are correctly decremented. This is the contrary part to
               to fluid_voice_init() where the sample's reference counter is
               incremented.
            */
            fluid_voice_unlock_rvoice(voice);
            fluid_voice_overflow_rvoice_finished(voice);

            if(fluid_voice_is_playing(voice))
            {
                fluid_voice_off(voice);
                /* If we only use fluid_voice_off(voice) it will trigger a delayed
                 * fluid_voice_stop(voice) via fluid_synth_check_finished_voices().
                 * But here, we are deleting the fluid_synth_t instance so
                 * fluid_voice_stop() will be never triggered resulting in
                 * SoundFont data never unloaded (i.e a serious memory leak).
                 * So, fluid_voice_stop() must be explicitly called to insure
                 * unloading SoundFont data
                 */
                fluid_voice_stop(voice);
            }
        }
    }

    /* also unset all presets for clean SoundFont unload */
    if(synth->channel != NULL)
    {
        for(i = 0; i < synth->midi_channels; i++)
        {
            if(synth->channel[i] != NULL)
            {
                fluid_channel_set_preset(synth->channel[i], NULL);
            }
        }
    }

    delete_fluid_rvoice_eventhandler(synth->eventhandler);

    /* delete all the SoundFonts */
    for(list = synth->sfont; list; list = fluid_list_next(list))
    {
        sfont = fluid_list_get(list);
        fluid_sfont_delete_internal(sfont);
    }

    delete_fluid_list(synth->sfont);

    /* delete all the SoundFont loaders */

    for(list = synth->loaders; list; list = fluid_list_next(list))
    {
        loader = (fluid_sfloader_t *) fluid_list_get(list);
        fluid_sfloader_delete(loader);
    }

    delete_fluid_list(synth->loaders);

    /* wait for and delete all the lazy sfont unloading timers */

    for(list = synth->fonts_to_be_unloaded; list; list = fluid_list_next(list))
    {
        fluid_timer_t* timer = fluid_list_get(list);
        // explicitly join to wait for the unload really to happen
        fluid_timer_join(timer);
        // delete_fluid_timer alone would stop the timer, even if it had not unloaded the soundfont yet
        delete_fluid_timer(timer);
    }

    delete_fluid_list(synth->fonts_to_be_unloaded);

    if(synth->channel != NULL)
    {
        for(i = 0; i < synth->midi_channels; i++)
        {
            delete_fluid_channel(synth->channel[i]);
        }

        FLUID_FREE(synth->channel);
    }

    if(synth->voice != NULL)
    {
        for(i = 0; i < synth->nvoice; i++)
        {
            delete_fluid_voice(synth->voice[i]);
        }

        FLUID_FREE(synth->voice);
    }


    /* free the tunings, if any */
    if(synth->tuning != NULL)
    {
        for(i = 0; i < 128; i++)
        {
            if(synth->tuning[i] != NULL)
            {
                for(k = 0; k < 128; k++)
                {
                    delete_fluid_tuning(synth->tuning[i][k]);
                }

                FLUID_FREE(synth->tuning[i]);
            }
        }

        FLUID_FREE(synth->tuning);
    }

    fluid_private_free(synth->tuning_iter);

#ifdef LADSPA
    /* Release the LADSPA effects unit */
    delete_fluid_ladspa_fx(synth->ladspa_fx);
#endif

    /* delete all default modulators */
    delete_fluid_list_mod(synth->default_mod);

    FLUID_FREE(synth->overflow.important_channels);

    fluid_rec_mutex_destroy(synth->mutex);

    FLUID_FREE(synth);

#if defined(LIBINSTPATCH_SUPPORT)
    if(fluid_instpatch_supports_multi_init())
    {
        fluid_instpatch_deinit();
    }
#endif
}

/**
 * Get a textual representation of the last error
 * @param synth FluidSynth instance
 * @return Pointer to string of last error message.  Valid until the same
 *   calling thread calls another FluidSynth function which fails.  String is
 *   internal and should not be modified or freed.
 * @deprecated This function is not thread-safe and does not work with multiple synths.
 * It has been deprecated. It may return "" in a future release and will eventually be removed.
 */
const char *
fluid_synth_error(fluid_synth_t *synth)
{
    return "";
}

/**
 * Send a note-on event to a FluidSynth object.
 *
 * This function will take care of proper legato playing. If a note on channel @p chan is
 * already playing at the given key @p key, it will be released (even if it is sustained).
 * In other words, overlapping notes are not allowed.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param key MIDI note number (0-127)
 * @param vel MIDI velocity (0-127, 0=noteoff)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int
fluid_synth_noteon(fluid_synth_t *synth, int chan, int key, int vel)
{
    int result;
    fluid_return_val_if_fail(key >= 0 && key <= 127, FLUID_FAILED);
    fluid_return_val_if_fail(vel >= 0 && vel <= 127, FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    /* Allowed only on MIDI channel enabled */
    FLUID_API_RETURN_IF_CHAN_DISABLED(FLUID_FAILED);

    result = fluid_synth_noteon_LOCAL(synth, chan, key, vel);
    FLUID_API_RETURN(result);
}

/* Local synthesis thread variant of fluid_synth_noteon */
static int
fluid_synth_noteon_LOCAL(fluid_synth_t *synth, int chan, int key, int vel)
{
    fluid_channel_t *channel ;

    /* notes with velocity zero go to noteoff  */
    if(vel == 0)
    {
        return fluid_synth_noteoff_LOCAL(synth, chan, key);
    }

    channel = synth->channel[chan];

    /* makes sure this channel has a preset */
    if(channel->preset == NULL)
    {
        if(synth->verbose)
        {
            FLUID_LOG(FLUID_INFO, "noteon\t%d\t%d\t%d\t%05d\t%.3f\t%.3f\t%.3f\t%d\t%s",
                      chan, key, vel, 0,
                      fluid_synth_get_ticks(synth) / 44100.0f,
                      (fluid_curtime() - synth->start) / 1000.0f,
                      0.0f, 0, "channel has no preset");
        }

        return FLUID_FAILED;
    }

    if(fluid_channel_is_playing_mono(channel)) /* channel is mono or legato CC is On) */
    {
        /* play the noteOn in monophonic */
        return fluid_synth_noteon_mono_LOCAL(synth, chan, key, vel);
    }
    else
    {
        /* channel is poly and legato CC is Off) */

        /* plays the noteOn in polyphonic */
        /* Sets the note at first position in monophonic list */
        /* In the case where the musician intends to inter the channel in monophonic
        (by depressing the CC legato on), the next noteOn mono could be played legato
         with the previous note poly (if the musician choose this).
            */
        fluid_channel_set_onenote_monolist(channel, (unsigned char) key,
                                           (unsigned char) vel);

        /* If there is another voice process on the same channel and key,
           advance it to the release phase. */
        fluid_synth_release_voice_on_same_note_LOCAL(synth, chan, key);

        /* a noteon poly is passed to fluid_synth_noteon_monopoly_legato().
          This allows an opportunity to get this note played legato with a previous
          note if a CC PTC have been received before this noteon. This behavior is
          a MIDI specification (see FluidPolymono-0004.pdf chapter 4.3-a ,3.4.11
          for details).
        */
        return fluid_synth_noteon_monopoly_legato(synth, chan, INVALID_NOTE, key, vel);
    }
}

/**
 * Sends a note-off event to a FluidSynth object.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param key MIDI note number (0-127)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise (may just mean that no
 *   voices matched the note off event)
 */
int
fluid_synth_noteoff(fluid_synth_t *synth, int chan, int key)
{
    int result;
    fluid_return_val_if_fail(key >= 0 && key <= 127, FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    /* Allowed only on MIDI channel enabled */
    FLUID_API_RETURN_IF_CHAN_DISABLED(FLUID_FAILED);

    result = fluid_synth_noteoff_LOCAL(synth, chan, key);
    FLUID_API_RETURN(result);
}

/* Local synthesis thread variant of fluid_synth_noteoff */
static int
fluid_synth_noteoff_LOCAL(fluid_synth_t *synth, int chan, int key)
{
    int status;
    fluid_channel_t *channel = synth->channel[chan];

    if(fluid_channel_is_playing_mono(channel)) /* channel is mono or legato CC is On) */
    {
        /* play the noteOff in monophonic */
        status = fluid_synth_noteoff_mono_LOCAL(synth, chan, key);
    }
    else
    {
        /* channel is poly and legato CC is Off) */
        /* removes the note from the monophonic list */
        if(channel->n_notes && key == fluid_channel_last_note(channel))
        {
            fluid_channel_clear_monolist(channel);
        }

        status = fluid_synth_noteoff_monopoly(synth, chan, key, 0);
    }

    /* Changes the state (Valid/Invalid) of the most recent note played in a
       staccato manner */
    fluid_channel_invalid_prev_note_staccato(channel);
    return status;
}

/* Damps voices on a channel (turn notes off), if they're sustained by
   sustain pedal */
static int
fluid_synth_damp_voices_by_sustain_LOCAL(fluid_synth_t *synth, int chan)
{
    fluid_channel_t *channel = synth->channel[chan];
    fluid_voice_t *voice;
    int i;

    for(i = 0; i < synth->polyphony; i++)
    {
        voice = synth->voice[i];

        if((fluid_voice_get_channel(voice) == chan) && fluid_voice_is_sustained(voice))
        {
            if(voice->key == channel->key_mono_sustained)
            {
                /* key_mono_sustained is a possible mono note sustainted
                (by sustain or sostenuto pedal). It must be marked released
                (INVALID_NOTE) here because it is released only by sustain pedal */
                channel->key_mono_sustained = INVALID_NOTE;
            }

            fluid_voice_release(voice);
        }
    }

    return FLUID_OK;
}

/* Damps voices on a channel (turn notes off), if they're sustained by
   sostenuto pedal */
static int
fluid_synth_damp_voices_by_sostenuto_LOCAL(fluid_synth_t *synth, int chan)
{
    fluid_channel_t *channel = synth->channel[chan];
    fluid_voice_t *voice;
    int i;

    for(i = 0; i < synth->polyphony; i++)
    {
        voice = synth->voice[i];

        if((fluid_voice_get_channel(voice) == chan) && fluid_voice_is_sostenuto(voice))
        {
            if(voice->key == channel->key_mono_sustained)
            {
                /* key_mono_sustained is a possible mono note sustainted
                (by sustain or sostenuto pedal). It must be marked released
                (INVALID_NOTE) here because it is released only by sostenuto pedal */
                channel->key_mono_sustained = INVALID_NOTE;
            }

            fluid_voice_release(voice);
        }
    }

    return FLUID_OK;
}

/**
 * Adds the specified modulator \c mod as default modulator to the synth. \c mod will
 * take effect for any subsequently created voice.
 * @param synth FluidSynth instance
 * @param mod Modulator info (values copied, passed in object can be freed immediately afterwards)
 * @param mode Determines how to handle an existing identical modulator (#fluid_synth_add_mod)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 *
 * @note Not realtime safe (due to internal memory allocation) and therefore should not be called
 * from synthesis context at the risk of stalling audio output.
 */
int
fluid_synth_add_default_mod(fluid_synth_t *synth, const fluid_mod_t *mod, int mode)
{
    fluid_mod_t *default_mod;
    fluid_mod_t *last_mod = NULL;
    fluid_mod_t *new_mod;

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(mod != NULL, FLUID_FAILED);
    fluid_return_val_if_fail((mode == FLUID_SYNTH_ADD) || (mode == FLUID_SYNTH_OVERWRITE) , FLUID_FAILED);

    /* Checks if modulators sources are valid */
    if(!fluid_mod_check_sources(mod, "api fluid_synth_add_default_mod mod"))
    {
        return FLUID_FAILED;
    }

    fluid_synth_api_enter(synth);

    default_mod = synth->default_mod;

    while(default_mod != NULL)
    {
        if(fluid_mod_test_identity(default_mod, mod))
        {
            if(mode == FLUID_SYNTH_ADD)
            {
                default_mod->amount += mod->amount;
            }
            else // mode == FLUID_SYNTH_OVERWRITE
            {
                default_mod->amount = mod->amount;
            }

            FLUID_API_RETURN(FLUID_OK);
        }

        last_mod = default_mod;
        default_mod = default_mod->next;
    }

    /* Add a new modulator (no existing modulator to add / overwrite). */
    new_mod = new_fluid_mod();

    if(new_mod == NULL)
    {
        FLUID_API_RETURN(FLUID_FAILED);
    }

    fluid_mod_clone(new_mod, mod);
    new_mod->next = NULL;

    if(last_mod == NULL)
    {
        synth->default_mod = new_mod;
    }
    else
    {
        last_mod->next = new_mod;
    }

    FLUID_API_RETURN(FLUID_OK);
}

/**
 * Removes the specified modulator \c mod from the synth's default modulator list.
 * fluid_mod_test_identity() will be used to test modulator matching.
 * @param synth synth instance
 * @param mod The modulator to remove
 * @return #FLUID_OK if a matching modulator was found and successfully removed, #FLUID_FAILED otherwise
 *
 * @note Not realtime safe (due to internal memory freeing) and therefore should not be called
 * from synthesis context at the risk of stalling audio output.
 */
int
fluid_synth_remove_default_mod(fluid_synth_t *synth, const fluid_mod_t *mod)
{
    fluid_mod_t *default_mod;
    fluid_mod_t *last_mod;

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(mod != NULL, FLUID_FAILED);
    fluid_synth_api_enter(synth);

    last_mod = default_mod = synth->default_mod;

    while(default_mod != NULL)
    {
        if(fluid_mod_test_identity(default_mod, mod))
        {
            if(synth->default_mod == default_mod)
            {
                synth->default_mod = default_mod->next;
            }
            else
            {
                last_mod->next = default_mod->next;
            }

            delete_fluid_mod(default_mod);
            FLUID_API_RETURN(FLUID_OK);
        }

        last_mod = default_mod;
        default_mod = default_mod->next;
    }

    FLUID_API_RETURN(FLUID_FAILED);
}


/**
 * Send a MIDI controller event on a MIDI channel.
 * 
 * Most CCs are 7-bits wide in FluidSynth. There are a few exceptions which may be 14-bits wide as are documented here:
 * https://github.com/FluidSynth/fluidsynth/wiki/FluidFeatures#midi-control-change-implementation-chart
 * 
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param num MIDI controller number (0-127)
 * @param val MIDI controller value (0-127)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @note This function supports MIDI Global Controllers which will be sent to
 * all channels of the basic channel if this basic channel is in mode OmniOff/Mono.
 * This is accomplished by sending the CC one MIDI channel below the basic
 * channel of the receiver.
 * Examples: let a synthesizer with 16 MIDI channels:
 * - Let a basic channel 7 in mode 3 (Omni Off, Mono). If MIDI channel 6 is disabled it
 *    could be used as CC global for all channels belonging to basic channel 7.
 * - Let a basic channel 0 in mode 3. If MIDI channel 15  is disabled it could be used
 *   as CC global for all channels belonging to basic channel 0.
 * @warning Contrary to the MIDI Standard, this function does not clear LSB controllers,
 * when MSB controllers are received.
 */
int
fluid_synth_cc(fluid_synth_t *synth, int chan, int num, int val)
{
    int result = FLUID_FAILED;
    fluid_channel_t *channel;
    fluid_return_val_if_fail(num >= 0 && num <= 127, FLUID_FAILED);
    fluid_return_val_if_fail(val >= 0 && val <= 127, FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    channel = synth->channel[chan];

    if(channel->mode &  FLUID_CHANNEL_ENABLED)
    {
        /* chan is enabled */
        if(synth->verbose)
        {
            FLUID_LOG(FLUID_INFO, "cc\t%d\t%d\t%d", chan, num, val);
        }

        fluid_channel_set_cc(channel, num, val);
        result = fluid_synth_cc_LOCAL(synth, chan, num);
    }
    else /* chan is disabled so it is a candidate for global channel */
    {
        /* looks for next basic channel */
        int n_chan = synth->midi_channels; /* MIDI Channels number */
        int basicchan ;

        if(chan < n_chan - 1)
        {
            basicchan = chan + 1;    /* next channel */
        }
        else
        {
            basicchan = 0;    /* wrap to 0 */
        }

        channel = synth->channel[basicchan];

        /* Channel must be a basicchan in mode OMNIOFF_MONO */
        if((channel->mode &  FLUID_CHANNEL_BASIC) &&
                ((channel->mode & FLUID_CHANNEL_MODE_MASK) == FLUID_CHANNEL_MODE_OMNIOFF_MONO))
        {
            /* sends cc to all channels in this basic channel */
            int i, nbr = channel->mode_val;

            for(i = basicchan; i < basicchan + nbr; i++)
            {
                if(synth->verbose)
                {
                    FLUID_LOG(FLUID_INFO, "cc\t%d\t%d\t%d", i, num, val);
                }

                fluid_channel_set_cc(synth->channel[i], num, val);
                result = fluid_synth_cc_LOCAL(synth, i, num);
            }
        }
        /* The channel chan is not a valid 'global channel' */
        else
        {
            result = FLUID_FAILED;
        }
    }

    FLUID_API_RETURN(result);
}

/* Local synthesis thread variant of MIDI CC set function.
 Most of CC are allowed to modulate but not all. A comment describes if CC num
 isn't allowed to modulate.
 Following explanations should help to understand both MIDI specifications and
 Soundfont specifications in regard to MIDI specs.

 MIDI specs:
 CC LSB (32 to 63) are LSB contributions to CC MSB (0 to 31).
 It's up to the synthesizer to decide to take LSB values into account or not.
 Actually Fluidsynth doesn't use CC LSB value inside fluid_voice_update_param()
 (once fluid_voice_modulate() has been triggered). This is because actually
 fluidsynth needs only 7 bits resolution (and not 14 bits) from these CCs.
 So fluidsynth is using only 7 bit MSB (except for portamento time).
 In regard to MIDI specs Fluidsynth behaves correctly.

 Soundfont specs 2.01 - 8.2.1:
 To deal correctly with MIDI CC (regardless if any synth will use CC MSB alone (7 bit)
 or both CCs MSB,LSB (14 bits) during synthesis), SF specs recommend not making use of
 CC LSB (i.e only CC MSB) in modulator sources to trigger modulation (i.e modulators
 with CC LSB connected to sources inputs should be ignored).
 These specifics are particularly suited for synths that use 14 bits CCs. In this case,
 the MIDI transmitter sends CC LSB first followed by CC MSB. The MIDI synth receives
 both CC LSB and CC MSB but only CC MSB will trigger the modulation.
 This will produce correct synthesis parameters update from a correct 14 bits CC.
 If in SF specs, modulator sources with CC LSB had been accepted, both CC LSB and
 CC MSB will triggers 2 modulations. This leads to incorrect synthesis parameters
 update followed by correct synthesis parameters update.

 However, as long as fluidsynth will use only CC 7 bits resolution, it is safe to ignore
 these SF recommendations on CC receive.
*/
static int
fluid_synth_cc_LOCAL(fluid_synth_t *synth, int channum, int num)
{
    fluid_channel_t *chan = synth->channel[channum];
    int nrpn_select;
    int value;

    value = fluid_channel_get_cc(chan, num);

    switch(num)
    {
    case LOCAL_CONTROL: /* not allowed to modulate (spec SF 2.01 - 8.2.1) */
        break;

    /* CC omnioff, omnion, mono, poly */
    /* not allowed to modulate (spec SF 2.01 - 8.2.1) */
    case POLY_OFF:
    case POLY_ON:
    case OMNI_OFF:
    case OMNI_ON:

        /* allowed only if channum is a basic channel */
        if(chan->mode &  FLUID_CHANNEL_BASIC)
        {
            /* Construction of new_mode from current channel mode and this CC mode */
            int new_mode = chan->mode & FLUID_CHANNEL_MODE_MASK;

            switch(num)
            {
            case POLY_OFF:
                new_mode |= FLUID_CHANNEL_POLY_OFF;
                break;

            case POLY_ON:
                new_mode &= ~FLUID_CHANNEL_POLY_OFF;
                break;

            case OMNI_OFF:
                new_mode |= FLUID_CHANNEL_OMNI_OFF;
                break;

            case OMNI_ON:
                new_mode &= ~FLUID_CHANNEL_OMNI_OFF;
                break;

            default: /* should never happen */
                return FLUID_FAILED;
            }

            /* MIDI specs: if value is 0 it means all channels from channum to next
                basic channel minus 1 (if any) or to MIDI channel count minus 1.
                However, if value is > 0 (e.g. 4), the group of channels will be be
                limited to 4.
            	value is ignored for #FLUID_CHANNEL_MODE_OMNIOFF_POLY as this mode
                implies a group of only one channel.
            */
            /* Checks value range and changes this existing basic channel group */
            value = fluid_synth_check_next_basic_channel(synth, channum, new_mode, value);

            if(value != FLUID_FAILED)
            {
                /* reset the current basic channel before changing it */
                fluid_synth_reset_basic_channel_LOCAL(synth, channum, chan->mode_val);
                fluid_synth_set_basic_channel_LOCAL(synth, channum, new_mode, value);
                break; /* FLUID_OK */
            }
        }

        return FLUID_FAILED;

    case LEGATO_SWITCH: /* not allowed to modulate */
        /* handles Poly/mono commutation on Legato pedal On/Off.*/
        fluid_channel_cc_legato(chan, value);
        break;

    case PORTAMENTO_SWITCH: /* not allowed to modulate */
        /* Special handling of the monophonic list  */
        /* Invalids the most recent note played in a staccato manner */
        fluid_channel_invalid_prev_note_staccato(chan);
        break;

    case SUSTAIN_SWITCH: /* not allowed to modulate */

        /* Release voices if Sustain switch is released */
        if(value < 64)  /* Sustain is released */
        {
            fluid_synth_damp_voices_by_sustain_LOCAL(synth, channum);
        }

        break;

    case SOSTENUTO_SWITCH: /* not allowed to modulate */

        /* Release voices if Sostetuno switch is released */
        if(value < 64)  /* Sostenuto is released */
        {
            fluid_synth_damp_voices_by_sostenuto_LOCAL(synth, channum);
        }
        else /* Sostenuto is depressed */
            /* Update sostenuto order id when pedaling on Sostenuto */
        {
            chan->sostenuto_orderid = synth->noteid; /* future voice id value */
        }

        break;

    case BANK_SELECT_MSB: /* not allowed to modulate (spec SF 2.01 - 8.2.1) */
        fluid_channel_set_bank_msb(chan, value & 0x7F);
        break;

    case BANK_SELECT_LSB: /* not allowed to modulate (spec SF 2.01 - 8.2.1) */
        fluid_channel_set_bank_lsb(chan, value & 0x7F);
        break;

    case ALL_NOTES_OFF: /* not allowed to modulate (spec SF 2.01 - 8.2.1) */
        fluid_synth_all_notes_off_LOCAL(synth, channum);
        break;

    case ALL_SOUND_OFF: /* not allowed to modulate (spec SF 2.01 - 8.2.1) */
        fluid_synth_all_sounds_off_LOCAL(synth, channum);
        break;

    case ALL_CTRL_OFF: /* not allowed to modulate (spec SF 2.01 - 8.2.1) */
        fluid_channel_init_ctrl(chan, 1);
        // the hold pedals have been reset, we maybe need to release voices
        fluid_synth_damp_voices_by_sustain_LOCAL(synth, channum);
        fluid_synth_damp_voices_by_sostenuto_LOCAL(synth, channum);
        fluid_synth_modulate_voices_all_LOCAL(synth, channum);
        break;

    case DATA_ENTRY_LSB: /* not allowed to modulate (spec SF 2.01 - 8.2.1) */
        break;

    case DATA_ENTRY_MSB: /* not allowed to modulate (spec SF 2.01 - 8.2.1) */
    {
        int data = (value << 7) + fluid_channel_get_cc(chan, DATA_ENTRY_LSB);

        if(chan->nrpn_active)   /* NRPN is active? */
        {
            /* SontFont 2.01 NRPN Message (Sect. 9.6, p. 74)  */
            if((fluid_channel_get_cc(chan, NRPN_MSB) == 120)
                    && (fluid_channel_get_cc(chan, NRPN_LSB) < 100))
            {
                nrpn_select = chan->nrpn_select;

                if(nrpn_select < GEN_LAST)
                {
                    float val = fluid_gen_scale_nrpn(nrpn_select, data);
                    fluid_synth_set_gen_LOCAL(synth, channum, nrpn_select, val);
                }

                chan->nrpn_select = 0;  /* Reset to 0 */
            }
        }
        else if(fluid_channel_get_cc(chan, RPN_MSB) == 0)      /* RPN is active: MSB = 0? */
        {
            switch(fluid_channel_get_cc(chan, RPN_LSB))
            {
            case RPN_PITCH_BEND_RANGE:    /* Set bend range in semitones */
                fluid_channel_set_pitch_wheel_sensitivity(synth->channel[channum], value);
                fluid_synth_update_pitch_wheel_sens_LOCAL(synth, channum);    /* Update bend range */
                /* FIXME - Handle LSB? (Fine bend range in cents) */
                break;

            case RPN_CHANNEL_FINE_TUNE:   /* Fine tune is 14 bit over +/-1 semitone (+/- 100 cents, 8192 = center) */
                fluid_synth_set_gen_LOCAL(synth, channum, GEN_FINETUNE,
                                          (float)(data - 8192) * (100.0f / 8192.0f));
                break;

            case RPN_CHANNEL_COARSE_TUNE: /* Coarse tune is 7 bit and in semitones (64 is center) */
                fluid_synth_set_gen_LOCAL(synth, channum, GEN_COARSETUNE,
                                          value - 64);
                break;

            case RPN_TUNING_PROGRAM_CHANGE:
                fluid_channel_set_tuning_prog(chan, value);
                fluid_synth_activate_tuning(synth, channum,
                                            fluid_channel_get_tuning_bank(chan),
                                            value, TRUE);
                break;

            case RPN_TUNING_BANK_SELECT:
                fluid_channel_set_tuning_bank(chan, value);
                break;

            case RPN_MODULATION_DEPTH_RANGE:
                break;
            }
        }

        break;
    }

    case NRPN_MSB: /* not allowed to modulate (spec SF 2.01 - 8.2.1) */
        fluid_channel_set_cc(chan, NRPN_LSB, 0);
        chan->nrpn_select = 0;
        chan->nrpn_active = 1;
        break;

    case NRPN_LSB: /* not allowed to modulate (spec SF 2.01 - 8.2.1) */

        /* SontFont 2.01 NRPN Message (Sect. 9.6, p. 74)  */
        if(fluid_channel_get_cc(chan, NRPN_MSB) == 120)
        {
            if(value == 100)
            {
                chan->nrpn_select += 100;
            }
            else if(value == 101)
            {
                chan->nrpn_select += 1000;
            }
            else if(value == 102)
            {
                chan->nrpn_select += 10000;
            }
            else if(value < 100)
            {
                chan->nrpn_select += value;
            }
        }

        chan->nrpn_active = 1;
        break;

    case RPN_MSB: /* not allowed to modulate (spec SF 2.01 - 8.2.1) */
    case RPN_LSB: /* not allowed to modulate (spec SF 2.01 - 8.2.1) */
        chan->nrpn_active = 0;
        break;

    case BREATH_MSB:
        /* handles CC Breath On/Off noteOn/noteOff mode */
        fluid_channel_cc_breath_note_on_off(chan, value);

    /* fall-through */
    default:
        /* CC lsb shouldn't allowed to modulate (spec SF 2.01 - 8.2.1) */
        /* However, as long fluidsynth will use only CC 7 bits resolution, it
           is safe to ignore these SF recommendations on CC receive. See
           explanations above */
        /* if (! (32 <= num && num <= 63)) */
        {
            return fluid_synth_modulate_voices_LOCAL(synth, channum, 1, num);
        }
    }

    return FLUID_OK;
}

/**
 * Get current MIDI controller value on a MIDI channel.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param num MIDI controller number (0-127)
 * @param pval Location to store MIDI controller value (0-127)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int
fluid_synth_get_cc(fluid_synth_t *synth, int chan, int num, int *pval)
{
    fluid_return_val_if_fail(num >= 0 && num < 128, FLUID_FAILED);
    fluid_return_val_if_fail(pval != NULL, FLUID_FAILED);

    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    /* Allowed only on MIDI channel enabled */
    FLUID_API_RETURN_IF_CHAN_DISABLED(FLUID_FAILED);

    *pval = fluid_channel_get_cc(synth->channel[chan], num);
    FLUID_API_RETURN(FLUID_OK);
}

/*
 * Handler for synth.device-id setting.
 */
static void
fluid_synth_handle_device_id(void *data, const char *name, int value)
{
    fluid_synth_t *synth = (fluid_synth_t *)data;
    fluid_return_if_fail(synth != NULL);

    fluid_synth_api_enter(synth);
    synth->device_id = value;
    fluid_synth_api_exit(synth);
}

/**
 * Process a MIDI SYSEX (system exclusive) message.
 * @param synth FluidSynth instance
 * @param data Buffer containing SYSEX data (not including 0xF0 and 0xF7)
 * @param len Length of data in buffer
 * @param response Buffer to store response to or NULL to ignore
 * @param response_len IN/OUT parameter, in: size of response buffer, out:
 *   amount of data written to response buffer (if #FLUID_FAILED is returned and
 *   this value is non-zero, it indicates the response buffer is too small)
 * @param handled Optional location to store boolean value if message was
 *   recognized and handled or not (set to TRUE if it was handled)
 * @param dryrun TRUE to just do a dry run but not actually execute the SYSEX
 *   command (useful for checking if a SYSEX message would be handled)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @since 1.1.0
 * @note When Fluidsynth receives an XG System Mode ON message, it compares the @p synth 's deviceID
 * directly with the deviceID of the SysEx message. This is contrary to the XG spec (page 42), which
 * requires to only compare the lower nibble. However, following the XG spec seems to break drum channels
 * for a lot of MIDI files out there and therefore we've decided for this customization. If you rely on
 * XG System Mode ON messages, make sure to set the setting \ref settings_synth_device-id to match the
 * deviceID provided in the SysEx message (in most cases, this will be <code>deviceID=16</code>).
 *
 * @code
 * SYSEX format (0xF0 and 0xF7 bytes shall not be passed to this function):
 * Non-realtime:    0xF0   0x7E      <DeviceId> [BODY] 0xF7
 * Realtime:        0xF0   0x7F      <DeviceId> [BODY] 0xF7
 * Tuning messages: 0xF0   0x7E/0x7F <DeviceId> 0x08 <sub ID2> [BODY] <ChkSum> 0xF7
 * GS DT1 messages: 0xF0   0x41      <DeviceId> 0x42 0x12 [ADDRESS (3 bytes)] [DATA] <ChkSum> 0xF7
 * @endcode
 */
int
fluid_synth_sysex(fluid_synth_t *synth, const char *data, int len,
                  char *response, int *response_len, int *handled, int dryrun)
{
    int avail_response = 0;

    if(handled)
    {
        *handled = FALSE;
    }

    if(response_len)
    {
        avail_response = *response_len;
        *response_len = 0;
    }

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(data != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(len > 0, FLUID_FAILED);
    fluid_return_val_if_fail(!response || response_len, FLUID_FAILED);

    if(len < 4)
    {
        return FLUID_OK;
    }

    /* MIDI tuning SYSEX message? */
    if((data[0] == MIDI_SYSEX_UNIV_NON_REALTIME || data[0] == MIDI_SYSEX_UNIV_REALTIME)
            && (data[1] == synth->device_id || data[1] == MIDI_SYSEX_DEVICE_ID_ALL || synth->device_id == MIDI_SYSEX_DEVICE_ID_ALL)
            && data[2] == MIDI_SYSEX_MIDI_TUNING_ID)
    {
        int result;
        fluid_synth_api_enter(synth);
        result = fluid_synth_sysex_midi_tuning(synth, data, len, response,
                                               response_len, avail_response,
                                               handled, dryrun);

        FLUID_API_RETURN(result);
    }

    /* GM or GM2 system on */
    if(data[0] == MIDI_SYSEX_UNIV_NON_REALTIME
            && (data[1] == synth->device_id || data[1] == MIDI_SYSEX_DEVICE_ID_ALL || synth->device_id == MIDI_SYSEX_DEVICE_ID_ALL)
            && data[2] == MIDI_SYSEX_GM_ID)
    {
        if(handled)
        {
            *handled = TRUE;
        }
        if(!dryrun && (data[3] == MIDI_SYSEX_GM_ON
                || data[3] == MIDI_SYSEX_GM2_ON))
        {
            int result;
            synth->bank_select = FLUID_BANK_STYLE_GM;
            fluid_synth_api_enter(synth);
            result = fluid_synth_system_reset_LOCAL(synth);
            FLUID_API_RETURN(result);
        }
        return FLUID_OK;
    }

    /* GS DT1 message */
    if(data[0] == MIDI_SYSEX_MANUF_ROLAND
            && (data[1] == synth->device_id || data[1] == MIDI_SYSEX_DEVICE_ID_ALL || synth->device_id == MIDI_SYSEX_DEVICE_ID_ALL)
            && data[2] == MIDI_SYSEX_GS_ID
            && data[3] == MIDI_SYSEX_GS_DT1)
    {
        int result;
        fluid_synth_api_enter(synth);
        result = fluid_synth_sysex_gs_dt1(synth, data, len, response,
                                          response_len, avail_response,
                                          handled, dryrun);
        FLUID_API_RETURN(result);
    }

    /* XG message */
    if(data[0] == MIDI_SYSEX_MANUF_YAMAHA
            && (data[1] == synth->device_id || data[1] == MIDI_SYSEX_DEVICE_ID_ALL || synth->device_id == MIDI_SYSEX_DEVICE_ID_ALL)
            && data[2] == MIDI_SYSEX_XG_ID)
    {
        int result;
        fluid_synth_api_enter(synth);
        result = fluid_synth_sysex_xg(synth, data, len, response,
                                      response_len, avail_response,
                                      handled, dryrun);
        FLUID_API_RETURN(result);
    }

    return FLUID_OK;
}

/* Handler for MIDI tuning SYSEX messages */
static int
fluid_synth_sysex_midi_tuning(fluid_synth_t *synth, const char *data, int len,
                              char *response, int *response_len, int avail_response,
                              int *handled, int dryrun)
{
    int realtime, msgid;
    int bank = 0, prog, channels;
    double tunedata[128];
    int keys[128];
    char name[17]={0};
    int note, frac, frac2;
    uint8_t chksum;
    int i, count, index;
    const char *dataptr;
    char *resptr;;

    realtime = data[0] == MIDI_SYSEX_UNIV_REALTIME;
    msgid = data[3];

    switch(msgid)
    {
    case MIDI_SYSEX_TUNING_BULK_DUMP_REQ:
    case MIDI_SYSEX_TUNING_BULK_DUMP_REQ_BANK:
        if(data[3] == MIDI_SYSEX_TUNING_BULK_DUMP_REQ)
        {
            if(len != 5 || data[4] & 0x80 || !response)
            {
                return FLUID_OK;
            }

            *response_len = 406;
            prog = data[4];
        }
        else
        {
            if(len != 6 || data[4] & 0x80 || data[5] & 0x80 || !response)
            {
                return FLUID_OK;
            }

            *response_len = 407;
            bank = data[4];
            prog = data[5];
        }

        if(dryrun)
        {
            if(handled)
            {
                *handled = TRUE;
            }

            return FLUID_OK;
        }

        if(avail_response < *response_len)
        {
            return FLUID_FAILED;
        }

        /* Get tuning data, return if tuning not found */
        if(fluid_synth_tuning_dump(synth, bank, prog, name, 17, tunedata) == FLUID_FAILED)
        {
            *response_len = 0;
            return FLUID_OK;
        }

        resptr = response;

        *resptr++ = MIDI_SYSEX_UNIV_NON_REALTIME;
        *resptr++ = synth->device_id;
        *resptr++ = MIDI_SYSEX_MIDI_TUNING_ID;
        *resptr++ = MIDI_SYSEX_TUNING_BULK_DUMP;

        if(msgid == MIDI_SYSEX_TUNING_BULK_DUMP_REQ_BANK)
        {
            *resptr++ = bank;
        }

        *resptr++ = prog;
        /* copy 16 ASCII characters (potentially not null terminated) to the sysex buffer */
        FLUID_MEMCPY(resptr, name, 16);
        resptr += 16;

        for(i = 0; i < 128; i++)
        {
            note = tunedata[i] / 100.0;
            fluid_clip(note, 0, 127);

            frac = ((tunedata[i] - note * 100.0) * 16384.0 + 50.0) / 100.0;
            fluid_clip(frac, 0, 16383);

            *resptr++ = note;
            *resptr++ = frac >> 7;
            *resptr++ = frac & 0x7F;
        }

        if(msgid == MIDI_SYSEX_TUNING_BULK_DUMP_REQ)
        {
            /* NOTE: Checksum is not as straight forward as the bank based messages */
            chksum = MIDI_SYSEX_UNIV_NON_REALTIME ^ MIDI_SYSEX_MIDI_TUNING_ID
                     ^ MIDI_SYSEX_TUNING_BULK_DUMP ^ prog;

            for(i = 21; i < 128 * 3 + 21; i++)
            {
                chksum ^= response[i];
            }
        }
        else
        {
            for(i = 1, chksum = 0; i < 406; i++)
            {
                chksum ^= response[i];
            }
        }

        *resptr++ = chksum & 0x7F;

        if(handled)
        {
            *handled = TRUE;
        }

        break;

    case MIDI_SYSEX_TUNING_NOTE_TUNE:
    case MIDI_SYSEX_TUNING_NOTE_TUNE_BANK:
        dataptr = data + 4;

        if(msgid == MIDI_SYSEX_TUNING_NOTE_TUNE)
        {
            if(len < 10 || data[4] & 0x80 || data[5] & 0x80 || len != data[5] * 4 + 6)
            {
                return FLUID_OK;
            }
        }
        else
        {
            if(len < 11 || data[4] & 0x80 || data[5] & 0x80 || data[6] & 0x80
                    || len != data[6] * 4 + 7)
            {
                return FLUID_OK;
            }

            bank = *dataptr++;
        }

        if(dryrun)
        {
            if(handled)
            {
                *handled = TRUE;
            }

            return FLUID_OK;
        }

        prog = *dataptr++;
        count = *dataptr++;

        for(i = 0, index = 0; i < count; i++)
        {
            note = *dataptr++;

            if(note & 0x80)
            {
                return FLUID_OK;
            }

            keys[index] = note;

            note = *dataptr++;
            frac = *dataptr++;
            frac2 = *dataptr++;

            if(note & 0x80 || frac & 0x80 || frac2 & 0x80)
            {
                return FLUID_OK;
            }

            frac = frac << 7 | frac2;

            /* No change pitch value?  Doesn't really make sense to send that, but.. */
            if(note == 0x7F && frac == 16383)
            {
                continue;
            }

            tunedata[index] = note * 100.0 + (frac * 100.0 / 16384.0);
            index++;
        }

        if(index > 0)
        {
            if(fluid_synth_tune_notes(synth, bank, prog, index, keys, tunedata,
                                      realtime) == FLUID_FAILED)
            {
                return FLUID_FAILED;
            }
        }

        if(handled)
        {
            *handled = TRUE;
        }

        break;

    case MIDI_SYSEX_TUNING_OCTAVE_TUNE_1BYTE:
    case MIDI_SYSEX_TUNING_OCTAVE_TUNE_2BYTE:
        if((msgid == MIDI_SYSEX_TUNING_OCTAVE_TUNE_1BYTE && len != 19)
                || (msgid == MIDI_SYSEX_TUNING_OCTAVE_TUNE_2BYTE && len != 31))
        {
            return FLUID_OK;
        }

        if(data[4] & 0x80 || data[5] & 0x80 || data[6] & 0x80)
        {
            return FLUID_OK;
        }

        if(dryrun)
        {
            if(handled)
            {
                *handled = TRUE;
            }

            return FLUID_OK;
        }

        channels = (data[4] & 0x03) << 14 | data[5] << 7 | data[6];

        if(msgid == MIDI_SYSEX_TUNING_OCTAVE_TUNE_1BYTE)
        {
            for(i = 0; i < 12; i++)
            {
                frac = data[i + 7];

                if(frac & 0x80)
                {
                    return FLUID_OK;
                }

                tunedata[i] = (int)frac - 64;
            }
        }
        else
        {
            for(i = 0; i < 12; i++)
            {
                frac = data[i * 2 + 7];
                frac2 = data[i * 2 + 8];

                if(frac & 0x80 || frac2 & 0x80)
                {
                    return FLUID_OK;
                }

                tunedata[i] = (((int)frac << 7 | (int)frac2) - 8192) * (200.0 / 16384.0);
            }
        }

        if(fluid_synth_activate_octave_tuning(synth, 0, 0, "SYSEX",
                                              tunedata, realtime) == FLUID_FAILED)
        {
            return FLUID_FAILED;
        }

        if(channels)
        {
            for(i = 0; i < 16; i++)
            {
                if(channels & (1 << i))
                {
                    fluid_synth_activate_tuning(synth, i, 0, 0, realtime);
                }
            }
        }

        if(handled)
        {
            *handled = TRUE;
        }

        break;
    }

    return FLUID_OK;
}

/* Handler for GS DT1 messages */
static int
fluid_synth_sysex_gs_dt1(fluid_synth_t *synth, const char *data, int len,
                              char *response, int *response_len, int avail_response,
                              int *handled, int dryrun)
{
    int addr;
    int len_data;
    int checksum = 0, i;

    if(len < 9) // at least one byte of data should be transmitted
    {
        FLUID_LOG(FLUID_INFO, "SysEx DT1: message too short, dropping it.");
        return FLUID_FAILED;
    }
    len_data = len - 8;
    addr = (data[4] << 16) | (data[5] << 8) | data[6];

    for (i = 4; i < len - 1; ++i)
    {
        checksum += data[i];
    }
    checksum = 0x80 - (checksum & 0x7F);
    if (checksum != data[len - 1])
    {
        FLUID_LOG(FLUID_INFO, "SysEx DT1: dropping message on addr 0x%x due to incorrect checksum 0x%x. Correct checksum: 0x%x", addr, (int)data[len - 1], checksum);
        return FLUID_FAILED;
    }

    if (addr == 0x40007F) // Mode set
    {
        if (len_data > 1 || (data[7] != 0 && data[7] != 0x7f))
        {
            FLUID_LOG(FLUID_INFO, "SysEx DT1: dropping invalid mode set message");
            return FLUID_FAILED;
        }
        if (handled)
        {
            *handled = TRUE;
        }
        if (!dryrun)
        {
            if (data[7] == 0)
            {
                synth->bank_select = FLUID_BANK_STYLE_GS;
            }
            else
            {
                synth->bank_select = FLUID_BANK_STYLE_GM;
            }
            return fluid_synth_system_reset_LOCAL(synth);
        }
        return FLUID_OK;
    }

    if (synth->bank_select != FLUID_BANK_STYLE_GS)
    {
        return FLUID_OK; // Silently ignore all other messages
    }

    if ((addr & 0xFFF0FF) == 0x401015) // Use for rhythm part
    {
        if (len_data > 1 || data[7] > 0x02)
        {
            FLUID_LOG(FLUID_INFO, "SysEx DT1: dropping invalid rhythm part message");
            return FLUID_FAILED;
        }
        if (handled)
        {
            *handled = TRUE;
        }
        if (!dryrun)
        {
            int chan = (addr >> 8) & 0x0F;
            //See the Patch Part parameters section in SC-88Pro/8850 owner's manual
            chan = chan >= 0x0a ? chan : (chan == 0 ? 9 : chan - 1);
            synth->channel[chan]->channel_type =
                data[7] == 0x00 ? CHANNEL_TYPE_MELODIC : CHANNEL_TYPE_DRUM;

            FLUID_LOG(FLUID_DBG, "SysEx DT1: setting MIDI channel %d to type %d", chan, (int)synth->channel[chan]->channel_type);
            //Roland synths seem to "remember" the last instrument a channel
            //used in the selected mode. This behavior is not replicated here.
            fluid_synth_program_change(synth, chan, 0);
        }
        return FLUID_OK;
    }

    //silently ignore
    return FLUID_OK;
}

/* Handler for XG messages */
static int
fluid_synth_sysex_xg(fluid_synth_t *synth, const char *data, int len,
                              char *response, int *response_len, int avail_response,
                              int *handled, int dryrun)
{
    int addr;
    int len_data;

    if(len < 7) // at least one byte of data should be transmitted
    {
        return FLUID_FAILED;
    }
    len_data = len - 6;
    addr = (data[3] << 16) | (data[4] << 8) | data[5];

    if (addr == 0x00007E // Reset
            || addr == 0x00007F) // Reset to factory
    {
        if (len_data > 1 || data[6] != 0)
        {
            return FLUID_FAILED;
        }
        if (handled)
        {
            *handled = TRUE;
        }
        if (!dryrun)
        {
            synth->bank_select = FLUID_BANK_STYLE_XG;
            return fluid_synth_system_reset_LOCAL(synth);
        }
        return FLUID_OK;
    }

    /* No other messages handled yet
    if (synth->bank_select != FLUID_BANK_STYLE_XG)
    {
        return FLUID_OK; // Silently ignore all other messages
    }*/

    //silently ignore
    return FLUID_OK;
}

/**
 * Turn off all voices that are playing on the given MIDI channel, by putting them into release phase.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1), (chan=-1 selects all channels)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @since 1.1.4
 */
int
fluid_synth_all_notes_off(fluid_synth_t *synth, int chan)
{
    int result;

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(chan >= -1, FLUID_FAILED);
    fluid_synth_api_enter(synth);

    if(chan >= synth->midi_channels)
    {
        result = FLUID_FAILED;
    }
    else
    {
        /* Allowed (even for channel disabled) as chan = -1 selects all channels */
        result = fluid_synth_all_notes_off_LOCAL(synth, chan);
    }

    FLUID_API_RETURN(result);
}

/* Local synthesis thread variant of all notes off, (chan=-1 selects all channels) */
//static int
int
fluid_synth_all_notes_off_LOCAL(fluid_synth_t *synth, int chan)
{
    fluid_voice_t *voice;
    int i;

    for(i = 0; i < synth->polyphony; i++)
    {
        voice = synth->voice[i];

        if(fluid_voice_is_playing(voice) && ((-1 == chan) || (chan == fluid_voice_get_channel(voice))))
        {
            fluid_voice_noteoff(voice);
        }
    }

    return FLUID_OK;
}

/**
 * Immediately stop all voices on the given MIDI channel (skips release phase).
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1), (chan=-1 selects all channels)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @since 1.1.4
 */
int
fluid_synth_all_sounds_off(fluid_synth_t *synth, int chan)
{
    int result;

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(chan >= -1, FLUID_FAILED);
    fluid_synth_api_enter(synth);

    if(chan >= synth->midi_channels)
    {
        result = FLUID_FAILED;
    }
    else
    {
        /* Allowed (even for channel disabled) as chan = -1 selects all channels */
        result = fluid_synth_all_sounds_off_LOCAL(synth, chan);
    }

    FLUID_API_RETURN(result);
}

/* Local synthesis thread variant of all sounds off, (chan=-1 selects all channels) */
static int
fluid_synth_all_sounds_off_LOCAL(fluid_synth_t *synth, int chan)
{
    fluid_voice_t *voice;
    int i;

    for(i = 0; i < synth->polyphony; i++)
    {
        voice = synth->voice[i];

        if(fluid_voice_is_playing(voice) && ((-1 == chan) || (chan == fluid_voice_get_channel(voice))))
        {
            fluid_voice_off(voice);
        }
    }

    return FLUID_OK;
}

/**
 * Reset reverb engine
 * @param synth FluidSynth instance
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int
fluid_synth_reset_reverb(fluid_synth_t *synth)
{
    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_synth_api_enter(synth);
    fluid_synth_update_mixer(synth, fluid_rvoice_mixer_reset_reverb, 0, 0.0f);
    FLUID_API_RETURN(FLUID_OK);
}

/**
 * Reset chorus engine
 * @param synth FluidSynth instance
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int
fluid_synth_reset_chorus(fluid_synth_t *synth)
{
    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_synth_api_enter(synth);
    fluid_synth_update_mixer(synth, fluid_rvoice_mixer_reset_chorus, 0, 0.0f);
    FLUID_API_RETURN(FLUID_OK);
}


/**
 * Send MIDI system reset command (big red 'panic' button), turns off notes, resets
 * controllers and restores initial basic channel configuration.
 * @param synth FluidSynth instance
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int
fluid_synth_system_reset(fluid_synth_t *synth)
{
    int result;
    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_synth_api_enter(synth);
    result = fluid_synth_system_reset_LOCAL(synth);
    FLUID_API_RETURN(result);
}

/* Local variant of the system reset command */
static int
fluid_synth_system_reset_LOCAL(fluid_synth_t *synth)
{
    int i;

    if(synth->verbose)
    {
        FLUID_LOG(FLUID_INFO, "=== systemreset ===");
    }

    fluid_synth_all_sounds_off_LOCAL(synth, -1);

    for(i = 0; i < synth->midi_channels; i++)
    {
        fluid_channel_reset(synth->channel[i]);
    }

    /* Basic channel 0, Mode Omni On Poly */
    fluid_synth_set_basic_channel(synth, 0, FLUID_CHANNEL_MODE_OMNION_POLY,
                                  synth->midi_channels);

    fluid_synth_update_mixer(synth, fluid_rvoice_mixer_reset_reverb, 0, 0.0f);
    fluid_synth_update_mixer(synth, fluid_rvoice_mixer_reset_chorus, 0, 0.0f);

    return FLUID_OK;
}

/**
 * Update voices on a MIDI channel after a MIDI control change.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param is_cc Boolean value indicating if ctrl is a CC controller or not
 * @param ctrl MIDI controller value
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
static int
fluid_synth_modulate_voices_LOCAL(fluid_synth_t *synth, int chan, int is_cc, int ctrl)
{
    fluid_voice_t *voice;
    int i;

    for(i = 0; i < synth->polyphony; i++)
    {
        voice = synth->voice[i];

        if(fluid_voice_get_channel(voice) == chan)
        {
            fluid_voice_modulate(voice, is_cc, ctrl);
        }
    }

    return FLUID_OK;
}

/**
 * Update voices on a MIDI channel after all MIDI controllers have been changed.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
static int
fluid_synth_modulate_voices_all_LOCAL(fluid_synth_t *synth, int chan)
{
    fluid_voice_t *voice;
    int i;

    for(i = 0; i < synth->polyphony; i++)
    {
        voice = synth->voice[i];

        if(fluid_voice_get_channel(voice) == chan)
        {
            fluid_voice_modulate_all(voice);
        }
    }

    return FLUID_OK;
}

/**
 * Set the MIDI channel pressure controller value.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param val MIDI channel pressure value (0-127)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int
fluid_synth_channel_pressure(fluid_synth_t *synth, int chan, int val)
{
    int result;
    fluid_return_val_if_fail(val >= 0 && val <= 127, FLUID_FAILED);

    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    /* Allowed only on MIDI channel enabled */
    FLUID_API_RETURN_IF_CHAN_DISABLED(FLUID_FAILED);

    if(synth->verbose)
    {
        FLUID_LOG(FLUID_INFO, "channelpressure\t%d\t%d", chan, val);
    }

    fluid_channel_set_channel_pressure(synth->channel[chan], val);
    result = fluid_synth_update_channel_pressure_LOCAL(synth, chan);

    FLUID_API_RETURN(result);
}

/* Updates channel pressure from within synthesis thread */
static int
fluid_synth_update_channel_pressure_LOCAL(fluid_synth_t *synth, int chan)
{
    return fluid_synth_modulate_voices_LOCAL(synth, chan, 0, FLUID_MOD_CHANNELPRESSURE);
}

/**
 * Set the MIDI polyphonic key pressure controller value.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param key MIDI key number (0-127)
 * @param val MIDI key pressure value (0-127)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @since 2.0.0
 */
int
fluid_synth_key_pressure(fluid_synth_t *synth, int chan, int key, int val)
{
    int result;
    fluid_return_val_if_fail(key >= 0 && key <= 127, FLUID_FAILED);
    fluid_return_val_if_fail(val >= 0 && val <= 127, FLUID_FAILED);

    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    /* Allowed only on MIDI channel enabled */
    FLUID_API_RETURN_IF_CHAN_DISABLED(FLUID_FAILED);

    if(synth->verbose)
    {
        FLUID_LOG(FLUID_INFO, "keypressure\t%d\t%d\t%d", chan, key, val);
    }

    fluid_channel_set_key_pressure(synth->channel[chan], key, val);
    result = fluid_synth_update_key_pressure_LOCAL(synth, chan, key);

    FLUID_API_RETURN(result);
}

/* Updates key pressure from within synthesis thread */
static int
fluid_synth_update_key_pressure_LOCAL(fluid_synth_t *synth, int chan, int key)
{
    fluid_voice_t *voice;
    int i;
    int result = FLUID_OK;

    for(i = 0; i < synth->polyphony; i++)
    {
        voice = synth->voice[i];

        if(voice->chan == chan && voice->key == key)
        {
            result = fluid_voice_modulate(voice, 0, FLUID_MOD_KEYPRESSURE);

            if(result != FLUID_OK)
            {
                return result;
            }
        }
    }

    return result;
}

/**
 * Set the MIDI pitch bend controller value on a MIDI channel.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param val MIDI pitch bend value (0-16383 with 8192 being center)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int
fluid_synth_pitch_bend(fluid_synth_t *synth, int chan, int val)
{
    int result;
    fluid_return_val_if_fail(val >= 0 && val <= 16383, FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    /* Allowed only on MIDI channel enabled */
    FLUID_API_RETURN_IF_CHAN_DISABLED(FLUID_FAILED);

    if(synth->verbose)
    {
        FLUID_LOG(FLUID_INFO, "pitchb\t%d\t%d", chan, val);
    }

    fluid_channel_set_pitch_bend(synth->channel[chan], val);
    result = fluid_synth_update_pitch_bend_LOCAL(synth, chan);

    FLUID_API_RETURN(result);
}

/* Local synthesis thread variant of pitch bend */
static int
fluid_synth_update_pitch_bend_LOCAL(fluid_synth_t *synth, int chan)
{
    return fluid_synth_modulate_voices_LOCAL(synth, chan, 0, FLUID_MOD_PITCHWHEEL);
}

/**
 * Get the MIDI pitch bend controller value on a MIDI channel.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param ppitch_bend Location to store MIDI pitch bend value (0-16383 with
 *   8192 being center)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int
fluid_synth_get_pitch_bend(fluid_synth_t *synth, int chan, int *ppitch_bend)
{
    int result;
    fluid_return_val_if_fail(ppitch_bend != NULL, FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    /* Allowed only on MIDI channel enabled */
    FLUID_API_RETURN_IF_CHAN_DISABLED(FLUID_FAILED);

    *ppitch_bend = fluid_channel_get_pitch_bend(synth->channel[chan]);
    result = FLUID_OK;

    FLUID_API_RETURN(result);
}

/**
 * Set MIDI pitch wheel sensitivity on a MIDI channel.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param val Pitch wheel sensitivity value in semitones
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int
fluid_synth_pitch_wheel_sens(fluid_synth_t *synth, int chan, int val)
{
    int result;
    fluid_return_val_if_fail(val >= 0 && val <= 72, FLUID_FAILED);        /* 6 octaves!?  Better than no limit.. */
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    /* Allowed only on MIDI channel enabled */
    FLUID_API_RETURN_IF_CHAN_DISABLED(FLUID_FAILED);

    if(synth->verbose)
    {
        FLUID_LOG(FLUID_INFO, "pitchsens\t%d\t%d", chan, val);
    }

    fluid_channel_set_pitch_wheel_sensitivity(synth->channel[chan], val);
    result = fluid_synth_update_pitch_wheel_sens_LOCAL(synth, chan);

    FLUID_API_RETURN(result);
}

/* Local synthesis thread variant of set pitch wheel sensitivity */
static int
fluid_synth_update_pitch_wheel_sens_LOCAL(fluid_synth_t *synth, int chan)
{
    return fluid_synth_modulate_voices_LOCAL(synth, chan, 0, FLUID_MOD_PITCHWHEELSENS);
}

/**
 * Get MIDI pitch wheel sensitivity on a MIDI channel.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param pval Location to store pitch wheel sensitivity value in semitones
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @since Sometime AFTER v1.0 API freeze.
 */
int
fluid_synth_get_pitch_wheel_sens(fluid_synth_t *synth, int chan, int *pval)
{
    int result;
    fluid_return_val_if_fail(pval != NULL, FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    /* Allowed only on MIDI channel enabled */
    FLUID_API_RETURN_IF_CHAN_DISABLED(FLUID_FAILED);

    *pval = fluid_channel_get_pitch_wheel_sensitivity(synth->channel[chan]);
    result = FLUID_OK;

    FLUID_API_RETURN(result);
}

/**
 * Assign a preset to a MIDI channel.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param preset Preset to assign to channel or NULL to clear (ownership is taken over)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
static int
fluid_synth_set_preset(fluid_synth_t *synth, int chan, fluid_preset_t *preset)
{
    fluid_channel_t *channel;

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(chan >= 0 && chan < synth->midi_channels, FLUID_FAILED);

    channel = synth->channel[chan];

    return fluid_channel_set_preset(channel, preset);
}

/* Get a preset by SoundFont, bank and program numbers.
 * Returns preset pointer or NULL.
 */
static fluid_preset_t *
fluid_synth_get_preset(fluid_synth_t *synth, int sfontnum,
                       int banknum, int prognum)
{
    fluid_sfont_t *sfont;
    fluid_list_t *list;

    /* 128 indicates an "unset" operation" */
    if(prognum == FLUID_UNSET_PROGRAM)
    {
        return NULL;
    }

    for(list = synth->sfont; list; list = fluid_list_next(list))
    {
        sfont = fluid_list_get(list);

        if(fluid_sfont_get_id(sfont) == sfontnum)
        {
            return fluid_sfont_get_preset(sfont, banknum - sfont->bankofs, prognum);
        }
    }

    return NULL;
}

/* Get a preset by SoundFont name, bank and program.
 * Returns preset pointer or NULL.
 */
static fluid_preset_t *
fluid_synth_get_preset_by_sfont_name(fluid_synth_t *synth, const char *sfontname,
                                     int banknum, int prognum)
{
    fluid_sfont_t *sfont;
    fluid_list_t *list;

    for(list = synth->sfont; list; list = fluid_list_next(list))
    {
        sfont = fluid_list_get(list);

        if(FLUID_STRCMP(fluid_sfont_get_name(sfont), sfontname) == 0)
        {
            return fluid_sfont_get_preset(sfont, banknum - sfont->bankofs, prognum);
        }
    }

    return NULL;
}

/* Find a preset by bank and program numbers.
 * Returns preset pointer or NULL.
 */
fluid_preset_t *
fluid_synth_find_preset(fluid_synth_t *synth, int banknum,
                        int prognum)
{
    fluid_preset_t *preset;
    fluid_sfont_t *sfont;
    fluid_list_t *list;

    for(list = synth->sfont; list; list = fluid_list_next(list))
    {
        sfont = fluid_list_get(list);

        preset = fluid_sfont_get_preset(sfont, banknum - sfont->bankofs, prognum);

        if(preset)
        {
            return preset;
        }
    }

    return NULL;
}

/**
 * Send a program change event on a MIDI channel.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param prognum MIDI program number (0-127)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
/* As of 1.1.1 prognum can be set to 128 to unset the preset.  Not documented
 * since fluid_synth_unset_program() should be used instead. */
int
fluid_synth_program_change(fluid_synth_t *synth, int chan, int prognum)
{
    fluid_preset_t *preset = NULL;
    fluid_channel_t *channel;
    int subst_bank, subst_prog, banknum = 0, result = FLUID_FAILED;

    fluid_return_val_if_fail(prognum >= 0 && prognum <= 128, FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    /* Allowed only on MIDI channel enabled */
    FLUID_API_RETURN_IF_CHAN_DISABLED(FLUID_FAILED);

    channel = synth->channel[chan];

    if(channel->channel_type == CHANNEL_TYPE_DRUM)
    {
        banknum = DRUM_INST_BANK;
    }
    else
    {
        fluid_channel_get_sfont_bank_prog(channel, NULL, &banknum, NULL);
    }

    if(synth->verbose)
    {
        FLUID_LOG(FLUID_INFO, "prog\t%d\t%d\t%d", chan, banknum, prognum);
    }

    /* I think this is a hack for MIDI files that do bank changes in GM mode.
    * Proper way to handle this would probably be to ignore bank changes when in
    * GM mode. - JG
    * This is now possible by setting synth.midi-bank-select=gm, but let the hack
    * stay for the time being. - DH
    */
    if(prognum != FLUID_UNSET_PROGRAM)
    {
        subst_bank = banknum;
        subst_prog = prognum;

        preset = fluid_synth_find_preset(synth, subst_bank, subst_prog);

        /* Fallback to another preset if not found */
        if(!preset)
        {
            /* Percussion: Fallback to preset 0 in percussion bank */
            if(channel->channel_type == CHANNEL_TYPE_DRUM)
            {
                subst_prog = 0;
                subst_bank = DRUM_INST_BANK;
                preset = fluid_synth_find_preset(synth, subst_bank, subst_prog);
            }
            /* Melodic instrument */
            else
            {
                /* Fallback first to bank 0:prognum */
                subst_bank = 0;
                preset = fluid_synth_find_preset(synth, subst_bank, subst_prog);

                /* Fallback to first preset in bank 0 (usually piano...) */
                if(!preset)
                {
                    subst_prog = 0;
                    preset = fluid_synth_find_preset(synth, subst_bank, subst_prog);
                }
            }

            if(preset)
            {
                FLUID_LOG(FLUID_WARN, "Instrument not found on channel %d [bank=%d prog=%d], substituted [bank=%d prog=%d]",
                          chan, banknum, prognum, subst_bank, subst_prog);
            }
            else
            {
                FLUID_LOG(FLUID_WARN, "No preset found on channel %d [bank=%d prog=%d]", chan, banknum, prognum);
            }
        }
    }

    /* Assign the SoundFont ID and program number to the channel */
    fluid_channel_set_sfont_bank_prog(channel, preset ? fluid_sfont_get_id(preset->sfont) : 0,
                                      -1, prognum);
    result = fluid_synth_set_preset(synth, chan, preset);

    FLUID_API_RETURN(result);
}

/**
 * Set instrument bank number on a MIDI channel.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param bank MIDI bank number
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @note This function does not change the instrument currently assigned to \c chan,
 * as it is usually called prior to fluid_synth_program_change(). If you still want
 * instrument changes to take effect immediately, call fluid_synth_program_reset()
 * after having set up the bank configuration.
 *
 */
int
fluid_synth_bank_select(fluid_synth_t *synth, int chan, int bank)
{
    int result;
    fluid_return_val_if_fail(bank <= 16383, FLUID_FAILED);
    fluid_return_val_if_fail(bank >= 0, FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    /* Allowed only on MIDI channel enabled */
    FLUID_API_RETURN_IF_CHAN_DISABLED(FLUID_FAILED);

    fluid_channel_set_sfont_bank_prog(synth->channel[chan], -1, bank, -1);
    result = FLUID_OK;

    FLUID_API_RETURN(result);
}

/**
 * Set SoundFont ID on a MIDI channel.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param sfont_id ID of a loaded SoundFont
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @note This function does not change the instrument currently assigned to \c chan,
 * as it is usually called prior to fluid_synth_bank_select() or fluid_synth_program_change().
 * If you still want instrument changes to take effect immediately, call fluid_synth_program_reset()
 * after having selected the soundfont.
 */
int
fluid_synth_sfont_select(fluid_synth_t *synth, int chan, int sfont_id)
{
    int result;
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    /* Allowed only on MIDI channel enabled */
    FLUID_API_RETURN_IF_CHAN_DISABLED(FLUID_FAILED);

    fluid_channel_set_sfont_bank_prog(synth->channel[chan], sfont_id, -1, -1);
    result = FLUID_OK;

    FLUID_API_RETURN(result);
}

/**
 * Set the preset of a MIDI channel to an unassigned state.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @since 1.1.1
 *
 * @note Channel retains its SoundFont ID and bank numbers, while the program
 * number is set to an "unset" state.  MIDI program changes may re-assign a
 * preset if one matches.
 */
int
fluid_synth_unset_program(fluid_synth_t *synth, int chan)
{
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);
    FLUID_API_RETURN(fluid_synth_program_change(synth, chan, FLUID_UNSET_PROGRAM));
}

/**
 * Get current SoundFont ID, bank number and program number for a MIDI channel.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param sfont_id Location to store SoundFont ID
 * @param bank_num Location to store MIDI bank number
 * @param preset_num Location to store MIDI program number
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int
fluid_synth_get_program(fluid_synth_t *synth, int chan, int *sfont_id,
                        int *bank_num, int *preset_num)
{
    int result;
    fluid_channel_t *channel;

    fluid_return_val_if_fail(sfont_id != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(bank_num != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(preset_num != NULL, FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    /* Allowed only on MIDI channel enabled */
    FLUID_API_RETURN_IF_CHAN_DISABLED(FLUID_FAILED);

    channel = synth->channel[chan];
    fluid_channel_get_sfont_bank_prog(channel, sfont_id, bank_num, preset_num);

    /* 128 indicates that the preset is unset.  Set to 0 to be backwards compatible. */
    if(*preset_num == FLUID_UNSET_PROGRAM)
    {
        *preset_num = 0;
    }

    result = FLUID_OK;

    FLUID_API_RETURN(result);
}

/**
 * Select an instrument on a MIDI channel by SoundFont ID, bank and program numbers.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param sfont_id ID of a loaded SoundFont
 * @param bank_num MIDI bank number
 * @param preset_num MIDI program number
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int
fluid_synth_program_select(fluid_synth_t *synth, int chan, int sfont_id,
                           int bank_num, int preset_num)
{
    fluid_preset_t *preset = NULL;
    fluid_channel_t *channel;
    int result;
    fluid_return_val_if_fail(bank_num >= 0, FLUID_FAILED);
    fluid_return_val_if_fail(preset_num >= 0, FLUID_FAILED);

    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    /* Allowed only on MIDI channel enabled */
    FLUID_API_RETURN_IF_CHAN_DISABLED(FLUID_FAILED);

    channel = synth->channel[chan];

    preset = fluid_synth_get_preset(synth, sfont_id, bank_num, preset_num);

    if(preset == NULL)
    {
        FLUID_LOG(FLUID_ERR,
                  "There is no preset with bank number %d and preset number %d in SoundFont %d",
                  bank_num, preset_num, sfont_id);
        FLUID_API_RETURN(FLUID_FAILED);
    }

    /* Assign the new SoundFont ID, bank and program number to the channel */
    fluid_channel_set_sfont_bank_prog(channel, sfont_id, bank_num, preset_num);
    result = fluid_synth_set_preset(synth, chan, preset);

    FLUID_API_RETURN(result);
}

/**
 * Pins all samples of the given preset.
 *
 * @param synth FluidSynth instance
 * @param sfont_id ID of a loaded SoundFont
 * @param bank_num MIDI bank number
 * @param preset_num MIDI program number
 * @return #FLUID_OK if the preset was found, pinned and loaded
 * into memory successfully. #FLUID_FAILED otherwise. Note that #FLUID_OK
 * is returned, even if <code>synth.dynamic-sample-loading</code> is disabled or 
 * the preset doesn't support dynamic-sample-loading.
 *
 * This function will attempt to pin all samples of the given preset and
 * load them into memory, if they are currently unloaded. "To pin" in this
 * context means preventing them from being unloaded by an upcoming channel
 * prog change.
 *
 * @note This function is only useful if \ref settings_synth_dynamic-sample-loading is enabled.
 * By default, dynamic-sample-loading is disabled and all samples are kept in memory.
 * Furthermore, this is only useful for presets which support dynamic-sample-loading (currently,
 * only preset loaded with the default soundfont loader do).
 *
 * @since 2.2.0
 */
int
fluid_synth_pin_preset(fluid_synth_t *synth, int sfont_id, int bank_num, int preset_num)
{
    int ret;
    fluid_preset_t *preset;

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(bank_num >= 0, FLUID_FAILED);
    fluid_return_val_if_fail(preset_num >= 0, FLUID_FAILED);

    fluid_synth_api_enter(synth);

    preset = fluid_synth_get_preset(synth, sfont_id, bank_num, preset_num);

    if(preset == NULL)
    {
        FLUID_LOG(FLUID_ERR,
                  "There is no preset with bank number %d and preset number %d in SoundFont %d",
                  bank_num, preset_num, sfont_id);
        FLUID_API_RETURN(FLUID_FAILED);
    }

    ret = fluid_preset_notify(preset, FLUID_PRESET_PIN, -1); // channel unused for pinning messages

    FLUID_API_RETURN(ret);
}

/**
 * Unpin all samples of the given preset.
 *
 * @param synth FluidSynth instance
 * @param sfont_id ID of a loaded SoundFont
 * @param bank_num MIDI bank number
 * @param preset_num MIDI program number
 * @return #FLUID_OK if preset was found, #FLUID_FAILED otherwise
 *
 * This function undoes the effect of fluid_synth_pin_preset(). If the preset is
 * not currently used, its samples will be unloaded.
 *
 * @note Only useful for presets loaded with the default soundfont loader and
 * only if \ref settings_synth_dynamic-sample-loading is enabled.
 *
 * @since 2.2.0
 */
int
fluid_synth_unpin_preset(fluid_synth_t *synth, int sfont_id, int bank_num, int preset_num)
{
    int ret;
    fluid_preset_t *preset;

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(bank_num >= 0, FLUID_FAILED);
    fluid_return_val_if_fail(preset_num >= 0, FLUID_FAILED);

    fluid_synth_api_enter(synth);

    preset = fluid_synth_get_preset(synth, sfont_id, bank_num, preset_num);

    if(preset == NULL)
    {
        FLUID_LOG(FLUID_ERR,
                  "There is no preset with bank number %d and preset number %d in SoundFont %d",
                  bank_num, preset_num, sfont_id);
        FLUID_API_RETURN(FLUID_FAILED);
    }

    ret = fluid_preset_notify(preset, FLUID_PRESET_UNPIN, -1); // channel unused for pinning messages

    FLUID_API_RETURN(ret);
}

/**
 * Select an instrument on a MIDI channel by SoundFont name, bank and program numbers.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param sfont_name Name of a loaded SoundFont
 * @param bank_num MIDI bank number
 * @param preset_num MIDI program number
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @since 1.1.0
 */
int
fluid_synth_program_select_by_sfont_name(fluid_synth_t *synth, int chan,
        const char *sfont_name, int bank_num,
        int preset_num)
{
    fluid_preset_t *preset = NULL;
    fluid_channel_t *channel;
    int result;
    fluid_return_val_if_fail(sfont_name != NULL, FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    /* Allowed only on MIDI channel enabled */
    FLUID_API_RETURN_IF_CHAN_DISABLED(FLUID_FAILED);

    channel = synth->channel[chan];

    preset = fluid_synth_get_preset_by_sfont_name(synth, sfont_name, bank_num,
             preset_num);

    if(preset == NULL)
    {
        FLUID_LOG(FLUID_ERR,
                  "There is no preset with bank number %d and preset number %d in SoundFont %s",
                  bank_num, preset_num, sfont_name);
        FLUID_API_RETURN(FLUID_FAILED);
    }

    /* Assign the new SoundFont ID, bank and program number to the channel */
    fluid_channel_set_sfont_bank_prog(channel, fluid_sfont_get_id(preset->sfont),
                                      bank_num, preset_num);
    result = fluid_synth_set_preset(synth, chan, preset);

    FLUID_API_RETURN(result);
}

/*
 * This function assures that every MIDI channel has a valid preset
 * (NULL is okay). This function is called after a SoundFont is
 * unloaded or reloaded.
 */
static void
fluid_synth_update_presets(fluid_synth_t *synth)
{
    fluid_channel_t *channel;
    fluid_preset_t *preset;
    int sfont, bank, prog;
    int chan;

    for(chan = 0; chan < synth->midi_channels; chan++)
    {
        channel = synth->channel[chan];
        fluid_channel_get_sfont_bank_prog(channel, &sfont, &bank, &prog);
        preset = fluid_synth_get_preset(synth, sfont, bank, prog);
        fluid_synth_set_preset(synth, chan, preset);
    }
}

static void
fluid_synth_set_sample_rate_LOCAL(fluid_synth_t *synth, float sample_rate)
{
    int i;
    fluid_clip(sample_rate, 8000.0f, 96000.0f);
    synth->sample_rate = sample_rate;

    synth->min_note_length_ticks = fluid_synth_get_min_note_length_LOCAL(synth);

    for(i = 0; i < synth->polyphony; i++)
    {
        fluid_voice_set_output_rate(synth->voice[i], sample_rate);
    }
}

/**
 * Set up an event to change the sample-rate of the synth during the next rendering call.
 * @warning This function is broken-by-design! Don't use it! Instead, specify the sample-rate when creating the synth.
 * @deprecated As of fluidsynth 2.1.0 this function has been deprecated.
 * Changing the sample-rate is generally not considered to be a real-time use-case, as it always produces some audible artifact ("click", "pop") on the dry sound and effects (because LFOs for chorus and reverb need to be reinitialized).
 * The sample-rate change may also require memory allocation deep down in the effect units.
 * However, this memory allocation may fail and there is no way for the caller to know that, because the actual change of the sample-rate is executed during rendering.
 * This function cannot (must not) do the sample-rate change itself, otherwise the synth needs to be locked down, causing rendering to block.
 * Esp. do not use this function if this @p synth instance is used by an audio driver, because the audio driver cannot be notified by this sample-rate change.
 * Long story short: don't use it.
 * @code{.cpp}
    fluid_synth_t* synth; // assume initialized
    // [...]
    // sample-rate change needed? Delete the audio driver, if any.
    delete_fluid_audio_driver(adriver);
    // then delete the synth
    delete_fluid_synth(synth);
    // update the sample-rate
    fluid_settings_setnum(settings, "synth.sample-rate", 22050.0);
    // and re-create objects
    synth = new_fluid_synth(settings);
    adriver = new_fluid_audio_driver(settings, synth);
 * @endcode
 * @param synth FluidSynth instance
 * @param sample_rate New sample-rate (Hz)
 * @since 1.1.2
 */
void
fluid_synth_set_sample_rate(fluid_synth_t *synth, float sample_rate)
{
    fluid_return_if_fail(synth != NULL);
    fluid_synth_api_enter(synth);

    fluid_synth_set_sample_rate_LOCAL(synth, sample_rate);

    fluid_synth_update_mixer(synth, fluid_rvoice_mixer_set_samplerate,
                             0, synth->sample_rate);
    fluid_synth_api_exit(synth);
}

// internal sample rate change function for the jack driver
// executes immediately, therefore, make sure no rendering call is running!
void
fluid_synth_set_sample_rate_immediately(fluid_synth_t *synth, float sample_rate)
{
    fluid_rvoice_param_t param[MAX_EVENT_PARAMS];
    fluid_return_if_fail(synth != NULL);
    fluid_synth_api_enter(synth);
    
    fluid_synth_set_sample_rate_LOCAL(synth, sample_rate);

    param[0].i = 0;
    param[1].real = synth->sample_rate;
    fluid_rvoice_mixer_set_samplerate(synth->eventhandler->mixer, param);
    
    fluid_synth_api_exit(synth);
}


/* Handler for synth.gain setting. */
static void
fluid_synth_handle_gain(void *data, const char *name, double value)
{
    fluid_synth_t *synth = (fluid_synth_t *)data;
    fluid_synth_set_gain(synth, (float) value);
}

/**
 * Set synth output gain value.
 * @param synth FluidSynth instance
 * @param gain Gain value (function clamps value to the range 0.0 to 10.0)
 */
void
fluid_synth_set_gain(fluid_synth_t *synth, float gain)
{
    fluid_return_if_fail(synth != NULL);
    fluid_synth_api_enter(synth);

    fluid_clip(gain, 0.0f, 10.0f);

    synth->gain = gain;
    fluid_synth_update_gain_LOCAL(synth);
    fluid_synth_api_exit(synth);
}

/* Called by synthesis thread to update the gain in all voices */
static void
fluid_synth_update_gain_LOCAL(fluid_synth_t *synth)
{
    fluid_voice_t *voice;
    float gain;
    int i;

    gain = synth->gain;

    for(i = 0; i < synth->polyphony; i++)
    {
        voice = synth->voice[i];

        if(fluid_voice_is_playing(voice))
        {
            fluid_voice_set_gain(voice, gain);
        }
    }
}

/**
 * Get synth output gain value.
 * @param synth FluidSynth instance
 * @return Synth gain value (0.0 to 10.0)
 */
float
fluid_synth_get_gain(fluid_synth_t *synth)
{
    float result;
    fluid_return_val_if_fail(synth != NULL, 0.0);
    fluid_synth_api_enter(synth);

    result = synth->gain;
    FLUID_API_RETURN(result);
}

/*
 * Handler for synth.polyphony setting.
 */
static void
fluid_synth_handle_polyphony(void *data, const char *name, int value)
{
    fluid_synth_t *synth = (fluid_synth_t *)data;
    fluid_synth_set_polyphony(synth, value);
}

/**
 * Set synthesizer polyphony (max number of voices).
 * @param synth FluidSynth instance
 * @param polyphony Polyphony to assign
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @since 1.0.6
 */
int
fluid_synth_set_polyphony(fluid_synth_t *synth, int polyphony)
{
    int result;
    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(polyphony >= 1 && polyphony <= 65535, FLUID_FAILED);
    fluid_synth_api_enter(synth);

    result = fluid_synth_update_polyphony_LOCAL(synth, polyphony);

    FLUID_API_RETURN(result);
}

/* Called by synthesis thread to update the polyphony value */
static int
fluid_synth_update_polyphony_LOCAL(fluid_synth_t *synth, int new_polyphony)
{
    fluid_voice_t *voice;
    int i;

    if(new_polyphony > synth->nvoice)
    {
        /* Create more voices */
        fluid_voice_t **new_voices = FLUID_REALLOC(synth->voice,
                                     sizeof(fluid_voice_t *) * new_polyphony);

        if(new_voices == NULL)
        {
            return FLUID_FAILED;
        }

        synth->voice = new_voices;

        for(i = synth->nvoice; i < new_polyphony; i++)
        {
            synth->voice[i] = new_fluid_voice(synth->eventhandler, synth->sample_rate);

            if(synth->voice[i] == NULL)
            {
                return FLUID_FAILED;
            }

            fluid_voice_set_custom_filter(synth->voice[i], synth->custom_filter_type, synth->custom_filter_flags);
        }

        synth->nvoice = new_polyphony;
    }

    synth->polyphony = new_polyphony;

    /* turn off any voices above the new limit */
    for(i = synth->polyphony; i < synth->nvoice; i++)
    {
        voice = synth->voice[i];

        if(fluid_voice_is_playing(voice))
        {
            fluid_voice_off(voice);
        }
    }

    fluid_synth_update_mixer(synth, fluid_rvoice_mixer_set_polyphony,
                             synth->polyphony, 0.0f);

    return FLUID_OK;
}

/**
 * Get current synthesizer polyphony (max number of voices).
 * @param synth FluidSynth instance
 * @return Synth polyphony value.
 * @since 1.0.6
 */
int
fluid_synth_get_polyphony(fluid_synth_t *synth)
{
    int result;
    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_synth_api_enter(synth);

    result = synth->polyphony;
    FLUID_API_RETURN(result);
}

/**
 * @brief Get current number of active voices.
 *
 * I.e. the no. of voices that have been
 * started and have not yet finished. Unless called from synthesis context,
 * this number does not necessarily have to be equal to the number of voices
 * currently processed by the DSP loop, see below.
 * @param synth FluidSynth instance
 * @return Number of currently active voices.
 * @since 1.1.0
 *
 * @note To generate accurate continuous statistics of the voice count, caller
 * should ensure this function is called synchronously with the audio synthesis
 * process. This can be done in the new_fluid_audio_driver2() audio callback
 * function for example. Otherwise every call to this function may return different
 * voice counts as it may change after any (concurrent) call to fluid_synth_write_*() made by
 * e.g. an audio driver or the applications audio rendering thread.
 */
int
fluid_synth_get_active_voice_count(fluid_synth_t *synth)
{
    int result;
    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_synth_api_enter(synth);

    result = synth->active_voice_count;
    FLUID_API_RETURN(result);
}

/**
 * Get the internal synthesis buffer size value.
 * @param synth FluidSynth instance
 * @return Internal buffer size in audio frames.
 *
 * Audio is synthesized at this number of frames at a time. Defaults to 64 frames. I.e. the synth can only react to notes,
 * control changes, and other audio affecting events after having processed 64 audio frames.
 */
int
fluid_synth_get_internal_bufsize(fluid_synth_t *synth)
{
    return FLUID_BUFSIZE;
}

/**
 * Resend a bank select and a program change for every channel and assign corresponding instruments.
 * @param synth FluidSynth instance
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 *
 * This function is called mainly after a SoundFont has been loaded,
 * unloaded or reloaded.
 */
int
fluid_synth_program_reset(fluid_synth_t *synth)
{
    int i, prog;
    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_synth_api_enter(synth);

    /* try to set the correct presets */
    for(i = 0; i < synth->midi_channels; i++)
    {
        fluid_channel_get_sfont_bank_prog(synth->channel[i], NULL, NULL, &prog);
        fluid_synth_program_change(synth, i, prog);
    }

    FLUID_API_RETURN(FLUID_OK);
}

/**
 * Synthesize a block of floating point audio to separate audio buffers (multi-channel rendering).
 *
 * @param synth FluidSynth instance
 * @param len Count of audio frames to synthesize
 * @param left Array of float buffers to store left channel of planar audio (as many as \c synth.audio-channels buffers, each of \c len in size)
 * @param right Array of float buffers to store right channel of planar audio (size: dito)
 * @param fx_left Since 1.1.7: If not \c NULL, array of float buffers to store left effect channels (as many as \c synth.effects-channels buffers, each of \c len in size)
 * @param fx_right Since 1.1.7: If not \c NULL, array of float buffers to store right effect channels (size: dito)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 *
 * First effect channel used by reverb, second for chorus.
 *
 * @note Should only be called from synthesis thread.
 *
 * @deprecated fluid_synth_nwrite_float() is deprecated and will be removed in a future release.
 * It may continue to work or it may return #FLUID_FAILED in the future. Consider using the more
 * powerful and flexible fluid_synth_process().
 *
 * Usage example:
 * @code{.cpp}
    const int FramesToRender = 64;
    int channels;
    // retrieve number of stereo audio channels
    fluid_settings_getint(settings, "synth.audio-channels", &channels);

    // we need twice as many (mono-)buffers
    channels *= 2;

    // fluid_synth_nwrite_float renders planar audio, e.g. if synth.audio-channels==16:
    // each midi channel gets rendered to its own stereo buffer, rather than having
    // one buffer and interleaved PCM
    float** mix_buf = new float*[channels];
    for(int i = 0; i < channels; i++)
    {
        mix_buf[i] = new float[FramesToRender];
    }

    // retrieve number of (stereo) effect channels (internally hardcoded to reverb (first chan)
    // and chrous (second chan))
    fluid_settings_getint(settings, "synth.effects-channels", &channels);
    channels *= 2;

    float** fx_buf = new float*[channels];
    for(int i = 0; i < channels; i++)
    {
        fx_buf[i] = new float[FramesToRender];
    }

    float** mix_buf_l = mix_buf;
    float** mix_buf_r = &mix_buf[channels/2];

    float** fx_buf_l = fx_buf;
    float** fx_buf_r = &fx_buf[channels/2];

    fluid_synth_nwrite_float(synth, FramesToRender, mix_buf_l, mix_buf_r, fx_buf_l, fx_buf_r)
 * @endcode
 */
int
fluid_synth_nwrite_float(fluid_synth_t *synth, int len,
                         float **left, float **right,
                         float **fx_left, float **fx_right)
{
    fluid_real_t *left_in, *fx_left_in;
    fluid_real_t *right_in, *fx_right_in;
    double time = fluid_utime();
    int i, num, available, count;
#ifdef WITH_FLOAT
    int bytes;
#endif
    float cpu_load;

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(left != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(right != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(len >= 0, FLUID_FAILED);
    fluid_return_val_if_fail(len != 0, FLUID_OK); // to avoid raising FE_DIVBYZERO below

    /* First, take what's still available in the buffer */
    count = 0;
    num = synth->cur;

    if(synth->cur < FLUID_BUFSIZE)
    {
        available = FLUID_BUFSIZE - synth->cur;
        fluid_rvoice_mixer_get_bufs(synth->eventhandler->mixer, &left_in, &right_in);
        fluid_rvoice_mixer_get_fx_bufs(synth->eventhandler->mixer, &fx_left_in, &fx_right_in);

        num = (available > len) ? len : available;
#ifdef WITH_FLOAT
        bytes = num * sizeof(float);
#endif

        for(i = 0; i < synth->audio_channels; i++)
        {
#ifdef WITH_FLOAT
            FLUID_MEMCPY(left[i], &left_in[i * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT + synth->cur], bytes);
            FLUID_MEMCPY(right[i], &right_in[i * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT + synth->cur], bytes);
#else //WITH_FLOAT
            int j;

            for(j = 0; j < num; j++)
            {
                left[i][j] = (float) left_in[i * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT + j + synth->cur];
                right[i][j] = (float) right_in[i * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT + j + synth->cur];
            }

#endif //WITH_FLOAT
        }

        for(i = 0; i < synth->effects_channels; i++)
        {
#ifdef WITH_FLOAT

            if(fx_left != NULL)
            {
                FLUID_MEMCPY(fx_left[i], &fx_left_in[i * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT + synth->cur], bytes);
            }

            if(fx_right != NULL)
            {
                FLUID_MEMCPY(fx_right[i], &fx_right_in[i * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT + synth->cur], bytes);
            }

#else //WITH_FLOAT
            int j;

            if(fx_left != NULL)
            {
                for(j = 0; j < num; j++)
                {
                    fx_left[i][j] = (float) fx_left_in[i * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT + j + synth->cur];
                }
            }

            if(fx_right != NULL)
            {
                for(j = 0; j < num; j++)
                {
                    fx_right[i][j] = (float) fx_right_in[i * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT + j + synth->cur];
                }
            }

#endif //WITH_FLOAT
        }

        count += num;
        num += synth->cur; /* if we're now done, num becomes the new synth->cur below */
    }

    /* Then, run one_block() and copy till we have 'len' samples  */
    while(count < len)
    {
        fluid_rvoice_mixer_set_mix_fx(synth->eventhandler->mixer, 0);
        fluid_synth_render_blocks(synth, 1); // TODO:
        fluid_rvoice_mixer_get_bufs(synth->eventhandler->mixer, &left_in, &right_in);
        fluid_rvoice_mixer_get_fx_bufs(synth->eventhandler->mixer, &fx_left_in, &fx_right_in);

        num = (FLUID_BUFSIZE > len - count) ? len - count : FLUID_BUFSIZE;
#ifdef WITH_FLOAT
        bytes = num * sizeof(float);
#endif

        for(i = 0; i < synth->audio_channels; i++)
        {
#ifdef WITH_FLOAT
            FLUID_MEMCPY(left[i] + count, &left_in[i * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT], bytes);
            FLUID_MEMCPY(right[i] + count, &right_in[i * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT], bytes);
#else //WITH_FLOAT
            int j;

            for(j = 0; j < num; j++)
            {
                left[i][j + count] = (float) left_in[i * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT + j];
                right[i][j + count] = (float) right_in[i * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT + j];
            }

#endif //WITH_FLOAT
        }

        for(i = 0; i < synth->effects_channels; i++)
        {
#ifdef WITH_FLOAT

            if(fx_left != NULL)
            {
                FLUID_MEMCPY(fx_left[i] + count, &fx_left_in[i * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT], bytes);
            }

            if(fx_right != NULL)
            {
                FLUID_MEMCPY(fx_right[i] + count, &fx_right_in[i * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT], bytes);
            }

#else //WITH_FLOAT
            int j;

            if(fx_left != NULL)
            {
                for(j = 0; j < num; j++)
                {
                    fx_left[i][j + count] = (float) fx_left_in[i * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT + j];
                }
            }

            if(fx_right != NULL)
            {
                for(j = 0; j < num; j++)
                {
                    fx_right[i][j + count] = (float) fx_right_in[i * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT + j];
                }
            }

#endif //WITH_FLOAT
        }

        count += num;
    }

    synth->cur = num;

    time = fluid_utime() - time;
    cpu_load = 0.5 * (fluid_atomic_float_get(&synth->cpu_load) + time * synth->sample_rate / len / 10000.0);
    fluid_atomic_float_set(&synth->cpu_load, cpu_load);

    return FLUID_OK;
}

/**
 * mixes the samples of \p in to \p out
 *
 * @param out the output sample buffer to mix to
 * @param ooff sample offset in \p out
 * @param in the rvoice_mixer input sample buffer to mix from
 * @param ioff sample offset in \p in
 * @param buf_idx the sample buffer index of \p in to mix from
 * @param num number of samples to mix
 */
static FLUID_INLINE void fluid_synth_mix_single_buffer(float *FLUID_RESTRICT out,
                                                       int ooff,
                                                       const fluid_real_t *FLUID_RESTRICT in,
                                                       int ioff,
                                                       int buf_idx,
                                                       int num)
{
    if(out != NULL)
    {
        int j;

        for(j = 0; j < num; j++)
        {
            out[j + ooff] += (float) in[buf_idx * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT + j + ioff];
        }
    }
}

/**
 * Synthesize floating point audio to stereo audio channels
 * (implements the default interface #fluid_audio_func_t).
 *
 * @param synth FluidSynth instance
 *
 * @param len Count of audio frames to synthesize and store in every single buffer provided by \p out and \p fx.
 * Zero value is permitted, the function does nothing and return FLUID_OK.
 *
 * @param nfx Count of arrays in \c fx. Must be a multiple of 2 (because of stereo)
 * and in the range <code>0 <= nfx/2 <= (fluid_synth_count_effects_channels() * fluid_synth_count_effects_groups())</code>.
 * Note that zero value is valid and allows to skip mixing effects in all fx output buffers.
 *
 * @param fx Array of buffers to store effects audio to. Buffers may
 * alias with buffers of \c out. Individual NULL buffers are permitted and will cause to skip mixing any audio into that buffer.
 *
 * @param nout Count of arrays in \c out. Must be a multiple of 2
 * (because of stereo) and in the range <code>0 <= nout/2 <= fluid_synth_count_audio_channels()</code>.
 * Note that zero value is valid and allows to skip mixing dry audio in all out output buffers.
 *
 * @param out Array of buffers to store (dry) audio to. Buffers may
 * alias with buffers of \c fx. Individual NULL buffers are permitted and will cause to skip mixing any audio into that buffer.
 *
 * @return #FLUID_OK on success,
 * #FLUID_FAILED otherwise,
 *  - <code>fx == NULL</code> while <code>nfx > 0</code>, or <code>out == NULL</code> while <code>nout > 0</code>.
 *  - \c nfx or \c nout not multiple of 2.
 *  - <code>len < 0</code>.
 *  - \c nfx or \c nout exceed the range explained above.
 *
 * Synthesize and <strong>mix</strong> audio to a given number of planar audio buffers.
 * Therefore pass <code>nout = N*2</code> float buffers to \p out in order to render
 * the synthesized audio to \p N stereo channels. Each float buffer must be
 * able to hold \p len elements.
 *
 * \p out contains an array of planar buffers for normal, dry, stereo
 * audio (alternating left and right). Like:
@code{.cpp}
out[0]  = left_buffer_audio_channel_0
out[1]  = right_buffer_audio_channel_0
out[2]  = left_buffer_audio_channel_1
out[3]  = right_buffer_audio_channel_1
...
out[ (i * 2 + 0) % nout ]  = left_buffer_audio_channel_i
out[ (i * 2 + 1) % nout ]  = right_buffer_audio_channel_i
@endcode
 *
 * for zero-based channel index \p i.
 * The buffer layout of \p fx used for storing effects
 * like reverb and chorus looks similar:
@code{.cpp}
fx[0]  = left_buffer_channel_of_reverb_unit_0
fx[1]  = right_buffer_channel_of_reverb_unit_0
fx[2]  = left_buffer_channel_of_chorus_unit_0
fx[3]  = right_buffer_channel_of_chorus_unit_0
fx[4]  = left_buffer_channel_of_reverb_unit_1
fx[5]  = right_buffer_channel_of_reverb_unit_1
fx[6]  = left_buffer_channel_of_chorus_unit_1
fx[7]  = right_buffer_channel_of_chorus_unit_1
fx[8]  = left_buffer_channel_of_reverb_unit_2
...
fx[ ((k * fluid_synth_count_effects_channels() + j) * 2 + 0) % nfx ]  = left_buffer_for_effect_channel_j_of_unit_k
fx[ ((k * fluid_synth_count_effects_channels() + j) * 2 + 1) % nfx ]  = right_buffer_for_effect_channel_j_of_unit_k
@endcode
 * where <code>0 <= k < fluid_synth_count_effects_groups()</code> is a zero-based index denoting the effects unit and
 * <code>0 <= j < fluid_synth_count_effects_channels()</code> is a zero-based index denoting the effect channel within
 * unit \p k.
 *
 * Any playing voice is assigned to audio channels based on the MIDI channel it's playing on: Let \p chan be the
 * zero-based MIDI channel index an arbitrary voice is playing on. To determine the audio channel and effects unit it is
 * going to be rendered to use:
 *
 * <code>i = chan % fluid_synth_count_audio_groups()</code>
 *
 * <code>k = chan % fluid_synth_count_effects_groups()</code>
 *
 * @parblock
 * @note The owner of the sample buffers must zero them out before calling this
 * function, because any synthesized audio is mixed (i.e. added) to the buffers.
 * E.g. if fluid_synth_process() is called from a custom audio driver process function
 * (see new_fluid_audio_driver2()), the audio driver takes care of zeroing the buffers.
 * @endparblock
 *
 * @parblock
 * @note No matter how many buffers you pass in, fluid_synth_process()
 * will always render all audio channels to the
 * buffers in \c out and all effects channels to the
 * buffers in \c fx, provided that <code>nout > 0</code> and <code>nfx > 0</code> respectively. If
 * <code>nout/2 < fluid_synth_count_audio_channels()</code> it will wrap around. Same
 * is true for effects audio if <code>nfx/2 < (fluid_synth_count_effects_channels() * fluid_synth_count_effects_groups())</code>.
 * See usage examples below.
 * @endparblock
 *
 * @parblock
 * @note Should only be called from synthesis thread.
 * @endparblock
 */
int
fluid_synth_process(fluid_synth_t *synth, int len, int nfx, float *fx[],
                    int nout, float *out[])
{
    return fluid_synth_process_LOCAL(synth, len, nfx, fx, nout, out, fluid_synth_render_blocks);
}

/* declared public (instead of static) for testing purpose */
int
fluid_synth_process_LOCAL(fluid_synth_t *synth, int len, int nfx, float *fx[],
                    int nout, float *out[], int (*block_render_func)(fluid_synth_t *, int))
{
    fluid_real_t *left_in, *fx_left_in;
    fluid_real_t *right_in, *fx_right_in;
    int nfxchan, nfxunits, naudchan;

    double time = fluid_utime();
    int i, f, num, count, buffered_blocks;

    float cpu_load;

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);

    /* fx NULL while nfx > 0 is invalid */
    fluid_return_val_if_fail((fx != NULL) || (nfx == 0), FLUID_FAILED);
    /* nfx must be multiple of 2. Note that 0 value is valid and
       allows to skip mixing in fx output buffers
    */
    fluid_return_val_if_fail(nfx % 2 == 0, FLUID_FAILED);

    /* out NULL while nout > 0 is invalid */
    fluid_return_val_if_fail((out != NULL) || (nout == 0), FLUID_FAILED);
    /* nout must be multiple of 2. Note that 0 value is valid and
       allows to skip mixing in out output buffers
    */
    fluid_return_val_if_fail(nout % 2 == 0, FLUID_FAILED);

    /* check len value. Note that 0 value is valid, the function does nothing and returns FLUID_OK.
    */
    fluid_return_val_if_fail(len >= 0, FLUID_FAILED);
    fluid_return_val_if_fail(len != 0, FLUID_OK); // to avoid raising FE_DIVBYZERO below

    nfxchan = synth->effects_channels;
    nfxunits = synth->effects_groups;
    naudchan = synth->audio_channels;

    fluid_return_val_if_fail(0 <= nfx / 2 && nfx / 2 <= nfxchan * nfxunits, FLUID_FAILED);
    fluid_return_val_if_fail(0 <= nout / 2 && nout / 2 <= naudchan, FLUID_FAILED);

    /* get internal mixer audio dry buffer's pointer (left and right channel) */
    fluid_rvoice_mixer_get_bufs(synth->eventhandler->mixer, &left_in, &right_in);
    /* get internal mixer audio effect buffer's pointer (left and right channel) */
    fluid_rvoice_mixer_get_fx_bufs(synth->eventhandler->mixer, &fx_left_in, &fx_right_in);

    /* Conversely to fluid_synth_write_float(),fluid_synth_write_s16() (which handle only one
       stereo output) we don't want rendered audio effect mixed in internal audio dry buffers.
       FALSE instructs the mixer that internal audio effects will be mixed in respective internal
       audio effects buffers.
    */
    fluid_rvoice_mixer_set_mix_fx(synth->eventhandler->mixer, FALSE);


    /* First, take what's still available in the buffer */
    count = 0;
    /* synth->cur indicates if available samples are still in internal mixer buffer */
    num = synth->cur;

    buffered_blocks = (synth->cur + FLUID_BUFSIZE - 1) / FLUID_BUFSIZE;
    if(synth->cur < buffered_blocks * FLUID_BUFSIZE)
    {
        /* yes, available sample are in internal mixer buffer */
        int available = (buffered_blocks * FLUID_BUFSIZE) - synth->cur;
        num = (available > len) ? len : available;

        /* mixing dry samples (or skip if requested by the caller) */
        if(nout != 0)
        {
            for(i = 0; i < naudchan; i++)
            {
                /* mix num left samples from input mixer buffer (left_in) at input offset
                   synth->cur to output buffer (out_buf) at offset 0 */
                float *out_buf = out[(i * 2) % nout];
                fluid_synth_mix_single_buffer(out_buf, 0, left_in, synth->cur, i, num);

                /* mix num right samples from input mixer buffer (right_in) at input offset
                   synth->cur to output buffer (out_buf) at offset 0 */
                out_buf = out[(i * 2 + 1) % nout];
                fluid_synth_mix_single_buffer(out_buf, 0, right_in, synth->cur, i, num);
            }
        }

        /* mixing effects samples (or skip if requested by the caller) */
        if(nfx != 0)
        {
            // loop over all effects units
            for(f = 0; f < nfxunits; f++)
            {
                // write out all effects (i.e. reverb and chorus)
                for(i = 0; i < nfxchan; i++)
                {
                    int buf_idx = f * nfxchan + i;

                    /* mix num left samples from input mixer buffer (fx_left_in) at input offset
                       synth->cur to output buffer (out_buf) at offset 0 */
                    float *out_buf = fx[(buf_idx * 2) % nfx];
                    fluid_synth_mix_single_buffer(out_buf, 0, fx_left_in, synth->cur, buf_idx, num);

                    /* mix num right samples from input mixer buffer (fx_right_in) at input offset
                       synth->cur to output buffer (out_buf) at offset 0 */
                    out_buf = fx[(buf_idx * 2 + 1) % nfx];
                    fluid_synth_mix_single_buffer(out_buf, 0, fx_right_in, synth->cur, buf_idx, num);
                }
            }
        }

        count += num;
        num += synth->cur; /* if we're now done, num becomes the new synth->cur below */
    }

    /* Then, render blocks and copy till we have 'len' samples  */
    while(count < len)
    {
        /* always render full bloc multiple of FLUID_BUFSIZE */
        int blocksleft = (len - count + FLUID_BUFSIZE - 1) / FLUID_BUFSIZE;
        /* render audio (dry and effect) to respective internal dry and effect buffers */
        int blockcount = block_render_func(synth, blocksleft);

        num = (blockcount * FLUID_BUFSIZE > len - count) ? len - count : blockcount * FLUID_BUFSIZE;

        /* mixing dry samples (or skip if requested by the caller) */
        if(nout != 0)
        {
            for(i = 0; i < naudchan; i++)
            {
                /* mix num left samples from input mixer buffer (left_in) at input offset
                   0 to output buffer (out_buf) at offset count */
                float *out_buf = out[(i * 2) % nout];
                fluid_synth_mix_single_buffer(out_buf, count, left_in, 0, i, num);

                /* mix num right samples from input mixer buffer (right_in) at input offset
                   0 to output buffer (out_buf) at offset count */
                out_buf = out[(i * 2 + 1) % nout];
                fluid_synth_mix_single_buffer(out_buf, count, right_in, 0, i, num);
            }
        }

        /* mixing effects samples (or skip if requested by the caller) */
        if(nfx != 0)
        {
            // loop over all effects units
            for(f = 0; f < nfxunits; f++)
            {
                // write out all effects (i.e. reverb and chorus)
                for(i = 0; i < nfxchan; i++)
                {
                    int buf_idx = f * nfxchan + i;

                    /* mix num left samples from input mixer buffer (fx_left_in) at input offset
                       0 to output buffer (out_buf) at offset count */
                    float *out_buf = fx[(buf_idx * 2) % nfx];
                    fluid_synth_mix_single_buffer(out_buf, count, fx_left_in, 0, buf_idx, num);

                    /* mix num right samples from input mixer buffer (fx_right_in) at input offset
                       0 to output buffer (out_buf) at offset count */
                    out_buf = fx[(buf_idx * 2 + 1) % nfx];
                    fluid_synth_mix_single_buffer(out_buf, count, fx_right_in, 0, buf_idx, num);
                }
            }
        }

        count += num;
    }

    synth->cur = num;

    time = fluid_utime() - time;
    cpu_load = 0.5 * (fluid_atomic_float_get(&synth->cpu_load) + time * synth->sample_rate / len / 10000.0);
    fluid_atomic_float_set(&synth->cpu_load, cpu_load);

    return FLUID_OK;
}


/**
 * Synthesize a block of floating point audio samples to audio buffers.
 * @param synth FluidSynth instance
 * @param len Count of audio frames to synthesize
 * @param lout Array of floats to store left channel of audio
 * @param loff Offset index in 'lout' for first sample
 * @param lincr Increment between samples stored to 'lout'
 * @param rout Array of floats to store right channel of audio
 * @param roff Offset index in 'rout' for first sample
 * @param rincr Increment between samples stored to 'rout'
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 *
 * Useful for storing interleaved stereo (lout = rout, loff = 0, roff = 1,
 * lincr = 2, rincr = 2).
 *
 * @note Should only be called from synthesis thread.
 * @note Reverb and Chorus are mixed to \c lout resp. \c rout.
 */
int
fluid_synth_write_float(fluid_synth_t *synth, int len,
                        void *lout, int loff, int lincr,
                        void *rout, int roff, int rincr)
{
    void *channels_out[2] = {lout, rout};
    int channels_off[2] = {loff, roff };
    int channels_incr[2] = {lincr, rincr };

    return fluid_synth_write_float_channels(synth, len, 2, channels_out,
                                            channels_off, channels_incr);
}

/**
 * Synthesize a block of float audio samples channels to audio buffers.
 * The function is convenient for audio driver to render multiple stereo
 * channels pairs on multi channels audio cards (i.e 2, 4, 6, 8,.. channels).
 *
 * @param synth FluidSynth instance.
 * @param len Count of audio frames to synthesize.
 * @param channels_count Count of channels in a frame.
 *  must be multiple of 2 and  channel_count/2 must not exceed the number
 *  of internal mixer buffers (synth->audio_groups)
 * @param channels_out Array of channels_count pointers on 16 bit words to
 *  store sample channels. Modified on return.
 * @param channels_off Array of channels_count offset index to add to respective pointer
 *  in channels_out for first sample.
 * @param channels_incr Array of channels_count increment between consecutive
 *  samples channels.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 *
 * Useful for storing:
 * - interleaved channels in a unique buffer.
 * - non interleaved channels in an unique buffer (or in distinct buffers).
 *
 * Example for interleaved 4 channels (c1, c2, c3, c4) and n samples (s1, s2,..sn)
 * in a unique buffer:
 * { s1:c1, s1:c2, s1:c3, s1:c4,  s2:c1, s2:c2, s2:c3, s2:c4,...
 *   sn:c1, sn:c2, sn:c3, sn:c4 }.
 *
 * @note Should only be called from synthesis thread.
 * @note Reverb and Chorus are mixed to \c lout resp. \c rout.
 */
int
fluid_synth_write_float_channels(fluid_synth_t *synth, int len,
                                 int channels_count,
                                 void *channels_out[], int channels_off[],
                                 int channels_incr[])
{
    return fluid_synth_write_float_channels_LOCAL(synth, len, channels_count,
                                      channels_out, channels_off, channels_incr,
                                      fluid_synth_render_blocks);
}

int
fluid_synth_write_float_channels_LOCAL(fluid_synth_t *synth, int len,
                                 int channels_count,
                                 void *channels_out[], int channels_off[],
                                 int channels_incr[],
                                 int (*block_render_func)(fluid_synth_t *, int))
{
    float **chan_out = (float **)channels_out;
    int n, cur, size;

    /* pointers on first input mixer buffer */
    fluid_real_t *left_in;
    fluid_real_t *right_in;
    int bufs_in_count; /* number of stereo input buffers */
    int i;

    /* start average cpu load probe */
    double time = fluid_utime();
    float cpu_load;

    /* start profiling duration probe (if profiling is enabled) */
    fluid_profile_ref_var(prof_ref);

    /* check parameters */
    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);

    fluid_return_val_if_fail(len >= 0, FLUID_FAILED);
    fluid_return_val_if_fail(len != 0, FLUID_OK); // to avoid raising FE_DIVBYZERO below

    /* check for valid channel_count: must be multiple of 2 and
       channel_count/2 must not exceed the number of internal mixer buffers
       (synth->audio_groups)
    */
    fluid_return_val_if_fail(!(channels_count & 1)  /* must be multiple of 2 */
                             && channels_count >= 2, FLUID_FAILED);

    bufs_in_count = (unsigned int)channels_count >> 1; /* channels_count/2 */
    fluid_return_val_if_fail(bufs_in_count <= synth->audio_groups, FLUID_FAILED);

    fluid_return_val_if_fail(channels_out != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(channels_off != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(channels_incr != NULL, FLUID_FAILED);

    /* initialize output channels buffers on first sample position */
    i = channels_count;
    do
    {
        i--;
        chan_out[i] += channels_off[i];
    }
    while(i);

    /* Conversely to fluid_synth_process(),
       we want rendered audio effect mixed in internal audio dry buffers.
       TRUE instructs the mixer that internal audio effects will be mixed in internal
       audio dry buffers.
    */
    fluid_rvoice_mixer_set_mix_fx(synth->eventhandler->mixer, TRUE);

    /* get first internal mixer audio dry buffer's pointer (left and right channel) */
    fluid_rvoice_mixer_get_bufs(synth->eventhandler->mixer, &left_in, &right_in);

    size = len;

    /* synth->cur indicates if available samples are still in internal mixer buffer */
    cur = synth->cur; /* get previous sample position in internal buffer (due to prvious call) */

    do
    {
        /* fill up the buffers as needed */
        if(cur >= synth->curmax)
        {
            /* render audio (dry and effect) to internal dry buffers */
            /* always render full blocs multiple of FLUID_BUFSIZE */
            int blocksleft = (size + FLUID_BUFSIZE - 1) / FLUID_BUFSIZE;
            synth->curmax = FLUID_BUFSIZE * block_render_func(synth, blocksleft);

            /* get first internal mixer audio dry buffer's pointer (left and right channel) */
            fluid_rvoice_mixer_get_bufs(synth->eventhandler->mixer, &left_in, &right_in);
            cur = 0;
        }

        /* calculate amount of available samples */
        n = synth->curmax - cur;

        /* keep track of emitted samples */
        if(n > size)
        {
            n = size;
        }

        size -= n;

        /* update pointers to current position */
        left_in  += cur + n;
        right_in += cur + n;

        /* set final cursor position */
        cur += n;

        /* reverse index */
        n = 0 - n;

        do
        {
            i = bufs_in_count;
            do
            {
                /* input sample index in stereo buffer i */
                int in_idx = --i * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT + n;
                int c = i << 1; /* channel index c to write */

                /* write left input sample to channel sample */
                *chan_out[c] = (float) left_in[in_idx];

                /* write right input sample to next channel sample */
                *chan_out[c+1] = (float) right_in[in_idx];

                /* advance output pointers */
                chan_out[c]   += channels_incr[c];
                chan_out[c+1] += channels_incr[c+1];
            }
            while(i);
        }
        while(++n < 0);
    }
    while(size);

    synth->cur = cur; /* save current sample position. It will be used on next call */

    /* save average cpu load, use by API for real time cpu load meter */
    time = fluid_utime() - time;
    cpu_load = 0.5 * (fluid_atomic_float_get(&synth->cpu_load) + time * synth->sample_rate / len / 10000.0);
    fluid_atomic_float_set(&synth->cpu_load, cpu_load);

    /* stop duration probe and save performance measurement (if profiling is enabled) */
    fluid_profile_write(FLUID_PROF_WRITE, prof_ref,
                        fluid_rvoice_mixer_get_active_voices(synth->eventhandler->mixer),
                        len);
    return FLUID_OK;
}

/* for testing purpose */
int
fluid_synth_write_float_LOCAL(fluid_synth_t *synth, int len,
                              void *lout, int loff, int lincr,
                              void *rout, int roff, int rincr,
                              int (*block_render_func)(fluid_synth_t *, int)
                             )
{
    void *channels_out[2] = {lout, rout};
    int channels_off[2] = {loff, roff };
    int channels_incr[2] = {lincr, rincr };

    return fluid_synth_write_float_channels_LOCAL(synth, len, 2, channels_out,
                                            channels_off, channels_incr,
                                            block_render_func);
}


#define DITHER_SIZE 48000
#define DITHER_CHANNELS 2

static float rand_table[DITHER_CHANNELS][DITHER_SIZE];

/* Init dither table */
static void
init_dither(void)
{
    float d, dp;
    int c, i;

    for(c = 0; c < DITHER_CHANNELS; c++)
    {
        dp = 0;

        for(i = 0; i < DITHER_SIZE - 1; i++)
        {
            d = rand() / (float)RAND_MAX - 0.5f;
            rand_table[c][i] = d - dp;
            dp = d;
        }

        rand_table[c][DITHER_SIZE - 1] = 0 - dp;
    }
}

/* A portable replacement for roundf(), seems it may actually be faster too! */
static FLUID_INLINE int16_t
round_clip_to_i16(float x)
{
    long i;

    if(x >= 0.0f)
    {
        i = (long)(x + 0.5f);

        if(FLUID_UNLIKELY(i > 32767))
        {
            i = 32767;
        }
    }
    else
    {
        i = (long)(x - 0.5f);

        if(FLUID_UNLIKELY(i < -32768))
        {
            i = -32768;
        }
    }

    return (int16_t)i;
}

/**
 * Synthesize a block of 16 bit audio samples to audio buffers.
 * @param synth FluidSynth instance
 * @param len Count of audio frames to synthesize
 * @param lout Array of 16 bit words to store left channel of audio
 * @param loff Offset index in 'lout' for first sample
 * @param lincr Increment between samples stored to 'lout'
 * @param rout Array of 16 bit words to store right channel of audio
 * @param roff Offset index in 'rout' for first sample
 * @param rincr Increment between samples stored to 'rout'
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 *
 * Useful for storing interleaved stereo (lout = rout, loff = 0, roff = 1,
 * lincr = 2, rincr = 2).
 *
 * @note Should only be called from synthesis thread.
 * @note Reverb and Chorus are mixed to \c lout resp. \c rout.
 * @note Dithering is performed when converting from internal floating point to
 * 16 bit audio.
 */
int
fluid_synth_write_s16(fluid_synth_t *synth, int len,
                      void *lout, int loff, int lincr,
                      void *rout, int roff, int rincr)
{
    void *channels_out[2] = {lout, rout};
    int channels_off[2] = {loff, roff };
    int channels_incr[2] = {lincr, rincr };

    return fluid_synth_write_s16_channels(synth, len, 2, channels_out,
                                          channels_off, channels_incr);
}

/**
 * Synthesize a block of 16 bit audio samples channels to audio buffers.
 * The function is convenient for audio driver to render multiple stereo
 * channels pairs on multi channels audio cards (i.e 2, 4, 6, 8,.. channels).
 *
 * @param synth FluidSynth instance.
 * @param len Count of audio frames to synthesize.
 * @param channels_count Count of channels in a frame.
 *  must be multiple of 2 and  channel_count/2 must not exceed the number
 *  of internal mixer buffers (synth->audio_groups)
 * @param channels_out Array of channels_count pointers on 16 bit words to
 *  store sample channels. Modified on return.
 * @param channels_off Array of channels_count offset index to add to respective pointer
 *  in channels_out for first sample.
 * @param channels_incr Array of channels_count increment between consecutive
 *  samples channels.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 *
 * Useful for storing:
 * - interleaved channels in a unique buffer.
 * - non interleaved channels in an unique buffer (or in distinct buffers).
 *
 * Example for interleaved 4 channels (c1, c2, c3, c4) and n samples (s1, s2,..sn)
 * in a unique buffer:
 * { s1:c1, s1:c2, s1:c3, s1:c4,  s2:c1, s2:c2, s2:c3, s2:c4, ....
 *   sn:c1, sn:c2, sn:c3, sn:c4 }.
 *
 * @note Should only be called from synthesis thread.
 * @note Reverb and Chorus are mixed to \c lout resp. \c rout.
 * @note Dithering is performed when converting from internal floating point to
 * 16 bit audio.
 */
int
fluid_synth_write_s16_channels(fluid_synth_t *synth, int len,
                               int channels_count,
                               void *channels_out[], int channels_off[],
                               int channels_incr[])
{
    int16_t **chan_out = (int16_t **)channels_out;
    int di, n, cur, size;

    /* pointers on first input mixer buffer */
    fluid_real_t *left_in;
    fluid_real_t *right_in;
    int bufs_in_count; /* number of stereo input buffers */
    int i;

    /* start average cpu load probe */
    double time = fluid_utime();
    float cpu_load;

    /* start profiling duration probe (if profiling is enabled) */
    fluid_profile_ref_var(prof_ref);

    /* check parameters */
    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);

    fluid_return_val_if_fail(len >= 0, FLUID_FAILED);
    fluid_return_val_if_fail(len != 0, FLUID_OK); // to avoid raising FE_DIVBYZERO below

    /* check for valid channel_count: must be multiple of 2 and
       channel_count/2 must not exceed the number of internal mixer buffers
       (synth->audio_groups)
    */
    fluid_return_val_if_fail(!(channels_count & 1)  /* must be multiple of 2 */
                             && channels_count >= 2, FLUID_FAILED);

    bufs_in_count = (unsigned int)channels_count >> 1; /* channels_count/2 */
    fluid_return_val_if_fail(bufs_in_count <= synth->audio_groups, FLUID_FAILED);

    fluid_return_val_if_fail(channels_out != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(channels_off != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(channels_incr != NULL, FLUID_FAILED);

    /* initialize output channels buffers on first sample position */
    i = channels_count;
    do
    {
        i--;
        chan_out[i] += channels_off[i];
    }
    while(i);

    /* Conversely to fluid_synth_process(),
       we want rendered audio effect mixed in internal audio dry buffers.
       TRUE instructs the mixer that internal audio effects will be mixed in internal
       audio dry buffers.
    */
    fluid_rvoice_mixer_set_mix_fx(synth->eventhandler->mixer, TRUE);
    /* get first internal mixer audio dry buffer's pointer (left and right channel) */
    fluid_rvoice_mixer_get_bufs(synth->eventhandler->mixer, &left_in, &right_in);

    size = len;
    /* synth->cur indicates if available samples are still in internal mixer buffer */
    cur = synth->cur; /* get previous sample position in internal buffer (due to prvious call) */
    di = synth->dither_index;

    do
    {
        /* fill up the buffers as needed */
        if(cur >= synth->curmax)
        {
            /* render audio (dry and effect) to internal dry buffers */
            /* always render full blocs multiple of FLUID_BUFSIZE */
            int blocksleft = (size + FLUID_BUFSIZE - 1) / FLUID_BUFSIZE;
            synth->curmax = FLUID_BUFSIZE * fluid_synth_render_blocks(synth, blocksleft);

            /* get first internal mixer audio dry buffer's pointer (left and right channel) */
            fluid_rvoice_mixer_get_bufs(synth->eventhandler->mixer, &left_in, &right_in);
            cur = 0;
        }

        /* calculate amount of available samples */
        n = synth->curmax - cur;

        /* keep track of emitted samples */
        if(n > size)
        {
            n = size;
        }

        size -= n;

        /* update pointers to current position */
        left_in  += cur + n;
        right_in += cur + n;

        /* set final cursor position */
        cur += n;

        /* reverse index */
        n = 0 - n;

        do
        {
            i = bufs_in_count;
            do
            {
                /* input sample index in stereo buffer i */
                int in_idx = --i * FLUID_BUFSIZE * FLUID_MIXER_MAX_BUFFERS_DEFAULT + n;
                int c = i << 1; /* channel index c to write */

                /* write left input sample to channel sample */
                *chan_out[c] = round_clip_to_i16(left_in[in_idx] * 32766.0f +
                                                 rand_table[0][di]);

                /* write right input sample to next channel sample */
                *chan_out[c+1] = round_clip_to_i16(right_in[in_idx] * 32766.0f +
                                                   rand_table[1][di]);
                /* advance output pointers */
                chan_out[c]   += channels_incr[c];
                chan_out[c+1] += channels_incr[c+1];
            }
            while(i);

            if(++di >= DITHER_SIZE)
            {
                di = 0;
            }
        }
        while(++n < 0);
    }
    while(size);

    synth->cur = cur; /* save current sample position. It will be used on next call */
    synth->dither_index = di;	/* keep dither buffer continuous */

    /* save average cpu load, used by API for real time cpu load meter */
    time = fluid_utime() - time;
    cpu_load = 0.5 * (fluid_atomic_float_get(&synth->cpu_load) + time * synth->sample_rate / len / 10000.0);
    fluid_atomic_float_set(&synth->cpu_load, cpu_load);

    /* stop duration probe and save performance measurement (if profiling is enabled) */
    fluid_profile_write(FLUID_PROF_WRITE, prof_ref,
                        fluid_rvoice_mixer_get_active_voices(synth->eventhandler->mixer),
                        len);
    return FLUID_OK;
}

/**
 * Converts stereo floating point sample data to signed 16 bit data with dithering.
 * @param dither_index Pointer to an integer which should be initialized to 0
 *   before the first call and passed unmodified to additional calls which are
 *   part of the same synthesis output.
 * @param len Length in frames to convert
 * @param lin Buffer of left audio samples to convert from
 * @param rin Buffer of right audio samples to convert from
 * @param lout Array of 16 bit words to store left channel of audio
 * @param loff Offset index in 'lout' for first sample
 * @param lincr Increment between samples stored to 'lout'
 * @param rout Array of 16 bit words to store right channel of audio
 * @param roff Offset index in 'rout' for first sample
 * @param rincr Increment between samples stored to 'rout'
 *
 * @note Currently private to libfluidsynth.
 */
void
fluid_synth_dither_s16(int *dither_index, int len, const float *lin, const float *rin,
                       void *lout, int loff, int lincr,
                       void *rout, int roff, int rincr)
{
    int i, j, k;
    int16_t *left_out = lout;
    int16_t *right_out = rout;
    int di = *dither_index;
    fluid_profile_ref_var(prof_ref);

    for(i = 0, j = loff, k = roff; i < len; i++, j += lincr, k += rincr)
    {
        left_out[j] = round_clip_to_i16(lin[i] * 32766.0f + rand_table[0][di]);
        right_out[k] = round_clip_to_i16(rin[i] * 32766.0f + rand_table[1][di]);

        if(++di >= DITHER_SIZE)
        {
            di = 0;
        }
    }

    *dither_index = di;	/* keep dither buffer continuous */

    fluid_profile(FLUID_PROF_WRITE, prof_ref, 0, len);
}

static void
fluid_synth_check_finished_voices(fluid_synth_t *synth)
{
    int j;
    fluid_rvoice_t *fv;

    while(NULL != (fv = fluid_rvoice_eventhandler_get_finished_voice(synth->eventhandler)))
    {
        for(j = 0; j < synth->polyphony; j++)
        {
            if(synth->voice[j]->rvoice == fv)
            {
                fluid_voice_unlock_rvoice(synth->voice[j]);
                fluid_voice_stop(synth->voice[j]);
                break;
            }
            else if(synth->voice[j]->overflow_rvoice == fv)
            {
                /* Unlock the overflow_rvoice of the voice.
                   Decrement the reference count of the sample owned by this
                   rvoice.
                */
                fluid_voice_overflow_rvoice_finished(synth->voice[j]);

                /* Decrement synth active voice count. Must not be incorporated
                   in fluid_voice_overflow_rvoice_finished() because
                   fluid_voice_overflow_rvoice_finished() is called also
                   at synth destruction and in this case the variable should be
                   accessed via voice->channel->synth->active_voice_count.
                   And for certain voices which are not playing, the field
                   voice->channel is NULL.
                */
                synth->active_voice_count--;
                break;
            }
        }
    }
}

/**
 * Process all waiting events in the rvoice queue.
 * Make sure no (other) rendering is running in parallel when
 * you call this function!
 */
void fluid_synth_process_event_queue(fluid_synth_t *synth)
{
    fluid_rvoice_eventhandler_dispatch_all(synth->eventhandler);
}


/**
 * Process blocks (FLUID_BUFSIZE) of audio.
 * Must be called from renderer thread only!
 * @return number of blocks rendered. Might (often) return less than requested
 */
static int
fluid_synth_render_blocks(fluid_synth_t *synth, int blockcount)
{
    int i, maxblocks;
    fluid_profile_ref_var(prof_ref);

    /* Assign ID of synthesis thread */
//  synth->synth_thread_id = fluid_thread_get_id ();

    fluid_check_fpe("??? Just starting up ???");

    fluid_rvoice_eventhandler_dispatch_all(synth->eventhandler);

    /* do not render more blocks than we can store internally */
    maxblocks = fluid_rvoice_mixer_get_bufcount(synth->eventhandler->mixer);

    if(blockcount > maxblocks)
    {
        blockcount = maxblocks;
    }

    for(i = 0; i < blockcount; i++)
    {
        fluid_sample_timer_process(synth);
        fluid_synth_add_ticks(synth, FLUID_BUFSIZE);

        /* If events have been queued waiting for fluid_rvoice_eventhandler_dispatch_all()
         * (should only happen with parallel render) stop processing and go for rendering
         */
        if(fluid_rvoice_eventhandler_dispatch_count(synth->eventhandler))
        {
            // Something has happened, we can't process more
            blockcount = i + 1;
            break;
        }
    }

    fluid_check_fpe("fluid_sample_timer_process");

    blockcount = fluid_rvoice_mixer_render(synth->eventhandler->mixer, blockcount);

    /* Testcase, that provokes a denormal floating point error */
#if 0
    {
        float num = 1;

        while(num != 0)
        {
            num *= 0.5;
        };
    };
#endif
    fluid_check_fpe("??? Remainder of synth_one_block ???");
    fluid_profile(FLUID_PROF_ONE_BLOCK, prof_ref,
                  fluid_rvoice_mixer_get_active_voices(synth->eventhandler->mixer),
                  blockcount * FLUID_BUFSIZE);
    return blockcount;
}

/*
 * Handler for synth.reverb.* and synth.chorus.* double settings.
 */
static void fluid_synth_handle_reverb_chorus_num(void *data, const char *name, double value)
{
    fluid_synth_t *synth = (fluid_synth_t *)data;
    fluid_return_if_fail(synth != NULL);

    if(FLUID_STRCMP(name, "synth.reverb.room-size") == 0)
    {
        fluid_synth_reverb_set_param(synth, -1, FLUID_REVERB_ROOMSIZE, value);
    }
    else if(FLUID_STRCMP(name, "synth.reverb.damp") == 0)
    {
        fluid_synth_reverb_set_param(synth, -1, FLUID_REVERB_DAMP, value);
    }
    else if(FLUID_STRCMP(name, "synth.reverb.width") == 0)
    {
        fluid_synth_reverb_set_param(synth, -1, FLUID_REVERB_WIDTH, value);
    }
    else if(FLUID_STRCMP(name, "synth.reverb.level") == 0)
    {
        fluid_synth_reverb_set_param(synth, -1, FLUID_REVERB_LEVEL, value);
    }
    else if(FLUID_STRCMP(name, "synth.chorus.depth") == 0)
    {
        fluid_synth_chorus_set_param(synth, -1, FLUID_CHORUS_DEPTH, value);
    }
    else if(FLUID_STRCMP(name, "synth.chorus.speed") == 0)
    {
        fluid_synth_chorus_set_param(synth, -1, FLUID_CHORUS_SPEED, value);
    }
    else if(FLUID_STRCMP(name, "synth.chorus.level") == 0)
    {
        fluid_synth_chorus_set_param(synth, -1, FLUID_CHORUS_LEVEL, value);
    }
}

/*
 * Handler for synth.reverb.* and synth.chorus.* integer settings.
 */
static void fluid_synth_handle_reverb_chorus_int(void *data, const char *name, int value)
{
    fluid_synth_t *synth = (fluid_synth_t *)data;
    fluid_return_if_fail(synth != NULL);

    if(FLUID_STRCMP(name, "synth.reverb.active") == 0)
    {
        fluid_synth_reverb_on(synth, -1, value);
    }
    else if(FLUID_STRCMP(name, "synth.chorus.active") == 0)
    {
        fluid_synth_chorus_on(synth, -1, value);
    }
    else if(FLUID_STRCMP(name, "synth.chorus.nr") == 0)
    {
		fluid_synth_chorus_set_param(synth, -1, FLUID_CHORUS_NR, (double)value);
    }
}

/*
 * Handler for synth.overflow.* settings.
 */
static void fluid_synth_handle_overflow(void *data, const char *name, double value)
{
    fluid_synth_t *synth = (fluid_synth_t *)data;
    fluid_return_if_fail(synth != NULL);

    fluid_synth_api_enter(synth);

    if(FLUID_STRCMP(name, "synth.overflow.percussion") == 0)
    {
        synth->overflow.percussion = value;
    }
    else if(FLUID_STRCMP(name, "synth.overflow.released") == 0)
    {
        synth->overflow.released = value;
    }
    else if(FLUID_STRCMP(name, "synth.overflow.sustained") == 0)
    {
        synth->overflow.sustained = value;
    }
    else if(FLUID_STRCMP(name, "synth.overflow.volume") == 0)
    {
        synth->overflow.volume = value;
    }
    else if(FLUID_STRCMP(name, "synth.overflow.age") == 0)
    {
        synth->overflow.age = value;
    }
    else if(FLUID_STRCMP(name, "synth.overflow.important") == 0)
    {
        synth->overflow.important = value;
    }

    fluid_synth_api_exit(synth);
}

/* Selects a voice for killing. */
static fluid_voice_t *
fluid_synth_free_voice_by_kill_LOCAL(fluid_synth_t *synth)
{
    int i;
    float best_prio = OVERFLOW_PRIO_CANNOT_KILL - 1;
    float this_voice_prio;
    fluid_voice_t *voice;
    int best_voice_index = -1;
    unsigned int ticks = fluid_synth_get_ticks(synth);

    for(i = 0; i < synth->polyphony; i++)
    {

        voice = synth->voice[i];

        /* safeguard against an available voice. */
        if(_AVAILABLE(voice))
        {
            return voice;
        }

        this_voice_prio = fluid_voice_get_overflow_prio(voice, &synth->overflow,
                          ticks);

        /* check if this voice has less priority than the previous candidate. */
        if(this_voice_prio < best_prio)
        {
            best_voice_index = i;
            best_prio = this_voice_prio;
        }
    }

    if(best_voice_index < 0)
    {
        return NULL;
    }

    voice = synth->voice[best_voice_index];
    FLUID_LOG(FLUID_DBG, "Killing voice %d, index %d, chan %d, key %d ",
              fluid_voice_get_id(voice), best_voice_index, fluid_voice_get_channel(voice), fluid_voice_get_key(voice));
    fluid_voice_off(voice);

    return voice;
}


/**
 * Allocate a synthesis voice.
 * @param synth FluidSynth instance
 * @param sample Sample to assign to the voice
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param key MIDI note number for the voice (0-127)
 * @param vel MIDI velocity for the voice (0-127)
 * @return Allocated synthesis voice or NULL on error
 *
 * This function is called by a SoundFont's preset in response to a noteon event.
 * The returned voice comes with default modulators and generators.
 * A single noteon event may create any number of voices, when the preset is layered.
 *
 * @note Should only be called from within synthesis thread, which includes
 * SoundFont loader preset noteon method.
 */
fluid_voice_t *
fluid_synth_alloc_voice(fluid_synth_t *synth, fluid_sample_t *sample,
                        int chan, int key, int vel)
{
    fluid_return_val_if_fail(sample != NULL, NULL);
    fluid_return_val_if_fail(sample->data != NULL, NULL);
    FLUID_API_ENTRY_CHAN(NULL);
    FLUID_API_RETURN(fluid_synth_alloc_voice_LOCAL(synth, sample, chan, key, vel, NULL));

}

fluid_voice_t *
fluid_synth_alloc_voice_LOCAL(fluid_synth_t *synth, fluid_sample_t *sample, int chan, int key, int vel, fluid_zone_range_t *zone_range)
{
    int i, k;
    fluid_voice_t *voice = NULL;
    fluid_channel_t *channel = NULL;
    unsigned int ticks;

    /* check if there's an available synthesis process */
    for(i = 0; i < synth->polyphony; i++)
    {
        if(_AVAILABLE(synth->voice[i]))
        {
            voice = synth->voice[i];
            break;
        }
    }

    /* No success yet? Then stop a running voice. */
    if(voice == NULL)
    {
        FLUID_LOG(FLUID_DBG, "Polyphony exceeded, trying to kill a voice");
        voice = fluid_synth_free_voice_by_kill_LOCAL(synth);
    }

    if(voice == NULL)
    {
        FLUID_LOG(FLUID_WARN, "Failed to allocate a synthesis process. (chan=%d,key=%d)", chan, key);
        return NULL;
    }

    ticks = fluid_synth_get_ticks(synth);

    if(synth->verbose)
    {
        k = 0;

        for(i = 0; i < synth->polyphony; i++)
        {
            if(!_AVAILABLE(synth->voice[i]))
            {
                k++;
            }
        }

        FLUID_LOG(FLUID_INFO, "noteon\t%d\t%d\t%d\t%05d\t%.3f\t%.3f\t%.3f\t%d",
                  chan, key, vel, synth->storeid,
                  (float) ticks / 44100.0f,
                  (fluid_curtime() - synth->start) / 1000.0f,
                  0.0f,
                  k);
    }

    channel = synth->channel[chan];

    if(fluid_voice_init(voice, sample, zone_range, channel, key, vel,
                        synth->storeid, ticks, synth->gain) != FLUID_OK)
    {
        FLUID_LOG(FLUID_WARN, "Failed to initialize voice");
        return NULL;
    }

    /* add the default modulators to the synthesis process. */
    /* custom_breath2att_modulator is not a default modulator specified in SF
      it is intended to replace default_vel2att_mod for this channel on demand using
      API fluid_synth_set_breath_mode() or shell command setbreathmode for this channel.
    */
    {
        int mono = fluid_channel_is_playing_mono(channel);
        fluid_mod_t *default_mod = synth->default_mod;

        while(default_mod != NULL)
        {
            if(
                /* See if default_mod is the velocity_to_attenuation modulator */
                fluid_mod_test_identity(default_mod, &default_vel2att_mod) &&
                // See if a replacement by custom_breath2att_modulator has been demanded
                // for this channel
                ((!mono && (channel->mode &  FLUID_CHANNEL_BREATH_POLY)) ||
                 (mono && (channel->mode &  FLUID_CHANNEL_BREATH_MONO)))
            )
            {
                // Replacement of default_vel2att modulator by custom_breath2att_modulator
                fluid_voice_add_mod_local(voice, &custom_breath2att_mod, FLUID_VOICE_DEFAULT, 0);
            }
            else
            {
                fluid_voice_add_mod_local(voice, default_mod, FLUID_VOICE_DEFAULT, 0);
            }

            // Next default modulator to add to the voice
            default_mod = default_mod->next;
        }
    }

    return voice;
}

/* Kill all voices on a given channel, which have the same exclusive class
 * generator as new_voice.
 */
static void
fluid_synth_kill_by_exclusive_class_LOCAL(fluid_synth_t *synth,
        fluid_voice_t *new_voice)
{
    int excl_class = fluid_voice_gen_value(new_voice, GEN_EXCLUSIVECLASS);
    int i;

    /* Excl. class 0: No exclusive class */
    if(excl_class == 0)
    {
        return;
    }

    /* Kill all notes on the same channel with the same exclusive class */
    for(i = 0; i < synth->polyphony; i++)
    {
        fluid_voice_t *existing_voice = synth->voice[i];

        /* If voice is playing, on the same channel, has same exclusive
         * class and is not part of the same noteon event (voice group), then kill it */

        if(fluid_voice_is_playing(existing_voice)
                && fluid_voice_get_channel(existing_voice) == fluid_voice_get_channel(new_voice)
                && fluid_voice_gen_value(existing_voice, GEN_EXCLUSIVECLASS) == excl_class
                && fluid_voice_get_id(existing_voice) != fluid_voice_get_id(new_voice))
        {
            fluid_voice_kill_excl(existing_voice);
        }
    }
}

/**
 * Activate a voice previously allocated with fluid_synth_alloc_voice().
 * @param synth FluidSynth instance
 * @param voice Voice to activate
 *
 * This function is called by a SoundFont's preset in response to a noteon
 * event.  Exclusive classes are processed here.
 *
 * @note Should only be called from within synthesis thread, which includes
 * SoundFont loader preset noteon method.
 */
void
fluid_synth_start_voice(fluid_synth_t *synth, fluid_voice_t *voice)
{
    fluid_return_if_fail(synth != NULL);
    fluid_return_if_fail(voice != NULL);
//  fluid_return_if_fail (fluid_synth_is_synth_thread (synth));
    fluid_synth_api_enter(synth);

    /* Find the exclusive class of this voice. If set, kill all voices
     * that match the exclusive class and are younger than the first
     * voice process created by this noteon event. */
    fluid_synth_kill_by_exclusive_class_LOCAL(synth, voice);

    fluid_voice_start(voice);     /* Start the new voice */
    fluid_voice_lock_rvoice(voice);
    fluid_rvoice_eventhandler_add_rvoice(synth->eventhandler, voice->rvoice);
    fluid_synth_api_exit(synth);
}

/**
 * Add a SoundFont loader to the synth. This function takes ownership of \c loader
 * and frees it automatically upon \c synth destruction.
 * @param synth FluidSynth instance
 * @param loader Loader API structure
 *
 * SoundFont loaders are used to add custom instrument loading to FluidSynth.
 * The caller supplied functions for loading files, allocating presets,
 * retrieving information on them and synthesizing note-on events.  Using this
 * method even non SoundFont instruments can be synthesized, although limited
 * to the SoundFont synthesis model.
 *
 * @note Should only be called before any SoundFont files are loaded.
 */
void
fluid_synth_add_sfloader(fluid_synth_t *synth, fluid_sfloader_t *loader)
{
    fluid_return_if_fail(synth != NULL);
    fluid_return_if_fail(loader != NULL);
    fluid_synth_api_enter(synth);

    /* Test if sfont is already loaded */
    if(synth->sfont == NULL)
    {
        synth->loaders = fluid_list_prepend(synth->loaders, loader);
    }

    fluid_synth_api_exit(synth);
}

/**
 * Load a SoundFont file (filename is interpreted by SoundFont loaders).
 * The newly loaded SoundFont will be put on top of the SoundFont
 * stack. Presets are searched starting from the SoundFont on the
 * top of the stack, working the way down the stack until a preset is found.
 *
 * @param synth FluidSynth instance
 * @param filename File to load
 * @param reset_presets TRUE to re-assign presets for all MIDI channels (equivalent to calling fluid_synth_program_reset())
 * @return SoundFont ID on success, #FLUID_FAILED on error
 * 
 * @note Since FluidSynth 2.2.0 @c filename is treated as an UTF8 encoded string on Windows. FluidSynth will convert it
 * to wide-char internally and then pass it to <code>_wfopen()</code>. Before FluidSynth 2.2.0, @c filename was treated as ANSI string
 * on Windows. All other platforms directly pass it to <code>fopen()</code> without any conversion (usually, UTF8 is accepted).
 */
int
fluid_synth_sfload(fluid_synth_t *synth, const char *filename, int reset_presets)
{
    fluid_sfont_t *sfont;
    fluid_list_t *list;
    fluid_sfloader_t *loader;
    int sfont_id;

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(filename != NULL, FLUID_FAILED);
    fluid_synth_api_enter(synth);

    sfont_id = synth->sfont_id;

    if(++sfont_id != FLUID_FAILED)
    {
        /* MT NOTE: Loaders list should not change. */

        for(list = synth->loaders; list; list = fluid_list_next(list))
        {
            loader = (fluid_sfloader_t *) fluid_list_get(list);

            sfont = fluid_sfloader_load(loader, filename);

            if(sfont != NULL)
            {
                sfont->refcount++;
                synth->sfont_id = sfont->id = sfont_id;

                synth->sfont = fluid_list_prepend(synth->sfont, sfont);   /* prepend to list */

                /* reset the presets for all channels if requested */
                if(reset_presets)
                {
                    fluid_synth_program_reset(synth);
                }

                FLUID_API_RETURN(sfont_id);
            }
        }
    }

    FLUID_LOG(FLUID_ERR, "Failed to load SoundFont \"%s\"", filename);
    FLUID_API_RETURN(FLUID_FAILED);
}

/**
 * Schedule a SoundFont for unloading.
 *
 * If the SoundFont isn't used anymore by any playing voices, it will be unloaded immediately.
 *
 * If any samples of the given SoundFont are still required by active voices,
 * the SoundFont will be unloaded in a lazy manner, once those voices have finished synthesizing.
 * If you call delete_fluid_synth(), all voices will be destroyed and the SoundFont
 * will be unloaded in any case.
 * Once this function returned, fluid_synth_sfcount() and similar functions will behave as if
 * the SoundFont has already been unloaded, even though the lazy-unloading is still pending.
 *
 * @note This lazy-unloading mechanism was broken between FluidSynth 1.1.4 and 2.1.5 . As a
 * consequence, SoundFonts scheduled for lazy-unloading may be never freed under certain
 * conditions. Calling delete_fluid_synth() does not recover this situation either.
 *
 * @param synth FluidSynth instance
 * @param id ID of SoundFont to unload
 * @param reset_presets TRUE to re-assign presets for all MIDI channels
 * @return #FLUID_OK if the given @p id was found, #FLUID_FAILED otherwise.
 */
int
fluid_synth_sfunload(fluid_synth_t *synth, int id, int reset_presets)
{
    fluid_sfont_t *sfont = NULL;
    fluid_list_t *list;

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_synth_api_enter(synth);

    /* remove the SoundFont from the list */
    for(list = synth->sfont; list; list = fluid_list_next(list))
    {
        sfont = fluid_list_get(list);

        if(fluid_sfont_get_id(sfont) == id)
        {
            synth->sfont = fluid_list_remove(synth->sfont, sfont);
            break;
        }
    }

    if(!list)
    {
        FLUID_LOG(FLUID_ERR, "No SoundFont with id = %d", id);
        FLUID_API_RETURN(FLUID_FAILED);
    }

    /* reset the presets for all channels (SoundFont will be freed when there are no more references) */
    if(reset_presets)
    {
        fluid_synth_program_reset(synth);
    }
    else
    {
        fluid_synth_update_presets(synth);
    }

    /* -- Remove synth->sfont list's reference to SoundFont */
    fluid_synth_sfont_unref(synth, sfont);

    FLUID_API_RETURN(FLUID_OK);
}

/* Unref a SoundFont and destroy if no more references */
void
fluid_synth_sfont_unref(fluid_synth_t *synth, fluid_sfont_t *sfont)
{
    fluid_return_if_fail(sfont != NULL);     /* Shouldn't happen, programming error if so */

    sfont->refcount--;             /* -- Remove the sfont list's reference */

    if(sfont->refcount == 0)  /* No more references? - Attempt delete */
    {
        if(fluid_sfont_delete_internal(sfont) == 0)      /* SoundFont loader can block SoundFont unload */
        {
            FLUID_LOG(FLUID_DBG, "Unloaded SoundFont");
        } /* spin off a timer thread to unload the sfont later (SoundFont loader blocked unload) */
        else
        {
            fluid_timer_t* timer = new_fluid_timer(100, fluid_synth_sfunload_callback, sfont, TRUE, FALSE, FALSE);
            synth->fonts_to_be_unloaded = fluid_list_prepend(synth->fonts_to_be_unloaded, timer);
        }
    }
}

/* Callback to continually attempt to unload a SoundFont,
 * only if a SoundFont loader blocked the unload operation */
static int
fluid_synth_sfunload_callback(void *data, unsigned int msec)
{
    fluid_sfont_t *sfont = data;

    if(fluid_sfont_delete_internal(sfont) == 0)
    {
        FLUID_LOG(FLUID_DBG, "Unloaded SoundFont");
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/**
 * Reload a SoundFont.  The SoundFont retains its ID and index on the SoundFont stack.
 * @param synth FluidSynth instance
 * @param id ID of SoundFont to reload
 * @return SoundFont ID on success, #FLUID_FAILED on error
 */
int
fluid_synth_sfreload(fluid_synth_t *synth, int id)
{
    char *filename = NULL;
    fluid_sfont_t *sfont;
    fluid_sfloader_t *loader;
    fluid_list_t *list;
    int index, ret = FLUID_FAILED;

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_synth_api_enter(synth);

    /* Search for SoundFont and get its index */
    for(list = synth->sfont, index = 0; list; list = fluid_list_next(list), index++)
    {
        sfont = fluid_list_get(list);

        if(fluid_sfont_get_id(sfont) == id)
        {
            break;
        }
    }

    if(!list)
    {
        FLUID_LOG(FLUID_ERR, "No SoundFont with id = %d", id);
        goto exit;
    }

    /* keep a copy of the SoundFont's filename */
    filename = FLUID_STRDUP(fluid_sfont_get_name(sfont));

    if(filename == NULL || fluid_synth_sfunload(synth, id, FALSE) != FLUID_OK)
    {
        goto exit;
    }

    /* MT Note: SoundFont loader list will not change */

    for(list = synth->loaders; list; list = fluid_list_next(list))
    {
        loader = (fluid_sfloader_t *) fluid_list_get(list);

        sfont = fluid_sfloader_load(loader, filename);

        if(sfont != NULL)
        {
            sfont->id = id;
            sfont->refcount++;

            synth->sfont = fluid_list_insert_at(synth->sfont, index, sfont);  /* insert the sfont at the same index */

            /* reset the presets for all channels */
            fluid_synth_update_presets(synth);
            ret = id;
            goto exit;
        }
    }

    FLUID_LOG(FLUID_ERR, "Failed to load SoundFont \"%s\"", filename);

exit:
    FLUID_FREE(filename);
    FLUID_API_RETURN(ret);
}

/**
 * Add a SoundFont. The SoundFont will be added to the top of the SoundFont stack and ownership is transferred to @p synth.
 * @param synth FluidSynth instance
 * @param sfont SoundFont to add
 * @return New assigned SoundFont ID or #FLUID_FAILED on error
 */
int
fluid_synth_add_sfont(fluid_synth_t *synth, fluid_sfont_t *sfont)
{
    int sfont_id;

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(sfont != NULL, FLUID_FAILED);
    fluid_synth_api_enter(synth);

    sfont_id = synth->sfont_id;

    if(++sfont_id != FLUID_FAILED)
    {
        synth->sfont_id = sfont->id = sfont_id;
        synth->sfont = fluid_list_prepend(synth->sfont, sfont);        /* prepend to list */

        /* reset the presets for all channels */
        fluid_synth_program_reset(synth);
    }

    FLUID_API_RETURN(sfont_id);
}

/**
 * Remove a SoundFont from the SoundFont stack without deleting it.
 * @param synth FluidSynth instance
 * @param sfont SoundFont to remove
 * @return #FLUID_OK if \c sfont successfully removed, #FLUID_FAILED otherwise
 *
 * SoundFont is not freed and is left as the responsibility of the caller.
 *
 * @note The SoundFont should only be freed after there are no presets
 *   referencing it.  This can only be ensured by the SoundFont loader and
 *   therefore this function should not normally be used.
 */
int
fluid_synth_remove_sfont(fluid_synth_t *synth, fluid_sfont_t *sfont)
{
    fluid_sfont_t *sfont_tmp;
    fluid_list_t *list;
    int ret = FLUID_FAILED;

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(sfont != NULL, FLUID_FAILED);
    fluid_synth_api_enter(synth);

    /* remove the SoundFont from the list */
    for(list = synth->sfont; list; list = fluid_list_next(list))
    {
        sfont_tmp = fluid_list_get(list);

        if(sfont_tmp == sfont)
        {
            synth->sfont = fluid_list_remove(synth->sfont, sfont_tmp);
            ret = FLUID_OK;
            break;
        }
    }

    /* reset the presets for all channels */
    fluid_synth_program_reset(synth);

    FLUID_API_RETURN(ret);
}

/**
 * Count number of loaded SoundFont files.
 * @param synth FluidSynth instance
 * @return Count of loaded SoundFont files.
 */
int
fluid_synth_sfcount(fluid_synth_t *synth)
{
    int count;

    fluid_return_val_if_fail(synth != NULL, 0);
    fluid_synth_api_enter(synth);
    count = fluid_list_size(synth->sfont);
    FLUID_API_RETURN(count);
}

/**
 * Get SoundFont by index.
 * @param synth FluidSynth instance
 * @param num SoundFont index on the stack (starting from 0 for top of stack).
 * @return SoundFont instance or NULL if invalid index
 *
 * @note Caller should be certain that SoundFont is not deleted (unloaded) for
 * the duration of use of the returned pointer.
 */
fluid_sfont_t *
fluid_synth_get_sfont(fluid_synth_t *synth, unsigned int num)
{
    fluid_sfont_t *sfont = NULL;
    fluid_list_t *list;

    fluid_return_val_if_fail(synth != NULL, NULL);
    fluid_synth_api_enter(synth);
    list = fluid_list_nth(synth->sfont, num);

    if(list)
    {
        sfont = fluid_list_get(list);
    }

    FLUID_API_RETURN(sfont);
}

/**
 * Get SoundFont by ID.
 * @param synth FluidSynth instance
 * @param id SoundFont ID
 * @return SoundFont instance or NULL if invalid ID
 *
 * @note Caller should be certain that SoundFont is not deleted (unloaded) for
 * the duration of use of the returned pointer.
 */
fluid_sfont_t *
fluid_synth_get_sfont_by_id(fluid_synth_t *synth, int id)
{
    fluid_sfont_t *sfont = NULL;
    fluid_list_t *list;

    fluid_return_val_if_fail(synth != NULL, NULL);
    fluid_synth_api_enter(synth);

    for(list = synth->sfont; list; list = fluid_list_next(list))
    {
        sfont = fluid_list_get(list);

        if(fluid_sfont_get_id(sfont) == id)
        {
            break;
        }
    }

    FLUID_API_RETURN(list ? sfont : NULL);
}

/**
 * Get SoundFont by name.
 * @param synth FluidSynth instance
 * @param name Name of SoundFont
 * @return SoundFont instance or NULL if invalid name
 * @since 1.1.0
 *
 * @note Caller should be certain that SoundFont is not deleted (unloaded) for
 * the duration of use of the returned pointer.
 */
fluid_sfont_t *
fluid_synth_get_sfont_by_name(fluid_synth_t *synth, const char *name)
{
    fluid_sfont_t *sfont = NULL;
    fluid_list_t *list;

    fluid_return_val_if_fail(synth != NULL, NULL);
    fluid_return_val_if_fail(name != NULL, NULL);
    fluid_synth_api_enter(synth);

    for(list = synth->sfont; list; list = fluid_list_next(list))
    {
        sfont = fluid_list_get(list);

        if(FLUID_STRCMP(fluid_sfont_get_name(sfont), name) == 0)
        {
            break;
        }
    }

    FLUID_API_RETURN(list ? sfont : NULL);
}

/**
 * Get active preset on a MIDI channel.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @return Preset or NULL if no preset active on \c chan
 *
 * @note Should only be called from within synthesis thread, which includes
 * SoundFont loader preset noteon methods. Not thread safe otherwise.
 */
fluid_preset_t *
fluid_synth_get_channel_preset(fluid_synth_t *synth, int chan)
{
    fluid_preset_t *result;
    fluid_channel_t *channel;
    FLUID_API_ENTRY_CHAN(NULL);

    channel = synth->channel[chan];
    result = channel->preset;
    fluid_synth_api_exit(synth);
    return result;
}

/**
 * Get list of currently playing voices.
 * @param synth FluidSynth instance
 * @param buf Array to store voices to (NULL terminated if not filled completely)
 * @param bufsize Count of indexes in buf
 * @param id Voice ID to search for or < 0 to return list of all playing voices
 *
 * @note Should only be called from within synthesis thread, which includes
 * SoundFont loader preset noteon methods.  Voices are only guaranteed to remain
 * unchanged until next synthesis process iteration.
 */
void
fluid_synth_get_voicelist(fluid_synth_t *synth, fluid_voice_t *buf[], int bufsize,
                          int id)
{
    int count = 0;
    int i;

    fluid_return_if_fail(synth != NULL);
    fluid_return_if_fail(buf != NULL);
    fluid_synth_api_enter(synth);

    for(i = 0; i < synth->polyphony && count < bufsize; i++)
    {
        fluid_voice_t *voice = synth->voice[i];

        if(fluid_voice_is_playing(voice) && (id < 0 || (int)voice->id == id))
        {
            buf[count++] = voice;
        }
    }

    if(count < bufsize)
    {
        buf[count] = NULL;
    }

    fluid_synth_api_exit(synth);
}

/**
 * Enable or disable reverb effect.
 * @param synth FluidSynth instance
 * @param on TRUE to enable chorus, FALSE to disable
 * @deprecated Use fluid_synth_reverb_on() instead.
 */
void
fluid_synth_set_reverb_on(fluid_synth_t *synth, int on)
{
    fluid_return_if_fail(synth != NULL);
    fluid_synth_api_enter(synth);

    synth->with_reverb = (on != 0);
    fluid_synth_update_mixer(synth, fluid_rvoice_mixer_set_reverb_enabled,
                             on != 0, 0.0f);
    fluid_synth_api_exit(synth);
}

/**
 * Enable or disable reverb on one fx group unit.
 * @param synth FluidSynth instance
 * @param fx_group Index of the fx group.
 *  Must be in the range <code>-1 to (fluid_synth_count_effects_groups()-1)</code>. If -1 the
 *  parameter will be applied to all fx groups.
 * @param on TRUE to enable reverb, FALSE to disable
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int
fluid_synth_reverb_on(fluid_synth_t *synth, int fx_group, int on)
{
    int ret;
	fluid_rvoice_param_t param[MAX_EVENT_PARAMS];
    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);

    fluid_synth_api_enter(synth);

    if(fx_group  < -1 || fx_group >= synth->effects_groups)
    {
        FLUID_API_RETURN(FLUID_FAILED);
    }

    if(fx_group  < 0 )
    {
        synth->with_reverb = (on != 0);
    }

    param[0].i = fx_group;
    param[1].i = on;
    ret = fluid_rvoice_eventhandler_push(synth->eventhandler,
                                         fluid_rvoice_mixer_reverb_enable,
                                         synth->eventhandler->mixer,
                                         param);

    FLUID_API_RETURN(ret);
}

/**
 * Activate a reverb preset.
 * @param synth FluidSynth instance
 * @param num Reverb preset number
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 *
 * @note Currently private to libfluidsynth.
 */
int
fluid_synth_set_reverb_preset(fluid_synth_t *synth, unsigned int num)
{
    double values[FLUID_REVERB_PARAM_LAST];

    fluid_return_val_if_fail(
        num < FLUID_N_ELEMENTS(revmodel_preset),
        FLUID_FAILED
    );

    values[FLUID_REVERB_ROOMSIZE] = revmodel_preset[num].roomsize;
    values[FLUID_REVERB_DAMP] = revmodel_preset[num].damp;
    values[FLUID_REVERB_WIDTH] = revmodel_preset[num].width;
    values[FLUID_REVERB_LEVEL] = revmodel_preset[num].level;
    fluid_synth_set_reverb_full(synth, -1, FLUID_REVMODEL_SET_ALL, values);
    return FLUID_OK;
}

/**
 * Set reverb parameters to all groups.
 *
 * @param synth FluidSynth instance
 * @param roomsize Reverb room size value (0.0-1.0)
 * @param damping Reverb damping value (0.0-1.0)
 * @param width Reverb width value (0.0-100.0)
 * @param level Reverb level value (0.0-1.0)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @deprecated Use the individual reverb setter functions in new code instead.
 */
int
fluid_synth_set_reverb(fluid_synth_t *synth, double roomsize, double damping,
                       double width, double level)
{
    double values[FLUID_REVERB_PARAM_LAST];

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);

    values[FLUID_REVERB_ROOMSIZE] = roomsize;
    values[FLUID_REVERB_DAMP] = damping;
    values[FLUID_REVERB_WIDTH] = width;
    values[FLUID_REVERB_LEVEL] = level;
    return fluid_synth_set_reverb_full(synth, -1, FLUID_REVMODEL_SET_ALL, values);
}

/**
 * Set reverb roomsize of all groups.
 *
 * @param synth FluidSynth instance
 * @param roomsize Reverb room size value (0.0-1.0)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @deprecated Use fluid_synth_set_reverb_group_roomsize() in new code instead.
 */
int fluid_synth_set_reverb_roomsize(fluid_synth_t *synth, double roomsize)
{
    return fluid_synth_reverb_set_param(synth, -1, FLUID_REVERB_ROOMSIZE, roomsize);
}

/**
 * Set reverb damping of all groups.
 *
 * @param synth FluidSynth instance
 * @param damping Reverb damping value (0.0-1.0)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @deprecated Use fluid_synth_set_reverb_group_damp() in new code instead.
 */
int fluid_synth_set_reverb_damp(fluid_synth_t *synth, double damping)
{
    return fluid_synth_reverb_set_param(synth, -1, FLUID_REVERB_DAMP, damping);
}

/**
 * Set reverb width of all groups.
 *
 * @param synth FluidSynth instance
 * @param width Reverb width value (0.0-100.0)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @deprecated Use fluid_synth_set_reverb_group_width() in new code instead.
 */
int fluid_synth_set_reverb_width(fluid_synth_t *synth, double width)
{
    return fluid_synth_reverb_set_param(synth, -1, FLUID_REVERB_WIDTH, width);
}

/**
 * Set reverb level of all groups.
 *
 * @param synth FluidSynth instance
 * @param level Reverb level value (0.0-1.0)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @deprecated Use fluid_synth_set_reverb_group_level() in new code instead.
 */
int fluid_synth_set_reverb_level(fluid_synth_t *synth, double level)
{
    return fluid_synth_reverb_set_param(synth, -1, FLUID_REVERB_LEVEL, level);
}

/**
 * Set reverb roomsize to one or all fx groups.
 * @param synth FluidSynth instance.
 * @param fx_group Index of the fx group.
 *  Must be in the range <code>-1 to (fluid_synth_count_effects_groups()-1)</code>. If -1 the
 *  parameter will be applied to all fx groups.
 * @param roomsize roomsize value to set. Must be in the range indicated by
 * synth.reverb.room-size setting.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int fluid_synth_set_reverb_group_roomsize(fluid_synth_t *synth, int fx_group,
                                          double roomsize)
{
    return fluid_synth_reverb_set_param(synth, fx_group, FLUID_REVERB_ROOMSIZE, roomsize);
}

/**
 * Set reverb damp to one or all fx groups.
 * @param synth FluidSynth instance.
 * @param fx_group Index of the fx group.
 *  Must be in the range <code>-1 to (fluid_synth_count_effects_groups()-1)</code>. If -1 the
 *  parameter will be applied to all fx groups.
 * @param damping damping value to set. Must be in the range indicated by
 * synth.reverb.damp setting.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int fluid_synth_set_reverb_group_damp(fluid_synth_t *synth, int fx_group,
                                      double damping)
{
    return fluid_synth_reverb_set_param(synth, fx_group, FLUID_REVERB_DAMP, damping);
}

/**
 * Set reverb width to one or all fx groups.
 * @param synth FluidSynth instance.
 * @param fx_group Index of the fx group.
 *  Must be in the range <code>-1 to (fluid_synth_count_effects_groups()-1)</code>. If -1 the
 *  parameter will be applied to all fx groups.
 * @param width width value to set. Must be in the range indicated by
 * synth.reverb.width setting.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int fluid_synth_set_reverb_group_width(fluid_synth_t *synth, int fx_group,
                                      double width)
{
    return fluid_synth_reverb_set_param(synth, fx_group, FLUID_REVERB_WIDTH, width);
}

/**
 * Set reverb level to one or all fx groups.
 * @param synth FluidSynth instance.
 * @param fx_group Index of the fx group.
 *  Must be in the range <code>-1 to (fluid_synth_count_effects_groups()-1)</code>. If -1 the
 *  parameter will be applied to all fx groups.
 * @param level output level to set. Must be in the range indicated by
 * synth.reverb.level setting.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int fluid_synth_set_reverb_group_level(fluid_synth_t *synth, int fx_group,
                                       double level)
{
    return fluid_synth_reverb_set_param(synth, fx_group, FLUID_REVERB_LEVEL, level);
}

/**
 * Set one reverb parameter to one fx groups.
 * @param synth FluidSynth instance.
 * @param fx_group Index of the fx group.
 *  Must be in the range <code>-1 to (fluid_synth_count_effects_groups()-1)</code>. If -1 the
 *  parameter will be applied to all fx groups.
 * @param enum indicating the parameter to set (#fluid_reverb_param).
 *  FLUID_REVERB_ROOMSIZE, roomsize Reverb room size value (0.0-1.0)
 *  FLUID_REVERB_DAMP, reverb damping value (0.0-1.0)
 *  FLUID_REVERB_WIDTH, reverb width value (0.0-100.0)
 *  FLUID_REVERB_LEVEL, reverb level value (0.0-1.0)
 * @param value, parameter value
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int
fluid_synth_reverb_set_param(fluid_synth_t *synth, int fx_group,
                             int param, double value)
{
    int ret;
    double values[FLUID_REVERB_PARAM_LAST] = {0.0};
    static const char *name[FLUID_REVERB_PARAM_LAST] =
    {
        "synth.reverb.room-size", "synth.reverb.damp",
        "synth.reverb.width", "synth.reverb.level"
    };

    double min; /* minimum value */
    double max; /* maximum value */

    /* check parameters */
    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_return_val_if_fail((param >= 0) && (param < FLUID_REVERB_PARAM_LAST), FLUID_FAILED);
    fluid_synth_api_enter(synth);

    if(fx_group  < -1 || fx_group >= synth->effects_groups)
    {
        FLUID_API_RETURN(FLUID_FAILED);
    }

    /* check if reverb value is in max min range */
    fluid_settings_getnum_range(synth->settings, name[param], &min, &max);
    if(value  < min || value > max)
    {
        FLUID_API_RETURN(FLUID_FAILED);
    }

    /* set the value */
    values[param] = value;
    ret = fluid_synth_set_reverb_full(synth, fx_group, FLUID_REVPARAM_TO_SETFLAG(param), values);
    FLUID_API_RETURN(ret);
}

int
fluid_synth_set_reverb_full(fluid_synth_t *synth, int fx_group, int set,
                            const double values[])
{
    fluid_rvoice_param_t param[MAX_EVENT_PARAMS];

    /* if non of the flags is set, fail */
    fluid_return_val_if_fail(set & FLUID_REVMODEL_SET_ALL, FLUID_FAILED);

    /* fx group shadow values are set here so that they will be returned if queried */
    fluid_rvoice_mixer_set_reverb_full(synth->eventhandler->mixer, fx_group, set,
                                       values);

    /* Synth shadow values are set here so that they will be returned if queried */
    if (fx_group < 0)
    {
        int i;
        for(i = 0; i < FLUID_REVERB_PARAM_LAST; i++)
        {
            if(set & FLUID_REVPARAM_TO_SETFLAG(i))
            {
                synth->reverb_param[i] = values[i];
            }
        }
    }

    param[0].i = fx_group;
    param[1].i = set;
    param[2].real = values[FLUID_REVERB_ROOMSIZE];
    param[3].real = values[FLUID_REVERB_DAMP];
    param[4].real = values[FLUID_REVERB_WIDTH];
    param[5].real = values[FLUID_REVERB_LEVEL];
    /* finally enqueue an rvoice event to the mixer to actual update reverb */
    return fluid_rvoice_eventhandler_push(synth->eventhandler,
                                         fluid_rvoice_mixer_set_reverb_params,
                                         synth->eventhandler->mixer,
                                         param);
}

/**
 * Get reverb room size of all fx groups.
 * @param synth FluidSynth instance
 * @return Reverb room size (0.0-1.2)
 * @deprecated Use fluid_synth_get_reverb_group_roomsize() in new code instead.
 */
double
fluid_synth_get_reverb_roomsize(fluid_synth_t *synth)
{
    double roomsize = 0.0;
    fluid_synth_reverb_get_param(synth, -1, FLUID_REVERB_ROOMSIZE, &roomsize);
    return roomsize;
}

/**
 * Get reverb damping of all fx groups.
 * @param synth FluidSynth instance
 * @return Reverb damping value (0.0-1.0)
 * @deprecated Use fluid_synth_get_reverb_group_damp() in new code instead.
 */
double
fluid_synth_get_reverb_damp(fluid_synth_t *synth)
{
    double damp = 0.0;
    fluid_synth_reverb_get_param(synth, -1, FLUID_REVERB_DAMP, &damp);
    return damp;
}

/**
 * Get reverb level of all fx groups.
 * @param synth FluidSynth instance
 * @return Reverb level value (0.0-1.0)
 * @deprecated Use fluid_synth_get_reverb_group_level() in new code instead.
 */
double
fluid_synth_get_reverb_level(fluid_synth_t *synth)
{
    double level = 0.0;
    fluid_synth_reverb_get_param(synth, -1, FLUID_REVERB_LEVEL, &level);
    return level;
}

/**
 * Get reverb width of all fx groups.
 * @param synth FluidSynth instance
 * @return Reverb width value (0.0-100.0)
 * @deprecated Use fluid_synth_get_reverb_group_width() in new code instead.
 */
double
fluid_synth_get_reverb_width(fluid_synth_t *synth)
{
    double width = 0.0;
    fluid_synth_reverb_get_param(synth, -1, FLUID_REVERB_WIDTH, &width);
    return width;
}

/**
 * get reverb roomsize of one or all groups.
 * @param synth FluidSynth instance.
 * @param fx_group Index of the fx group.
 *  Must be in the range <code>-1 to (fluid_synth_count_effects_groups()-1)</code>. If -1 the
 *  parameter common to all fx groups is fetched.
 * @param roomsize valid pointer on the value to return.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int fluid_synth_get_reverb_group_roomsize(fluid_synth_t *synth, int fx_group,
                                          double *roomsize)
{
    return fluid_synth_reverb_get_param(synth, fx_group, FLUID_REVERB_ROOMSIZE, roomsize);
}

/**
 * get reverb damp of one or all groups.
 * @param synth FluidSynth instance.
 * @param fx_group Index of the fx group.
 *  Must be in the range <code>-1 to (fluid_synth_count_effects_groups()-1)</code>. If -1 the
 *  parameter common to all fx groups is fetched.
 * @param damping valid pointer on the value to return.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int fluid_synth_get_reverb_group_damp(fluid_synth_t *synth, int fx_group,
                                      double *damping)
{
    return fluid_synth_reverb_get_param(synth, fx_group, FLUID_REVERB_DAMP, damping);
}

/**
 * get reverb width of one or all groups
 * @param synth FluidSynth instance.
 * @param fx_group Index of the fx group.
 *  Must be in the range <code>-1 to (fluid_synth_count_effects_groups()-1)</code>. If -1 the
 *  parameter common to all fx groups is fetched.
 * @param width valid pointer on the value to return.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int fluid_synth_get_reverb_group_width(fluid_synth_t *synth, int fx_group,
                                       double *width)
{
    return fluid_synth_reverb_get_param(synth, fx_group, FLUID_REVERB_WIDTH, width);
}

/**
 * get reverb level of one or all groups.
 * @param synth FluidSynth instance.
 * @param fx_group Index of the fx group.
 *  Must be in the range <code>-1 to (fluid_synth_count_effects_groups()-1)</code>. If -1 the
 *  parameter common to all fx groups is fetched.
 * @param level valid pointer on the value to return.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int fluid_synth_get_reverb_group_level(fluid_synth_t *synth, int fx_group,
                                       double *level)
{
    return fluid_synth_reverb_get_param(synth, fx_group, FLUID_REVERB_LEVEL, level);
}


/**
 * Get one reverb parameter value of one fx groups.
 * @param synth FluidSynth instance
 * @param fx_group index of the fx group to get parameter value from.
 *  Must be in the range -1 to synth->effects_groups-1. If -1 get the
 *  parameter common to all fx groups.
 * @param enum indicating the parameter to get (#fluid_reverb_param).
 *  FLUID_REVERB_ROOMSIZE, reverb room size value.
 *  FLUID_REVERB_DAMP, reverb damping value.
 *  FLUID_REVERB_WIDTH, reverb width value.
 *  FLUID_REVERB_LEVEL, reverb level value.
 * @param value pointer on the value to return.
 * @return FLUID_OK if success, FLUID_FAILED otherwise.
 */
static int fluid_synth_reverb_get_param(fluid_synth_t *synth, int fx_group,
                                        int param, double *value)
{
    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_return_val_if_fail((param >= 0) && (param < FLUID_REVERB_PARAM_LAST), FLUID_FAILED);
    fluid_return_val_if_fail(value != NULL, FLUID_FAILED);
    fluid_synth_api_enter(synth);

    if(fx_group  < -1 || fx_group >= synth->effects_groups)
    {
        FLUID_API_RETURN(FLUID_FAILED);
    }

    if (fx_group < 0)
    {
        /* return reverb param common to all fx groups */
        *value = synth->reverb_param[param];
    }
    else
    {
        /* return reverb param of fx group at index fx_group */
        *value = fluid_rvoice_mixer_reverb_get_param(synth->eventhandler->mixer,
                                                     fx_group, param);
    }

    FLUID_API_RETURN(FLUID_OK);
}

/**
 * Enable or disable all chorus groups.
 * @param synth FluidSynth instance
 * @param on TRUE to enable chorus, FALSE to disable
 * @deprecated Use fluid_synth_chorus_on() in new code instead.
 */
void
fluid_synth_set_chorus_on(fluid_synth_t *synth, int on)
{
    fluid_return_if_fail(synth != NULL);
    fluid_synth_api_enter(synth);

    synth->with_chorus = (on != 0);
    fluid_synth_update_mixer(synth, fluid_rvoice_mixer_set_chorus_enabled,
                             on != 0, 0.0f);
    fluid_synth_api_exit(synth);
}

/**
 * Enable or disable chorus on one or all groups.
 * @param synth FluidSynth instance
 * @param fx_group Index of the fx group.
 *  Must be in the range <code>-1 to (fluid_synth_count_effects_groups()-1)</code>. If -1 the
 *  parameter will be applied to all fx groups.
 * @param on TRUE to enable chorus, FALSE to disable
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int
fluid_synth_chorus_on(fluid_synth_t *synth, int fx_group, int on)
{
    int ret;
	fluid_rvoice_param_t param[MAX_EVENT_PARAMS];
    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);

    fluid_synth_api_enter(synth);

    if(fx_group  < -1 || fx_group >= synth->effects_groups)
    {
        FLUID_API_RETURN(FLUID_FAILED);
    }

    if(fx_group  < 0 )
    {
        synth->with_chorus = (on != 0);
    }

    param[0].i = fx_group;
    param[1].i = on;
    ret = fluid_rvoice_eventhandler_push(synth->eventhandler,
                                         fluid_rvoice_mixer_chorus_enable,
                                         synth->eventhandler->mixer,
                                         param);

    FLUID_API_RETURN(ret);
}

/**
 * Set chorus parameters to all fx groups.
 * Keep in mind, that the needed CPU time is proportional to 'nr'.
 * @param synth FluidSynth instance
 * @param nr Chorus voice count (0-99, CPU time consumption proportional to
 *   this value)
 * @param level Chorus level (0.0-10.0)
 * @param speed Chorus speed in Hz (0.1-5.0)
 * @param depth_ms Chorus depth (max value depends on synth sample-rate,
 *   0.0-21.0 is safe for sample-rate values up to 96KHz)
 * @param type Chorus waveform type (#fluid_chorus_mod)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @deprecated Use the individual chorus setter functions in new code instead.
 * 
 * Keep in mind, that the needed CPU time is proportional to 'nr'.
 */
int fluid_synth_set_chorus(fluid_synth_t *synth, int nr, double level,
                           double speed, double depth_ms, int type)
{
    double values[FLUID_CHORUS_PARAM_LAST];

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);

    values[FLUID_CHORUS_NR] = nr;
    values[FLUID_CHORUS_LEVEL] = level;
    values[FLUID_CHORUS_SPEED] = speed;
    values[FLUID_CHORUS_DEPTH] = depth_ms;
    values[FLUID_CHORUS_TYPE] = type;
    return fluid_synth_set_chorus_full(synth, -1, FLUID_CHORUS_SET_ALL, values);
}

/**
 * Set the chorus voice count of all groups.
 *
 * @param synth FluidSynth instance
 * @param nr Chorus voice count (0-99, CPU time consumption proportional to
 *   this value)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @deprecated Use fluid_synth_set_chorus_group_nr() in new code instead.
 */
int fluid_synth_set_chorus_nr(fluid_synth_t *synth, int nr)
{
    return fluid_synth_chorus_set_param(synth, -1, FLUID_CHORUS_NR, nr);
}

/**
 * Set the chorus level of all groups.
 *
 * @param synth FluidSynth instance
 * @param level Chorus level (0.0-10.0)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @deprecated Use fluid_synth_set_chorus_group_level() in new code instead.
 */
int fluid_synth_set_chorus_level(fluid_synth_t *synth, double level)
{
    return fluid_synth_chorus_set_param(synth, -1, FLUID_CHORUS_LEVEL, level);
}

/**
 * Set the chorus speed of all groups.
 *
 * @param synth FluidSynth instance
 * @param speed Chorus speed in Hz (0.1-5.0)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @deprecated Use fluid_synth_set_chorus_group_speed() in new code instead.
 */
int fluid_synth_set_chorus_speed(fluid_synth_t *synth, double speed)
{
    return fluid_synth_chorus_set_param(synth, -1, FLUID_CHORUS_SPEED, speed);
}

/**
 * Set the chorus depth of all groups.
 *
 * @param synth FluidSynth instance
 * @param depth_ms Chorus depth (max value depends on synth sample-rate,
 *   0.0-21.0 is safe for sample-rate values up to 96KHz)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @deprecated Use fluid_synth_set_chorus_group_depth() in new code instead.
 */
int fluid_synth_set_chorus_depth(fluid_synth_t *synth, double depth_ms)
{
    return fluid_synth_chorus_set_param(synth, -1, FLUID_CHORUS_DEPTH, depth_ms);
}

/**
 * Set the chorus type of all groups.
 *
 * @param synth FluidSynth instance
 * @param type Chorus waveform type (#fluid_chorus_mod)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @deprecated Use fluid_synth_set_chorus_group_type() in new code instead.
 */
int fluid_synth_set_chorus_type(fluid_synth_t *synth, int type)
{
    return fluid_synth_chorus_set_param(synth, -1, FLUID_CHORUS_TYPE, type);
}

/**
 * Set chorus voice count nr to one or all chorus groups.
 * @param synth FluidSynth instance.
 * @param fx_group Index of the fx group.
 *  Must be in the range <code>-1 to (fluid_synth_count_effects_groups()-1)</code>. If -1 the
 *  parameter will be applied to all groups.
 * @param nr Voice count to set. Must be in the range indicated by \setting{synth_chorus_nr}
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int
fluid_synth_set_chorus_group_nr(fluid_synth_t *synth, int fx_group, int nr)
{
    return fluid_synth_chorus_set_param(synth, fx_group, FLUID_CHORUS_NR, (double)nr);
}

/**
 * Set chorus output level to one or all chorus groups.
 * @param synth FluidSynth instance.
 * @param fx_group Index of the fx group.
 *  Must be in the range <code>-1 to (fluid_synth_count_effects_groups()-1)</code>. If -1 the
 *  parameter will be applied to all groups.
 * @param level Output level to set. Must be in the range indicated by \setting{synth_chorus_level}
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int
fluid_synth_set_chorus_group_level(fluid_synth_t *synth, int fx_group, double level)
{
    return fluid_synth_chorus_set_param(synth, fx_group, FLUID_CHORUS_LEVEL, level);
}

/**
 * Set chorus lfo speed to one or all chorus groups.
 * @param synth FluidSynth instance.
 * @param fx_group Index of the fx group.
 *  Must be in the range <code>-1 to (fluid_synth_count_effects_groups()-1)</code>. If -1 the
 *  parameter will be applied to all groups.
 * @param speed Lfo speed to set. Must be in the range indicated by \setting{synth_chorus_speed}
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int
fluid_synth_set_chorus_group_speed(fluid_synth_t *synth, int fx_group, double speed)
{
    return fluid_synth_chorus_set_param(synth, fx_group, FLUID_CHORUS_SPEED, speed);
}

/**
 * Set chorus lfo depth to one or all chorus groups.
 * @param synth FluidSynth instance.
 * @param fx_group Index of the fx group.
 *  Must be in the range <code>-1 to (fluid_synth_count_effects_groups()-1)</code>. If -1 the
 *  parameter will be applied to all groups.
 * @param depth_ms lfo depth to set. Must be in the range indicated by \setting{synth_chorus_depth}
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int
fluid_synth_set_chorus_group_depth(fluid_synth_t *synth, int fx_group, double depth_ms)
{
    return fluid_synth_chorus_set_param(synth, fx_group, FLUID_CHORUS_DEPTH, depth_ms);
}

/**
 * Set chorus lfo waveform type to one or all chorus groups.
 * @param synth FluidSynth instance.
 * @param fx_group Index of the fx group.
 *  Must be in the range <code>-1 to (fluid_synth_count_effects_groups()-1)</code>. If -1 the
 *  parameter will be applied to all groups.
 * @param type Lfo waveform type to set. (#fluid_chorus_mod)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int
fluid_synth_set_chorus_group_type(fluid_synth_t *synth, int fx_group, int type)
{
    return fluid_synth_chorus_set_param(synth, fx_group, FLUID_CHORUS_TYPE, (double)type);
}

/**
 * Set one chorus parameter to one fx groups.
 * @param synth FluidSynth instance.
 * @param fx_group Index of the fx group.
 *  Must be in the range <code>-1 to (fluid_synth_count_effects_groups()-1)</code>. If -1 the
 *  parameter will be applied to all groups.
 * @param enum indicating the parameter to set (#fluid_chorus_param).
 *  FLUID_CHORUS_NR, chorus voice count (0-99, CPU time consumption proportional to
 *  this value).
 *  FLUID_CHORUS_LEVEL, chorus level (0.0-10.0).
 *  FLUID_CHORUS_SPEED, chorus speed in Hz (0.1-5.0).
 *  FLUID_CHORUS_DEPTH, chorus depth (max value depends on synth sample-rate,
 *   0.0-21.0 is safe for sample-rate values up to 96KHz).
 *  FLUID_CHORUS_TYPE, chorus waveform type (#fluid_chorus_mod)
 * @param value, parameter value
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int
fluid_synth_chorus_set_param(fluid_synth_t *synth, int fx_group, int param,
                             double value)
{
    int ret;
    double values[FLUID_CHORUS_PARAM_LAST] = {0.0};

    /* setting name (except lfo waveform type) */
    static const char *name[FLUID_CHORUS_PARAM_LAST-1] =
    {
        "synth.chorus.nr", "synth.chorus.level",
        "synth.chorus.speed", "synth.chorus.depth"
    };

    /* check parameters */
    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_return_val_if_fail((param >= 0) && (param < FLUID_CHORUS_PARAM_LAST), FLUID_FAILED);
    fluid_synth_api_enter(synth);

    if(fx_group  < -1 || fx_group >= synth->effects_groups)
    {
        FLUID_API_RETURN(FLUID_FAILED);
    }

    /* check if chorus value is in max min range */
    if(param == FLUID_CHORUS_TYPE || param == FLUID_CHORUS_NR) /* integer value */
    {
        int min = FLUID_CHORUS_MOD_SINE;
        int max = FLUID_CHORUS_MOD_TRIANGLE;
        if(param == FLUID_CHORUS_NR)
        {
            fluid_settings_getint_range(synth->settings, name[param], &min, &max);
        }
        if((int)value  < min || (int)value > max)
        {
            FLUID_API_RETURN(FLUID_FAILED);
        }
    }
    else /* float value */
    {
        double min;
        double max;
        fluid_settings_getnum_range(synth->settings, name[param], &min, &max);
        if(value  < min || value > max)
        {
            FLUID_API_RETURN(FLUID_FAILED);
        }
    }

    /* set the value */
    values[param] = value;
    ret = fluid_synth_set_chorus_full(synth, fx_group,
                                      FLUID_CHORPARAM_TO_SETFLAG(param), values);
    FLUID_API_RETURN(ret);
}

int
fluid_synth_set_chorus_full(fluid_synth_t *synth, int fx_group, int set,
                            const double values[])
{
    fluid_rvoice_param_t param[MAX_EVENT_PARAMS];

    /* if non of the flags is set, fail */
    fluid_return_val_if_fail(set & FLUID_CHORUS_SET_ALL, FLUID_FAILED);

    /* fx group shadow values are set here so that they will be returned if queried */
    fluid_rvoice_mixer_set_chorus_full(synth->eventhandler->mixer, fx_group,
                                       set, values);

    /* Synth shadow values are set here so that they will be returned if queried */
    if (fx_group < 0)
    {
        int i;
        for(i = 0; i < FLUID_CHORUS_PARAM_LAST; i++)
        {
            if(set & FLUID_CHORPARAM_TO_SETFLAG(i))
            {
                synth->chorus_param[i] = values[i];
            }
        }
    }

    param[0].i = fx_group;
    param[1].i = set;
    param[2].i = (int)values[FLUID_CHORUS_NR];
    param[3].real = values[FLUID_CHORUS_LEVEL];
    param[4].real = values[FLUID_CHORUS_SPEED];
    param[5].real = values[FLUID_CHORUS_DEPTH];
    param[6].i = (int)values[FLUID_CHORUS_TYPE];
    return fluid_rvoice_eventhandler_push(synth->eventhandler,
                                         fluid_rvoice_mixer_set_chorus_params,
                                         synth->eventhandler->mixer,
                                         param);
}

/**
 * Get chorus voice number (delay line count) value of all fx groups.
 * @param synth FluidSynth instance
 * @return Chorus voice count
 * @deprecated Use fluid_synth_get_chorus_group_nr() in new code instead.
 */
int
fluid_synth_get_chorus_nr(fluid_synth_t *synth)
{
    double nr = 0.0;
    fluid_synth_chorus_get_param(synth, -1, FLUID_CHORUS_NR, &nr);
    return (int)nr;
}

/**
 * Get chorus level of all fx groups.
 * @param synth FluidSynth instance
 * @return Chorus level value
 * @deprecated Use fluid_synth_get_chorus_group_level() in new code instead.
 */
double
fluid_synth_get_chorus_level(fluid_synth_t *synth)
{
    double level = 0.0;
    fluid_synth_chorus_get_param(synth, -1, FLUID_CHORUS_LEVEL, &level);
    return level;
}

/**
 * Get chorus speed in Hz of all fx groups.
 * @param synth FluidSynth instance
 * @return Chorus speed in Hz
 * @deprecated Use fluid_synth_get_chorus_group_speed() in new code instead.
 */
double
fluid_synth_get_chorus_speed(fluid_synth_t *synth)
{
    double speed = 0.0;
    fluid_synth_chorus_get_param(synth, -1, FLUID_CHORUS_SPEED, &speed);
    return speed;
}

/**
 * Get chorus depth of all fx groups.
 * @param synth FluidSynth instance
 * @return Chorus depth
 * @deprecated Use fluid_synth_get_chorus_group_depth() in new code instead.
 */
double
fluid_synth_get_chorus_depth(fluid_synth_t *synth)
{
    double depth = 0.0;
    fluid_synth_chorus_get_param(synth, -1, FLUID_CHORUS_DEPTH, &depth);
    return depth;
}

/**
 * Get chorus waveform type of all fx groups.
 * @param synth FluidSynth instance
 * @return Chorus waveform type (#fluid_chorus_mod)
 * @deprecated Use fluid_synth_get_chorus_group_type() in new code instead.
 */
int
fluid_synth_get_chorus_type(fluid_synth_t *synth)
{
    double type = 0.0;
    fluid_synth_chorus_get_param(synth, -1, FLUID_CHORUS_TYPE, &type);
    return (int)type;
}

/**
 * Get chorus count nr of one or all fx groups.
 * @param synth FluidSynth instance.
 * @param fx_group Index of the fx group from which to fetch the chorus voice count.
 *  Must be in the range <code>-1 to (fluid_synth_count_effects_groups()-1)</code>. If -1 the
 *  parameter common to all fx groups is fetched.
 * @param nr valid pointer on value to return.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int
fluid_synth_get_chorus_group_nr(fluid_synth_t *synth, int fx_group, int *nr)
{
    double num_nr = 0.0;
    int status;
    status = fluid_synth_chorus_get_param(synth, fx_group, FLUID_CHORUS_NR, &num_nr);
    *nr = (int)num_nr;
    return status;
}

/**
 * Get chorus output level of one or all fx groups.
 * @param synth FluidSynth instance.
 * @param fx_group Index of the fx group from which chorus level to fetch.
 *  Must be in the range <code>-1 to (fluid_synth_count_effects_groups()-1)</code>. If -1 the
 *  parameter common to all fx groups is fetched.
 * @param level valid pointer on value to return.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int
fluid_synth_get_chorus_group_level(fluid_synth_t *synth, int fx_group, double *level)
{
    return fluid_synth_chorus_get_param(synth, fx_group, FLUID_CHORUS_LEVEL, level);
}

/**
 * Get chorus waveform lfo speed of one or all fx groups.
 * @param synth FluidSynth instance.
 * @param fx_group Index of the fx group from which lfo speed to fetch.
 *  Must be in the range <code>-1 to (fluid_synth_count_effects_groups()-1)</code>. If -1 the
 *  parameter common to all fx groups is fetched.
 * @param speed valid pointer on value to return.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise.
 */
int
fluid_synth_get_chorus_group_speed(fluid_synth_t *synth, int fx_group, double *speed)
{
    return fluid_synth_chorus_get_param(synth, fx_group, FLUID_CHORUS_SPEED, speed);
}

/**
 * Get chorus lfo depth of one or all fx groups.
 * @param synth FluidSynth instance
 * @param fx_group Index of the fx group from which lfo depth to fetch.
 *  Must be in the range <code>-1 to (fluid_synth_count_effects_groups()-1)</code>. If -1 the
 *  parameter common to all fx groups is fetched.
 * @param depth_ms valid pointer on value to return.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int
fluid_synth_get_chorus_group_depth(fluid_synth_t *synth, int fx_group, double *depth_ms)
{
    return fluid_synth_chorus_get_param(synth, fx_group, FLUID_CHORUS_DEPTH, depth_ms);
}

/**
 * Get chorus waveform type of one or all fx groups.
 * @param synth FluidSynth instance
 * @param fx_group Index of the fx group from which to fetch the waveform type.
 *  Must be in the range <code>-1 to (fluid_synth_count_effects_groups()-1)</code>. If -1 the
 *  parameter common to all fx groups is fetched.
 * @param type valid pointer on waveform type to return (#fluid_chorus_mod)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int
fluid_synth_get_chorus_group_type(fluid_synth_t *synth, int fx_group, int *type)
{
    double num_type = 0.0;
    int status;
    status = fluid_synth_chorus_get_param(synth, fx_group, FLUID_CHORUS_TYPE, &num_type);
    *type = (int)num_type;
    return status;
}

/**
 * Get chorus parameter value of one or all fx groups.
 * @param synth FluidSynth instance
 * @param fx_group index of the fx group
 * @param enum indicating the parameter to get.
 *  FLUID_CHORUS_NR, chorus voice count.
 *  FLUID_CHORUS_LEVEL, chorus level.
 *  FLUID_CHORUS_SPEED, chorus speed.
 *  FLUID_CHORUS_DEPTH, chorus depth.
 *  FLUID_CHORUS_TYPE, chorus waveform type.
 * @param value pointer on the value to return.
 * @return FLUID_OK if success, FLUID_FAILED otherwise.
 */
static int fluid_synth_chorus_get_param(fluid_synth_t *synth, int fx_group,
                                        int param, double *value)
{
    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_return_val_if_fail((param >= 0) && (param < FLUID_CHORUS_PARAM_LAST), FLUID_FAILED);
    fluid_return_val_if_fail(value != NULL, FLUID_FAILED);
    fluid_synth_api_enter(synth);

    if(fx_group  < -1 || fx_group >= synth->effects_groups)
    {
        FLUID_API_RETURN(FLUID_FAILED);
    }

    if (fx_group < 0)
    {
        /* return chorus param common to all fx groups */
        *value = synth->chorus_param[param];
    }
    else
    {
        /* return chorus param of fx group at index group */
        *value = fluid_rvoice_mixer_chorus_get_param(synth->eventhandler->mixer,
                                                     fx_group, param);
    }

    FLUID_API_RETURN(FLUID_OK);
}

/*
 * If the same note is hit twice on the same channel, then the older
 * voice process is advanced to the release stage.  Using a mechanical
 * MIDI controller, the only way this can happen is when the sustain
 * pedal is held.  In this case the behaviour implemented here is
 * natural for many instruments.  Note: One noteon event can trigger
 * several voice processes, for example a stereo sample.  Don't
 * release those...
 */
void
fluid_synth_release_voice_on_same_note_LOCAL(fluid_synth_t *synth, int chan,
        int key)
{
    int i;
    fluid_voice_t *voice;

    /* storeid is a parameter for fluid_voice_init() */
    synth->storeid = synth->noteid++;

    /* for "monophonic playing" key is the previous sustained note
      if it exists (0 to 127) or INVALID_NOTE otherwise */
    if(key == INVALID_NOTE)
    {
        return;
    }

    for(i = 0; i < synth->polyphony; i++)
    {
        voice = synth->voice[i];

        if(fluid_voice_is_playing(voice)
                && (fluid_voice_get_channel(voice) == chan)
                && (fluid_voice_get_key(voice) == key)
                && (fluid_voice_get_id(voice) != synth->noteid))
        {
            /* Id of voices that was sustained by sostenuto */
            if(fluid_voice_is_sostenuto(voice))
            {
                synth->storeid = fluid_voice_get_id(voice);
            }

            /* Force the voice into release stage except if pedaling
               (sostenuto or sustain) is active */
            fluid_voice_noteoff(voice);
        }
    }
}

/**
 * Set synthesis interpolation method on one or all MIDI channels.
 * @param synth FluidSynth instance
 * @param chan MIDI channel to set interpolation method on or -1 for all channels
 * @param interp_method Interpolation method (#fluid_interp)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int
fluid_synth_set_interp_method(fluid_synth_t *synth, int chan, int interp_method)
{
    int i;

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_synth_api_enter(synth);

    if(chan < -1 || chan >= synth->midi_channels)
    {
        FLUID_API_RETURN(FLUID_FAILED);
    }

    if(synth->channel[0] == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Channels don't exist (yet)!");
        FLUID_API_RETURN(FLUID_FAILED);
    }

    for(i = 0; i < synth->midi_channels; i++)
    {
        if(chan < 0 || fluid_channel_get_num(synth->channel[i]) == chan)
        {
            fluid_channel_set_interp_method(synth->channel[i], interp_method);
        }
    }

    FLUID_API_RETURN(FLUID_OK);
};

/**
 * Get the total count of MIDI channels.
 * @param synth FluidSynth instance
 * @return Count of MIDI channels
 */
int
fluid_synth_count_midi_channels(fluid_synth_t *synth)
{
    int result;
    fluid_return_val_if_fail(synth != NULL, 0);
    fluid_synth_api_enter(synth);

    result = synth->midi_channels;
    FLUID_API_RETURN(result);
}

/**
 * Get the total count of audio channels.
 * @param synth FluidSynth instance
 * @return Count of audio channel stereo pairs (1 = 2 channels, 2 = 4, etc)
 */
int
fluid_synth_count_audio_channels(fluid_synth_t *synth)
{
    int result;
    fluid_return_val_if_fail(synth != NULL, 0);
    fluid_synth_api_enter(synth);

    result = synth->audio_channels;
    FLUID_API_RETURN(result);
}

/**
 * Get the total number of allocated audio channels.  Usually identical to the
 * number of audio channels.  Can be employed by LADSPA effects subsystem.
 *
 * @param synth FluidSynth instance
 * @return Count of audio group stereo pairs (1 = 2 channels, 2 = 4, etc)
 */
int
fluid_synth_count_audio_groups(fluid_synth_t *synth)
{
    int result;
    fluid_return_val_if_fail(synth != NULL, 0);
    fluid_synth_api_enter(synth);

    result = synth->audio_groups;
    FLUID_API_RETURN(result);
}

/**
 * Get the total number of allocated effects channels.
 * @param synth FluidSynth instance
 * @return Count of allocated effects channels
 */
int
fluid_synth_count_effects_channels(fluid_synth_t *synth)
{
    int result;
    fluid_return_val_if_fail(synth != NULL, 0);
    fluid_synth_api_enter(synth);

    result = synth->effects_channels;
    FLUID_API_RETURN(result);
}

/**
 * Get the total number of allocated effects units.
 * 
 * This is the same number as initially provided by the setting \setting{synth_effects-groups}.
 * @param synth FluidSynth instance
 * @return Count of allocated effects units
 */
int
fluid_synth_count_effects_groups(fluid_synth_t *synth)
{
    int result;
    fluid_return_val_if_fail(synth != NULL, 0);
    fluid_synth_api_enter(synth);

    result = synth->effects_groups;
    FLUID_API_RETURN(result);
}

/**
 * Get the synth CPU load value.
 * @param synth FluidSynth instance
 * @return Estimated CPU load value in percent (0-100)
 */
double
fluid_synth_get_cpu_load(fluid_synth_t *synth)
{
    fluid_return_val_if_fail(synth != NULL, 0);
    return fluid_atomic_float_get(&synth->cpu_load);
}

/* Get tuning for a given bank:program */
static fluid_tuning_t *
fluid_synth_get_tuning(fluid_synth_t *synth, int bank, int prog)
{

    if((synth->tuning == NULL) ||
            (synth->tuning[bank] == NULL) ||
            (synth->tuning[bank][prog] == NULL))
    {
        return NULL;
    }

    return synth->tuning[bank][prog];
}

/* Replace tuning on a given bank:program (need not already exist).
 * Synth mutex should already be locked by caller. */
static int
fluid_synth_replace_tuning_LOCK(fluid_synth_t *synth, fluid_tuning_t *tuning,
                                int bank, int prog, int apply)
{
    fluid_tuning_t *old_tuning;

    if(synth->tuning == NULL)
    {
        synth->tuning = FLUID_ARRAY(fluid_tuning_t **, 128);

        if(synth->tuning == NULL)
        {
            FLUID_LOG(FLUID_PANIC, "Out of memory");
            return FLUID_FAILED;
        }

        FLUID_MEMSET(synth->tuning, 0, 128 * sizeof(fluid_tuning_t **));
    }

    if(synth->tuning[bank] == NULL)
    {
        synth->tuning[bank] = FLUID_ARRAY(fluid_tuning_t *, 128);

        if(synth->tuning[bank] == NULL)
        {
            FLUID_LOG(FLUID_PANIC, "Out of memory");
            return FLUID_FAILED;
        }

        FLUID_MEMSET(synth->tuning[bank], 0, 128 * sizeof(fluid_tuning_t *));
    }

    old_tuning = synth->tuning[bank][prog];
    synth->tuning[bank][prog] = tuning;

    if(old_tuning)
    {
        if(!fluid_tuning_unref(old_tuning, 1))       /* -- unref old tuning */
        {
            /* Replace old tuning if present */
            fluid_synth_replace_tuning_LOCAL(synth, old_tuning, tuning, apply, FALSE);
        }
    }

    return FLUID_OK;
}

/* Replace a tuning with a new one in all MIDI channels.  new_tuning can be
 * NULL, in which case channels are reset to default equal tempered scale. */
static void
fluid_synth_replace_tuning_LOCAL(fluid_synth_t *synth, fluid_tuning_t *old_tuning,
                                 fluid_tuning_t *new_tuning, int apply, int unref_new)
{
    fluid_channel_t *channel;
    int old_tuning_unref = 0;
    int i;

    for(i = 0; i < synth->midi_channels; i++)
    {
        channel = synth->channel[i];

        if(fluid_channel_get_tuning(channel) == old_tuning)
        {
            old_tuning_unref++;

            if(new_tuning)
            {
                fluid_tuning_ref(new_tuning);    /* ++ ref new tuning for channel */
            }

            fluid_channel_set_tuning(channel, new_tuning);

            if(apply)
            {
                fluid_synth_update_voice_tuning_LOCAL(synth, channel);
            }
        }
    }

    /* Send unref old tuning event if any unrefs */
    if(old_tuning && old_tuning_unref)
    {
        fluid_tuning_unref(old_tuning, old_tuning_unref);
    }

    if(!unref_new || !new_tuning)
    {
        return;
    }

    fluid_tuning_unref(new_tuning, 1);
}

/* Update voice tunings in realtime */
static void
fluid_synth_update_voice_tuning_LOCAL(fluid_synth_t *synth, fluid_channel_t *channel)
{
    fluid_voice_t *voice;
    int i;

    for(i = 0; i < synth->polyphony; i++)
    {
        voice = synth->voice[i];

        if(fluid_voice_is_on(voice) && (voice->channel == channel))
        {
            fluid_voice_calculate_gen_pitch(voice);
            fluid_voice_update_param(voice, GEN_PITCH);
        }
    }
}

/**
 * Set the tuning of the entire MIDI note scale.
 * @param synth FluidSynth instance
 * @param bank Tuning bank number (0-127), not related to MIDI instrument bank
 * @param prog Tuning preset number (0-127), not related to MIDI instrument program
 * @param name Label name for this tuning
 * @param pitch Array of pitch values (length of 128, each value is number of
 *   cents, for example normally note 0 is 0.0, 1 is 100.0, 60 is 6000.0, etc).
 *   Pass NULL to create a equal tempered (normal) scale.
 * @param apply TRUE to apply new tuning in realtime to existing notes which
 *   are using the replaced tuning (if any), FALSE otherwise
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @since 1.1.0
 */
int
fluid_synth_activate_key_tuning(fluid_synth_t *synth, int bank, int prog,
                                const char *name, const double *pitch, int apply)
{
    fluid_tuning_t *tuning;
    int retval = FLUID_OK;

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(bank >= 0 && bank < 128, FLUID_FAILED);
    fluid_return_val_if_fail(prog >= 0 && prog < 128, FLUID_FAILED);
    fluid_return_val_if_fail(name != NULL, FLUID_FAILED);

    fluid_synth_api_enter(synth);

    tuning = new_fluid_tuning(name, bank, prog);

    if(tuning)
    {
        if(pitch)
        {
            fluid_tuning_set_all(tuning, pitch);
        }

        retval = fluid_synth_replace_tuning_LOCK(synth, tuning, bank, prog, apply);

        if(retval == FLUID_FAILED)
        {
            fluid_tuning_unref(tuning, 1);
        }
    }
    else
    {
        retval = FLUID_FAILED;
    }

    FLUID_API_RETURN(retval);
}

/**
 * Activate an octave tuning on every octave in the MIDI note scale.
 * @param synth FluidSynth instance
 * @param bank Tuning bank number (0-127), not related to MIDI instrument bank
 * @param prog Tuning preset number (0-127), not related to MIDI instrument program
 * @param name Label name for this tuning
 * @param pitch Array of pitch values (length of 12 for each note of an octave
 *   starting at note C, values are number of offset cents to add to the normal
 *   tuning amount)
 * @param apply TRUE to apply new tuning in realtime to existing notes which
 *   are using the replaced tuning (if any), FALSE otherwise
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @since 1.1.0
 */
int
fluid_synth_activate_octave_tuning(fluid_synth_t *synth, int bank, int prog,
                                   const char *name, const double *pitch, int apply)
{
    fluid_tuning_t *tuning;
    int retval = FLUID_OK;

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(bank >= 0 && bank < 128, FLUID_FAILED);
    fluid_return_val_if_fail(prog >= 0 && prog < 128, FLUID_FAILED);
    fluid_return_val_if_fail(name != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(pitch != NULL, FLUID_FAILED);

    fluid_synth_api_enter(synth);
    tuning = new_fluid_tuning(name, bank, prog);

    if(tuning)
    {
        fluid_tuning_set_octave(tuning, pitch);
        retval = fluid_synth_replace_tuning_LOCK(synth, tuning, bank, prog, apply);

        if(retval == FLUID_FAILED)
        {
            fluid_tuning_unref(tuning, 1);
        }
    }
    else
    {
        retval = FLUID_FAILED;
    }

    FLUID_API_RETURN(retval);
}

/**
 * Set tuning values for one or more MIDI notes for an existing tuning.
 * @param synth FluidSynth instance
 * @param bank Tuning bank number (0-127), not related to MIDI instrument bank
 * @param prog Tuning preset number (0-127), not related to MIDI instrument program
 * @param len Number of MIDI notes to assign
 * @param key Array of MIDI key numbers (length of 'len', values 0-127)
 * @param pitch Array of pitch values (length of 'len', values are number of
 *   cents from MIDI note 0)
 * @param apply TRUE to apply tuning change in realtime to existing notes using
 *   the specified tuning, FALSE otherwise
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 *
 * @note Prior to version 1.1.0 it was an error to specify a tuning that didn't
 * already exist. Starting with 1.1.0, the default equal tempered scale will be
 * used as a basis, if no tuning exists for the given bank and prog.
 */
int
fluid_synth_tune_notes(fluid_synth_t *synth, int bank, int prog,
                       int len, const int *key, const double *pitch, int apply)
{
    fluid_tuning_t *old_tuning, *new_tuning;
    int retval = FLUID_OK;
    int i;

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(bank >= 0 && bank < 128, FLUID_FAILED);
    fluid_return_val_if_fail(prog >= 0 && prog < 128, FLUID_FAILED);
    fluid_return_val_if_fail(len > 0, FLUID_FAILED);
    fluid_return_val_if_fail(key != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(pitch != NULL, FLUID_FAILED);

    fluid_synth_api_enter(synth);

    old_tuning = fluid_synth_get_tuning(synth, bank, prog);

    if(old_tuning)
    {
        new_tuning = fluid_tuning_duplicate(old_tuning);
    }
    else
    {
        new_tuning = new_fluid_tuning("Unnamed", bank, prog);
    }

    if(new_tuning)
    {
        for(i = 0; i < len; i++)
        {
            fluid_tuning_set_pitch(new_tuning, key[i], pitch[i]);
        }

        retval = fluid_synth_replace_tuning_LOCK(synth, new_tuning, bank, prog, apply);

        if(retval == FLUID_FAILED)
        {
            fluid_tuning_unref(new_tuning, 1);
        }
    }
    else
    {
        retval = FLUID_FAILED;
    }

    FLUID_API_RETURN(retval);
}

/**
 * Activate a tuning scale on a MIDI channel.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param bank Tuning bank number (0-127), not related to MIDI instrument bank
 * @param prog Tuning preset number (0-127), not related to MIDI instrument program
 * @param apply TRUE to apply tuning change to active notes, FALSE otherwise
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @since 1.1.0
 *
 * @note A default equal tempered scale will be created, if no tuning exists
 * on the given bank and prog.
 */
int
fluid_synth_activate_tuning(fluid_synth_t *synth, int chan, int bank, int prog,
                            int apply)
{
    fluid_tuning_t *tuning;
    int retval = FLUID_OK;

    //fluid_return_val_if_fail (synth != NULL, FLUID_FAILED);
    //fluid_return_val_if_fail (chan >= 0 && chan < synth->midi_channels, FLUID_FAILED);
    fluid_return_val_if_fail(bank >= 0 && bank < 128, FLUID_FAILED);
    fluid_return_val_if_fail(prog >= 0 && prog < 128, FLUID_FAILED);

    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    tuning = fluid_synth_get_tuning(synth, bank, prog);

    /* If no tuning exists, create a new default tuning.  We do this, so that
     * it can be replaced later, if any changes are made. */
    if(!tuning)
    {
        tuning = new_fluid_tuning("Unnamed", bank, prog);

        if(tuning)
        {
            fluid_synth_replace_tuning_LOCK(synth, tuning, bank, prog, FALSE);
        }
    }

    if(tuning)
    {
        fluid_tuning_ref(tuning);    /* ++ ref for outside of lock */
    }

    if(!tuning)
    {
        FLUID_API_RETURN(FLUID_FAILED);
    }

    fluid_tuning_ref(tuning);     /* ++ ref new tuning for following function */
    retval = fluid_synth_set_tuning_LOCAL(synth, chan, tuning, apply);

    fluid_tuning_unref(tuning, 1);    /* -- unref for outside of lock */

    FLUID_API_RETURN(retval);
}

/* Local synthesis thread set tuning function (takes over tuning reference) */
static int
fluid_synth_set_tuning_LOCAL(fluid_synth_t *synth, int chan,
                             fluid_tuning_t *tuning, int apply)
{
    fluid_tuning_t *old_tuning;
    fluid_channel_t *channel;

    channel = synth->channel[chan];

    old_tuning = fluid_channel_get_tuning(channel);
    fluid_channel_set_tuning(channel, tuning);    /* !! Takes over callers reference */

    if(apply)
    {
        fluid_synth_update_voice_tuning_LOCAL(synth, channel);
    }

    /* Send unref old tuning event */
    if(old_tuning)
    {
        fluid_tuning_unref(old_tuning, 1);
    }


    return FLUID_OK;
}

/**
 * Clear tuning scale on a MIDI channel (use default equal tempered scale).
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param apply TRUE to apply tuning change to active notes, FALSE otherwise
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @since 1.1.0
 */
int
fluid_synth_deactivate_tuning(fluid_synth_t *synth, int chan, int apply)
{
    int retval = FLUID_OK;

    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    retval = fluid_synth_set_tuning_LOCAL(synth, chan, NULL, apply);

    FLUID_API_RETURN(retval);
}

/**
 * Start tuning iteration.
 * @param synth FluidSynth instance
 */
void
fluid_synth_tuning_iteration_start(fluid_synth_t *synth)
{
    fluid_return_if_fail(synth != NULL);
    fluid_synth_api_enter(synth);
    fluid_private_set(synth->tuning_iter, FLUID_INT_TO_POINTER(0));
    fluid_synth_api_exit(synth);
}

/**
 * Advance to next tuning.
 * @param synth FluidSynth instance
 * @param bank Location to store MIDI bank number of next tuning scale
 * @param prog Location to store MIDI program number of next tuning scale
 * @return 1 if tuning iteration advanced, 0 if no more tunings
 */
int
fluid_synth_tuning_iteration_next(fluid_synth_t *synth, int *bank, int *prog)
{
    void *pval;
    int b = 0, p = 0;

    fluid_return_val_if_fail(synth != NULL, 0);
    fluid_return_val_if_fail(bank != NULL, 0);
    fluid_return_val_if_fail(prog != NULL, 0);
    fluid_synth_api_enter(synth);

    /* Current tuning iteration stored as: bank << 8 | program */
    pval = fluid_private_get(synth->tuning_iter);
    p = FLUID_POINTER_TO_INT(pval);
    b = (p >> 8) & 0xFF;
    p &= 0xFF;

    if(!synth->tuning)
    {
        FLUID_API_RETURN(0);
    }

    for(; b < 128; b++, p = 0)
    {
        if(synth->tuning[b] == NULL)
        {
            continue;
        }

        for(; p < 128; p++)
        {
            if(synth->tuning[b][p] == NULL)
            {
                continue;
            }

            *bank = b;
            *prog = p;

            if(p < 127)
            {
                fluid_private_set(synth->tuning_iter,
                                  FLUID_INT_TO_POINTER(b << 8 | (p + 1)));
            }
            else
            {
                fluid_private_set(synth->tuning_iter, FLUID_INT_TO_POINTER((b + 1) << 8));
            }

            FLUID_API_RETURN(1);
        }
    }

    FLUID_API_RETURN(0);
}

/**
 * Get the entire note tuning for a given MIDI bank and program.
 * @param synth FluidSynth instance
 * @param bank MIDI bank number of tuning
 * @param prog MIDI program number of tuning
 * @param name Location to store tuning name or NULL to ignore
 * @param len Maximum number of chars to store to 'name' (including NULL byte)
 * @param pitch Array to store tuning scale to or NULL to ignore (len of 128)
 * @return #FLUID_OK if matching tuning was found, #FLUID_FAILED otherwise
 */
int
fluid_synth_tuning_dump(fluid_synth_t *synth, int bank, int prog,
                        char *name, int len, double *pitch)
{
    fluid_tuning_t *tuning;

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_synth_api_enter(synth);

    tuning = fluid_synth_get_tuning(synth, bank, prog);

    if(tuning)
    {
        if(name)
        {
            FLUID_SNPRINTF(name, len - 1, "%s", fluid_tuning_get_name(tuning));
            name[len - 1] = 0;  /* make sure the string is null terminated */
        }

        if(pitch)
        {
            FLUID_MEMCPY(pitch, fluid_tuning_get_all(tuning), 128 * sizeof(double));
        }
    }

    FLUID_API_RETURN(tuning ? FLUID_OK : FLUID_FAILED);
}

/**
 * Get settings assigned to a synth.
 * @param synth FluidSynth instance
 * @return FluidSynth settings which are assigned to the synth
 */
fluid_settings_t *
fluid_synth_get_settings(fluid_synth_t *synth)
{
    fluid_return_val_if_fail(synth != NULL, NULL);

    return synth->settings;
}

/**
 * Apply an offset to a SoundFont generator on a MIDI channel.
 *
 * This function allows to set an offset for the specified destination generator in real-time.
 * The offset will be applied immediately to all voices that are currently and subsequently playing
 * on the given MIDI channel. This functionality works equivalent to using NRPN MIDI messages to
 * manipulate synthesis parameters. See SoundFont spec, paragraph 8.1.3, for details on SoundFont
 * generator parameters and valid ranges, as well as paragraph 9.6 for details on NRPN messages.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param param SoundFont generator ID (#fluid_gen_type)
 * @param value Offset value (in native units of the generator) to assign to the MIDI channel
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int fluid_synth_set_gen(fluid_synth_t *synth, int chan, int param, float value)
{
    fluid_return_val_if_fail(param >= 0 && param < GEN_LAST, FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    fluid_synth_set_gen_LOCAL(synth, chan, param, value);

    FLUID_API_RETURN(FLUID_OK);
}

/* Synthesis thread local set gen function */
static void
fluid_synth_set_gen_LOCAL(fluid_synth_t *synth, int chan, int param, float value)
{
    fluid_voice_t *voice;
    int i;

    fluid_channel_set_gen(synth->channel[chan], param, value);

    for(i = 0; i < synth->polyphony; i++)
    {
        voice = synth->voice[i];

        if(fluid_voice_get_channel(voice) == chan)
        {
            fluid_voice_set_param(voice, param, value);
        }
    }
}

/**
 * Retrieve the generator NRPN offset assigned to a MIDI channel.
 *
 * The value returned is in native units of the generator. By default, the offset is zero.
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param param SoundFont generator ID (#fluid_gen_type)
 * @return Current NRPN generator offset value assigned to the MIDI channel
 */
float
fluid_synth_get_gen(fluid_synth_t *synth, int chan, int param)
{
    float result;
    fluid_return_val_if_fail(param >= 0 && param < GEN_LAST, FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    result = fluid_channel_get_gen(synth->channel[chan], param);
    FLUID_API_RETURN(result);
}

/**
 * Handle MIDI event from MIDI router, used as a callback function.
 * @param data FluidSynth instance
 * @param event MIDI event to handle
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int
fluid_synth_handle_midi_event(void *data, fluid_midi_event_t *event)
{
    fluid_synth_t *synth = (fluid_synth_t *) data;
    int type = fluid_midi_event_get_type(event);
    int chan = fluid_midi_event_get_channel(event);

    switch(type)
    {
    case NOTE_ON:
        return fluid_synth_noteon(synth, chan,
                                  fluid_midi_event_get_key(event),
                                  fluid_midi_event_get_velocity(event));

    case NOTE_OFF:
        return fluid_synth_noteoff(synth, chan, fluid_midi_event_get_key(event));

    case CONTROL_CHANGE:
        return fluid_synth_cc(synth, chan,
                              fluid_midi_event_get_control(event),
                              fluid_midi_event_get_value(event));

    case PROGRAM_CHANGE:
        return fluid_synth_program_change(synth, chan, fluid_midi_event_get_program(event));

    case CHANNEL_PRESSURE:
        return fluid_synth_channel_pressure(synth, chan, fluid_midi_event_get_program(event));

    case KEY_PRESSURE:
        return fluid_synth_key_pressure(synth, chan,
                                        fluid_midi_event_get_key(event),
                                        fluid_midi_event_get_value(event));

    case PITCH_BEND:
        return fluid_synth_pitch_bend(synth, chan, fluid_midi_event_get_pitch(event));

    case MIDI_SYSTEM_RESET:
        return fluid_synth_system_reset(synth);

    case MIDI_SYSEX:
        return fluid_synth_sysex(synth, event->paramptr, event->param1, NULL, NULL, NULL, FALSE);

    case MIDI_TEXT:
    case MIDI_LYRIC:
    case MIDI_SET_TEMPO:
        return FLUID_OK;
    }

    return FLUID_FAILED;
}

/**
 * Create and start voices using an arbitrary preset and a MIDI note on event.
 *
 * Using this function is only supported when the setting @c synth.dynamic-sample-loading is false!
 * @param synth FluidSynth instance
 * @param id Voice group ID to use (can be used with fluid_synth_stop()).
 * @param preset Preset to synthesize
 * @param audio_chan Unused currently, set to 0
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param key MIDI note number (0-127)
 * @param vel MIDI velocity number (1-127)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 *
 * @note Should only be called from within synthesis thread, which includes
 * SoundFont loader preset noteon method.
 */
int
fluid_synth_start(fluid_synth_t *synth, unsigned int id, fluid_preset_t *preset,
                  int audio_chan, int chan, int key, int vel)
{
    int result, dynamic_samples;
    fluid_return_val_if_fail(preset != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(key >= 0 && key <= 127, FLUID_FAILED);
    fluid_return_val_if_fail(vel >= 1 && vel <= 127, FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    fluid_settings_getint(fluid_synth_get_settings(synth), "synth.dynamic-sample-loading", &dynamic_samples);
    if(dynamic_samples)
    {
        // The preset might not be currently used, thus its sample data may not be loaded.
        // This guard is to avoid a NULL deref in rvoice_write().
        FLUID_LOG(FLUID_ERR, "Calling fluid_synth_start() while synth.dynamic-sample-loading is enabled is not supported.");
        // Although we would be able to select the preset (and load it's samples) we have no way to
        // unselect the preset again in fluid_synth_stop(). Also dynamic sample loading was intended
        // to be used only when presets have been selected on a MIDI channel.
        // Note that even if the preset is currently selected on a channel, it could be unselected at
        // any time. And we would end up with a NULL sample->data again, because we are not referencing
        // the preset here. Thus failure is our only option.
        result = FLUID_FAILED;
    }
    else
    {
        synth->storeid = id;
        result = fluid_preset_noteon(preset, synth, chan, key, vel);
    }

    FLUID_API_RETURN(result);
}

/**
 * Stop notes for a given note event voice ID.
 * @param synth FluidSynth instance
 * @param id Voice note event ID
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 *
 * @note In FluidSynth versions prior to 1.1.0 #FLUID_FAILED would be returned
 * if no matching voice note event ID was found.  Versions after 1.1.0 only
 * return #FLUID_FAILED if an error occurs.
 */
int
fluid_synth_stop(fluid_synth_t *synth, unsigned int id)
{
    int result;
    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_synth_api_enter(synth);
    fluid_synth_stop_LOCAL(synth, id);
    result = FLUID_OK;
    FLUID_API_RETURN(result);
}

/* Local synthesis thread variant of fluid_synth_stop */
static void
fluid_synth_stop_LOCAL(fluid_synth_t *synth, unsigned int id)
{
    fluid_voice_t *voice;
    int i;

    for(i = 0; i < synth->polyphony; i++)
    {
        voice = synth->voice[i];

        if(fluid_voice_is_on(voice) && (fluid_voice_get_id(voice) == id))
        {
            fluid_voice_noteoff(voice);
        }
    }
}

/**
 * Offset the bank numbers of a loaded SoundFont, i.e.\ subtract
 * \c offset from any bank number when assigning instruments.
 *
 * @param synth FluidSynth instance
 * @param sfont_id ID of a loaded SoundFont
 * @param offset Bank offset value to apply to all instruments
 * @return #FLUID_OK if the offset was set successfully, #FLUID_FAILED otherwise
 */
int
fluid_synth_set_bank_offset(fluid_synth_t *synth, int sfont_id, int offset)
{
    fluid_sfont_t *sfont;
    fluid_list_t *list;

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_synth_api_enter(synth);

    for(list = synth->sfont; list; list = fluid_list_next(list))
    {
        sfont = fluid_list_get(list);

        if(fluid_sfont_get_id(sfont) == sfont_id)
        {
            sfont->bankofs = offset;
            break;
        }
    }

    if(!list)
    {
        FLUID_LOG(FLUID_ERR, "No SoundFont with id = %d", sfont_id);
        FLUID_API_RETURN(FLUID_FAILED);
    }

    FLUID_API_RETURN(FLUID_OK);
}

/**
 * Get bank offset of a loaded SoundFont.
 * @param synth FluidSynth instance
 * @param sfont_id ID of a loaded SoundFont
 * @return SoundFont bank offset value
 */
int
fluid_synth_get_bank_offset(fluid_synth_t *synth, int sfont_id)
{
    fluid_sfont_t *sfont;
    fluid_list_t *list;
    int offset = 0;

    fluid_return_val_if_fail(synth != NULL, 0);
    fluid_synth_api_enter(synth);

    for(list = synth->sfont; list; list = fluid_list_next(list))
    {
        sfont = fluid_list_get(list);

        if(fluid_sfont_get_id(sfont) == sfont_id)
        {
            offset = sfont->bankofs;
            break;
        }
    }

    if(!list)
    {
        FLUID_LOG(FLUID_ERR, "No SoundFont with id = %d", sfont_id);
        FLUID_API_RETURN(0);
    }

    FLUID_API_RETURN(offset);
}

void
fluid_synth_api_enter(fluid_synth_t *synth)
{
    if(synth->use_mutex)
    {
        fluid_rec_mutex_lock(synth->mutex);
    }

    if(!synth->public_api_count)
    {
        fluid_synth_check_finished_voices(synth);
    }

    synth->public_api_count++;
}

void fluid_synth_api_exit(fluid_synth_t *synth)
{
    synth->public_api_count--;

    if(!synth->public_api_count)
    {
        fluid_rvoice_eventhandler_flush(synth->eventhandler);
    }

    if(synth->use_mutex)
    {
        fluid_rec_mutex_unlock(synth->mutex);
    }

}

/**
 * Set midi channel type
 * @param synth FluidSynth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param type MIDI channel type (#fluid_midi_channel_type)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @since 1.1.4
 */
int fluid_synth_set_channel_type(fluid_synth_t *synth, int chan, int type)
{
    fluid_return_val_if_fail((type >= CHANNEL_TYPE_MELODIC) && (type <= CHANNEL_TYPE_DRUM), FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    synth->channel[chan]->channel_type = type;

    FLUID_API_RETURN(FLUID_OK);
}

/**
 * Return the LADSPA effects instance used by FluidSynth
 *
 * @param synth FluidSynth instance
 * @return pointer to LADSPA fx or NULL if compiled without LADSPA support or LADSPA is not active
 */
fluid_ladspa_fx_t *fluid_synth_get_ladspa_fx(fluid_synth_t *synth)
{
    fluid_return_val_if_fail(synth != NULL, NULL);

    return synth->ladspa_fx;
}

/**
 * Configure a general-purpose IIR biquad filter.
 *
 * @param synth FluidSynth instance
 * @param type Type of the IIR filter to use (see #fluid_iir_filter_type)
 * @param flags Additional flags to customize this filter or zero to stay with the default (see #fluid_iir_filter_flags)
 * @return #FLUID_OK if the settings have been successfully applied, otherwise #FLUID_FAILED
 *
 * This is an optional, additional filter that operates independently from the default low-pass filter required by the Soundfont2 standard.
 * By default this filter is off (#FLUID_IIR_DISABLED).
 */
int fluid_synth_set_custom_filter(fluid_synth_t *synth, int type, int flags)
{
    int i;
    fluid_voice_t *voice;

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(type >= FLUID_IIR_DISABLED && type < FLUID_IIR_LAST, FLUID_FAILED);

    fluid_synth_api_enter(synth);

    synth->custom_filter_type = type;
    synth->custom_filter_flags = flags;

    for(i = 0; i < synth->polyphony; i++)
    {
        voice = synth->voice[i];

        fluid_voice_set_custom_filter(voice, type, flags);
    }

    FLUID_API_RETURN(FLUID_OK);
}

/**
 * Set the important channels for voice overflow priority calculation.
 *
 * @param synth FluidSynth instance
 * @param channels comma-separated list of channel numbers
 * @return #FLUID_OK on success, otherwise #FLUID_FAILED
 */
static int fluid_synth_set_important_channels(fluid_synth_t *synth, const char *channels)
{
    int i;
    int retval = FLUID_FAILED;
    int *values = NULL;
    int num_values;
    fluid_overflow_prio_t *scores;

    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);

    scores = &synth->overflow;

    if(scores->num_important_channels < synth->midi_channels)
    {
        scores->important_channels = FLUID_REALLOC(scores->important_channels,
                                     sizeof(*scores->important_channels) * synth->midi_channels);

        if(scores->important_channels == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            goto exit;
        }

        scores->num_important_channels = synth->midi_channels;
    }

    FLUID_MEMSET(scores->important_channels, FALSE,
                 sizeof(*scores->important_channels) * scores->num_important_channels);

    if(channels != NULL)
    {
        values = FLUID_ARRAY(int, synth->midi_channels);

        if(values == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            goto exit;
        }

        /* Every channel given in the comma-separated list of channel numbers
         * is set to TRUE, i.e. flagging it as "important". Channel numbers are
         * 1-based. */
        num_values = fluid_settings_split_csv(channels, values, synth->midi_channels);

        for(i = 0; i < num_values; i++)
        {
            if(values[i] > 0 && values[i] <= synth->midi_channels)
            {
                scores->important_channels[values[i] - 1] = TRUE;
            }
        }
    }

    retval = FLUID_OK;

exit:
    FLUID_FREE(values);
    return retval;
}

/*
 * Handler for synth.overflow.important-channels setting.
 */
static void fluid_synth_handle_important_channels(void *data, const char *name,
        const char *value)
{
    fluid_synth_t *synth = (fluid_synth_t *)data;

    fluid_synth_api_enter(synth);
    fluid_synth_set_important_channels(synth, value);
    fluid_synth_api_exit(synth);
}


/* API legato mode *********************************************************/

/**
 * Sets the legato mode of a channel.
 *
 * @param synth the synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param legatomode The legato mode as indicated by #fluid_channel_legato_mode.
 *
 * @return
 * - #FLUID_OK on success.
 * - #FLUID_FAILED
 *   - \a synth is NULL.
 *   - \a chan is outside MIDI channel count.
 *   - \a legatomode is invalid.
 */
int fluid_synth_set_legato_mode(fluid_synth_t *synth, int chan, int legatomode)
{
    /* checks parameters first */
    fluid_return_val_if_fail(legatomode >= 0, FLUID_FAILED);
    fluid_return_val_if_fail(legatomode < FLUID_CHANNEL_LEGATO_MODE_LAST, FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);
    /**/
    synth->channel[chan]->legatomode = legatomode;
    /**/
    FLUID_API_RETURN(FLUID_OK);
}

/**
 * Gets the legato mode of a channel.
 *
 * @param synth the synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param legatomode The legato mode as indicated by #fluid_channel_legato_mode.
 *
 * @return
 * - #FLUID_OK on success.
 * - #FLUID_FAILED
 *   - \a synth is NULL.
 *   - \a chan is outside MIDI channel count.
 *   - \a legatomode is NULL.
 */
int fluid_synth_get_legato_mode(fluid_synth_t *synth, int chan, int *legatomode)
{
    /* checks parameters first */
    fluid_return_val_if_fail(legatomode != NULL, FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);
    /**/
    * legatomode = synth->channel[chan]->legatomode;
    /**/
    FLUID_API_RETURN(FLUID_OK);
}

/* API portamento mode *********************************************************/

/**
 * Sets the portamento mode of a channel.
 *
 * @param synth the synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param portamentomode The portamento mode as indicated by #fluid_channel_portamento_mode.
 * @return
 * - #FLUID_OK on success.
 * - #FLUID_FAILED
 *   - \a synth is NULL.
 *   - \a chan is outside MIDI channel count.
 *   - \a portamentomode is invalid.
 */
int fluid_synth_set_portamento_mode(fluid_synth_t *synth, int chan,
                                    int portamentomode)
{
    /* checks parameters first */
    fluid_return_val_if_fail(portamentomode >= 0, FLUID_FAILED);
    fluid_return_val_if_fail(portamentomode < FLUID_CHANNEL_PORTAMENTO_MODE_LAST, FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);
    /**/
    synth->channel[chan]->portamentomode = portamentomode;
    /**/
    FLUID_API_RETURN(FLUID_OK);
}

/**
 * Gets the portamento mode of a channel.
 *
 * @param synth the synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param portamentomode Pointer to the portamento mode as indicated by #fluid_channel_portamento_mode.
 * @return
 * - #FLUID_OK on success.
 * - #FLUID_FAILED
 *   - \a synth is NULL.
 *   - \a chan is outside MIDI channel count.
 *   - \a portamentomode is NULL.
 */
int fluid_synth_get_portamento_mode(fluid_synth_t *synth, int chan,
                                    int *portamentomode)
{
    /* checks parameters first */
    fluid_return_val_if_fail(portamentomode != NULL, FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);
    /**/
    * portamentomode = synth->channel[chan]->portamentomode;
    /**/
    FLUID_API_RETURN(FLUID_OK);
}

/*  API breath mode *********************************************************/

/**
 * Sets the breath mode of a channel.
 *
 * @param synth the synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param breathmode The breath mode as indicated by #fluid_channel_breath_flags.
 *
 * @return
 * - #FLUID_OK on success.
 * - #FLUID_FAILED
 *   - \a synth is NULL.
 *   - \a chan is outside MIDI channel count.
 */
int fluid_synth_set_breath_mode(fluid_synth_t *synth, int chan, int breathmode)
{
    /* checks parameters first */
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);
    /**/
    fluid_channel_set_breath_info(synth->channel[chan], breathmode);
    /**/
    FLUID_API_RETURN(FLUID_OK);
}

/**
 * Gets the breath mode of a channel.
 *
 * @param synth the synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param breathmode Pointer to the returned breath mode as indicated by #fluid_channel_breath_flags.
 *
 * @return
 * - #FLUID_OK on success.
 * - #FLUID_FAILED
 *   - \a synth is NULL.
 *   - \a chan is outside MIDI channel count.
 *   - \a breathmode is NULL.
 */
int fluid_synth_get_breath_mode(fluid_synth_t *synth, int chan, int *breathmode)
{
    /* checks parameters first */
    fluid_return_val_if_fail(breathmode != NULL, FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);
    /**/
    * breathmode = fluid_channel_get_breath_info(synth->channel[chan]);
    /**/
    FLUID_API_RETURN(FLUID_OK);
}

/**  API Poly/mono mode ******************************************************/

/*
 * Resets a basic channel group of MIDI channels.
 * @param synth the synth instance.
 * @param chan the beginning channel of the group.
 * @param nbr_chan the number of channel in the group.
*/
static void
fluid_synth_reset_basic_channel_LOCAL(fluid_synth_t *synth, int chan, int nbr_chan)
{
    int i;

    for(i = chan; i < chan + nbr_chan; i++)
    {
        fluid_channel_reset_basic_channel_info(synth->channel[i]);
        synth->channel[i]->mode_val = 0;
    }
}

/**
 * Disables and unassigns all channels from a basic channel group.
 *
 * @param synth The synth instance.
 * @param chan The basic channel of the group to reset or -1 to reset all channels.
 * @note By default (i.e. on creation after new_fluid_synth() and after fluid_synth_system_reset())
 * a synth instance has one basic channel at channel 0 in mode #FLUID_CHANNEL_MODE_OMNION_POLY.
 * All other channels belong to this basic channel group. Make sure to call this function before
 * setting any custom basic channel setup.
 *
 * @return
 *  - #FLUID_OK on success.
 *  - #FLUID_FAILED
 *    - \a synth is NULL.
 *    - \a chan is outside MIDI channel count.
 *    - \a chan isn't a basic channel.
 */
int fluid_synth_reset_basic_channel(fluid_synth_t *synth, int chan)
{
    int nbr_chan;

    /* checks parameters first */
    if(chan < 0)
    {
        fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);
        fluid_synth_api_enter(synth);
        /* The range is all MIDI channels from 0 to MIDI channel count -1 */
        chan = 0; /* beginning chan */
        nbr_chan =  synth->midi_channels; /* MIDI Channels number */
    }
    else
    {
        FLUID_API_ENTRY_CHAN(FLUID_FAILED);

        /* checks if chan is a basic channel */
        if(!(synth->channel[chan]->mode &  FLUID_CHANNEL_BASIC))
        {
            FLUID_API_RETURN(FLUID_FAILED);
        }

        /* The range is all MIDI channels in the group from chan */
        nbr_chan = synth->channel[chan]->mode_val; /* nbr of channels in the group */
    }

    /* resets the range of MIDI channels */
    fluid_synth_reset_basic_channel_LOCAL(synth, chan, nbr_chan);
    FLUID_API_RETURN(FLUID_OK);
}

/**
 * Checks if a new basic channel group overlaps the next basic channel group.
 *
 * On success the function returns the possible number of channel for this
 * new basic channel group.
 * The function fails if the new group overlaps the next basic channel group.
 *
 * @param see fluid_synth_set_basic_channel.
 * @return
 * - On success, the effective number of channels for this new basic channel group,
 *   #FLUID_FAILED otherwise.
 * - #FLUID_FAILED
 *   - \a val has a number of channels overlapping next basic channel group or been
 *     above MIDI channel count.
 */
static int
fluid_synth_check_next_basic_channel(fluid_synth_t *synth, int basicchan, int mode, int val)
{
    int i, n_chan = synth->midi_channels; /* MIDI Channels count */
    int real_val = val; /* real number of channels in the group */

    /* adjusts val range */
    if(mode == FLUID_CHANNEL_MODE_OMNIOFF_POLY)
    {
        real_val = 1; /* mode poly omnioff implies a group of only one channel.*/
    }
    else if(val == 0)
    {
        /* mode poly omnion (0), mono omnion (1), mono omni off (3) */
        /* value 0 means all possible channels from basicchan to MIDI channel count -1.*/
        real_val = n_chan - basicchan;
    }
    /* checks if val range is above MIDI channel count */
    else if(basicchan + val > n_chan)
    {
        return FLUID_FAILED;
    }

    /* checks if this basic channel group overlaps next basic channel group */
    for(i = basicchan + 1; i < basicchan + real_val; i++)
    {
        if(synth->channel[i]->mode &  FLUID_CHANNEL_BASIC)
        {
            /* A value of 0 for val means all possible channels from basicchan to
            to the next basic channel -1 (if any).
            When i reaches the next basic channel group, real_val will be
            limited if it is possible */
            if(val == 0)
            {
                /* limitation of real_val */
                real_val = i - basicchan;
                break;
            }

            /* overlap with the next basic channel group */
            return FLUID_FAILED;
        }
    }

    return real_val;
}

/**
 * Sets a new basic channel group only. The function doesn't allow to change an
 * existing basic channel.
 *
 * The function fails if any channel overlaps any existing basic channel group.
 * To make room if necessary, basic channel groups can be cleared using
 * fluid_synth_reset_basic_channel().
 *
 * @param synth the synth instance.
 * @param chan the basic Channel number (0 to MIDI channel count-1).
 * @param mode the MIDI mode to use for chan (see #fluid_basic_channel_modes).
 * @param val number of channels in the group.
 * @note \a val is only relevant for mode #FLUID_CHANNEL_MODE_OMNION_POLY,
 * #FLUID_CHANNEL_MODE_OMNION_MONO and #FLUID_CHANNEL_MODE_OMNIOFF_MONO. A value
 * of 0 means all possible channels from \a chan to to next basic channel minus 1 (if any)
 * or to MIDI channel count minus 1. Val is ignored for #FLUID_CHANNEL_MODE_OMNIOFF_POLY
 * as this mode implies a group of only one channel.
 * @return
 * - #FLUID_OK on success.
 * - #FLUID_FAILED
 *   - \a synth is NULL.
 *   - \a chan is outside MIDI channel count.
 *   - \a mode is invalid.
 *   - \a val has a number of channels overlapping another basic channel group or been
 *     above MIDI channel count.
 *   - When the function fails, any existing basic channels aren't modified.
 */
int fluid_synth_set_basic_channel(fluid_synth_t *synth, int chan, int mode, int val)
{
    /* check parameters */
    fluid_return_val_if_fail(mode >= 0, FLUID_FAILED);
    fluid_return_val_if_fail(mode < FLUID_CHANNEL_MODE_LAST, FLUID_FAILED);
    fluid_return_val_if_fail(val >= 0, FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    /**/
    if(val > 0 && chan + val > synth->midi_channels)
    {
        FLUID_API_RETURN(FLUID_FAILED);
    }

    /* Checks if there is an overlap with the next basic channel */
    val = fluid_synth_check_next_basic_channel(synth, chan, mode, val);

    if(val == FLUID_FAILED || synth->channel[chan]->mode &  FLUID_CHANNEL_ENABLED)
    {
        /* overlap with the next or previous channel group */
        FLUID_LOG(FLUID_INFO, "basic channel %d overlaps another group", chan);
        FLUID_API_RETURN(FLUID_FAILED);
    }

    /* sets a new basic channel group */
    fluid_synth_set_basic_channel_LOCAL(synth, chan, mode, val);
    /**/
    FLUID_API_RETURN(FLUID_OK);
}

/*
 * Local version of fluid_synth_set_basic_channel(), called internally:
 * - by fluid_synth_set_basic_channel() to set a new basic channel group.
 * - during creation new_fluid_synth() or on CC reset to set a default basic channel group.
 * - on CC ominoff, CC omnion, CC poly , CC mono to change an existing basic channel group.
 *
 * @param see fluid_synth_set_basic_channel()
*/
static void
fluid_synth_set_basic_channel_LOCAL(fluid_synth_t *synth, int basicchan, int mode, int val)
{
    int i;

    /* sets the basic channel group */
    for(i = basicchan; i < basicchan + val; i++)
    {
        int new_mode = mode; /* OMNI_OFF/ON, MONO/POLY ,others bits are zero */
        int new_val;
        /* MIDI specs: when mode is changed, channel must receive ALL_NOTES_OFF */
        fluid_synth_all_notes_off_LOCAL(synth, i);

        if(i == basicchan)
        {
            new_mode |= FLUID_CHANNEL_BASIC; /* First channel in the group */
            new_val = val;	/* number of channels in the group */
        }
        else
        {
            new_val = 0; /* val is 0 for other channel than basic channel */
        }

        /* Channel is enabled */
        new_mode |= FLUID_CHANNEL_ENABLED;
        /* Now new_mode is OMNI OFF/ON,MONO/POLY, BASIC_CHANNEL or not and enabled */
        fluid_channel_set_basic_channel_info(synth->channel[i], new_mode);
        synth->channel[i]->mode_val = new_val;
    }
}

/**
 * Searches a previous basic channel starting from chan.
 *
 * @param synth the synth instance.
 * @param chan starting index of the search (including chan).
 * @return index of the basic channel if found , FLUID_FAILED otherwise.
 */
static int fluid_synth_get_previous_basic_channel(fluid_synth_t *synth, int chan)
{
    for(; chan >= 0; chan--)
    {
        /* searches previous basic channel */
        if(synth->channel[chan]->mode &  FLUID_CHANNEL_BASIC)
        {
            /* chan is the previous basic channel */
            return chan;
        }
    }

    return FLUID_FAILED;
}

/**
 * Returns poly mono mode information of any MIDI channel.
 *
 * @param synth the synth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param basic_chan_out Buffer to store the basic channel \a chan belongs to or #FLUID_FAILED if \a chan is disabled.
 * @param mode_out Buffer to store the mode of \a chan (see #fluid_basic_channel_modes) or #FLUID_FAILED if \a chan is disabled.
 * @param val_out Buffer to store the total number of channels in this basic channel group or #FLUID_FAILED if \a chan is disabled.
 * @note If any of \a basic_chan_out, \a mode_out, \a val_out pointer is NULL
 *  the corresponding information isn't returned.
 *
 * @return
 * - #FLUID_OK on success.
 * - #FLUID_FAILED
 *   - \a synth is NULL.
 *   - \a chan is outside MIDI channel count.
 */
int fluid_synth_get_basic_channel(fluid_synth_t *synth, int chan,
                                  int *basic_chan_out,
                                  int *mode_out,
                                  int *val_out)
{
    int basic_chan = FLUID_FAILED;
    int mode = FLUID_FAILED;
    int val = FLUID_FAILED;

    /* checks parameters first */
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);

    if((synth->channel[chan]->mode &  FLUID_CHANNEL_ENABLED) &&
            /* chan is enabled , we search the basic channel chan belongs to */
            (basic_chan = fluid_synth_get_previous_basic_channel(synth, chan)) != FLUID_FAILED)
    {
        mode = synth->channel[chan]->mode & FLUID_CHANNEL_MODE_MASK;
        val = synth->channel[basic_chan]->mode_val;
    }

    /* returns the information if they are requested */
    if(basic_chan_out)
    {
        * basic_chan_out = basic_chan;
    }

    if(mode_out)
    {
        * mode_out = mode;
    }

    if(val_out)
    {
        * val_out = val;
    }

    FLUID_API_RETURN(FLUID_OK);
}
