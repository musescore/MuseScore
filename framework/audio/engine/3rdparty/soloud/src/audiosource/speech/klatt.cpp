#include <math.h>
#include <stdlib.h>
#include "klatt.h"
#include "darray.h"
#include "resonator.h"

#ifndef PI
#define PI 3.1415926535897932384626433832795f
#endif

#ifndef NULL
#define NULL 0
#endif

class Interp
{
public:
	float mSteady;
	float mFixed;
	char  mProportion;
	char  mExtDelay;
	char  mIntDelay;
};


enum Eparm_e
{
  ELM_FN, ELM_F1, ELM_F2, ELM_F3, 
  ELM_B1, ELM_B2, ELM_B3, ELM_AN, 
  ELM_A1, ELM_A2, ELM_A3, ELM_A4, 
  ELM_A5, ELM_A6, ELM_AB, ELM_AV, 
  ELM_AVC, ELM_ASP, ELM_AF, 
  ELM_COUNT
};
 
class Element
{
public:
	  const char *mName; // unused
	  const char mRK;
	  const char mDU;
	  const char mUD;
	  unsigned char mFont; // unused
	  const char  *mDict; // unused
	  const char  *mIpa; // unused
	  int   mFeat; // only ELM_FEATURE_VWL
	  Interp mInterpolator[ELM_COUNT];
 };

enum ELEMENT_FEATURES
{
	ELM_FEATURE_ALV = 0x00000001,
	ELM_FEATURE_APR = 0x00000002,
	ELM_FEATURE_BCK = 0x00000004,
	ELM_FEATURE_BLB = 0x00000008,
	ELM_FEATURE_CNT = 0x00000010,
	ELM_FEATURE_DNT = 0x00000020,
	ELM_FEATURE_FNT = 0x00000040,
	ELM_FEATURE_FRC = 0x00000080,
	ELM_FEATURE_GLT = 0x00000100,
	ELM_FEATURE_HGH = 0x00000200,
	ELM_FEATURE_LAT = 0x00000400,
	ELM_FEATURE_LBD = 0x00000800,
	ELM_FEATURE_LBV = 0x00001000,
	ELM_FEATURE_LMD = 0x00002000,
	ELM_FEATURE_LOW = 0x00004000,
	ELM_FEATURE_MDL = 0x00008000,
	ELM_FEATURE_NAS = 0x00010000,
	ELM_FEATURE_PAL = 0x00020000,
	ELM_FEATURE_PLA = 0x00040000,
	ELM_FEATURE_RND = 0x00080000,
	ELM_FEATURE_RZD = 0x00100000,
	ELM_FEATURE_SMH = 0x00200000,
	ELM_FEATURE_STP = 0x00400000,
	ELM_FEATURE_UMD = 0x00800000,
	ELM_FEATURE_UNR = 0x01000000,
	ELM_FEATURE_VCD = 0x02000000,
	ELM_FEATURE_VEL = 0x04000000,
	ELM_FEATURE_VLS = 0x08000000,
	ELM_FEATURE_VWL = 0x10000000
};

enum ELEMENTS 
{
	ELM_END = 0,	
	ELM_Q,	ELM_P,	ELM_PY,	ELM_PZ,	ELM_T,	ELM_TY,	
	ELM_TZ,	ELM_K,	ELM_KY,	ELM_KZ,	ELM_B,	ELM_BY,	ELM_BZ,	
	ELM_D,	ELM_DY,	ELM_DZ,	ELM_G,	ELM_GY,	ELM_GZ,	ELM_M,	
	ELM_N,	ELM_NG,	ELM_F,	ELM_TH,	ELM_S,	ELM_SH,	ELM_X,
	ELM_H,	ELM_V,	ELM_QQ,	ELM_DH,	ELM_DI,	ELM_Z,	ELM_ZZ,
	ELM_ZH,	ELM_CH,	ELM_CI,	ELM_J,	ELM_JY,	ELM_L,	ELM_LL,
	ELM_RX,	ELM_R,	ELM_W,	ELM_Y,	ELM_I,	ELM_E,	ELM_AA,
	ELM_U,	ELM_O,	ELM_OO,	ELM_A,	ELM_EE,	ELM_ER,	ELM_AR,
	ELM_AW,	ELM_UU,	ELM_AI,	ELM_IE,	ELM_OI,	ELM_OU,	ELM_OV,
	ELM_OA,	ELM_IA,	ELM_IB,	ELM_AIR,ELM_OOR,ELM_OR
};

#define PHONEME_COUNT 53
#define AMP_ADJ 14
#define StressDur(e,s) (s,((e->mDU + e->mUD)/2))




class PhonemeToElements 
{
public:
	int mKey;
	char mData[8];
};

/* Order is important - 2 byte phonemes first, otherwise
   the search function will fail*/
