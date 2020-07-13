/*
SoLoud audio engine
Copyright (c) 2013-2018 Jari Komppa

vizsn speech synthesizer (c) by Ville-Matias Heikkilä,
released under WTFPL, http://www.wtfpl.net/txt/copying/
(in short, "do whatever you want to")

Integration and changes to work with SoLoud by Jari Komppa,
released under same license.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "soloud_vizsn.h"

/*

 lähtöfunktiot: voice, noise
 muut:          pitch
 volyymit:      voice, asp, fric, bypass (4kpl)
 resonaattorit: lp, nz (ar), npc, 1p, 2p, 3p, 4p, 5p, 6p, out (10kpl)


   [voice] [noise]
   <pitch>    |
      |       |
      |       |
   (re.LP)    |
      |       |
   (*voice) (*asp)
      |       |
      `--[+]--'
          |
       (ar.NZ)
          |
       (re.NPC)
          |
   .-----------.    [noise]
   |           |       |
(re.1P)     (diff)  (*fric)
   |           |       |
   |          [+]------'
   |           |
  [-]-(re.4P)--[
   |           |
  [-]-(re.3P)--[   <-- selvitäänkö kahdella tuossa vaiheessa?
   |           |
  [-]-(re.2P)--[
   |           |
   |        (*bypass)
   |           |
   `----[-]----'
         |'
      (re.OUT)
         |
        out

    resonaattori:

    x = a*input + b*y + c*z

    ja tulos kiertää x=>y=>z

    antiresonaattori:

    x = a*input + b*y + c*z

    inputti kiertää i=>y=>z
*/

#define RLP 0
#define RNZ 1
#define RNPC 2
#define ROUT 3
#define R1P 4
#define R2P 5
#define R3P 6
#define R4P 7
#define R5P 8
#define R6P 9
#define P_A 0
#define P_AE 1
#define P_E 2
#define P_OE 3
#define P_O 4
#define P_I 5
#define P_Y 6
#define P_U 7
#define P_H 8
#define P_V 9
#define P_J 10
#define P_S 11
#define P_L 12
#define P_R 13
#define P_K 14
#define P_T 15
#define P_P 16
#define P_N 17
#define P_M 18
#define P_NG 19
#define P_NEW 12
#define P_END -1
#define P_CLR -2

#define filter(i,v) i##last=i+(i##last*v);
#define plosive(a) ((((a) >= P_K) && ((a) <= P_P)) || ((a) == P_CLR))

static const float vowtab[8][4][2] =
{
	/* a */ 0.10f, 1.6f, 0, 0, 0.2f, 1.5f,  0, 0,
	/* ä */ 0.10f, 1.6f, 0, 0, 0.2f, 0,     0, 0,

	/* e */ 0.08f, 1.8f, 0, 0, 0.2f, -0.8f, 0, 0,
	/* ö */ 0.08f, 1.8f, 0, 0, 0.3f, 0.9f,  0, 0,
	/* o */ 0.08f, 1.8f, 0, 0, 0.2f, 1.6f,  0, 0,

	/* i */ 0.05f, 1.9f, 0, 0, 0.2f, -1.5f, 0, 0,
	/* y */ 0.05f, 1.9f, 0, 0, 0.2f, 0.8f,  0, 0,
	/* u */ 0.05f, 1.9f, 0, 0, 0.1f, 1.7f,  0, 0
};

