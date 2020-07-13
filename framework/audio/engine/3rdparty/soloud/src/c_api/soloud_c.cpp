/* **************************************************
 *  WARNING: this is a generated file. Do not edit. *
 *  Any edits will be overwritten by the generator. *
 ************************************************** */

/*
SoLoud audio engine
Copyright (c) 2013-2020 Jari Komppa

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

/* SoLoud C-Api Code Generator (c)2013-2020 Jari Komppa http://iki.fi/sol/ */

#include "../include/soloud.h"
#include "../include/soloud_audiosource.h"
#include "../include/soloud_bassboostfilter.h"
#include "../include/soloud_biquadresonantfilter.h"
#include "../include/soloud_bus.h"
#include "../include/soloud_dcremovalfilter.h"
#include "../include/soloud_echofilter.h"
#include "../include/soloud_fader.h"
#include "../include/soloud_fft.h"
#include "../include/soloud_fftfilter.h"
#include "../include/soloud_filter.h"
#include "../include/soloud_flangerfilter.h"
#include "../include/soloud_freeverbfilter.h"
#include "../include/soloud_lofifilter.h"
#include "../include/soloud_monotone.h"
#include "../include/soloud_noise.h"
#include "../include/soloud_openmpt.h"
#include "../include/soloud_queue.h"
#include "../include/soloud_robotizefilter.h"
#include "../include/soloud_sfxr.h"
#include "../include/soloud_speech.h"
#include "../include/soloud_tedsid.h"
#include "../include/soloud_vic.h"
#include "../include/soloud_vizsn.h"
#include "../include/soloud_wav.h"
#include "../include/soloud_waveshaperfilter.h"
#include "../include/soloud_wavstream.h"

using namespace SoLoud;

extern "C"
{

void Soloud_destroy(void * aClassPtr)
{
  delete (Soloud *)aClassPtr;
}

void * Soloud_create()
{
  return (void *)new Soloud;
}

int Soloud_init(void * aClassPtr)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->init();
}

int Soloud_initEx(void * aClassPtr, unsigned int aFlags, unsigned int aBackend, unsigned int aSamplerate, unsigned int aBufferSize, unsigned int aChannels)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->init(aFlags, aBackend, aSamplerate, aBufferSize, aChannels);
}

void Soloud_deinit(void * aClassPtr)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->deinit();
}

unsigned int Soloud_getVersion(void * aClassPtr)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getVersion();
}

const char * Soloud_getErrorString(void * aClassPtr, int aErrorCode)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getErrorString(aErrorCode);
}

unsigned int Soloud_getBackendId(void * aClassPtr)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getBackendId();
}

const char * Soloud_getBackendString(void * aClassPtr)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getBackendString();
}

unsigned int Soloud_getBackendChannels(void * aClassPtr)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getBackendChannels();
}

unsigned int Soloud_getBackendSamplerate(void * aClassPtr)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getBackendSamplerate();
}

unsigned int Soloud_getBackendBufferSize(void * aClassPtr)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getBackendBufferSize();
}

int Soloud_setSpeakerPosition(void * aClassPtr, unsigned int aChannel, float aX, float aY, float aZ)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->setSpeakerPosition(aChannel, aX, aY, aZ);
}

int Soloud_getSpeakerPosition(void * aClassPtr, unsigned int aChannel, float * aX, float * aY, float * aZ)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getSpeakerPosition(aChannel, *aX, *aY, *aZ);
}

unsigned int Soloud_play(void * aClassPtr, AudioSource * aSound)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->play(*aSound);
}

unsigned int Soloud_playEx(void * aClassPtr, AudioSource * aSound, float aVolume, float aPan, int aPaused, unsigned int aBus)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->play(*aSound, aVolume, aPan, !!aPaused, aBus);
}

unsigned int Soloud_playClocked(void * aClassPtr, double aSoundTime, AudioSource * aSound)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->playClocked(aSoundTime, *aSound);
}

unsigned int Soloud_playClockedEx(void * aClassPtr, double aSoundTime, AudioSource * aSound, float aVolume, float aPan, unsigned int aBus)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->playClocked(aSoundTime, *aSound, aVolume, aPan, aBus);
}

unsigned int Soloud_play3d(void * aClassPtr, AudioSource * aSound, float aPosX, float aPosY, float aPosZ)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->play3d(*aSound, aPosX, aPosY, aPosZ);
}

unsigned int Soloud_play3dEx(void * aClassPtr, AudioSource * aSound, float aPosX, float aPosY, float aPosZ, float aVelX, float aVelY, float aVelZ, float aVolume, int aPaused, unsigned int aBus)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->play3d(*aSound, aPosX, aPosY, aPosZ, aVelX, aVelY, aVelZ, aVolume, !!aPaused, aBus);
}

unsigned int Soloud_play3dClocked(void * aClassPtr, double aSoundTime, AudioSource * aSound, float aPosX, float aPosY, float aPosZ)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->play3dClocked(aSoundTime, *aSound, aPosX, aPosY, aPosZ);
}

unsigned int Soloud_play3dClockedEx(void * aClassPtr, double aSoundTime, AudioSource * aSound, float aPosX, float aPosY, float aPosZ, float aVelX, float aVelY, float aVelZ, float aVolume, unsigned int aBus)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->play3dClocked(aSoundTime, *aSound, aPosX, aPosY, aPosZ, aVelX, aVelY, aVelZ, aVolume, aBus);
}

unsigned int Soloud_playBackground(void * aClassPtr, AudioSource * aSound)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->playBackground(*aSound);
}

unsigned int Soloud_playBackgroundEx(void * aClassPtr, AudioSource * aSound, float aVolume, int aPaused, unsigned int aBus)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->playBackground(*aSound, aVolume, !!aPaused, aBus);
}

int Soloud_seek(void * aClassPtr, unsigned int aVoiceHandle, double aSeconds)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->seek(aVoiceHandle, aSeconds);
}

void Soloud_stop(void * aClassPtr, unsigned int aVoiceHandle)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->stop(aVoiceHandle);
}

void Soloud_stopAll(void * aClassPtr)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->stopAll();
}

void Soloud_stopAudioSource(void * aClassPtr, AudioSource * aSound)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->stopAudioSource(*aSound);
}

int Soloud_countAudioSource(void * aClassPtr, AudioSource * aSound)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->countAudioSource(*aSound);
}

void Soloud_setFilterParameter(void * aClassPtr, unsigned int aVoiceHandle, unsigned int aFilterId, unsigned int aAttributeId, float aValue)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->setFilterParameter(aVoiceHandle, aFilterId, aAttributeId, aValue);
}

float Soloud_getFilterParameter(void * aClassPtr, unsigned int aVoiceHandle, unsigned int aFilterId, unsigned int aAttributeId)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getFilterParameter(aVoiceHandle, aFilterId, aAttributeId);
}

void Soloud_fadeFilterParameter(void * aClassPtr, unsigned int aVoiceHandle, unsigned int aFilterId, unsigned int aAttributeId, float aTo, double aTime)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->fadeFilterParameter(aVoiceHandle, aFilterId, aAttributeId, aTo, aTime);
}

void Soloud_oscillateFilterParameter(void * aClassPtr, unsigned int aVoiceHandle, unsigned int aFilterId, unsigned int aAttributeId, float aFrom, float aTo, double aTime)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->oscillateFilterParameter(aVoiceHandle, aFilterId, aAttributeId, aFrom, aTo, aTime);
}

double Soloud_getStreamTime(void * aClassPtr, unsigned int aVoiceHandle)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getStreamTime(aVoiceHandle);
}

double Soloud_getStreamPosition(void * aClassPtr, unsigned int aVoiceHandle)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getStreamPosition(aVoiceHandle);
}

int Soloud_getPause(void * aClassPtr, unsigned int aVoiceHandle)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getPause(aVoiceHandle);
}

float Soloud_getVolume(void * aClassPtr, unsigned int aVoiceHandle)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getVolume(aVoiceHandle);
}

float Soloud_getOverallVolume(void * aClassPtr, unsigned int aVoiceHandle)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getOverallVolume(aVoiceHandle);
}

float Soloud_getPan(void * aClassPtr, unsigned int aVoiceHandle)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getPan(aVoiceHandle);
}

float Soloud_getSamplerate(void * aClassPtr, unsigned int aVoiceHandle)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getSamplerate(aVoiceHandle);
}

int Soloud_getProtectVoice(void * aClassPtr, unsigned int aVoiceHandle)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getProtectVoice(aVoiceHandle);
}

unsigned int Soloud_getActiveVoiceCount(void * aClassPtr)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getActiveVoiceCount();
}

unsigned int Soloud_getVoiceCount(void * aClassPtr)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getVoiceCount();
}

int Soloud_isValidVoiceHandle(void * aClassPtr, unsigned int aVoiceHandle)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->isValidVoiceHandle(aVoiceHandle);
}

float Soloud_getRelativePlaySpeed(void * aClassPtr, unsigned int aVoiceHandle)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getRelativePlaySpeed(aVoiceHandle);
}

float Soloud_getPostClipScaler(void * aClassPtr)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getPostClipScaler();
}

float Soloud_getGlobalVolume(void * aClassPtr)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getGlobalVolume();
}

unsigned int Soloud_getMaxActiveVoiceCount(void * aClassPtr)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getMaxActiveVoiceCount();
}

int Soloud_getLooping(void * aClassPtr, unsigned int aVoiceHandle)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getLooping(aVoiceHandle);
}

double Soloud_getLoopPoint(void * aClassPtr, unsigned int aVoiceHandle)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getLoopPoint(aVoiceHandle);
}

void Soloud_setLoopPoint(void * aClassPtr, unsigned int aVoiceHandle, double aLoopPoint)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->setLoopPoint(aVoiceHandle, aLoopPoint);
}

void Soloud_setLooping(void * aClassPtr, unsigned int aVoiceHandle, int aLooping)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->setLooping(aVoiceHandle, !!aLooping);
}

int Soloud_setMaxActiveVoiceCount(void * aClassPtr, unsigned int aVoiceCount)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->setMaxActiveVoiceCount(aVoiceCount);
}

void Soloud_setInaudibleBehavior(void * aClassPtr, unsigned int aVoiceHandle, int aMustTick, int aKill)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->setInaudibleBehavior(aVoiceHandle, !!aMustTick, !!aKill);
}

void Soloud_setGlobalVolume(void * aClassPtr, float aVolume)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->setGlobalVolume(aVolume);
}

void Soloud_setPostClipScaler(void * aClassPtr, float aScaler)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->setPostClipScaler(aScaler);
}

void Soloud_setPause(void * aClassPtr, unsigned int aVoiceHandle, int aPause)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->setPause(aVoiceHandle, !!aPause);
}

void Soloud_setPauseAll(void * aClassPtr, int aPause)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->setPauseAll(!!aPause);
}

int Soloud_setRelativePlaySpeed(void * aClassPtr, unsigned int aVoiceHandle, float aSpeed)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->setRelativePlaySpeed(aVoiceHandle, aSpeed);
}

void Soloud_setProtectVoice(void * aClassPtr, unsigned int aVoiceHandle, int aProtect)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->setProtectVoice(aVoiceHandle, !!aProtect);
}

void Soloud_setSamplerate(void * aClassPtr, unsigned int aVoiceHandle, float aSamplerate)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->setSamplerate(aVoiceHandle, aSamplerate);
}

