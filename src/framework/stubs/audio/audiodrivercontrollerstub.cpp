/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "audiodrivercontrollerstub.h"

#include "audiodriverstub.h"

using namespace muse::audio;

std::string AudioDriverControllerStub::currentAudioApi() const
{
    return {};
}

IAudioDriverPtr AudioDriverControllerStub::audioDriver() const
{
    if (!m_audioDriver) {
        m_audioDriver = std::make_shared<AudioDriverStub>();
    }

    return m_audioDriver;
}

void AudioDriverControllerStub::changeAudioDriver(const std::string&)
{
}

muse::async::Notification AudioDriverControllerStub::audioDriverChanged() const
{
    return {};
}

std::vector<std::string> AudioDriverControllerStub::availableAudioApiList() const
{
    return {};
}

void AudioDriverControllerStub::selectOutputDevice(const std::string&)
{
}

void AudioDriverControllerStub::changeBufferSize(samples_t)
{
}

void AudioDriverControllerStub::changeSampleRate(sample_rate_t)
{
}
