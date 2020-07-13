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

/*********************************************************************************
*
* Testing goal is primarily for sanity checks, to verify that everything did not
* blow up due to some innocent-looking change.
*
* In some cases this means that all we're testing is that yeah, there's noise, or
* that yeah, the noise changes when we changed a parameter.
*
* Some tests against known good values can also be done, for deterministic processes.
*
**********************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "soloud.h"
#include "soloud_bassboostfilter.h"
#include "soloud_biquadresonantfilter.h"
#include "soloud_dcremovalfilter.h"
#include "soloud_echofilter.h"
#include "soloud_flangerfilter.h"
#include "soloud_lofifilter.h"
#include "soloud_monotone.h"
#include "soloud_openmpt.h"
#include "soloud_robotizefilter.h"
#include "soloud_sfxr.h"
#include "soloud_speech.h"
#include "soloud_tedsid.h"
#include "soloud_vic.h"
#include "soloud_wav.h"
#include "soloud_waveshaperfilter.h"
#include "soloud_wavstream.h"

// This option is useful while developing tests:
//#define NO_LASTKNOWN_CHECK

int errorcount = 0;
int tests = 0;
int verbose = 1;

float lastknownscratch[2048];
FILE *lastknownfile = 0;
int lastknownwrite = 0;

#define CHECK_RES(x) tests++; if ((x)) { errorcount++; printf("Error on line %d, %s(): %s\n",__LINE__,__FUNCTION__, soloud.getErrorString((x)));}
#define CHECK(x) tests++; if (!(x)) { errorcount++; printf("Error on line %d, %s(): Check \"%s\" fail\n",__LINE__,__FUNCTION__,#x);}
#define CHECK_BUF_NONZERO(x, n) tests++; { int i, zero = 1; for (i = 0; i < (n); i++) if ((x)[i] != 0) zero = 0; if (zero) { errorcount++; printf("Error on line %d, %s(): buffer not nonzero\n",__LINE__,__FUNCTION__);}}
#define CHECK_BUF_ZERO(x, n) tests++; { int i, zero = 1; for (i = 0; i < (n); i++) if ((x)[i] != 0) zero = 0; if (!zero) { errorcount++; printf("Error on line %d, %s(): buffer not zero\n",__LINE__,__FUNCTION__);}}
#define CHECK_BUF_DIFF(x, y, n) tests++; { int i, same = 1; for (i = 0; i < (n); i++) if (fabs((x)[i] - (y)[i]) > 0.00001) same = 0; if (same) { errorcount++; printf("Error on line %d, %s(): buffers are equal\n",__LINE__,__FUNCTION__);}}
#define CHECK_BUF_SAME(x, y, n) tests++; { int i, same = 1; for (i = 0; i < (n); i++) if (fabs((x)[i] - (y)[i]) > 0.00001) same = 0; if (!same) { errorcount++; printf("Error on line %d, %s(): buffers differ\n",__LINE__,__FUNCTION__);}}
#define CHECK_BUF_SAME_LASTKNOWN(x, n) tests++; { int i, same = 1; for (i = 0; i < (n); i++) if (fabs((x)[i] - lastknownscratch[i]) > 0.00001) same = 0; if (!same) { errorcount++; printf("Error on line %d, %s(): output differs from last known\n",__LINE__,__FUNCTION__);}}
#define CHECK_BUF_GTE(x, y, n) tests++; { int i, lt = 0; for (i = 0; i < (n); i++) if (fabs((x)[i]) - fabs((y)[i]) < 0) lt = 1; if (lt) { errorcount++; printf("Error on line %d, %s(): buffer %s magnitude not bigger than buffer %s \n",__LINE__,__FUNCTION__,#x, #y);}}
#define CHECKLASTKNOWN(x, n) if (lastknownwrite && lastknownfile) { fwrite((x),1,(n)*sizeof(float),lastknownfile); } else if (lastknownfile) { fread(lastknownscratch,1,(n)*sizeof(float),lastknownfile); CHECK_BUF_SAME_LASTKNOWN((x), n); }

#if defined(_MSC_VER)
#include <windows.h>

long getmsec()
{
	LARGE_INTEGER ts, freq;

	QueryPerformanceCounter(&ts);
	QueryPerformanceFrequency(&freq);
	ts.QuadPart *= 1000;
	ts.QuadPart /= freq.QuadPart;

	return (long)ts.QuadPart;
}
#else
#include <time.h>

long getmsec()
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (ts->tv_sec * 1000) + (ts->tv_nsec / 1000000)
}
#endif


void writeHeader()
{
	unsigned char buf[46] = {
		0x52, 0x49, 0x46, 0x46, // RIFF
		0xa4, 0x3e, 0x00, 0x00, // length of file - 8
		0x57, 0x41, 0x56, 0x45, // WAVE
		0x66, 0x6d, 0x74, 0x20, // fmt
		0x12, 0x00, 0x00, 0x00, // PCM block
		0x03, 0x00,             // Uncompressed PCM, float
		0x02, 0x00,             // Channels (stereo)
		0x44, 0xac, 0x00, 0x00, // 44100Hz
		0x20, 0x62, 0x05, 0x00, // 44100*8=352800 bytes per sec
		0x08, 0x00,             // Number of bytes per sample for all channels
		0x20, 0x00,             // Bits per channel (32)
		0x00, 0x00,             // 0 bytes of extension
		0x64, 0x61, 0x74, 0x61, // data
		0x80, 0x3e, 0x00, 0x00  // bytes of data
		// 46 bytes up to this point
	};
	int *flen = (int*)(buf + 4);
	int *dlen = (int*)(buf + 42);
	if (!lastknownfile || !lastknownwrite)
		return;
	int len = ftell(lastknownfile);
	*flen = len - 8;
	*dlen = len - 46;
	fseek(lastknownfile, 0, SEEK_SET);
	fwrite(buf, 1, 46, lastknownfile);
}

void generateTestWave(SoLoud::Wav &aWav)
{
	unsigned char buf[16044] = { 
		0x52, 0x49, 0x46, 0x46, // RIFF
		0xa4, 0x3e, 0x00, 0x00, // length of file - 8
		0x57, 0x41, 0x56, 0x45, // WAVE
		0x66, 0x6d, 0x74, 0x20, // fmt
		0x10, 0x00, 0x00, 0x00, // PCM block
		0x01, 0x00,             // Uncompressed PCM
		0x01, 0x00,             // Channels (mono)
		0x40, 0x1f, 0x00, 0x00, // 8000Hz
		0x40, 0x1f, 0x00, 0x00, // 8000 bytes per sec
		0x01, 0x00,             // Number of bytes per sample for all channels
		0x08, 0x00,             // Bits per channel
		0x64, 0x61, 0x74, 0x61, // data
		0x80, 0x3e, 0x00, 0x00, // bytes of data
		// 44 bytes up to this point
	};
	unsigned int buflen = sizeof(buf);
	int i;
	for (i = 0; i < 16000; i++)
	{
		buf[i + 44] = ((i&1)?1:-1)*(char)((sin(i*i * 0.000001) * 0x7f) + i);
	}
	aWav.loadMem(buf, buflen, true, false);
}

void printinfo(const char * format, ...)
{
	if (!verbose)
		return;
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

// Some info tests
//
// Soloud.init
// Soloud.deinit
// Soloud.getVersion
// Soloud.getErrorString
// Soloud.getBackendId
// Soloud.getBackendString
// Soloud.getBackendChannels
// Soloud.getBackendSamplerate
// Soloud.getBackendBufferSize
// Soloud.mix
// Soloud.mixSigned16
// Prg.rand
// Prg.srand
// wav.getLength
void testMisc()
{
	float scratch[2048];
	short scratch_i16[2048];
	SoLoud::result res;
	SoLoud::Soloud soloud;
	res = soloud.init(SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::NULLDRIVER);
	CHECK_RES(res);
	SoLoud::Wav wav;
	generateTestWave(wav);
	CHECK(wav.getLength() != 0);
	int ver = soloud.getVersion();
	CHECK(ver == SOLOUD_VERSION);
	printinfo("SoLoud version %d\n", ver);
	CHECK(soloud.getErrorString(0) != 0);
	printinfo("Backend %d: %s, %d channels, %d samplerate, %d buffersize\n",
		soloud.getBackendId(),
		soloud.getBackendString(),
		soloud.getBackendChannels(),
		soloud.getBackendSamplerate(),
		soloud.getBackendBufferSize());
	CHECK(soloud.getBackendId() != 0);
	CHECK(soloud.getBackendString() != 0);
	CHECK(soloud.getBackendChannels() != 0);
	CHECK(soloud.getBackendSamplerate() != 0);
	CHECK(soloud.getBackendBufferSize() != 0);

	soloud.mix(scratch, 1000);
	soloud.mixSigned16(scratch_i16, 1000);
	CHECK_BUF_ZERO(scratch, 2000);
	CHECK_BUF_ZERO(scratch_i16, 2000);
	soloud.play(wav);
	soloud.mix(scratch, 1000);
	soloud.mixSigned16(scratch_i16, 1000);
	CHECK_BUF_NONZERO(scratch, 2000);
	CHECK_BUF_NONZERO(scratch_i16, 2000);

	SoLoud::Misc::Prg prg;
	prg.srand(0x1337);
	int a = prg.rand();
	prg.srand(0x1337);
	int b = prg.rand();
	CHECK(a == b);
	a = 0;
	for (b = 0; b < 100; b++)
		if (prg.rand() != prg.rand())
			a = 1;
	CHECK(a == 1);

	soloud.deinit();
}

// Test parameter getters
//
// Soloud.getFilterParameter
// Soloud.getStreamTime
// Soloud.getPause
// Soloud.getVolume
// Soloud.getOverallVolume
// Soloud.getPan
// Soloud.getSamplerate
// Soloud.getProtectVoice
// Soloud.getActiveVoiceCount
// Soloud.getVoiceCount
// Soloud.isValidVoiceHandle
// Soloud.getRelativePlaySpeed
// Soloud.getPostClipScaler
// Soloud.getGlobalVolume
// Soloud.getMaxActiveVoiceCount
// Soloud.setMaxActiveVoiceCount
// Soloud.getLooping
// Soloud.get3dSoundSpeed
void testGetters()
{
	float scratch[2048];
	SoLoud::result res;
	SoLoud::Soloud soloud; 
	SoLoud::Sfxr sfxr;
	SoLoud::BiquadResonantFilter filter;
	res = soloud.init(SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::NULLDRIVER);
	CHECK_RES(res);
	res = sfxr.loadPreset(4, 0);
	CHECK_RES(res);
	sfxr.setFilter(0, &filter);

	CHECK(soloud.getActiveVoiceCount() == 0);
	CHECK(soloud.getVoiceCount() == 0);

	CHECK(soloud.isValidVoiceHandle((SoLoud::handle)0xbaadf00d) == 0);
	int h = soloud.play(sfxr);
	CHECK(soloud.isValidVoiceHandle(h));
	
	CHECK(soloud.getActiveVoiceCount() == 1);
	CHECK(soloud.getVoiceCount() == 1);

	float v_in, v_out;
	v_in = 0.7447f;
	soloud.setFilterParameter(h, 0, 0, v_in);
	v_out = soloud.getFilterParameter(h, 0, 0);
	CHECK(fabs(v_in - v_out) < 0.00001);
	
	CHECK(soloud.getStreamTime(h) < 0.00001);
	soloud.mix(scratch, 1000);
	CHECK(soloud.getStreamTime(h) > 0.00001);
	
	CHECK(soloud.getPause(h) == 0);
	soloud.setPause(h, true);
	CHECK(soloud.getPause(h) != 0);

	float oldvol = soloud.getOverallVolume(h);
	soloud.setVolume(h, v_in);
	v_out = soloud.getVolume(h);
	CHECK(fabs(v_in - v_out) < 0.00001);
	CHECK(fabs(oldvol - v_out) > 0.00001);

	soloud.setPan(h, v_in);
	CHECK(fabs(v_in - soloud.getPan(h)) < 0.00001);

	soloud.setSamplerate(h, v_in);
	CHECK(fabs(v_in - soloud.getSamplerate(h)) < 0.00001);

	CHECK(soloud.getProtectVoice(h) == 0);
	soloud.setProtectVoice(h, true);
	CHECK(soloud.getProtectVoice(h) != 0);

	soloud.setRelativePlaySpeed(h, v_in);
	CHECK(fabs(v_in - soloud.getRelativePlaySpeed(h)) < 0.00001);

	soloud.setPostClipScaler(v_in);
	CHECK(fabs(v_in - soloud.getPostClipScaler()) < 0.00001);

	soloud.setGlobalVolume(v_in);
	CHECK(fabs(v_in - soloud.getGlobalVolume()) < 0.00001);

	CHECK(soloud.getLooping(h) == 0);
	soloud.setLooping(h, true);
	CHECK(soloud.getLooping(h) != 0);

	CHECK(soloud.getMaxActiveVoiceCount() > 0);
	soloud.setMaxActiveVoiceCount(123);
	CHECK(soloud.getMaxActiveVoiceCount() == 123);

	soloud.set3dSoundSpeed(123);
	CHECK(soloud.get3dSoundSpeed() == 123);

	soloud.deinit();
}

// Visualization API tests
//
// Soloud.setVisualizationEnable
// Soloud.calcFFT
// Soloud.getWave
// Soloud.getApproximateVolume
// Bus.setVisualizationEnable
// Bus.calcFFT
// Bus.getWave
// Bus.getApproximateVolume
void testVis()
{
	float scratch[2048];
	SoLoud::result res;
	SoLoud::Soloud soloud; 
	SoLoud::Sfxr sfxr;
	SoLoud::Bus bus;
	res = soloud.init(SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::NULLDRIVER);
	CHECK_RES(res);
	res = sfxr.loadPreset(4, 0);
	CHECK_RES(res);

	int bush = soloud.play(bus);
	int h = bus.play(sfxr);
	soloud.setVisualizationEnable(true);
	bus.setVisualizationEnable(true);

	soloud.mix(scratch, 1000);
	float approxvol = soloud.getApproximateVolume(0);
	CHECK(approxvol != 0);

	float *w = soloud.getWave();
	CHECK(w != NULL);
	if (w)
	{
		int i;
		int nonzero = 0;
		for (i = 0; i < 256; i++)
			if (w[i] != 0)
				nonzero = 1;
		CHECK(nonzero != 0);
	}

	approxvol = bus.getApproximateVolume(0);
	CHECK(approxvol != 0);

	w = bus.getWave();
	CHECK(w != NULL);
	if (w)
	{
		int i;
		int nonzero = 0;
		for (i = 0; i < 256; i++)
			if (w[i] != 0)
				nonzero = 1;
		CHECK(nonzero != 0);
	}

	w = soloud.calcFFT();
	CHECK(w != NULL);
	if (w)
	{
		int i;
		int nonzero = 0;
		for (i = 0; i < 256; i++)
			if (w[i] != 0)
				nonzero = 1;
		CHECK(nonzero != 0);
	}

	w = bus.calcFFT();
	CHECK(w != NULL);
	if (w)
	{
		int i;
		int nonzero = 0;
		for (i = 0; i < 256; i++)
			if (w[i] != 0)
				nonzero = 1;
		CHECK(nonzero != 0);
	}

	soloud.deinit();
}

// Test various play-related calls
// 
// Soloud.play
// Soloud.playClocked
// Soloud.playBackground
// Soloud.seek
// Soloud.stop
// Soloud.stopAll
// Soloud.stopAudioSource
// Bus.play
// Bus.playClocked
// Soloud.setDelaySamples
// Bus.setChannels
// Soloud.setProtectVoice
void testPlay()
{
	float scratch[2048];
	float ref[2048];
	SoLoud::result res;
	SoLoud::Soloud soloud; 
	SoLoud::Wav wav;
	SoLoud::Bus bus;
	generateTestWave(wav);
	res = soloud.init(SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::NULLDRIVER);
	CHECK_RES(res);

	int h = soloud.play(wav);
	soloud.mix(ref, 1000);
	CHECKLASTKNOWN(ref, 2000);
	CHECK_BUF_NONZERO(ref, 2000);
	soloud.stop(h);
	soloud.mix(scratch, 1000);
	CHECK_BUF_ZERO(scratch, 2000);
	h = soloud.play(wav);
	soloud.stopAudioSource(wav);
	soloud.mix(scratch, 1000);
	CHECK_BUF_ZERO(scratch, 2000);
	h = soloud.play(wav);
	soloud.stopAll();
	soloud.mix(scratch, 1000);
	CHECK_BUF_ZERO(scratch, 2000);

	h = soloud.playClocked(0.01, wav);
	soloud.stopAll();
	h = soloud.playClocked(0.015, wav);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_NONZERO(scratch, 2000);
	CHECK_BUF_DIFF(scratch, ref, 2000);
	soloud.stopAll();

	h = soloud.play(wav,1,0,true);
	soloud.setDelaySamples(h, 10);
	soloud.setPause(h, false);
	soloud.mix(scratch, 1000);
	CHECK_BUF_SAME(scratch + 20, ref, 2000-20);
	soloud.stopAll();

	/*
	// Not a bug, but a feature of the linear resampler.
	soloud.play(bus);
	h = bus.play(wav);
	soloud.mix(scratch, 1000);
	CHECK_BUF_SAME(scratch, ref, 2000); // TODO: fails, seems to offset by a sample when played through bus.
	soloud.stopAll();
	*/

	soloud.play(bus);
	h = bus.playClocked(0.01, wav);
	soloud.stopAll();
	soloud.play(bus);
	h = bus.playClocked(0.015, wav);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_NONZERO(scratch, 2000);
	CHECK_BUF_DIFF(scratch, ref, 2000);
	soloud.stopAll();

	bus.setChannels(1);
	soloud.play(bus);
	h = bus.play(wav);
	soloud.mix(scratch, 1000);
	CHECK_BUF_NONZERO(scratch, 2000);
	CHECKLASTKNOWN(scratch, 2000);
	soloud.stopAll();

	bus.setChannels(2);
	soloud.play(bus);
	h = bus.play(wav);
	soloud.mix(scratch, 1000);
	CHECK_BUF_NONZERO(scratch, 2000);
	CHECKLASTKNOWN(scratch, 2000);
	soloud.stopAll();

	bus.setChannels(4);
	soloud.play(bus);
	h = bus.play(wav);
	soloud.mix(scratch, 1000);
	CHECK_BUF_NONZERO(scratch, 2000);
	CHECKLASTKNOWN(scratch, 2000);
	soloud.stopAll();

	bus.setChannels(6);
	soloud.play(bus);
	h = bus.play(wav);
	soloud.mix(scratch, 1000);
	CHECK_BUF_NONZERO(scratch, 2000);
	CHECKLASTKNOWN(scratch, 2000);
	soloud.stopAll();

	h = soloud.play(wav);
	soloud.seek(h, 0.1);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_NONZERO(scratch, 2000);
	CHECK_BUF_DIFF(scratch, ref, 2000);
	soloud.stopAll();
	
	h = soloud.playBackground(wav);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_NONZERO(scratch, 2000);
	CHECK_BUF_GTE(scratch, ref, 2000);

	h = soloud.play(wav, 0.5f);
	int i;
	for (i = 0; i < 3000; i++)
		soloud.play(wav, 1.0f);
	CHECK(soloud.isValidVoiceHandle(h) == 0);
	soloud.stopAll();

	h = soloud.play(wav, 0.5f);
	soloud.setProtectVoice(h, true);
	for (i = 0; i < 3000; i++)
		soloud.play(wav, 1.0f);
	CHECK(soloud.isValidVoiceHandle(h) != 0);
	soloud.stopAll();


	soloud.deinit();
}