void Soloud_setPan(void * aClassPtr, unsigned int aVoiceHandle, float aPan)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->setPan(aVoiceHandle, aPan);
}

void Soloud_setPanAbsolute(void * aClassPtr, unsigned int aVoiceHandle, float aLVolume, float aRVolume)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->setPanAbsolute(aVoiceHandle, aLVolume, aRVolume);
}

void Soloud_setPanAbsoluteEx(void * aClassPtr, unsigned int aVoiceHandle, float aLVolume, float aRVolume, float aLBVolume, float aRBVolume, float aCVolume, float aSVolume)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->setPanAbsolute(aVoiceHandle, aLVolume, aRVolume, aLBVolume, aRBVolume, aCVolume, aSVolume);
}

void Soloud_setVolume(void * aClassPtr, unsigned int aVoiceHandle, float aVolume)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->setVolume(aVoiceHandle, aVolume);
}

void Soloud_setDelaySamples(void * aClassPtr, unsigned int aVoiceHandle, unsigned int aSamples)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->setDelaySamples(aVoiceHandle, aSamples);
}

void Soloud_fadeVolume(void * aClassPtr, unsigned int aVoiceHandle, float aTo, double aTime)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->fadeVolume(aVoiceHandle, aTo, aTime);
}

void Soloud_fadePan(void * aClassPtr, unsigned int aVoiceHandle, float aTo, double aTime)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->fadePan(aVoiceHandle, aTo, aTime);
}

void Soloud_fadeRelativePlaySpeed(void * aClassPtr, unsigned int aVoiceHandle, float aTo, double aTime)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->fadeRelativePlaySpeed(aVoiceHandle, aTo, aTime);
}

void Soloud_fadeGlobalVolume(void * aClassPtr, float aTo, double aTime)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->fadeGlobalVolume(aTo, aTime);
}

void Soloud_schedulePause(void * aClassPtr, unsigned int aVoiceHandle, double aTime)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->schedulePause(aVoiceHandle, aTime);
}

void Soloud_scheduleStop(void * aClassPtr, unsigned int aVoiceHandle, double aTime)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->scheduleStop(aVoiceHandle, aTime);
}

void Soloud_oscillateVolume(void * aClassPtr, unsigned int aVoiceHandle, float aFrom, float aTo, double aTime)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->oscillateVolume(aVoiceHandle, aFrom, aTo, aTime);
}

void Soloud_oscillatePan(void * aClassPtr, unsigned int aVoiceHandle, float aFrom, float aTo, double aTime)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->oscillatePan(aVoiceHandle, aFrom, aTo, aTime);
}

void Soloud_oscillateRelativePlaySpeed(void * aClassPtr, unsigned int aVoiceHandle, float aFrom, float aTo, double aTime)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->oscillateRelativePlaySpeed(aVoiceHandle, aFrom, aTo, aTime);
}

void Soloud_oscillateGlobalVolume(void * aClassPtr, float aFrom, float aTo, double aTime)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->oscillateGlobalVolume(aFrom, aTo, aTime);
}

void Soloud_setGlobalFilter(void * aClassPtr, unsigned int aFilterId, Filter * aFilter)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->setGlobalFilter(aFilterId, aFilter);
}

void Soloud_setVisualizationEnable(void * aClassPtr, int aEnable)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->setVisualizationEnable(!!aEnable);
}

float * Soloud_calcFFT(void * aClassPtr)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->calcFFT();
}

float * Soloud_getWave(void * aClassPtr)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getWave();
}

float Soloud_getApproximateVolume(void * aClassPtr, unsigned int aChannel)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getApproximateVolume(aChannel);
}

unsigned int Soloud_getLoopCount(void * aClassPtr, unsigned int aVoiceHandle)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getLoopCount(aVoiceHandle);
}

float Soloud_getInfo(void * aClassPtr, unsigned int aVoiceHandle, unsigned int aInfoKey)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->getInfo(aVoiceHandle, aInfoKey);
}

unsigned int Soloud_createVoiceGroup(void * aClassPtr)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->createVoiceGroup();
}

int Soloud_destroyVoiceGroup(void * aClassPtr, unsigned int aVoiceGroupHandle)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->destroyVoiceGroup(aVoiceGroupHandle);
}

int Soloud_addVoiceToGroup(void * aClassPtr, unsigned int aVoiceGroupHandle, unsigned int aVoiceHandle)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->addVoiceToGroup(aVoiceGroupHandle, aVoiceHandle);
}

int Soloud_isVoiceGroup(void * aClassPtr, unsigned int aVoiceGroupHandle)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->isVoiceGroup(aVoiceGroupHandle);
}

int Soloud_isVoiceGroupEmpty(void * aClassPtr, unsigned int aVoiceGroupHandle)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->isVoiceGroupEmpty(aVoiceGroupHandle);
}

void Soloud_update3dAudio(void * aClassPtr)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->update3dAudio();
}

int Soloud_set3dSoundSpeed(void * aClassPtr, float aSpeed)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->set3dSoundSpeed(aSpeed);
}

float Soloud_get3dSoundSpeed(void * aClassPtr)
{
	Soloud * cl = (Soloud *)aClassPtr;
	return cl->get3dSoundSpeed();
}

void Soloud_set3dListenerParameters(void * aClassPtr, float aPosX, float aPosY, float aPosZ, float aAtX, float aAtY, float aAtZ, float aUpX, float aUpY, float aUpZ)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->set3dListenerParameters(aPosX, aPosY, aPosZ, aAtX, aAtY, aAtZ, aUpX, aUpY, aUpZ);
}

void Soloud_set3dListenerParametersEx(void * aClassPtr, float aPosX, float aPosY, float aPosZ, float aAtX, float aAtY, float aAtZ, float aUpX, float aUpY, float aUpZ, float aVelocityX, float aVelocityY, float aVelocityZ)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->set3dListenerParameters(aPosX, aPosY, aPosZ, aAtX, aAtY, aAtZ, aUpX, aUpY, aUpZ, aVelocityX, aVelocityY, aVelocityZ);
}

void Soloud_set3dListenerPosition(void * aClassPtr, float aPosX, float aPosY, float aPosZ)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->set3dListenerPosition(aPosX, aPosY, aPosZ);
}

void Soloud_set3dListenerAt(void * aClassPtr, float aAtX, float aAtY, float aAtZ)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->set3dListenerAt(aAtX, aAtY, aAtZ);
}

void Soloud_set3dListenerUp(void * aClassPtr, float aUpX, float aUpY, float aUpZ)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->set3dListenerUp(aUpX, aUpY, aUpZ);
}

void Soloud_set3dListenerVelocity(void * aClassPtr, float aVelocityX, float aVelocityY, float aVelocityZ)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->set3dListenerVelocity(aVelocityX, aVelocityY, aVelocityZ);
}

void Soloud_set3dSourceParameters(void * aClassPtr, unsigned int aVoiceHandle, float aPosX, float aPosY, float aPosZ)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->set3dSourceParameters(aVoiceHandle, aPosX, aPosY, aPosZ);
}

void Soloud_set3dSourceParametersEx(void * aClassPtr, unsigned int aVoiceHandle, float aPosX, float aPosY, float aPosZ, float aVelocityX, float aVelocityY, float aVelocityZ)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->set3dSourceParameters(aVoiceHandle, aPosX, aPosY, aPosZ, aVelocityX, aVelocityY, aVelocityZ);
}

void Soloud_set3dSourcePosition(void * aClassPtr, unsigned int aVoiceHandle, float aPosX, float aPosY, float aPosZ)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->set3dSourcePosition(aVoiceHandle, aPosX, aPosY, aPosZ);
}

void Soloud_set3dSourceVelocity(void * aClassPtr, unsigned int aVoiceHandle, float aVelocityX, float aVelocityY, float aVelocityZ)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->set3dSourceVelocity(aVoiceHandle, aVelocityX, aVelocityY, aVelocityZ);
}

void Soloud_set3dSourceMinMaxDistance(void * aClassPtr, unsigned int aVoiceHandle, float aMinDistance, float aMaxDistance)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->set3dSourceMinMaxDistance(aVoiceHandle, aMinDistance, aMaxDistance);
}

void Soloud_set3dSourceAttenuation(void * aClassPtr, unsigned int aVoiceHandle, unsigned int aAttenuationModel, float aAttenuationRolloffFactor)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->set3dSourceAttenuation(aVoiceHandle, aAttenuationModel, aAttenuationRolloffFactor);
}

void Soloud_set3dSourceDopplerFactor(void * aClassPtr, unsigned int aVoiceHandle, float aDopplerFactor)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->set3dSourceDopplerFactor(aVoiceHandle, aDopplerFactor);
}

void Soloud_mix(void * aClassPtr, float * aBuffer, unsigned int aSamples)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->mix(aBuffer, aSamples);
}

void Soloud_mixSigned16(void * aClassPtr, short * aBuffer, unsigned int aSamples)
{
	Soloud * cl = (Soloud *)aClassPtr;
	cl->mixSigned16(aBuffer, aSamples);
}

void BassboostFilter_destroy(void * aClassPtr)
{
  delete (BassboostFilter *)aClassPtr;
}

int BassboostFilter_getParamCount(void * aClassPtr)
{
	BassboostFilter * cl = (BassboostFilter *)aClassPtr;
	return cl->getParamCount();
}

const char * BassboostFilter_getParamName(void * aClassPtr, unsigned int aParamIndex)
{
	BassboostFilter * cl = (BassboostFilter *)aClassPtr;
	return cl->getParamName(aParamIndex);
}

unsigned int BassboostFilter_getParamType(void * aClassPtr, unsigned int aParamIndex)
{
	BassboostFilter * cl = (BassboostFilter *)aClassPtr;
	return cl->getParamType(aParamIndex);
}

float BassboostFilter_getParamMax(void * aClassPtr, unsigned int aParamIndex)
{
	BassboostFilter * cl = (BassboostFilter *)aClassPtr;
	return cl->getParamMax(aParamIndex);
}

float BassboostFilter_getParamMin(void * aClassPtr, unsigned int aParamIndex)
{
	BassboostFilter * cl = (BassboostFilter *)aClassPtr;
	return cl->getParamMin(aParamIndex);
}

int BassboostFilter_setParams(void * aClassPtr, float aBoost)
{
	BassboostFilter * cl = (BassboostFilter *)aClassPtr;
	return cl->setParams(aBoost);
}

void * BassboostFilter_create()
{
  return (void *)new BassboostFilter;
}

void BiquadResonantFilter_destroy(void * aClassPtr)
{
  delete (BiquadResonantFilter *)aClassPtr;
}

int BiquadResonantFilter_getParamCount(void * aClassPtr)
{
	BiquadResonantFilter * cl = (BiquadResonantFilter *)aClassPtr;
	return cl->getParamCount();
}

const char * BiquadResonantFilter_getParamName(void * aClassPtr, unsigned int aParamIndex)
{
	BiquadResonantFilter * cl = (BiquadResonantFilter *)aClassPtr;
	return cl->getParamName(aParamIndex);
}

unsigned int BiquadResonantFilter_getParamType(void * aClassPtr, unsigned int aParamIndex)
{
	BiquadResonantFilter * cl = (BiquadResonantFilter *)aClassPtr;
	return cl->getParamType(aParamIndex);
}

float BiquadResonantFilter_getParamMax(void * aClassPtr, unsigned int aParamIndex)
{
	BiquadResonantFilter * cl = (BiquadResonantFilter *)aClassPtr;
	return cl->getParamMax(aParamIndex);
}

