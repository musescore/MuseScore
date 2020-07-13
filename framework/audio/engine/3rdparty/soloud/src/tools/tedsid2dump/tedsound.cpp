#include "tedmem.h"
#include <math.h>
#ifdef _DEBUG
#include <cstdio>
#endif

#define PRECISION 0
#define OSCRELOADVAL (0x3FF << PRECISION)

unsigned int		   TED::masterVolume;
static int             Volume;
static int             Snd1Status;
static int             Snd2Status;
static int             SndNoiseStatus;
static int             DAStatus;
static unsigned short  Freq1;
static unsigned short  Freq2;
static int             NoiseCounter;
static int             FlipFlop[2];
static int             dcOutput[2];
static int             oscCount[2];
static int             OscReload[2];
static int             oscStep;
static unsigned char   noise[256]; // 0-8

inline void TED::setFreq(unsigned int channel, int freq)
{
	dcOutput[channel] = (freq == 0x3FE) ? 1 : 0;
	OscReload[channel] = ((freq + 1)&0x3FF) << PRECISION;
}

void TED::oscillatorReset()
{
	FlipFlop[0] = dcOutput[0] = 0;
	FlipFlop[1] = dcOutput[1] = 0;
	oscCount[0] = 0;
	oscCount[1] = 0;
	NoiseCounter = 0;
	Freq1 = Freq2 = 0;
	DAStatus = Snd1Status = Snd2Status = 0;
}

// call only once!
void TED::oscillatorInit()
{
	oscillatorReset();
	/* initialise im with 0xa8 */
	int im = 0xa8;
    for (unsigned int i = 0; i<256; i++) {
		noise[i] = im & 1;
		im = (im<<1)+(1^((im>>7)&1)^((im>>5)&1)^((im>>4)&1)^((im>>1)&1));
    }
	oscStep = (1 << PRECISION) << 0;

	// set player specific parameters
	waveForm[0] = waveForm[1] = 1;
	masterVolume = 8;
	setplaybackSpeed(3);// 5);// 3);
	enableChannel(0, true);
	enableChannel(1, true);
	enableChannel(2, true);
}

extern void storeregwrite(int reg, int value);

void TED::writeSoundReg(unsigned int reg, unsigned char value)
{
	storeregwrite(reg + 64, value);

	switch (reg) {
		case 0:
			Freq1 = (Freq1 & 0x300) | value;
			setFreq(0, Freq1);
			break;
		case 1:
			Freq2 = (Freq2 & 0x300) | value;
			setFreq(1, Freq2);
			break;
		case 2:
			Freq2 = (Freq2 & 0xFF) | (value << 8);
			setFreq(1, Freq2);
			break;
		case 3:
			if (DAStatus = value & 0x80) {
				FlipFlop[0] = 1;
				FlipFlop[1] = 1;
				oscCount[0] = OscReload[0];
				oscCount[1] = OscReload[1];
				NoiseCounter = 0xFF;
			}
			Volume = value & 0x0F;
			if (Volume > 8) Volume = 8;
			Volume = (Volume << 8) * masterVolume / 10;
			Snd1Status = value & 0x10;
			Snd2Status = value & 0x20;
			SndNoiseStatus = value & 0x40;
			break;
		case 4:
			Freq1 = (Freq1 & 0xFF) | (value << 8);
			setFreq(0, Freq1);
			break;
	}
}

inline unsigned int TED::waveSquare(unsigned int channel)
{
	return Volume;
}

inline unsigned int TED::waveSawTooth(unsigned int channel)
{
	unsigned int mod;

#if 0
	int msb = OSCRELOADVAL + 1 - OscReload[channel];
	int diff = 2 * msb - int(FlipFlop[channel]) * msb - int(oscCount[channel]) + int(OscReload[channel]);
	//if (diff < 0) diff = 0;
	//if (oscCount[channel] >= 0x3fa) diff = 0;
	mod = (Volume * diff) / (2 * msb);
#else
	int diff = int(oscCount[channel]) - int(OscReload[channel]);
	if (diff < 0) diff = 0;
	mod = (Volume * diff) / (OSCRELOADVAL + 1 - OscReload[channel]);
#endif
	return mod;
}