class customAttenuatorCollider : public SoLoud::AudioCollider, public SoLoud::AudioAttenuator
{
public:
	virtual float collide(SoLoud::Soloud *aSoloud, SoLoud::AudioSourceInstance3dData *aAudioInstance3dData, int aUserData)
	{
		return 0.5f;
	}
	virtual float attenuate(float aDistance, float aMinDistance, float aMaxDistance, float aRolloffFactor)
	{
		return 0.5f;
	}
};

// Test various 3d functions
// 
// Soloud.play3d
// Soloud.play3dClocked
// Bus.play3d
// Bus.play3dClocked
// Soloud.set3dListenerParameters
// Soloud.set3dListenerPosition
// Soloud.set3dListenerAt
// Soloud.set3dListenerUp
// Soloud.set3dListenerVelocity
// Soloud.set3dSourceParameters
// Soloud.set3dSourcePosition
// Soloud.set3dSourceVelocity
// Soloud.set3dSourceMinMaxDistance
// Soloud.set3dSourceAttenuation
// Soloud.set3dSourceDopplerFactor
// Wav.set3dMinMaxDistance
// Wav.set3dAttenuation
// Wav.set3dDopplerFactor
// Wav.set3dListenerRelative
// Wav.set3dDistanceDelay
// Wav.set3dCollider
// Wav.set3dAttenuator
// Soloud.update3dAudio
// Soloud.setSpeakerPosition
// Soloud.getSpeakerPosition
// Soloud.set3dSoundSpeed
// Wav.setInaudibleBehavior
// Soloud.setInaudibleBehavior
void test3d()
{
	customAttenuatorCollider customAC;
	float scratch[2048];
	float ref[2048];
	SoLoud::result res;
	SoLoud::Soloud soloud;
	SoLoud::Bus bus;
	SoLoud::Wav wav;
	generateTestWave(wav);
	res = soloud.init(SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::NULLDRIVER);
	CHECK_RES(res);
	soloud.set3dListenerParameters(0, 5, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0);
	soloud.set3dSoundSpeed(343);
	soloud.setSpeakerPosition(0, 2, 0, 1);
	soloud.setSpeakerPosition(0, -2, 0, 1);
	wav.set3dMinMaxDistance(1, 200);
	wav.set3dAttenuation(SoLoud::AudioSource::EXPONENTIAL_DISTANCE, 0.5);
	soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.update3dAudio();
	soloud.mix(ref, 1000);
	CHECK_BUF_NONZERO(ref, 2000);
	CHECKLASTKNOWN(ref, 2000);
	soloud.stopAll();

	soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_SAME(ref, scratch, 2000);
	soloud.stopAll();

	wav.set3dAttenuation(SoLoud::AudioSource::EXPONENTIAL_DISTANCE, 0.25);
	soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();
	wav.set3dAttenuation(SoLoud::AudioSource::EXPONENTIAL_DISTANCE, 0.5);

	wav.set3dMinMaxDistance(1, 20);
	soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();
	wav.set3dMinMaxDistance(1, 200);

	soloud.play3dClocked(0.01f, wav, 10, 20, 30, 1, 1, 1);
	soloud.stopAll();
	soloud.play3dClocked(0.02f, wav, 10, 20, 30, 1, 1, 1);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	CHECK_BUF_NONZERO(scratch, 2000);
	soloud.stopAll();

	soloud.set3dSoundSpeed(34);
	soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();
	soloud.set3dSoundSpeed(343);

	soloud.set3dListenerParameters(1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0);
	soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();
	soloud.set3dListenerParameters(0, 5, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0);

	soloud.set3dListenerPosition(1, 2, 3);
	soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();
	soloud.set3dListenerParameters(0, 5, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0);

	soloud.set3dListenerAt(1, 2, 3);
	soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();
	soloud.set3dListenerParameters(0, 5, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0);

	soloud.set3dListenerUp(1, 2, 3);
	soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();
	soloud.set3dListenerParameters(0, 5, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0);

	soloud.set3dListenerVelocity(1, -1, 0);
	soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();
	soloud.set3dListenerParameters(0, 5, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0);

	int h = soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.set3dSourceAttenuation(h, SoLoud::AudioSource::LINEAR_DISTANCE, 0.5f);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();

	h = soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.set3dSourceDopplerFactor(h, 2.5f);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();

	h = soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.set3dSourceMinMaxDistance(h, 1, 20);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();

	h = soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.set3dSourcePosition(h, 20, 10, 30);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();

	h = soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.set3dSourceVelocity(h, 2, 1, 3);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();

	h = soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.set3dSourceParameters(h, 1, 2, 3, 4, 5, 6);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();

	h = soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.set3dSourceParameters(h, 1, 2, 3, 4, 5, 6);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();

	soloud.setSpeakerPosition(0, 3, -1, -1);
	soloud.setSpeakerPosition(0, -3, -1, 1);
	soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();
	soloud.setSpeakerPosition(0, 2, 0, 1);
	soloud.setSpeakerPosition(0, -2, 0, 1);

	wav.set3dListenerRelative(true);
	soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();
	wav.set3dListenerRelative(false);

	wav.set3dDistanceDelay(true);
	soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000); // skip time to get to when the sound starts 
	soloud.mix(scratch, 1000);
	soloud.mix(scratch, 1000);
	soloud.mix(scratch, 1000);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();
	wav.set3dDistanceDelay(false);

	wav.set3dDopplerFactor(15.0f);
	soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();
	wav.set3dDopplerFactor(1.0f);

	wav.set3dAttenuator(&customAC);
	soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();
	wav.set3dAttenuator(0);

	wav.set3dCollider(&customAC);
	soloud.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();
	wav.set3dCollider(0);

	wav.setInaudibleBehavior(false, false); // don't tick, but don't kill if inaudible
	h = soloud.play3d(wav, 10, 20, 30, 1, 1, 1, 0.0f);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECK(soloud.isValidVoiceHandle(h) == 1);
	soloud.stopAll();

	h = soloud.play3d(wav, 10, 20, 30, 1, 1, 1, 0.0f);
	soloud.setInaudibleBehavior(h, false, true); // don't tick, kill if inaudible
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECK(soloud.isValidVoiceHandle(h) == 0);
	soloud.stopAll();

	wav.setInaudibleBehavior(false, true); // don't tick, kill if inaudible
	h = soloud.play3d(wav, 10, 20, 30, 1, 1, 1, 0.0f);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECK(soloud.isValidVoiceHandle(h) == 0);
	soloud.stopAll();

	wav.setInaudibleBehavior(false, false); // don't tick, but don't kill if inaudible
	h = soloud.play3d(wav, 10, 20, 30, 1, 1, 1, 0.0f);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	soloud.setVolume(h, 1.0f);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECK_BUF_GTE(ref, scratch, 2000); // fails, looks like 3d audio has some initial volume ramp problem..
	soloud.stopAll();

	wav.setInaudibleBehavior(true, false);
	h = soloud.play3d(wav, 10, 20, 30, 1, 1, 1, 0.0f);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	soloud.setVolume(h, 1.0f);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	CHECK_BUF_NONZERO(scratch, 2000);
	CHECKLASTKNOWN(scratch, 2000);
	soloud.stopAll();

	soloud.play(bus);
	bus.play3d(wav, 10, 20, 30, 1, 1, 1);
	soloud.update3dAudio();
	soloud.mix(ref, 1000);
	CHECK_BUF_NONZERO(ref, 2000);
	CHECKLASTKNOWN(ref, 2000);
	soloud.stopAll();

	soloud.play(bus);
	bus.play3dClocked(2.01f, wav, 10, 20, 30, 1, 1, 1);
	soloud.stopAll();
	soloud.play(bus);
	bus.play3dClocked(2.02f, wav, 10, 20, 30, 1, 1, 1);
	soloud.update3dAudio();
	soloud.mix(scratch, 1000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	CHECK_BUF_NONZERO(scratch, 2000);
	CHECKLASTKNOWN(scratch, 2000);
	soloud.stopAll();

	float x, y, z;
	soloud.setSpeakerPosition(0, 1, 2, 3);
	soloud.getSpeakerPosition(0, x, y, z);
	CHECK(x == 1 && y == 2 && z == 3);
	soloud.setSpeakerPosition(0, 4, 5, 6);
	soloud.getSpeakerPosition(0, x, y, z);
	CHECK(x != 1 && y != 2 && z != 3);

	soloud.deinit();
}