float BiquadResonantFilter_getParamMin(void * aClassPtr, unsigned int aParamIndex)
{
	BiquadResonantFilter * cl = (BiquadResonantFilter *)aClassPtr;
	return cl->getParamMin(aParamIndex);
}

void * BiquadResonantFilter_create()
{
  return (void *)new BiquadResonantFilter;
}

int BiquadResonantFilter_setParams(void * aClassPtr, int aType, float aFrequency, float aResonance)
{
	BiquadResonantFilter * cl = (BiquadResonantFilter *)aClassPtr;
	return cl->setParams(aType, aFrequency, aResonance);
}

void Bus_destroy(void * aClassPtr)
{
  delete (Bus *)aClassPtr;
}

void * Bus_create()
{
  return (void *)new Bus;
}

void Bus_setFilter(void * aClassPtr, unsigned int aFilterId, Filter * aFilter)
{
	Bus * cl = (Bus *)aClassPtr;
	cl->setFilter(aFilterId, aFilter);
}

unsigned int Bus_play(void * aClassPtr, AudioSource * aSound)
{
	Bus * cl = (Bus *)aClassPtr;
	return cl->play(*aSound);
}

unsigned int Bus_playEx(void * aClassPtr, AudioSource * aSound, float aVolume, float aPan, int aPaused)
{
	Bus * cl = (Bus *)aClassPtr;
	return cl->play(*aSound, aVolume, aPan, !!aPaused);
}

unsigned int Bus_playClocked(void * aClassPtr, double aSoundTime, AudioSource * aSound)
{
	Bus * cl = (Bus *)aClassPtr;
	return cl->playClocked(aSoundTime, *aSound);
}

unsigned int Bus_playClockedEx(void * aClassPtr, double aSoundTime, AudioSource * aSound, float aVolume, float aPan)
{
	Bus * cl = (Bus *)aClassPtr;
	return cl->playClocked(aSoundTime, *aSound, aVolume, aPan);
}

unsigned int Bus_play3d(void * aClassPtr, AudioSource * aSound, float aPosX, float aPosY, float aPosZ)
{
	Bus * cl = (Bus *)aClassPtr;
	return cl->play3d(*aSound, aPosX, aPosY, aPosZ);
}

unsigned int Bus_play3dEx(void * aClassPtr, AudioSource * aSound, float aPosX, float aPosY, float aPosZ, float aVelX, float aVelY, float aVelZ, float aVolume, int aPaused)
{
	Bus * cl = (Bus *)aClassPtr;
	return cl->play3d(*aSound, aPosX, aPosY, aPosZ, aVelX, aVelY, aVelZ, aVolume, !!aPaused);
}

unsigned int Bus_play3dClocked(void * aClassPtr, double aSoundTime, AudioSource * aSound, float aPosX, float aPosY, float aPosZ)
{
	Bus * cl = (Bus *)aClassPtr;
	return cl->play3dClocked(aSoundTime, *aSound, aPosX, aPosY, aPosZ);
}

unsigned int Bus_play3dClockedEx(void * aClassPtr, double aSoundTime, AudioSource * aSound, float aPosX, float aPosY, float aPosZ, float aVelX, float aVelY, float aVelZ, float aVolume)
{
	Bus * cl = (Bus *)aClassPtr;
	return cl->play3dClocked(aSoundTime, *aSound, aPosX, aPosY, aPosZ, aVelX, aVelY, aVelZ, aVolume);
}

int Bus_setChannels(void * aClassPtr, unsigned int aChannels)
{
	Bus * cl = (Bus *)aClassPtr;
	return cl->setChannels(aChannels);
}

void Bus_setVisualizationEnable(void * aClassPtr, int aEnable)
{
	Bus * cl = (Bus *)aClassPtr;
	cl->setVisualizationEnable(!!aEnable);
}

void Bus_annexSound(void * aClassPtr, unsigned int aVoiceHandle)
{
	Bus * cl = (Bus *)aClassPtr;
	cl->annexSound(aVoiceHandle);
}

float * Bus_calcFFT(void * aClassPtr)
{
	Bus * cl = (Bus *)aClassPtr;
	return cl->calcFFT();
}

float * Bus_getWave(void * aClassPtr)
{
	Bus * cl = (Bus *)aClassPtr;
	return cl->getWave();
}

float Bus_getApproximateVolume(void * aClassPtr, unsigned int aChannel)
{
	Bus * cl = (Bus *)aClassPtr;
	return cl->getApproximateVolume(aChannel);
}

unsigned int Bus_getActiveVoiceCount(void * aClassPtr)
{
	Bus * cl = (Bus *)aClassPtr;
	return cl->getActiveVoiceCount();
}

void Bus_setVolume(void * aClassPtr, float aVolume)
{
	Bus * cl = (Bus *)aClassPtr;
	cl->setVolume(aVolume);
}

void Bus_setLooping(void * aClassPtr, int aLoop)
{
	Bus * cl = (Bus *)aClassPtr;
	cl->setLooping(!!aLoop);
}

void Bus_set3dMinMaxDistance(void * aClassPtr, float aMinDistance, float aMaxDistance)
{
	Bus * cl = (Bus *)aClassPtr;
	cl->set3dMinMaxDistance(aMinDistance, aMaxDistance);
}

void Bus_set3dAttenuation(void * aClassPtr, unsigned int aAttenuationModel, float aAttenuationRolloffFactor)
{
	Bus * cl = (Bus *)aClassPtr;
	cl->set3dAttenuation(aAttenuationModel, aAttenuationRolloffFactor);
}

void Bus_set3dDopplerFactor(void * aClassPtr, float aDopplerFactor)
{
	Bus * cl = (Bus *)aClassPtr;
	cl->set3dDopplerFactor(aDopplerFactor);
}

void Bus_set3dListenerRelative(void * aClassPtr, int aListenerRelative)
{
	Bus * cl = (Bus *)aClassPtr;
	cl->set3dListenerRelative(!!aListenerRelative);
}

void Bus_set3dDistanceDelay(void * aClassPtr, int aDistanceDelay)
{
	Bus * cl = (Bus *)aClassPtr;
	cl->set3dDistanceDelay(!!aDistanceDelay);
}

void Bus_set3dCollider(void * aClassPtr, AudioCollider * aCollider)
{
	Bus * cl = (Bus *)aClassPtr;
	cl->set3dCollider(aCollider);
}

void Bus_set3dColliderEx(void * aClassPtr, AudioCollider * aCollider, int aUserData)
{
	Bus * cl = (Bus *)aClassPtr;
	cl->set3dCollider(aCollider, aUserData);
}

void Bus_set3dAttenuator(void * aClassPtr, AudioAttenuator * aAttenuator)
{
	Bus * cl = (Bus *)aClassPtr;
	cl->set3dAttenuator(aAttenuator);
}

void Bus_setInaudibleBehavior(void * aClassPtr, int aMustTick, int aKill)
{
	Bus * cl = (Bus *)aClassPtr;
	cl->setInaudibleBehavior(!!aMustTick, !!aKill);
}

void Bus_setLoopPoint(void * aClassPtr, double aLoopPoint)
{
	Bus * cl = (Bus *)aClassPtr;
	cl->setLoopPoint(aLoopPoint);
}

double Bus_getLoopPoint(void * aClassPtr)
{
	Bus * cl = (Bus *)aClassPtr;
	return cl->getLoopPoint();
}

void Bus_stop(void * aClassPtr)
{
	Bus * cl = (Bus *)aClassPtr;
	cl->stop();
}

void DCRemovalFilter_destroy(void * aClassPtr)
{
  delete (DCRemovalFilter *)aClassPtr;
}

void * DCRemovalFilter_create()
{
  return (void *)new DCRemovalFilter;
}

int DCRemovalFilter_setParams(void * aClassPtr)
{
	DCRemovalFilter * cl = (DCRemovalFilter *)aClassPtr;
	return cl->setParams();
}

int DCRemovalFilter_setParamsEx(void * aClassPtr, float aLength)
{
	DCRemovalFilter * cl = (DCRemovalFilter *)aClassPtr;
	return cl->setParams(aLength);
}

int DCRemovalFilter_getParamCount(void * aClassPtr)
{
	DCRemovalFilter * cl = (DCRemovalFilter *)aClassPtr;
	return cl->getParamCount();
}

const char * DCRemovalFilter_getParamName(void * aClassPtr, unsigned int aParamIndex)
{
	DCRemovalFilter * cl = (DCRemovalFilter *)aClassPtr;
	return cl->getParamName(aParamIndex);
}

unsigned int DCRemovalFilter_getParamType(void * aClassPtr, unsigned int aParamIndex)
{
	DCRemovalFilter * cl = (DCRemovalFilter *)aClassPtr;
	return cl->getParamType(aParamIndex);
}

float DCRemovalFilter_getParamMax(void * aClassPtr, unsigned int aParamIndex)
{
	DCRemovalFilter * cl = (DCRemovalFilter *)aClassPtr;
	return cl->getParamMax(aParamIndex);
}

float DCRemovalFilter_getParamMin(void * aClassPtr, unsigned int aParamIndex)
{
	DCRemovalFilter * cl = (DCRemovalFilter *)aClassPtr;
	return cl->getParamMin(aParamIndex);
}

void EchoFilter_destroy(void * aClassPtr)
{
  delete (EchoFilter *)aClassPtr;
}

int EchoFilter_getParamCount(void * aClassPtr)
{
	EchoFilter * cl = (EchoFilter *)aClassPtr;
	return cl->getParamCount();
}

const char * EchoFilter_getParamName(void * aClassPtr, unsigned int aParamIndex)
{
	EchoFilter * cl = (EchoFilter *)aClassPtr;
	return cl->getParamName(aParamIndex);
}

unsigned int EchoFilter_getParamType(void * aClassPtr, unsigned int aParamIndex)
{
	EchoFilter * cl = (EchoFilter *)aClassPtr;
	return cl->getParamType(aParamIndex);
}

float EchoFilter_getParamMax(void * aClassPtr, unsigned int aParamIndex)
{
	EchoFilter * cl = (EchoFilter *)aClassPtr;
	return cl->getParamMax(aParamIndex);
}

float EchoFilter_getParamMin(void * aClassPtr, unsigned int aParamIndex)
{
	EchoFilter * cl = (EchoFilter *)aClassPtr;
	return cl->getParamMin(aParamIndex);
}

void * EchoFilter_create()
{
  return (void *)new EchoFilter;
}

int EchoFilter_setParams(void * aClassPtr, float aDelay)
{
	EchoFilter * cl = (EchoFilter *)aClassPtr;
	return cl->setParams(aDelay);
}

int EchoFilter_setParamsEx(void * aClassPtr, float aDelay, float aDecay, float aFilter)
{
	EchoFilter * cl = (EchoFilter *)aClassPtr;
	return cl->setParams(aDelay, aDecay, aFilter);
}

void FFTFilter_destroy(void * aClassPtr)
{
  delete (FFTFilter *)aClassPtr;
}

void * FFTFilter_create()
{
  return (void *)new FFTFilter;
}

int FFTFilter_getParamCount(void * aClassPtr)
{
	FFTFilter * cl = (FFTFilter *)aClassPtr;
	return cl->getParamCount();
}

