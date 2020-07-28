/*
TED/SID module for SoLoud audio engine
Copyright (c) 2015 Jari Komppa

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
#include "sid.h"
#include "ted.h"
#include "soloud_tedsid.h"
#include "soloud_file.h"

namespace SoLoud
{

	TedSidInstance::TedSidInstance(TedSid *aParent)
	{
		mParent = aParent;
		mSampleCount = 0;
		mSID = new SIDsound(mParent->mModel, 0);
		mSID->setFrequency(0);
		mSID->setSampleRate(TED_SOUND_CLOCK);		
		mSID->setFrequency(1);

		mTED = new TED();
		mTED->oscillatorInit();

		mNextReg = 100; // NOP
		mNextVal = 0;
		int i;
		for (i = 0; i < 128; i++)
			mRegValues[i] = 0;
	}

	unsigned int TedSidInstance::getAudio(float *aBuffer, unsigned int aSamplesToRead, unsigned int /*aBufferSize*/)
	{
		unsigned int i;
		for (i = 0; i < aSamplesToRead; i++)
		{
		    tick();
			short sample;
			mSID->calcSamples(&sample, 1);
			short tedsample = 0;
			mTED->renderSound(1, &tedsample);
			aBuffer[i] = (sample + tedsample) / 8192.0f;
			mSampleCount--;
		}
		return aSamplesToRead;
	}
	
	void TedSidInstance::tick()
	{
	    if (mParent->mFile == 0)
	        return;

		while (mSampleCount == 0)
		{
			mRegValues[mNextReg] = mNextVal;
			if (mNextReg < 64)
			{
				mSID->write(mNextReg, (unsigned char)mNextVal);
			}
			else
			if (mNextReg < 64 + 5)
			{
				mTED->writeSoundReg(mNextReg - 64, (unsigned char)mNextVal);
			}
//			mSampleCount = mParent->mFile->read16();
			mNextVal = mParent->mFile->read8();
			mNextReg = mParent->mFile->read8();
			if (mNextReg & 0x80)
			{
				// timestamp!
				mSampleCount = ((int)(mNextReg & 0x7f) << 8) | mNextVal;
				mNextVal = mParent->mFile->read8();
				mNextReg = mParent->mFile->read8();
			}
			if (mParent->mFile->eof())
				mParent->mFile->seek(8);
		}
	}

	float TedSidInstance::getInfo(unsigned int aInfoKey)
	{
		return (float)mRegValues[aInfoKey & 127];
	}

	bool TedSidInstance::hasEnded()
	{
		return 0;
	}

	TedSidInstance::~TedSidInstance()
	{
		delete mSID;
		delete mTED;
	}

	TedSid::TedSid()
	{
		mBaseSamplerate = TED_SOUND_CLOCK;
		mChannels = 1;
		mFile = 0;
		mFileOwned = false;
		mModel = 0;
	}

	TedSid::~TedSid()
	{
		stop();
		if (mFileOwned)
			delete mFile;
	}

	result TedSid::loadMem(const unsigned char *aMem, unsigned int aLength, bool aCopy, bool aTakeOwnership)
	{
		if (!aMem || aLength == 0)
			return INVALID_PARAMETER;
		MemoryFile *mf = new MemoryFile;
		if (!mf)
			return OUT_OF_MEMORY;
		int res = mf->openMem(aMem, aLength, aCopy, aTakeOwnership);
		if (res != SO_NO_ERROR)
		{
			delete mf;
			return res;
		}
		res = loadFile(mf);
		if (res != SO_NO_ERROR)
		{
			delete mf;
			return res;
		}
		mFileOwned = aCopy || aTakeOwnership;

		return SO_NO_ERROR;
	}

	result TedSid::load(const char *aFilename)
	{
		if (!aFilename)
			return INVALID_PARAMETER;
		DiskFile *df = new DiskFile;
		if (!df) return OUT_OF_MEMORY;
		int res = df->open(aFilename);
		if (res != SO_NO_ERROR)
		{
			delete df;
			return res;
		}
		res = loadFile(df);
		if (res != SO_NO_ERROR)
		{
			delete df;
			return res;
		}
		mFileOwned = true;				
		return SO_NO_ERROR;
	}

	result TedSid::loadToMem(const char *aFilename)
	{
		if (!aFilename)
			return INVALID_PARAMETER;
		MemoryFile *mf = new MemoryFile;
		if (!mf) return OUT_OF_MEMORY;
		int res = mf->openToMem(aFilename);
		if (res != SO_NO_ERROR)
		{
			delete mf;
			return res;
		}
		res = loadFile(mf);
		if (res != SO_NO_ERROR)
		{
			delete mf;
			return res;
		}
		mFileOwned = true;
		return SO_NO_ERROR;
	}

	result TedSid::loadFileToMem(File *aFile)
	{
		if (!aFile)
			return INVALID_PARAMETER;
		MemoryFile *mf = new MemoryFile;
		if (!mf) return OUT_OF_MEMORY;
		int res = mf->openFileToMem(aFile);
		if (res != SO_NO_ERROR)
		{
			delete mf;
			return res;
		}
		res = loadFile(mf);
		if (res != SO_NO_ERROR)
		{
			delete mf;
			return res;
		}
		mFileOwned = true;
		return SO_NO_ERROR;
	}

	result TedSid::loadFile(File *aFile)
	{
		if (aFile == NULL)
			return INVALID_PARAMETER;
		if (mFileOwned)
			delete mFile;
		// Expect a file wih header and at least one reg write
		if (aFile->length() < 4+4+2+2) return FILE_LOAD_FAILED;

		aFile->seek(0);
		if (aFile->read8() != 'D') return FILE_LOAD_FAILED;
		if (aFile->read8() != 'u') return FILE_LOAD_FAILED;
		if (aFile->read8() != 'm') return FILE_LOAD_FAILED;
		if (aFile->read8() != 'p') return FILE_LOAD_FAILED;
		if (aFile->read8() != 0) return FILE_LOAD_FAILED;
		mModel = aFile->read8();
		aFile->seek(8);

		mFile = aFile;
		mFileOwned = false;


		return SO_NO_ERROR;
	}


	AudioSourceInstance * TedSid::createInstance() 
	{
		return new TedSidInstance(this);
	}

};