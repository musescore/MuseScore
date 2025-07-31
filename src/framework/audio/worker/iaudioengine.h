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

#include "global/async/notification.h"

#include "audio/common/audiotypes.h"

#include "internal/mixer.h"

namespace muse::audio::worker {
class IAudioEngine : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAudioEngine)

public:
    virtual ~IAudioEngine() = default;

    virtual sample_rate_t sampleRate() const = 0;

    virtual void setSampleRate(const sample_rate_t sampleRate) = 0;
    virtual void setReadBufferSize(const uint16_t readBufferSize) = 0;
    virtual void setAudioChannelsCount(const audioch_t count) = 0;

    virtual RenderMode mode() const = 0;
    virtual void setMode(const RenderMode newMode) = 0;
    virtual async::Notification modeChanged() const = 0;

    virtual MixerPtr mixer() const = 0;
};
}
