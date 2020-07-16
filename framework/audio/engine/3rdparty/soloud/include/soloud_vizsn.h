/*
SoLoud audio engine
Copyright (c) 2013-2018 Jari Komppa

vizsn speech synthesizer (c) by Ville-Matias Heikkilä,
released under WTFPL, http://www.wtfpl.net/txt/copying/
(in short, "do whatever you want to")

Integration and changes to work with SoLoud by Jari Komppa,
released under same license.
*/

#ifndef SOLOUD_VIZSN_H
#define SOLOUD_VIZSN_H

#include "soloud.h"

namespace SoLoud
{
	class Vizsn;

	struct VizsnResonator
	{
	public:
		float a, b, c, p1, p2;

		float resonate(float i);
		float antiresonate(float i);
	};

	struct VizsnBank
	{
		VizsnResonator r[10];
		float pitch;
		float frica, voice, aspir, bypas, breth;
	};

	class VizsnInstance : public AudioSourceInstance
	{
	public:
		VizsnInstance(Vizsn *aParent);
		~VizsnInstance();

		virtual unsigned int getAudio(float *aBuffer, unsigned int aSamplesToRead, unsigned int aBufferSize);
		virtual bool hasEnded();

    public:
        Vizsn *mParent;
		VizsnBank mBank0, mBank1, mBank0to1;
		int mNper, mNmod, mNopen;
		int mEchobuf[1024], mPtr;
		int mCurrentVoiceType;
		float mPitch;
		char *mS;
		float mBuf[2048];
		unsigned int mBufwrite;
		unsigned int mBufread;
		float vcsrc(int aPitch, int aVoicetype);
		float noisrc();
		float genwave();
		void setphone(VizsnBank *aB, char aP, float aPitch);
		void slidePrepare(int aNumtix);
		void slideTick();
		int mA;
		int mB;
		int mOrgv;
		float mGlotlast;
	};

	class Vizsn : public AudioSource
	{
	public:
		char *mText;
		Vizsn();
		virtual ~Vizsn();
		void setText(char *aText);
	public:
		virtual AudioSourceInstance *createInstance();
	};
};

#endif
