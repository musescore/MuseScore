/*
SoLoud audio engine
Copyright (c) 2013-2018 Jari Komppa

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
#if defined(WITH_SDL2)

#include <stdlib.h>
#if defined(_MSC_VER)
#define WINDOWS_VERSION
#endif
#include "SDL.h"
#include <math.h>


typedef Uint32            (*SDL2_WasInit_t)(Uint32 flags);
typedef int               (*SDL2_InitSubSystem_t)(Uint32 flags);
typedef SDL_AudioDeviceID (*SDL2_OpenAudioDevice_t)(const char*          device,
												    int                  iscapture,
												    const SDL_AudioSpec* desired,
												    SDL_AudioSpec*       obtained,
												    int                  allowed_changes);
typedef void              (*SDL2_CloseAudioDevice_t)(SDL_AudioDeviceID dev);
typedef void              (*SDL2_PauseAudioDevice_t)(SDL_AudioDeviceID dev,
												     int               pause_on);

static SDL2_WasInit_t SDL2_WasInit = NULL;
static SDL2_InitSubSystem_t SDL2_InitSubSystem = NULL;
static SDL2_OpenAudioDevice_t SDL2_OpenAudioDevice = NULL;
static SDL2_CloseAudioDevice_t SDL2_CloseAudioDevice = NULL;
static SDL2_PauseAudioDevice_t SDL2_PauseAudioDevice = NULL;

#ifdef WINDOWS_VERSION
#include <windows.h>

static HMODULE sdl2_openDll()
{
	HMODULE res = LoadLibraryA("SDL2.dll");
    return res;
}

static void* sdl2_getDllProc(HMODULE aDllHandle, const char *aProcName)
{
    return (void*)GetProcAddress(aDllHandle, (LPCSTR)aProcName);
}

#else
#include <dlfcn.h> // dll functions

static void * sdl2_openDll()
{
	void * res;
	res = dlopen("/Library/Frameworks/SDL2.framework/SDL2", RTLD_LAZY);
	if (!res) res = dlopen("SDL2.so", RTLD_LAZY);
	if (!res) res = dlopen("libSDL2.so", RTLD_LAZY);
    return res;
}

static void* sdl2_getDllProc(void * aLibrary, const char *aProcName)
{
    return dlsym(aLibrary, aProcName);
}

#endif

static int sdl2_load_dll()
{
#ifdef WINDOWS_VERSION
	HMODULE dll = NULL;
#else
	void * dll = NULL;
#endif

	if (SDL2_OpenAudioDevice != NULL)
	{
		return 1;
	}

    dll = sdl2_openDll();

    if (dll)
    {
		SDL2_WasInit = (SDL2_WasInit_t)sdl2_getDllProc(dll, "SDL_WasInit");
		SDL2_InitSubSystem = (SDL2_InitSubSystem_t)sdl2_getDllProc(dll, "SDL_InitSubSystem");
		SDL2_OpenAudioDevice = (SDL2_OpenAudioDevice_t)sdl2_getDllProc(dll, "SDL_OpenAudioDevice");
		SDL2_CloseAudioDevice = (SDL2_CloseAudioDevice_t)sdl2_getDllProc(dll, "SDL_CloseAudioDevice");
		SDL2_PauseAudioDevice = (SDL2_PauseAudioDevice_t)sdl2_getDllProc(dll, "SDL_PauseAudioDevice");

        if (SDL2_WasInit &&
        	SDL2_InitSubSystem &&
        	SDL2_OpenAudioDevice &&
			SDL2_CloseAudioDevice &&
			SDL2_PauseAudioDevice)
        {
        	return 1;
        }
	}
	SDL2_OpenAudioDevice = NULL;
    return 0;
}

int dll_SDL2_found()
{
	return sdl2_load_dll();
}

Uint32 dll_SDL2_WasInit(Uint32 flags)
{
	if (SDL2_WasInit)
		return SDL2_WasInit(flags);
	return 0;
}

int dll_SDL2_InitSubSystem(Uint32 flags)
{
	if (SDL2_InitSubSystem)
		return SDL2_InitSubSystem(flags);
	return -1;
}

SDL_AudioDeviceID dll_SDL2_OpenAudioDevice(const char*          device,
										   int                  iscapture,
										   const SDL_AudioSpec* desired,
										   SDL_AudioSpec*       obtained,
										   int                  allowed_changes)
{
	if (SDL2_OpenAudioDevice)
		return SDL2_OpenAudioDevice(device, iscapture, desired, obtained, allowed_changes);
	return 0;
}

void dll_SDL2_CloseAudioDevice(SDL_AudioDeviceID dev)
{
	if (SDL2_CloseAudioDevice)
		SDL2_CloseAudioDevice(dev);
}

void dll_SDL2_PauseAudioDevice(SDL_AudioDeviceID dev,
							   int               pause_on)
{
	if (SDL2_PauseAudioDevice)
		SDL2_PauseAudioDevice(dev, pause_on);
}

#endif