static PhonemeToElements phoneme_to_elements[PHONEME_COUNT] = 
{
	/* mKey, count, 0-7 elements */
/* tS */ 0x5374, 2, ELM_CH, ELM_CI, 0, 0, 0, 0, 0,
/* dZ */ 0x5a64, 4, ELM_J, ELM_JY, ELM_QQ, ELM_JY, 0, 0, 0,
/* rr */ 0x7272, 3, ELM_R, ELM_QQ, ELM_R, 0, 0, 0, 0,
/* eI */ 0x4965, 2, ELM_AI, ELM_I, 0, 0, 0, 0, 0,
/* aI */ 0x4961, 2, ELM_IE, ELM_I, 0, 0, 0, 0, 0,
/* oI */ 0x496f, 2, ELM_OI, ELM_I, 0, 0, 0, 0, 0,
/* aU */ 0x5561, 2, ELM_OU, ELM_OV, 0, 0, 0, 0, 0,
/* @U */ 0x5540, 2, ELM_OA, ELM_OV, 0, 0, 0, 0, 0,
/* I@ */ 0x4049, 2, ELM_IA, ELM_IB, 0, 0, 0, 0, 0,
/* e@ */ 0x4065, 2, ELM_AIR, ELM_IB, 0, 0, 0, 0, 0,
/* U@ */ 0x4055, 2, ELM_OOR, ELM_IB, 0, 0, 0, 0, 0,
/* O@ */ 0x404f, 2, ELM_OR, ELM_IB, 0, 0, 0, 0, 0,
/* oU */ 0x556f, 2, ELM_OI, ELM_OV, 0, 0, 0, 0, 0,
/*    */ 0x0020, 1, ELM_Q, 0, 0, 0, 0, 0, 0,
/* p  */ 0x0070, 3, ELM_P, ELM_PY, ELM_PZ, 0, 0, 0, 0,
/* t  */ 0x0074, 3, ELM_T, ELM_TY, ELM_TZ, 0, 0, 0, 0,
/* k  */ 0x006b, 3, ELM_K, ELM_KY, ELM_KZ, 0, 0, 0, 0,
/* b  */ 0x0062, 3, ELM_B, ELM_BY, ELM_BZ, 0, 0, 0, 0,
/* d  */ 0x0064, 3, ELM_D, ELM_DY, ELM_DZ, 0, 0, 0, 0,
/* g  */ 0x0067, 3, ELM_G, ELM_GY, ELM_GZ, 0, 0, 0, 0,
/* m  */ 0x006d, 1, ELM_M, 0, 0, 0, 0, 0, 0,
/* n  */ 0x006e, 1, ELM_N, 0, 0, 0, 0, 0, 0,
/* N  */ 0x004e, 1, ELM_NG, 0, 0, 0, 0, 0, 0,
/* f  */ 0x0066, 1, ELM_F, 0, 0, 0, 0, 0, 0,
/* T  */ 0x0054, 1, ELM_TH, 0, 0, 0, 0, 0, 0,
/* s  */ 0x0073, 1, ELM_S, 0, 0, 0, 0, 0, 0,
/* S  */ 0x0053, 1, ELM_SH, 0, 0, 0, 0, 0, 0,
/* h  */ 0x0068, 1, ELM_H, 0, 0, 0, 0, 0, 0,
/* v  */ 0x0076, 3, ELM_V, ELM_QQ, ELM_V, 0, 0, 0, 0,
/* D  */ 0x0044, 3, ELM_DH, ELM_QQ, ELM_DI, 0, 0, 0, 0,
/* z  */ 0x007a, 3, ELM_Z, ELM_QQ, ELM_ZZ, 0, 0, 0, 0,
/* Z  */ 0x005a, 3, ELM_ZH, ELM_QQ, ELM_ZH, 0, 0, 0, 0,
/* l  */ 0x006c, 1, ELM_L, 0, 0, 0, 0, 0, 0,
/* r  */ 0x0072, 1, ELM_R, 0, 0, 0, 0, 0, 0,
/* R  */ 0x0052, 1, ELM_RX, 0, 0, 0, 0, 0, 0,
/* w  */ 0x0077, 1, ELM_W, 0, 0, 0, 0, 0, 0,
/* x  */ 0x0078, 1, ELM_X, 0, 0, 0, 0, 0, 0,
/* %  */ 0x0025, 1, ELM_QQ, 0, 0, 0, 0, 0, 0,
/* j  */ 0x006a, 1, ELM_Y, 0, 0, 0, 0, 0, 0,
/* I  */ 0x0049, 1, ELM_I, 0, 0, 0, 0, 0, 0,
/* e  */ 0x0065, 1, ELM_E, 0, 0, 0, 0, 0, 0,
/* &  */ 0x0026, 1, ELM_AA, 0, 0, 0, 0, 0, 0,
/* V  */ 0x0056, 1, ELM_U, 0, 0, 0, 0, 0, 0,
/* 0  */ 0x0030, 1, ELM_O, 0, 0, 0, 0, 0, 0,
/* U  */ 0x0055, 1, ELM_OO, 0, 0, 0, 0, 0, 0,
/* @  */ 0x0040, 1, ELM_A, 0, 0, 0, 0, 0, 0,
/* i  */ 0x0069, 1, ELM_EE, 0, 0, 0, 0, 0, 0,
/* 3  */ 0x0033, 1, ELM_ER, 0, 0, 0, 0, 0, 0,
/* A  */ 0x0041, 1, ELM_AR, 0, 0, 0, 0, 0, 0,
/* O  */ 0x004f, 1, ELM_AW, 0, 0, 0, 0, 0, 0,
/* u  */ 0x0075, 1, ELM_UU, 0, 0, 0, 0, 0, 0,
/* o  */ 0x006f, 1, ELM_OI, 0, 0, 0, 0, 0, 0,
/* .  */ 0x002e, 1, ELM_END,0, 0, 0, 0, 0, 0,
};

