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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "soloud_sfxr.h"
#include "soloud_file.h"

namespace SoLoud
{
	SfxrInstance::SfxrInstance(Sfxr *aParent)
	{
		mParent = aParent;
		mParams = aParent->mParams;
		mRand.srand(0x792352);
		resetSample(false);
		playing_sample = 1;
	}

#define frnd(x) ((float)(mRand.rand()%10001)/10000*(x))

	unsigned int SfxrInstance::getAudio(float *aBuffer, unsigned int aSamplesToRead, unsigned int /*aBufferSize*/)
	{
		float *buffer = aBuffer;
		unsigned int i;
		for (i = 0; i < aSamplesToRead; i++)
		{
			rep_time++;
			if (rep_limit != 0 && rep_time >= rep_limit)
			{
				rep_time = 0;
				resetSample(true);
			}

			// frequency envelopes/arpeggios
			arp_time++;
			if (arp_limit != 0 && arp_time >= arp_limit)
			{
				arp_limit = 0;
				fperiod *= arp_mod;
			}
			fslide += fdslide;
			fperiod *= fslide;
			if (fperiod > fmaxperiod)
			{
				fperiod = fmaxperiod;
				if (mParams.p_freq_limit > 0.0f)
				{
					if (mFlags & LOOPING)
					{
						resetSample(false);
					}
					else
					{
						playing_sample = false;
						return i;
					}
				}
			}
			float rfperiod = (float)fperiod;
			if (vib_amp > 0.0f)
			{
				vib_phase += vib_speed;
				rfperiod = (float)(fperiod * (1.0 + sin(vib_phase) * vib_amp));
			}
			period = (int)rfperiod;
			if (period < 8) period = 8;
			square_duty += square_slide;
			if (square_duty < 0.0f) square_duty = 0.0f;
			if (square_duty > 0.5f) square_duty = 0.5f;		
			// volume envelope
			env_time++;
			if (env_time > env_length[env_stage])
			{
				env_time = 0;
				env_stage++;
				if (env_stage == 3)
				{
					if (mFlags & LOOPING)
					{
						resetSample(false);
					}
					else
					{
						playing_sample = false;
						return i;
					}
				}
			}
			if (env_stage == 0)
			{
				if (env_length[0])
				{
					env_vol = (float)env_time / env_length[0];
				}
				else
				{
					env_vol = 0;
				}
			}
			if (env_stage == 1)
			{
				if (env_length[1])
				{
					env_vol = 1.0f + (float)pow(1.0f - (float)env_time / env_length[1], 1.0f) * 2.0f * mParams.p_env_punch;
				}
				else
				{
					env_vol = 0;
				}
			}
			if (env_stage == 2)
			{
				if (env_length[2])
				{
					env_vol = 1.0f - (float)env_time / env_length[2];
				}
				else
				{
					env_vol = 0;
				}
			}

			// phaser step
			fphase += fdphase;
			iphase = abs((int)fphase);
			if (iphase > 1023) iphase = 1023;

			if (flthp_d != 0.0f)
			{
				flthp *= flthp_d;
				if (flthp < 0.00001f) flthp = 0.00001f;
				if (flthp > 0.1f) flthp = 0.1f;
			}

			float ssample = 0.0f;
			for (int si = 0; si < 8; si++) // 8x supersampling
			{
				float sample = 0.0f;
				phase++;
				if (phase >= period)
				{
					phase %= period;
					if (mParams.wave_type == 3)
					{
						for (int k = 0; k < 32; k++)
						{
							noise_buffer[k] = frnd(2.0f) - 1.0f;
						}
					}
				}
				// base waveform
				float fp = (float)phase / period;
				switch (mParams.wave_type)
				{
				case 0: // square
					if (fp < square_duty)
					{
						sample = 0.5f;
					}
					else
					{
						sample = -0.5f;
					}
					break;
				case 1: // sawtooth
					sample = 1.0f - fp * 2;
					break;
				case 2: // sine
					sample = (float)sin(fp * 2 * M_PI);
					break;
				case 3: // noise
					sample = noise_buffer[phase * 32 / period];
					break;
				}
				// lp filter
				float pp = fltp;
				fltw *= fltw_d;
				if (fltw < 0.0f) fltw = 0.0f;
				if (fltw > 0.1f) fltw = 0.1f;
				if (mParams.p_lpf_freq != 1.0f)
				{
					fltdp += (sample - fltp) * fltw;
					fltdp -= fltdp * fltdmp;
				}
				else
				{
					fltp = sample;
					fltdp = 0.0f;
				}
				fltp += fltdp;
				// hp filter
				fltphp += fltp - pp;
				fltphp -= fltphp * flthp;
				sample = fltphp;
				// phaser
				phaser_buffer[ipp & 1023] = sample;
				sample += phaser_buffer[(ipp - iphase + 1024) & 1023];
				ipp = (ipp + 1) & 1023;
				// final accumulation and envelope application
				ssample += sample*env_vol;
			}
			ssample = ssample / 8 * mParams.master_vol;

			ssample *= 2.0f * mParams.sound_vol;

			if (buffer != NULL)
			{
				if (ssample > 1.0f) ssample = 1.0f;
				if (ssample < -1.0f) ssample = -1.0f;
				*buffer = ssample;
				buffer++;
			}
		}
		return aSamplesToRead;
	}

