/*
SoLoud audio engine
Copyright (c) 2013-2015 Jari Komppa

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

#include <math.h>
#include "soloud_internal.h"

// 3d audio operations

namespace SoLoud
{
	struct vec3
	{
		float mX, mY, mZ;

		bool null()
		{
			if (mX == 0 && mY == 0 && mZ == 0)
				return true;
			return false;
		}

		void neg()
		{
			mX = -mX;
			mY = -mY;
			mZ = -mZ;
		}

		float mag()
		{
			return (float)sqrt(mX * mX + mY * mY + mZ * mZ);
		}
		
		void normalize()
		{
			float m = mag();
			if (m == 0)
			{
				mX = mY = mZ = 0;
				return;
			}
			mX /= m;
			mY /= m;
			mZ /= m;
		}
		
		float dot(const vec3 &a)
		{
			return mX * a.mX + mY * a.mY + mZ * a.mZ;
		}
		
		vec3 sub(const vec3 &a)
		{
			vec3 r;
			r.mX = mX - a.mX;
			r.mY = mY - a.mY;
			r.mZ = mZ - a.mZ;
			return r;
		}

		vec3 cross(const vec3 &a)
		{
			vec3 r;

			r.mX = mY * a.mZ - a.mY * mZ;
			r.mY = mZ * a.mX - a.mZ * mX;
			r.mZ = mX * a.mY - a.mX * mY;

			return r;
		}
	};

	struct mat3
	{
		vec3 m[3];

		vec3 mul(const vec3 &a)
		{
			vec3 r;

			r.mX = m[0].mX * a.mX + m[0].mY * a.mY + m[0].mZ * a.mZ;
			r.mY = m[1].mX * a.mX + m[1].mY * a.mY + m[1].mZ * a.mZ;
			r.mZ = m[2].mX * a.mX + m[2].mY * a.mY + m[2].mZ * a.mZ;

			return r;
		}

		void lookatRH(const vec3 &at, vec3 up)
		{
			vec3 z = at;
			z.normalize();
			vec3 x = up.cross(z);
			x.normalize();
			vec3 y = z.cross(x);
			m[0] = x;
			m[1] = y;
			m[2] = z;
		}

		void lookatLH(const vec3 &at, vec3 up)
		{
			vec3 z = at;
			z.normalize();
			vec3 x = up.cross(z);
			x.normalize();
			vec3 y = z.cross(x);
			x.neg();  // flip x
			m[0] = x;
			m[1] = y;
			m[2] = z;
		}
	};

#ifndef MIN
#define MIN(a,b) ((a) < (b)) ? (a) : (b)
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b)) ? (a) : (b)
#endif

	float doppler(vec3 aDeltaPos, const vec3 &aSrcVel, const vec3 &aDstVel, float aFactor, float aSoundSpeed)
	{
		float deltamag = aDeltaPos.mag();
		if (deltamag == 0)
			return 1.0f;
		float vls = aDeltaPos.dot(aDstVel) / deltamag;
		float vss = aDeltaPos.dot(aSrcVel) / deltamag;
		float maxspeed = aSoundSpeed / aFactor;
		vss = MIN(vss, maxspeed);
		vls = MIN(vls, maxspeed);
		return (aSoundSpeed - aFactor * vls) / (aSoundSpeed - aFactor * vss);
	}

	float attenuateInvDistance(float aDistance, float aMinDistance, float aMaxDistance, float aRolloffFactor)
	{
		float distance = MAX(aDistance, aMinDistance);
		distance = MIN(distance, aMaxDistance);
		return aMinDistance / (aMinDistance + aRolloffFactor * (distance - aMinDistance));
	}

	float attenuateLinearDistance(float aDistance, float aMinDistance, float aMaxDistance, float aRolloffFactor)
	{
		float distance = MAX(aDistance, aMinDistance);
		distance = MIN(distance, aMaxDistance);
		return 1 - aRolloffFactor * (distance - aMinDistance) / (aMaxDistance - aMinDistance);
	}

	float attenuateExponentialDistance(float aDistance, float aMinDistance, float aMaxDistance, float aRolloffFactor)
	{
		float distance = MAX(aDistance, aMinDistance);
		distance = MIN(distance, aMaxDistance);
		return (float)pow(distance / aMinDistance, -aRolloffFactor);
	}

	void Soloud::update3dVoices_internal(unsigned int *aVoiceArray, unsigned int aVoiceCount)
	{
		vec3 speaker[MAX_CHANNELS];

		int i;
		for (i = 0; i < (signed)mChannels; i++)
		{
			speaker[i].mX = m3dSpeakerPosition[3 * i + 0];
			speaker[i].mY = m3dSpeakerPosition[3 * i + 1];
			speaker[i].mZ = m3dSpeakerPosition[3 * i + 2];
			speaker[i].normalize();
		}
		for (; i < MAX_CHANNELS; i++)
		{
			speaker[i].mX = 0;
			speaker[i].mY = 0;
			speaker[i].mZ = 0;
		}

		vec3 lpos, lvel, at, up;
		at.mX = m3dAt[0];
		at.mY = m3dAt[1];
		at.mZ = m3dAt[2];
		up.mX = m3dUp[0];
		up.mY = m3dUp[1];
		up.mZ = m3dUp[2];
		lpos.mX = m3dPosition[0];
		lpos.mY = m3dPosition[1];
		lpos.mZ = m3dPosition[2];
		lvel.mX = m3dVelocity[0];
		lvel.mY = m3dVelocity[1];
		lvel.mZ = m3dVelocity[2];
		mat3 m;
		if (mFlags & LEFT_HANDED_3D)
		{
			m.lookatLH(at, up);
		}
		else
		{
			m.lookatRH(at, up);
		}

		for (i = 0; i < (signed)aVoiceCount; i++)
		{
			AudioSourceInstance3dData * v = &m3dData[aVoiceArray[i]];

			float vol = 1;

			// custom collider
			if (v->mCollider)
			{
				vol *= v->mCollider->collide(this, v, v->mColliderData);
			}

			vec3 pos, vel;
			pos.mX = v->m3dPosition[0];
			pos.mY = v->m3dPosition[1];
			pos.mZ = v->m3dPosition[2];

			vel.mX = v->m3dVelocity[0];
			vel.mY = v->m3dVelocity[1];
			vel.mZ = v->m3dVelocity[2];

			if (!(v->mFlags & AudioSourceInstance::LISTENER_RELATIVE))
			{
				pos = pos.sub(lpos);
			}

			float dist = pos.mag();

			// attenuation

			if (v->mAttenuator)
			{
				vol *= v->mAttenuator->attenuate(dist, v->m3dMinDistance, v->m3dMaxDistance, v->m3dAttenuationRolloff);
			}
			else
			{
				switch (v->m3dAttenuationModel)
				{
				case AudioSource::INVERSE_DISTANCE:
					vol *= attenuateInvDistance(dist, v->m3dMinDistance, v->m3dMaxDistance, v->m3dAttenuationRolloff);
					break;
				case AudioSource::LINEAR_DISTANCE:
					vol *= attenuateLinearDistance(dist, v->m3dMinDistance, v->m3dMaxDistance, v->m3dAttenuationRolloff);
					break;
				case AudioSource::EXPONENTIAL_DISTANCE:
					vol *= attenuateExponentialDistance(dist, v->m3dMinDistance, v->m3dMaxDistance, v->m3dAttenuationRolloff);
					break;
				default:
					//case AudioSource::NO_ATTENUATION:
					break;
				}
			}

			// cone

			// (todo) vol *= conev;

			// doppler
			v->mDopplerValue = doppler(pos, vel, lvel, v->m3dDopplerFactor, m3dSoundSpeed);

			// panning
			pos = m.mul(pos);
			pos.normalize();

			// Apply volume to channels based on speaker vectors
			int j;
			for (j = 0; j < (signed)mChannels; j++)
			{
				float speakervol = (speaker[j].dot(pos) + 1) / 2;
				if (speaker[j].null())
					speakervol = 1;
				// Different speaker "focus" calculations to try, if the default "bleeds" too much..
				//speakervol = (speakervol * speakervol + speakervol) / 2;
				//speakervol = speakervol * speakervol;
				v->mChannelVolume[j] = vol * speakervol;
			}
			for (; j < MAX_CHANNELS; j++)
			{
				v->mChannelVolume[j] = 0;
			}

			v->m3dVolume = vol;
		}
	}

	void Soloud::update3dAudio()
	{
		unsigned int voicecount = 0;
		unsigned int voices[VOICE_COUNT];

		// Step 1 - find voices that need 3d processing
		lockAudioMutex_internal();
		int i;
		for (i = 0; i < (signed)mHighestVoice; i++)
		{
			if (mVoice[i] && mVoice[i]->mFlags & AudioSourceInstance::PROCESS_3D)
			{
				voices[voicecount] = i;
				voicecount++;
				m3dData[i].mFlags = mVoice[i]->mFlags;
			}
		}
		unlockAudioMutex_internal();

		// Step 2 - do 3d processing

		update3dVoices_internal(voices, voicecount);

		// Step 3 - update SoLoud voices

		lockAudioMutex_internal();
		for (i = 0; i < (int)voicecount; i++)
		{
			AudioSourceInstance3dData * v = &m3dData[voices[i]];
			AudioSourceInstance * vi = mVoice[voices[i]];
			if (vi)
			{
				updateVoiceRelativePlaySpeed_internal(voices[i]);
				updateVoiceVolume_internal(voices[i]);
				int j;
				for (j = 0; j < MAX_CHANNELS; j++)
				{
					vi->mChannelVolume[j] = v->mChannelVolume[j];
				}

				if (vi->mOverallVolume < 0.001f)
				{
					// Inaudible.
					vi->mFlags |= AudioSourceInstance::INAUDIBLE;

					if (vi->mFlags & AudioSourceInstance::INAUDIBLE_KILL)
					{
						stopVoice_internal(voices[i]);
					}
				}
				else
				{
					vi->mFlags &= ~AudioSourceInstance::INAUDIBLE;
				}
			}
		}

		mActiveVoiceDirty = true;
		unlockAudioMutex_internal();
	}


	handle Soloud::play3d(AudioSource &aSound, float aPosX, float aPosY, float aPosZ, float aVelX, float aVelY, float aVelZ, float aVolume, bool aPaused, unsigned int aBus)
	{
		handle h = play(aSound, aVolume, 0, 1, aBus);
		lockAudioMutex_internal();
		int v = getVoiceFromHandle_internal(h);
		if (v < 0) 
		{
			unlockAudioMutex_internal();
			return h;
		}
		m3dData[v].mHandle = h;
		mVoice[v]->mFlags |= AudioSourceInstance::PROCESS_3D;
		set3dSourceParameters(h, aPosX, aPosY, aPosZ, aVelX, aVelY, aVelZ);

		int samples = 0;
		if (aSound.mFlags & AudioSource::DISTANCE_DELAY)
		{
			vec3 pos;
			pos.mX = aPosX;
			pos.mY = aPosY;
			pos.mZ = aPosZ;
			if (!(mVoice[v]->mFlags & AudioSource::LISTENER_RELATIVE))
			{
				pos.mX -= m3dPosition[0];
				pos.mY -= m3dPosition[1];
				pos.mZ -= m3dPosition[2];
			}
			float dist = pos.mag();
			samples += (int)floor((dist / m3dSoundSpeed) * mSamplerate);
		}

		update3dVoices_internal((unsigned int *)&v, 1);
		updateVoiceRelativePlaySpeed_internal(v);
		int j;
		for (j = 0; j < MAX_CHANNELS; j++)
		{
			mVoice[v]->mChannelVolume[j] = m3dData[v].mChannelVolume[j];
		}

		updateVoiceVolume_internal(v);
		
		// Fix initial voice volume ramp up
		int i;
		for (i = 0; i < MAX_CHANNELS; i++)
		{
			mVoice[v]->mCurrentChannelVolume[i] = mVoice[v]->mChannelVolume[i] * mVoice[v]->mOverallVolume;
		}

		if (mVoice[v]->mOverallVolume < 0.01f)
		{
			// Inaudible.
			mVoice[v]->mFlags |= AudioSourceInstance::INAUDIBLE;

			if (mVoice[v]->mFlags & AudioSourceInstance::INAUDIBLE_KILL)
			{
				stopVoice_internal(v);
			}
		}
		else
		{
			mVoice[v]->mFlags &= ~AudioSourceInstance::INAUDIBLE;
		}
		mActiveVoiceDirty = true;

		unlockAudioMutex_internal();
		setDelaySamples(h, samples);
		setPause(h, aPaused);
		return h;
	}

	handle Soloud::play3dClocked(time aSoundTime, AudioSource &aSound, float aPosX, float aPosY, float aPosZ, float aVelX, float aVelY, float aVelZ, float aVolume, unsigned int aBus)
	{
		handle h = play(aSound, aVolume, 0, 1, aBus);
		lockAudioMutex_internal();
		int v = getVoiceFromHandle_internal(h);
		if (v < 0) 
		{
			unlockAudioMutex_internal();
			return h;
		}
		m3dData[v].mHandle = h;
		mVoice[v]->mFlags |= AudioSourceInstance::PROCESS_3D;
		set3dSourceParameters(h, aPosX, aPosY, aPosZ, aVelX, aVelY, aVelZ);
		time lasttime = mLastClockedTime;
		if (lasttime == 0)
		{
			lasttime = aSoundTime;
			mLastClockedTime = aSoundTime;
		}
		vec3 pos;
		pos.mX = aPosX;
		pos.mY = aPosY;
		pos.mZ = aPosZ;
		unlockAudioMutex_internal();
		
		int samples = (int)floor((aSoundTime - lasttime) * mSamplerate);		
		// Make sure we don't delay too much (or overflow)
		if (samples < 0 || samples > 2048) samples = 0;

		if (aSound.mFlags & AudioSource::DISTANCE_DELAY)
		{
			float dist = pos.mag();
			samples += (int)floor((dist / m3dSoundSpeed) * mSamplerate);
		}

		update3dVoices_internal((unsigned int *)&v, 1);
		lockAudioMutex_internal();
		updateVoiceRelativePlaySpeed_internal(v);
		int j;
		for (j = 0; j < MAX_CHANNELS; j++)
		{
			mVoice[v]->mChannelVolume[j] = m3dData[v].mChannelVolume[j];
		}

		updateVoiceVolume_internal(v);

		// Fix initial voice volume ramp up
		int i;
		for (i = 0; i < MAX_CHANNELS; i++)
		{
			mVoice[v]->mCurrentChannelVolume[i] = mVoice[v]->mChannelVolume[i] * mVoice[v]->mOverallVolume;
		}

		if (mVoice[v]->mOverallVolume < 0.01f)
		{
			// Inaudible.
			mVoice[v]->mFlags |= AudioSourceInstance::INAUDIBLE;

			if (mVoice[v]->mFlags & AudioSourceInstance::INAUDIBLE_KILL)
			{
				stopVoice_internal(v);
			}
		}
		else
		{
			mVoice[v]->mFlags &= ~AudioSourceInstance::INAUDIBLE;
		}
		mActiveVoiceDirty = true;
		unlockAudioMutex_internal();

		setDelaySamples(h, samples);
		setPause(h, 0);
		return h;
	}


	
	result Soloud::set3dSoundSpeed(float aSpeed)
	{
		if (aSpeed <= 0)
			return INVALID_PARAMETER;
		m3dSoundSpeed = aSpeed;
		return SO_NO_ERROR;
	}

	
	float Soloud::get3dSoundSpeed()
	{
		return m3dSoundSpeed;
	}

	
	void Soloud::set3dListenerParameters(float aPosX, float aPosY, float aPosZ, float aAtX, float aAtY, float aAtZ, float aUpX, float aUpY, float aUpZ, float aVelocityX, float aVelocityY, float aVelocityZ)
	{
		m3dPosition[0] = aPosX;
		m3dPosition[1] = aPosY;
		m3dPosition[2] = aPosZ;
		m3dAt[0] = aAtX;
		m3dAt[1] = aAtY;
		m3dAt[2] = aAtZ;
		m3dUp[0] = aUpX;
		m3dUp[1] = aUpY;
		m3dUp[2] = aUpZ;
		m3dVelocity[0] = aVelocityX;
		m3dVelocity[1] = aVelocityY;
		m3dVelocity[2] = aVelocityZ;
	}

	
	void Soloud::set3dListenerPosition(float aPosX, float aPosY, float aPosZ)
	{
		m3dPosition[0] = aPosX;
		m3dPosition[1] = aPosY;
		m3dPosition[2] = aPosZ;
	}

	
	void Soloud::set3dListenerAt(float aAtX, float aAtY, float aAtZ)
	{
		m3dAt[0] = aAtX;
		m3dAt[1] = aAtY;
		m3dAt[2] = aAtZ;
	}

	
	void Soloud::set3dListenerUp(float aUpX, float aUpY, float aUpZ)
	{
		m3dUp[0] = aUpX;
		m3dUp[1] = aUpY;
		m3dUp[2] = aUpZ;
	}

	
	void Soloud::set3dListenerVelocity(float aVelocityX, float aVelocityY, float aVelocityZ)
	{
		m3dVelocity[0] = aVelocityX;
		m3dVelocity[1] = aVelocityY;
		m3dVelocity[2] = aVelocityZ;
	}

	
	void Soloud::set3dSourceParameters(handle aVoiceHandle, float aPosX, float aPosY, float aPosZ, float aVelocityX, float aVelocityY, float aVelocityZ)
	{
		FOR_ALL_VOICES_PRE_3D
			m3dData[ch].m3dPosition[0] = aPosX;
			m3dData[ch].m3dPosition[1] = aPosY;
			m3dData[ch].m3dPosition[2] = aPosZ;
			m3dData[ch].m3dVelocity[0] = aVelocityX;
			m3dData[ch].m3dVelocity[1] = aVelocityY;
			m3dData[ch].m3dVelocity[2] = aVelocityZ;
		FOR_ALL_VOICES_POST_3D
	}

	
	void Soloud::set3dSourcePosition(handle aVoiceHandle, float aPosX, float aPosY, float aPosZ)
	{
		FOR_ALL_VOICES_PRE_3D
			m3dData[ch].m3dPosition[0] = aPosX;
			m3dData[ch].m3dPosition[1] = aPosY;
			m3dData[ch].m3dPosition[2] = aPosZ;
		FOR_ALL_VOICES_POST_3D
	}

	
	void Soloud::set3dSourceVelocity(handle aVoiceHandle, float aVelocityX, float aVelocityY, float aVelocityZ)
	{
		FOR_ALL_VOICES_PRE_3D
			m3dData[ch].m3dVelocity[0] = aVelocityX;
			m3dData[ch].m3dVelocity[1] = aVelocityY;
			m3dData[ch].m3dVelocity[2] = aVelocityZ;
		FOR_ALL_VOICES_POST_3D
	}

	
	void Soloud::set3dSourceMinMaxDistance(handle aVoiceHandle, float aMinDistance, float aMaxDistance)
	{
		FOR_ALL_VOICES_PRE_3D
			m3dData[ch].m3dMinDistance = aMinDistance;
			m3dData[ch].m3dMaxDistance = aMaxDistance;
		FOR_ALL_VOICES_POST_3D
	}

	
	void Soloud::set3dSourceAttenuation(handle aVoiceHandle, unsigned int aAttenuationModel, float aAttenuationRolloffFactor)
	{
		FOR_ALL_VOICES_PRE_3D
			m3dData[ch].m3dAttenuationModel = aAttenuationModel;
			m3dData[ch].m3dAttenuationRolloff = aAttenuationRolloffFactor;
		FOR_ALL_VOICES_POST_3D
	}

	
	void Soloud::set3dSourceDopplerFactor(handle aVoiceHandle, float aDopplerFactor)
	{
		FOR_ALL_VOICES_PRE_3D
			m3dData[ch].m3dDopplerFactor = aDopplerFactor;
		FOR_ALL_VOICES_POST_3D
	}
};
