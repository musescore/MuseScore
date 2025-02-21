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

#include "iaudiodriver.h"

namespace muse::audio {
class AlsaDriverState : public AudioDriverState
{
public:
    AlsaDriverState();
    ~AlsaDriverState();

    std::string name() const override;
    bool open(const IAudioDriver::Spec& spec, IAudioDriver::Spec* activeSpec) override;
    void close() override;
    bool isOpened() const override;
    bool pushMidiEvent(muse::midi::Event& e) override;
    std::string deviceName() const;
    void deviceName(const std::string newDeviceName);

    void* m_alsaDeviceHandle = nullptr;

    float* m_buffer = nullptr;
    bool m_audioProcessingDone = false;
    pthread_t m_threadHandle = 0;
private:
    std::string m_deviceName = "default";
    void alsaCleanup();
};
}

#endif // MU_AUDIO_ALSAAUDIODRIVER_H