const char * FFTFilter_getParamName(void * aClassPtr, unsigned int aParamIndex)
{
	FFTFilter * cl = (FFTFilter *)aClassPtr;
	return cl->getParamName(aParamIndex);
}

unsigned int FFTFilter_getParamType(void * aClassPtr, unsigned int aParamIndex)
{
	FFTFilter * cl = (FFTFilter *)aClassPtr;
	return cl->getParamType(aParamIndex);
}

float FFTFilter_getParamMax(void * aClassPtr, unsigned int aParamIndex)
{
	FFTFilter * cl = (FFTFilter *)aClassPtr;
	return cl->getParamMax(aParamIndex);
}

float FFTFilter_getParamMin(void * aClassPtr, unsigned int aParamIndex)
{
	FFTFilter * cl = (FFTFilter *)aClassPtr;
	return cl->getParamMin(aParamIndex);
}

void FlangerFilter_destroy(void * aClassPtr)
{
  delete (FlangerFilter *)aClassPtr;
}

int FlangerFilter_getParamCount(void * aClassPtr)
{
	FlangerFilter * cl = (FlangerFilter *)aClassPtr;
	return cl->getParamCount();
}

const char * FlangerFilter_getParamName(void * aClassPtr, unsigned int aParamIndex)
{
	FlangerFilter * cl = (FlangerFilter *)aClassPtr;
	return cl->getParamName(aParamIndex);
}

unsigned int FlangerFilter_getParamType(void * aClassPtr, unsigned int aParamIndex)
{
	FlangerFilter * cl = (FlangerFilter *)aClassPtr;
	return cl->getParamType(aParamIndex);
}

float FlangerFilter_getParamMax(void * aClassPtr, unsigned int aParamIndex)
{
	FlangerFilter * cl = (FlangerFilter *)aClassPtr;
	return cl->getParamMax(aParamIndex);
}

float FlangerFilter_getParamMin(void * aClassPtr, unsigned int aParamIndex)
{
	FlangerFilter * cl = (FlangerFilter *)aClassPtr;
	return cl->getParamMin(aParamIndex);
}

void * FlangerFilter_create()
{
  return (void *)new FlangerFilter;
}

int FlangerFilter_setParams(void * aClassPtr, float aDelay, float aFreq)
{
	FlangerFilter * cl = (FlangerFilter *)aClassPtr;
	return cl->setParams(aDelay, aFreq);
}

void FreeverbFilter_destroy(void * aClassPtr)
{
  delete (FreeverbFilter *)aClassPtr;
}

int FreeverbFilter_getParamCount(void * aClassPtr)
{
	FreeverbFilter * cl = (FreeverbFilter *)aClassPtr;
	return cl->getParamCount();
}

const char * FreeverbFilter_getParamName(void * aClassPtr, unsigned int aParamIndex)
{
	FreeverbFilter * cl = (FreeverbFilter *)aClassPtr;
	return cl->getParamName(aParamIndex);
}

unsigned int FreeverbFilter_getParamType(void * aClassPtr, unsigned int aParamIndex)
{
	FreeverbFilter * cl = (FreeverbFilter *)aClassPtr;
	return cl->getParamType(aParamIndex);
}

float FreeverbFilter_getParamMax(void * aClassPtr, unsigned int aParamIndex)
{
	FreeverbFilter * cl = (FreeverbFilter *)aClassPtr;
	return cl->getParamMax(aParamIndex);
}

float FreeverbFilter_getParamMin(void * aClassPtr, unsigned int aParamIndex)
{
	FreeverbFilter * cl = (FreeverbFilter *)aClassPtr;
	return cl->getParamMin(aParamIndex);
}

void * FreeverbFilter_create()
{
  return (void *)new FreeverbFilter;
}

int FreeverbFilter_setParams(void * aClassPtr, float aMode, float aRoomSize, float aDamp, float aWidth)
{
	FreeverbFilter * cl = (FreeverbFilter *)aClassPtr;
	return cl->setParams(aMode, aRoomSize, aDamp, aWidth);
}

void LofiFilter_destroy(void * aClassPtr)
{
  delete (LofiFilter *)aClassPtr;
}

int LofiFilter_getParamCount(void * aClassPtr)
{
	LofiFilter * cl = (LofiFilter *)aClassPtr;
	return cl->getParamCount();
}

const char * LofiFilter_getParamName(void * aClassPtr, unsigned int aParamIndex)
{
	LofiFilter * cl = (LofiFilter *)aClassPtr;
	return cl->getParamName(aParamIndex);
}

unsigned int LofiFilter_getParamType(void * aClassPtr, unsigned int aParamIndex)
{
	LofiFilter * cl = (LofiFilter *)aClassPtr;
	return cl->getParamType(aParamIndex);
}

float LofiFilter_getParamMax(void * aClassPtr, unsigned int aParamIndex)
{
	LofiFilter * cl = (LofiFilter *)aClassPtr;
	return cl->getParamMax(aParamIndex);
}

float LofiFilter_getParamMin(void * aClassPtr, unsigned int aParamIndex)
{
	LofiFilter * cl = (LofiFilter *)aClassPtr;
	return cl->getParamMin(aParamIndex);
}

void * LofiFilter_create()
{
  return (void *)new LofiFilter;
}

int LofiFilter_setParams(void * aClassPtr, float aSampleRate, float aBitdepth)
{
	LofiFilter * cl = (LofiFilter *)aClassPtr;
	return cl->setParams(aSampleRate, aBitdepth);
}

void Monotone_destroy(void * aClassPtr)
{
  delete (Monotone *)aClassPtr;
}

void * Monotone_create()
{
  return (void *)new Monotone;
}

int Monotone_setParams(void * aClassPtr, int aHardwareChannels)
{
	Monotone * cl = (Monotone *)aClassPtr;
	return cl->setParams(aHardwareChannels);
}

int Monotone_setParamsEx(void * aClassPtr, int aHardwareChannels, int aWaveform)
{
	Monotone * cl = (Monotone *)aClassPtr;
	return cl->setParams(aHardwareChannels, aWaveform);
}

int Monotone_load(void * aClassPtr, const char * aFilename)
{
	Monotone * cl = (Monotone *)aClassPtr;
	return cl->load(aFilename);
}

int Monotone_loadMem(void * aClassPtr, const unsigned char * aMem, unsigned int aLength)
{
	Monotone * cl = (Monotone *)aClassPtr;
	return cl->loadMem(aMem, aLength);
}

int Monotone_loadMemEx(void * aClassPtr, const unsigned char * aMem, unsigned int aLength, int aCopy, int aTakeOwnership)
{
	Monotone * cl = (Monotone *)aClassPtr;
	return cl->loadMem(aMem, aLength, !!aCopy, !!aTakeOwnership);
}

int Monotone_loadFile(void * aClassPtr, File * aFile)
{
	Monotone * cl = (Monotone *)aClassPtr;
	return cl->loadFile(aFile);
}

void Monotone_setVolume(void * aClassPtr, float aVolume)
{
	Monotone * cl = (Monotone *)aClassPtr;
	cl->setVolume(aVolume);
}

void Monotone_setLooping(void * aClassPtr, int aLoop)
{
	Monotone * cl = (Monotone *)aClassPtr;
	cl->setLooping(!!aLoop);
}

void Monotone_set3dMinMaxDistance(void * aClassPtr, float aMinDistance, float aMaxDistance)
{
	Monotone * cl = (Monotone *)aClassPtr;
	cl->set3dMinMaxDistance(aMinDistance, aMaxDistance);
}

void Monotone_set3dAttenuation(void * aClassPtr, unsigned int aAttenuationModel, float aAttenuationRolloffFactor)
{
	Monotone * cl = (Monotone *)aClassPtr;
	cl->set3dAttenuation(aAttenuationModel, aAttenuationRolloffFactor);
}

void Monotone_set3dDopplerFactor(void * aClassPtr, float aDopplerFactor)
{
	Monotone * cl = (Monotone *)aClassPtr;
	cl->set3dDopplerFactor(aDopplerFactor);
}

void Monotone_set3dListenerRelative(void * aClassPtr, int aListenerRelative)
{
	Monotone * cl = (Monotone *)aClassPtr;
	cl->set3dListenerRelative(!!aListenerRelative);
}

void Monotone_set3dDistanceDelay(void * aClassPtr, int aDistanceDelay)
{
	Monotone * cl = (Monotone *)aClassPtr;
	cl->set3dDistanceDelay(!!aDistanceDelay);
}

void Monotone_set3dCollider(void * aClassPtr, AudioCollider * aCollider)
{
	Monotone * cl = (Monotone *)aClassPtr;
	cl->set3dCollider(aCollider);
}

void Monotone_set3dColliderEx(void * aClassPtr, AudioCollider * aCollider, int aUserData)
{
	Monotone * cl = (Monotone *)aClassPtr;
	cl->set3dCollider(aCollider, aUserData);
}

void Monotone_set3dAttenuator(void * aClassPtr, AudioAttenuator * aAttenuator)
{
	Monotone * cl = (Monotone *)aClassPtr;
	cl->set3dAttenuator(aAttenuator);
}

void Monotone_setInaudibleBehavior(void * aClassPtr, int aMustTick, int aKill)
{
	Monotone * cl = (Monotone *)aClassPtr;
	cl->setInaudibleBehavior(!!aMustTick, !!aKill);
}

void Monotone_setLoopPoint(void * aClassPtr, double aLoopPoint)
{
	Monotone * cl = (Monotone *)aClassPtr;
	cl->setLoopPoint(aLoopPoint);
}

double Monotone_getLoopPoint(void * aClassPtr)
{
	Monotone * cl = (Monotone *)aClassPtr;
	return cl->getLoopPoint();
}

void Monotone_setFilter(void * aClassPtr, unsigned int aFilterId, Filter * aFilter)
{
	Monotone * cl = (Monotone *)aClassPtr;
	cl->setFilter(aFilterId, aFilter);
}

void Monotone_stop(void * aClassPtr)
{
	Monotone * cl = (Monotone *)aClassPtr;
	cl->stop();
}

void Noise_destroy(void * aClassPtr)
{
  delete (Noise *)aClassPtr;
}

void * Noise_create()
{
  return (void *)new Noise;
}

void Noise_setOctaveScale(void * aClassPtr, float aOct0, float aOct1, float aOct2, float aOct3, float aOct4, float aOct5, float aOct6, float aOct7, float aOct8, float aOct9)
{
	Noise * cl = (Noise *)aClassPtr;
	cl->setOctaveScale(aOct0, aOct1, aOct2, aOct3, aOct4, aOct5, aOct6, aOct7, aOct8, aOct9);
}

void Noise_setType(void * aClassPtr, int aType)
{
	Noise * cl = (Noise *)aClassPtr;
	cl->setType(aType);
}

void Noise_setVolume(void * aClassPtr, float aVolume)
{
	Noise * cl = (Noise *)aClassPtr;
	cl->setVolume(aVolume);
}

void Noise_setLooping(void * aClassPtr, int aLoop)
{
	Noise * cl = (Noise *)aClassPtr;
	cl->setLooping(!!aLoop);
}

void Noise_set3dMinMaxDistance(void * aClassPtr, float aMinDistance, float aMaxDistance)
{
	Noise * cl = (Noise *)aClassPtr;
	cl->set3dMinMaxDistance(aMinDistance, aMaxDistance);
}