	bool SfxrInstance::hasEnded()
	{
		return !playing_sample;
	}

	void SfxrInstance::resetSample(bool aRestart)
	{
		if(!aRestart)
			phase=0;
		fperiod=100.0/(mParams.p_base_freq*mParams.p_base_freq+0.001);
		period=(int)fperiod;
		fmaxperiod=100.0/(mParams.p_freq_limit*mParams.p_freq_limit+0.001);
		fslide=1.0-pow((double)mParams.p_freq_ramp, 3.0)*0.01;
		fdslide=-pow((double)mParams.p_freq_dramp, 3.0)*0.000001;
		square_duty=0.5f-mParams.p_duty*0.5f;
		square_slide=-mParams.p_duty_ramp*0.00005f;
		if(mParams.p_arp_mod>=0.0f)
			arp_mod=1.0-pow((double)mParams.p_arp_mod, 2.0)*0.9;
		else
			arp_mod=1.0+pow((double)mParams.p_arp_mod, 2.0)*10.0;
		arp_time=0;
		arp_limit=(int)(pow(1.0f-mParams.p_arp_speed, 2.0f)*20000+32);
		if(mParams.p_arp_speed==1.0f)
			arp_limit=0;
		if(!aRestart)
		{
			// reset filter
			fltp=0.0f;
			fltdp=0.0f;
			fltw=(float)pow(mParams.p_lpf_freq, 3.0f)*0.1f;
			fltw_d=1.0f+mParams.p_lpf_ramp*0.0001f;
			fltdmp=5.0f/(1.0f+(float)pow(mParams.p_lpf_resonance, 2.0f)*20.0f)*(0.01f+fltw);
			if(fltdmp>0.8f) fltdmp=0.8f;
			fltphp=0.0f;
			flthp=(float)pow(mParams.p_hpf_freq, 2.0f)*0.1f;
			flthp_d=(float)(1.0+mParams.p_hpf_ramp*0.0003f);
			// reset vibrato
			vib_phase=0.0f;
			vib_speed=(float)pow(mParams.p_vib_speed, 2.0f)*0.01f;
			vib_amp=mParams.p_vib_strength*0.5f;
			// reset envelope
			env_vol=0.0f;
			env_stage=0;
			env_time=0;
			env_length[0]=(int)(mParams.p_env_attack*mParams.p_env_attack*100000.0f);
			env_length[1]=(int)(mParams.p_env_sustain*mParams.p_env_sustain*100000.0f);
			env_length[2]=(int)(mParams.p_env_decay*mParams.p_env_decay*100000.0f);

			fphase=(float)pow(mParams.p_pha_offset, 2.0f)*1020.0f;
			if(mParams.p_pha_offset<0.0f) fphase=-fphase;
			fdphase=(float)pow(mParams.p_pha_ramp, 2.0f)*1.0f;
			if(mParams.p_pha_ramp<0.0f) fdphase=-fdphase;
			iphase=abs((int)fphase);
			ipp=0;
			for(int i=0;i<1024;i++)
				phaser_buffer[i]=0.0f;

			for(int i=0;i<32;i++)
				noise_buffer[i]=frnd(2.0f)-1.0f;

			rep_time=0;
			rep_limit=(int)(pow(1.0f-mParams.p_repeat_speed, 2.0f)*20000+32);
			if(mParams.p_repeat_speed==0.0f)
				rep_limit=0;
		}
	}


#define rnd(n) (mRand.rand()%((n)+1))
#undef frnd
#define frnd(x) ((float)(mRand.rand()%10001)/10000*(x))