// Test various filter options
//
// BiquadResonantFilter.setParams
// LofiFilter.setParams
// EchoFilter.setParams
// BassboostFilter.setParams
// FlangerFilter.setParams
// DCRemovalFilter.setParams
// Soloud.setFilterParameter
// Soloud.fadeFilterParameter
// Soloud.oscillateFilterParameter
// Soloud.setGlobalFilter
// WaveShaperFilter.setParams
void testFilters()
{
	float scratch[2048];
	float ref[2048];
	float ref2[2048];
	SoLoud::result res;
	SoLoud::Soloud soloud;
	SoLoud::Wav wav;
	generateTestWave(wav);
	SoLoud::BiquadResonantFilter biquad;
	SoLoud::LofiFilter lofi;
	SoLoud::EchoFilter echo;
	SoLoud::BassboostFilter bass;
	SoLoud::FlangerFilter flang;
	SoLoud::DCRemovalFilter dc;
	SoLoud::FFTFilter fft;
	SoLoud::RobotizeFilter rob;
	SoLoud::WaveShaperFilter wshap;

	res = soloud.init(SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::NULLDRIVER);
	CHECK_RES(res);

	soloud.play(wav);
	soloud.mix(ref, 1000);
	soloud.stopAll();
	CHECKLASTKNOWN(ref, 2000);


	lofi.setParams(4000, 5);
	wav.setFilter(0, &lofi);
	soloud.play(wav);
	soloud.mix(ref2, 1000);
	CHECKLASTKNOWN(ref2, 2000);
	CHECK_BUF_DIFF(ref, ref2, 2000);
	soloud.stopAll();
	lofi.setParams(2000, 3);
	soloud.play(wav);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref2, scratch, 2000);
	soloud.stopAll();
	lofi.setParams(4000, 5);
	int h = soloud.play(wav);
	soloud.setFilterParameter(h, 0, 0, 0.5f);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	lofi.setParams(4000, 5);
	h = soloud.play(wav);
	soloud.fadeFilterParameter(h, 0, 0, 0.5f, 1.0f);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	lofi.setParams(4000, 5);
	h = soloud.play(wav);
	soloud.oscillateFilterParameter(h, 0, 0, 0.25f, 0.75f, 0.5f);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);

	biquad.setParams(SoLoud::BiquadResonantFilter::LOWPASS, 2000, 5);
	wav.setFilter(0, &biquad);
	soloud.play(wav);
	soloud.mix(ref2, 1000);
	CHECKLASTKNOWN(ref2, 2000);
	CHECK_BUF_DIFF(ref, ref2, 2000);
	soloud.stopAll();
	biquad.setParams(SoLoud::BiquadResonantFilter::HIGHPASS, 1000, 5);
	soloud.play(wav);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref2, scratch, 2000);
	soloud.stopAll();

	echo.setParams(0.05f, 0.5f, 0.5f);
	wav.setFilter(0, &echo);
	soloud.play(wav);
	soloud.mix(ref2, 1000);
	CHECKLASTKNOWN(ref2, 2000);
	CHECK_BUF_DIFF(ref, ref2, 2000);
	soloud.stopAll();
	echo.setParams(0.01f, 0.75f, 0.1f);
	soloud.play(wav);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref2, scratch, 2000);
	soloud.stopAll();

	bass.setParams(2);
	wav.setFilter(0, &bass);
	soloud.play(wav);
	soloud.mix(ref2, 1000);
	CHECKLASTKNOWN(ref2, 2000);
	CHECK_BUF_DIFF(ref, ref2, 2000);
	soloud.stopAll();
	bass.setParams(10);
	soloud.play(wav);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref2, scratch, 2000);
	soloud.stopAll();

	dc.setParams(0.1f);
	wav.setFilter(0, &dc);
	soloud.play(wav);
	soloud.mix(ref2, 1000);
	CHECKLASTKNOWN(ref2, 2000);
	CHECK_BUF_DIFF(ref, ref2, 2000);
	soloud.stopAll();
	dc.setParams(0.5f);
	soloud.play(wav);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref2, scratch, 2000);
	soloud.stopAll();

	flang.setParams(0.05f, 10);
	wav.setFilter(0, &flang);
	soloud.play(wav);
	soloud.mix(ref2, 1000);
	CHECKLASTKNOWN(ref2, 2000);
	CHECK_BUF_DIFF(ref, ref2, 2000);
	soloud.stopAll();
	flang.setParams(0.005f, 5);
	soloud.play(wav);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref2, scratch, 2000);
	soloud.stopAll();
	wav.setFilter(0, 0);
	soloud.setGlobalFilter(0, &flang);
	soloud.play(wav);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref2, scratch, 2000);
	soloud.stopAll();

	wav.setFilter(0, &fft);
	soloud.play(wav);
	soloud.mix(ref2, 1000);
	CHECKLASTKNOWN(ref2, 2000);
	CHECK_BUF_DIFF(ref, ref2, 2000);
	soloud.stopAll();
	wav.setFilter(0, 0);

	wav.setFilter(0, &rob);
	soloud.play(wav);
	soloud.mix(ref2, 1000);
	CHECKLASTKNOWN(ref2, 2000);
	CHECK_BUF_DIFF(ref, ref2, 2000);
	soloud.stopAll();
	wav.setFilter(0, 0);

	wshap.setParams(0.05f);
	wav.setFilter(0, &wshap);
	soloud.play(wav);
	soloud.mix(ref2, 1000);
	CHECKLASTKNOWN(ref2, 2000);
	CHECK_BUF_DIFF(ref, ref2, 2000);
	soloud.stopAll();
	wshap.setParams(0.005f);
	soloud.play(wav);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref2, scratch, 2000);
	soloud.stopAll();
	wav.setFilter(0, 0);

	soloud.deinit();
}

