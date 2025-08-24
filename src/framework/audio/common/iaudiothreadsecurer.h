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

#ifndef MUSE_AUDIO_IAUDIOTHREADSECURER_H
#define MUSE_AUDIO_IAUDIOTHREADSECURER_H

#include <cassert>
#include <thread>

#include "global/modularity/ioc.h"

namespace muse::audio {
class IAudioThreadSecurer : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAudioThreadSecurer)
public:
    virtual ~IAudioThreadSecurer() = default;
    virtual bool isMainThread() const = 0;
    virtual std::thread::id mainThreadId() const = 0;
    virtual bool isAudioWorkerThread() const = 0;
    virtual std::thread::id workerThreadId() const = 0;
};
}

#define ONLY_AUDIO_THREAD(securer) assert(securer()->isAudioWorkerThread())
#define ONLY_MAIN_THREAD(securer) assert(securer()->isMainThread())
#define ONLY_AUDIO_OR_MAIN_THREAD(securer) assert(securer()->isMainThread() || securer()->isAudioWorkerThread())

#endif // MUSE_AUDIO_IAUDIOTHREADSECURER_H