inline unsigned int TED::waveTriangle(unsigned int channel)
{
	unsigned int mod;
	int msb;

#if 0
	msb = OSCRELOADVAL + 1 - OscReload[channel];
	int diff = FlipFlop[channel] ? int(oscCount[channel]) - int(OscReload[channel]) 
		: int(OSCRELOADVAL) - int(oscCount[channel]);
	//if (diff < 0) diff = 0;
	//if (oscCount[channel] >= 0x3fa) diff = 0;
	mod = (3 * Volume * diff / msb / 2);
#else
	/*
		msb = (OscReload[channel] + OSCRELOADVAL) / 2;
	int diff = oscCount[channel] < msb ? oscCount[channel] - OscReload[channel] : OSCRELOADVAL - oscCount[channel];
	mod = (2 * diff * Volume / (OSCRELOADVAL - OscReload[channel] + 1));
	if (mod > Volume) mod = Volume;
	*/
	msb = (OscReload[channel] + OSCRELOADVAL) / 2;
	mod = oscCount[channel] < msb ? oscCount[channel] : (oscCount[channel] - msb);
	mod = (mod * Volume / msb);
#endif
	return mod;
}

inline unsigned int TED::getWaveSample(unsigned int channel, unsigned int wave)
{
	unsigned int sm;

	switch (wave) {
		default:
		case 1: // square
			return waveSquare(channel);
			break;
		case 2: // sawtooth
			return waveSawTooth(channel);
			break;
		case 4: // triangle
			return waveTriangle(channel);
			break;

		// combined waveforms á la SID
		case 3: // square + sawtooth
			sm = waveSawTooth(channel) + waveSquare(channel);
			return sm /= 2;
			break;
		case 5: // square + triangle
			sm = waveTriangle(channel) + waveSquare(channel);
			return sm /= 2;
			break;
		case 6: // sawtooth + triangle
			sm = waveTriangle(channel) + waveSawTooth(channel);
			return sm /= 2;
			break;
		case 7: // square + sawtooth + triangle
			sm = waveTriangle(channel) + waveSawTooth(channel) + waveSquare(channel);
			return sm /= 3;
			break;
	}
}

void TED::renderSound(unsigned int nrsamples)
{
	// Calculate the buffer...
	if (DAStatus) {// digi?
	} else {
		unsigned int result;
		for (;nrsamples--;) {
			// Channel 1
			if (dcOutput[0]) {
				FlipFlop[0] = 1;
			} else if ((oscCount[0] += oscStep) >= OSCRELOADVAL) {
				FlipFlop[0] ^= 1;
				oscCount[0] = OscReload[0] + (oscCount[0] - OSCRELOADVAL);
			}
			// Channel 2
			if (dcOutput[1]) {
				FlipFlop[1] = 1;
			} else if ((oscCount[1] += oscStep) >= OSCRELOADVAL) {
				NoiseCounter = (NoiseCounter + 1) & 0xFF;
				FlipFlop[1] ^= 1;
				oscCount[1] = OscReload[1] + (oscCount[1] - OSCRELOADVAL);
			}
			result = (Snd1Status && FlipFlop[0]) ? (getWaveSample(0, waveForm[0]) & channelMask[0]) : 0;
			if (Snd2Status && FlipFlop[1]) {
				result += getWaveSample(1, waveForm[1]) & channelMask[1];
			} else if (SndNoiseStatus && noise[NoiseCounter] & channelMask[2]) {
				result += Volume;
			}
		}   // for
	}
}

void TED::setMasterVolume(unsigned int shift)
{
	unsigned int vol = Ram[0xFF11] & 0x0f;
	if (vol > 8) vol = 8;
	Volume = (vol << 8) * shift / 10;
	masterVolume = shift;
}

void TED::selectWaveForm(unsigned int channel, unsigned int wave)
{
	waveForm[channel] = wave;
}

void TED::setplaybackSpeed(unsigned int speed)
{
	unsigned int speeds[] = { 16, 8, 4, 3, 2, 1 };
	playbackSpeed = speeds[(speed - 1) % (sizeof(speeds) / sizeof(unsigned int))];
}

unsigned int TED::getTimeSinceLastReset()
{
	ClockCycle elapsedCycles = CycleCounter - lastResetCycle;
	unsigned int secondsPlayed = (unsigned int)(double(elapsedCycles) / double(TED_SOUND_CLOCK * 4) + 0.5);
	return secondsPlayed;
}

void TED::setSampleRate(unsigned int value)
{
	initFilter(value, filterOrder);
}

void TED::setFilterOrder(unsigned int value)
{
	initFilter(sampleRate, value);
}

void TED::initFilter(unsigned int sampleRate_, unsigned int filterOrder_)
{
	filterOrder = filterOrder_;
	sampleRate = sampleRate_;
}
