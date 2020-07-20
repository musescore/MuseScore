//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "winaudiodriver.h"

#include <windows.h>
#include <mmsystem.h>

#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif

#include "log.h"

using namespace mu::audio::engine;

namespace  {
static const int BUFFER_COUNT = 2;

struct WinMMData
{
    //AlignedFloatBuffer buffer;
    short* sampleBuffer[BUFFER_COUNT];
    WAVEHDR header[BUFFER_COUNT];
    HWAVEOUT waveOut;
    HANDLE bufferEndEvent;
    HANDLE audioProcessingDoneEvent;
    int samples;
    int channels;
    HANDLE threadHandle;
    IAudioDriver::Callback callback;
    void* userdata;
};

static WinMMData* _winMMData{ nullptr };

static DWORD WINAPI winMMThread(LPVOID aParam)
{
    WinMMData* data = static_cast<WinMMData*>(aParam);
    while (WAIT_OBJECT_0 != WaitForSingleObject(data->audioProcessingDoneEvent, 0)) {
        for (int i=0; i < BUFFER_COUNT; ++i) {
            if (0 != (data->header[i].dwFlags & WHDR_INQUEUE)) {
                continue;
            }
            short* tgtBuf = data->sampleBuffer[i];

            uint8_t* stream = (uint8_t*)tgtBuf;
            data->callback(data->userdata, stream, data->samples * data->channels * sizeof(short));

            if (MMSYSERR_NOERROR != waveOutWrite(data->waveOut, &data->header[i], sizeof(WAVEHDR))) {
                return 0;
            }
        }
        WaitForSingleObject(data->bufferEndEvent, INFINITE);
    }

    return 0;
}

static void winMMCleanup()
{
    SetEvent(_winMMData->audioProcessingDoneEvent);
    SetEvent(_winMMData->bufferEndEvent);
    if (_winMMData->threadHandle) {
        WaitForSingleObject(_winMMData->threadHandle, INFINITE);
        CloseHandle(_winMMData->threadHandle);
    }
    waveOutReset(_winMMData->waveOut);
    for (int i=0; i < BUFFER_COUNT; ++i) {
        waveOutUnprepareHeader(_winMMData->waveOut, &_winMMData->header[i], sizeof(WAVEHDR));
        if (nullptr != _winMMData->sampleBuffer[i]) {
            delete[] _winMMData->sampleBuffer[i];
        }
    }
    waveOutClose(_winMMData->waveOut);
    CloseHandle(_winMMData->audioProcessingDoneEvent);
    CloseHandle(_winMMData->bufferEndEvent);
    delete _winMMData;
    _winMMData = nullptr;
}
}

WinAudioDriver::WinAudioDriver()
{
}

std::string WinAudioDriver::name() const
{
    return "MUAUDIO(WinMM)";
}

bool WinAudioDriver::open(const Spec& spec, Spec* activeSpec)
{
    _winMMData = new WinMMData;
    ZeroMemory(_winMMData, sizeof(WinMMData));
    _winMMData->samples = spec.samples;
    _winMMData->channels = spec.channels;
    _winMMData->callback = spec.callback;
    _winMMData->userdata = spec.userdata;
    _winMMData->bufferEndEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (nullptr == _winMMData->bufferEndEvent) {
        return false;
    }

    _winMMData->audioProcessingDoneEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (nullptr == _winMMData->audioProcessingDoneEvent) {
        return false;
    }

    WAVEFORMATEX format;
    ZeroMemory(&format, sizeof(WAVEFORMATEX));
    format.nChannels = spec.channels;
    format.nSamplesPerSec = spec.freq;
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.wBitsPerSample = sizeof(short) * 8;
    format.nBlockAlign = (format.nChannels * format.wBitsPerSample) / 8;
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
    if (MMSYSERR_NOERROR != waveOutOpen(&_winMMData->waveOut, WAVE_MAPPER, &format,
                                        reinterpret_cast<DWORD_PTR>(_winMMData->bufferEndEvent), 0, CALLBACK_EVENT)) {
        return false;
    }

    for (int i=0; i < BUFFER_COUNT; ++i) {
        _winMMData->sampleBuffer[i] = new short[_winMMData->samples * format.nChannels];
        ZeroMemory(&_winMMData->header[i], sizeof(WAVEHDR));
        _winMMData->header[i].dwBufferLength = _winMMData->samples * sizeof(short) * format.nChannels;
        _winMMData->header[i].lpData = reinterpret_cast<LPSTR>(_winMMData->sampleBuffer[i]);
        if (MMSYSERR_NOERROR != waveOutPrepareHeader(_winMMData->waveOut, &_winMMData->header[i],
                                                     sizeof(WAVEHDR))) {
            return false;
        }
    }

    if (activeSpec) {
        *activeSpec = spec;
        activeSpec->format = Format::AudioS16;
        activeSpec->samples = _winMMData->samples * format.nChannels;
    }

    _winMMData->threadHandle = CreateThread(nullptr, 0, winMMThread, _winMMData, 0, nullptr);
    if (nullptr == _winMMData->threadHandle) {
        return false;
    }

    return true;
}

void WinAudioDriver::close()
{
    winMMCleanup();
}

bool WinAudioDriver::isOpened() const
{
    return _winMMData != nullptr;
}