static Element gElement[] =
{
#include "Elements.def"
};

static short clip(float input)
{
	int temp = (int)input;
	/* clip on boundaries of 16-bit word */

	if (temp < -32767)
	{
		//assert?
		temp = -32767;
	}
	else
	if (temp > 32767)
	{
		//assert?
		temp = 32767;
	}

	return (short)(temp);
}

/* Convert from decibels to a linear scale factor */
static float DBtoLIN(int dB)
{
	/*
	* Convertion table, db to linear, 87 dB --> 32767
	*                                 86 dB --> 29491 (1 dB down = 0.5**1/6)
	*                                 ...
	*                                 81 dB --> 16384 (6 dB down = 0.5)
	*                                 ...
	*                                  0 dB -->     0
	*
	* The just noticeable difference for a change in intensity of a vowel
	*   is approximately 1 dB.  Thus all amplitudes are quantized to 1 dB
	*   steps.
	*/

	static const float amptable[88] =
	{
		0.0, 0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 6.0, 7.0,
		8.0, 9.0, 10.0, 11.0, 13.0,
		14.0, 16.0, 18.0, 20.0, 22.0,
		25.0, 28.0, 32.0, 35.0, 40.0,
		45.0, 51.0, 57.0, 64.0, 71.0,
		80.0, 90.0, 101.0, 114.0, 128.0,
		142.0, 159.0, 179.0, 202.0, 227.0,
		256.0, 284.0, 318.0, 359.0, 405.0,
		455.0, 512.0, 568.0, 638.0, 719.0,
		811.0, 911.0, 1024.0, 1137.0, 1276.0,
		1438.0, 1622.0, 1823.0, 2048.0, 2273.0,
		2552.0, 2875.0, 3244.0, 3645.0, 4096.0,
		4547.0, 5104.0, 5751.0, 6488.0, 7291.0,
		8192.0, 9093.0, 10207.0, 11502.0, 12976.0,
		14582.0, 16384.0, 18350.0, 20644.0, 23429.0,
		26214.0, 29491.0, 32767.0
	};

	// Check limits or argument (can be removed in final product)
	if (dB < 0)
	{
		dB = 0;
	}
	else
	if (dB >= 88)
	{
		dB = 87;
	}

	return amptable[dB] * 0.001f;
}



klatt_frame::klatt_frame() :
	mF0FundamentalFreq(1330),	mVoicingAmpdb(60),				mFormant1Freq(500),  
	mFormant1Bandwidth(60),		mFormant2Freq(1500),			mFormant2Bandwidth(90),
	mFormant3Freq(2800),		mFormant3Bandwidth(150),		mFormant4Freq(3250), 
	mFormant4Bandwidth(200),	mFormant5Freq(3700),			mFormant5Bandwidth(200),  
	mFormant6Freq(4990),		mFormant6Bandwidth(500),		mNasalZeroFreq(270),  
	mNasalZeroBandwidth(100),	mNasalPoleFreq(270),			mNasalPoleBandwidth(100), 
	mAspirationAmpdb(0),		mNoSamplesInOpenPeriod(30),		mVoicingBreathiness(0),      
	mVoicingSpectralTiltdb(10), mFricationAmpdb(0),				mSkewnessOfAlternatePeriods(0),   
	mFormant1Ampdb(0),			mFormant1ParallelBandwidth(80), mFormant2Ampdb(0),      
	mFormant2ParallelBandwidth(200), mFormant3Ampdb(0),			mFormant3ParallelBandwidth(350),
	mFormant4Ampdb(0),			mFormant4ParallelBandwidth(500), mFormant5Ampdb(0),      
	mFormant5ParallelBandwidth(600), mFormant6Ampdb(0),			mFormant6ParallelBandwidth(800),    
	mParallelNasalPoleAmpdb(0), mBypassFricationAmpdb(0),       mPalallelVoicingAmpdb(0),   
	mOverallGaindb(62)  
{
};


klatt::klatt() : 
	mBaseF0(1330),
	mBaseSpeed(10.0f),
	mBaseDeclination(0.5f),
	mBaseWaveform(KW_SAW),
	mF0Flutter(0), 
	mSampleRate(0), 
	mNspFr(0),
	mF0FundamentalFreq(0),
	mVoicingAmpdb(0),
	mSkewnessOfAlternatePeriods(0),
	mTimeCount(0), 
	mNPer(0),
	mT0(0),
	mNOpen(0),
	mNMod(0),
	mAmpVoice(0),
	mAmpBypas(0),
	mAmpAspir(0),
	mAmpFrica(0),
	mAmpBreth(0),
	mSkew(0),
	mVLast(0),
	mNLast(0),
	mGlotLast(0),
	mDecay(0),
	mOneMd(0),
	mSeed(5),
	mElementCount(0),
	mElement(0),
	mElementIndex(0),
	mLastElement(0),
	mTStress(0),
	mNTStress(0),
	mTop(0)
{
}

