/*
SoLoud audio engine
Copyright (c) 2013-2014 Jari Komppa

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

#include "soloud.h"
#include "soloud_thread.h"

#if !defined(WITH_WINMM)

namespace SoLoud
{
	result winmm_init(Soloud *aSoloud, unsigned int aFlags, unsigned int aSamplerate, unsigned int aBuffer)
	{
		return NOT_IMPLEMENTED;
	}
};

#else

#include <windows.h>
#include <mmsystem.h>

#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif

namespace SoLoud
{
    static const int BUFFER_COUNT = 2;

    struct SoLoudWinMMData
    {
        AlignedFloatBuffer buffer;
        short *sampleBuffer[BUFFER_COUNT];
        WAVEHDR header[BUFFER_COUNT];
        HWAVEOUT waveOut;
        HANDLE bufferEndEvent;
        HANDLE audioProcessingDoneEvent;
        Soloud *soloud;
        int samples;
        Thread::ThreadHandle threadHandle;
        SoLoudWinMMData()
        {
            buffer.clear();
            for (int i = 0; i < BUFFER_COUNT; i++)
            {
                sampleBuffer[i] = 0;
                memset(&header[i], 0, sizeof(WAVEHDR));
            }
            waveOut = 0;
            bufferEndEvent = 0;
            audioProcessingDoneEvent = 0;
            soloud = 0;
            samples = 0;
            threadHandle = 0;
        }
    };

    static void winMMThread(LPVOID aParam)
    {
        SoLoudWinMMData *data = static_cast<SoLoudWinMMData*>(aParam);
        while (WAIT_OBJECT_0 != WaitForSingleObject(data->audioProcessingDoneEvent, 0)) 
        {
            for (int i=0;i<BUFFER_COUNT;++i) 
            {
                if (0 != (data->header[i].dwFlags & WHDR_INQUEUE)) 
                {
                    continue;
                }
                short *tgtBuf = data->sampleBuffer[i];
				
				data->soloud->mixSigned16(tgtBuf, data->samples);

				if (MMSYSERR_NOERROR != waveOutWrite(data->waveOut, &data->header[i], 
                                                     sizeof(WAVEHDR))) 
                {
                    return;
                }
            }
            WaitForSingleObject(data->bufferEndEvent, INFINITE);
        }
    }

    static void winMMCleanup(Soloud *aSoloud)
    {
        if (0 == aSoloud->mBackendData)
        {
            return;
        }
        SoLoudWinMMData *data = static_cast<SoLoudWinMMData*>(aSoloud->mBackendData);
        if (data->audioProcessingDoneEvent)
        {
            SetEvent(data->audioProcessingDoneEvent);
        }
        if (data->bufferEndEvent)
        {
            SetEvent(data->bufferEndEvent);
        }
		if (data->threadHandle)
		{
			Thread::wait(data->threadHandle);
			Thread::release(data->threadHandle);
		}
        if (data->waveOut)
        {
            waveOutReset(data->waveOut);

            for (int i = 0; i < BUFFER_COUNT; ++i)
            {
                waveOutUnprepareHeader(data->waveOut, &data->header[i], sizeof(WAVEHDR));
                if (0 != data->sampleBuffer[i])
                {
                    delete[] data->sampleBuffer[i];
                }
            }
            waveOutClose(data->waveOut);
        }
        if (data->audioProcessingDoneEvent)
        {
            CloseHandle(data->audioProcessingDoneEvent);
        }
        if (data->bufferEndEvent)
        {
            CloseHandle(data->bufferEndEvent);
        }
        delete data;
        aSoloud->mBackendData = 0;
    }

	result winmm_init(Soloud *aSoloud, unsigned int aFlags, unsigned int aSamplerate, unsigned int aBuffer, unsigned int aChannels)
    {
        SoLoudWinMMData *data = new SoLoudWinMMData;
        aSoloud->mBackendData = data;
        aSoloud->mBackendCleanupFunc = winMMCleanup;
        data->samples = aBuffer;
        data->soloud = aSoloud;
        data->bufferEndEvent = CreateEvent(0, FALSE, FALSE, 0);
        if (0 == data->bufferEndEvent)
        {
            winMMCleanup(aSoloud);
            return UNKNOWN_ERROR;
        }
        data->audioProcessingDoneEvent = CreateEvent(0, FALSE, FALSE, 0);
        if (0 == data->audioProcessingDoneEvent)
        {
            winMMCleanup(aSoloud);
            return UNKNOWN_ERROR;
        }
        WAVEFORMATEX format;
        ZeroMemory(&format, sizeof(WAVEFORMATEX));
        format.nChannels = (WORD)aChannels;
        format.nSamplesPerSec = aSamplerate;
        format.wFormatTag = WAVE_FORMAT_PCM;
        format.wBitsPerSample = sizeof(short)*8;
        format.nBlockAlign = (format.nChannels*format.wBitsPerSample)/8;
        format.nAvgBytesPerSec = format.nSamplesPerSec*format.nBlockAlign;
        if (MMSYSERR_NOERROR != waveOutOpen(&data->waveOut, WAVE_MAPPER, &format, 
                            reinterpret_cast<DWORD_PTR>(data->bufferEndEvent), 0, CALLBACK_EVENT)) 
        {
            winMMCleanup(aSoloud);
            return UNKNOWN_ERROR;
        }
        data->buffer.init(data->samples*format.nChannels);
        for (int i=0;i<BUFFER_COUNT;++i) 
        {
            data->sampleBuffer[i] = new short[data->samples*format.nChannels];
            ZeroMemory(&data->header[i], sizeof(WAVEHDR));
            data->header[i].dwBufferLength = data->samples*sizeof(short)*format.nChannels;
            data->header[i].lpData = reinterpret_cast<LPSTR>(data->sampleBuffer[i]);
            if (MMSYSERR_NOERROR != waveOutPrepareHeader(data->waveOut, &data->header[i], 
                                                         sizeof(WAVEHDR))) 
            {
                winMMCleanup(aSoloud);
                return UNKNOWN_ERROR;
            }
        }
        aSoloud->postinit_internal(aSamplerate, data->samples * format.nChannels, aFlags, aChannels);
        data->threadHandle = Thread::createThread(winMMThread, data);
        if (0 == data->threadHandle)
        {
            winMMCleanup(aSoloud);
            return UNKNOWN_ERROR;
        }
        aSoloud->mBackendString = "WinMM";
        return 0;
    }
};

#endif