/* integerisoi (8bit): kerro kolmella */
static const float voo[13][5 + 3 * 10] =
{
	/* frikatiivit & puolivokaalit */

	/* h */
	0.0f, 0.0f, 0.36f, 0.0f, 0.0f, 0.32f, 1.36f, -0.67f, 22.0f, -42.0f, 21.0f, 0.04f, 1.9f, -0.92f, 1.17f, 0.8f, -0.02f,
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

	/* v */
	0.7f, 0.0f, 0.0f, 0.0f, 0.0f, 0.32f, 1.36f, -0.67f, 22.0f, -42.0f, 21.0f, 0.04f, 1.9f, -0.92f, 1.17f, 0.8f, -0.02f,
	0.005f, 1.9f, -0.95f, 0.0f, 0.0f, 0.0f, 0.04f, 1.5f, -0.93f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

	/* j */
	0.7f, 0.0f, 0.0f, 0.0f, 0.0f, 0.32f, 1.36f, -0.67f, 22.0f, -42.0f, 21.0f, 0.04f, 1.9f, -0.92f, 1.17f, 0.8f, -0.02f,
	0.005f, 1.9f, -0.95f, 0.0f, 0.0f, 0.0f, 0.04f, -1.5f, -0.93f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

	/* s */
	0.0f, 0.0f, 0.36f, 0.0f, 0.0f, 0.32f, 1.36f, -0.67f, 22.0f, -42.0f, 21.0f, 0.04f, 1.9f, -0.92f, 1.7f, 0.1f, -0.02f,
	0.0f, 0.1f, -0.02f, 0.0f, 1.76f, -0.85f, 0.01f, 0.42f, -0.93f, 0.02f, -1.37f, -0.68f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

	/* l */
	0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.32f, 1.36f, -0.67f, 22.0f, -42.0f, 21.0f, 0.04f, 1.88f, -0.92f, 1.17f, 0.8f, -0.02f,
	0.1f, -0.5f, -0.93f, 0.6f, 1.0f, -0.93f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

	/* r */
	0.8f, 0.4f, 0.0f, 0.0f, 0.0f, 0.32f, 1.36f, -0.67f, 22.0f, -42.0f, 21.0f, 0.04f, 1.9f, -0.92f, 1.17f, 0.8f, -0.02f,
	-0.2f, 0.0f, -0.93f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

	/* klusiilit */

	/* k */
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.32f, 1.36f, -0.67f, 22.0f, -42.0f, 21.0f, 0.04f, 1.9f, -0.92f, 1.17f, 0.8f, -0.02f,
	0.0f, 1.95f, -0.94f, 0.4f, 1.0f, -0.93f, 0.6f, 1.0f, -0.89f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

	/* t */
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.32f, 1.36f, -0.67f, 22.0f, -42.0f, 21.0f, 0.04f, 1.9f, -0.92f, 1.17f, 0.8f, -0.02f,
	0.0f, 1.6f, -0.94f, 0.0f, 0.3f, -0.93f, 1.5f, -0.6f, -0.89f, 1.8f, -1.5f, -0.68f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

	/* p */
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.32f, 1.36f, -0.67f, 22.0f, -42.0f, 21.0f, 0.04f, 1.9f, -0.92f, 1.17f, 0.8f, -0.02f,
	0.01f, 1.9f, -0.94f, 1.5f, 1.7f, -0.93f, 1.0f, 1.0f, -0.89f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

	/* nasaalit */

	/* n */
	0.81f, 0.0f, 0.0f, 0.0f, 0.0f, 0.32f, 1.36f, -0.67f, 15.0f, -20.0f, 10.0f, 0.04f, 1.88f, -0.92f, 1.17f, 0.1f, -0.02f,
	0.02f, 1.83f, -0.97f, 0.10f, 0.26f, -0.83f, 0.06f, -0.85f, -0.82f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

	/* m */
	0.81f, 0.0f, 0.0f, 0.0f, 0.0f, 0.32f, 1.36f, -0.67f, 15.0f, -20.0f, 10.0f, 0.04f, 1.88f, -0.92f, 1.17f, 0.1f, -0.02f,
	0.08f, 1.5f, -0.94f, 0.06f, 1.0f, -0.9f, 0.05f, 1.5f, -0.89f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

	/* ng */
	0.81f, 0.0f, 0.0f, 0.0f, 0.0f, 0.32f, 1.36f, -0.67f, 15.0f, -20.0f, 10.0f, 0.04f, 1.88f, -0.92f, 1.17f, 0.1f, -0.02f,
	0.1f, 1.6f, -0.94f, 0.7f, -1.0f, -0.91f, 0.1f, 1.16f, -0.91f, 0.03f, -1.3f, -0.68f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
};

static const char keyz[] =
{
	P_A,  P_P,  P_S,  P_T,  P_E,  P_V,  P_NG,
	P_H,  P_I,  P_J,  P_K,  P_L,  P_M,  P_N,
	P_O,  P_P,  P_K,  P_R,  P_S,  P_T,  P_U,
	P_V,  P_U,  P_S,  P_Y,  P_S,  P_AE, P_OE
};

namespace SoLoud
{
	float VizsnResonator::resonate(float i)
	{
		float x = (a * i) + (b * p1) + (c * p2);
		p2 = p1;
		p1 = x;
		return x;
	}

	float VizsnResonator::antiresonate(float i)
	{
		float x = a * i + b * p1 + c * p2;
		p2 = p1;
		p1 = i;
		return x;
	}
	VizsnInstance::VizsnInstance(Vizsn *aParent)
	{
		mParent = aParent;
		mPtr = 0;
		mCurrentVoiceType = 6;
		memset(mEchobuf, 0, 1024 * sizeof(int));
		mPitch = 800;
		mS = mParent->mText;
		mBufwrite = 0;
		mBufread = 0;
		mA = 0;
		mB = 100;
		mOrgv = -1;
		mGlotlast = 0;
	}

