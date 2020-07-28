/*
SoLoud audio engine
Copyright (c) 2013-2019 Jari Komppa

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

#if !defined(WITH_WASAPI)

namespace SoLoud
{
	result wasapi_init(Soloud *aSoloud, unsigned int aFlags, unsigned int aSamplerate, unsigned int aBuffer)
	{
		return NOT_IMPLEMENTED;
	}
};

#else

#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

#define SAFE_RELEASE(x) \
    if (0 != (x)) \
    { \
        (x)->Release(); \
        (x) = 0; \
    }

namespace SoLoud
{
    struct WASAPIData
    {
        IMMDeviceEnumerator *deviceEnumerator;
        IMMDevice *device;
        IAudioClient *audioClient;
        IAudioRenderClient *renderClient;
        HANDLE bufferEndEvent;
        HANDLE audioProcessingDoneEvent;
        Thread::ThreadHandle thread;
        Soloud *soloud;
        UINT32 bufferFrames;
        int channels;
		REFERENCE_TIME duration;
		unsigned int sampleRate;
		volatile bool resetRequired;
		class MMNotificationClient *notificationClient;
    };

	class MMNotificationClient : public IMMNotificationClient
	{
	public:
		MMNotificationClient(WASAPIData *aData) : IMMNotificationClient(), mRef(1), mData(aData) {}

		// IUnknown methods -- AddRef, Release, and QueryInterface

		ULONG STDMETHODCALLTYPE AddRef()
		{
			return InterlockedIncrement(&mRef);
		}

		ULONG STDMETHODCALLTYPE Release()
		{
			ULONG ulRef = InterlockedDecrement(&mRef);
			if (0 == ulRef)
			{
				delete this;
			}
			return ulRef;
		}

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface)
		{
			if (IID_IUnknown == riid)
			{
				AddRef();
				*ppvInterface = (IUnknown*)this;
			}
			else if (__uuidof(IMMNotificationClient) == riid)
			{
				AddRef();
				*ppvInterface = (IMMNotificationClient*)this;
			}
			else
			{
				*ppvInterface = NULL;
				return E_NOINTERFACE;
			}
			return S_OK;
		}

		HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR /*pwstrDeviceId*/, DWORD /*dwNewState*/) { return S_OK; }
		HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR /*pwstrDeviceId*/) { return S_OK; }
		HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR /*pwstrDeviceId*/) { return S_OK; }
		HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR /*pwstrDeviceId*/, const PROPERTYKEY /*key*/) { return S_OK; }
		HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole /*role*/, LPCWSTR /*pwstrDefaultDeviceId*/)
		{
			if (flow == eRender)
			{
				// The default rendering device just changed -> notify the thread to
				// recreate the audio graph
				mData->resetRequired = true;
				SetEvent(mData->bufferEndEvent);
			}
			return S_OK;
		}

	private:
		WASAPIData *mData;
		LONG mRef;
	};

    static HRESULT wasapiSubmitBuffer(WASAPIData *aData, UINT32 aFrames)
    {
        BYTE *buffer = 0;
		HRESULT res = aData->renderClient->GetBuffer(aFrames, &buffer);
        if (FAILED(res))
        {
            return res;
        }
		aData->soloud->mixSigned16((short *)buffer, aFrames);
		return aData->renderClient->ReleaseBuffer(aFrames, 0);
    }

    static void wasapiThread(LPVOID aParam)
    {
		bool started = false;
		bool resetRequired = false;
        WASAPIData *data = static_cast<WASAPIData*>(aParam);
        while (WAIT_OBJECT_0 != WaitForSingleObject(data->audioProcessingDoneEvent, 0)) 
        {
			resetRequired = resetRequired || data->resetRequired;
			if (!resetRequired)
			{
				UINT32 padding = 0;
				resetRequired = FAILED(data->audioClient->GetCurrentPadding(&padding));
				if (!resetRequired)
				{
					resetRequired = FAILED(wasapiSubmitBuffer(data, data->bufferFrames - padding));
				}
				if (!resetRequired && !started)
				{
					resetRequired = FAILED(data->audioClient->Start());
					started = true;
				}
				WaitForSingleObject(data->bufferEndEvent, INFINITE);
			}
			else
			{
				// There was an error. Tear down audio graph:
				data->resetRequired = false;			
				if (0 != data->audioClient)
				{
					data->audioClient->Stop();
				}
				SAFE_RELEASE(data->renderClient);
				SAFE_RELEASE(data->audioClient);
				SAFE_RELEASE(data->device);

				// Recreate audio graph:
				bool reinitFailed = FAILED(data->deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &data->device));
				if (!reinitFailed)
				{
					reinitFailed = FAILED(data->device->Activate(__uuidof(IAudioClient),
						                  CLSCTX_ALL, 0, reinterpret_cast<void**>(&data->audioClient)));
				}
				// Get mixing format from WASAPI to figure out a samplerate it can support
				WAVEFORMATEX* mixFormat = 0;
				WAVEFORMATEX format;
				ZeroMemory(&format, sizeof(WAVEFORMATEX));
				if (!reinitFailed) 
				{
					reinitFailed = FAILED(data->audioClient->GetMixFormat(&mixFormat));
				}
				if (!reinitFailed)
				{
					// At this point, we really must use the same mixing rate SoLoud uses,
					// because we can't change the sampling rate after initialization.
					// This *can* make the recreation fail every time if the rates do not match,
					// which makes this thread spin in a loop, trying to recreate everything.
					if (mixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
					{
						WAVEFORMATEXTENSIBLE* ext = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(mixFormat);
						format.nChannels = ext->Format.nChannels;
						format.nSamplesPerSec = data->sampleRate;
						format.wFormatTag = WAVE_FORMAT_PCM;
						format.wBitsPerSample = sizeof(short) * 8;
						format.nBlockAlign = (format.nChannels * format.wBitsPerSample) / 8;
						format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
					}
					else
					{
						CopyMemory(&format, mixFormat, sizeof(WAVEFORMATEX));
						format.nSamplesPerSec = data->sampleRate;
					}
					CoTaskMemFree(mixFormat);
				}
				if (!reinitFailed)
				{
					reinitFailed = FAILED(data->audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
						AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
						data->duration, 0, &format, 0));
				}
				data->bufferFrames = 0;
				if (!reinitFailed)
				{
					reinitFailed = FAILED(data->audioClient->GetBufferSize(&data->bufferFrames));
				}
				if (!reinitFailed)
				{
					reinitFailed = FAILED(data->audioClient->GetService(__uuidof(IAudioRenderClient),
						reinterpret_cast<void**>(&data->renderClient)));
				}
				if (!reinitFailed)
				{
					reinitFailed = FAILED(data->audioClient->SetEventHandle(data->bufferEndEvent));
				}
				started = false;
				resetRequired = reinitFailed;
			}
        }
    }

    static void wasapiCleanup(Soloud *aSoloud)
    {
        if (0 == aSoloud->mBackendData)
        {
            return;
        }
        WASAPIData *data = static_cast<WASAPIData*>(aSoloud->mBackendData);
        SetEvent(data->audioProcessingDoneEvent);
        SetEvent(data->bufferEndEvent);
		if (data->thread)
		{
			Thread::wait(data->thread);
			Thread::release(data->thread);
		}
		if (0 != data->notificationClient)
		{
			data->deviceEnumerator->UnregisterEndpointNotificationCallback(data->notificationClient);
			data->notificationClient->Release();
		}
        CloseHandle(data->bufferEndEvent);
        CloseHandle(data->audioProcessingDoneEvent);
        if (0 != data->audioClient)
        {
            data->audioClient->Stop();
        }
        SAFE_RELEASE(data->renderClient);
        SAFE_RELEASE(data->audioClient);
        SAFE_RELEASE(data->device);
        SAFE_RELEASE(data->deviceEnumerator);
        delete data;
        aSoloud->mBackendData = 0;
        CoUninitialize();
    }

	result wasapi_init(Soloud *aSoloud, unsigned int aFlags, unsigned int /*aSamplerate*/, unsigned int aBuffer, unsigned int /*aChannels*/)
    {
		CoInitializeEx(0, COINIT_MULTITHREADED);
        WASAPIData *data = new WASAPIData;
        ZeroMemory(data, sizeof(WASAPIData));
        aSoloud->mBackendData = data;
        aSoloud->mBackendCleanupFunc = wasapiCleanup;
		
        data->bufferEndEvent = CreateEvent(0, FALSE, FALSE, 0);
        if (0 == data->bufferEndEvent)
        {
            return UNKNOWN_ERROR;
        }
        data->audioProcessingDoneEvent = CreateEvent(0, FALSE, FALSE, 0);
        if (0 == data->audioProcessingDoneEvent)
        {
            return UNKNOWN_ERROR;
        }
        if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, CLSCTX_ALL, 
                   __uuidof(IMMDeviceEnumerator), 
                   reinterpret_cast<void**>(&data->deviceEnumerator)))) 
        {
            return UNKNOWN_ERROR;
        }
		data->notificationClient = new MMNotificationClient(data);
		/*HRESULT result = */data->deviceEnumerator->RegisterEndpointNotificationCallback(data->notificationClient);
        if (FAILED(data->deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, 
                                                                   &data->device))) 
        {
            return UNKNOWN_ERROR;
        }
        if (FAILED(data->device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, 0, 
                                          reinterpret_cast<void**>(&data->audioClient)))) 
        {
            return UNKNOWN_ERROR;
        }
		// Get mixing format from WASAPI to figure out a samplerate it can support
		WAVEFORMATEX* mixFormat = 0;
		WAVEFORMATEX format;
		ZeroMemory(&format, sizeof(WAVEFORMATEX));
		if (FAILED(data->audioClient->GetMixFormat(&mixFormat)))
		{
			return UNKNOWN_ERROR;
		}
		if (mixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
		{
			WAVEFORMATEXTENSIBLE* ext = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(mixFormat);
			format.nChannels = ext->Format.nChannels;
			format.nSamplesPerSec = ext->Format.nSamplesPerSec;
			format.wFormatTag = WAVE_FORMAT_PCM;
			format.wBitsPerSample = sizeof(short) * 8;
			format.nBlockAlign = (format.nChannels * format.wBitsPerSample) / 8;
			format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
		}
		else
		{
			CopyMemory(&format, mixFormat, sizeof(WAVEFORMATEX));
		}
		CoTaskMemFree(mixFormat);
        REFERENCE_TIME dur = static_cast<REFERENCE_TIME>(static_cast<double>(aBuffer)
			/ (static_cast<double>(format.nSamplesPerSec)*(1.0/10000000.0)));
		HRESULT res = data->audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
			AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
			dur, 0, &format, 0);
		if (FAILED(res))
        {
            return UNKNOWN_ERROR;
        }
        data->bufferFrames = 0;
        if (FAILED(data->audioClient->GetBufferSize(&data->bufferFrames)))
        {
            return UNKNOWN_ERROR;
        }
        if (FAILED(data->audioClient->GetService(__uuidof(IAudioRenderClient), 
                                                 reinterpret_cast<void**>(&data->renderClient)))) 
        {
            return UNKNOWN_ERROR;
        }
        if (FAILED(data->audioClient->SetEventHandle(data->bufferEndEvent)))
        {
            return UNKNOWN_ERROR;
        }
		data->duration = dur;
		data->sampleRate = format.nSamplesPerSec;
        data->channels = format.nChannels;
        data->soloud = aSoloud;
        aSoloud->postinit_internal(format.nSamplesPerSec, data->bufferFrames * format.nChannels, aFlags, 2);
        data->thread = Thread::createThread(wasapiThread, data);
        if (0 == data->thread)
        {
            return UNKNOWN_ERROR;
        }
        aSoloud->mBackendString = "WASAPI";
        return 0;
    }
};
#endif