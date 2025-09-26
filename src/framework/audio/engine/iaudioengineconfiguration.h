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

#include "global/modularity/imoduleinterface.h"

#include "global/async/channel.h"
#include "audio/common/audiotypes.h"

namespace muse::audio::engine {
class IAudioEngineConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAudioEngineConfiguration)
public:
    virtual ~IAudioEngineConfiguration() = default;

    virtual bool autoProcessOnlineSoundsInBackground() const = 0;
    virtual async::Channel<bool> autoProcessOnlineSoundsInBackgroundChanged() const = 0;

    virtual AudioInputParams defaultAudioInputParams() const = 0;

    virtual size_t desiredAudioThreadNumber() const = 0;
    virtual size_t minTrackCountForMultithreading() const = 0;
};
}