	VizsnInstance::~VizsnInstance()
	{
	}

	unsigned int VizsnInstance::getAudio(float *aBuffer, unsigned int aSamplesToRead, unsigned int /*aBufferSize*/)
	{
		unsigned int idx = 0;
		int i, j;
		if (mBufwrite > mBufread)
		{
			for (; mBufwrite > mBufread && idx < aSamplesToRead; mBufread++)
			{
				aBuffer[idx] = mBuf[mBufread];
				idx++;
			}
		}
		if (idx == aSamplesToRead) return aSamplesToRead;
		mBufwrite = mBufread = 0;
		while (idx + mBufwrite < aSamplesToRead)
		{
			setphone(&mBank0, *mS, mPitch);

			if (*mS == P_END)
			{
				mBuf[mBufwrite] = 0;
				mBufwrite++;
				SOLOUD_ASSERT(mBufwrite < 2048);
				break;
			}

			setphone(&mBank1, mS[1], mPitch);//pitch+=50);  -- RAISE SOUND

			slidePrepare(50);

			mNper = 0;

			for (i = plosive(*mS) ? 100 : 300; i; i--)
			{
				mBuf[mBufwrite] = genwave();
				mBufwrite++;
				SOLOUD_ASSERT(mBufwrite < 2048);
			}

			if (!plosive(mS[1]))
			{
				for (i = 50; i; i--)
				{
					for (j = 10; j; j--)
					{
						mBuf[mBufwrite] = genwave();
						mBufwrite++;
						SOLOUD_ASSERT(mBufwrite < 2048);
					}
					slideTick();
				}
			}
			else
			{
				for (i = 50; i; i--)
				{
					for (j = 3; j; j--)
					{
						mBuf[mBufwrite] = genwave();
						mBufwrite++;
						SOLOUD_ASSERT(mBufwrite < 2048);
					}
				}
			}

			mS++;

			memcpy(&mBank0, &mBank1, sizeof(VizsnBank));
		}
		for (; idx < aSamplesToRead; idx++)
		{
			aBuffer[idx] = mBuf[mBufread];
			mBufread++;
		}
		return aSamplesToRead;
	}

	float VizsnInstance::vcsrc(int aPitch, int aVoicetype)
	{		
		mA += aPitch;

		if (mOrgv != aVoicetype)
		{
			mOrgv = aVoicetype;
			mA = 0;
			mB = 100;
		}

		switch (aVoicetype)
		{
		case 0:	return (mA & (256 + 128 + 32)) * 5 * 0.0002f;
		case 1:	return (float)(sin(mA * 0.0002) * cos(mA * 0.0003) * 0.2f + ((rand() % 200 - 100) / 300.0f)); // ilmava
		case 2: return (float)tan(mA*0.00002)*0.01f; // burpy
		case 3: return ((mA & 65535) > 32768 ? 65535 : 0) * 0.00001f; // square wave
		case 4: return (float)mA * (float)mA * 0.0000000002f; // kuisku
		case 5:	mA += 3; mB++; return ((mA & 255) > ((mB >> 2) & 255)) ? 0.3f : 0.0f;
		case 7:	return ((mA >> 8) & (256 + 128)) * 0.001f; // robottipulssi
		case 8:	return (float)(rand() % (1 + ((mA & 65535) >> 8))) / 256; // -- hiukka ihmisempi tsaatana
		case 9:	return ((float)(rand() & 32767)) / 32767; // -- noise: tsaatana
		case 6: // fallthrough
		default: return (mA & 65535) * (0.001f / 256); /*-- sawtooth: near natural */
		}
	}

	float VizsnInstance::noisrc()
	{
		return ((float)(rand() & 32767)) / 32768;
	}

	float VizsnInstance::genwave()
	{		
		float s, o, noise, voice, glot, parglot;
		int ob;

		noise = noisrc();

		if (mNper > mNmod)
		{
			noise *= 0.5f;
		}

		s = mBank0.frica * noise;

		voice = vcsrc((int)floor(mBank0.pitch), mCurrentVoiceType);
		voice = mBank0.r[RLP].resonate(voice);

		if (mNper < mNopen)
		{
			voice += mBank0.breth * noise;
		}

		parglot = glot = (mBank0.voice * voice) + (mBank0.aspir * noise);

		parglot = mBank0.r[RNZ].antiresonate(parglot);
		parglot = mBank0.r[RNPC].resonate(parglot);
		s += (parglot - mGlotlast);
		mGlotlast = parglot;
		o = mBank0.r[R1P].resonate(parglot);

		int i;
		for (i = R4P; i > R2P; i--)
		{
			o = mBank0.r[i].resonate(s) - o;
		}

		o = mBank0.r[ROUT].resonate(mBank0.bypas * s - o);

		/*********/

		ob = (int)floor(o * 400 * 256 + (mEchobuf[mPtr] / 4));
		mEchobuf[mPtr] = ob;
		mPtr = (mPtr + 1) & 1023;

		ob = (ob >> 8) + 128;

		if (ob < 0)	ob = 0;
		if (ob > 255) ob = 255;

		mNper++;

		return ob * (1.0f / 255.0f);
	}

