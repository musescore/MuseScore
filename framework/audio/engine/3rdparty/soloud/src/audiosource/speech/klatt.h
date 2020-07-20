#ifndef KLATT_H
#define KLATT_H

#include "resonator.h"

#define CASCADE_PARALLEL      1
#define ALL_PARALLEL          2
#define NPAR                 40

class klatt_frame
{
public:
	int mF0FundamentalFreq;          // Voicing fund freq in Hz                       
	int mVoicingAmpdb;               // Amp of voicing in dB,            0 to   70    
	int mFormant1Freq;               // First formant freq in Hz,        200 to 1300  
	int mFormant1Bandwidth;          // First formant bw in Hz,          40 to 1000   
	int mFormant2Freq;               // Second formant freq in Hz,       550 to 3000  
	int mFormant2Bandwidth;          // Second formant bw in Hz,         40 to 1000   
	int mFormant3Freq;               // Third formant freq in Hz,        1200 to 4999 
	int mFormant3Bandwidth;          // Third formant bw in Hz,          40 to 1000   
	int mFormant4Freq;               // Fourth formant freq in Hz,       1200 to 4999 
	int mFormant4Bandwidth;          // Fourth formant bw in Hz,         40 to 1000   
	int mFormant5Freq;               // Fifth formant freq in Hz,        1200 to 4999 
	int mFormant5Bandwidth;          // Fifth formant bw in Hz,          40 to 1000   
	int mFormant6Freq;               // Sixth formant freq in Hz,        1200 to 4999 
	int mFormant6Bandwidth;          // Sixth formant bw in Hz,          40 to 2000   
	int mNasalZeroFreq;              // Nasal zero freq in Hz,           248 to  528  
	int mNasalZeroBandwidth;         // Nasal zero bw in Hz,             40 to 1000   
	int mNasalPoleFreq;              // Nasal pole freq in Hz,           248 to  528  
	int mNasalPoleBandwidth;         // Nasal pole bw in Hz,             40 to 1000   
	int mAspirationAmpdb;            // Amp of aspiration in dB,         0 to   70    
	int mNoSamplesInOpenPeriod;      // # of samples in open period,     10 to   65   
	int mVoicingBreathiness;         // Breathiness in voicing,          0 to   80    
	int mVoicingSpectralTiltdb;      // Voicing spectral tilt in dB,     0 to   24    
	int mFricationAmpdb;             // Amp of frication in dB,          0 to   80    
	int mSkewnessOfAlternatePeriods; // Skewness of alternate periods,   0 to   40 in sample#/2
	int mFormant1Ampdb;              // Amp of par 1st formant in dB,    0 to   80  
	int mFormant1ParallelBandwidth;  // Par. 1st formant bw in Hz,       40 to 1000 
	int mFormant2Ampdb;              // Amp of F2 frication in dB,       0 to   80  
	int mFormant2ParallelBandwidth;  // Par. 2nd formant bw in Hz,       40 to 1000 
	int mFormant3Ampdb;              // Amp of F3 frication in dB,       0 to   80  
	int mFormant3ParallelBandwidth;  // Par. 3rd formant bw in Hz,       40 to 1000 
	int mFormant4Ampdb;              // Amp of F4 frication in dB,       0 to   80  
	int mFormant4ParallelBandwidth;  // Par. 4th formant bw in Hz,       40 to 1000 
	int mFormant5Ampdb;              // Amp of F5 frication in dB,       0 to   80  
	int mFormant5ParallelBandwidth;  // Par. 5th formant bw in Hz,       40 to 1000 
	int mFormant6Ampdb;              // Amp of F6 (same as r6pa),        0 to   80  
	int mFormant6ParallelBandwidth;  // Par. 6th formant bw in Hz,       40 to 2000 
	int mParallelNasalPoleAmpdb;     // Amp of par nasal pole in dB,     0 to   80  
	int mBypassFricationAmpdb;       // Amp of bypass fric. in dB,       0 to   80  
	int mPalallelVoicingAmpdb;       // Amp of voicing,  par in dB,      0 to   70  
	int mOverallGaindb;              // Overall gain, 60 dB is unity,    0 to   60  
	klatt_frame();
};

class darray;
class Element;

class Slope
{
public:
	float mValue;                   /* boundary value */
	int mTime;                      /* transition time */
	Slope() 
	{
		mValue = 0;
		mTime = 0;
	}
};


enum KLATT_WAVEFORM
{
	KW_SAW,
	KW_TRIANGLE,
	KW_SIN,
	KW_SQUARE,
	KW_PULSE,
	KW_NOISE,
	KW_WARBLE
};

class klatt
{
	// resonators
	resonator mParallelFormant1, mParallelFormant2, mParallelFormant3, 
		      mParallelFormant4, mParallelFormant5, mParallelFormant6,
		      mParallelResoNasalPole, mNasalPole, mNasalZero, 
			  mCritDampedGlotLowPassFilter, mDownSampLowPassFilter, mOutputLowPassFilter;
public:
	int mBaseF0;
	float mBaseSpeed;
	float mBaseDeclination;
	int mBaseWaveform;

	int mF0Flutter;
	int mSampleRate;
	int mNspFr;
	int mF0FundamentalFreq;        // Voicing fund freq in Hz  
	int mVoicingAmpdb;          // Amp of voicing in dB,    0 to   70  
	int mSkewnessOfAlternatePeriods;         // Skewness of alternate periods,0 to   40  
	int mTimeCount;     // used for f0 flutter
	int mNPer;          // Current loc in voicing period   40000 samp/s
	int mT0;            // Fundamental period in output samples times 4 
	int mNOpen;         // Number of samples in open phase of period  
	int mNMod;          // Position in period to begin noise amp. modul 

	// Various amplitude variables used in main loop

	float mAmpVoice;     // mVoicingAmpdb converted to linear gain  
	float mAmpBypas;     // mBypassFricationAmpdb converted to linear gain  
	float mAmpAspir;     // AP converted to linear gain  
	float mAmpFrica;     // mFricationAmpdb converted to linear gain  
	float mAmpBreth;     // ATURB converted to linear gain  

	// State variables of sound sources

	int mSkew;                  // Alternating jitter, in half-period units  
	float mVLast;               // Previous output of voice  
	float mNLast;               // Previous output of random number generator  
	float mGlotLast;            // Previous value of glotout  
	float mDecay;               // mVoicingSpectralTiltdb converted to exponential time const  
	float mOneMd;               // in voicing one-pole ELM_FEATURE_LOW-pass filter  

	unsigned int mSeed;			// random seed



	float natural_source(int aNper);

	void frame_init();
	void flutter();
	void pitch_synch_par_reset(int ns);
	void parwave(short int *jwave);
	void init(int aBaseFrequency = 1330, float aBaseSpeed = 10.0f, float aBaseDeclination = 0.5f, int aBaseWaveform = KW_SAW);
	static int phone_to_elm(char *aPhoneme, int aCount, darray *aElement);

	int mElementCount;
	unsigned char *mElement;
	int mElementIndex;
	klatt_frame mFrame;
	Element * mLastElement;
	int mTStress;
	int mNTStress;
	Slope mStressS;
	Slope mStressE;
	float mTop;
	void initsynth(int aElementCount,unsigned char *aElement);
	int synth(int aSampleCount, short *aSamplePointer);
	klatt();
};

#endif