/*
function FLUTTER

This function adds F0 flutter, as specified in:

"Analysis, synthesis and perception of voice quality variations among
female and male talkers" D.H. Klatt and L.C. Klatt JASA 87(2) February 1990.
Flutter is added by applying a quasi-random element constructed from three
slowly varying sine waves.
*/
void klatt::flutter()
{
	int original_f0 = mFrame.mF0FundamentalFreq / 10;
	float fla = (float) mF0Flutter / 50;
	float flb = (float) original_f0 / 100;
	float flc = (float)sin(2 * PI * 12.7 * mTimeCount);
	float fld = (float)sin(2 * PI * 7.1 * mTimeCount);
	float fle = (float)sin(2 * PI * 4.7 * mTimeCount);
	float delta_f0 = fla * flb * (flc + fld + fle) * 10;
	mF0FundamentalFreq += (int) delta_f0;
}

/* Vwave is the differentiated glottal flow waveform, there is a weak
spectral zero around 800 Hz, magic constants a,b reset pitch-synch
*/

float klatt::natural_source(int aNper)
{
	// See if glottis open 
	if (aNper < mNOpen)
	{
		switch (mBaseWaveform)
		{
		case KW_TRIANGLE:
			return ((aNper % 200) - 100) * 81.92f; // triangle
		case KW_SIN:
			return (float)(sin(aNper * 0.0314) * 8192); // sin
		case KW_SQUARE:
			return ((aNper % 200) - 100) > 0 ? 8192.0f : -8192.0f; // square
		case KW_PULSE:
			return ((aNper % 200) - 100) > 50 ? 8192.0f : -8192.0f; // pulse
		case KW_NOISE:
			return (int)mNLast & 1 ? -8192.0f : 8192.0f;
		case KW_WARBLE:
				return (int)mNLast & 7 ? -8192.0f : 8192.0f;
		case KW_SAW: // fallthrough
		default:
			return (abs((aNper % 200) - 100) - 50) * 163.84f; // saw
		}	
	}
	else
	{
		// Glottis closed 
		return (0.0);
	}
	
}

/* Reset selected parameters pitch-synchronously */

void klatt::pitch_synch_par_reset(int ns)
{	
	if (mF0FundamentalFreq > 0)
	{
		mT0 = (40 * mSampleRate) / mF0FundamentalFreq;

		/* Period in samp*4 */
		mAmpVoice = DBtoLIN(mVoicingAmpdb);

		/* Duration of period before amplitude modulation */
		mNMod = mT0;

		if (mVoicingAmpdb > 0)
		{
			mNMod >>= 1;
		}

		/* Breathiness of voicing waveform */

		mAmpBreth = DBtoLIN(mFrame.mVoicingBreathiness) * 0.1f;

		/* Set open phase of glottal period */
		/* where  40 <= open phase <= 263 */

		mNOpen = 4 * mFrame.mNoSamplesInOpenPeriod;

		if (mNOpen >= (mT0 - 1))
		{
			mNOpen = mT0 - 2;
		}

		if (mNOpen < 40)
		{
			mNOpen = 40;                  /* F0 max = 1000 Hz */
		}

		int temp;
		float temp1;

		temp = mSampleRate / mNOpen;
		mCritDampedGlotLowPassFilter.initResonator(0L, temp, mSampleRate);

		/* Make gain at F1 about constant */

		temp1 = mNOpen * .00833f;
		mCritDampedGlotLowPassFilter.setGain(temp1 * temp1);

		/* Truncate skewness so as not to exceed duration of closed phase
		of glottal period */

		temp = mT0 - mNOpen;

		if (mSkewnessOfAlternatePeriods > temp)
		{
			mSkewnessOfAlternatePeriods = temp;
		}

		if (mSkew >= 0)
		{
			mSkew = mSkewnessOfAlternatePeriods;                /* Reset mSkew to requested mSkewnessOfAlternatePeriods */
		}
		else
		{
			mSkew = -mSkewnessOfAlternatePeriods;
		}

		/* Add skewness to closed portion of voicing period */

		mT0 = mT0 + mSkew;
		mSkew = -mSkew;
	}
	else
	{
		mT0 = 4;                        /* Default for f0 undefined */
		mAmpVoice = 0.0;
		mNMod = mT0;
		mAmpBreth = 0.0;
	}

	/* Reset these pars pitch synchronously or at update rate if f0=0 */

	if ((mT0 != 4) || (ns == 0))
	{
		/* Set one-pole ELM_FEATURE_LOW-pass filter that tilts glottal source */
		mDecay = (0.033f * mFrame.mVoicingSpectralTiltdb);	/* Function of samp_rate ? */

		if (mDecay > 0.0f)
		{
			mOneMd = 1.0f - mDecay;
		}
		else
		{
			mOneMd = 1.0f;
		}
	}
}


/* Get variable parameters from host computer,
initially also get definition of fixed pars
*/