void Noise_set3dAttenuation(void * aClassPtr, unsigned int aAttenuationModel, float aAttenuationRolloffFactor)
{
	Noise * cl = (Noise *)aClassPtr;
	cl->set3dAttenuation(aAttenuationModel, aAttenuationRolloffFactor);
}

void Noise_set3dDopplerFactor(void * aClassPtr, float aDopplerFactor)
{
	Noise * cl = (Noise *)aClassPtr;
	cl->set3dDopplerFactor(aDopplerFactor);
}

void Noise_set3dListenerRelative(void * aClassPtr, int aListenerRelative)
{
	Noise * cl = (Noise *)aClassPtr;
	cl->set3dListenerRelative(!!aListenerRelative);
}

void Noise_set3dDistanceDelay(void * aClassPtr, int aDistanceDelay)
{
	Noise * cl = (Noise *)aClassPtr;
	cl->set3dDistanceDelay(!!aDistanceDelay);
}

void Noise_set3dCollider(void * aClassPtr, AudioCollider * aCollider)
{
	Noise * cl = (Noise *)aClassPtr;
	cl->set3dCollider(aCollider);
}

void Noise_set3dColliderEx(void * aClassPtr, AudioCollider * aCollider, int aUserData)
{
	Noise * cl = (Noise *)aClassPtr;
	cl->set3dCollider(aCollider, aUserData);
}

void Noise_set3dAttenuator(void * aClassPtr, AudioAttenuator * aAttenuator)
{
	Noise * cl = (Noise *)aClassPtr;
	cl->set3dAttenuator(aAttenuator);
}

void Noise_setInaudibleBehavior(void * aClassPtr, int aMustTick, int aKill)
{
	Noise * cl = (Noise *)aClassPtr;
	cl->setInaudibleBehavior(!!aMustTick, !!aKill);
}

void Noise_setLoopPoint(void * aClassPtr, double aLoopPoint)
{
	Noise * cl = (Noise *)aClassPtr;
	cl->setLoopPoint(aLoopPoint);
}

double Noise_getLoopPoint(void * aClassPtr)
{
	Noise * cl = (Noise *)aClassPtr;
	return cl->getLoopPoint();
}

void Noise_setFilter(void * aClassPtr, unsigned int aFilterId, Filter * aFilter)
{
	Noise * cl = (Noise *)aClassPtr;
	cl->setFilter(aFilterId, aFilter);
}

void Noise_stop(void * aClassPtr)
{
	Noise * cl = (Noise *)aClassPtr;
	cl->stop();
}

void Openmpt_destroy(void * aClassPtr)
{
  delete (Openmpt *)aClassPtr;
}

void * Openmpt_create()
{
  return (void *)new Openmpt;
}

int Openmpt_load(void * aClassPtr, const char * aFilename)
{
	Openmpt * cl = (Openmpt *)aClassPtr;
	return cl->load(aFilename);
}

int Openmpt_loadMem(void * aClassPtr, const unsigned char * aMem, unsigned int aLength)
{
	Openmpt * cl = (Openmpt *)aClassPtr;
	return cl->loadMem(aMem, aLength);
}

int Openmpt_loadMemEx(void * aClassPtr, const unsigned char * aMem, unsigned int aLength, int aCopy, int aTakeOwnership)
{
	Openmpt * cl = (Openmpt *)aClassPtr;
	return cl->loadMem(aMem, aLength, !!aCopy, !!aTakeOwnership);
}

int Openmpt_loadFile(void * aClassPtr, File * aFile)
{
	Openmpt * cl = (Openmpt *)aClassPtr;
	return cl->loadFile(aFile);
}

void Openmpt_setVolume(void * aClassPtr, float aVolume)
{
	Openmpt * cl = (Openmpt *)aClassPtr;
	cl->setVolume(aVolume);
}

void Openmpt_setLooping(void * aClassPtr, int aLoop)
{
	Openmpt * cl = (Openmpt *)aClassPtr;
	cl->setLooping(!!aLoop);
}

void Openmpt_set3dMinMaxDistance(void * aClassPtr, float aMinDistance, float aMaxDistance)
{
	Openmpt * cl = (Openmpt *)aClassPtr;
	cl->set3dMinMaxDistance(aMinDistance, aMaxDistance);
}

void Openmpt_set3dAttenuation(void * aClassPtr, unsigned int aAttenuationModel, float aAttenuationRolloffFactor)
{
	Openmpt * cl = (Openmpt *)aClassPtr;
	cl->set3dAttenuation(aAttenuationModel, aAttenuationRolloffFactor);
}

void Openmpt_set3dDopplerFactor(void * aClassPtr, float aDopplerFactor)
{
	Openmpt * cl = (Openmpt *)aClassPtr;
	cl->set3dDopplerFactor(aDopplerFactor);
}

void Openmpt_set3dListenerRelative(void * aClassPtr, int aListenerRelative)
{
	Openmpt * cl = (Openmpt *)aClassPtr;
	cl->set3dListenerRelative(!!aListenerRelative);
}

void Openmpt_set3dDistanceDelay(void * aClassPtr, int aDistanceDelay)
{
	Openmpt * cl = (Openmpt *)aClassPtr;
	cl->set3dDistanceDelay(!!aDistanceDelay);
}

void Openmpt_set3dCollider(void * aClassPtr, AudioCollider * aCollider)
{
	Openmpt * cl = (Openmpt *)aClassPtr;
	cl->set3dCollider(aCollider);
}

void Openmpt_set3dColliderEx(void * aClassPtr, AudioCollider * aCollider, int aUserData)
{
	Openmpt * cl = (Openmpt *)aClassPtr;
	cl->set3dCollider(aCollider, aUserData);
}

void Openmpt_set3dAttenuator(void * aClassPtr, AudioAttenuator * aAttenuator)
{
	Openmpt * cl = (Openmpt *)aClassPtr;
	cl->set3dAttenuator(aAttenuator);
}

void Openmpt_setInaudibleBehavior(void * aClassPtr, int aMustTick, int aKill)
{
	Openmpt * cl = (Openmpt *)aClassPtr;
	cl->setInaudibleBehavior(!!aMustTick, !!aKill);
}

void Openmpt_setLoopPoint(void * aClassPtr, double aLoopPoint)
{
	Openmpt * cl = (Openmpt *)aClassPtr;
	cl->setLoopPoint(aLoopPoint);
}

double Openmpt_getLoopPoint(void * aClassPtr)
{
	Openmpt * cl = (Openmpt *)aClassPtr;
	return cl->getLoopPoint();
}

void Openmpt_setFilter(void * aClassPtr, unsigned int aFilterId, Filter * aFilter)
{
	Openmpt * cl = (Openmpt *)aClassPtr;
	cl->setFilter(aFilterId, aFilter);
}

void Openmpt_stop(void * aClassPtr)
{
	Openmpt * cl = (Openmpt *)aClassPtr;
	cl->stop();
}

void Queue_destroy(void * aClassPtr)
{
  delete (Queue *)aClassPtr;
}

void * Queue_create()
{
  return (void *)new Queue;
}

int Queue_play(void * aClassPtr, AudioSource * aSound)
{
	Queue * cl = (Queue *)aClassPtr;
	return cl->play(*aSound);
}

unsigned int Queue_getQueueCount(void * aClassPtr)
{
	Queue * cl = (Queue *)aClassPtr;
	return cl->getQueueCount();
}

int Queue_isCurrentlyPlaying(void * aClassPtr, AudioSource * aSound)
{
	Queue * cl = (Queue *)aClassPtr;
	return cl->isCurrentlyPlaying(*aSound);
}

int Queue_setParamsFromAudioSource(void * aClassPtr, AudioSource * aSound)
{
	Queue * cl = (Queue *)aClassPtr;
	return cl->setParamsFromAudioSource(*aSound);
}

int Queue_setParams(void * aClassPtr, float aSamplerate)
{
	Queue * cl = (Queue *)aClassPtr;
	return cl->setParams(aSamplerate);
}

int Queue_setParamsEx(void * aClassPtr, float aSamplerate, unsigned int aChannels)
{
	Queue * cl = (Queue *)aClassPtr;
	return cl->setParams(aSamplerate, aChannels);
}

void Queue_setVolume(void * aClassPtr, float aVolume)
{
	Queue * cl = (Queue *)aClassPtr;
	cl->setVolume(aVolume);
}

void Queue_setLooping(void * aClassPtr, int aLoop)
{
	Queue * cl = (Queue *)aClassPtr;
	cl->setLooping(!!aLoop);
}

void Queue_set3dMinMaxDistance(void * aClassPtr, float aMinDistance, float aMaxDistance)
{
	Queue * cl = (Queue *)aClassPtr;
	cl->set3dMinMaxDistance(aMinDistance, aMaxDistance);
}

void Queue_set3dAttenuation(void * aClassPtr, unsigned int aAttenuationModel, float aAttenuationRolloffFactor)
{
	Queue * cl = (Queue *)aClassPtr;
	cl->set3dAttenuation(aAttenuationModel, aAttenuationRolloffFactor);
}

void Queue_set3dDopplerFactor(void * aClassPtr, float aDopplerFactor)
{
	Queue * cl = (Queue *)aClassPtr;
	cl->set3dDopplerFactor(aDopplerFactor);
}

void Queue_set3dListenerRelative(void * aClassPtr, int aListenerRelative)
{
	Queue * cl = (Queue *)aClassPtr;
	cl->set3dListenerRelative(!!aListenerRelative);
}

void Queue_set3dDistanceDelay(void * aClassPtr, int aDistanceDelay)
{
	Queue * cl = (Queue *)aClassPtr;
	cl->set3dDistanceDelay(!!aDistanceDelay);
}

void Queue_set3dCollider(void * aClassPtr, AudioCollider * aCollider)
{
	Queue * cl = (Queue *)aClassPtr;
	cl->set3dCollider(aCollider);
}

void Queue_set3dColliderEx(void * aClassPtr, AudioCollider * aCollider, int aUserData)
{
	Queue * cl = (Queue *)aClassPtr;
	cl->set3dCollider(aCollider, aUserData);
}

void Queue_set3dAttenuator(void * aClassPtr, AudioAttenuator * aAttenuator)
{
	Queue * cl = (Queue *)aClassPtr;
	cl->set3dAttenuator(aAttenuator);
}

void Queue_setInaudibleBehavior(void * aClassPtr, int aMustTick, int aKill)
{
	Queue * cl = (Queue *)aClassPtr;
	cl->setInaudibleBehavior(!!aMustTick, !!aKill);
}

void Queue_setLoopPoint(void * aClassPtr, double aLoopPoint)
{
	Queue * cl = (Queue *)aClassPtr;
	cl->setLoopPoint(aLoopPoint);
}

double Queue_getLoopPoint(void * aClassPtr)
{
	Queue * cl = (Queue *)aClassPtr;
	return cl->getLoopPoint();
}

void Queue_setFilter(void * aClassPtr, unsigned int aFilterId, Filter * aFilter)
{
	Queue * cl = (Queue *)aClassPtr;
	cl->setFilter(aFilterId, aFilter);
}

void Queue_stop(void * aClassPtr)
{
	Queue * cl = (Queue *)aClassPtr;
	cl->stop();
}

void RobotizeFilter_destroy(void * aClassPtr)
{
  delete (RobotizeFilter *)aClassPtr;
}

int RobotizeFilter_getParamCount(void * aClassPtr)
{
	RobotizeFilter * cl = (RobotizeFilter *)aClassPtr;
	return cl->getParamCount();
}

