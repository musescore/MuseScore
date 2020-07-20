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
#if defined(WITH_PORTAUDIO)

#include <stdlib.h>
#include <math.h>

#if defined(_MSC_VER)
#define WINDOWS_VERSION
#endif

#include "portaudio.h"

typedef PaError (*Pa_InitializeProc)( void );
typedef PaError (*Pa_TerminateProc)( void );
typedef PaError (*Pa_CloseStreamProc)( PaStream *stream );
typedef PaError (*Pa_StartStreamProc)( PaStream *stream );
typedef PaError (*Pa_OpenDefaultStreamProc)( PaStream** stream,
                              int numInputChannels,
                              int numOutputChannels,
                              PaSampleFormat sampleFormat,
                              double sampleRate,
                              unsigned long framesPerBuffer,
                              PaStreamCallback *streamCallback,
                              void *userData );

static Pa_InitializeProc dPa_Initialize = NULL;
static Pa_TerminateProc dPa_Terminate = NULL;
static Pa_CloseStreamProc dPa_CloseStream = NULL;
static Pa_StartStreamProc dPa_StartStream = NULL;
static Pa_OpenDefaultStreamProc dPa_OpenDefaultStream = NULL;



#ifdef WINDOWS_VERSION
#include <windows.h>

static HMODULE pta_openDll()
{
    HMODULE dllh = LoadLibrary("portaudio_x86.dll");
    return dllh;
}

static void *pta_getdllproc(HMODULE dllhandle, const char *procname)
{
    HMODULE dllh = (HMODULE)dllhandle;
    return (void*)GetProcAddress(dllh, (LPCSTR)procname);
}

#else
#include <dlfcn.h> // dll functions

static void* pta_openDll()
{
    void* res = dlopen("libportaudio_x86.so", RTLD_LAZY);

//	if (!res) res = dlopen("/Library/Frameworks/PortAudio.framework", RTLD_LAZY);

	return res;
}

static void *pta_getdllproc(void* dllhandle, const char *procname)
{
    void* library = dllhandle;
    return dlsym(library,procname);
}

#endif



static int pta_load_dll()
{
#ifdef WINDOWS_VERSION
	HMODULE dll = NULL;
#else
	void * dll = NULL;
#endif

	if (dPa_OpenDefaultStream != NULL)
	{
		return 1;
	}

    dll = pta_openDll();

    if (dll)
    {
		dPa_Initialize = (Pa_InitializeProc)pta_getdllproc(dll,"Pa_Initialize");
		dPa_Terminate = (Pa_TerminateProc)pta_getdllproc(dll,"Pa_Terminate");
		dPa_CloseStream = (Pa_CloseStreamProc)pta_getdllproc(dll,"Pa_CloseStream");
		dPa_StartStream = (Pa_StartStreamProc)pta_getdllproc(dll,"Pa_StartStream");
		dPa_OpenDefaultStream = (Pa_OpenDefaultStreamProc)pta_getdllproc(dll,"Pa_OpenDefaultStream");

		if (dPa_Initialize == NULL ||
			dPa_Terminate == NULL ||
			dPa_CloseStream == NULL ||
			dPa_StartStream == NULL ||
			dPa_OpenDefaultStream == NULL)
		{
			dPa_OpenDefaultStream = NULL;
			return 0;
		}
		return 1;
	}
	return 0;
}

int dll_Pa_found()
{
	return pta_load_dll();
}

PaError dll_Pa_Initialize( void )
{
	if (pta_load_dll())
		return dPa_Initialize();
	return paNotInitialized;
}

PaError dll_Pa_Terminate( void )
{
	if (pta_load_dll())
		return dPa_Terminate();
	return paNotInitialized;
}

PaError dll_Pa_CloseStream( PaStream *stream )
{
	if (pta_load_dll())
		return dPa_CloseStream(stream);
	return paNotInitialized;
}

PaError dll_Pa_StartStream( PaStream *stream )
{
	if (pta_load_dll())
		return dPa_StartStream(stream);
	return paNotInitialized;
}

PaError dll_Pa_OpenDefaultStream( PaStream** stream,
                            int numInputChannels,
                            int numOutputChannels,
                            PaSampleFormat sampleFormat,
                            double sampleRate,
                            unsigned long framesPerBuffer,
                            PaStreamCallback *streamCallback,
                            void *userData )
{
	if (pta_load_dll())
		return dPa_OpenDefaultStream(stream,numInputChannels,numOutputChannels,sampleFormat,sampleRate,framesPerBuffer,streamCallback,userData);
	return paNotInitialized;
}

#endif