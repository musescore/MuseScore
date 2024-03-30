/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#ifndef MU_AUDIO_ALSAAUDIODRIVER_H
#define MU_AUDIO_ALSAAUDIODRIVER_H

#include "async/asyncable.h"

#include "iaudiodriver.h"

#include "audiodeviceslistener.h"

namespace muse::audio {

class AlsaDriverState
{
public:
    AlsaDriverState();
    ~AlsaDriverState();

    std::string name() const;
    bool open(const Spec& spec, Spec* activeSpec);
    void close();
    bool isOpened() const;

private:
    float* buffer = nullptr;
    void* alsaDeviceHandle = nullptr;
    unsigned long samples = 0;
    int channels = 0;
    bool audioProcessingDone = false;
    pthread_t threadHandle = 0;
    IAudioDriver::Callback callback;
    void* userdata = nullptr;
    IAudioDriver::Spec format;
    void alsaCleanup();
};
}

#endif // MU_AUDIO_ALSAAUDIODRIVER_H