const char * RobotizeFilter_getParamName(void * aClassPtr, unsigned int aParamIndex)
{
	RobotizeFilter * cl = (RobotizeFilter *)aClassPtr;
	return cl->getParamName(aParamIndex);
}

unsigned int RobotizeFilter_getParamType(void * aClassPtr, unsigned int aParamIndex)
{
	RobotizeFilter * cl = (RobotizeFilter *)aClassPtr;
	return cl->getParamType(aParamIndex);
}

float RobotizeFilter_getParamMax(void * aClassPtr, unsigned int aParamIndex)
{
	RobotizeFilter * cl = (RobotizeFilter *)aClassPtr;
	return cl->getParamMax(aParamIndex);
}

float RobotizeFilter_getParamMin(void * aClassPtr, unsigned int aParamIndex)
{
	RobotizeFilter * cl = (RobotizeFilter *)aClassPtr;
	return cl->getParamMin(aParamIndex);
}

void RobotizeFilter_setParams(void * aClassPtr, float aFreq, int aWaveform)
{
	RobotizeFilter * cl = (RobotizeFilter *)aClassPtr;
	cl->setParams(aFreq, aWaveform);
}

void * RobotizeFilter_create()
{
  return (void *)new RobotizeFilter;
}

void Sfxr_destroy(void * aClassPtr)
{
  delete (Sfxr *)aClassPtr;
}

void * Sfxr_create()
{
  return (void *)new Sfxr;
}

void Sfxr_resetParams(void * aClassPtr)
{
	Sfxr * cl = (Sfxr *)aClassPtr;
	cl->resetParams();
}

int Sfxr_loadParams(void * aClassPtr, const char * aFilename)
{
	Sfxr * cl = (Sfxr *)aClassPtr;
	return cl->loadParams(aFilename);
}

int Sfxr_loadParamsMem(void * aClassPtr, unsigned char * aMem, unsigned int aLength)
{
	Sfxr * cl = (Sfxr *)aClassPtr;
	return cl->loadParamsMem(aMem, aLength);
}

int Sfxr_loadParamsMemEx(void * aClassPtr, unsigned char * aMem, unsigned int aLength, int aCopy, int aTakeOwnership)
{
	Sfxr * cl = (Sfxr *)aClassPtr;
	return cl->loadParamsMem(aMem, aLength, !!aCopy, !!aTakeOwnership);
}

int Sfxr_loadParamsFile(void * aClassPtr, File * aFile)
{
	Sfxr * cl = (Sfxr *)aClassPtr;
	return cl->loadParamsFile(aFile);
}

int Sfxr_loadPreset(void * aClassPtr, int aPresetNo, int aRandSeed)
{
	Sfxr * cl = (Sfxr *)aClassPtr;
	return cl->loadPreset(aPresetNo, aRandSeed);
}

void Sfxr_setVolume(void * aClassPtr, float aVolume)
{
	Sfxr * cl = (Sfxr *)aClassPtr;
	cl->setVolume(aVolume);
}

void Sfxr_setLooping(void * aClassPtr, int aLoop)
{
	Sfxr * cl = (Sfxr *)aClassPtr;
	cl->setLooping(!!aLoop);
}

void Sfxr_set3dMinMaxDistance(void * aClassPtr, float aMinDistance, float aMaxDistance)
{
	Sfxr * cl = (Sfxr *)aClassPtr;
	cl->set3dMinMaxDistance(aMinDistance, aMaxDistance);
}

void Sfxr_set3dAttenuation(void * aClassPtr, unsigned int aAttenuationModel, float aAttenuationRolloffFactor)
{
	Sfxr * cl = (Sfxr *)aClassPtr;
	cl->set3dAttenuation(aAttenuationModel, aAttenuationRolloffFactor);
}

void Sfxr_set3dDopplerFactor(void * aClassPtr, float aDopplerFactor)
{
	Sfxr * cl = (Sfxr *)aClassPtr;
	cl->set3dDopplerFactor(aDopplerFactor);
}

void Sfxr_set3dListenerRelative(void * aClassPtr, int aListenerRelative)
{
	Sfxr * cl = (Sfxr *)aClassPtr;
	cl->set3dListenerRelative(!!aListenerRelative);
}

void Sfxr_set3dDistanceDelay(void * aClassPtr, int aDistanceDelay)
{
	Sfxr * cl = (Sfxr *)aClassPtr;
	cl->set3dDistanceDelay(!!aDistanceDelay);
}

void Sfxr_set3dCollider(void * aClassPtr, AudioCollider * aCollider)
{
	Sfxr * cl = (Sfxr *)aClassPtr;
	cl->set3dCollider(aCollider);
}

void Sfxr_set3dColliderEx(void * aClassPtr, AudioCollider * aCollider, int aUserData)
{
	Sfxr * cl = (Sfxr *)aClassPtr;
	cl->set3dCollider(aCollider, aUserData);
}

void Sfxr_set3dAttenuator(void * aClassPtr, AudioAttenuator * aAttenuator)
{
	Sfxr * cl = (Sfxr *)aClassPtr;
	cl->set3dAttenuator(aAttenuator);
}

void Sfxr_setInaudibleBehavior(void * aClassPtr, int aMustTick, int aKill)
{
	Sfxr * cl = (Sfxr *)aClassPtr;
	cl->setInaudibleBehavior(!!aMustTick, !!aKill);
}

void Sfxr_setLoopPoint(void * aClassPtr, double aLoopPoint)
{
	Sfxr * cl = (Sfxr *)aClassPtr;
	cl->setLoopPoint(aLoopPoint);
}

double Sfxr_getLoopPoint(void * aClassPtr)
{
	Sfxr * cl = (Sfxr *)aClassPtr;
	return cl->getLoopPoint();
}

void Sfxr_setFilter(void * aClassPtr, unsigned int aFilterId, Filter * aFilter)
{
	Sfxr * cl = (Sfxr *)aClassPtr;
	cl->setFilter(aFilterId, aFilter);
}

void Sfxr_stop(void * aClassPtr)
{
	Sfxr * cl = (Sfxr *)aClassPtr;
	cl->stop();
}

void Speech_destroy(void * aClassPtr)
{
  delete (Speech *)aClassPtr;
}

void * Speech_create()
{
  return (void *)new Speech;
}

int Speech_setText(void * aClassPtr, const char * aText)
{
	Speech * cl = (Speech *)aClassPtr;
	return cl->setText(aText);
}

int Speech_setParams(void * aClassPtr)
{
	Speech * cl = (Speech *)aClassPtr;
	return cl->setParams();
}

int Speech_setParamsEx(void * aClassPtr, unsigned int aBaseFrequency, float aBaseSpeed, float aBaseDeclination, int aBaseWaveform)
{
	Speech * cl = (Speech *)aClassPtr;
	return cl->setParams(aBaseFrequency, aBaseSpeed, aBaseDeclination, aBaseWaveform);
}

void Speech_setVolume(void * aClassPtr, float aVolume)
{
	Speech * cl = (Speech *)aClassPtr;
	cl->setVolume(aVolume);
}

void Speech_setLooping(void * aClassPtr, int aLoop)
{
	Speech * cl = (Speech *)aClassPtr;
	cl->setLooping(!!aLoop);
}

void Speech_set3dMinMaxDistance(void * aClassPtr, float aMinDistance, float aMaxDistance)
{
	Speech * cl = (Speech *)aClassPtr;
	cl->set3dMinMaxDistance(aMinDistance, aMaxDistance);
}

void Speech_set3dAttenuation(void * aClassPtr, unsigned int aAttenuationModel, float aAttenuationRolloffFactor)
{
	Speech * cl = (Speech *)aClassPtr;
	cl->set3dAttenuation(aAttenuationModel, aAttenuationRolloffFactor);
}

void Speech_set3dDopplerFactor(void * aClassPtr, float aDopplerFactor)
{
	Speech * cl = (Speech *)aClassPtr;
	cl->set3dDopplerFactor(aDopplerFactor);
}

void Speech_set3dListenerRelative(void * aClassPtr, int aListenerRelative)
{
	Speech * cl = (Speech *)aClassPtr;
	cl->set3dListenerRelative(!!aListenerRelative);
}

void Speech_set3dDistanceDelay(void * aClassPtr, int aDistanceDelay)
{
	Speech * cl = (Speech *)aClassPtr;
	cl->set3dDistanceDelay(!!aDistanceDelay);
}

void Speech_set3dCollider(void * aClassPtr, AudioCollider * aCollider)
{
	Speech * cl = (Speech *)aClassPtr;
	cl->set3dCollider(aCollider);
}

void Speech_set3dColliderEx(void * aClassPtr, AudioCollider * aCollider, int aUserData)
{
	Speech * cl = (Speech *)aClassPtr;
	cl->set3dCollider(aCollider, aUserData);
}

void Speech_set3dAttenuator(void * aClassPtr, AudioAttenuator * aAttenuator)
{
	Speech * cl = (Speech *)aClassPtr;
	cl->set3dAttenuator(aAttenuator);
}

void Speech_setInaudibleBehavior(void * aClassPtr, int aMustTick, int aKill)
{
	Speech * cl = (Speech *)aClassPtr;
	cl->setInaudibleBehavior(!!aMustTick, !!aKill);
}

void Speech_setLoopPoint(void * aClassPtr, double aLoopPoint)
{
	Speech * cl = (Speech *)aClassPtr;
	cl->setLoopPoint(aLoopPoint);
}

double Speech_getLoopPoint(void * aClassPtr)
{
	Speech * cl = (Speech *)aClassPtr;
	return cl->getLoopPoint();
}

void Speech_setFilter(void * aClassPtr, unsigned int aFilterId, Filter * aFilter)
{
	Speech * cl = (Speech *)aClassPtr;
	cl->setFilter(aFilterId, aFilter);
}

void Speech_stop(void * aClassPtr)
{
	Speech * cl = (Speech *)aClassPtr;
	cl->stop();
}

void TedSid_destroy(void * aClassPtr)
{
  delete (TedSid *)aClassPtr;
}

void * TedSid_create()
{
  return (void *)new TedSid;
}

int TedSid_load(void * aClassPtr, const char * aFilename)
{
	TedSid * cl = (TedSid *)aClassPtr;
	return cl->load(aFilename);
}

int TedSid_loadToMem(void * aClassPtr, const char * aFilename)
{
	TedSid * cl = (TedSid *)aClassPtr;
	return cl->loadToMem(aFilename);
}

int TedSid_loadMem(void * aClassPtr, const unsigned char * aMem, unsigned int aLength)
{
	TedSid * cl = (TedSid *)aClassPtr;
	return cl->loadMem(aMem, aLength);
}

int TedSid_loadMemEx(void * aClassPtr, const unsigned char * aMem, unsigned int aLength, int aCopy, int aTakeOwnership)
{
	TedSid * cl = (TedSid *)aClassPtr;
	return cl->loadMem(aMem, aLength, !!aCopy, !!aTakeOwnership);
}

int TedSid_loadFileToMem(void * aClassPtr, File * aFile)
{
	TedSid * cl = (TedSid *)aClassPtr;
	return cl->loadFileToMem(aFile);
}

int TedSid_loadFile(void * aClassPtr, File * aFile)
{
	TedSid * cl = (TedSid *)aClassPtr;
	return cl->loadFile(aFile);
}