// Test various core functionality
// 
// Soloud.setGlobalVolume
// Soloud.setPostClipScaler
// Soloud.setPause
// Soloud.setPauseAll
// Soloud.setRelativePlaySpeed
// Soloud.setSamplerate
// Soloud.setPan
// Soloud.setPanAbsolute
// Soloud.setVolume
// Wav.setVolume
// Soloud.fadeVolume
// Soloud.fadePan
// Soloud.fadeRelativePlaySpeed
// Soloud.fadeGlobalVolume
// Soloud.schedulePause
// Soloud.scheduleStop
// Soloud.oscillateVolume
// Soloud.oscillatePan
// Soloud.oscillateRelativePlaySpeed
// Soloud.oscillateGlobalVolume
// Soloud.setLooping
// Soloud.getLoopCount
// Soloud.createVoiceGroup
// Soloud.destroyVoiceGroup
// Soloud.addVoiceToGroup
// Soloud.isVoiceGroup
// Soloud.isVoiceGroupEmpty
// Soloud.countAudioSource
// Soloud.getStreamPosition
void testCore()
{
	float scratch[2048];
	float ref[2048];
	SoLoud::result res;
	SoLoud::Soloud soloud;
	SoLoud::Wav wav;
	generateTestWave(wav);
	res = soloud.init(SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::NULLDRIVER);
	CHECK_RES(res);

	soloud.setPostClipScaler(1.0f);
	soloud.setGlobalVolume(1.0f);
	soloud.play(wav);
	soloud.mix(ref, 1000);
	soloud.stopAll();

	soloud.setGlobalVolume(0.5f);
	soloud.play(wav);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_GTE(ref, scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();
	soloud.setGlobalVolume(1.0f);

	soloud.setPostClipScaler(0.5f);
	soloud.play(wav);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_GTE(ref, scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();
	soloud.setPostClipScaler(1.0f);

	int h = soloud.play(wav);
	soloud.setPause(h, true);
	soloud.mix(scratch, 1000);
	CHECK_BUF_ZERO(scratch, 2000);
	soloud.stopAll();

	soloud.play(wav);
	soloud.setPauseAll(true);
	soloud.mix(scratch, 1000);
	CHECK_BUF_ZERO(scratch, 2000);
	soloud.stopAll();

	h = soloud.play(wav);
	soloud.setRelativePlaySpeed(h, 0.3f);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();

	h = soloud.play(wav);
	soloud.setSamplerate(h, 12345);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();

	h = soloud.play(wav);
	soloud.setPan(h, 0.75f);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();

	h = soloud.play(wav);
	soloud.setPanAbsolute(h, 0.75f, 0.25f);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();

	wav.setVolume(0.5f);
	h = soloud.play(wav);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_GTE(ref, scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();
	wav.setVolume(1.0f);

	h = soloud.play(wav);
	soloud.setVolume(h, 0.5f);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_GTE(ref, scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();

	h = soloud.play(wav);
	soloud.fadeVolume(h, 0.5f, 1.0f);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_GTE(ref, scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();

	h = soloud.play(wav);
	soloud.fadePan(h, 0.75f, 1.0f);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();

	h = soloud.play(wav);
	soloud.fadeRelativePlaySpeed(h, 0.5f, 1.0f);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();

	soloud.play(wav);
	soloud.fadeGlobalVolume(0.5f, 1.0f);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_GTE(ref, scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();
	soloud.setGlobalVolume(1.0f);

	h = soloud.play(wav);
	soloud.schedulePause(h, 0.015f); // note: pausescheduler works on mix granularity
	soloud.mix(scratch, 500);
	soloud.mix(scratch+1000, 500);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_NONZERO(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	CHECK(soloud.getVoiceCount() > 0);
	soloud.stopAll();

	h = soloud.play(wav);
	soloud.scheduleStop(h, 0.015f); // note: stopscheduler works on mix granularity
	soloud.mix(scratch, 500);
	soloud.mix(scratch + 1000, 500);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_NONZERO(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	CHECK(soloud.getVoiceCount() == 0);
	soloud.stopAll();

	h = soloud.play(wav);
	soloud.oscillateVolume(h, 0.25f, 0.75f, 0.2f);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_GTE(ref, scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();

	h = soloud.play(wav);
	soloud.oscillatePan(h, 0.25f, 0.75f, 0.2f);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();

	h = soloud.play(wav);
	soloud.oscillateRelativePlaySpeed(h, 0.75f, 1.25f, 0.2f);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();

	soloud.play(wav);
	soloud.oscillateGlobalVolume(0.25f, 0.75f, 0.2f);
	soloud.mix(scratch, 1000);
	CHECKLASTKNOWN(scratch, 2000);
	CHECK_BUF_GTE(ref, scratch, 2000);
	CHECK_BUF_DIFF(ref, scratch, 2000);
	soloud.stopAll();
	soloud.setGlobalVolume(1.0f);

	h = soloud.play(wav);
	soloud.setLooping(h, false);
	int i;
	for (i = 0; i < 100; i++)
		soloud.mix(scratch, 1000);
	CHECK(soloud.getVoiceCount() == 0);
	soloud.stopAll();

	h = soloud.play(wav);
	soloud.setLooping(h, true);
	CHECK(soloud.getLoopCount(h) == 0);
	for (i = 0; i < 100; i++)
		soloud.mix(scratch, 1000);
	CHECK(soloud.getVoiceCount() != 0);
	CHECK(soloud.getLoopCount(h) != 0);
	soloud.stopAll();

	int vg = soloud.createVoiceGroup();
	CHECK(soloud.isVoiceGroup(vg) != 0);
	CHECK(soloud.isVoiceGroupEmpty(vg) != 0);
	h = soloud.play(wav);
	soloud.addVoiceToGroup(vg, h);
	CHECK(soloud.isVoiceGroup(h) == 0);
	CHECK(soloud.isVoiceGroupEmpty(vg) == 0);
	soloud.destroyVoiceGroup(vg);
	CHECK(soloud.isVoiceGroup(vg) == 0);

	h = soloud.play(wav);
	CHECK(soloud.getStreamPosition(h) == 0);
	soloud.setLooping(h, true);
	for (i = 0; i < 100; i++)
		soloud.mix(scratch, 1000);
	CHECK(soloud.getStreamPosition(h) != 0);
	CHECK(soloud.countAudioSource(wav) != 0);
	soloud.stopAll();
	CHECK(soloud.countAudioSource(wav) == 0);


	soloud.deinit();
}

// Test speech audio source
//
// Speech.setText
// Speech.setParams
// Speech.setLooping
// Speech.setLoopPoint
// Speech.getLoopPoint
// Speech.stop
// Soloud.getLoopPoint
// Soloud.setLoopPoint
void testSpeech()
{
	float scratch[2048];
	float ref[2048];
	SoLoud::result res;
	SoLoud::Soloud soloud;
	SoLoud::Speech speech;
	res = soloud.init(SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::NULLDRIVER);
	CHECK_RES(res);

	speech.setParams(1330, 10, 0.5f, 1);
	speech.setText("Why is it so loud");
	soloud.play(speech);
	soloud.mix(ref, 1000);
	CHECK_BUF_NONZERO(ref, 2000);
	CHECKLASTKNOWN(ref, 2000);
	soloud.stopAll();

	speech.setParams(1330, 10, 0.5f, 1);
	speech.setText("Why is it so loud");
	soloud.play(speech);
	soloud.mix(scratch, 1000);
	CHECK_BUF_SAME(scratch, ref, 2000);
	CHECKLASTKNOWN(scratch, 2000);
	soloud.stopAll();

	speech.setText("a different text");
	soloud.play(speech);
	soloud.mix(scratch, 1000);
	CHECK_BUF_DIFF(scratch, ref, 2000);
	CHECKLASTKNOWN(scratch, 2000);
	soloud.stopAll();
	speech.setText("Why is it so loud");

	speech.setParams(1000, 5, 0, 2);
	soloud.play(speech);
	soloud.mix(scratch, 1000);
	CHECK_BUF_DIFF(scratch, ref, 2000);
	CHECKLASTKNOWN(scratch, 2000);
	soloud.stopAll();
	speech.setParams(1330, 10, 0.5f, 1);

	speech.setLooping(false);
	soloud.play(speech);
	int i;
	for (i = 0; i < 100; i++)
		soloud.mix(scratch, 1000);
	CHECK(soloud.getActiveVoiceCount() == 0);
	soloud.stopAll();

	speech.setLooping(true);
	soloud.play(speech);
	for (i = 0; i < 100; i++)
		soloud.mix(scratch, 1000);
	soloud.mix(ref, 1000);
	CHECK(soloud.getActiveVoiceCount() != 0);
	speech.stop();
	CHECK(soloud.getActiveVoiceCount() == 0);
	soloud.stopAll();

	speech.setLoopPoint(0.1f);
	CHECK(speech.getLoopPoint() == 0.1f);
	speech.setLoopPoint(0.5f);
	CHECK(speech.getLoopPoint() != 0.1f);
	int handle = soloud.play(speech);
	soloud.setLoopPoint(handle, 0.1f);
	CHECK(soloud.getLoopPoint(handle) == 0.1f);
	soloud.setLoopPoint(handle, 0.5f);
	CHECK(soloud.getLoopPoint(handle) != 0.1f);

	for (i = 0; i < 100; i++)
		soloud.mix(scratch, 1000);
	soloud.mix(scratch, 1000);
	CHECK_BUF_DIFF(ref, scratch, 1000);
	CHECKLASTKNOWN(ref, 1000);

	soloud.deinit();
}

void testMixer()
{
	SoLoud::Soloud soloud;
	SoLoud::Wav wav;
	soloud.init(SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::NULLDRIVER);

	float val[16] = { 0.8f, -0.8f, 0.8f, -0.8f, 0.8f, -0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f };
	wav.loadRawWave(val, 2, 441, 1, true);
	float scratch[2048];
	wav.setLooping(true);
	int h = soloud.play(wav);
	int i;
	for (i = 0; i < 10; i++)
		soloud.mix(scratch + 200 * i, 100);
	int waszero = 0;
	for (i = 0; i < 2000; i++)
		if (scratch[i] < 0.00001)
		{
			if (!waszero)
			{
				printf("Zero at index %d", i);
			}
			waszero = 1;
		}
		else
		{
			if (waszero)
			{
				printf(" - %d\n", i - 1);
			}
			waszero = 0;
		}
	if (waszero)
	{
		printf(" - %d\n", i - 1);
	}

	soloud.stopAll();
	soloud.deinit();
}

// 4.8 base
// 5.1 disable_simd
// 4.6 with DAZ/FTZ

void testSpeedThings()
{
	float scratch[2048];
	SoLoud::result res;
	SoLoud::Soloud soloud;
	SoLoud::Wav wav;
	res = soloud.init(SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::NULLDRIVER);
	CHECK_RES(res);
	int j;
	generateTestWave(wav);
	int k;
	long fst = getmsec();
	for (k = 0; k < 10; k++)
	{
		long st = getmsec();
		for (j = 0; j < 500; j++)
		{
			int i;
			for (i = 0; i < 10; i++)
			{
				soloud.play(wav);
			}
			for (i = 0; i < 20; i++)
				soloud.mix(scratch, 1000);
		}
		long et = getmsec();
		printf("Mix loop took %3.3f sec\n", (et - st) / 1000.0f);
	}
	long fet = getmsec();
	printf("Total %3.3f sec\n", (fet - fst) / 1000.0f);
	soloud.deinit();
}

int main(int parc, char ** pars)
{
#ifndef NO_LASTKNOWN_CHECK
	lastknownfile = fopen("lastknown.wav", "rb");
	if (!lastknownfile)
	{
		printf("lastknown.wav not found, writing one.\n");
		lastknownfile = fopen("lastknown.wav", "wb");
		lastknownwrite = 1;
	}
	else
	{
		// skip header
		int i;
		for (i = 0; i < 46; i++)
			fgetc(lastknownfile);
	}
	writeHeader();
#endif

	testMisc();
	testGetters();
	testVis();
	testPlay();
	test3d();
	testFilters();
	testCore();
	testSpeech();
//	testSpeedThings();
//	testMixer();
	printf("\n%d tests, %d error(s) ", tests, errorcount);
	if (!lastknownwrite && errorcount)
		printf("(To rebuild lastknown.wav, simply delete it)\n");
	printf("\n");
#ifndef NO_LASTKNOWN_CHECK
	writeHeader();
	if (lastknownfile)
		fclose(lastknownfile);
	if (lastknownfile && lastknownwrite)
		printf("lastknown.wav written.\n");
#endif
	return 0;
}

/*
TODO:
----
SoLoud::Monotone
SoLoud::Openmpt
SoLoud::Queue
SoLoud::WavStream
SoLoud::Vizsn
SoLoud::Vic
SoLoud::TedSid
Wav.load
Wav.loadMem
Wav.loadFile
Wav.getLength
Wav.loadRawWave8
Wav.loadRawWave16
Wav.loadRawWave
WavStream.load
WavStream.loadMem
WavStream.loadToMem
WavStream.loadFile
WavStream.loadFileToMem
WavStream.getLength
Sfxr.resetParams
Sfxr.loadParams
Sfxr.loadParamsMem
Sfxr.loadParamsFile
Sfxr.loadPreset
Openmpt.load
Openmpt.loadMem
Openmpt.loadFile
Monotone.setParams
Monotone.load
Monotone.loadMem
Monotone.loadFile
TedSid.load
TedSid.loadToMem
TedSid.loadMem
TedSid.loadFileToMem
TedSid.loadFile
Soloud.getInfo
Queue.play
Queue.getQueueCount
Queue.isCurrentlyPlaying
Queue.setParamsFromAudioSource
Queue.setParams
Vic.setModel
Vic.getModel
Vic.setRegister
Vic.getRegister
Vizsn.setText

Not tested - abstract class
---------------------------
AudioAttenuator.attenuate

Not tested - the functionality is the same for all audio sources, tested for some source:
-----------------------------------------------------------------------------------------
Queue.setVolume
Vic.setVolume
Vizsn.setVolume
Wav.setLooping
WavStream.setLooping
Sfxr.setLooping
Openmpt.setLooping
Monotone.setLooping
TedSid.setLooping
Queue.setLooping
Queue.setInaudibleBehavior
Vic.setLooping
Vizsn.setLooping
Bus.setLoopPoint
Bus.getLoopPoint
Monotone.setLoopPoint
Monotone.getLoopPoint
Openmpt.setLoopPoint
Openmpt.getLoopPoint
Queue.setLoopPoint
Queue.getLoopPoint
Sfxr.setLoopPoint
Sfxr.getLoopPoint
TedSid.setLoopPoint
TedSid.getLoopPoint
Vic.setLoopPoint
Vic.getLoopPoint
Vizsn.setLoopPoint
Vizsn.getLoopPoint
Wav.setLoopPoint
Wav.getLoopPoint
WavStream.setLoopPoint
WavStream.getLoopPoint
Wav.stop
WavStream.stop
Sfxr.stop
Openmpt.stop
Monotone.stop
TedSid.stop
Bus.setLooping
Bus.stop
TedSid.setVolume
Monotone.setVolume
Openmpt.setVolume
Sfxr.setVolume
WavStream.setVolume
Speech.setVolume
Bus.setVolume
Bus.setFilter
Bus.set3dMinMaxDistance
Bus.set3dAttenuation
Bus.set3dDopplerFactor
Bus.set3dListenerRelative
Bus.set3dDistanceDelay
Bus.set3dCollider
Bus.set3dAttenuator
Bus.setInaudibleBehavior
Speech.set3dMinMaxDistance
Speech.set3dAttenuation
Speech.set3dDopplerFactor
Speech.set3dListenerRelative
Speech.set3dDistanceDelay
Speech.set3dCollider
Speech.set3dAttenuator
Speech.setInaudibleBehavior
Speech.setFilter
WavStream.set3dMinMaxDistance
WavStream.set3dAttenuation
WavStream.set3dDopplerFactor
WavStream.set3dListenerRelative
WavStream.set3dDistanceDelay
WavStream.set3dCollider
WavStream.set3dAttenuator
WavStream.setInaudibleBehavior
WavStream.setFilter
Sfxr.set3dMinMaxDistance
Sfxr.set3dAttenuation
Sfxr.set3dDopplerFactor
Sfxr.set3dListenerRelative
Sfxr.set3dDistanceDelay
Sfxr.set3dCollider
Sfxr.set3dAttenuator
Sfxr.setInaudibleBehavior
Sfxr.setFilter
Openmpt.set3dMinMaxDistance
Openmpt.set3dAttenuation
Openmpt.set3dDopplerFactor
Openmpt.set3dListenerRelative
Openmpt.set3dDistanceDelay
Openmpt.set3dCollider
Openmpt.set3dAttenuator
Openmpt.setInaudibleBehavior
Openmpt.setFilter
Monotone.set3dMinMaxDistance
Monotone.set3dAttenuation
Monotone.set3dDopplerFactor
Monotone.set3dListenerRelative
Monotone.set3dDistanceDelay
Monotone.set3dCollider
Monotone.set3dAttenuator
Monotone.setInaudibleBehavior
Monotone.setFilter
TedSid.set3dMinMaxDistance
TedSid.set3dAttenuation
TedSid.set3dDopplerFactor
TedSid.set3dListenerRelative
TedSid.set3dDistanceDelay
TedSid.set3dCollider
TedSid.set3dAttenuator
TedSid.setInaudibleBehavior
TedSid.setFilter
Vizsn.set3dMinMaxDistance
Vizsn.set3dAttenuation
Vizsn.set3dDopplerFactor
Vizsn.set3dListenerRelative
Vizsn.set3dDistanceDelay
Vizsn.set3dCollider
Vizsn.set3dAttenuator
Vizsn.setInaudibleBehavior
Vizsn.setFilter
Vic.set3dMinMaxDistance
Vic.set3dAttenuation
Vic.set3dDopplerFactor
Vic.set3dListenerRelative
Vic.set3dDistanceDelay
Vic.set3dCollider
Vic.set3dAttenuator
Vic.setInaudibleBehavior
Vic.setFilter
Queue.set3dMinMaxDistance
Queue.set3dAttenuation
Queue.set3dDopplerFactor
Queue.set3dListenerRelative
Queue.set3dDistanceDelay
Queue.set3dCollider
Queue.set3dAttenuator
Queue.setFilter
Vizsn.stop
Queue.stop
Vic.stop
*/