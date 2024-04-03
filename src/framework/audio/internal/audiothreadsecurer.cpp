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

#include "audiothreadsecurer.h"

#include "audiosanitizer.h"

using namespace muse::audio;

bool AudioThreadSecurer::isMainThread() const
{
    return AudioSanitizer::isMainThread();
}

std::thread::id AudioThreadSecurer::mainThreadId() const
{
    return AudioSanitizer::mainThread();
}

bool AudioThreadSecurer::isAudioWorkerThread() const
{
    return AudioSanitizer::isWorkerThread();
}

std::thread::id AudioThreadSecurer::workerThreadId() const
{
    return AudioSanitizer::workerThread();
}