	result Sfxr::loadPreset(int aPresetNo, int aRandSeed)
	{
		if (aPresetNo < 0 || aPresetNo > 6)
			return INVALID_PARAMETER;

		resetParams();
		mRand.srand(aRandSeed);
		switch(aPresetNo)
		{
		case 0: // pickup/coin
			mParams.p_base_freq=0.4f+frnd(0.5f);
			mParams.p_env_attack=0.0f;
			mParams.p_env_sustain=frnd(0.1f);
			mParams.p_env_decay=0.1f+frnd(0.4f);
			mParams.p_env_punch=0.3f+frnd(0.3f);
			if(rnd(1))
			{
				mParams.p_arp_speed=0.5f+frnd(0.2f);
				mParams.p_arp_mod=0.2f+frnd(0.4f);
			}
			break;
		case 1: // laser/shoot
			mParams.wave_type=rnd(2);
			if(mParams.wave_type==2 && rnd(1))
				mParams.wave_type=rnd(1);
			mParams.p_base_freq=0.5f+frnd(0.5f);
			mParams.p_freq_limit=mParams.p_base_freq-0.2f-frnd(0.6f);
			if(mParams.p_freq_limit<0.2f) mParams.p_freq_limit=0.2f;
			mParams.p_freq_ramp=-0.15f-frnd(0.2f);
			if(rnd(2)==0)
			{
				mParams.p_base_freq=0.3f+frnd(0.6f);
				mParams.p_freq_limit=frnd(0.1f);
				mParams.p_freq_ramp=-0.35f-frnd(0.3f);
			}
			if(rnd(1))
			{
				mParams.p_duty=frnd(0.5f);
				mParams.p_duty_ramp=frnd(0.2f);
			}
			else
			{
				mParams.p_duty=0.4f+frnd(0.5f);
				mParams.p_duty_ramp=-frnd(0.7f);
			}
			mParams.p_env_attack=0.0f;
			mParams.p_env_sustain=0.1f+frnd(0.2f);
			mParams.p_env_decay=frnd(0.4f);
			if(rnd(1))
				mParams.p_env_punch=frnd(0.3f);
			if(rnd(2)==0)
			{
				mParams.p_pha_offset=frnd(0.2f);
				mParams.p_pha_ramp=-frnd(0.2f);
			}
			if(rnd(1))
				mParams.p_hpf_freq=frnd(0.3f);
			break;
		case 2: // explosion
			mParams.wave_type=3;
			if(rnd(1))
			{
				mParams.p_base_freq=0.1f+frnd(0.4f);
				mParams.p_freq_ramp=-0.1f+frnd(0.4f);
			}
			else
			{
				mParams.p_base_freq=0.2f+frnd(0.7f);
				mParams.p_freq_ramp=-0.2f-frnd(0.2f);
			}
			mParams.p_base_freq*=mParams.p_base_freq;
			if(rnd(4)==0)
				mParams.p_freq_ramp=0.0f;
			if(rnd(2)==0)
				mParams.p_repeat_speed=0.3f+frnd(0.5f);
			mParams.p_env_attack=0.0f;
			mParams.p_env_sustain=0.1f+frnd(0.3f);
			mParams.p_env_decay=frnd(0.5f);
			if(rnd(1)==0)
			{
				mParams.p_pha_offset=-0.3f+frnd(0.9f);
				mParams.p_pha_ramp=-frnd(0.3f);
			}
			mParams.p_env_punch=0.2f+frnd(0.6f);
			if(rnd(1))
			{
				mParams.p_vib_strength=frnd(0.7f);
				mParams.p_vib_speed=frnd(0.6f);
			}
			if(rnd(2)==0)
			{
				mParams.p_arp_speed=0.6f+frnd(0.3f);
				mParams.p_arp_mod=0.8f-frnd(1.6f);
			}
			break;
		case 3: // powerup
			if(rnd(1))
				mParams.wave_type=1;
			else
				mParams.p_duty=frnd(0.6f);
			if(rnd(1))
			{
				mParams.p_base_freq=0.2f+frnd(0.3f);
				mParams.p_freq_ramp=0.1f+frnd(0.4f);
				mParams.p_repeat_speed=0.4f+frnd(0.4f);
			}
			else
			{
				mParams.p_base_freq=0.2f+frnd(0.3f);
				mParams.p_freq_ramp=0.05f+frnd(0.2f);
				if(rnd(1))
				{
					mParams.p_vib_strength=frnd(0.7f);
					mParams.p_vib_speed=frnd(0.6f);
				}
			}
			mParams.p_env_attack=0.0f;
			mParams.p_env_sustain=frnd(0.4f);
			mParams.p_env_decay=0.1f+frnd(0.4f);
			break;
		case 4: // hit/hurt
			mParams.wave_type=rnd(2);
			if(mParams.wave_type==2)
				mParams.wave_type=3;
			if(mParams.wave_type==0)
				mParams.p_duty=frnd(0.6f);
			mParams.p_base_freq=0.2f+frnd(0.6f);
			mParams.p_freq_ramp=-0.3f-frnd(0.4f);
			mParams.p_env_attack=0.0f;
			mParams.p_env_sustain=frnd(0.1f);
			mParams.p_env_decay=0.1f+frnd(0.2f);
			if(rnd(1))
				mParams.p_hpf_freq=frnd(0.3f);
			break;
		case 5: // jump
			mParams.wave_type=0;
			mParams.p_duty=frnd(0.6f);
			mParams.p_base_freq=0.3f+frnd(0.3f);
			mParams.p_freq_ramp=0.1f+frnd(0.2f);
			mParams.p_env_attack=0.0f;
			mParams.p_env_sustain=0.1f+frnd(0.3f);
			mParams.p_env_decay=0.1f+frnd(0.2f);
			if(rnd(1))
				mParams.p_hpf_freq=frnd(0.3f);
			if(rnd(1))
				mParams.p_lpf_freq=1.0f-frnd(0.6f);
			break;
		case 6: // blip/select
			mParams.wave_type=rnd(1);
			if(mParams.wave_type==0)
				mParams.p_duty=frnd(0.6f);
			mParams.p_base_freq=0.2f+frnd(0.4f);
			mParams.p_env_attack=0.0f;
			mParams.p_env_sustain=0.1f+frnd(0.1f);
			mParams.p_env_decay=frnd(0.2f);
			mParams.p_hpf_freq=0.1f;
			break;
		}
		return 0;
	}
	
