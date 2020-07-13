//  Issues:
//  - Filter cutoff frequencies not 100% accurate
//  - Combined waveforms of the 6581 incorrect (SID card used 8580 anyway)
//  - filter distortion not emulated
//  - no joystick or paddle support
//  - probably many more

#include <math.h>
#ifndef __vita__
#include <memory.h>
#endif
#include "sid.h"
//#include "Tedmem.h"

#define DIGIBLASTER_MULT 14

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

// Hack to store master volume
//unsigned int SIDsound::masterVolume = 0;

//
//	Random number generator for noise waveform
//

// Test a bit. Returns 1 if bit is set.
inline static long bit(long val, unsigned int bitnr) 
{
	return (val >> bitnr) & 1;
}

inline void SIDsound::updateShiftReg(SIDVoice &v)
{
	unsigned int shiftReg = v.shiftReg;
	unsigned int bit22 = bit(shiftReg,22);
	unsigned int bit17 = bit(shiftReg,17);

	// Shift 1 bit left
	shiftReg = ((shiftReg) << 1);// & 0x7fffff;

	// Feed bit 0 
	v.shiftReg = shiftReg | (bit22 ^ bit17);
}

inline int SIDsound::waveNoise(SIDVoice &v)
{
	unsigned int shiftReg = v.shiftReg;
	// Pick out bits to make output value, left shift by 4
	return 
		(bit(shiftReg,22) << 11) |
		(bit(shiftReg,20) << 10) |
		(bit(shiftReg,16) << 9) |
		(bit(shiftReg,13) << 8) |
		(bit(shiftReg,11) << 7) |
		(bit(shiftReg, 7) << 6) |
		(bit(shiftReg, 4) << 5) |
		(bit(shiftReg, 2) << 4);
};

void SIDsound::setModel(unsigned int model) 
{
	int i;

	switch (model) {
		case SID8580DB:
		case SID8580:
			for ( i=0; i<2048; i++) {
				double x = i / 8.0;
				//double cf = 12500.0 * i / 2048.0; // specs and YAPE
				// approximate with a 3-degree polynomial
				//double cf = 0.0003*x*x*x + 0.0882*x*x + 44.49*x - 38.409;
				// approximate with a 2-degree polynomial
				//double cf = -0.0177*x*x + 55.261*x - 55.518; // CSG 8580R4
				double cf = -0.0156*x*x + 48.473*x - 45.074; // 8580R5
				cutOffFreq[i] = cf <= 0 ? 0 : cf;
			}
			dcWave = 0x800;
			dcMixer = 0;
			dcVoice = 0;
			break;

		case SID6581: // R4 actually
			for (i=0; i<1024; i++) {
				cutOffFreq[i] = (tanh(((double)i/1.5 - 1024.0)/1024.0*M_PI) + tanh(M_PI))
					* (6000.0 - 220.0) + 220.0;
			}
			for (; i<1056; i++) {
				double x = ((double)i - 1024.0) / (1056.0 - 1003.);
				cutOffFreq[i] = x*(1315.0 - 1003.0) + 1003.0;
			}
			for (; i<2048; i++) {
				double x = ((double)i - 1056.0) / (2048.0 - 1056.0);
				cutOffFreq[i] = //(tanh (((double)i - 2048.0)/1024.0*M_PI) + tanh(M_PI))
					//* (20163.0 - 1315.0) + 1315.0;
					(20163.0 - 1315.0) * x + 1315.0;
			}
			dcWave = 0x380;
			dcMixer = -0xFFF*0xFF/18 >> 7;
			dcVoice = 0x800*0xFF;
			break;

		case SID6581R1: // 6581 R1
			for (i=0; i<1024; i++) { 
				cutOffFreq[i] = (tanh(((double)i-1024.0)/1024.0*M_PI) + tanh(M_PI))
					* (6000.0 - 220.0) + 220.0;
			}
			for (; i<2048; i++) { 
				cutOffFreq[i] = (tanh (((double)i-2048.0)/1024.0*M_PI) + tanh(M_PI))
					* (18000.0 - 4600.0) + 4600.0;
			}
			dcWave = 0x380;
			dcMixer = -0xFFF*0xFF/18 >> 7;
			dcVoice = 0x800*0xFF;
			break;
	}
	setFilterCutoff();
	model_ = model;
}

