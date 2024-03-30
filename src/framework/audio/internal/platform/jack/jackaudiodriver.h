/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) MuseScore BVBA and others
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

#ifndef MUSE_AUDIO_JACKAUDIODRIVER_H
#define MUSE_AUDIO_JACKAUDIODRIVER_H

#include <jack/jack.h>

#include "iaudiodriver.h"

namespace muse::audio {
class JackDriverState : public AudioDriverState
{
public:
    JackDriverState();
    ~JackDriverState();

    std::string name() const override;
    bool open(const IAudioDriver::Spec& spec, IAudioDriver::Spec* activeSpec) override;
    void close() override;
    bool isOpened() const override;
    std::string deviceName() const;
    void deviceName(const std::string newDeviceName);

    void* m_jackDeviceHandle = nullptr;

    float* m_buffer = nullptr;

    std::vector<jack_port_t*> m_outputPorts;

private:
    std::string m_deviceName;
};
}

#endif // MUSE_AUDIO_JACKAUDIODRIVER_H
