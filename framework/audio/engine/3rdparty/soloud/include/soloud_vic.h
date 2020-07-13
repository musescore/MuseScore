/*
SoLoud audio engine
Copyright (c) 2015 Jari Komppa

VIC 6560/6561 sound chip emulator
Copyright (c) 2015 Petri Hakkinen

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

#ifndef SOLOUD_VIC_H
#define SOLOUD_VIC_H

#include "soloud.h"

/*
A very bare bones emulator for Commodore VIC-20 sound chip. Supports both PAL and NTSC models.
Bass, alto and soprano should be quite close to original vic, noise probably not so.

The first three channels (bass, alto and soprano) are square waveform generators with 7-bit frequency.
The highest bit of each oscillator register switches the oscillator on/off.
The fourth oscillator generates a noise waveform.

VIC-20 does not have per channel volume control, only global volume,
which you can change by setting audio source's volume.

To get that authentic moldy VIC-20 sound, the audio source should be coupled with a biquad resonant filter
with the following params: type = LOWPASS, sample rate = 44100, frequency = 1500, resonance = 2.0.
*/

namespace SoLoud
{
	class Vic;

	class VicInstance : public AudioSourceInstance
	{
	public:
		VicInstance(Vic *aParent);
		~VicInstance();

		virtual unsigned int getAudio(float *aBuffer, unsigned int aSamplesToRead, unsigned int aBufferSize);
		virtual bool hasEnded();

	public:
		Vic*			m_parent;
		unsigned int	m_phase[4];
		unsigned int	m_noisePos;
	};

	class Vic : public AudioSource
	{
	public:
		// VIC model
		enum
		{
			PAL	= 0,
			NTSC
		};

		// VIC sound registers
		enum
		{
			BASS = 0,
			ALTO,
			SOPRANO,
			NOISE,
			MAX_REGS
		};

		Vic();

		virtual ~Vic();
		
		void setModel(int model);

		int getModel() const;

		void setRegister(int reg, unsigned char value);

		unsigned char getRegister(int reg);

	public:
		virtual AudioSourceInstance *createInstance();
		int				m_model;
		float			m_clocks[4];		// base clock frequencies for oscillators, dependent on VIC model
		unsigned char	m_regs[MAX_REGS];		
		unsigned char 	m_noise[8192];
	};
};

#endif
