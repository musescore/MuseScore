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

#ifndef _FLUIDSYNTH_AUDIO_H
#define _FLUIDSYNTH_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file audio.h
 * @brief Functions for audio driver output.
 * @defgroup AudioFunctions Functions for audio output
 *
 * Defines functions for creating audio driver output.  Use
 * new_fluid_audio_driver() to create a new audio driver for a given synth
 * and configuration settings.  The function new_fluid_audio_driver2() can be
 * used if custom audio processing is desired before the audio is sent to the
 * audio driver (although it is not as efficient).
 *
 * @sa @ref CreatingAudioDriver
 */

/**
 * Callback function type used with new_fluid_audio_driver2() to allow for
 * custom user audio processing before the audio is sent to the driver. This
 * function is responsible for rendering audio to the buffers.
 * The buffers passed to this function are allocated and owned by the respective
 * audio driver and are only valid during that specific call (do not cache them).
 * For further details please refer to fluid_synth_process().
 * @note Whereas fluid_synth_process() allows aliasing buffers, there is the guarentee that @p out
 * and @p fx buffers provided by fluidsynth's audio drivers never alias. This prevents downstream
 * applications from e.g. applying a custom effect accidentially to the same buffer multiple times.
 * @param data The user data parameter as passed to new_fluid_audio_driver2().
 * @param len Count of audio frames to synthesize.
 * @param nfx Count of arrays in \c fx.
 * @param fx Array of buffers to store effects audio to. Buffers may alias with buffers of \c out.
 * @param nout Count of arrays in \c out.
 * @param out Array of buffers to store (dry) audio to. Buffers may alias with buffers of \c fx.
 * @return Should return #FLUID_OK on success, #FLUID_FAILED if an error occurred.
 */
typedef int (*fluid_audio_func_t)(void *data, int len,
                                  int nfx, float *fx[],
                                  int nout, float *out[]);

FLUIDSYNTH_API fluid_audio_driver_t *new_fluid_audio_driver(fluid_settings_t *settings,
        fluid_synth_t *synth);

FLUIDSYNTH_API fluid_audio_driver_t *new_fluid_audio_driver2(fluid_settings_t *settings,
        fluid_audio_func_t func,
        void *data);

FLUIDSYNTH_API void delete_fluid_audio_driver(fluid_audio_driver_t *driver);

FLUIDSYNTH_API fluid_file_renderer_t *new_fluid_file_renderer(fluid_synth_t *synth);
FLUIDSYNTH_API int fluid_file_renderer_process_block(fluid_file_renderer_t *dev);
FLUIDSYNTH_API void delete_fluid_file_renderer(fluid_file_renderer_t *dev);
FLUIDSYNTH_API int fluid_file_set_encoding_quality(fluid_file_renderer_t *dev, double q);

FLUIDSYNTH_API int fluid_audio_driver_register(const char **adrivers);

#ifdef __cplusplus
}
#endif

#endif /* _FLUIDSYNTH_AUDIO_H */
