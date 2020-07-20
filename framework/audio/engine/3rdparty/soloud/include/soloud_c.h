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

#ifndef SOLOUD_C_H_INCLUDED
#define SOLOUD_C_H_INCLUDED

#ifdef  __cplusplus
extern "C" {
#endif
// Collected enumerations
enum SOLOUD_ENUMS
{
	SOLOUD_AUTO = 0,
	SOLOUD_SDL1 = 1,
	SOLOUD_SDL2 = 2,
	SOLOUD_PORTAUDIO = 3,
	SOLOUD_WINMM = 4,
	SOLOUD_XAUDIO2 = 5,
	SOLOUD_WASAPI = 6,
	SOLOUD_ALSA = 7,
	SOLOUD_JACK = 8,
	SOLOUD_OSS = 9,
	SOLOUD_OPENAL = 10,
	SOLOUD_COREAUDIO = 11,
	SOLOUD_OPENSLES = 12,
	SOLOUD_VITA_HOMEBREW = 13,
	SOLOUD_MINIAUDIO = 14,
	SOLOUD_NOSOUND = 15,
	SOLOUD_NULLDRIVER = 16,
	SOLOUD_BACKEND_MAX = 17,
	SOLOUD_CLIP_ROUNDOFF = 1,
	SOLOUD_ENABLE_VISUALIZATION = 2,
	SOLOUD_LEFT_HANDED_3D = 4,
	SOLOUD_NO_FPU_REGISTER_CHANGE = 8,
	BASSBOOSTFILTER_WET = 0,
	BASSBOOSTFILTER_BOOST = 1,
	BIQUADRESONANTFILTER_LOWPASS = 0,
	BIQUADRESONANTFILTER_HIGHPASS = 1,
	BIQUADRESONANTFILTER_BANDPASS = 2,
	BIQUADRESONANTFILTER_WET = 0,
	BIQUADRESONANTFILTER_TYPE = 1,
	BIQUADRESONANTFILTER_FREQUENCY = 2,
	BIQUADRESONANTFILTER_RESONANCE = 3,
	ECHOFILTER_WET = 0,
	ECHOFILTER_DELAY = 1,
	ECHOFILTER_DECAY = 2,
	ECHOFILTER_FILTER = 3,
	FLANGERFILTER_WET = 0,
	FLANGERFILTER_DELAY = 1,
	FLANGERFILTER_FREQ = 2,
	FREEVERBFILTER_WET = 0,
	FREEVERBFILTER_FREEZE = 1,
	FREEVERBFILTER_ROOMSIZE = 2,
	FREEVERBFILTER_DAMP = 3,
	FREEVERBFILTER_WIDTH = 4,
	LOFIFILTER_WET = 0,
	LOFIFILTER_SAMPLERATE = 1,
	LOFIFILTER_BITDEPTH = 2,
	NOISE_WHITE = 0,
	NOISE_PINK = 1,
	NOISE_BROWNISH = 2,
	NOISE_BLUEISH = 3,
	ROBOTIZEFILTER_WET = 0,
	ROBOTIZEFILTER_FREQ = 1,
	ROBOTIZEFILTER_WAVE = 2,
	SFXR_COIN = 0,
	SFXR_LASER = 1,
	SFXR_EXPLOSION = 2,
	SFXR_POWERUP = 3,
	SFXR_HURT = 4,
	SFXR_JUMP = 5,
	SFXR_BLIP = 6,
	SPEECH_KW_SAW = 0,
	SPEECH_KW_TRIANGLE = 1,
	SPEECH_KW_SIN = 2,
	SPEECH_KW_SQUARE = 3,
	SPEECH_KW_PULSE = 4,
	SPEECH_KW_NOISE = 5,
	SPEECH_KW_WARBLE = 6,
	VIC_PAL = 0,
	VIC_NTSC = 1,
	VIC_BASS = 0,
	VIC_ALTO = 1,
	VIC_SOPRANO = 2,
	VIC_NOISE = 3,
	VIC_MAX_REGS = 4,
	WAVESHAPERFILTER_WET = 0,
	WAVESHAPERFILTER_AMOUNT = 1
};

// Object handle typedefs
typedef void * AlignedFloatBuffer;
typedef void * TinyAlignedFloatBuffer;
typedef void * Soloud;
typedef void * AudioCollider;
typedef void * AudioAttenuator;
typedef void * AudioSource;
typedef void * BassboostFilter;
typedef void * BiquadResonantFilter;
typedef void * Bus;
typedef void * DCRemovalFilter;
typedef void * EchoFilter;
typedef void * Fader;
typedef void * FFTFilter;
typedef void * Filter;
typedef void * FlangerFilter;
typedef void * FreeverbFilter;
typedef void * LofiFilter;
typedef void * Monotone;
typedef void * Noise;
typedef void * Openmpt;
typedef void * Queue;
typedef void * RobotizeFilter;
typedef void * Sfxr;
typedef void * Speech;
typedef void * TedSid;
typedef void * Vic;
typedef void * Vizsn;
typedef void * Wav;
typedef void * WaveShaperFilter;
typedef void * WavStream;
typedef void * File;

/*
 * Soloud
 */
void Soloud_destroy(Soloud * aSoloud);
Soloud * Soloud_create();
int Soloud_init(Soloud * aSoloud);
int Soloud_initEx(Soloud * aSoloud, unsigned int aFlags /* = Soloud::CLIP_ROUNDOFF */, unsigned int aBackend /* = Soloud::AUTO */, unsigned int aSamplerate /* = Soloud::AUTO */, unsigned int aBufferSize /* = Soloud::AUTO */, unsigned int aChannels /* = 2 */);
void Soloud_deinit(Soloud * aSoloud);
unsigned int Soloud_getVersion(Soloud * aSoloud);
const char * Soloud_getErrorString(Soloud * aSoloud, int aErrorCode);
unsigned int Soloud_getBackendId(Soloud * aSoloud);
const char * Soloud_getBackendString(Soloud * aSoloud);
unsigned int Soloud_getBackendChannels(Soloud * aSoloud);
unsigned int Soloud_getBackendSamplerate(Soloud * aSoloud);
unsigned int Soloud_getBackendBufferSize(Soloud * aSoloud);
int Soloud_setSpeakerPosition(Soloud * aSoloud, unsigned int aChannel, float aX, float aY, float aZ);
int Soloud_getSpeakerPosition(Soloud * aSoloud, unsigned int aChannel, float * aX, float * aY, float * aZ);
unsigned int Soloud_play(Soloud * aSoloud, AudioSource * aSound);
unsigned int Soloud_playEx(Soloud * aSoloud, AudioSource * aSound, float aVolume /* = -1.0f */, float aPan /* = 0.0f */, int aPaused /* = 0 */, unsigned int aBus /* = 0 */);
unsigned int Soloud_playClocked(Soloud * aSoloud, double aSoundTime, AudioSource * aSound);
unsigned int Soloud_playClockedEx(Soloud * aSoloud, double aSoundTime, AudioSource * aSound, float aVolume /* = -1.0f */, float aPan /* = 0.0f */, unsigned int aBus /* = 0 */);
unsigned int Soloud_play3d(Soloud * aSoloud, AudioSource * aSound, float aPosX, float aPosY, float aPosZ);
unsigned int Soloud_play3dEx(Soloud * aSoloud, AudioSource * aSound, float aPosX, float aPosY, float aPosZ, float aVelX /* = 0.0f */, float aVelY /* = 0.0f */, float aVelZ /* = 0.0f */, float aVolume /* = 1.0f */, int aPaused /* = 0 */, unsigned int aBus /* = 0 */);
unsigned int Soloud_play3dClocked(Soloud * aSoloud, double aSoundTime, AudioSource * aSound, float aPosX, float aPosY, float aPosZ);
unsigned int Soloud_play3dClockedEx(Soloud * aSoloud, double aSoundTime, AudioSource * aSound, float aPosX, float aPosY, float aPosZ, float aVelX /* = 0.0f */, float aVelY /* = 0.0f */, float aVelZ /* = 0.0f */, float aVolume /* = 1.0f */, unsigned int aBus /* = 0 */);
unsigned int Soloud_playBackground(Soloud * aSoloud, AudioSource * aSound);
unsigned int Soloud_playBackgroundEx(Soloud * aSoloud, AudioSource * aSound, float aVolume /* = -1.0f */, int aPaused /* = 0 */, unsigned int aBus /* = 0 */);
int Soloud_seek(Soloud * aSoloud, unsigned int aVoiceHandle, double aSeconds);
void Soloud_stop(Soloud * aSoloud, unsigned int aVoiceHandle);
void Soloud_stopAll(Soloud * aSoloud);
void Soloud_stopAudioSource(Soloud * aSoloud, AudioSource * aSound);
int Soloud_countAudioSource(Soloud * aSoloud, AudioSource * aSound);
void Soloud_setFilterParameter(Soloud * aSoloud, unsigned int aVoiceHandle, unsigned int aFilterId, unsigned int aAttributeId, float aValue);
float Soloud_getFilterParameter(Soloud * aSoloud, unsigned int aVoiceHandle, unsigned int aFilterId, unsigned int aAttributeId);
void Soloud_fadeFilterParameter(Soloud * aSoloud, unsigned int aVoiceHandle, unsigned int aFilterId, unsigned int aAttributeId, float aTo, double aTime);
void Soloud_oscillateFilterParameter(Soloud * aSoloud, unsigned int aVoiceHandle, unsigned int aFilterId, unsigned int aAttributeId, float aFrom, float aTo, double aTime);
double Soloud_getStreamTime(Soloud * aSoloud, unsigned int aVoiceHandle);
double Soloud_getStreamPosition(Soloud * aSoloud, unsigned int aVoiceHandle);
int Soloud_getPause(Soloud * aSoloud, unsigned int aVoiceHandle);
float Soloud_getVolume(Soloud * aSoloud, unsigned int aVoiceHandle);
float Soloud_getOverallVolume(Soloud * aSoloud, unsigned int aVoiceHandle);
float Soloud_getPan(Soloud * aSoloud, unsigned int aVoiceHandle);
float Soloud_getSamplerate(Soloud * aSoloud, unsigned int aVoiceHandle);
int Soloud_getProtectVoice(Soloud * aSoloud, unsigned int aVoiceHandle);
unsigned int Soloud_getActiveVoiceCount(Soloud * aSoloud);
unsigned int Soloud_getVoiceCount(Soloud * aSoloud);
int Soloud_isValidVoiceHandle(Soloud * aSoloud, unsigned int aVoiceHandle);
float Soloud_getRelativePlaySpeed(Soloud * aSoloud, unsigned int aVoiceHandle);
float Soloud_getPostClipScaler(Soloud * aSoloud);
float Soloud_getGlobalVolume(Soloud * aSoloud);
unsigned int Soloud_getMaxActiveVoiceCount(Soloud * aSoloud);
int Soloud_getLooping(Soloud * aSoloud, unsigned int aVoiceHandle);
double Soloud_getLoopPoint(Soloud * aSoloud, unsigned int aVoiceHandle);
void Soloud_setLoopPoint(Soloud * aSoloud, unsigned int aVoiceHandle, double aLoopPoint);
void Soloud_setLooping(Soloud * aSoloud, unsigned int aVoiceHandle, int aLooping);
int Soloud_setMaxActiveVoiceCount(Soloud * aSoloud, unsigned int aVoiceCount);
void Soloud_setInaudibleBehavior(Soloud * aSoloud, unsigned int aVoiceHandle, int aMustTick, int aKill);
void Soloud_setGlobalVolume(Soloud * aSoloud, float aVolume);
void Soloud_setPostClipScaler(Soloud * aSoloud, float aScaler);
void Soloud_setPause(Soloud * aSoloud, unsigned int aVoiceHandle, int aPause);
void Soloud_setPauseAll(Soloud * aSoloud, int aPause);
int Soloud_setRelativePlaySpeed(Soloud * aSoloud, unsigned int aVoiceHandle, float aSpeed);
void Soloud_setProtectVoice(Soloud * aSoloud, unsigned int aVoiceHandle, int aProtect);
void Soloud_setSamplerate(Soloud * aSoloud, unsigned int aVoiceHandle, float aSamplerate);
void Soloud_setPan(Soloud * aSoloud, unsigned int aVoiceHandle, float aPan);
void Soloud_setPanAbsolute(Soloud * aSoloud, unsigned int aVoiceHandle, float aLVolume, float aRVolume);
void Soloud_setPanAbsoluteEx(Soloud * aSoloud, unsigned int aVoiceHandle, float aLVolume, float aRVolume, float aLBVolume /* = 0 */, float aRBVolume /* = 0 */, float aCVolume /* = 0 */, float aSVolume /* = 0 */);
void Soloud_setVolume(Soloud * aSoloud, unsigned int aVoiceHandle, float aVolume);
void Soloud_setDelaySamples(Soloud * aSoloud, unsigned int aVoiceHandle, unsigned int aSamples);
void Soloud_fadeVolume(Soloud * aSoloud, unsigned int aVoiceHandle, float aTo, double aTime);
void Soloud_fadePan(Soloud * aSoloud, unsigned int aVoiceHandle, float aTo, double aTime);
void Soloud_fadeRelativePlaySpeed(Soloud * aSoloud, unsigned int aVoiceHandle, float aTo, double aTime);
void Soloud_fadeGlobalVolume(Soloud * aSoloud, float aTo, double aTime);
void Soloud_schedulePause(Soloud * aSoloud, unsigned int aVoiceHandle, double aTime);
void Soloud_scheduleStop(Soloud * aSoloud, unsigned int aVoiceHandle, double aTime);
void Soloud_oscillateVolume(Soloud * aSoloud, unsigned int aVoiceHandle, float aFrom, float aTo, double aTime);
void Soloud_oscillatePan(Soloud * aSoloud, unsigned int aVoiceHandle, float aFrom, float aTo, double aTime);
void Soloud_oscillateRelativePlaySpeed(Soloud * aSoloud, unsigned int aVoiceHandle, float aFrom, float aTo, double aTime);
void Soloud_oscillateGlobalVolume(Soloud * aSoloud, float aFrom, float aTo, double aTime);
void Soloud_setGlobalFilter(Soloud * aSoloud, unsigned int aFilterId, Filter * aFilter);
void Soloud_setVisualizationEnable(Soloud * aSoloud, int aEnable);
float * Soloud_calcFFT(Soloud * aSoloud);
float * Soloud_getWave(Soloud * aSoloud);
float Soloud_getApproximateVolume(Soloud * aSoloud, unsigned int aChannel);
unsigned int Soloud_getLoopCount(Soloud * aSoloud, unsigned int aVoiceHandle);
float Soloud_getInfo(Soloud * aSoloud, unsigned int aVoiceHandle, unsigned int aInfoKey);
unsigned int Soloud_createVoiceGroup(Soloud * aSoloud);
int Soloud_destroyVoiceGroup(Soloud * aSoloud, unsigned int aVoiceGroupHandle);
int Soloud_addVoiceToGroup(Soloud * aSoloud, unsigned int aVoiceGroupHandle, unsigned int aVoiceHandle);
int Soloud_isVoiceGroup(Soloud * aSoloud, unsigned int aVoiceGroupHandle);
int Soloud_isVoiceGroupEmpty(Soloud * aSoloud, unsigned int aVoiceGroupHandle);
void Soloud_update3dAudio(Soloud * aSoloud);
int Soloud_set3dSoundSpeed(Soloud * aSoloud, float aSpeed);
float Soloud_get3dSoundSpeed(Soloud * aSoloud);
void Soloud_set3dListenerParameters(Soloud * aSoloud, float aPosX, float aPosY, float aPosZ, float aAtX, float aAtY, float aAtZ, float aUpX, float aUpY, float aUpZ);
void Soloud_set3dListenerParametersEx(Soloud * aSoloud, float aPosX, float aPosY, float aPosZ, float aAtX, float aAtY, float aAtZ, float aUpX, float aUpY, float aUpZ, float aVelocityX /* = 0.0f */, float aVelocityY /* = 0.0f */, float aVelocityZ /* = 0.0f */);
void Soloud_set3dListenerPosition(Soloud * aSoloud, float aPosX, float aPosY, float aPosZ);
void Soloud_set3dListenerAt(Soloud * aSoloud, float aAtX, float aAtY, float aAtZ);
void Soloud_set3dListenerUp(Soloud * aSoloud, float aUpX, float aUpY, float aUpZ);
void Soloud_set3dListenerVelocity(Soloud * aSoloud, float aVelocityX, float aVelocityY, float aVelocityZ);
void Soloud_set3dSourceParameters(Soloud * aSoloud, unsigned int aVoiceHandle, float aPosX, float aPosY, float aPosZ);
void Soloud_set3dSourceParametersEx(Soloud * aSoloud, unsigned int aVoiceHandle, float aPosX, float aPosY, float aPosZ, float aVelocityX /* = 0.0f */, float aVelocityY /* = 0.0f */, float aVelocityZ /* = 0.0f */);
void Soloud_set3dSourcePosition(Soloud * aSoloud, unsigned int aVoiceHandle, float aPosX, float aPosY, float aPosZ);
void Soloud_set3dSourceVelocity(Soloud * aSoloud, unsigned int aVoiceHandle, float aVelocityX, float aVelocityY, float aVelocityZ);
void Soloud_set3dSourceMinMaxDistance(Soloud * aSoloud, unsigned int aVoiceHandle, float aMinDistance, float aMaxDistance);
void Soloud_set3dSourceAttenuation(Soloud * aSoloud, unsigned int aVoiceHandle, unsigned int aAttenuationModel, float aAttenuationRolloffFactor);
void Soloud_set3dSourceDopplerFactor(Soloud * aSoloud, unsigned int aVoiceHandle, float aDopplerFactor);
void Soloud_mix(Soloud * aSoloud, float * aBuffer, unsigned int aSamples);
void Soloud_mixSigned16(Soloud * aSoloud, short * aBuffer, unsigned int aSamples);

/*
 * BassboostFilter
 */
void BassboostFilter_destroy(BassboostFilter * aBassboostFilter);
int BassboostFilter_getParamCount(BassboostFilter * aBassboostFilter);
const char * BassboostFilter_getParamName(BassboostFilter * aBassboostFilter, unsigned int aParamIndex);
unsigned int BassboostFilter_getParamType(BassboostFilter * aBassboostFilter, unsigned int aParamIndex);
float BassboostFilter_getParamMax(BassboostFilter * aBassboostFilter, unsigned int aParamIndex);
float BassboostFilter_getParamMin(BassboostFilter * aBassboostFilter, unsigned int aParamIndex);
int BassboostFilter_setParams(BassboostFilter * aBassboostFilter, float aBoost);
BassboostFilter * BassboostFilter_create();

/*
 * BiquadResonantFilter
 */
void BiquadResonantFilter_destroy(BiquadResonantFilter * aBiquadResonantFilter);
int BiquadResonantFilter_getParamCount(BiquadResonantFilter * aBiquadResonantFilter);
const char * BiquadResonantFilter_getParamName(BiquadResonantFilter * aBiquadResonantFilter, unsigned int aParamIndex);
unsigned int BiquadResonantFilter_getParamType(BiquadResonantFilter * aBiquadResonantFilter, unsigned int aParamIndex);
float BiquadResonantFilter_getParamMax(BiquadResonantFilter * aBiquadResonantFilter, unsigned int aParamIndex);
float BiquadResonantFilter_getParamMin(BiquadResonantFilter * aBiquadResonantFilter, unsigned int aParamIndex);
BiquadResonantFilter * BiquadResonantFilter_create();
int BiquadResonantFilter_setParams(BiquadResonantFilter * aBiquadResonantFilter, int aType, float aFrequency, float aResonance);

/*
 * Bus
 */
void Bus_destroy(Bus * aBus);
Bus * Bus_create();
void Bus_setFilter(Bus * aBus, unsigned int aFilterId, Filter * aFilter);
unsigned int Bus_play(Bus * aBus, AudioSource * aSound);
unsigned int Bus_playEx(Bus * aBus, AudioSource * aSound, float aVolume /* = 1.0f */, float aPan /* = 0.0f */, int aPaused /* = 0 */);
unsigned int Bus_playClocked(Bus * aBus, double aSoundTime, AudioSource * aSound);
unsigned int Bus_playClockedEx(Bus * aBus, double aSoundTime, AudioSource * aSound, float aVolume /* = 1.0f */, float aPan /* = 0.0f */);
unsigned int Bus_play3d(Bus * aBus, AudioSource * aSound, float aPosX, float aPosY, float aPosZ);
unsigned int Bus_play3dEx(Bus * aBus, AudioSource * aSound, float aPosX, float aPosY, float aPosZ, float aVelX /* = 0.0f */, float aVelY /* = 0.0f */, float aVelZ /* = 0.0f */, float aVolume /* = 1.0f */, int aPaused /* = 0 */);
unsigned int Bus_play3dClocked(Bus * aBus, double aSoundTime, AudioSource * aSound, float aPosX, float aPosY, float aPosZ);
unsigned int Bus_play3dClockedEx(Bus * aBus, double aSoundTime, AudioSource * aSound, float aPosX, float aPosY, float aPosZ, float aVelX /* = 0.0f */, float aVelY /* = 0.0f */, float aVelZ /* = 0.0f */, float aVolume /* = 1.0f */);
int Bus_setChannels(Bus * aBus, unsigned int aChannels);
void Bus_setVisualizationEnable(Bus * aBus, int aEnable);
void Bus_annexSound(Bus * aBus, unsigned int aVoiceHandle);
float * Bus_calcFFT(Bus * aBus);
float * Bus_getWave(Bus * aBus);
float Bus_getApproximateVolume(Bus * aBus, unsigned int aChannel);
unsigned int Bus_getActiveVoiceCount(Bus * aBus);
void Bus_setVolume(Bus * aBus, float aVolume);
void Bus_setLooping(Bus * aBus, int aLoop);
void Bus_set3dMinMaxDistance(Bus * aBus, float aMinDistance, float aMaxDistance);
void Bus_set3dAttenuation(Bus * aBus, unsigned int aAttenuationModel, float aAttenuationRolloffFactor);
void Bus_set3dDopplerFactor(Bus * aBus, float aDopplerFactor);
void Bus_set3dListenerRelative(Bus * aBus, int aListenerRelative);
void Bus_set3dDistanceDelay(Bus * aBus, int aDistanceDelay);
void Bus_set3dCollider(Bus * aBus, AudioCollider * aCollider);
void Bus_set3dColliderEx(Bus * aBus, AudioCollider * aCollider, int aUserData /* = 0 */);
void Bus_set3dAttenuator(Bus * aBus, AudioAttenuator * aAttenuator);
void Bus_setInaudibleBehavior(Bus * aBus, int aMustTick, int aKill);
void Bus_setLoopPoint(Bus * aBus, double aLoopPoint);
double Bus_getLoopPoint(Bus * aBus);
void Bus_stop(Bus * aBus);

/*
 * DCRemovalFilter
 */
void DCRemovalFilter_destroy(DCRemovalFilter * aDCRemovalFilter);
DCRemovalFilter * DCRemovalFilter_create();
int DCRemovalFilter_setParams(DCRemovalFilter * aDCRemovalFilter);
int DCRemovalFilter_setParamsEx(DCRemovalFilter * aDCRemovalFilter, float aLength /* = 0.1f */);
int DCRemovalFilter_getParamCount(DCRemovalFilter * aDCRemovalFilter);
const char * DCRemovalFilter_getParamName(DCRemovalFilter * aDCRemovalFilter, unsigned int aParamIndex);
unsigned int DCRemovalFilter_getParamType(DCRemovalFilter * aDCRemovalFilter, unsigned int aParamIndex);
float DCRemovalFilter_getParamMax(DCRemovalFilter * aDCRemovalFilter, unsigned int aParamIndex);
float DCRemovalFilter_getParamMin(DCRemovalFilter * aDCRemovalFilter, unsigned int aParamIndex);

/*
 * EchoFilter
 */
void EchoFilter_destroy(EchoFilter * aEchoFilter);
int EchoFilter_getParamCount(EchoFilter * aEchoFilter);
const char * EchoFilter_getParamName(EchoFilter * aEchoFilter, unsigned int aParamIndex);
unsigned int EchoFilter_getParamType(EchoFilter * aEchoFilter, unsigned int aParamIndex);
float EchoFilter_getParamMax(EchoFilter * aEchoFilter, unsigned int aParamIndex);
float EchoFilter_getParamMin(EchoFilter * aEchoFilter, unsigned int aParamIndex);
EchoFilter * EchoFilter_create();
int EchoFilter_setParams(EchoFilter * aEchoFilter, float aDelay);
int EchoFilter_setParamsEx(EchoFilter * aEchoFilter, float aDelay, float aDecay /* = 0.7f */, float aFilter /* = 0.0f */);

/*
 * FFTFilter
 */
void FFTFilter_destroy(FFTFilter * aFFTFilter);
FFTFilter * FFTFilter_create();
int FFTFilter_getParamCount(FFTFilter * aFFTFilter);
const char * FFTFilter_getParamName(FFTFilter * aFFTFilter, unsigned int aParamIndex);
unsigned int FFTFilter_getParamType(FFTFilter * aFFTFilter, unsigned int aParamIndex);
float FFTFilter_getParamMax(FFTFilter * aFFTFilter, unsigned int aParamIndex);
float FFTFilter_getParamMin(FFTFilter * aFFTFilter, unsigned int aParamIndex);

/*
 * FlangerFilter
 */
void FlangerFilter_destroy(FlangerFilter * aFlangerFilter);
int FlangerFilter_getParamCount(FlangerFilter * aFlangerFilter);
const char * FlangerFilter_getParamName(FlangerFilter * aFlangerFilter, unsigned int aParamIndex);
unsigned int FlangerFilter_getParamType(FlangerFilter * aFlangerFilter, unsigned int aParamIndex);
float FlangerFilter_getParamMax(FlangerFilter * aFlangerFilter, unsigned int aParamIndex);
float FlangerFilter_getParamMin(FlangerFilter * aFlangerFilter, unsigned int aParamIndex);
FlangerFilter * FlangerFilter_create();
int FlangerFilter_setParams(FlangerFilter * aFlangerFilter, float aDelay, float aFreq);

/*
 * FreeverbFilter
 */
void FreeverbFilter_destroy(FreeverbFilter * aFreeverbFilter);
int FreeverbFilter_getParamCount(FreeverbFilter * aFreeverbFilter);
const char * FreeverbFilter_getParamName(FreeverbFilter * aFreeverbFilter, unsigned int aParamIndex);
unsigned int FreeverbFilter_getParamType(FreeverbFilter * aFreeverbFilter, unsigned int aParamIndex);
float FreeverbFilter_getParamMax(FreeverbFilter * aFreeverbFilter, unsigned int aParamIndex);
float FreeverbFilter_getParamMin(FreeverbFilter * aFreeverbFilter, unsigned int aParamIndex);
FreeverbFilter * FreeverbFilter_create();
int FreeverbFilter_setParams(FreeverbFilter * aFreeverbFilter, float aMode, float aRoomSize, float aDamp, float aWidth);

/*
 * LofiFilter
 */
void LofiFilter_destroy(LofiFilter * aLofiFilter);
int LofiFilter_getParamCount(LofiFilter * aLofiFilter);
const char * LofiFilter_getParamName(LofiFilter * aLofiFilter, unsigned int aParamIndex);
unsigned int LofiFilter_getParamType(LofiFilter * aLofiFilter, unsigned int aParamIndex);
float LofiFilter_getParamMax(LofiFilter * aLofiFilter, unsigned int aParamIndex);
float LofiFilter_getParamMin(LofiFilter * aLofiFilter, unsigned int aParamIndex);
LofiFilter * LofiFilter_create();
int LofiFilter_setParams(LofiFilter * aLofiFilter, float aSampleRate, float aBitdepth);

/*
 * Monotone
 */
void Monotone_destroy(Monotone * aMonotone);
Monotone * Monotone_create();
int Monotone_setParams(Monotone * aMonotone, int aHardwareChannels);
int Monotone_setParamsEx(Monotone * aMonotone, int aHardwareChannels, int aWaveform /* = SoLoud::Misc::WAVE_SQUARE */);
int Monotone_load(Monotone * aMonotone, const char * aFilename);
int Monotone_loadMem(Monotone * aMonotone, const unsigned char * aMem, unsigned int aLength);
int Monotone_loadMemEx(Monotone * aMonotone, const unsigned char * aMem, unsigned int aLength, int aCopy /* = false */, int aTakeOwnership /* = true */);
int Monotone_loadFile(Monotone * aMonotone, File * aFile);
void Monotone_setVolume(Monotone * aMonotone, float aVolume);
void Monotone_setLooping(Monotone * aMonotone, int aLoop);
void Monotone_set3dMinMaxDistance(Monotone * aMonotone, float aMinDistance, float aMaxDistance);
void Monotone_set3dAttenuation(Monotone * aMonotone, unsigned int aAttenuationModel, float aAttenuationRolloffFactor);
void Monotone_set3dDopplerFactor(Monotone * aMonotone, float aDopplerFactor);
void Monotone_set3dListenerRelative(Monotone * aMonotone, int aListenerRelative);
void Monotone_set3dDistanceDelay(Monotone * aMonotone, int aDistanceDelay);
void Monotone_set3dCollider(Monotone * aMonotone, AudioCollider * aCollider);
void Monotone_set3dColliderEx(Monotone * aMonotone, AudioCollider * aCollider, int aUserData /* = 0 */);
void Monotone_set3dAttenuator(Monotone * aMonotone, AudioAttenuator * aAttenuator);
void Monotone_setInaudibleBehavior(Monotone * aMonotone, int aMustTick, int aKill);
void Monotone_setLoopPoint(Monotone * aMonotone, double aLoopPoint);
double Monotone_getLoopPoint(Monotone * aMonotone);
void Monotone_setFilter(Monotone * aMonotone, unsigned int aFilterId, Filter * aFilter);
void Monotone_stop(Monotone * aMonotone);

/*
 * Noise
 */
void Noise_destroy(Noise * aNoise);
Noise * Noise_create();
void Noise_setOctaveScale(Noise * aNoise, float aOct0, float aOct1, float aOct2, float aOct3, float aOct4, float aOct5, float aOct6, float aOct7, float aOct8, float aOct9);
void Noise_setType(Noise * aNoise, int aType);
void Noise_setVolume(Noise * aNoise, float aVolume);
void Noise_setLooping(Noise * aNoise, int aLoop);
void Noise_set3dMinMaxDistance(Noise * aNoise, float aMinDistance, float aMaxDistance);
void Noise_set3dAttenuation(Noise * aNoise, unsigned int aAttenuationModel, float aAttenuationRolloffFactor);
void Noise_set3dDopplerFactor(Noise * aNoise, float aDopplerFactor);
void Noise_set3dListenerRelative(Noise * aNoise, int aListenerRelative);
void Noise_set3dDistanceDelay(Noise * aNoise, int aDistanceDelay);
void Noise_set3dCollider(Noise * aNoise, AudioCollider * aCollider);
void Noise_set3dColliderEx(Noise * aNoise, AudioCollider * aCollider, int aUserData /* = 0 */);
void Noise_set3dAttenuator(Noise * aNoise, AudioAttenuator * aAttenuator);
void Noise_setInaudibleBehavior(Noise * aNoise, int aMustTick, int aKill);
void Noise_setLoopPoint(Noise * aNoise, double aLoopPoint);
double Noise_getLoopPoint(Noise * aNoise);
void Noise_setFilter(Noise * aNoise, unsigned int aFilterId, Filter * aFilter);
void Noise_stop(Noise * aNoise);

/*
 * Openmpt
 */
void Openmpt_destroy(Openmpt * aOpenmpt);
Openmpt * Openmpt_create();
int Openmpt_load(Openmpt * aOpenmpt, const char * aFilename);
int Openmpt_loadMem(Openmpt * aOpenmpt, const unsigned char * aMem, unsigned int aLength);
int Openmpt_loadMemEx(Openmpt * aOpenmpt, const unsigned char * aMem, unsigned int aLength, int aCopy /* = false */, int aTakeOwnership /* = true */);
int Openmpt_loadFile(Openmpt * aOpenmpt, File * aFile);
void Openmpt_setVolume(Openmpt * aOpenmpt, float aVolume);
void Openmpt_setLooping(Openmpt * aOpenmpt, int aLoop);
void Openmpt_set3dMinMaxDistance(Openmpt * aOpenmpt, float aMinDistance, float aMaxDistance);
void Openmpt_set3dAttenuation(Openmpt * aOpenmpt, unsigned int aAttenuationModel, float aAttenuationRolloffFactor);
void Openmpt_set3dDopplerFactor(Openmpt * aOpenmpt, float aDopplerFactor);
void Openmpt_set3dListenerRelative(Openmpt * aOpenmpt, int aListenerRelative);
void Openmpt_set3dDistanceDelay(Openmpt * aOpenmpt, int aDistanceDelay);
void Openmpt_set3dCollider(Openmpt * aOpenmpt, AudioCollider * aCollider);
void Openmpt_set3dColliderEx(Openmpt * aOpenmpt, AudioCollider * aCollider, int aUserData /* = 0 */);
void Openmpt_set3dAttenuator(Openmpt * aOpenmpt, AudioAttenuator * aAttenuator);
void Openmpt_setInaudibleBehavior(Openmpt * aOpenmpt, int aMustTick, int aKill);
void Openmpt_setLoopPoint(Openmpt * aOpenmpt, double aLoopPoint);
double Openmpt_getLoopPoint(Openmpt * aOpenmpt);
void Openmpt_setFilter(Openmpt * aOpenmpt, unsigned int aFilterId, Filter * aFilter);
void Openmpt_stop(Openmpt * aOpenmpt);

/*
 * Queue
 */
void Queue_destroy(Queue * aQueue);
Queue * Queue_create();
int Queue_play(Queue * aQueue, AudioSource * aSound);
unsigned int Queue_getQueueCount(Queue * aQueue);
int Queue_isCurrentlyPlaying(Queue * aQueue, AudioSource * aSound);
int Queue_setParamsFromAudioSource(Queue * aQueue, AudioSource * aSound);
int Queue_setParams(Queue * aQueue, float aSamplerate);
int Queue_setParamsEx(Queue * aQueue, float aSamplerate, unsigned int aChannels /* = 2 */);
void Queue_setVolume(Queue * aQueue, float aVolume);
void Queue_setLooping(Queue * aQueue, int aLoop);
void Queue_set3dMinMaxDistance(Queue * aQueue, float aMinDistance, float aMaxDistance);
void Queue_set3dAttenuation(Queue * aQueue, unsigned int aAttenuationModel, float aAttenuationRolloffFactor);
void Queue_set3dDopplerFactor(Queue * aQueue, float aDopplerFactor);
void Queue_set3dListenerRelative(Queue * aQueue, int aListenerRelative);
void Queue_set3dDistanceDelay(Queue * aQueue, int aDistanceDelay);
void Queue_set3dCollider(Queue * aQueue, AudioCollider * aCollider);
void Queue_set3dColliderEx(Queue * aQueue, AudioCollider * aCollider, int aUserData /* = 0 */);
void Queue_set3dAttenuator(Queue * aQueue, AudioAttenuator * aAttenuator);
void Queue_setInaudibleBehavior(Queue * aQueue, int aMustTick, int aKill);
void Queue_setLoopPoint(Queue * aQueue, double aLoopPoint);
double Queue_getLoopPoint(Queue * aQueue);
void Queue_setFilter(Queue * aQueue, unsigned int aFilterId, Filter * aFilter);
void Queue_stop(Queue * aQueue);

/*
 * RobotizeFilter
 */
void RobotizeFilter_destroy(RobotizeFilter * aRobotizeFilter);
int RobotizeFilter_getParamCount(RobotizeFilter * aRobotizeFilter);
const char * RobotizeFilter_getParamName(RobotizeFilter * aRobotizeFilter, unsigned int aParamIndex);
unsigned int RobotizeFilter_getParamType(RobotizeFilter * aRobotizeFilter, unsigned int aParamIndex);
float RobotizeFilter_getParamMax(RobotizeFilter * aRobotizeFilter, unsigned int aParamIndex);
float RobotizeFilter_getParamMin(RobotizeFilter * aRobotizeFilter, unsigned int aParamIndex);
void RobotizeFilter_setParams(RobotizeFilter * aRobotizeFilter, float aFreq, int aWaveform);
RobotizeFilter * RobotizeFilter_create();

/*
 * Sfxr
 */
void Sfxr_destroy(Sfxr * aSfxr);
Sfxr * Sfxr_create();
void Sfxr_resetParams(Sfxr * aSfxr);
int Sfxr_loadParams(Sfxr * aSfxr, const char * aFilename);
int Sfxr_loadParamsMem(Sfxr * aSfxr, unsigned char * aMem, unsigned int aLength);
int Sfxr_loadParamsMemEx(Sfxr * aSfxr, unsigned char * aMem, unsigned int aLength, int aCopy /* = false */, int aTakeOwnership /* = true */);
int Sfxr_loadParamsFile(Sfxr * aSfxr, File * aFile);
int Sfxr_loadPreset(Sfxr * aSfxr, int aPresetNo, int aRandSeed);
void Sfxr_setVolume(Sfxr * aSfxr, float aVolume);
void Sfxr_setLooping(Sfxr * aSfxr, int aLoop);
void Sfxr_set3dMinMaxDistance(Sfxr * aSfxr, float aMinDistance, float aMaxDistance);
void Sfxr_set3dAttenuation(Sfxr * aSfxr, unsigned int aAttenuationModel, float aAttenuationRolloffFactor);
void Sfxr_set3dDopplerFactor(Sfxr * aSfxr, float aDopplerFactor);
void Sfxr_set3dListenerRelative(Sfxr * aSfxr, int aListenerRelative);
void Sfxr_set3dDistanceDelay(Sfxr * aSfxr, int aDistanceDelay);
void Sfxr_set3dCollider(Sfxr * aSfxr, AudioCollider * aCollider);
void Sfxr_set3dColliderEx(Sfxr * aSfxr, AudioCollider * aCollider, int aUserData /* = 0 */);
void Sfxr_set3dAttenuator(Sfxr * aSfxr, AudioAttenuator * aAttenuator);
void Sfxr_setInaudibleBehavior(Sfxr * aSfxr, int aMustTick, int aKill);
void Sfxr_setLoopPoint(Sfxr * aSfxr, double aLoopPoint);
double Sfxr_getLoopPoint(Sfxr * aSfxr);
void Sfxr_setFilter(Sfxr * aSfxr, unsigned int aFilterId, Filter * aFilter);
void Sfxr_stop(Sfxr * aSfxr);

/*
 * Speech
 */
void Speech_destroy(Speech * aSpeech);
Speech * Speech_create();
int Speech_setText(Speech * aSpeech, const char * aText);
int Speech_setParams(Speech * aSpeech);
int Speech_setParamsEx(Speech * aSpeech, unsigned int aBaseFrequency /* = 1330 */, float aBaseSpeed /* = 10.0f */, float aBaseDeclination /* = 0.5f */, int aBaseWaveform /* = KW_TRIANGLE */);
void Speech_setVolume(Speech * aSpeech, float aVolume);
void Speech_setLooping(Speech * aSpeech, int aLoop);
void Speech_set3dMinMaxDistance(Speech * aSpeech, float aMinDistance, float aMaxDistance);
void Speech_set3dAttenuation(Speech * aSpeech, unsigned int aAttenuationModel, float aAttenuationRolloffFactor);
void Speech_set3dDopplerFactor(Speech * aSpeech, float aDopplerFactor);
void Speech_set3dListenerRelative(Speech * aSpeech, int aListenerRelative);
void Speech_set3dDistanceDelay(Speech * aSpeech, int aDistanceDelay);
void Speech_set3dCollider(Speech * aSpeech, AudioCollider * aCollider);
void Speech_set3dColliderEx(Speech * aSpeech, AudioCollider * aCollider, int aUserData /* = 0 */);
void Speech_set3dAttenuator(Speech * aSpeech, AudioAttenuator * aAttenuator);
void Speech_setInaudibleBehavior(Speech * aSpeech, int aMustTick, int aKill);
void Speech_setLoopPoint(Speech * aSpeech, double aLoopPoint);
double Speech_getLoopPoint(Speech * aSpeech);
void Speech_setFilter(Speech * aSpeech, unsigned int aFilterId, Filter * aFilter);
void Speech_stop(Speech * aSpeech);

/*
 * TedSid
 */
void TedSid_destroy(TedSid * aTedSid);
TedSid * TedSid_create();
int TedSid_load(TedSid * aTedSid, const char * aFilename);
int TedSid_loadToMem(TedSid * aTedSid, const char * aFilename);
int TedSid_loadMem(TedSid * aTedSid, const unsigned char * aMem, unsigned int aLength);
int TedSid_loadMemEx(TedSid * aTedSid, const unsigned char * aMem, unsigned int aLength, int aCopy /* = false */, int aTakeOwnership /* = true */);
int TedSid_loadFileToMem(TedSid * aTedSid, File * aFile);
int TedSid_loadFile(TedSid * aTedSid, File * aFile);
void TedSid_setVolume(TedSid * aTedSid, float aVolume);
void TedSid_setLooping(TedSid * aTedSid, int aLoop);
void TedSid_set3dMinMaxDistance(TedSid * aTedSid, float aMinDistance, float aMaxDistance);
void TedSid_set3dAttenuation(TedSid * aTedSid, unsigned int aAttenuationModel, float aAttenuationRolloffFactor);
void TedSid_set3dDopplerFactor(TedSid * aTedSid, float aDopplerFactor);
void TedSid_set3dListenerRelative(TedSid * aTedSid, int aListenerRelative);
void TedSid_set3dDistanceDelay(TedSid * aTedSid, int aDistanceDelay);
void TedSid_set3dCollider(TedSid * aTedSid, AudioCollider * aCollider);
void TedSid_set3dColliderEx(TedSid * aTedSid, AudioCollider * aCollider, int aUserData /* = 0 */);
void TedSid_set3dAttenuator(TedSid * aTedSid, AudioAttenuator * aAttenuator);
void TedSid_setInaudibleBehavior(TedSid * aTedSid, int aMustTick, int aKill);
void TedSid_setLoopPoint(TedSid * aTedSid, double aLoopPoint);
double TedSid_getLoopPoint(TedSid * aTedSid);
void TedSid_setFilter(TedSid * aTedSid, unsigned int aFilterId, Filter * aFilter);
void TedSid_stop(TedSid * aTedSid);

/*
 * Vic
 */
void Vic_destroy(Vic * aVic);
Vic * Vic_create();
void Vic_setModel(Vic * aVic, int model);
int Vic_getModel(Vic * aVic);
void Vic_setRegister(Vic * aVic, int reg, unsigned char value);
unsigned char Vic_getRegister(Vic * aVic, int reg);
void Vic_setVolume(Vic * aVic, float aVolume);
void Vic_setLooping(Vic * aVic, int aLoop);
void Vic_set3dMinMaxDistance(Vic * aVic, float aMinDistance, float aMaxDistance);
void Vic_set3dAttenuation(Vic * aVic, unsigned int aAttenuationModel, float aAttenuationRolloffFactor);
void Vic_set3dDopplerFactor(Vic * aVic, float aDopplerFactor);
void Vic_set3dListenerRelative(Vic * aVic, int aListenerRelative);
void Vic_set3dDistanceDelay(Vic * aVic, int aDistanceDelay);
void Vic_set3dCollider(Vic * aVic, AudioCollider * aCollider);
void Vic_set3dColliderEx(Vic * aVic, AudioCollider * aCollider, int aUserData /* = 0 */);
void Vic_set3dAttenuator(Vic * aVic, AudioAttenuator * aAttenuator);
void Vic_setInaudibleBehavior(Vic * aVic, int aMustTick, int aKill);
void Vic_setLoopPoint(Vic * aVic, double aLoopPoint);
double Vic_getLoopPoint(Vic * aVic);
void Vic_setFilter(Vic * aVic, unsigned int aFilterId, Filter * aFilter);
void Vic_stop(Vic * aVic);

/*
 * Vizsn
 */
void Vizsn_destroy(Vizsn * aVizsn);
Vizsn * Vizsn_create();
void Vizsn_setText(Vizsn * aVizsn, char * aText);
void Vizsn_setVolume(Vizsn * aVizsn, float aVolume);
void Vizsn_setLooping(Vizsn * aVizsn, int aLoop);
void Vizsn_set3dMinMaxDistance(Vizsn * aVizsn, float aMinDistance, float aMaxDistance);
void Vizsn_set3dAttenuation(Vizsn * aVizsn, unsigned int aAttenuationModel, float aAttenuationRolloffFactor);
void Vizsn_set3dDopplerFactor(Vizsn * aVizsn, float aDopplerFactor);
void Vizsn_set3dListenerRelative(Vizsn * aVizsn, int aListenerRelative);
void Vizsn_set3dDistanceDelay(Vizsn * aVizsn, int aDistanceDelay);
void Vizsn_set3dCollider(Vizsn * aVizsn, AudioCollider * aCollider);
void Vizsn_set3dColliderEx(Vizsn * aVizsn, AudioCollider * aCollider, int aUserData /* = 0 */);
void Vizsn_set3dAttenuator(Vizsn * aVizsn, AudioAttenuator * aAttenuator);
void Vizsn_setInaudibleBehavior(Vizsn * aVizsn, int aMustTick, int aKill);
void Vizsn_setLoopPoint(Vizsn * aVizsn, double aLoopPoint);
double Vizsn_getLoopPoint(Vizsn * aVizsn);
void Vizsn_setFilter(Vizsn * aVizsn, unsigned int aFilterId, Filter * aFilter);
void Vizsn_stop(Vizsn * aVizsn);

/*
 * Wav
 */
void Wav_destroy(Wav * aWav);
Wav * Wav_create();
int Wav_load(Wav * aWav, const char * aFilename);
int Wav_loadMem(Wav * aWav, const unsigned char * aMem, unsigned int aLength);
int Wav_loadMemEx(Wav * aWav, const unsigned char * aMem, unsigned int aLength, int aCopy /* = false */, int aTakeOwnership /* = true */);
int Wav_loadFile(Wav * aWav, File * aFile);
int Wav_loadRawWave8(Wav * aWav, unsigned char * aMem, unsigned int aLength);
int Wav_loadRawWave8Ex(Wav * aWav, unsigned char * aMem, unsigned int aLength, float aSamplerate /* = 44100.0f */, unsigned int aChannels /* = 1 */);
int Wav_loadRawWave16(Wav * aWav, short * aMem, unsigned int aLength);
int Wav_loadRawWave16Ex(Wav * aWav, short * aMem, unsigned int aLength, float aSamplerate /* = 44100.0f */, unsigned int aChannels /* = 1 */);
int Wav_loadRawWave(Wav * aWav, float * aMem, unsigned int aLength);
int Wav_loadRawWaveEx(Wav * aWav, float * aMem, unsigned int aLength, float aSamplerate /* = 44100.0f */, unsigned int aChannels /* = 1 */, int aCopy /* = false */, int aTakeOwnership /* = true */);
double Wav_getLength(Wav * aWav);
void Wav_setVolume(Wav * aWav, float aVolume);
void Wav_setLooping(Wav * aWav, int aLoop);
void Wav_set3dMinMaxDistance(Wav * aWav, float aMinDistance, float aMaxDistance);
void Wav_set3dAttenuation(Wav * aWav, unsigned int aAttenuationModel, float aAttenuationRolloffFactor);
void Wav_set3dDopplerFactor(Wav * aWav, float aDopplerFactor);
void Wav_set3dListenerRelative(Wav * aWav, int aListenerRelative);
void Wav_set3dDistanceDelay(Wav * aWav, int aDistanceDelay);
void Wav_set3dCollider(Wav * aWav, AudioCollider * aCollider);
void Wav_set3dColliderEx(Wav * aWav, AudioCollider * aCollider, int aUserData /* = 0 */);
void Wav_set3dAttenuator(Wav * aWav, AudioAttenuator * aAttenuator);
void Wav_setInaudibleBehavior(Wav * aWav, int aMustTick, int aKill);
void Wav_setLoopPoint(Wav * aWav, double aLoopPoint);
double Wav_getLoopPoint(Wav * aWav);
void Wav_setFilter(Wav * aWav, unsigned int aFilterId, Filter * aFilter);
void Wav_stop(Wav * aWav);

/*
 * WaveShaperFilter
 */
void WaveShaperFilter_destroy(WaveShaperFilter * aWaveShaperFilter);
int WaveShaperFilter_setParams(WaveShaperFilter * aWaveShaperFilter, float aAmount);
WaveShaperFilter * WaveShaperFilter_create();
int WaveShaperFilter_getParamCount(WaveShaperFilter * aWaveShaperFilter);
const char * WaveShaperFilter_getParamName(WaveShaperFilter * aWaveShaperFilter, unsigned int aParamIndex);
unsigned int WaveShaperFilter_getParamType(WaveShaperFilter * aWaveShaperFilter, unsigned int aParamIndex);
float WaveShaperFilter_getParamMax(WaveShaperFilter * aWaveShaperFilter, unsigned int aParamIndex);
float WaveShaperFilter_getParamMin(WaveShaperFilter * aWaveShaperFilter, unsigned int aParamIndex);

/*
 * WavStream
 */
void WavStream_destroy(WavStream * aWavStream);
WavStream * WavStream_create();
int WavStream_load(WavStream * aWavStream, const char * aFilename);
int WavStream_loadMem(WavStream * aWavStream, const unsigned char * aData, unsigned int aDataLen);
int WavStream_loadMemEx(WavStream * aWavStream, const unsigned char * aData, unsigned int aDataLen, int aCopy /* = false */, int aTakeOwnership /* = true */);
int WavStream_loadToMem(WavStream * aWavStream, const char * aFilename);
int WavStream_loadFile(WavStream * aWavStream, File * aFile);
int WavStream_loadFileToMem(WavStream * aWavStream, File * aFile);
double WavStream_getLength(WavStream * aWavStream);
void WavStream_setVolume(WavStream * aWavStream, float aVolume);
void WavStream_setLooping(WavStream * aWavStream, int aLoop);
void WavStream_set3dMinMaxDistance(WavStream * aWavStream, float aMinDistance, float aMaxDistance);
void WavStream_set3dAttenuation(WavStream * aWavStream, unsigned int aAttenuationModel, float aAttenuationRolloffFactor);
void WavStream_set3dDopplerFactor(WavStream * aWavStream, float aDopplerFactor);
void WavStream_set3dListenerRelative(WavStream * aWavStream, int aListenerRelative);
void WavStream_set3dDistanceDelay(WavStream * aWavStream, int aDistanceDelay);
void WavStream_set3dCollider(WavStream * aWavStream, AudioCollider * aCollider);
void WavStream_set3dColliderEx(WavStream * aWavStream, AudioCollider * aCollider, int aUserData /* = 0 */);
void WavStream_set3dAttenuator(WavStream * aWavStream, AudioAttenuator * aAttenuator);
void WavStream_setInaudibleBehavior(WavStream * aWavStream, int aMustTick, int aKill);
void WavStream_setLoopPoint(WavStream * aWavStream, double aLoopPoint);
double WavStream_getLoopPoint(WavStream * aWavStream);
void WavStream_setFilter(WavStream * aWavStream, unsigned int aFilterId, Filter * aFilter);
void WavStream_stop(WavStream * aWavStream);
#ifdef  __cplusplus
} // extern "C"
#endif

#endif // SOLOUD_C_H_INCLUDED

