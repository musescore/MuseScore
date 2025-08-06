/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "winmmdriver.h"

#include <windows.h>
#include <mmsystem.h>

#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif

#include "log.h"

using namespace muse::audio;

namespace  {
static const int BUFFER_COUNT = 4;

struct WinMMData
{
    short* sampleBuffer[BUFFER_COUNT];
    WAVEHDR header[BUFFER_COUNT];
    HWAVEOUT waveOut = 0;
    HANDLE bufferEndEvent = 0;
    HANDLE audioProcessingDoneEvent = 0;
    int samples = 0;
    int channels = 0;
    HANDLE threadHandle = 0;
    IAudioDriver::Callback callback;
    void* userdata = nullptr;

    WinMMData()
    {
        for (int i = 0; i < BUFFER_COUNT; i++) {
            sampleBuffer[i] = 0;
            memset(&header[i], 0, sizeof(WAVEHDR));
        }
    }
};

static WinMMData* s_winMMData = nullptr;

static DWORD WINAPI winMMThread(LPVOID aParam)
{
    WinMMData* data = static_cast<WinMMData*>(aParam);

    while (WAIT_OBJECT_0 != WaitForSingleObject(data->audioProcessingDoneEvent, 0)) {
        for (int i = 0; i < BUFFER_COUNT; ++i) {
            if (0 != (data->header[i].dwFlags & WHDR_INQUEUE)) {
                continue;
            }

            short* tgtBuf = data->sampleBuffer[i];

            //data->soloud->mixSigned16(tgtBuf, data->samples);
            uint8_t* stream = (uint8_t*)tgtBuf;
            data->callback(data->userdata, stream, data->samples * data->channels * sizeof(short));

            MMRESULT res = waveOutWrite(data->waveOut, &data->header[i], sizeof(WAVEHDR));
            if (MMSYSERR_NOERROR != res) {
                LOGE() << "failed wave out write, err: " << res;
                return 0;
            }
        }

        WaitForSingleObject(data->bufferEndEvent, INFINITE);
    }

    return 0;
}

static void winMMCleanup()
{
    WinMMData* data = s_winMMData;
    if (data->audioProcessingDoneEvent) {
        SetEvent(data->audioProcessingDoneEvent);
    }

    if (data->bufferEndEvent) {
        SetEvent(data->bufferEndEvent);
    }

    if (data->threadHandle) {
        WaitForSingleObject(data->threadHandle, INFINITE);
        CloseHandle(data->threadHandle);
    }

    if (data->waveOut) {
        waveOutReset(data->waveOut);

        for (int i = 0; i < BUFFER_COUNT; ++i) {
            waveOutUnprepareHeader(data->waveOut, &data->header[i], sizeof(WAVEHDR));
            if (0 != data->sampleBuffer[i]) {
                delete[] data->sampleBuffer[i];
            }
        }
        waveOutClose(data->waveOut);
    }

    if (data->audioProcessingDoneEvent) {
        CloseHandle(data->audioProcessingDoneEvent);
    }

    if (data->bufferEndEvent) {
        CloseHandle(data->bufferEndEvent);
    }

    delete s_winMMData;
    s_winMMData = nullptr;
}
}

std::string WinmmDriver::name() const
{
    return "MUAUDIO(WinMM)";
}

bool WinmmDriver::open(const Spec& spec, Spec* activeSpec)
{
    s_winMMData = new WinMMData;
    ZeroMemory(s_winMMData, sizeof(WinMMData));

    s_winMMData->samples = spec.samples;
    s_winMMData->channels = spec.channels;
    s_winMMData->callback = spec.callback;
    s_winMMData->userdata = spec.userdata;

    s_winMMData->bufferEndEvent = CreateEvent(0, FALSE, FALSE, 0);
    if (0 == s_winMMData->bufferEndEvent) {
        LOGE() << "failed create bufferEndEvent";
        winMMCleanup();
        return false;
    }

    s_winMMData->audioProcessingDoneEvent = CreateEvent(0, FALSE, FALSE, 0);
    if (0 == s_winMMData->audioProcessingDoneEvent) {
        LOGE() << "failed create audioProcessingDoneEvent";
        winMMCleanup();
        return false;
    }

    WAVEFORMATEX format;
    ZeroMemory(&format, sizeof(WAVEFORMATEX));

    format.nChannels = static_cast<WORD>(spec.channels);
    format.nSamplesPerSec = static_cast<DWORD>(spec.sampleRate);
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.wBitsPerSample = sizeof(short) * 8;
    format.nBlockAlign = (format.nChannels * format.wBitsPerSample) / 8;
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

    MMRESULT res = waveOutOpen(&s_winMMData->waveOut, WAVE_MAPPER, &format,
                               reinterpret_cast<DWORD_PTR>(s_winMMData->bufferEndEvent), 0, CALLBACK_EVENT);

    if (MMSYSERR_NOERROR != res) {
        LOGE() << "failed wave out open, err: " << res;
        winMMCleanup();
        return false;
    }

    for (int i = 0; i < BUFFER_COUNT; ++i) {
        s_winMMData->sampleBuffer[i] = new short[s_winMMData->samples * format.nChannels];
        ZeroMemory(&s_winMMData->header[i], sizeof(WAVEHDR));

        s_winMMData->header[i].dwBufferLength = s_winMMData->samples * sizeof(short) * format.nChannels;
        s_winMMData->header[i].lpData = reinterpret_cast<LPSTR>(s_winMMData->sampleBuffer[i]);

        MMRESULT phres = waveOutPrepareHeader(s_winMMData->waveOut, &s_winMMData->header[i], sizeof(WAVEHDR));
        if (MMSYSERR_NOERROR != phres) {
            LOGE() << "failed wave out prepare header, err: " << res;
            winMMCleanup();
            return false;
        }
    }

    if (activeSpec) {
        *activeSpec = spec;
        activeSpec->format = Format::AudioS16;
    }

    s_winMMData->threadHandle = CreateThread(nullptr, 0, winMMThread, s_winMMData, 0, nullptr);
    if (!s_winMMData->threadHandle) {
        LOGE() << "failed create thread";
        winMMCleanup();
        return false;
    }

    return true;
}

void WinmmDriver::close()
{
    winMMCleanup();
}

bool WinmmDriver::isOpened() const
{
    return s_winMMData != nullptr;
}

std::string WinmmDriver::outputDevice() const
{
    NOT_IMPLEMENTED;
    return "default";
}

bool WinmmDriver::selectOutputDevice(const std::string& /*name*/)
{
    NOT_IMPLEMENTED;
    return false;
}

std::vector<std::string> WinmmDriver::availableOutputDevices() const
{
    NOT_IMPLEMENTED;
    return { "default" };
}

async::Notification WinmmDriver::availableOutputDevicesChanged() const
{
    NOT_IMPLEMENTED;
    return async::Notification();
}

void WinmmDriver::resume()
{
}

void WinmmDriver::suspend()
{
}
