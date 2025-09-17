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
#pragma once

#include "modularity/imoduleinterface.h"

#include "global/async/channel.h"

#include "audio/common/audiotypes.h"

#include "internal/mixer.h"

namespace muse::audio::worker {
class IAudioEngine : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAudioEngine)

public:
    virtual ~IAudioEngine() = default;

    virtual void setOutputSpec(const OutputSpec& outputSpec) = 0;
    virtual OutputSpec outputSpec() const = 0;
    virtual async::Channel<OutputSpec> outputSpecChanged() const = 0;

    virtual RenderMode mode() const = 0;
    virtual void setMode(const RenderMode newMode) = 0;
    virtual async::Channel<RenderMode> modeChanged() const = 0;

    virtual MixerPtr mixer() const = 0;

    virtual void processAudioData() = 0;
    virtual void popAudioData(float* dest, size_t sampleCount) = 0;
};
}
