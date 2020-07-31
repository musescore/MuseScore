/*
SoLoud audio engine
Copyright (c) 2013-2018 Jari Komppa

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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_file.h"
#include "stb_vorbis.h"
#include "dr_mp3.h"
#include "dr_wav.h"
#include "dr_flac.h"

namespace SoLoud
{
	WavInstance::WavInstance(Wav *aParent)
	{
		mParent = aParent;
		mOffset = 0;
	}

	unsigned int WavInstance::getAudio(float *aBuffer, unsigned int aSamplesToRead, unsigned int aBufferSize)
	{		
		if (mParent->mData == NULL)
			return 0;

		unsigned int dataleft = mParent->mSampleCount - mOffset;
		unsigned int copylen = dataleft;
		if (copylen > aSamplesToRead)
			copylen = aSamplesToRead;

		unsigned int i;
		for (i = 0; i < mChannels; i++)
		{
			memcpy(aBuffer + i * aBufferSize, mParent->mData + mOffset + i * mParent->mSampleCount, sizeof(float) * copylen);
		}

		mOffset += copylen;
		return copylen;
	}

	result WavInstance::rewind()
	{
		mOffset = 0;
		mStreamPosition = 0.0f;
		return 0;
	}

	bool WavInstance::hasEnded()
	{
		if (!(mFlags & AudioSourceInstance::LOOPING) && mOffset >= mParent->mSampleCount)
		{
			return 1;
		}
		return 0;
	}

	Wav::Wav()
	{
		mData = NULL;
		mSampleCount = 0;
	}
	
	Wav::~Wav()
	{
		stop();
		delete[] mData;
	}

#define MAKEDWORD(a,b,c,d) (((d) << 24) | ((c) << 16) | ((b) << 8) | (a))

	result Wav::loadwav(MemoryFile *aReader)
	{
		drwav decoder;

		if (!drwav_init_memory(&decoder, aReader->getMemPtr(), aReader->length(),NULL))
		{
			return FILE_LOAD_FAILED;
		}

		drwav_uint64 samples = decoder.totalPCMFrameCount;

		if (!samples)
		{
			drwav_uninit(&decoder);
			return FILE_LOAD_FAILED;
		}

		mData = new float[(unsigned int)(samples * decoder.channels)];
		mBaseSamplerate = (float)decoder.sampleRate;
		mSampleCount = (unsigned int)samples;
		mChannels = decoder.channels;

		unsigned int i, j, k;
		for (i = 0; i < mSampleCount; i += 512)
		{
			float tmp[512 * MAX_CHANNELS];
			unsigned int blockSize = (mSampleCount - i) > 512 ? 512 : mSampleCount - i;
			drwav_read_pcm_frames_f32(&decoder, blockSize, tmp);
			for (j = 0; j < blockSize; j++)
			{
				for (k = 0; k < decoder.channels; k++)
				{
					mData[k * mSampleCount + i + j] = tmp[j * decoder.channels + k];
				}
			}
		}
		drwav_uninit(&decoder);

		return SO_NO_ERROR;
	}

	result Wav::loadogg(MemoryFile *aReader)
	{	
		int e = 0;
		stb_vorbis *vorbis = 0;
		vorbis = stb_vorbis_open_memory(aReader->getMemPtr(), aReader->length(), &e, 0);

		if (0 == vorbis)
		{
			return FILE_LOAD_FAILED;
		}

        stb_vorbis_info info = stb_vorbis_get_info(vorbis);
		mBaseSamplerate = (float)info.sample_rate;
        int samples = stb_vorbis_stream_length_in_samples(vorbis);

		if (info.channels > MAX_CHANNELS)
		{
			mChannels = MAX_CHANNELS;
		}
		else
		{
			mChannels = info.channels;
		}
		mData = new float[samples * mChannels];
		mSampleCount = samples;
		samples = 0;
		while(1)
		{
			float **outputs;
            int n = stb_vorbis_get_frame_float(vorbis, NULL, &outputs);
			if (n == 0)
            {
				break;
            }

			unsigned int ch;
			for (ch = 0; ch < mChannels; ch++)
				memcpy(mData + samples + mSampleCount * ch, outputs[ch], sizeof(float) * n);

			samples += n;
		}
        stb_vorbis_close(vorbis);

		return 0;
	}

	result Wav::loadmp3(MemoryFile *aReader)
	{
		drmp3 decoder;

		if (!drmp3_init_memory(&decoder, aReader->getMemPtr(), aReader->length(), NULL, NULL))
		{
			return FILE_LOAD_FAILED;
		}

		drmp3_uint64 samples = drmp3_get_pcm_frame_count(&decoder);

		if (!samples)
		{
			drmp3_uninit(&decoder);
			return FILE_LOAD_FAILED;
		}

		mData = new float[(unsigned int)(samples * decoder.channels)];
		mBaseSamplerate = (float)decoder.sampleRate;
		mSampleCount = (unsigned int)samples;
		mChannels = decoder.channels;
		drmp3_seek_to_pcm_frame(&decoder, 0); 

		unsigned int i, j, k;
		for (i = 0; i<mSampleCount; i += 512)
		{
			float tmp[512 * MAX_CHANNELS];
			unsigned int blockSize = (mSampleCount - i) > 512 ? 512 : mSampleCount - i;
			drmp3_read_pcm_frames_f32(&decoder, blockSize, tmp);
			for (j = 0; j < blockSize; j++) 
			{
				for (k = 0; k < decoder.channels; k++) 
				{
					mData[k * mSampleCount + i + j] = tmp[j * decoder.channels + k];
				}
			}
		}
		drmp3_uninit(&decoder);

		return SO_NO_ERROR;
	}

	result Wav::loadflac(MemoryFile *aReader)
	{
		drflac *decoder = drflac_open_memory(aReader->mDataPtr, aReader->mDataLength, NULL);

		if (!decoder)
		{
			return FILE_LOAD_FAILED;
		}

		drflac_uint64 samples = decoder->totalPCMFrameCount;

		if (!samples)
		{
			drflac_close(decoder);
			return FILE_LOAD_FAILED;
		}

		mData = new float[(unsigned int)(samples * decoder->channels)];
		mBaseSamplerate = (float)decoder->sampleRate;
		mSampleCount = (unsigned int)samples;
		mChannels = decoder->channels;
		drflac_seek_to_pcm_frame(decoder, 0);

		unsigned int i, j, k;
		for (i = 0; i < mSampleCount; i += 512)
		{
			float tmp[512 * MAX_CHANNELS];
			unsigned int blockSize = (mSampleCount - i) > 512 ? 512 : mSampleCount - i;
			drflac_read_pcm_frames_f32(decoder, blockSize, tmp);
			for (j = 0; j < blockSize; j++)
			{
				for (k = 0; k < decoder->channels; k++)
				{
					mData[k * mSampleCount + i + j] = tmp[j * decoder->channels + k];
				}
			}
		}
		drflac_close(decoder);

		return SO_NO_ERROR;
	}

    result Wav::testAndLoadFile(MemoryFile *aReader)
    {
		delete[] mData;
		mData = 0;
		mSampleCount = 0;
		mChannels = 1;
        int tag = aReader->read32();
		if (tag == MAKEDWORD('O','g','g','S')) 
        {
			return loadogg(aReader);

		} 
        else if (tag == MAKEDWORD('R','I','F','F')) 
        {
			return loadwav(aReader);
		}
		else if (tag == MAKEDWORD('f', 'L', 'a', 'C'))
		{
			return loadflac(aReader);
		}
		else if (loadmp3(aReader) == SO_NO_ERROR)
		{
			return SO_NO_ERROR;
		}

		return FILE_LOAD_FAILED;
    }

	result Wav::load(const char *aFilename)
	{
		if (aFilename == 0)
			return INVALID_PARAMETER;
		stop();
		DiskFile dr;
		int res = dr.open(aFilename);
		if (res == SO_NO_ERROR)
			return loadFile(&dr);
		return res;
	}

	result Wav::loadMem(const unsigned char *aMem, unsigned int aLength, bool aCopy, bool aTakeOwnership)
	{
		if (aMem == NULL || aLength == 0)
			return INVALID_PARAMETER;
		stop();

		MemoryFile dr;
        dr.openMem(aMem, aLength, aCopy, aTakeOwnership);
		return testAndLoadFile(&dr);
	}

	result Wav::loadFile(File *aFile)
	{
		if (!aFile)
			return INVALID_PARAMETER;
		stop();

		MemoryFile mr;
		result res = mr.openFileToMem(aFile);

		if (res != SO_NO_ERROR)
		{
			return res;
		}
		return testAndLoadFile(&mr);
	}

	AudioSourceInstance *Wav::createInstance()
	{
		return new WavInstance(this);
	}

	double Wav::getLength()
	{
		if (mBaseSamplerate == 0)
			return 0;
		return mSampleCount / mBaseSamplerate;
	}

	result Wav::loadRawWave8(unsigned char *aMem, unsigned int aLength, float aSamplerate, unsigned int aChannels)
	{
		if (aMem == 0 || aLength == 0 || aSamplerate <= 0 || aChannels < 1)
			return INVALID_PARAMETER;
		stop();
		delete[] mData;
		mData = new float[aLength];	
		mSampleCount = aLength / aChannels;
		mChannels = aChannels;
		mBaseSamplerate = aSamplerate;
		unsigned int i;
		for (i = 0; i < aLength; i++)
			mData[i] = ((signed)aMem[i] - 128) / (float)0x80;
		return SO_NO_ERROR;
	}

	result Wav::loadRawWave16(short *aMem, unsigned int aLength, float aSamplerate, unsigned int aChannels)
	{
		if (aMem == 0 || aLength == 0 || aSamplerate <= 0 || aChannels < 1)
			return INVALID_PARAMETER;
		stop();
		delete[] mData;
		mData = new float[aLength];
		mSampleCount = aLength / aChannels;
		mChannels = aChannels;
		mBaseSamplerate = aSamplerate;
		unsigned int i;
		for (i = 0; i < aLength; i++)
			mData[i] = ((signed short)aMem[i]) / (float)0x8000;
		return SO_NO_ERROR;
	}

	result Wav::loadRawWave(float *aMem, unsigned int aLength, float aSamplerate, unsigned int aChannels, bool aCopy, bool aTakeOwndership)
	{
		if (aMem == 0 || aLength == 0 || aSamplerate <= 0 || aChannels < 1)
			return INVALID_PARAMETER;
		stop();
		delete[] mData;
		if (aCopy == true || aTakeOwndership == false)
		{
			mData = new float[aLength];
			memcpy(mData, aMem, sizeof(float) * aLength);
		}
		else
		{
			mData = aMem;
		}
		mSampleCount = aLength / aChannels;
		mChannels = aChannels;
		mBaseSamplerate = aSamplerate;
		return SO_NO_ERROR;
	}
};