	void VizsnInstance::setphone(VizsnBank *aB, char aP, float /*aPitch*/)
	{
		int i;
		aB->frica = aB->aspir = aB->bypas = aB->breth = aB->voice = 0;
		aB->pitch = mPitch;

		if (aP < 0)
		{
			for (i = 0; i < 10; i++)
			{
				aB->r[i].p1 = aB->r[i].p2 = aB->r[i].a = aB->r[i].b = aB->r[i].c = 0;
			}
		}
		else
		{
			if (aP < 8)
			{
				/* vokaali */
				VizsnResonator *r = aB->r;
				const float *s = vowtab[aP][0];

				r[R1P].c = -0.95f; r[R2P].c = -0.93f; r[R3P].c = -0.88f; r[R4P].c = -0.67f;
				r[RLP].a = 0.31f;  r[RLP].b = 1.35f;  r[RLP].c = -0.67f;
				r[RNZ].a = 22.0f;  r[RNZ].b = -42.0f; r[RNZ].c = 21.0f;
				r[RNPC].a = 0.04f; r[RNPC].b = 1.87f; r[RNPC].c = -0.92f;
				r[ROUT].a = 1.16f; r[ROUT].b = 0.08f; r[ROUT].c = -0.02f;

				r += R1P;

				for (i = 4; i; i--)
				{
					r->a = *s++;
					r->b = *s++;
					r++;
				}

				aB->voice = 0.8f;
			}
			else
			{
				/* v */
				const float *v = voo[aP - 8];

				aB->voice = *v++;
				aB->aspir = *v++;
				aB->frica = *v++;
				aB->bypas = *v++;
				aB->breth = *v++;

				int j;
				for (j = 0; j < 10; j++)
				{
					aB->r[j].a = *v++;
					aB->r[j].b = *v++;
					aB->r[j].c = *v++;
				}

				aB->voice = 0.8f;

				aB->breth = 0.18f;
			}
		}
	}

	void VizsnInstance::slidePrepare(int aNumtix)
	{
		int i;

		for (i = 0; i < 10; i++)
		{
			mBank0to1.r[i].a = (mBank1.r[i].a - mBank0.r[i].a) / aNumtix;
			mBank0to1.r[i].b = (mBank1.r[i].b - mBank0.r[i].b) / aNumtix;
			mBank0to1.r[i].c = (mBank1.r[i].c - mBank0.r[i].c) / aNumtix;
		}

		mBank0to1.pitch = (mBank1.pitch - mBank0.pitch) / aNumtix;
	}

	void VizsnInstance::slideTick()
	{
		int i;

		for (i = 0; i < 10; i++)
		{
			mBank0.r[i].a += mBank0to1.r[i].a;
			mBank0.r[i].b += mBank0to1.r[i].b;
			mBank0.r[i].c += mBank0to1.r[i].c;
		}

		mBank0.pitch += mBank0to1.pitch;
	}

	bool VizsnInstance::hasEnded()
	{
		return *mS == P_END;
	}

	Vizsn::Vizsn()
	{
		mBaseSamplerate = 8000;
		mText = 0;
	}

	Vizsn::~Vizsn()
	{
		stop();
	}

	AudioSourceInstance * Vizsn::createInstance()
	{
		return new VizsnInstance(this);
	}

	void Vizsn::setText(char *aText)
	{
		if (!aText)
			return;
		stop();
		delete[] mText;
		int len = (int)strlen(aText);
		mText = new char[len + 3];
		memcpy(mText+1, aText, len);
		mText[0] = P_CLR;
		int i;
		for (i = 0; i < len; i++)
		{
			int c = mText[i + 1];
			if (c == '\x84' || c == -124) c = '{'; // ä
			if (c == '\x94' || c == -108) c = '|'; // ö
			if (c >= 'a' && c <= '|')
			{
				mText[i + 1] = keyz[c - 'a'];
			}
			else
			{
				mText[i + 1] = P_CLR;
			}
		}
		mText[len + 1] = P_END;
		mText[len + 2] = 0;
	}

};
