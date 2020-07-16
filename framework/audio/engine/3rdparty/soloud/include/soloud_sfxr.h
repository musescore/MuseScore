/*
SFXR module for SoLoud audio engine
Copyright (c) 2014 Jari Komppa
Based on code (c) by Tomas Pettersson, re-licensed under zlib by permission

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

#ifndef SFXR_H
#define SFXR_H

#include "soloud.h"
#include "soloud_misc.h"

namespace SoLoud
{
	class File;

	struct SfxrParams
	{
		int wave_type;

		float p_base_freq;
		float p_freq_limit;
		float p_freq_ramp;
		float p_freq_dramp;
		float p_duty;
		float p_duty_ramp;

		float p_vib_strength;
		float p_vib_speed;
		float p_vib_delay;

		float p_env_attack;
		float p_env_sustain;
		float p_env_decay;
		float p_env_punch;

		bool filter_on;
		float p_lpf_resonance;
		float p_lpf_freq;
		float p_lpf_ramp;
		float p_hpf_freq;
		float p_hpf_ramp;

		float p_pha_offset;
		float p_pha_ramp;

		float p_repeat_speed;

		float p_arp_speed;
		float p_arp_mod;

		float master_vol;

		float sound_vol;
	};

	class Sfxr;

	class SfxrInstance : public AudioSourceInstance
	{
		Sfxr *mParent;

		Misc::Prg mRand;
		SfxrParams mParams;

		bool playing_sample;
		int phase;
		double fperiod;
		double fmaxperiod;
		double fslide;
		double fdslide;
		int period;
		float square_duty;
		float square_slide;
		int env_stage;
		int env_time;
		int env_length[3];
		float env_vol;
		float fphase;
		float fdphase;
		int iphase;
		float phaser_buffer[1024];
		int ipp;
		float noise_buffer[32];
		float fltp;
		float fltdp;
		float fltw;
		float fltw_d;
		float fltdmp;
		float fltphp;
		float flthp;
		float flthp_d;
		float vib_phase;
		float vib_speed;
		float vib_amp;
		int rep_time;
		int rep_limit;
		int arp_time;
		int arp_limit;
		double arp_mod;

		void resetSample(bool aRestart);

	public:
		SfxrInstance(Sfxr *aParent);
		virtual unsigned int getAudio(float *aBuffer, unsigned int aSamplesToRead, unsigned int aBufferSize);
		virtual bool hasEnded();
	};

	class Sfxr : public AudioSource
	{
	public:
		SfxrParams mParams;

		enum SFXR_PRESETS 
		{
			COIN,
			LASER,
			EXPLOSION,
			POWERUP,
			HURT,
			JUMP,
			BLIP
		};

		Misc::Prg mRand;
		
		Sfxr();
		virtual ~Sfxr();
		void resetParams();
		result loadParams(const char* aFilename);
		result loadParamsMem(unsigned char *aMem, unsigned int aLength, bool aCopy = false, bool aTakeOwnership = true);
		result loadParamsFile(File *aFile);

		result loadPreset(int aPresetNo, int aRandSeed);
		virtual AudioSourceInstance *createInstance();
	};
};

#endif