void klatt::frame_init()
{
	int mOverallGaindb;                       /* Overall gain, 60 dB is unity  0 to   60  */
	float amp_parF1;                 /* mFormant1Ampdb converted to linear gain  */
	float amp_parFN;                 /* mParallelNasalPoleAmpdb converted to linear gain  */
	float amp_parF2;                 /* mFormant2Ampdb converted to linear gain  */
	float amp_parF3;                 /* mFormant3Ampdb converted to linear gain  */
	float amp_parF4;                 /* mFormant4Ampdb converted to linear gain  */
	float amp_parF5;                 /* mFormant5Ampdb converted to linear gain  */
	float amp_parF6;                 /* mFormant6Ampdb converted to linear gain  */

	/* Read  speech frame definition into temp store
       and move some parameters into active use immediately
       (voice-excited ones are updated pitch synchronously
       to avoid waveform glitches).
	 */

	mF0FundamentalFreq = mFrame.mF0FundamentalFreq;
	mVoicingAmpdb = mFrame.mVoicingAmpdb - 7;

	if (mVoicingAmpdb < 0) mVoicingAmpdb = 0;

	mAmpAspir = DBtoLIN(mFrame.mAspirationAmpdb) * .05f;
	mAmpFrica = DBtoLIN(mFrame.mFricationAmpdb) * 0.25f;
	mSkewnessOfAlternatePeriods = mFrame.mSkewnessOfAlternatePeriods;

	/* Fudge factors (which comprehend affects of formants on each other?)
       with these in place ALL_PARALLEL should sound as close as
	   possible to CASCADE_PARALLEL.
	   Possible problem feeding in Holmes's amplitudes given this.
	*/
	amp_parF1 = DBtoLIN(mFrame.mFormant1Ampdb) * 0.4f;	/* -7.96 dB */
	amp_parF2 = DBtoLIN(mFrame.mFormant2Ampdb) * 0.15f;	/* -16.5 dB */
	amp_parF3 = DBtoLIN(mFrame.mFormant3Ampdb) * 0.06f;	/* -24.4 dB */
	amp_parF4 = DBtoLIN(mFrame.mFormant4Ampdb) * 0.04f;	/* -28.0 dB */
	amp_parF5 = DBtoLIN(mFrame.mFormant5Ampdb) * 0.022f;	/* -33.2 dB */
	amp_parF6 = DBtoLIN(mFrame.mFormant6Ampdb) * 0.03f;	/* -30.5 dB */
	amp_parFN = DBtoLIN(mFrame.mParallelNasalPoleAmpdb) * 0.6f;	/* -4.44 dB */
	mAmpBypas = DBtoLIN(mFrame.mBypassFricationAmpdb) * 0.05f;	/* -26.0 db */

	// Set coeficients of nasal resonator and zero antiresonator 
	mNasalPole.initResonator(mFrame.mNasalPoleFreq, mFrame.mNasalPoleBandwidth, mSampleRate);

	mNasalZero.initAntiresonator(mFrame.mNasalZeroFreq, mFrame.mNasalZeroBandwidth, mSampleRate);

	// Set coefficients of parallel resonators, and amplitude of outputs 
	mParallelFormant1.initResonator(mFrame.mFormant1Freq, mFrame.mFormant1ParallelBandwidth, mSampleRate);
	mParallelFormant1.setGain(amp_parF1);

	mParallelResoNasalPole.initResonator(mFrame.mNasalPoleFreq, mFrame.mNasalPoleBandwidth, mSampleRate);
	mParallelResoNasalPole.setGain(amp_parFN);

	mParallelFormant2.initResonator(mFrame.mFormant2Freq, mFrame.mFormant2ParallelBandwidth, mSampleRate);
	mParallelFormant2.setGain(amp_parF2);

	mParallelFormant3.initResonator(mFrame.mFormant3Freq, mFrame.mFormant3ParallelBandwidth, mSampleRate);
	mParallelFormant3.setGain(amp_parF3);

	mParallelFormant4.initResonator(mFrame.mFormant4Freq, mFrame.mFormant4ParallelBandwidth, mSampleRate);
	mParallelFormant4.setGain(amp_parF4);

	mParallelFormant5.initResonator(mFrame.mFormant5Freq, mFrame.mFormant5ParallelBandwidth, mSampleRate);
	mParallelFormant5.setGain(amp_parF5);

	mParallelFormant6.initResonator(mFrame.mFormant6Freq, mFrame.mFormant6ParallelBandwidth, mSampleRate);
	mParallelFormant6.setGain(amp_parF6);


	/* fold overall gain into output resonator */
	mOverallGaindb = mFrame.mOverallGaindb - 3;

	if (mOverallGaindb <= 0)
		mOverallGaindb = 57;

	/* output ELM_FEATURE_LOW-pass filter - resonator with freq 0 and BW = globals->mSampleRate
	Thus 3db point is globals->mSampleRate/2 i.e. Nyquist limit.
	Only 3db down seems rather mild...
	*/
	mOutputLowPassFilter.initResonator(0L, (int)mSampleRate, mSampleRate);
	mOutputLowPassFilter.setGain(DBtoLIN(mOverallGaindb));
}

/*
function PARWAV

CONVERT FRAME OF PARAMETER DATA TO A WAVEFORM CHUNK
Synthesize globals->mNspFr samples of waveform and store in jwave[].
*/

