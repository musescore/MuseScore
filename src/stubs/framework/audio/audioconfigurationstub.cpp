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
#include "audioconfigurationstub.h"

using namespace mu::audio;
using namespace mu;

std::string AudioConfigurationStub::currentAudioApi() const
{
    return std::string();
}

void AudioConfigurationStub::setCurrentAudioApi(const std::string&)
{
}

int AudioConfigurationStub::audioChannelsCount() const
{
    return 2;
}

unsigned int AudioConfigurationStub::driverBufferSize() const
{
    return 0;
}

bool AudioConfigurationStub::isShowControlsInMixer() const
{
    return false;
}

void AudioConfigurationStub::setIsShowControlsInMixer(bool)
{
}

// synthesizers
std::vector<io::path> AudioConfigurationStub::soundFontPaths() const
{
    return std::vector<io::path>();
}

const synth::SynthesizerState& AudioConfigurationStub::synthesizerState() const
{
    static synth::SynthesizerState s;
    return s;
}

Ret AudioConfigurationStub::saveSynthesizerState(const synth::SynthesizerState&)
{
    return make_ret(Ret::Code::NotImplemented);
}

async::Notification AudioConfigurationStub::synthesizerStateChanged() const
{
    return async::Notification();
}

async::Notification AudioConfigurationStub::synthesizerStateGroupChanged(const std::string&) const
{
    return async::Notification();
}
