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

#pragma once

#include "iaudiodriver.h"

#include "modularity/imoduleinterface.h"

namespace muse::audio {
class IAudioDriverController : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAudioDriverController)

public:
    virtual ~IAudioDriverController() = default;

    virtual std::string currentAudioApi() const = 0;
    virtual IAudioDriverPtr audioDriver() const = 0;
    virtual void changeAudioDriver(const std::string& name) = 0;
    virtual async::Notification audioDriverChanged() const = 0;

    virtual std::vector<std::string> availableAudioApiList() const = 0;

    virtual void selectOutputDevice(const std::string& deviceId) = 0;
    virtual void changeBufferSize(samples_t samples) = 0;
    virtual void changeSampleRate(sample_rate_t sampleRate) = 0;
};
}
