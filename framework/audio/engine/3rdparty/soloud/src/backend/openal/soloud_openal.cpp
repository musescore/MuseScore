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
#include <math.h>

#include "soloud.h"
#include "soloud_thread.h"

#if !defined(WITH_OPENAL)

namespace SoLoud
{
	result openal_init(SoLoud::Soloud *aSoloud, unsigned int aFlags, unsigned int aSamplerate, unsigned int aBuffer)
	{
		return NOT_IMPLEMENTED;
	}
}
#else

#ifdef __APPLE__
#include "OpenAL/al.h"
#include "OpenAL/alc.h"
#else
#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"
#endif

#define NUM_BUFFERS 2

static ALCdevice* device = NULL;
static ALCcontext* context = NULL;
static ALenum format = 0;
static ALuint buffers[NUM_BUFFERS];
static ALuint source = 0;
static int frequency = 0;
static volatile int threadrun = 0;
static int buffersize = 0;
static short* bufferdata = 0;

extern "C"
{
	int dll_al_found();
	ALCdevice* dll_alc_OpenDevice(const ALCchar *devicename);
	void dll_alc_CloseDevice(ALCdevice *device);
	ALCcontext* dll_alc_CreateContext(ALCdevice *device, const ALCint* attrlist);
	void dll_alc_DestroyContext(ALCcontext *context);
	ALCboolean dll_alc_MakeContextCurrent(ALCcontext *context);
	void dll_al_GetSourcei(ALuint source, ALenum param, ALint *value);
	void dll_al_SourceQueueBuffers(ALuint source, ALsizei nb, const ALuint *buffers);
	void dll_al_SourceUnqueueBuffers(ALuint source, ALsizei nb, ALuint *buffers);
	void dll_al_BufferData(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq);
	void dll_al_SourcePlay(ALuint source);
	void dll_al_SourceStop(ALuint source);
	void dll_al_GenBuffers(ALsizei n, ALuint *buffers);
	void dll_al_DeleteBuffers(ALsizei n, ALuint *buffers);
	void dll_al_GenSources(ALsizei n, ALuint *sources);
	void dll_al_DeleteSources(ALsizei n, ALuint *sources);
}

namespace SoLoud
{
	void soloud_openal_deinit(SoLoud::Soloud * /*aSoloud*/)
	{
		threadrun++;
		while (threadrun == 1)
		{
			Thread::sleep(10);
		}

		dll_al_SourceStop(source);
		dll_al_DeleteSources(1, &source);
		dll_al_DeleteBuffers(NUM_BUFFERS, buffers);

		dll_alc_MakeContextCurrent(NULL);
		dll_alc_DestroyContext(context);
		dll_alc_CloseDevice(device);
		
		free(bufferdata);

		device = NULL;
		context = NULL;
		format = 0;
		source = 0;
		frequency = 0;
		threadrun = 0;
		buffersize = 0;
		bufferdata = 0;
	}
#if 0	
	static void openal_mutex_lock(void * mutex)
	{
		Thread::lockMutex(mutex);
	}

	static void openal_mutex_unlock(void * mutex)
	{
		Thread::unlockMutex(mutex);
	}
#endif
	static void openal_iterate(SoLoud::Soloud *aSoloud)
	{
		ALuint buffer = 0;
		ALint buffersProcessed = 0;
		ALint state;
		dll_al_GetSourcei(source, AL_BUFFERS_PROCESSED, &buffersProcessed);

		while (buffersProcessed--) 
		{
			aSoloud->mixSigned16(bufferdata,buffersize);

			dll_al_SourceUnqueueBuffers(source, 1, &buffer);

			dll_al_BufferData(buffer, format, bufferdata, buffersize*4, frequency);

			dll_al_SourceQueueBuffers(source, 1, &buffer);
		}

		dll_al_GetSourcei(source, AL_SOURCE_STATE, &state);
		if (state != AL_PLAYING)
			dll_al_SourcePlay(source);
	}

	static void openal_thread(void *aParam)
	{
		Soloud *soloud = (Soloud *)aParam;
		while (threadrun == 0)
		{
			openal_iterate(soloud);
			Thread::sleep(1);
		}
		threadrun++;
	}

	result openal_init(SoLoud::Soloud *aSoloud, unsigned int aFlags, unsigned int aSamplerate, unsigned int aBuffer, unsigned int /*aChannels*/)
	{
		if (!dll_al_found())
			return DLL_NOT_FOUND;

		aSoloud->postinit_internal(aSamplerate,aBuffer,aFlags,2);
		aSoloud->mBackendCleanupFunc = soloud_openal_deinit;

		device = dll_alc_OpenDevice(NULL);
		context = dll_alc_CreateContext(device, NULL);
		dll_alc_MakeContextCurrent(context);
		format = AL_FORMAT_STEREO16;
		dll_al_GenBuffers(NUM_BUFFERS, buffers);
		dll_al_GenSources(1, &source);
		buffersize = aBuffer;
		bufferdata = (short*)malloc(buffersize*2*2);

		if (!bufferdata)
			return OUT_OF_MEMORY;

		frequency = aSamplerate;

		int i;
		for (i = 0; i < buffersize*2; i++)
			bufferdata[i] = 0;
		for (i = 0; i < NUM_BUFFERS; i++)
		{
			dll_al_BufferData(buffers[i], format, bufferdata, buffersize, frequency);
			dll_al_SourceQueueBuffers(source, 1, &buffers[i]);
		}

		dll_al_SourcePlay(source);

		Thread::createThread(openal_thread, (void*)aSoloud);

        aSoloud->mBackendString = "OpenAL";
		return 0;
	}	
};
#endif
