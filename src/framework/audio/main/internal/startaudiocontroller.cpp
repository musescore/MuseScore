/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
#include "startaudiocontroller.h"

#include "global/realfn.h"

#include "devtools/inputlag.h"

#include "log.h"

using namespace muse::audio;

static void measureInputLag(const float* buf, const size_t size)
{
    if (INPUT_LAG_TIMER_STARTED) {
        for (size_t i = 0; i < size; ++i) {
            if (!muse::RealIsNull(buf[i])) {
                STOP_INPUT_LAG_TIMER;
                return;
            }
        }
    }
}

void StartAudioController::startAudioProcessing(const IApplication::RunMode& mode)
{
    IAudioDriver::Spec requiredSpec;
    requiredSpec.format = IAudioDriver::Format::AudioF32;
    requiredSpec.output.sampleRate = configuration()->sampleRate();
    requiredSpec.output.audioChannelCount = configuration()->audioChannelsCount();
    requiredSpec.output.samplesPerChannel = configuration()->driverBufferSize();

    //! NOTE In the web, callback works via messages and is configured in the worker
#ifndef Q_OS_WASM
    worker::IAudioWorker* worker = audioWorker().get();
    if (configuration()->shouldMeasureInputLag()) {
        requiredSpec.callback = [worker](void* /*userdata*/, uint8_t* stream, int byteCount) {
            auto samplesPerChannel = byteCount / (2 * sizeof(float));  // 2 == m_configuration->audioChannelsCount()
            float* dest = reinterpret_cast<float*>(stream);
            worker->popAudioData(dest, samplesPerChannel);
            measureInputLag(dest, samplesPerChannel * 2);
        };
    } else {
        requiredSpec.callback = [worker](void* /*userdata*/, uint8_t* stream, int byteCount) {
            auto samplesPerChannel = byteCount / (2 * sizeof(float));
            worker->popAudioData(reinterpret_cast<float*>(stream), samplesPerChannel);
        };
    }
#endif

    if (mode == IApplication::RunMode::GuiApp) {
        audioDriver()->init();

        IAudioDriver::Spec activeSpec;
        if (audioDriver()->open(requiredSpec, &activeSpec)) {
            audioWorker()->run(activeSpec.output, configuration()->workerConfig());
            return;
        }

        LOGE() << "audio output open failed";
    }

    audioWorker()->run(requiredSpec.output, configuration()->workerConfig());
}

void StartAudioController::stopAudioProcessing()
{
    if (audioDriver()->isOpened()) {
        audioDriver()->close();
    }

    if (audioWorker()->isRunning()) {
        audioWorker()->stop();
    }
}

IAudioDriverPtr StartAudioController::audioDriver() const
{
    return audioDriverController()->audioDriver();
}