	void Sfxr::resetParams()
	{
		mParams.wave_type=0;

		mParams.p_base_freq=0.3f;
		mParams.p_freq_limit=0.0f;
		mParams.p_freq_ramp=0.0f;
		mParams.p_freq_dramp=0.0f;
		mParams.p_duty=0.0f;
		mParams.p_duty_ramp=0.0f;

		mParams.p_vib_strength=0.0f;
		mParams.p_vib_speed=0.0f;
		mParams.p_vib_delay=0.0f;

		mParams.p_env_attack=0.0f;
		mParams.p_env_sustain=0.3f;
		mParams.p_env_decay=0.4f;
		mParams.p_env_punch=0.0f;

		mParams.filter_on=false;
		mParams.p_lpf_resonance=0.0f;
		mParams.p_lpf_freq=1.0f;
		mParams.p_lpf_ramp=0.0f;
		mParams.p_hpf_freq=0.0f;
		mParams.p_hpf_ramp=0.0f;
	
		mParams.p_pha_offset=0.0f;
		mParams.p_pha_ramp=0.0f;

		mParams.p_repeat_speed=0.0f;

		mParams.p_arp_speed=0.0f;
		mParams.p_arp_mod=0.0f;

		mParams.master_vol=0.05f;
		mParams.sound_vol=0.5f;
	}

