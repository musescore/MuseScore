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

#ifndef _FLUIDSYNTH_SFONT_H
#define _FLUIDSYNTH_SFONT_H

#ifdef __cplusplus
extern "C" {
#endif



  /**
   *
   *   SoundFont plugins
   *
   *    It is possible to add new SoundFont loaders to the
   *    synthesizer. The API uses a couple of "interfaces" (structures
   *    with callback functions): fluid_sfloader_t, fluid_sfont_t, and
   *    fluid_preset_t. 
   *
   *    To add a new SoundFont loader to the synthesizer, call
   *    fluid_synth_add_sfloader() and pass a pointer to an
   *    fluid_sfloader_t structure. The important callback function in
   *    this structure is "load", which should try to load a file and
   *    returns a fluid_sfont_t structure, or NULL if it fails.
   *
   *    The fluid_sfont_t structure contains a callback to obtain the
   *    name of the soundfont. It contains two functions to iterate
   *    though the contained presets, and one function to obtain a
   *    preset corresponding to a bank and preset number. This
   *    function should return an fluid_preset_t structure.
   *
   *    The fluid_preset_t structure contains some functions to obtain
   *    information from the preset (name, bank, number). The most
   *    important callback is the noteon function. The noteon function
   *    should call fluid_synth_alloc_voice() for every sample that has
   *    to be played. fluid_synth_alloc_voice() expects a pointer to a
   *    fluid_sample_t structure and returns a pointer to the opaque
   *    fluid_voice_t structure. To set or increments the values of a
   *    generator, use fluid_voice_gen_{set,incr}. When you are
   *    finished initializing the voice call fluid_voice_start() to
   *    start playing the synthesis voice.
   * */

  enum {
    FLUID_PRESET_SELECTED,
    FLUID_PRESET_UNSELECTED,
    FLUID_SAMPLE_DONE
  };


/*
 * fluid_sfloader_t
 */

struct _fluid_sfloader_t {
  /** Private data */
  void* data;

  /** The free must free the memory allocated for the loader in
   * addition to any private data. It should return 0 if no error
   * occured, non-zero otherwise.*/
  int (*free)(fluid_sfloader_t* loader);

  /** Load a file. Returns NULL if an error occured. */
  fluid_sfont_t* (*load)(fluid_sfloader_t* loader, const char* filename);
};


/*
 * fluid_sfont_t
 */

struct _fluid_sfont_t {
  void* data;
  unsigned int id;

  /** The 'free' callback function should return 0 when it was able to
      free all resources. It should return a non-zero value if some of
      the samples could not be freed because they are still in use. */
  int (*free)(fluid_sfont_t* sfont);

  /** Return the name of the sfont */
  char* (*get_name)(fluid_sfont_t* sfont);

  /** Return the preset with the specified bank and preset number. All
   *  the fields, including the 'sfont' field, should * be filled
   *  in. If the preset cannot be found, the function returns NULL. */
  fluid_preset_t* (*get_preset)(fluid_sfont_t* sfont, unsigned int bank, unsigned int prenum);

  void (*iteration_start)(fluid_sfont_t* sfont);

  /* return 0 when no more presets are available, 1 otherwise */
  int (*iteration_next)(fluid_sfont_t* sfont, fluid_preset_t* preset);
};

#define fluid_sfont_get_id(_sf) ((_sf)->id)


/*
 * fluid_preset_t 
 */

struct _fluid_preset_t {
  void* data;
  fluid_sfont_t* sfont;
  int (*free)(fluid_preset_t* preset);
  char* (*get_name)(fluid_preset_t* preset);
  int (*get_banknum)(fluid_preset_t* preset);
  int (*get_num)(fluid_preset_t* preset);

  /** handle a noteon event. Returns 0 if no error occured. */
  int (*noteon)(fluid_preset_t* preset, fluid_synth_t* synth, int chan, int key, int vel);

  /** Implement this function if the preset needs to be notified about
      preset select and unselect events. */
  int (*notify)(fluid_preset_t* preset, int reason, int chan);
};


/*
 * fluid_sample_t
 */

struct _fluid_sample_t
{
  char name[21];
  unsigned int start;
  unsigned int end;	/* Note: Index of last valid sample point (contrary to SF spec) */
  unsigned int loopstart;
  unsigned int loopend;	/* Note: first point following the loop (superimposed on loopstart) */
  unsigned int samplerate;
  int origpitch;
  int pitchadj;
  int sampletype;
  int valid;
  short* data;

  /** The amplitude, that will lower the level of the sample's loop to
      the noise floor. Needed for note turnoff optimization, will be
      filled out automatically */
  /* Set this to zero, when submitting a new sample. */
  int amplitude_that_reaches_noise_floor_is_valid; 
  double amplitude_that_reaches_noise_floor;

  /** Count the number of playing voices that use this sample. */
  unsigned int refcount;

  /** Implement this function if the sample or SoundFont needs to be
      notified when the sample is no longer used. */
  int (*notify)(fluid_sample_t* sample, int reason);

  /** Pointer to SoundFont specific data */
  void* userdata;
};


#define fluid_sample_refcount(_sample) ((_sample)->refcount)


/** Sample types */

#define FLUID_SAMPLETYPE_MONO	1
#define FLUID_SAMPLETYPE_RIGHT	2
#define FLUID_SAMPLETYPE_LEFT	4
#define FLUID_SAMPLETYPE_LINKED	8
#define FLUID_SAMPLETYPE_OGG_VORBIS	0x10 /**< Flag for #fluid_sample_t \a sampletype field for Ogg Vorbis compressed samples */
#define FLUID_SAMPLETYPE_OGG_VORBIS_UNPACKED    0x20
#define FLUID_SAMPLETYPE_ROM	0x8000



#ifdef __cplusplus
}
#endif

#endif /* _FLUIDSYNTH_SFONT_H */
