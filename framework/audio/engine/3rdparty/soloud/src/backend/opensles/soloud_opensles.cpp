/*
SoLoud audio engine
Copyright (c) 2013-2015 Jari Komppa

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
#include <math.h>
#include <memory.h>

#include "soloud.h"
#include "soloud_thread.h"

#if !defined(WITH_OPENSLES)
namespace SoLoud
{
	result opensles_init(SoLoud::Soloud *aSoloud, unsigned int aFlags, unsigned int aSamplerate, unsigned int aBuffer)
	{
		return NOT_IMPLEMENTED;
	}
}
#else

#include "SLES/OpenSLES.h"
#include "SLES/OpenSLES_Platform.h"

#if defined( __ANDROID__ )
#include "SLES/OpenSLES_Android.h"
#endif

// Error logging.
#if defined( __ANDROID__ ) 
#  include <android/log.h>
#  define LOG_ERROR( _msg ) \
   __android_log_print( ANDROID_LOG_ERROR, "SoLoud", _msg )
#  define LOG_INFO( _msg ) \
   __android_log_print( ANDROID_LOG_INFO, "SoLoud", _msg )

#else
   printf( _msg )
#endif

#define NUM_BUFFERS 2

namespace SoLoud
{
	struct BackendData
	{
		BackendData()
		{
			memset(this, 0, sizeof(BackendData));
		}

		~BackendData()
		{
			// Wait until thread is done.
			threadrun++;
			while (threadrun == 1)
			{
				Thread::sleep(10);
			}

			if(playerObj)
			{
				(*playerObj)->Destroy(playerObj);
			}

			if(outputMixObj)
			{
				(*outputMixObj)->Destroy(outputMixObj);
			}

			if(engineObj)
			{
				(*engineObj)->Destroy(engineObj);
			}

			for(int idx = 0; idx < NUM_BUFFERS; ++idx)
			{
				delete [] outputBuffers[idx];
			}
		}

		// Engine.
		SLObjectItf engineObj;
		SLEngineItf engine;

		// Output mix.
		SLObjectItf outputMixObj;
		SLVolumeItf outputMixVol;

		// Data.
		SLDataLocator_OutputMix outLocator;
		SLDataSink dstDataSink;

		// Player.
		SLObjectItf playerObj;
		SLPlayItf player;
		SLVolumeItf playerVol;
		SLAndroidSimpleBufferQueueItf playerBufferQueue;

		unsigned int bufferSize;
		unsigned int channels;
		short *outputBuffers[ NUM_BUFFERS ];
		int buffersQueued;
		int activeBuffer;
		volatile int threadrun;

		SLDataLocator_AndroidSimpleBufferQueue inLocator;
	};

 	void soloud_opensles_deinit(SoLoud::Soloud *aSoloud)
	{
		BackendData *data = static_cast<BackendData*>(aSoloud->mBackendData);
		delete data;
		aSoloud->mBackendData = NULL;
	}

	static void opensles_iterate(SoLoud::Soloud *aSoloud)
	{
		BackendData *data = static_cast<BackendData*>(aSoloud->mBackendData);

		// If we have no buffered queued, queue one up for playback.
		if(data->buffersQueued == 0)
		{
			// Get next output buffer, advance, next buffer.
			short * outputBuffer = data->outputBuffers[data->activeBuffer];
			data->activeBuffer = (data->activeBuffer + 1) % NUM_BUFFERS;
			short * nextBuffer = data->outputBuffers[data->activeBuffer];

			// Mix this buffer. 
			const int bufferSizeBytes = data->bufferSize * data->channels * sizeof(short);
			(*data->playerBufferQueue)->Enqueue(data->playerBufferQueue, outputBuffer, bufferSizeBytes);
			++data->buffersQueued;

			aSoloud->mixSigned16(nextBuffer,data->bufferSize);
		}
	}

	static void opensles_thread(void *aParam)
	{
		Soloud *soloud = static_cast<Soloud*>(aParam);
		BackendData *data = static_cast<BackendData*>(soloud->mBackendData);
		while (data->threadrun == 0)
		{
			opensles_iterate(soloud);

			// TODO: Condition var?
			Thread::sleep(1);
		}
		data->threadrun++;
	}

	static void SLAPIENTRY soloud_opensles_play_callback(SLPlayItf player, void *context, SLuint32 event)
	{
		Soloud *soloud = static_cast<Soloud*>(context);
		BackendData *data = static_cast<BackendData*>(soloud->mBackendData);
		if( event & SL_PLAYEVENT_HEADATEND )
		{
			data->buffersQueued--;
		}
	}

	result opensles_init(SoLoud::Soloud *aSoloud, unsigned int aFlags, unsigned int aSamplerate, unsigned int aBuffer, unsigned int aChannels)
	{
		BackendData *data = new BackendData();

		// Allocate output buffer to mix into.
		data->bufferSize = aBuffer;
		data->channels = aChannels;
		const int bufferSizeBytes = data->bufferSize * data->channels * sizeof(short);
		for(int idx = 0; idx < NUM_BUFFERS; ++idx)
		{
			data->outputBuffers[idx] = new short[ data->bufferSize * data->channels ];
			memset(data->outputBuffers[idx], 0, bufferSizeBytes);
		}

		// Create engine.
		const SLEngineOption opts[] = { SL_ENGINEOPTION_THREADSAFE, SL_BOOLEAN_TRUE };
		if(slCreateEngine( &data->engineObj, 1, opts, 0, NULL, NULL ) != SL_RESULT_SUCCESS)
		{
			LOG_ERROR( "Failed to create engine." );
			return UNKNOWN_ERROR;
		}

		// Realize and get engine interfaxce.	
		(*data->engineObj)->Realize(data->engineObj, SL_BOOLEAN_FALSE);
		if((*data->engineObj)->GetInterface(data->engineObj, SL_IID_ENGINE, &data->engine) != SL_RESULT_SUCCESS)
		{
			LOG_ERROR( "Failed to get engine interface." );
			return UNKNOWN_ERROR;
		}

		// Create output mix.
		const SLInterfaceID ids[] = { SL_IID_VOLUME };
		const SLboolean req[] = { SL_BOOLEAN_FALSE };

		if((*data->engine)->CreateOutputMix(data->engine, &data->outputMixObj, 1, ids, req) != SL_RESULT_SUCCESS)
		{
			LOG_ERROR( "Failed to create output mix object." );
			return UNKNOWN_ERROR;
		}
		(*data->outputMixObj)->Realize(data->outputMixObj, SL_BOOLEAN_FALSE);

		if((*data->outputMixObj)->GetInterface(data->outputMixObj, SL_IID_VOLUME, &data->outputMixVol) != SL_RESULT_SUCCESS)
		{
			LOG_INFO( "Failed to get output mix volume interface." );
		}

		// Create android buffer queue.
		data->inLocator.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
		data->inLocator.numBuffers = NUM_BUFFERS;

		// Setup data format.
		SLDataFormat_PCM format;
		format.formatType = SL_DATAFORMAT_PCM;
		format.numChannels = data->channels;
		format.samplesPerSec = aSamplerate * 1000; //mHz
		format.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
		format.containerSize = 16;
		format.endianness = SL_BYTEORDER_LITTLEENDIAN;

		// Determine channel mask.
		if(data->channels == 2)
		{
			format.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
		}
		else
		{
			format.channelMask = SL_SPEAKER_FRONT_CENTER;
		}
		 
		SLDataSource src;
		src.pLocator = &data->inLocator;
		src.pFormat = &format;

		// Output mix.
		data->outLocator.locatorType = SL_DATALOCATOR_OUTPUTMIX;
		data->outLocator.outputMix = data->outputMixObj;
		 
		data->dstDataSink.pLocator = &data->outLocator;
		data->dstDataSink.pFormat = NULL;

		// Setup player.
		{
			const SLInterfaceID ids[] = { SL_IID_VOLUME, SL_IID_ANDROIDSIMPLEBUFFERQUEUE };
			const SLboolean req[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
 
			(*data->engine)->CreateAudioPlayer(data->engine, &data->playerObj, &src, &data->dstDataSink, sizeof(ids) / sizeof(ids[0]), ids, req);
		 
			(*data->playerObj)->Realize(data->playerObj, SL_BOOLEAN_FALSE);
	 
			(*data->playerObj)->GetInterface(data->playerObj, SL_IID_PLAY, &data->player);
			(*data->playerObj)->GetInterface(data->playerObj, SL_IID_VOLUME, &data->playerVol);
 
			(*data->playerObj)->GetInterface(data->playerObj, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &data->playerBufferQueue);
		}

		aSoloud->mBackendData = data;		// Must be set before callback

		// Begin playing.
		{
			const int bufferSizeBytes = data->bufferSize * data->channels * sizeof(short);
			(*data->playerBufferQueue)->Enqueue(data->playerBufferQueue, data->outputBuffers[0], bufferSizeBytes);
			data->activeBuffer = (data->activeBuffer + 1) % NUM_BUFFERS;

			(*data->player)->RegisterCallback(data->player, soloud_opensles_play_callback, aSoloud);
			(*data->player)->SetCallbackEventsMask(data->player, SL_PLAYEVENT_HEADATEND);
			(*data->player)->SetPlayState(data->player, SL_PLAYSTATE_PLAYING);

		}

		//
		aSoloud->postinit_internal(aSamplerate,data->bufferSize,aFlags,2);
		aSoloud->mBackendCleanupFunc = soloud_opensles_deinit;

		LOG_INFO( "Creating audio thread." );
		Thread::createThread(opensles_thread, (void*)aSoloud);

		aSoloud->mBackendString = "OpenSL ES";
		return SO_NO_ERROR;
	}	
};
#endif