void klatt::parwave(short int *jwave)
{
	/* Output of cascade branch, also final output  */

	/* Initialize synthesizer and get specification for current speech
	frame from host microcomputer */

	frame_init();

	if (mF0Flutter != 0)
	{
		mTimeCount++;                  /* used for f0 flutter */
		flutter();       /* add f0 flutter */
	}

	/* MAIN LOOP, for each output sample of current frame: */

	int ns;
	for (ns = 0; ns < mNspFr; ns++)
	{
		float noise;
		int n4;
		float sourc;                   /* Sound source if all-parallel config used  */
		float glotout;                 /* Output of glottal sound source  */
		float par_glotout;             /* Output of parallelglottal sound sourc  */
		float voice = 0;               /* Current sample of voicing waveform  */
		float frics;                   /* Frication sound source  */
		float aspiration;              /* Aspiration sound source  */
		int nrand;                    /* Varible used by random number generator  */

		/* Our own code like rand(), but portable
		whole upper 31 bits of seed random
		assumes 32-bit unsigned arithmetic
		with untested code to handle larger.
		*/
		mSeed = mSeed * 1664525 + 1;

		mSeed &= 0xFFFFFFFF;

		/* Shift top bits of seed up to top of int then back down to LS 14 bits */
		/* Assumes 8 bits per sizeof unit i.e. a "byte" */
		nrand = (((int) mSeed) << (8 * sizeof(int) - 32)) >> (8 * sizeof(int) - 14);

		/* Tilt down noise spectrum by soft ELM_FEATURE_LOW-pass filter having
		*    a pole near the origin in the z-plane, i.e.
		*    output = input + (0.75 * lastoutput) */

		noise = nrand + (0.75f * mNLast);	/* Function of samp_rate ? */

		mNLast = noise;

		/* Amplitude modulate noise (reduce noise amplitude during
		second half of glottal period) if voicing simultaneously present
		*/

		if (mNPer > mNMod)
		{
			noise *= 0.5f;
		}

		/* Compute frication noise */
		sourc = frics = mAmpFrica * noise;

		/* Compute voicing waveform : (run glottal source simulation at
		4 times normal sample rate to minimize quantization noise in
		period of female voice)
		*/

		for (n4 = 0; n4 < 4; n4++)
		{
			/* use a more-natural-shaped source waveform with excitation
			occurring both upon opening and upon closure, stronest at closure */
			voice = natural_source(mNPer);

			/* Reset period when counter 'mNPer' reaches mT0 */

			if (mNPer >= mT0)
			{
				mNPer = 0;
				pitch_synch_par_reset(ns);
			}

			/* Low-pass filter voicing waveform before downsampling from 4*globals->mSampleRate */
			/* to globals->mSampleRate samples/sec.  Resonator f=.09*globals->mSampleRate, bw=.06*globals->mSampleRate  */

			voice = mDownSampLowPassFilter.resonate(voice);	/* in=voice, out=voice */

			/* Increment counter that keeps track of 4*globals->mSampleRate samples/sec */
			mNPer++;
		}

		/* Tilt spectrum of voicing source down by soft ELM_FEATURE_LOW-pass filtering, amount
		of tilt determined by mVoicingSpectralTiltdb
		*/
		voice = (voice * mOneMd) + (mVLast * mDecay);

		mVLast = voice;

		/* Add breathiness during glottal open phase */
		if (mNPer < mNOpen)
		{
			/* Amount of breathiness determined by parameter mVoicingBreathiness */
			/* Use nrand rather than noise because noise is ELM_FEATURE_LOW-passed */
			voice += mAmpBreth * nrand;
		}

		/* Set amplitude of voicing */
		glotout = mAmpVoice * voice;

		/* Compute aspiration amplitude and add to voicing source */
		aspiration = mAmpAspir * noise;

		glotout += aspiration;

		par_glotout = glotout;

		/* NIS - rsynth "hack"
		As Holmes' scheme is weak at nasals and (physically) nasal cavity
		is "back near glottis" feed glottal source through nasal resonators
		Don't think this is quite right, but improves things a bit
		*/
		par_glotout = mNasalZero.antiresonate(par_glotout);
		par_glotout = mNasalPole.resonate(par_glotout);
		/* And just use mParallelFormant1 NOT mParallelResoNasalPole */		
		float out = mParallelFormant1.resonate(par_glotout);
		/* Sound sourc for other parallel resonators is frication
		plus first difference of voicing waveform.
		*/
		sourc += (par_glotout - mGlotLast);
		mGlotLast = par_glotout;

		/* Standard parallel vocal tract
		Formants F6,F5,F4,F3,F2, outputs added with alternating sign
		*/
		out = mParallelFormant6.resonate(sourc) - out;
		out = mParallelFormant5.resonate(sourc) - out;
		out = mParallelFormant4.resonate(sourc) - out;
		out = mParallelFormant3.resonate(sourc) - out;
		out = mParallelFormant2.resonate(sourc) - out;

		out = mAmpBypas * sourc - out;
		out = mOutputLowPassFilter.resonate(out);

		*jwave++ = clip(out); /* Convert back to integer */
	}
}



static char * phoneme_to_element_lookup(char *s, void ** data)
{
	int key8 = *s;
	int key16 = key8 + (s[1] << 8);
	if (s[1] == 0) key16 = -1; // avoid key8==key16
	int i;
	for (i = 0; i < PHONEME_COUNT; i++)
	{
		if (phoneme_to_elements[i].mKey == key16)
		{
			*data = &phoneme_to_elements[i].mData;
			return s+2;
		}
		if (phoneme_to_elements[i].mKey == key8)
		{
			*data = &phoneme_to_elements[i].mData;
			return s+1;
		}
	}
	// should never happen
	*data = NULL;
	return s+1;
}