void TedSid_setVolume(void * aClassPtr, float aVolume)
{
	TedSid * cl = (TedSid *)aClassPtr;
	cl->setVolume(aVolume);
}

void TedSid_setLooping(void * aClassPtr, int aLoop)
{
	TedSid * cl = (TedSid *)aClassPtr;
	cl->setLooping(!!aLoop);
}

void TedSid_set3dMinMaxDistance(void * aClassPtr, float aMinDistance, float aMaxDistance)
{
	TedSid * cl = (TedSid *)aClassPtr;
	cl->set3dMinMaxDistance(aMinDistance, aMaxDistance);
}

void TedSid_set3dAttenuation(void * aClassPtr, unsigned int aAttenuationModel, float aAttenuationRolloffFactor)
{
	TedSid * cl = (TedSid *)aClassPtr;
	cl->set3dAttenuation(aAttenuationModel, aAttenuationRolloffFactor);
}

void TedSid_set3dDopplerFactor(void * aClassPtr, float aDopplerFactor)
{
	TedSid * cl = (TedSid *)aClassPtr;
	cl->set3dDopplerFactor(aDopplerFactor);
}

void TedSid_set3dListenerRelative(void * aClassPtr, int aListenerRelative)
{
	TedSid * cl = (TedSid *)aClassPtr;
	cl->set3dListenerRelative(!!aListenerRelative);
}

void TedSid_set3dDistanceDelay(void * aClassPtr, int aDistanceDelay)
{
	TedSid * cl = (TedSid *)aClassPtr;
	cl->set3dDistanceDelay(!!aDistanceDelay);
}

void TedSid_set3dCollider(void * aClassPtr, AudioCollider * aCollider)
{
	TedSid * cl = (TedSid *)aClassPtr;
	cl->set3dCollider(aCollider);
}

void TedSid_set3dColliderEx(void * aClassPtr, AudioCollider * aCollider, int aUserData)
{
	TedSid * cl = (TedSid *)aClassPtr;
	cl->set3dCollider(aCollider, aUserData);
}

void TedSid_set3dAttenuator(void * aClassPtr, AudioAttenuator * aAttenuator)
{
	TedSid * cl = (TedSid *)aClassPtr;
	cl->set3dAttenuator(aAttenuator);
}

void TedSid_setInaudibleBehavior(void * aClassPtr, int aMustTick, int aKill)
{
	TedSid * cl = (TedSid *)aClassPtr;
	cl->setInaudibleBehavior(!!aMustTick, !!aKill);
}

void TedSid_setLoopPoint(void * aClassPtr, double aLoopPoint)
{
	TedSid * cl = (TedSid *)aClassPtr;
	cl->setLoopPoint(aLoopPoint);
}

double TedSid_getLoopPoint(void * aClassPtr)
{
	TedSid * cl = (TedSid *)aClassPtr;
	return cl->getLoopPoint();
}

void TedSid_setFilter(void * aClassPtr, unsigned int aFilterId, Filter * aFilter)
{
	TedSid * cl = (TedSid *)aClassPtr;
	cl->setFilter(aFilterId, aFilter);
}

void TedSid_stop(void * aClassPtr)
{
	TedSid * cl = (TedSid *)aClassPtr;
	cl->stop();
}

void Vic_destroy(void * aClassPtr)
{
  delete (Vic *)aClassPtr;
}

void * Vic_create()
{
  return (void *)new Vic;
}

void Vic_setModel(void * aClassPtr, int model)
{
	Vic * cl = (Vic *)aClassPtr;
	cl->setModel(model);
}

int Vic_getModel(void * aClassPtr)
{
	Vic * cl = (Vic *)aClassPtr;
	return cl->getModel();
}

void Vic_setRegister(void * aClassPtr, int reg, unsigned char value)
{
	Vic * cl = (Vic *)aClassPtr;
	cl->setRegister(reg, value);
}

unsigned char Vic_getRegister(void * aClassPtr, int reg)
{
	Vic * cl = (Vic *)aClassPtr;
	return cl->getRegister(reg);
}

void Vic_setVolume(void * aClassPtr, float aVolume)
{
	Vic * cl = (Vic *)aClassPtr;
	cl->setVolume(aVolume);
}

void Vic_setLooping(void * aClassPtr, int aLoop)
{
	Vic * cl = (Vic *)aClassPtr;
	cl->setLooping(!!aLoop);
}

void Vic_set3dMinMaxDistance(void * aClassPtr, float aMinDistance, float aMaxDistance)
{
	Vic * cl = (Vic *)aClassPtr;
	cl->set3dMinMaxDistance(aMinDistance, aMaxDistance);
}

void Vic_set3dAttenuation(void * aClassPtr, unsigned int aAttenuationModel, float aAttenuationRolloffFactor)
{
	Vic * cl = (Vic *)aClassPtr;
	cl->set3dAttenuation(aAttenuationModel, aAttenuationRolloffFactor);
}

void Vic_set3dDopplerFactor(void * aClassPtr, float aDopplerFactor)
{
	Vic * cl = (Vic *)aClassPtr;
	cl->set3dDopplerFactor(aDopplerFactor);
}

void Vic_set3dListenerRelative(void * aClassPtr, int aListenerRelative)
{
	Vic * cl = (Vic *)aClassPtr;
	cl->set3dListenerRelative(!!aListenerRelative);
}

void Vic_set3dDistanceDelay(void * aClassPtr, int aDistanceDelay)
{
	Vic * cl = (Vic *)aClassPtr;
	cl->set3dDistanceDelay(!!aDistanceDelay);
}

void Vic_set3dCollider(void * aClassPtr, AudioCollider * aCollider)
{
	Vic * cl = (Vic *)aClassPtr;
	cl->set3dCollider(aCollider);
}

void Vic_set3dColliderEx(void * aClassPtr, AudioCollider * aCollider, int aUserData)
{
	Vic * cl = (Vic *)aClassPtr;
	cl->set3dCollider(aCollider, aUserData);
}

void Vic_set3dAttenuator(void * aClassPtr, AudioAttenuator * aAttenuator)
{
	Vic * cl = (Vic *)aClassPtr;
	cl->set3dAttenuator(aAttenuator);
}

void Vic_setInaudibleBehavior(void * aClassPtr, int aMustTick, int aKill)
{
	Vic * cl = (Vic *)aClassPtr;
	cl->setInaudibleBehavior(!!aMustTick, !!aKill);
}

void Vic_setLoopPoint(void * aClassPtr, double aLoopPoint)
{
	Vic * cl = (Vic *)aClassPtr;
	cl->setLoopPoint(aLoopPoint);
}

double Vic_getLoopPoint(void * aClassPtr)
{
	Vic * cl = (Vic *)aClassPtr;
	return cl->getLoopPoint();
}

void Vic_setFilter(void * aClassPtr, unsigned int aFilterId, Filter * aFilter)
{
	Vic * cl = (Vic *)aClassPtr;
	cl->setFilter(aFilterId, aFilter);
}

void Vic_stop(void * aClassPtr)
{
	Vic * cl = (Vic *)aClassPtr;
	cl->stop();
}

void Vizsn_destroy(void * aClassPtr)
{
  delete (Vizsn *)aClassPtr;
}

void * Vizsn_create()
{
  return (void *)new Vizsn;
}

void Vizsn_setText(void * aClassPtr, char * aText)
{
	Vizsn * cl = (Vizsn *)aClassPtr;
	cl->setText(aText);
}

void Vizsn_setVolume(void * aClassPtr, float aVolume)
{
	Vizsn * cl = (Vizsn *)aClassPtr;
	cl->setVolume(aVolume);
}

void Vizsn_setLooping(void * aClassPtr, int aLoop)
{
	Vizsn * cl = (Vizsn *)aClassPtr;
	cl->setLooping(!!aLoop);
}

void Vizsn_set3dMinMaxDistance(void * aClassPtr, float aMinDistance, float aMaxDistance)
{
	Vizsn * cl = (Vizsn *)aClassPtr;
	cl->set3dMinMaxDistance(aMinDistance, aMaxDistance);
}

void Vizsn_set3dAttenuation(void * aClassPtr, unsigned int aAttenuationModel, float aAttenuationRolloffFactor)
{
	Vizsn * cl = (Vizsn *)aClassPtr;
	cl->set3dAttenuation(aAttenuationModel, aAttenuationRolloffFactor);
}

void Vizsn_set3dDopplerFactor(void * aClassPtr, float aDopplerFactor)
{
	Vizsn * cl = (Vizsn *)aClassPtr;
	cl->set3dDopplerFactor(aDopplerFactor);
}

void Vizsn_set3dListenerRelative(void * aClassPtr, int aListenerRelative)
{
	Vizsn * cl = (Vizsn *)aClassPtr;
	cl->set3dListenerRelative(!!aListenerRelative);
}

void Vizsn_set3dDistanceDelay(void * aClassPtr, int aDistanceDelay)
{
	Vizsn * cl = (Vizsn *)aClassPtr;
	cl->set3dDistanceDelay(!!aDistanceDelay);
}

void Vizsn_set3dCollider(void * aClassPtr, AudioCollider * aCollider)
{
	Vizsn * cl = (Vizsn *)aClassPtr;
	cl->set3dCollider(aCollider);
}

void Vizsn_set3dColliderEx(void * aClassPtr, AudioCollider * aCollider, int aUserData)
{
	Vizsn * cl = (Vizsn *)aClassPtr;
	cl->set3dCollider(aCollider, aUserData);
}

void Vizsn_set3dAttenuator(void * aClassPtr, AudioAttenuator * aAttenuator)
{
	Vizsn * cl = (Vizsn *)aClassPtr;
	cl->set3dAttenuator(aAttenuator);
}

void Vizsn_setInaudibleBehavior(void * aClassPtr, int aMustTick, int aKill)
{
	Vizsn * cl = (Vizsn *)aClassPtr;
	cl->setInaudibleBehavior(!!aMustTick, !!aKill);
}

void Vizsn_setLoopPoint(void * aClassPtr, double aLoopPoint)
{
	Vizsn * cl = (Vizsn *)aClassPtr;
	cl->setLoopPoint(aLoopPoint);
}

double Vizsn_getLoopPoint(void * aClassPtr)
{
	Vizsn * cl = (Vizsn *)aClassPtr;
	return cl->getLoopPoint();
}

void Vizsn_setFilter(void * aClassPtr, unsigned int aFilterId, Filter * aFilter)
{
	Vizsn * cl = (Vizsn *)aClassPtr;
	cl->setFilter(aFilterId, aFilter);
}

void Vizsn_stop(void * aClassPtr)
{
	Vizsn * cl = (Vizsn *)aClassPtr;
	cl->stop();
}

void Wav_destroy(void * aClassPtr)
{
  delete (Wav *)aClassPtr;
}

void * Wav_create()
{
  return (void *)new Wav;
}

int Wav_load(void * aClassPtr, const char * aFilename)
{
	Wav * cl = (Wav *)aClassPtr;
	return cl->load(aFilename);
}

int Wav_loadMem(void * aClassPtr, const unsigned char * aMem, unsigned int aLength)
{
	Wav * cl = (Wav *)aClassPtr;
	return cl->loadMem(aMem, aLength);
}

int Wav_loadMemEx(void * aClassPtr, const unsigned char * aMem, unsigned int aLength, int aCopy, int aTakeOwnership)
{
	Wav * cl = (Wav *)aClassPtr;
	return cl->loadMem(aMem, aLength, !!aCopy, !!aTakeOwnership);
}

