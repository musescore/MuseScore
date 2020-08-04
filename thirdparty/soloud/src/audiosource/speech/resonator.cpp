#include <math.h>
#include "resonator.h"

#ifndef PI
#define PI 3.1415926535897932384626433832795f
#endif

/* Convert formant freqencies and bandwidth into resonator difference equation coefficents
	*/
void resonator::initResonator(
	int aFrequency,                       /* Frequency of resonator in Hz  */
	int aBandwidth,                      /* Bandwidth of resonator in Hz  */
	int aSamplerate)
{
	float arg = (-PI / aSamplerate) * aBandwidth;
	float r = (float)exp(arg);  
	mC = -(r * r);             
	arg = (-2.0f * PI / aSamplerate) * aFrequency;
	mB = r * (float)cos(arg) * 2.0f;   
	mA = 1.0f - mB - mC;    
}

/* Convert formant freqencies and bandwidth into anti-resonator difference equation constants
	*/
void resonator::initAntiresonator(
	int aFrequency,                       /* Frequency of resonator in Hz  */
	int aBandwidth,                      /* Bandwidth of resonator in Hz  */
	int aSamplerate)
{
	initResonator(aFrequency, aBandwidth, aSamplerate); /* First compute ordinary resonator coefficients */
	/* Now convert to antiresonator coefficients */
	mA = 1.0f / mA;             /* a'=  1/a */
	mB *= -mA;                  /* b'= -b/a */
	mC *= -mA;                  /* c'= -c/a */
}

/* Generic resonator function */
float resonator::resonate(float input)
{
	float x = mA * input + mB * mP1 + mC * mP2;
	mP2 = mP1;
	mP1 = x;
	return x;
}

/* Generic anti-resonator function
	Same as resonator except that a,b,c need to be set with initAntiresonator()
	and we save inputs in p1/p2 rather than outputs.
	There is currently only one of these - "mNasalZero"
*/
/*  Output = (mNasalZero.a * input) + (mNasalZero.b * oldin1) + (mNasalZero.c * oldin2) */

float resonator::antiresonate(float input)
{
	float x = mA * input + mB * mP1 + mC * mP2;
	mP2 = mP1;
	mP1 = input;
	return x;
}

resonator::resonator()
{
	mA = mB = mC = mP1 = mP2 = 0;
}

resonator::~resonator()
{
}

void resonator::setGain(float aG)
{
	mA *= aG;
}