int klatt::phone_to_elm(char *aPhoneme, int aCount, darray *aElement)
{
	int stress = 0;
	char *s = aPhoneme;
	int t = 0;
	char *limit = s + aCount;

	while (s < limit && *s)
	{
		char *e = NULL;
		s = phoneme_to_element_lookup(s, (void**)&e);

		if (e)
		{
			int n = *e++;

			while (n-- > 0)
			{
				int x = *e++;
				Element * p = &gElement[x];
				/* This works because only vowels have mUD != mDU,
				and we set stress just before a vowel
				*/
				aElement->put(x);

				if (!(p->mFeat & ELM_FEATURE_VWL))
					stress = 0;

				int stressdur = StressDur(p,stress);

				t += stressdur;

				aElement->put(stressdur);
				aElement->put(stress);
			}
		}

		else
		{
			char ch = *s++;

			switch (ch)
			{

			case '\'':                /* Primary stress */
				stress = 3;
				break;

			case ',':                 /* Secondary stress */
				stress = 2;
				break;

			case '+':                 /* Tertiary stress */
				stress = 1;
				break;

			case '-':                 /* hyphen in input */
				break;

			default:
//				fprintf(stderr, "Ignoring %c in '%.*s'\n", ch, aCount, aPhoneme);
				break;
			}
		}
	}

	return t;
}



/* 'a' is dominant element, 'b' is dominated
    ext is flag to say to use external times from 'a' rather
    than internal i.e. ext != 0 if 'a' is NOT current element.
 */

static void set_trans(Slope *t, Element * a, Element * b,int ext, int /* e */)
{
	int i;

	for (i = 0; i < ELM_COUNT; i++)
	{
		t[i].mTime = ((ext) ? a->mInterpolator[i].mExtDelay : a->mInterpolator[i].mIntDelay);

		if (t[i].mTime)
		{
			t[i].mValue = a->mInterpolator[i].mFixed + (a->mInterpolator[i].mProportion * b->mInterpolator[i].mSteady) * 0.01f; // mProportion is in scale 0..100, so *0.01.
		}
		else
		{
			t[i].mValue = b->mInterpolator[i].mSteady;
		}
	}
}

static float lerp(float a, float b, int t, int d)
{
	if (t <= 0)
	{
		return a;
	}

	if (t >= d)
	{
		return b;
	}

	float f = (float)t / (float)d;
	return a + (b - a) * f;
}

static float interpolate(Slope *aStartSlope, Slope *aEndSlope, float aMidValue, int aTime, int aDuration)
{
	int steadyTime = aDuration - (aStartSlope->mTime + aEndSlope->mTime);

	if (steadyTime >= 0)
	{
		// Interpolate to a midpoint, stay there for a while, then interpolate to end

		if (aTime < aStartSlope->mTime)
		{
			// interpolate to the first value
			return lerp(aStartSlope->mValue, aMidValue, aTime, aStartSlope->mTime);
		}
		// reached midpoint

		aTime -= aStartSlope->mTime;

		if (aTime <= steadyTime)
		{
			// still at steady state
			return aMidValue;  
		}

		// interpolate to the end
		return lerp(aMidValue, aEndSlope->mValue, aTime - steadyTime, aEndSlope->mTime);
	}
	else
	{
		// No steady state
		float f = 1.0f - ((float) aTime / (float) aDuration);
		float sp = lerp(aStartSlope->mValue, aMidValue, aTime, aStartSlope->mTime);
		float ep = lerp(aEndSlope->mValue, aMidValue, aDuration - aTime, aEndSlope->mTime);
		return f * sp + ((float) 1.0 - f) * ep;
	}
}



void klatt::initsynth(int aElementCount,unsigned char *aElement)
{
	mElement = aElement;
	mElementCount = aElementCount;
	mElementIndex = 0;
	mLastElement = &gElement[0];
	mSeed = 5;
	mTStress = 0;
	mNTStress = 0;
	mFrame.mF0FundamentalFreq = mBaseF0;
	mTop = 1.1f * mFrame.mF0FundamentalFreq;
	mFrame.mNasalPoleFreq = (int)mLastElement->mInterpolator[ELM_FN].mSteady;
	mFrame.mFormant1ParallelBandwidth = mFrame.mFormant1Bandwidth = 60;
	mFrame.mFormant2ParallelBandwidth = mFrame.mFormant2Bandwidth = 90;
	mFrame.mFormant3ParallelBandwidth = mFrame.mFormant3Bandwidth = 150;
//	mFrame.mFormant4ParallelBandwidth = (default)

	// Set stress attack/decay slope 
	mStressS.mTime = 40;
	mStressE.mTime = 40;
	mStressE.mValue = 0.0;
}

