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

#ifndef _FLUIDSYNTH_SYNTH_H
#define _FLUIDSYNTH_SYNTH_H


#ifdef __cplusplus
extern "C" {
#endif


  /**   Embedded synthesizer
   *  
   *    You create a new synthesizer with new_fluid_synth() and you destroy
   *    if with delete_fluid_synth(). Use the settings structure to specify
   *    the synthesizer characteristics. 
   *
   *    You have to load a SoundFont in order to hear any sound. For that
   *    you use the fluid_synth_sfload() function.
   *
   *    You can use the audio driver functions described below to open
   *    the audio device and create a background audio thread.
   *  
   *    The API for sending MIDI events is probably what you expect:
   *    fluid_synth_noteon(), fluid_synth_noteoff(), ...
   * 
   */


  /** Creates a new synthesizer object. 
   *
   *  Creates a new synthesizer object. As soon as the synthesizer is
   *  created, it will start playing.  
   *
   * \param settings a pointer to a settings structure
   * \return a newly allocated synthesizer or NULL in case of error
   */
FLUIDSYNTH_API fluid_synth_t* new_fluid_synth(fluid_settings_t* settings);

FLUIDSYNTH_API void fluid_synth_set_sample_rate(fluid_synth_t* synth, float sample_rate);


  /** 
   * Deletes the synthesizer previously created with new_fluid_synth.
   *
   * \param synth the synthesizer object
   * \return 0 if no error occured, -1 otherwise 
   */
FLUIDSYNTH_API int delete_fluid_synth(fluid_synth_t* synth);


  /** Get a reference to the settings of the synthesizer.
   *
   * \param synth the synthesizer object
   * \return pointer to the settings
   */
FLUIDSYNTH_API fluid_settings_t* fluid_synth_get_settings(fluid_synth_t* synth);


  /*
   * 
   * MIDI channel messages 
   *
   */

  /** Send a noteon message. Returns 0 if no error occurred, -1 otherwise. */
FLUIDSYNTH_API int fluid_synth_noteon(fluid_synth_t* synth, int chan, int key, int vel);

  /** Send a noteoff message. Returns 0 if no error occurred, -1 otherwise.  */
FLUIDSYNTH_API int fluid_synth_noteoff(fluid_synth_t* synth, int chan, int key);

  /** Send a control change message. Returns 0 if no error occurred, -1 otherwise.  */
FLUIDSYNTH_API int fluid_synth_cc(fluid_synth_t* synth, int chan, int ctrl, int val);

  /** Get a control value. Returns 0 if no error occurred, -1 otherwise.  */
FLUIDSYNTH_API int fluid_synth_get_cc(fluid_synth_t* synth, int chan, int ctrl, int* pval);

  /** Send a pitch bend message. Returns 0 if no error occurred, -1 otherwise.  */
FLUIDSYNTH_API int fluid_synth_pitch_bend(fluid_synth_t* synth, int chan, int val);

  /** Get the pitch bend value. Returns 0 if no error occurred, -1 otherwise. */
FLUIDSYNTH_API 
int fluid_synth_get_pitch_bend(fluid_synth_t* synth, int chan, int* ppitch_bend);

  /** Set the pitch wheel sensitivity. Returns 0 if no error occurred, -1 otherwise. */
FLUIDSYNTH_API int fluid_synth_pitch_wheel_sens(fluid_synth_t* synth, int chan, int val);

  /** Get the pitch wheel sensitivity. Returns 0 if no error occurred, -1 otherwise. */
FLUIDSYNTH_API int fluid_synth_get_pitch_wheel_sens(fluid_synth_t* synth, int chan, int* pval);

  /** Send a program change message. Returns 0 if no error occurred, -1 otherwise. */
FLUIDSYNTH_API int fluid_synth_program_change(fluid_synth_t* synth, int chan, int program);

FLUIDSYNTH_API int fluid_synth_channel_pressure(fluid_synth_t* synth, int chan, int val);
FLUIDSYNTH_API int fluid_synth_sysex(fluid_synth_t *synth, const char *data, int len,
                                     char *response, int *response_len, int *handled, int dryrun);

  /** Select a bank. Returns 0 if no error occurred, -1 otherwise. */
FLUIDSYNTH_API 
int fluid_synth_bank_select(fluid_synth_t* synth, int chan, unsigned int bank);

  /** Select a sfont. Returns 0 if no error occurred, -1 otherwise. */
FLUIDSYNTH_API 
int fluid_synth_sfont_select(fluid_synth_t* synth, int chan, unsigned int sfont_id);

  /** Select a preset for a channel. The preset is specified by the
      SoundFont ID, the bank number, and the preset number. This
      allows any preset to be selected and circumvents preset masking
      due to previously loaded SoundFonts on the SoundFont stack.

      \param synth The synthesizer
      \param chan The channel on which to set the preset
      \param sfont_id The ID of the SoundFont 
      \param bank_num The bank number
      \param preset_num The preset number
      \return 0 if no errors occured, -1 otherwise
  */
FLUIDSYNTH_API 
int fluid_synth_program_select(fluid_synth_t* synth, int chan, 
			      unsigned int sfont_id, 
			      unsigned int bank_num, 
			      unsigned int preset_num);

  /** Returns the program, bank, and SoundFont number of the preset on
      a given channel. Returns 0 if no error occurred, -1 otherwise. */
FLUIDSYNTH_API 
int fluid_synth_get_program(fluid_synth_t* synth, int chan, 
			   unsigned int* sfont_id, 
			   unsigned int* bank_num, 
			   unsigned int* preset_num);

  /** Send a bank select and a program change to every channel to
   *  reinitialize the preset of the channel. This function is useful
   *  mainly after a SoundFont has been loaded, unloaded or
   *  reloaded. . Returns 0 if no error occurred, -1 otherwise. */
FLUIDSYNTH_API int fluid_synth_program_reset(fluid_synth_t* synth);

  /** Send a reset. A reset turns all the notes off and resets the
      controller values. */
FLUIDSYNTH_API int fluid_synth_system_reset(fluid_synth_t* synth);

FLUIDSYNTH_API int fluid_synth_all_notes_off(fluid_synth_t *synth, int chan);
FLUIDSYNTH_API int fluid_synth_all_sounds_off(fluid_synth_t *synth, int chan);


  /*
   * 
   * Low level access 
   *
   */

  /** Create and start voices using a preset. The id passed as
   * argument will be used as the voice group id.  */
FLUIDSYNTH_API int fluid_synth_start(fluid_synth_t* synth, unsigned int id, 
				     fluid_preset_t* preset, int audio_chan, 
				     int midi_chan, int key, int vel);

  /** Stop the voices in the voice group defined by id. */
FLUIDSYNTH_API int fluid_synth_stop(fluid_synth_t* synth, unsigned int id);

  /** Change the value of a generator of the voices in the voice group
   * defined by id. */
/* FLUIDSYNTH_API int fluid_synth_ctrl(fluid_synth_t* synth, int id,  */
/* 				    int gen, float value,  */
/* 				    int absolute, int normalized); */


  /*
   * 
   * SoundFont management 
   *
   */

  /** Set an optional function callback each time a preset has finished loading.
      This can be useful when calling fluid_synth_sfload asynchronously.
      The function must be formatted like this:
      void my_callback_function(int bank, int num, char* name)

      \param callback Pointer to the function
  */
FLUIDSYNTH_API
void fluid_synth_set_preset_callback(void* callback);

  /** Loads a SoundFont file and creates a new SoundFont. The newly
      loaded SoundFont will be put on top of the SoundFont
      stack. Presets are searched starting from the SoundFont on the
      top of the stack, working the way down the stack until a preset
      is found.

      \param synth The synthesizer object
      \param filename The file name
      \param reset_presets If non-zero, the presets on the channels will be reset
      \returns The ID of the loaded SoundFont, or -1 in case of error
  */
FLUIDSYNTH_API 
int fluid_synth_sfload(fluid_synth_t* synth, const char* filename, int reset_presets);

  /** Reload a SoundFont. The reloaded SoundFont retains its ID and
      index on the stack.

      \param synth The synthesizer object
      \param id The id of the SoundFont
      \returns The ID of the loaded SoundFont, or -1 in case of error
  */
FLUIDSYNTH_API int fluid_synth_sfreload(fluid_synth_t* synth, unsigned int id);

  /** Removes a SoundFont from the stack and deallocates it.

      \param synth The synthesizer object
      \param id The id of the SoundFont
      \param reset_presets If TRUE then presets will be reset for all channels
      \returns 0 if no error, -1 otherwise
  */
FLUIDSYNTH_API int fluid_synth_sfunload(fluid_synth_t* synth, unsigned int id, int reset_presets);

  /** Add a SoundFont. The SoundFont will be put on top of
      the SoundFont stack.

      \param synth The synthesizer object
      \param sfont The SoundFont
      \returns The ID of the loaded SoundFont, or -1 in case of error
  */
FLUIDSYNTH_API int fluid_synth_add_sfont(fluid_synth_t* synth, fluid_sfont_t* sfont);

  /** Remove a SoundFont that was previously added using
   *  fluid_synth_add_sfont(). The synthesizer does not delete the
   *  SoundFont; this is responsability of the caller.

      \param synth The synthesizer object
      \param sfont The SoundFont
  */
FLUIDSYNTH_API void fluid_synth_remove_sfont(fluid_synth_t* synth, fluid_sfont_t* sfont);

  /** Count the number of loaded SoundFonts.

      \param synth The synthesizer object
      \returns The number of loaded SoundFonts 
  */
FLUIDSYNTH_API int fluid_synth_sfcount(fluid_synth_t* synth);

  /** Get a SoundFont. The SoundFont is specified by its index on the
      stack. The top of the stack has index zero. 
    
      \param synth The synthesizer object
      \param num The number of the SoundFont (0 <= num < sfcount)
      \returns A pointer to the SoundFont
  */
FLUIDSYNTH_API fluid_sfont_t* fluid_synth_get_sfont(fluid_synth_t* synth, unsigned int num);

  /** Get a SoundFont. The SoundFont is specified by its ID.
    
      \param synth The synthesizer object
      \param id The id of the sfont
      \returns A pointer to the SoundFont
  */
FLUIDSYNTH_API fluid_sfont_t* fluid_synth_get_sfont_by_id(fluid_synth_t* synth, unsigned int id);


  /** Get the preset of a channel */
FLUIDSYNTH_API fluid_preset_t* fluid_synth_get_channel_preset(fluid_synth_t* synth, int chan);

  /** Offset the bank numbers in a SoundFont. Returns -1 if an error
   * occured (out of memory or negative offset) */ 
FLUIDSYNTH_API int fluid_synth_set_bank_offset(fluid_synth_t* synth, int sfont_id, int offset);

  /** Get the offset of the bank numbers in a SoundFont. */ 
FLUIDSYNTH_API int fluid_synth_get_bank_offset(fluid_synth_t* synth, int sfont_id);



  /*
   * 
   * Reverb 
   *
   */

  /** Set the parameters for the built-in reverb unit */
FLUIDSYNTH_API void fluid_synth_set_reverb(fluid_synth_t* synth, double roomsize, 
					 double damping, double width, double level);

  /** Turn on (1) / off (0) the built-in reverb unit */
FLUIDSYNTH_API void fluid_synth_set_reverb_on(fluid_synth_t* synth, int on);


  /** Query the current state of the reverb. */
FLUIDSYNTH_API double fluid_synth_get_reverb_roomsize(fluid_synth_t* synth);
FLUIDSYNTH_API double fluid_synth_get_reverb_damp(fluid_synth_t* synth);
FLUIDSYNTH_API double fluid_synth_get_reverb_level(fluid_synth_t* synth);
FLUIDSYNTH_API double fluid_synth_get_reverb_width(fluid_synth_t* synth);

  /* Those are the default settings for the reverb */
#define FLUID_REVERB_DEFAULT_ROOMSIZE 0.2f
#define FLUID_REVERB_DEFAULT_DAMP 0.0f
#define FLUID_REVERB_DEFAULT_WIDTH 0.5f
#define FLUID_REVERB_DEFAULT_LEVEL 0.9f



  /*
   * 
   * Chorus 
   *
   */

enum fluid_chorus_mod {
  FLUID_CHORUS_MOD_SINE = 0,
  FLUID_CHORUS_MOD_TRIANGLE = 1
};

  /** Set up the chorus. It should be turned on with fluid_synth_set_chorus_on.
   * If faulty parameters are given, all new settings are discarded.
   * Keep in mind, that the needed CPU time is proportional to 'nr'.
   */
FLUIDSYNTH_API void fluid_synth_set_chorus(fluid_synth_t* synth, int nr, double level, 
					 double speed, double depth_ms, int type);

  /** Turn on (1) / off (0) the built-in chorus unit */
FLUIDSYNTH_API void fluid_synth_set_chorus_on(fluid_synth_t* synth, int on);

  /** Query the current state of the chorus. */
FLUIDSYNTH_API int fluid_synth_get_chorus_nr(fluid_synth_t* synth);
FLUIDSYNTH_API double fluid_synth_get_chorus_level(fluid_synth_t* synth);
FLUIDSYNTH_API double fluid_synth_get_chorus_speed_Hz(fluid_synth_t* synth);
FLUIDSYNTH_API double fluid_synth_get_chorus_depth_ms(fluid_synth_t* synth);
FLUIDSYNTH_API int fluid_synth_get_chorus_type(fluid_synth_t* synth); /* see fluid_chorus_mod */

  /* Those are the default settings for the chorus. */
#define FLUID_CHORUS_DEFAULT_N 3
#define FLUID_CHORUS_DEFAULT_LEVEL 2.0f
#define FLUID_CHORUS_DEFAULT_SPEED 0.3f
#define FLUID_CHORUS_DEFAULT_DEPTH 8.0f
#define FLUID_CHORUS_DEFAULT_TYPE FLUID_CHORUS_MOD_SINE 



  /*
   * 
   * Audio and MIDI channels 
   *
   */

  /** Returns the number of MIDI channels that the synthesizer uses
      internally */
FLUIDSYNTH_API int fluid_synth_count_midi_channels(fluid_synth_t* synth);

  /** Returns the number of audio channels that the synthesizer uses
      internally */
FLUIDSYNTH_API int fluid_synth_count_audio_channels(fluid_synth_t* synth);

  /** Returns the number of audio groups that the synthesizer uses
      internally. This is usually identical to audio_channels. */
FLUIDSYNTH_API int fluid_synth_count_audio_groups(fluid_synth_t* synth);

  /** Returns the number of effects channels that the synthesizer uses
      internally */
FLUIDSYNTH_API int fluid_synth_count_effects_channels(fluid_synth_t* synth);



  /*
   * 
   * Synthesis parameters 
   *
   */

  /** Set the master gain */
FLUIDSYNTH_API void fluid_synth_set_gain(fluid_synth_t* synth, float gain);

  /** Get the master gain */
FLUIDSYNTH_API float fluid_synth_get_gain(fluid_synth_t* synth);

  /** Set the polyphony limit (FluidSynth >= 1.0.6) */
FLUIDSYNTH_API int fluid_synth_set_polyphony(fluid_synth_t* synth, int polyphony);

  /** Get the polyphony limit (FluidSynth >= 1.0.6) */
FLUIDSYNTH_API int fluid_synth_get_polyphony(fluid_synth_t* synth);

  /** Get the internal buffer size. The internal buffer size if not the
      same thing as the buffer size specified in the
      settings. Internally, the synth *always* uses a specific buffer
      size independent of the buffer size used by the audio driver. The
      internal buffer size is normally 64 samples. The reason why it
      uses an internal buffer size is to allow audio drivers to call the
      synthesizer with a variable buffer length. The internal buffer
      size is useful for client who want to optimize their buffer sizes.
  */
FLUIDSYNTH_API int fluid_synth_get_internal_bufsize(fluid_synth_t* synth);

  /** Set the interpolation method for one channel or all channels (chan = -1) */
FLUIDSYNTH_API 
int fluid_synth_set_interp_method(fluid_synth_t* synth, int chan, int interp_method);

  /* Flags to choose the interpolation method */
enum fluid_interp {
  /* no interpolation: Fastest, but questionable audio quality */
  FLUID_INTERP_NONE = 0,
  /* Straight-line interpolation: A bit slower, reasonable audio quality */
  FLUID_INTERP_LINEAR = 1,
  /* Fourth-order interpolation: Requires 50 % of the whole DSP processing time, good quality 
   * Default. */
  FLUID_INTERP_DEFAULT = 4,
  FLUID_INTERP_4THORDER = 4,
  FLUID_INTERP_7THORDER = 7,
  FLUID_INTERP_HIGHEST=7
};




  /*
   * 
   * Generator interface 
   *
   */

  /** Change the value of a generator. This function allows to control
      all synthesis parameters in real-time. The changes are additive,
      i.e. they add up to the existing parameter value. This function is
      similar to sending an NRPN message to the synthesizer. The
      function accepts a float as the value of the parameter. The
      parameter numbers and ranges are described in the SoundFont 2.01
      specification, paragraph 8.1.3, page 48. See also 'fluid_gen_type'.

      \param synth The synthesizer object.
      \param chan The MIDI channel number.
      \param param The parameter number.
      \param value The parameter value.
      \returns Your favorite dish.
  */
FLUIDSYNTH_API 
int fluid_synth_set_gen(fluid_synth_t* synth, int chan, int param, float value);


  /** Retreive the value of a generator. This function returns the value
      set by a previous call 'fluid_synth_set_gen' or by an NRPN message.

      \param synth The synthesizer object.
      \param chan The MIDI channel number.
      \param param The generator number.
      \returns The value of the generator.  
  */
FLUIDSYNTH_API float fluid_synth_get_gen(fluid_synth_t* synth, int chan, int param);




  /*
   * 
   * Tuning 
   *
   */

  /** Create a new key-based tuning with given name, number, and
      pitches. The array 'pitches' should have length 128 and contains
      the pitch in cents of every key in cents. However, if 'pitches' is
      NULL, a new tuning is created with the well-tempered scale.
    
      \param synth The synthesizer object
      \param tuning_bank The tuning bank number [0-127]
      \param tuning_prog The tuning program number [0-127]
      \param name The name of the tuning
      \param pitch The array of pitch values. The array length has to be 128.
  */
FLUIDSYNTH_API 
int fluid_synth_create_key_tuning(fluid_synth_t* synth, int tuning_bank, int tuning_prog,
                                 const char* name, double* pitch);

  /** Create a new octave-based tuning with given name, number, and
      pitches.  The array 'pitches' should have length 12 and contains
      derivation in cents from the well-tempered scale. For example, if
      pitches[0] equals -33, then the C-keys will be tuned 33 cents
      below the well-tempered C. 

      \param synth The synthesizer object
      \param tuning_bank The tuning bank number [0-127]
      \param tuning_prog The tuning program number [0-127]
      \param name The name of the tuning
      \param pitch The array of pitch derivations. The array length has to be 12.
  */
FLUIDSYNTH_API 
int fluid_synth_create_octave_tuning(fluid_synth_t* synth, int tuning_bank, int tuning_prog,
                                    const char* name, const double* pitch);

FLUIDSYNTH_API
int fluid_synth_activate_octave_tuning(fluid_synth_t* synth, int bank, int prog,
                                       const char* name, const double* pitch, int apply);

  /** Request a note tuning changes. Both they 'keys' and 'pitches'
      arrays should be of length 'num_pitches'. If 'apply' is non-zero,
      the changes should be applied in real-time, i.e. sounding notes
      will have their pitch updated. 'APPLY' IS CURRENTLY IGNORED. The
      changes will be available for newly triggered notes only. 

      \param synth The synthesizer object
      \param tuning_bank The tuning bank number [0-127]
      \param tuning_prog The tuning program number [0-127]
      \param len The length of the keys and pitch arrays
      \param keys The array of keys values.
      \param pitch The array of pitch values.
      \param apply Flag to indicate whether to changes should be applied in real-time.    
  */
FLUIDSYNTH_API 
int fluid_synth_tune_notes(fluid_synth_t* synth, int tuning_bank, int tuning_prog,
			  int len, int *keys, double* pitch, int apply);

  /** Select a tuning for a channel. 

  \param synth The synthesizer object
  \param chan The channel number [0-max channels]
  \param tuning_bank The tuning bank number [0-127]
  \param tuning_prog The tuning program number [0-127]
  */
FLUIDSYNTH_API 
int fluid_synth_select_tuning(fluid_synth_t* synth, int chan, int tuning_bank, int tuning_prog);

int fluid_synth_activate_tuning(fluid_synth_t* synth, int chan, int bank, int prog, int apply);

  /** Set the tuning to the default well-tempered tuning on a channel.

  \param synth The synthesizer object
  \param chan The channel number [0-max channels]
  */
FLUIDSYNTH_API int fluid_synth_reset_tuning(fluid_synth_t* synth, int chan);

  /** Start the iteration throught the list of available tunings.

  \param synth The synthesizer object
  */
FLUIDSYNTH_API void fluid_synth_tuning_iteration_start(fluid_synth_t* synth);


  /** Get the next tuning in the iteration. This functions stores the
      bank and program number of the next tuning in the pointers given as
      arguments.

      \param synth The synthesizer object
      \param bank Pointer to an int to store the bank number
      \param prog Pointer to an int to store the program number
      \returns 1 if there is a next tuning, 0 otherwise
  */
FLUIDSYNTH_API 
int fluid_synth_tuning_iteration_next(fluid_synth_t* synth, int* bank, int* prog);


  /** Dump the data of a tuning. This functions stores the name and
      pitch values of a tuning in the pointers given as arguments. Both
      name and pitch can be NULL is the data is not needed.

      \param synth The synthesizer object
      \param bank The tuning bank number [0-127]
      \param prog The tuning program number [0-127]
      \param name Pointer to a buffer to store the name
      \param len The length of the name buffer
      \param pitch Pointer to buffer to store the pitch values
  */
FLUIDSYNTH_API int fluid_synth_tuning_dump(fluid_synth_t* synth, int bank, int prog, 
					 char* name, int len, double* pitch);




  /*
   * 
   * Misc 
   *
   */

  /** Get a textual representation of the last error */
FLUIDSYNTH_API char* fluid_synth_error(fluid_synth_t* synth);


  /*
   *  
   *    Synthesizer plugin
   *  
   *    
   *    To create a synthesizer plugin, create the synthesizer as
   *    explained above. Once the synthesizer is created you can call
   *    any of the functions below to get the audio. 
   * 
   */

  /** Generate a number of samples. This function expects two signed
   *  16bits buffers (left and right channel) that will be filled with
   *  samples.
   *
   *  \param synth The synthesizer
   *  \param len The number of samples to generate
   *  \param lout The sample buffer for the left channel
   *  \param loff The offset, in samples, in the left buffer where the writing pointer starts
   *  \param lincr The increment, in samples, of the writing pointer in the left buffer 
   *  \param rout The sample buffer for the right channel
   *  \param roff The offset, in samples, in the right buffer where the writing pointer starts
   *  \param rincr The increment, in samples, of the writing pointer in the right buffer 
   *  \returns 0 if no error occured, non-zero otherwise
   */

FLUIDSYNTH_API int fluid_synth_write_s16(fluid_synth_t* synth, int len, 
				       void* lout, int loff, int lincr, 
				       void* rout, int roff, int rincr);


  /** Generate a number of samples. This function expects two floating
   *  point buffers (left and right channel) that will be filled with
   *  samples.
   *
   *  \param synth The synthesizer
   *  \param len The number of samples to generate
   *  \param lout The sample buffer for the left channel
   *  \param loff The offset, in samples, in the left buffer where the writing pointer starts
   *  \param lincr The increment, in samples, of the writing pointer in the left buffer 
   *  \param rout The sample buffer for the right channel
   *  \param roff The offset, in samples, in the right buffer where the writing pointer starts
   *  \param rincr The increment, in samples, of the writing pointer in the right buffer 
   *  \returns 0 if no error occured, non-zero otherwise
   */

FLUIDSYNTH_API int fluid_synth_write_float(fluid_synth_t* synth, int len, 
					 void* lout, int loff, int lincr, 
					 void* rout, int roff, int rincr);

FLUIDSYNTH_API int fluid_synth_nwrite_float(fluid_synth_t* synth, int len, 
					  float** left, float** right, 
					  float** fx_left, float** fx_right);


/** Generate a number of samples. This function expects planar  floating
 *  point buffers (first half of the buffer is filled by the left channel, second half is filled by the right channel) that will be filled with
 *  samples.
 *
 *  \param synth The synthesizer
 *  \param len The number of samples to generate
 *  \param out The sample buffer for the left channel
  *  \returns 0 if no error occured, non-zero otherwise
 */

FLUIDSYNTH_API int fluid_synth_pnwrite_float(fluid_synth_t* synth, int len, void* lout, void* rout);

  /** Generate a number of samples. This function implements the
   *  default interface defined in fluidsynth/audio.h. This function
   *  ignores the input buffers and expects at least two output
   *  buffer.
   *
   *  \param synth The synthesizer
   *  \param len The number of samples to generate
   *  \param nin The number of input buffers
   *  \param in The array of input buffers
   *  \param nout The number of output buffers
   *  \param out The array of output buffers
   *  \returns 0 if no error occured, non-zero otherwise
   */

FLUIDSYNTH_API int fluid_synth_process(fluid_synth_t* synth, int len,
				     int nin, float** in, 
				     int nout, float** out);



  /* Type definition of the synthesizer's audio callback function. */
typedef int (*fluid_audio_callback_t)(fluid_synth_t* synth, int len, 
				     void* out1, int loff, int lincr, 
				     void* out2, int roff, int rincr);





  /*
   *  Synthesizer's interface to handle SoundFont loaders 
   */


  /** Add a SoundFont loader to the synthesizer. Note that SoundFont
      loader don't necessarily load SoundFonts. They can load any type
      of wavetable data but export a SoundFont interface. */
FLUIDSYNTH_API void fluid_synth_add_sfloader(fluid_synth_t* synth, fluid_sfloader_t* loader);

  /** Allocate a synthesis voice. This function is called by a
      soundfont's preset in response to a noteon event.
      The returned voice comes with default modulators installed (velocity-to-attenuation,
      velocity to filter, ...)
      Note: A single noteon event may create any number of voices, when the preset is layered. 
      Typically 1 (mono) or 2 (stereo).*/
FLUIDSYNTH_API fluid_voice_t* fluid_synth_alloc_voice(fluid_synth_t* synth, fluid_sample_t* sample, 
						   int channum, int key, int vel);

  /** Start a synthesis voice. This function is called by a
      soundfont's preset in response to a noteon event after the voice
      has been allocated with fluid_synth_alloc_voice() and
      initialized. 
      Exclusive classes are processed here.*/
FLUIDSYNTH_API void fluid_synth_start_voice(fluid_synth_t* synth, fluid_voice_t* voice);


  /** Write a list of all voices matching ID into buf, but not more than bufsize voices.
   * If ID <0, return all voices. */
FLUIDSYNTH_API void fluid_synth_get_voicelist(fluid_synth_t* synth, 
					    fluid_voice_t* buf[], int bufsize, int ID);


//midi router disabled
//  /** This is a hack to get command handlers working */
//FLUIDSYNTH_API void fluid_synth_set_midi_router(fluid_synth_t* synth,
//					      fluid_midi_router_t* router);

#ifdef __cplusplus
}
#endif

#endif /* _FLUIDSYNTH_SYNTH_H */
