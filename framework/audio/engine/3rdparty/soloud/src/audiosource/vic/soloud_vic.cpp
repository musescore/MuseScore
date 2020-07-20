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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "soloud_vic.h"

namespace SoLoud
{

	VicInstance::VicInstance(Vic *aParent)
	{
		m_parent = aParent;

		for(int i = 0; i < 4; i++)
			m_phase[i] = 0;

		m_noisePos = 0;
	}

	VicInstance::~VicInstance()
	{
	}

	unsigned int VicInstance::getAudio(float *aBuffer, unsigned int aSamplesToRead, unsigned int /*aBufferSize*/)
	{
		unsigned int phaseAdder[4] = { 0, 0, 0, 0 };
		for(int i = 0; i < 4; i++)
		{
			unsigned char reg = m_parent->getRegister(i);
			if(reg >= 128)
			{
				float freq = m_parent->m_clocks[i] / (float)(reg < 255 ? 255 - reg : 1);
				phaseAdder[i] = (unsigned int)(freq * 65536.0f / 44100.0f + 0.5f);
			}
		}

		for(int i = 0; i < (signed)aSamplesToRead; i++)
		{
			float s = 0.0f;

			// square waves
			for(int v = 0; v < 3; v++)
			{
				if(phaseAdder[v] != 0)
				{
					s += (m_phase[v] < 32768 ? 0.5f : -0.5f);
					m_phase[v] = (m_phase[v] + phaseAdder[v]) & 65535;
				}
			}

			// noise
			if(phaseAdder[3] != 0)
			{
				s += (float)m_parent->m_noise[m_noisePos] / 255.0f - 0.5f;

				m_phase[3] += phaseAdder[3];

				if(m_phase[3] >= 32768)
				{
					m_noisePos = (m_noisePos + 1) & 8191;
					m_phase[3] &= 32767;
				}
			}

			aBuffer[i] = s / 4.0f;
		}
		return aSamplesToRead;
	}

	bool VicInstance::hasEnded()
	{
		return false;
	}

	Vic::Vic()
	{
		mBaseSamplerate = 44100;
		setModel(PAL);

		for(int i = 0; i < MAX_REGS; i++)
			m_regs[i] = 0;

		// Galois LFSR (source: https://en.wikipedia.org/wiki/Linear_feedback_shift_register)
	    unsigned short lfsr = 0xACE1u;
		for(int i = 0; i < 8192; i++)
		{
		    unsigned lsb = lfsr & 1;
		    lfsr >>= 1;
		    lfsr ^= (unsigned)(-(signed)lsb) & 0xB400u;
		    m_noise[i] = (lfsr & 0xff) ^ (lfsr >> 8);
		}
	}

	Vic::~Vic()
	{
		stop();
	}

	void Vic::setModel(int model)
	{
		m_model = model;

		switch(model)
		{
		case PAL:
			m_clocks[0] = 4329.0f;
			m_clocks[1] = 8659.0f;
			m_clocks[2] = 17320.0f;
			m_clocks[3] = 34640.0f;
			break;

		case NTSC:
			m_clocks[0] = 3995.0f;
			m_clocks[1] = 7990.0f;
			m_clocks[2] = 15980.0f;
			m_clocks[3] = 31960.0f;
			break;
		}
	}

	int Vic::getModel() const
	{
		return m_model;
	}

	void Vic::setRegister(int reg, unsigned char value) 
	{ 
		m_regs[reg] = value; 
	}
	
	unsigned char Vic::getRegister(int reg)
	{ 
		return m_regs[reg]; 
	}

	AudioSourceInstance * Vic::createInstance() 
	{
		return new VicInstance(this);
	}

};