int klatt::synth(int /* aSampleCount */, short *aSamplePointer)
{
	short *samp = aSamplePointer;

	if (mElementIndex >= mElementCount)
		return -1;

	Element * currentElement = &gElement[mElement[mElementIndex++]];
	int dur = mElement[mElementIndex++];
	mElementIndex++; // skip stress 
	
	if (currentElement->mRK == 31) // "END"
	{
		// Reset the fundamental frequency top
		mFrame.mF0FundamentalFreq = mBaseF0;
		mTop = 1.1f * mFrame.mF0FundamentalFreq;
	}

	// Skip zero length elements which are only there to affect
	// boundary values of adjacent elements		

	if (dur > 0)
	{
		Element * ne = (mElementIndex < mElementCount) ? &gElement[mElement[mElementIndex]] : &gElement[0];
		Slope start[ELM_COUNT];
		Slope end[ELM_COUNT];
		int t;

		if (currentElement->mRK > mLastElement->mRK)
		{
			set_trans(start, currentElement, mLastElement, 0, 's');
			// we dominate last 
		}
		else
		{
			set_trans(start, mLastElement, currentElement, 1, 's');
			// last dominates us 
		}

		if (ne->mRK > currentElement->mRK)
		{
			set_trans(end, ne, currentElement, 1, 'e');
			// next dominates us 
		}
		else
		{
			set_trans(end, currentElement, ne, 0, 'e');
			// we dominate next 
		}

		for (t = 0; t < dur; t++, mTStress++)
		{
			float base = mTop * 0.8f; // 3 * top / 5 
			float tp[ELM_COUNT];

			if (mTStress == mNTStress)
			{
				int j = mElementIndex;
				mStressS = mStressE;
				mTStress = 0;
				mNTStress = dur;

				while (j <= mElementCount)
				{
					Element * e   = (j < mElementCount) ? &gElement[mElement[j++]] : &gElement[0];
					int du = (j < mElementCount) ? mElement[j++] : 0;
					int s  = (j < mElementCount) ? mElement[j++] : 3;

					if (s || e->mFeat & ELM_FEATURE_VWL)
					{
						int d = 0;

						if (s)
							mStressE.mValue = (float) s / 3;
						else
							mStressE.mValue = (float) 0.1;

						do
						{
							d += du;
							e = (j < mElementCount) ? &gElement[mElement[j++]] : &gElement[0];
							du = mElement[j++];
						}

						while ((e->mFeat & ELM_FEATURE_VWL) && mElement[j++] == s);

						mNTStress += d / 2;

						break;
					}

					mNTStress += du;
				}
			}

			int j;
			for (j = 0; j < ELM_COUNT; j++)
			{
				tp[j] = interpolate(&start[j], &end[j], (float) currentElement->mInterpolator[j].mSteady, t, dur);
			}

			// Now call the synth for each frame 

			mFrame.mF0FundamentalFreq = (int)(base + (mTop - base) * interpolate(&mStressS, &mStressE, (float)0, mTStress, mNTStress));
			mFrame.mVoicingAmpdb = mFrame.mPalallelVoicingAmpdb = (int)tp[ELM_AV];
			mFrame.mFricationAmpdb = (int)tp[ELM_AF];
			mFrame.mNasalZeroFreq = (int)tp[ELM_FN];
			mFrame.mAspirationAmpdb = (int)tp[ELM_ASP];
			mFrame.mVoicingBreathiness = (int)tp[ELM_AVC];
			mFrame.mFormant1ParallelBandwidth = mFrame.mFormant1Bandwidth = (int)tp[ELM_B1];
			mFrame.mFormant2ParallelBandwidth = mFrame.mFormant2Bandwidth = (int)tp[ELM_B2];
			mFrame.mFormant3ParallelBandwidth = mFrame.mFormant3Bandwidth = (int)tp[ELM_B3];
			mFrame.mFormant1Freq = (int)tp[ELM_F1];
			mFrame.mFormant2Freq = (int)tp[ELM_F2];
			mFrame.mFormant3Freq = (int)tp[ELM_F3];

			// AMP_ADJ + is a kludge to get amplitudes up to klatt-compatible levels
				
				
			//pars.mParallelNasalPoleAmpdb  = AMP_ADJ + tp[ELM_AN];
				
			mFrame.mBypassFricationAmpdb = AMP_ADJ + (int)tp[ELM_AB];
			mFrame.mFormant5Ampdb = AMP_ADJ + (int)tp[ELM_A5];
			mFrame.mFormant6Ampdb = AMP_ADJ + (int)tp[ELM_A6];
			mFrame.mFormant1Ampdb = AMP_ADJ + (int)tp[ELM_A1];
			mFrame.mFormant2Ampdb = AMP_ADJ + (int)tp[ELM_A2];
			mFrame.mFormant3Ampdb = AMP_ADJ + (int)tp[ELM_A3];
			mFrame.mFormant4Ampdb = AMP_ADJ + (int)tp[ELM_A4];

			parwave(samp);

			samp += mNspFr;

			// Declination of f0 envelope 0.25Hz / cS 
			mTop -= mBaseDeclination;// 0.5;
		}
	}

	mLastElement = currentElement;

	return (int)(samp - aSamplePointer);
}


void klatt::init(int aBaseFrequency, float aBaseSpeed, float aBaseDeclination, int aBaseWaveform)
{
	mBaseF0 = aBaseFrequency;
	mBaseSpeed = aBaseSpeed;
	mBaseDeclination = aBaseDeclination;
	mBaseWaveform = aBaseWaveform;

    mSampleRate = 11025;
    mF0Flutter = 0;
	mF0FundamentalFreq = mBaseF0;
	mFrame.mF0FundamentalFreq = mBaseF0;

	int FLPhz = (950 * mSampleRate) / 10000;
	int BLPhz = (630 * mSampleRate) / 10000;
	mNspFr = (int)(mSampleRate * mBaseSpeed) / 1000; 

	mDownSampLowPassFilter.initResonator(FLPhz, BLPhz, mSampleRate);

	mNPer = 0;                        /* LG */
	mT0 = 0;                          /* LG */

	mVLast = 0;                       /* Previous output of voice  */
	mNLast = 0;                       /* Previous output of random number generator  */
	mGlotLast = 0;                    /* Previous value of glotout  */
}