	result Sfxr::loadParamsMem(unsigned char *aMem, unsigned int aLength, bool aCopy, bool aTakeOwnership)
	{
		MemoryFile mf;
		int res = mf.openMem(aMem, aLength, aCopy, aTakeOwnership);
		if (res != SO_NO_ERROR)
			return res;
		return loadParamsFile(&mf);
	}

	result Sfxr::loadParams(const char *aFilename)
	{
		DiskFile df;
		int res = df.open(aFilename);
		if (res != SO_NO_ERROR)
			return res;
		return loadParamsFile(&df);
	}

	result Sfxr::loadParamsFile(File *aFile)
	{
		int version=0;
		aFile->read((unsigned char*)&version, sizeof(int));
		if(version!=100 && version!=101 && version!=102)
		{
			return FILE_LOAD_FAILED;
		}

		aFile->read((unsigned char*)&mParams.wave_type, sizeof(int));


		mParams.sound_vol=0.5f;
		if(version==102)
			aFile->read((unsigned char*)&mParams.sound_vol, sizeof(float));

		aFile->read((unsigned char*)&mParams.p_base_freq, sizeof(float));
		aFile->read((unsigned char*)&mParams.p_freq_limit, sizeof(float));
		aFile->read((unsigned char*)&mParams.p_freq_ramp, sizeof(float));
		if(version>=101)
			aFile->read((unsigned char*)&mParams.p_freq_dramp, sizeof(float));
		aFile->read((unsigned char*)&mParams.p_duty, sizeof(float));
		aFile->read((unsigned char*)&mParams.p_duty_ramp, sizeof(float));

		aFile->read((unsigned char*)&mParams.p_vib_strength, sizeof(float));
		aFile->read((unsigned char*)&mParams.p_vib_speed, sizeof(float));
		aFile->read((unsigned char*)&mParams.p_vib_delay, sizeof(float));

		aFile->read((unsigned char*)&mParams.p_env_attack, sizeof(float));
		aFile->read((unsigned char*)&mParams.p_env_sustain, sizeof(float));
		aFile->read((unsigned char*)&mParams.p_env_decay, sizeof(float));
		aFile->read((unsigned char*)&mParams.p_env_punch, sizeof(float));

		aFile->read((unsigned char*)&mParams.filter_on, sizeof(bool));
		aFile->read((unsigned char*)&mParams.p_lpf_resonance, sizeof(float));
		aFile->read((unsigned char*)&mParams.p_lpf_freq, sizeof(float));
		aFile->read((unsigned char*)&mParams.p_lpf_ramp, sizeof(float));
		aFile->read((unsigned char*)&mParams.p_hpf_freq, sizeof(float));
		aFile->read((unsigned char*)&mParams.p_hpf_ramp, sizeof(float));
	
		aFile->read((unsigned char*)&mParams.p_pha_offset, sizeof(float));
		aFile->read((unsigned char*)&mParams.p_pha_ramp, sizeof(float));

		aFile->read((unsigned char*)&mParams.p_repeat_speed, sizeof(float));

		if(version>=101)
		{
			aFile->read((unsigned char*)&mParams.p_arp_speed, sizeof(float));
			aFile->read((unsigned char*)&mParams.p_arp_mod, sizeof(float));
		}

		return 0;
	}

	Sfxr::~Sfxr()
	{
		stop();
	}

	Sfxr::Sfxr()
	{
		resetParams();
		mBaseSamplerate = 44100;
	}


	AudioSourceInstance * Sfxr::createInstance() 
	{
		return new SfxrInstance(this);
	}

};