int Wav_loadFile(void * aClassPtr, File * aFile)
{
	Wav * cl = (Wav *)aClassPtr;
	return cl->loadFile(aFile);
}

int Wav_loadRawWave8(void * aClassPtr, unsigned char * aMem, unsigned int aLength)
{
	Wav * cl = (Wav *)aClassPtr;
	return cl->loadRawWave8(aMem, aLength);
}

int Wav_loadRawWave8Ex(void * aClassPtr, unsigned char * aMem, unsigned int aLength, float aSamplerate, unsigned int aChannels)
{
	Wav * cl = (Wav *)aClassPtr;
	return cl->loadRawWave8(aMem, aLength, aSamplerate, aChannels);
}

int Wav_loadRawWave16(void * aClassPtr, short * aMem, unsigned int aLength)
{
	Wav * cl = (Wav *)aClassPtr;
	return cl->loadRawWave16(aMem, aLength);
}

int Wav_loadRawWave16Ex(void * aClassPtr, short * aMem, unsigned int aLength, float aSamplerate, unsigned int aChannels)
{
	Wav * cl = (Wav *)aClassPtr;
	return cl->loadRawWave16(aMem, aLength, aSamplerate, aChannels);
}

int Wav_loadRawWave(void * aClassPtr, float * aMem, unsigned int aLength)
{
	Wav * cl = (Wav *)aClassPtr;
	return cl->loadRawWave(aMem, aLength);
}

int Wav_loadRawWaveEx(void * aClassPtr, float * aMem, unsigned int aLength, float aSamplerate, unsigned int aChannels, int aCopy, int aTakeOwnership)
{
	Wav * cl = (Wav *)aClassPtr;
	return cl->loadRawWave(aMem, aLength, aSamplerate, aChannels, !!aCopy, !!aTakeOwnership);
}

double Wav_getLength(void * aClassPtr)
{
	Wav * cl = (Wav *)aClassPtr;
	return cl->getLength();
}

void Wav_setVolume(void * aClassPtr, float aVolume)
{
	Wav * cl = (Wav *)aClassPtr;
	cl->setVolume(aVolume);
}

void Wav_setLooping(void * aClassPtr, int aLoop)
{
	Wav * cl = (Wav *)aClassPtr;
	cl->setLooping(!!aLoop);
}

void Wav_set3dMinMaxDistance(void * aClassPtr, float aMinDistance, float aMaxDistance)
{
	Wav * cl = (Wav *)aClassPtr;
	cl->set3dMinMaxDistance(aMinDistance, aMaxDistance);
}

void Wav_set3dAttenuation(void * aClassPtr, unsigned int aAttenuationModel, float aAttenuationRolloffFactor)
{
	Wav * cl = (Wav *)aClassPtr;
	cl->set3dAttenuation(aAttenuationModel, aAttenuationRolloffFactor);
}

void Wav_set3dDopplerFactor(void * aClassPtr, float aDopplerFactor)
{
	Wav * cl = (Wav *)aClassPtr;
	cl->set3dDopplerFactor(aDopplerFactor);
}

void Wav_set3dListenerRelative(void * aClassPtr, int aListenerRelative)
{
	Wav * cl = (Wav *)aClassPtr;
	cl->set3dListenerRelative(!!aListenerRelative);
}

void Wav_set3dDistanceDelay(void * aClassPtr, int aDistanceDelay)
{
	Wav * cl = (Wav *)aClassPtr;
	cl->set3dDistanceDelay(!!aDistanceDelay);
}

void Wav_set3dCollider(void * aClassPtr, AudioCollider * aCollider)
{
	Wav * cl = (Wav *)aClassPtr;
	cl->set3dCollider(aCollider);
}

void Wav_set3dColliderEx(void * aClassPtr, AudioCollider * aCollider, int aUserData)
{
	Wav * cl = (Wav *)aClassPtr;
	cl->set3dCollider(aCollider, aUserData);
}

void Wav_set3dAttenuator(void * aClassPtr, AudioAttenuator * aAttenuator)
{
	Wav * cl = (Wav *)aClassPtr;
	cl->set3dAttenuator(aAttenuator);
}

void Wav_setInaudibleBehavior(void * aClassPtr, int aMustTick, int aKill)
{
	Wav * cl = (Wav *)aClassPtr;
	cl->setInaudibleBehavior(!!aMustTick, !!aKill);
}

void Wav_setLoopPoint(void * aClassPtr, double aLoopPoint)
{
	Wav * cl = (Wav *)aClassPtr;
	cl->setLoopPoint(aLoopPoint);
}

double Wav_getLoopPoint(void * aClassPtr)
{
	Wav * cl = (Wav *)aClassPtr;
	return cl->getLoopPoint();
}

void Wav_setFilter(void * aClassPtr, unsigned int aFilterId, Filter * aFilter)
{
	Wav * cl = (Wav *)aClassPtr;
	cl->setFilter(aFilterId, aFilter);
}

void Wav_stop(void * aClassPtr)
{
	Wav * cl = (Wav *)aClassPtr;
	cl->stop();
}

void WaveShaperFilter_destroy(void * aClassPtr)
{
  delete (WaveShaperFilter *)aClassPtr;
}

int WaveShaperFilter_setParams(void * aClassPtr, float aAmount)
{
	WaveShaperFilter * cl = (WaveShaperFilter *)aClassPtr;
	return cl->setParams(aAmount);
}

void * WaveShaperFilter_create()
{
  return (void *)new WaveShaperFilter;
}

int WaveShaperFilter_getParamCount(void * aClassPtr)
{
	WaveShaperFilter * cl = (WaveShaperFilter *)aClassPtr;
	return cl->getParamCount();
}

const char * WaveShaperFilter_getParamName(void * aClassPtr, unsigned int aParamIndex)
{
	WaveShaperFilter * cl = (WaveShaperFilter *)aClassPtr;
	return cl->getParamName(aParamIndex);
}

unsigned int WaveShaperFilter_getParamType(void * aClassPtr, unsigned int aParamIndex)
{
	WaveShaperFilter * cl = (WaveShaperFilter *)aClassPtr;
	return cl->getParamType(aParamIndex);
}

float WaveShaperFilter_getParamMax(void * aClassPtr, unsigned int aParamIndex)
{
	WaveShaperFilter * cl = (WaveShaperFilter *)aClassPtr;
	return cl->getParamMax(aParamIndex);
}

float WaveShaperFilter_getParamMin(void * aClassPtr, unsigned int aParamIndex)
{
	WaveShaperFilter * cl = (WaveShaperFilter *)aClassPtr;
	return cl->getParamMin(aParamIndex);
}

void WavStream_destroy(void * aClassPtr)
{
  delete (WavStream *)aClassPtr;
}

void * WavStream_create()
{
  return (void *)new WavStream;
}

int WavStream_load(void * aClassPtr, const char * aFilename)
{
	WavStream * cl = (WavStream *)aClassPtr;
	return cl->load(aFilename);
}

int WavStream_loadMem(void * aClassPtr, const unsigned char * aData, unsigned int aDataLen)
{
	WavStream * cl = (WavStream *)aClassPtr;
	return cl->loadMem(aData, aDataLen);
}

int WavStream_loadMemEx(void * aClassPtr, const unsigned char * aData, unsigned int aDataLen, int aCopy, int aTakeOwnership)
{
	WavStream * cl = (WavStream *)aClassPtr;
	return cl->loadMem(aData, aDataLen, !!aCopy, !!aTakeOwnership);
}

int WavStream_loadToMem(void * aClassPtr, const char * aFilename)
{
	WavStream * cl = (WavStream *)aClassPtr;
	return cl->loadToMem(aFilename);
}

int WavStream_loadFile(void * aClassPtr, File * aFile)
{
	WavStream * cl = (WavStream *)aClassPtr;
	return cl->loadFile(aFile);
}

int WavStream_loadFileToMem(void * aClassPtr, File * aFile)
{
	WavStream * cl = (WavStream *)aClassPtr;
	return cl->loadFileToMem(aFile);
}

double WavStream_getLength(void * aClassPtr)
{
	WavStream * cl = (WavStream *)aClassPtr;
	return cl->getLength();
}

void WavStream_setVolume(void * aClassPtr, float aVolume)
{
	WavStream * cl = (WavStream *)aClassPtr;
	cl->setVolume(aVolume);
}

void WavStream_setLooping(void * aClassPtr, int aLoop)
{
	WavStream * cl = (WavStream *)aClassPtr;
	cl->setLooping(!!aLoop);
}

void WavStream_set3dMinMaxDistance(void * aClassPtr, float aMinDistance, float aMaxDistance)
{
	WavStream * cl = (WavStream *)aClassPtr;
	cl->set3dMinMaxDistance(aMinDistance, aMaxDistance);
}

void WavStream_set3dAttenuation(void * aClassPtr, unsigned int aAttenuationModel, float aAttenuationRolloffFactor)
{
	WavStream * cl = (WavStream *)aClassPtr;
	cl->set3dAttenuation(aAttenuationModel, aAttenuationRolloffFactor);
}

void WavStream_set3dDopplerFactor(void * aClassPtr, float aDopplerFactor)
{
	WavStream * cl = (WavStream *)aClassPtr;
	cl->set3dDopplerFactor(aDopplerFactor);
}

void WavStream_set3dListenerRelative(void * aClassPtr, int aListenerRelative)
{
	WavStream * cl = (WavStream *)aClassPtr;
	cl->set3dListenerRelative(!!aListenerRelative);
}

void WavStream_set3dDistanceDelay(void * aClassPtr, int aDistanceDelay)
{
	WavStream * cl = (WavStream *)aClassPtr;
	cl->set3dDistanceDelay(!!aDistanceDelay);
}

void WavStream_set3dCollider(void * aClassPtr, AudioCollider * aCollider)
{
	WavStream * cl = (WavStream *)aClassPtr;
	cl->set3dCollider(aCollider);
}

void WavStream_set3dColliderEx(void * aClassPtr, AudioCollider * aCollider, int aUserData)
{
	WavStream * cl = (WavStream *)aClassPtr;
	cl->set3dCollider(aCollider, aUserData);
}

void WavStream_set3dAttenuator(void * aClassPtr, AudioAttenuator * aAttenuator)
{
	WavStream * cl = (WavStream *)aClassPtr;
	cl->set3dAttenuator(aAttenuator);
}

void WavStream_setInaudibleBehavior(void * aClassPtr, int aMustTick, int aKill)
{
	WavStream * cl = (WavStream *)aClassPtr;
	cl->setInaudibleBehavior(!!aMustTick, !!aKill);
}

void WavStream_setLoopPoint(void * aClassPtr, double aLoopPoint)
{
	WavStream * cl = (WavStream *)aClassPtr;
	cl->setLoopPoint(aLoopPoint);
}

double WavStream_getLoopPoint(void * aClassPtr)
{
	WavStream * cl = (WavStream *)aClassPtr;
	return cl->getLoopPoint();
}

void WavStream_setFilter(void * aClassPtr, unsigned int aFilterId, Filter * aFilter)
{
	WavStream * cl = (WavStream *)aClassPtr;
	cl->setFilter(aFilterId, aFilter);
}

void WavStream_stop(void * aClassPtr)
{
	WavStream * cl = (WavStream *)aClassPtr;
	cl->stop();
}

} // extern "C"

