/*
SoLoud audio engine
Copyright (c) 2013-2014 Jari Komppa

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
*/
#include <stdlib.h>

#include "soloud.h"

#if !defined(WITH_SDL1)

namespace SoLoud
{
	result sdl1_init(SoLoud::Soloud * /*aSoloud*/, unsigned int /*aFlags*/, unsigned int /*aSamplerate*/, unsigned int /*aBuffer*/)
	{
		return NOT_IMPLEMENTED;
	}
}

#else

#include "SDL.h"
#include <math.h>


extern "C"
{
	int dll_SDL1_found();
	int dll_SDL1_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained);
	void dll_SDL1_CloseAudio();
	void dll_SDL1_PauseAudio(int pause_on);
};


namespace SoLoud
{
	static SDL_AudioSpec gActiveAudioSpec;

	void soloud_sdl1_audiomixer(void *userdata, Uint8 *stream, int len)
	{
		short *buf = (short*)stream;
		SoLoud::Soloud *soloud = (SoLoud::Soloud *)userdata;
		int samples = len / (gActiveAudioSpec.channels * sizeof(short));
		soloud->mixSigned16(buf, samples);
	}

	static void soloud_sdl1_deinit(SoLoud::Soloud *aSoloud)
	{
		dll_SDL1_CloseAudio();
	}

	result sdl1_init(SoLoud::Soloud *aSoloud, unsigned int aFlags, unsigned int aSamplerate, unsigned int aBuffer, unsigned int aChannels)
	{
		if (!dll_SDL1_found())
			return DLL_NOT_FOUND;

		SDL_AudioSpec as;
		as.freq = aSamplerate;
		as.format = AUDIO_S16;
		as.channels = aChannels;
		as.samples = aBuffer;
		as.callback = soloud_sdl1_audiomixer;
		as.userdata = (void*)aSoloud;

		if (dll_SDL1_OpenAudio(&as, &gActiveAudioSpec) < 0)
		{
			return UNKNOWN_ERROR;
		}

		aSoloud->postinit_internal(gActiveAudioSpec.freq, gActiveAudioSpec.samples, aFlags, gActiveAudioSpec.channels);

		aSoloud->mBackendCleanupFunc = soloud_sdl1_deinit;

		dll_SDL1_PauseAudio(0);
        aSoloud->mBackendString = "SDL (dynamic)";
		return 0;
	}
	
};
#endif