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

#ifndef _FLUIDSYNTH_GEN_H
#define _FLUIDSYNTH_GEN_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file gen.h
 * @brief Functions and defines for SoundFont generator effects.
 */

/**
 * Generator (effect) numbers (Soundfont 2.01 specifications section 8.1.3)
 */
enum fluid_gen_type {
  GEN_STARTADDROFS,		/**< Sample start address offset (0-32767) */
  GEN_ENDADDROFS,		/**< Sample end address offset (-32767-0) */
  GEN_STARTLOOPADDROFS,		/**< Sample loop start address offset (-32767-32767) */
  GEN_ENDLOOPADDROFS,		/**< Sample loop end address offset (-32767-32767) */
  GEN_STARTADDRCOARSEOFS,	/**< Sample start address coarse offset (X 32768) */
  GEN_MODLFOTOPITCH,		/**< Modulation LFO to pitch */
  GEN_VIBLFOTOPITCH,		/**< Vibrato LFO to pitch */
  GEN_MODENVTOPITCH,		/**< Modulation envelope to pitch */
  GEN_FILTERFC,			/**< Filter cutoff */
  GEN_FILTERQ,			/**< Filter Q */
  GEN_MODLFOTOFILTERFC,		/**< Modulation LFO to filter cutoff */
  GEN_MODENVTOFILTERFC,		/**< Modulation envelope to filter cutoff */
  GEN_ENDADDRCOARSEOFS,		/**< Sample end address coarse offset (X 32768) */
  GEN_MODLFOTOVOL,		/**< Modulation LFO to volume */
  GEN_UNUSED1,			/**< Unused */
  GEN_CHORUSSEND,		/**< Chorus send amount */
  GEN_REVERBSEND,		/**< Reverb send amount */
  GEN_PAN,			/**< Stereo panning */
  GEN_UNUSED2,			/**< Unused */
  GEN_UNUSED3,			/**< Unused */
  GEN_UNUSED4,			/**< Unused */
  GEN_MODLFODELAY,		/**< Modulation LFO delay */
  GEN_MODLFOFREQ,		/**< Modulation LFO frequency */
  GEN_VIBLFODELAY,		/**< Vibrato LFO delay */
  GEN_VIBLFOFREQ,		/**< Vibrato LFO frequency */
  GEN_MODENVDELAY,		/**< Modulation envelope delay */
  GEN_MODENVATTACK,		/**< Modulation envelope attack */
  GEN_MODENVHOLD,		/**< Modulation envelope hold */
  GEN_MODENVDECAY,		/**< Modulation envelope decay */
  GEN_MODENVSUSTAIN,		/**< Modulation envelope sustain */
  GEN_MODENVRELEASE,		/**< Modulation envelope release */
  GEN_KEYTOMODENVHOLD,		/**< Key to modulation envelope hold */
  GEN_KEYTOMODENVDECAY,		/**< Key to modulation envelope decay */
  GEN_VOLENVDELAY,		/**< Volume envelope delay */
  GEN_VOLENVATTACK,		/**< Volume envelope attack */
  GEN_VOLENVHOLD,		/**< Volume envelope hold */
  GEN_VOLENVDECAY,		/**< Volume envelope decay */
  GEN_VOLENVSUSTAIN,		/**< Volume envelope sustain */
  GEN_VOLENVRELEASE,		/**< Volume envelope release */
  GEN_KEYTOVOLENVHOLD,		/**< Key to volume envelope hold */
  GEN_KEYTOVOLENVDECAY,		/**< Key to volume envelope decay */
  GEN_INSTRUMENT,		/**< Instrument ID (shouldn't be set by user) */
  GEN_RESERVED1,		/**< Reserved */
  GEN_KEYRANGE,			/**< MIDI note range */
  GEN_VELRANGE,			/**< MIDI velocity range */
  GEN_STARTLOOPADDRCOARSEOFS,	/**< Sample start loop address coarse offset (X 32768) */
  GEN_KEYNUM,			/**< Fixed MIDI note number */
  GEN_VELOCITY,			/**< Fixed MIDI velocity value */
  GEN_ATTENUATION,		/**< Initial volume attenuation */
  GEN_RESERVED2,		/**< Reserved */
  GEN_ENDLOOPADDRCOARSEOFS,	/**< Sample end loop address coarse offset (X 32768) */
  GEN_COARSETUNE,		/**< Coarse tuning */
  GEN_FINETUNE,			/**< Fine tuning */
  GEN_SAMPLEID,			/**< Sample ID (shouldn't be set by user) */
  GEN_SAMPLEMODE,		/**< Sample mode flags */
  GEN_RESERVED3,		/**< Reserved */
  GEN_SCALETUNE,		/**< Scale tuning */
  GEN_EXCLUSIVECLASS,		/**< Exclusive class number */
  GEN_OVERRIDEROOTKEY,		/**< Sample root note override */

  /* the initial pitch is not a "standard" generator. It is not
   * mentioned in the list of generator in the SF2 specifications. It
   * is used, however, as the destination for the default pitch wheel
   * modulator. */
  GEN_PITCH,			/**< Pitch (NOTE: Not a real SoundFont generator) */
  GEN_LAST			/**< Value defines the count of generators (#fluid_gen_type) */
};


/**
 * SoundFont generator structure.
 */
typedef struct _fluid_gen_t
{
  unsigned char flags; /**< Is the generator set or not (#fluid_gen_flags) */
  double val;          /**< The nominal value */
  double mod;          /**< Change by modulators */
  double nrpn;         /**< Change by NRPN messages */
} fluid_gen_t;

/**
 * Enum value for 'flags' field of #_fluid_gen_t (not really flags).
 */
enum fluid_gen_flags
{
  GEN_UNUSED,		/**< Generator value is not set */
  GEN_SET,		/**< Generator value is set */
  GEN_ABS_NRPN		/**< DOCME */
};

FLUIDSYNTH_API int fluid_gen_set_default_values(fluid_gen_t* gen);



#ifdef __cplusplus
}
#endif
#endif /* _FLUIDSYNTH_GEN_H */

