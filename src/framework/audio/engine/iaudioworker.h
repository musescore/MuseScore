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

namespace muse::audio::engine {
class IAudioWorker : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAudioWorker)
public:
    virtual ~IAudioWorker() = default;

    virtual void registerExports() = 0; // temporary

    virtual void run() = 0;
    virtual void stop() = 0;
    virtual bool isRunning() const = 0;
    virtual async::Channel<bool> isRunningChanged() const = 0;

    virtual void popAudioData(float* dest, size_t sampleCount) = 0;
};
}
