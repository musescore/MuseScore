/*
MONOTONE module for SoLoud audio engine
Copyright (c) 2013-2020 Jari Komppa

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
#include "soloud_monotone.h"
#include "soloud_file.h"

namespace SoLoud
{

	MonotoneInstance::MonotoneInstance(Monotone *aParent)
	{
		mParent = aParent;
		mOrder = 0;
		mRow = 0;
		mTempo = 4;
		mSampleCount = 0;
		mNextChannel = 0;
		mRowTick = 0;
		int i;
		for (i = 0; i < 12; i++)
		{
			mOutput[i].mSamplePos = 0;
			mOutput[i].mSamplePosInc = 0;
			mOutput[i].mEnabled = i < mParent->mHardwareChannels && i < mParent->mSong.mTotalTracks;			
			mChannel[i].mEnabled = i < mParent->mSong.mTotalTracks;
			mChannel[i].mActive = 0;
			mChannel[i].mArpCounter = 0;
			mChannel[i].mLastNote = 0;
			mChannel[i].mPortamentoToNote = 0;
			mChannel[i].mArp = 0;
			mChannel[i].mVibrato = 0;
			mChannel[i].mVibratoIndex = 0;
			mChannel[i].mVibratoDepth = 1;
			mChannel[i].mVibratoSpeed = 1;
		}
	}

	unsigned int MonotoneInstance::getAudio(float *aBuffer, unsigned int aSamplesToRead, unsigned int /*aBufferSize*/)
	{
		int samplesPerTick = (int)floor(mSamplerate / 60);
		unsigned int i;
		for (i = 0; i < 12; i++)
		{
			mOutput[i].mEnabled = i < (unsigned int)mParent->mHardwareChannels && i < (unsigned int)mParent->mSong.mTotalTracks;
		}
		for (i = 0; i < aSamplesToRead; i++)
		{
			if ((mSampleCount % samplesPerTick) == 0)
			{
				// new tick
				mRowTick++;
				if (mRowTick >= mTempo)
				{
					mRowTick = 0;
					// Process row
					int patternjump = mOrder + 1;
					int rowjump = 0;
					int dojump = 0;
					int pattern = mParent->mSong.mOrder[mOrder];
					int j;
					for (j = 0; j < mParent->mSong.mTotalTracks; j++)
					{
						unsigned int d = mParent->mSong.mPatternData[(pattern * 64 + mRow) * mParent->mSong.mTotalTracks + j];
						unsigned int note = (d >> 9) & 127;
						unsigned int effect = (d >> 6) & 7;
						unsigned int effectdata = (d)& 63;
						unsigned int effectdata1 = (d >> 3) & 7;
						unsigned int effectdata2 = (d >> 0) & 7;

						// by default, effects are off, and have to be set on every row.
						mChannel[j].mPortamento = 0;
						mChannel[j].mArp = 0;
						mChannel[j].mVibrato = 0;
						
						int oldhz = mChannel[j].mFreq[0];

						if (note == 127)
						{
							// noteEnd
							mChannel[j].mActive = 0;
							mChannel[j].mFreq[0] = 0;
							mChannel[j].mFreq[1] = 0;
							mChannel[j].mFreq[2] = 0;
							mChannel[j].mPortamento = 0;
							mChannel[j].mLastNote = 0;
						}
						else
						if (note != 0)
						{
							mChannel[j].mActive = 1;
							mChannel[j].mFreq[0] = mParent->mNotesHz[note * 8];
							mChannel[j].mFreq[1] = mChannel[j].mFreq[0];
							mChannel[j].mFreq[2] = mChannel[j].mFreq[0];
							mChannel[j].mPortamento = 0;
							mChannel[j].mLastNote = note;
							mChannel[j].mVibratoIndex = 0;
						}
						else
						if (note == 0)
						{
							note = mChannel[j].mLastNote;
						}

						switch (effect)
						{
						case 0x0:
							// arp
							mChannel[j].mFreq[1] = mParent->mNotesHz[(note + effectdata1) * 8];
							mChannel[j].mFreq[2] = mParent->mNotesHz[(note + effectdata2) * 8];
							if (effectdata1 || effectdata2)
								mChannel[j].mArp = 1;
							break;
						case 0x1:
							// portamento up
							mChannel[j].mPortamento = effectdata;
							break;
						case 0x2:
							// portamento down
							mChannel[j].mPortamento = -(signed)effectdata;
							break;
						case 0x3:
							// portamento to note
							mChannel[j].mPortamentoToNote = mParent->mNotesHz[note * 8];
							if (oldhz != mChannel[j].mPortamentoToNote)
							{
								mChannel[j].mFreq[0] = oldhz;
								mChannel[j].mPortamento = effectdata;
								if (oldhz > mChannel[j].mPortamentoToNote)
									mChannel[j].mPortamento *= -1;							
							}
							else
							{
								mChannel[j].mPortamentoToNote = 0;
							}
							break;
						case 0x4:
							// vibrato
							mChannel[j].mVibrato = 1;
							if (effectdata2 != 0) mChannel[j].mVibratoDepth = effectdata2;
							if (effectdata1 != 0) mChannel[j].mVibratoSpeed = effectdata1;
							break;
						case 0x5:
							// pattern jump
							patternjump = effectdata;
							dojump = 1;
							break;
						case 0x6:
							// row jump
							rowjump = effectdata;
							dojump = 1;
							break;
						case 0x7:
							// set speed
							mTempo = effectdata;
							break;
						}
					}					
					
					mRow++;

					if (dojump)
					{
						mRow = rowjump;
						mOrder = patternjump;
					}

					if (mRow == 64)
					{
						mRow = 0;
						mOrder++;
						if (mParent->mSong.mOrder[mOrder] == 0xff)
							mOrder = 0;
					}
				}

				int j;

				// per tick events
				for (j = 0; j < mParent->mSong.mTotalTracks; j++)
				{
					if (mChannel[j].mActive)
					{
						if (mChannel[j].mVibrato)
						{
							mChannel[j].mFreq[0] = mParent->mNotesHz[mChannel[j].mLastNote * 8 + (mParent->mVibTable[mChannel[j].mVibratoIndex] * mChannel[j].mVibratoDepth) / 64];
							mChannel[j].mVibratoIndex += mChannel[j].mVibratoSpeed;
							mChannel[j].mVibratoIndex %= 32;
						}
						if (mChannel[j].mPortamento && mRowTick != 0)
						{
							mChannel[j].mFreq[0] += mChannel[j].mPortamento;
							if (mChannel[j].mPortamentoToNote)
							{
								if ((mChannel[j].mPortamento > 0 && mChannel[j].mFreq[0] >= mChannel[j].mPortamentoToNote) ||
   									(mChannel[j].mPortamento < 0 && mChannel[j].mFreq[0] <= mChannel[j].mPortamentoToNote))
								{
									mChannel[j].mFreq[0] = mChannel[j].mPortamentoToNote;
									mChannel[j].mPortamentoToNote = 0;
								}
							}
						}
					}
				}

				// Channel fill

				int gotit = 0;
				int tries = 0;

				for (j = 0; j < mParent->mHardwareChannels; j++)
					mOutput[j].mSamplePosInc = 0;

				while (gotit < mParent->mHardwareChannels && tries < mParent->mSong.mTotalTracks)
				{
					if (mChannel[mNextChannel].mActive)
					{
						if (mChannel[mNextChannel].mArp)
						{
							mOutput[gotit].mSamplePosInc = 1.0f / (mSamplerate / mChannel[mNextChannel].mFreq[mChannel[mNextChannel].mArpCounter]);
							mChannel[mNextChannel].mArpCounter++;
							mChannel[mNextChannel].mArpCounter %= 3;
						}
						else
						{
							mOutput[gotit].mSamplePosInc = 1.0f / (mSamplerate / mChannel[mNextChannel].mFreq[0]);
						}
						gotit++;
					}
					mNextChannel++;
					mNextChannel %= mParent->mSong.mTotalTracks;
					tries++;
				}								
			}
			
			aBuffer[i] = 0;
			int j;
			for (j = 0; j < 12; j++)
			{
				if (mOutput[j].mEnabled)
				{
					float bleh = mOutput[j].mSamplePos + mOutput[j].mSamplePosInc;
					mOutput[j].mSamplePos = bleh - (long)bleh;					
					aBuffer[i] += SoLoud::Misc::generateWaveform(mParent->mWaveform, mOutput[j].mSamplePos) * 0.5f;
				}
			}

			mSampleCount++;
		}
		return aSamplesToRead;
	}

	bool MonotoneInstance::hasEnded()
	{
		return 0;
	}

	Monotone::Monotone()
	{
		int i;
		float temphz = 27.5f;
		int IBO = 12; // Intervals Between Octaves
		int IBN = 8; // Intervals Between Notes
		// Fun fact: the resulting constant is pretty 
		// close to Proton Mass consntant 1.00727646688
		float interval = 1.00724641222f;//exp(ln(2)/(IBO*IBN));
		int maxnote = 3 + (8 * IBO) + 1;

		mNotesHz[0] = 440;
		mNotesHz[1 * IBN] = (int)floor(temphz + 0.5f);

		for (i = (1 * IBN) - 1; i > 1; i--)
		{
			temphz = temphz / interval;
			if (temphz < 19) temphz = 19; // orig limitation, we could go lower though
			mNotesHz[i] = (int)floor(temphz + 0.5f);
		}
		temphz = 27.5f;
		for (i = (1 * IBN) + 1; i < maxnote * IBN; i++)
		{
			temphz = temphz * interval;
			mNotesHz[i] = (int)floor(temphz + 0.5f);
		}

		for (i = 0; i < 32; i++)
			mVibTable[i] = (int)floor(0.5 + 64 * sin(i * M_PI / 32 * 2));

		mSong.mTitle = 0;
		mSong.mComment = 0;
		mSong.mPatternData = 0;

		mBaseSamplerate = 44100;
		mChannels = 1;

		mHardwareChannels = 1;
		mWaveform = SoLoud::Misc::WAVE_SQUARE;
	}

	void Monotone::clear()
	{
		stop();

		delete[] mSong.mTitle;
		delete[] mSong.mComment;
		delete[] mSong.mPatternData;

		mSong.mTitle = 0;
		mSong.mComment = 0;
		mSong.mPatternData = 0;
	}

	Monotone::~Monotone()
	{
		stop();
		clear();
	}

	static char * mystrdup(const char *src)
	{
		int len = (int)strlen(src);
		char * res = new char[len + 1];
		memcpy(res, src, len);
		res[len] = 0;
		return res;
	}

	result Monotone::setParams(int aHardwareChannels, int aWaveform)
	{
		if (aHardwareChannels <= 0 || aWaveform < 0)
			return INVALID_PARAMETER;
		mHardwareChannels = aHardwareChannels;
		mWaveform = aWaveform;
		return SO_NO_ERROR;
	}
	
	result Monotone::loadMem(const unsigned char *aMem, unsigned int aLength, bool aCopy, bool aTakeOwnership)
	{
		MemoryFile mf;
		int res = mf.openMem(aMem, aLength, aCopy, aTakeOwnership);
		if (res != SO_NO_ERROR)
			return res;
		return loadFile(&mf);
	}

	result Monotone::load(const char *aFilename)
	{
		DiskFile df;
		int res = df.open(aFilename);
		if (res != SO_NO_ERROR)
			return res;
		return loadFile(&df);
	}

	result Monotone::loadFile(File *aFile)
	{
		if (aFile == NULL)
			return INVALID_PARAMETER;
		clear();
		int i;
		unsigned char temp[200];
		aFile->read(temp, 9);
		char magic[] = "\bMONOTONE";
		for (i = 0; i < 9; i++)
		{
			if (temp[i] != magic[i])
			{
				return FILE_LOAD_FAILED;
			}
		}
		aFile->read(temp, 41);
		temp[temp[0] + 1] = 0; // pascal -> asciiz: pascal strings have length as first byte
		mSong.mTitle = mystrdup((char*)temp + 1);
		aFile->read(temp, 41);
		temp[temp[0] + 1] = 0; // pascal -> asciiz: pascal strings have length as first byte
		mSong.mComment = mystrdup((char*)temp + 1);
		aFile->read(temp, 4);
		mSong.mVersion = temp[0];
		mSong.mTotalPatterns = temp[1];
		mSong.mTotalTracks = temp[2];
		mSong.mCellSize = temp[3];
		if (mSong.mVersion != 1 || mSong.mCellSize != 2)
		{
			return FILE_LOAD_FAILED;
		}
		aFile->read(mSong.mOrder, 256);
		int totalnotes = 64 * mSong.mTotalPatterns * mSong.mTotalTracks;
		mSong.mPatternData = new unsigned int[totalnotes];
		for (i = 0; i < totalnotes; i++)
		{
			aFile->read(temp, 2);
			unsigned int datavalue = temp[0] | (temp[1] << 8);
			mSong.mPatternData[i] = datavalue;
			//unsigned int note = (datavalue >> 9) & 127;
			//unsigned int effect = (datavalue >> 6) & 7;
			//unsigned int effectdata = (datavalue)& 63;
			//unsigned int effectdata1 = (datavalue >> 3) & 7;
			//unsigned int effectdata2 = (datavalue >> 0) & 7;
		}

		return SO_NO_ERROR;
	}


	AudioSourceInstance * Monotone::createInstance() 
	{
		return new MonotoneInstance(this);
	}

};