// Static data members
const unsigned int SIDsound::RateCountPeriod[16] = {
	0x7F00,0x0006,0x003C,0x0330,0x20C0,0x6755,0x3800,0x500E,
	0x1212,0x0222,0x1848,0x59B8,0x3840,0x77E2,0x7625,0x0A93
};

const unsigned char SIDsound::envGenDRdivisors[256] = {
	30,30,30,30,30,30,16,16,16,16,16,16,16,16,8,8, 
	8,8,8,8,8,8,8,8,8,8,4,4,4,4,4,4,
	4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
	4,4,4,4,4,4,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

void SIDsound::calcEnvelopeTable()
{
	// number of SIDsound envelope clocks per sample (0x1FFFFF)
	const double deltaSampleCyclesFloat = ((double) sidBaseFreq * 256.0) / (double)sampleRate;
	sidCyclesPerSampleInt = (unsigned int) (deltaSampleCyclesFloat + 0.5);
}

void SIDsound::setFrequency(unsigned int sid_frequency)
{
	switch (sid_frequency) {
		case 0:
			sidBaseFreq = TED_SOUND_CLOCK * 4; // 312 * 114 * 50 / 2;
			break;
		default:
			sidBaseFreq = SOUND_FREQ_PAL_C64;
			break;
	}
	calcEnvelopeTable();
}

void SIDsound::setSampleRate(unsigned int sampleRate_)
{
	sampleRate = sampleRate_;
	calcEnvelopeTable();
}

SIDsound::SIDsound(unsigned int model, unsigned int chnlDisableMask) : enableDigiBlaster(false), sampleRate(0)
{
	unsigned int i;
	masterVolume = 0;

	// Link voices together
	for (i=0; i<3; i++) {
		voice[i].modulatedBy = &voice[(i+2)%3]; // previous voice
		voice[i].modulatesThis = &voice[(i+1)%3]; // next voice
		voice[i].disabled = !!((chnlDisableMask >> i) & 1);
	}

	filterCutoff = 0;
	setModel(model);
	setFrequency(0);
	reset();
}

void SIDsound::reset(void)
{
	volume = masterVolume;

	lastByteWritten = 0;

	for (int v=0; v<3; v++) {
		voice[v].wave = WAVE_NONE;
		voice[v].egState = EG_FROZEN;
		voice[v].accu = voice[v].add = 0;
		voice[v].freq = voice[v].pw = 0;
		voice[v].envCurrLevel = voice[v].envSustainLevel = 0;
		voice[v].gate = voice[v].ring = voice[v].test = 0;
		voice[v].filter = voice[v].sync = 0;
		voice[v].muted = 0;
		// Initial value of internal shift register
		voice[v].shiftReg = 0x7FFFFC;
		voice[v].envExpCounter = 0;
		voice[v].envAttackAdd = voice[v].envDecaySub = voice[v].envReleaseSub = 0;
		voice[v].envCounterCompare = RateCountPeriod[0];
		voice[v].envCounter = 0x7fff;
	}

	filterType = FILTER_NONE;
	filterCutoff = filterResonance = 0;

	Vhp = Vbp = Vlp = 0;
	setFilterCutoff();
	setResonance();

	dcDigiBlaster = 0;
	clockDeltaRemainder = 0;
}

inline int SIDsound::getWaveSample(SIDVoice &v)
{
	switch (v.wave) {
		case WAVE_TRI:
			return waveTriangle(v);
		case WAVE_SAW:
			return waveSaw(v);
		case WAVE_PULSE:
			return wavePulse(v);
		case WAVE_TRISAW:
			return waveTriSaw(v);
		case WAVE_TRIPULSE:
			return waveTriPulse(v);
		case WAVE_SAWPULSE:
			return waveSawPulse(v);
		case WAVE_TRISAWPULSE:
			return waveTriSawPulse(v);
		case WAVE_NOISE:
			return waveNoise(v);
		default:
			return 0x800;
	}
}

unsigned char SIDsound::read(unsigned int adr)
{
	switch(adr) {
		case 0x19:
		case 0x1A:
			// POTX/POTY paddle AD converters (unemulated)
			lastByteWritten = 0;
			return 0xFF;

		// Voice 3 (only) oscillator readout
		case 0x1B:
			lastByteWritten = 0;
			return (unsigned char)(getWaveSample(voice[2])>>0); // 4?

		// Voice 3 EG readout
		case 0x1C:
			return (unsigned char)(voice[2].envCurrLevel);

		case 0x1E: // Digiblaster DAC readout
			if (enableDigiBlaster && model_ == SID8580)
			{
				return (unsigned char) (dcDigiBlaster >> DIGIBLASTER_MULT);
			}
			return lastByteWritten;

		default:
			// Write-only registers return the last value written
			return lastByteWritten;
	}
}

void SIDsound::write(unsigned int adr, unsigned char value)
{	
	lastByteWritten = value;

	SIDVoice &v = voice[adr/7];
	switch (adr) {
		case 0:
		case 7:
		case 14:
			v.freq = (unsigned short)((v.freq & 0xff00) | value);
			v.add = (unsigned int)(((double)v.freq 
				* sidBaseFreq) * 16.0 / sampleRate + 0.5);
			break;

		case 1:
		case 8:
		case 15:
			v.freq = (unsigned short)((v.freq & 0xff) | (value << 8));
			v.add = (unsigned int)(((double)v.freq 
				* sidBaseFreq) * 16.0 / sampleRate + 0.5);
			break;

		case 2:
		case 9:
		case 16:
			v.pw = (unsigned short)((v.pw & 0x0f00) | value);
			break;

		case 3:
		case 10:
		case 17:
			v.pw = (unsigned short)((v.pw & 0xff) | ((value & 0xf) << 8));
			break;

		case 4:
		case 11:
		case 18:
			if ((value & 1) != (unsigned char) v.gate) {
				if (value & 1) {
					// gate on
					v.egState = EG_ATTACK;
					v.envCounterCompare = v.envAttackAdd;
				} else {
					// gate off
#if 00
					if (v.egState != EG_FROZEN)
#endif
						v.egState = EG_RELEASE;
					v.envCounterCompare = v.envReleaseSub;
				}
				v.gate = value & 1;
			}
			v.modulatedBy->sync = value & 2;
			v.ring = value & 4;
			if ((value & 8) && !v.test) {
				v.accu = 0; //(model_ >= SID8580) ? 0 : 0;
				unsigned int bit19 = (v.shiftReg >> 19) & 1;
				v.shiftReg = (v.shiftReg & 0x7ffffd) | ((bit19^1) << 1);
				v.test = 0xFFF;
			} else if (v.test && !(value & 8)) {
				unsigned int bit0 = ((v.shiftReg >> 22) ^ (v.shiftReg >> 17)) & 0x1;
				v.shiftReg <<= 1;
				v.shiftReg &= 0x7fffff;
				v.shiftReg |= bit0;
				v.test = 0x000;
			}
			v.wave = (value >> 4) & 0x0F;
			if (v.wave > 8) {
				v.shiftReg &= 0x7fffff^(1<<22)^(1<<20)^(1<<16)^(1<<13)^(1<<11)^(1<<7)^(1<<4)^(1<<2);
			}
			break;

		case 5:
		case 12:
		case 19:
			v.envAttackAdd = value >> 4;
			v.envDecaySub = value & 0x0F;
			if (v.egState == EG_ATTACK)
				v.envCounterCompare = v.envAttackAdd;
			else if (v.egState == EG_DECAY)
				v.envCounterCompare = v.envDecaySub;
			break;

		case 6:
		case 13:
		case 20:
			v.envSustainLevel = (value >> 4) * 0x11;
			v.envReleaseSub = value & 0x0F;
			if (v.egState == EG_RELEASE)
				v.envCounterCompare = v.envReleaseSub;
			break;

		case 21:
			if ((unsigned int)(value&7) != (filterCutoff&7)) {
				filterCutoff = (value&7)|(filterCutoff&0x7F8);
				setFilterCutoff();
			}
			break;

		case 22:
			filterCutoff = (value<<3)|(filterCutoff&7);
			setFilterCutoff();
			break;

		case 23:
			voice[0].filter = value & 1;
			voice[1].filter = value & 2;
			voice[2].filter = value & 4;
			filterResonance = (unsigned char)(value >> 4);
			setResonance();
			break;

		case 24:
			volume = value & 0x0F;
			voice[2].muted = value & 0x80;
			filterType = (unsigned char)((value >> 4) & 7);
			break;

		case 30: // Digiblaster DAC
			if (enableDigiBlaster && model_ == SID8580)
			{
				dcDigiBlaster = (value ^ 0x00) << DIGIBLASTER_MULT;
			}
			break;

		case 31: // Digiblaster ADC
			break;
	}
}

inline void SIDsound::setFilterCutoff()
{
	const double freqDomainDivCoeff = 2 * M_PI * 1.048576;
	w0 = int(cutOffFreq[filterCutoff] * freqDomainDivCoeff);
	// Limit cutoff to Nyquist frq to keep the sample based filter stable
	const double NyquistFrq = double(sampleRate) / 2;
	const double maxCutOff = NyquistFrq > 16000.0 ? 16000.0 : NyquistFrq;
	const int w0MaxDt = int(maxCutOff * freqDomainDivCoeff); // 16000
	if (w0 > w0MaxDt) w0 = w0MaxDt;
}

inline void SIDsound::setResonance()
{
	resonanceCoeffDiv1024 = (int) (1024.0/(0.707 + 1.9 * (double) filterResonance / 15.0) + 0.5); // 2.3
}

inline unsigned int SIDsound::clock()
{
	unsigned int count = sidCyclesPerSampleInt >> 8;
	unsigned int tmp = sidCyclesPerSampleInt & 0xFF;
	unsigned int newCount = clockDeltaRemainder + tmp;
	
	if (newCount >= 0x100) {
		clockDeltaRemainder = newCount & 0xFF;
		count++;
	} else {
		clockDeltaRemainder = newCount;
	}
	return count;
}

// simplified version of http://bel.fi/~alankila/c64-sw/index-cpp.html
inline int SIDsound::filterOutput(unsigned int cycles, int Vi)
{
	int w0deltaTime = w0 >> 6;
	Vi >>= 7;
	unsigned int count = cycles;

	do {
		int dVlp = (w0deltaTime * Vbp >> 14);
		Vlp -= dVlp;
		int dVbp = (w0deltaTime * Vhp >> 14);
		Vbp -= dVbp;
		Vhp = (Vbp * resonanceCoeffDiv1024 >> 10) - Vlp - Vi;
	} while (--count);

	int Vf;

	switch (filterType) {
		default:
		case FILTER_NONE:
			Vf = 0;
			break;
		case FILTER_LP:
			Vf = Vlp;
			break;
		case FILTER_BP:
			Vf = Vbp;
			break;
		case FILTER_LPBP:
			Vf = Vlp + Vbp;
			break;
		case FILTER_HP:
			Vf = Vhp;
			break;
		case FILTER_NOTCH:
			Vf = Vlp + Vhp;
			break;
		case FILTER_HPBP:
			Vf = Vbp + Vhp;
			break;
		case FILTER_ALL:
			Vf = Vlp + Vbp + Vhp;
			break;
	}
	return Vf << 7;
}

// Envelope based on:
// http://blog.kevtris.org/?p=13
inline int SIDsound::doEnvelopeGenerator(unsigned int cycles, SIDVoice &v)
{
	unsigned int count = cycles;

	do {
		unsigned int LFSR = v.envCounter;
		if (LFSR != RateCountPeriod[v.envCounterCompare]) {
			const unsigned int feedback = ((LFSR >> 14) ^ (LFSR >> 13)) & 1;
			LFSR = ((LFSR << 1) | feedback) & 0x7FFF;
			v.envCounter = LFSR;
		} else {
			// LFSR = 0x7fff reset LFSR
			v.envCounter = 0x7fff;

			if (v.egState == EG_ATTACK || ++v.envExpCounter == envGenDRdivisors[v.envCurrLevel]) {

				v.envExpCounter = 0;

				switch (v.egState) {

				case EG_ATTACK:
					// According to Bob Yannes, Attack is linear...
					if ( ((++v.envCurrLevel) & 0xFF) == 0xFF) {
						v.egState = EG_DECAY;
						v.envCounterCompare = v.envDecaySub;
					}
					break;

				case EG_DECAY:
					if (v.envCurrLevel != v.envSustainLevel) {
						v.envCurrLevel--;
						v.envCurrLevel &= 0xFF;
						if (!v.envCurrLevel)
							v.egState = EG_FROZEN;
					}
					break;

				case EG_RELEASE:
					v.envCurrLevel = (v.envCurrLevel - 1) & 0xFF;
					if (!v.envCurrLevel)
						v.egState = EG_FROZEN;
					break;

				case EG_FROZEN:
					v.envCurrLevel = 0;
					break;
				}
			}
		}
	} while (--count);

	return v.envCurrLevel & 0xFF; // envelope is 8 bits
}

void SIDsound::calcSamples(short *buf, long accu)
{
	for (;accu--;) {
		// Outputs for normal and filtered sounds
		int sumFilteredOutput = 0;
		int sumOutput = 0;
		int sample;

		const unsigned int cyclesToDo = clock();
		// Loop for the three voices
		int j = 2;
		do {
			SIDVoice &v = voice[j];
			int envelope = doEnvelopeGenerator(cyclesToDo, v);
			// Waveform generator
			if (!v.test) {
#if 1
				unsigned int accPrev = v.accu;
				// Update accumulator
				v.accu += v.add;
				// FIXME Apply ring modulation.
				if (v.sync && !(accPrev & 0x8000000) && (v.accu & 0x8000000)
           			)
#else
				v.accPrev = v.accu;
				// Update accumulator if test bit not set
				v.accu += v.add;
				unsigned int accPrev = v.accPrev;
				if (v.sync && !(v.accPrev & 0x8000000) && (v.accu & 0x8000000)
    				&& !( v.modulatedBy->sync && !(v.modulatedBy->accPrev & 0x800000) && 
        			(v.modulatedBy->accu & 0x800000)) 
           			)
#endif
					v.modulatesThis->accu = 0;
				if (v.freq) {
					unsigned int accNext = accPrev;
					unsigned int freq = v.freq << 4;
					do {
						accNext += freq;
						// noise shift register is updating even when waveform is not selected
						if (!(accPrev & 0x0800000) && (accNext & 0x0800000))
							updateShiftReg(v);
						accPrev = accNext;
					} while ( accNext < v.accu );
				}
				// real accu is 24 bit but we use FP integer arithmetic
				v.accu &= 0xFFFFFFF;
			}
			int output = v.disabled ? 0x0800 : getWaveSample(v);

			if (v.filter)
				sumFilteredOutput += (output - dcWave) * envelope + dcVoice;
			else {
				if (v.muted)
					sumOutput += (0x0800 - dcWave) * envelope + dcVoice;
				else
					sumOutput += (output - dcWave) * envelope + dcVoice;
			}
		} while (j--);

		int accu2 = (sumOutput + filterOutput(cyclesToDo, sumFilteredOutput) 
			+ dcMixer + dcDigiBlaster) * volume;

#if 1
		sample = accu2 >> 12;
#else
		unsigned int interPolationFac = (clockDeltaRemainder - sidCyclesPerSampleInt) & 0xFF;
		accu2 >>= 7;
		sample = (prevAccu * (0xFF ^ interPolationFac) + accu2 * (interPolationFac)) >> 12;
		prevAccu = accu2;
#endif

		*buf++ = (short) sample;
	}
}

SIDsound::~SIDsound()
{
	masterVolume = volume;
}
