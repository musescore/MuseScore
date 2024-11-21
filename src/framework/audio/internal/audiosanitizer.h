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
#ifndef MUSE_AUDIO_AUDIOSANITIZER_H
#define MUSE_AUDIO_AUDIOSANITIZER_H

//! NOTE This is dev tools

#include <cassert>
#include <set>
#include <thread>

namespace muse::audio {
class AudioSanitizer
{
public:

    static void setupMainThread();
    static std::thread::id mainThread();
    static bool isMainThread();

    static void setupWorkerThread();
    static void setMixerThreads(const std::set<std::thread::id>& threadIdSet);
    static std::thread::id workerThread();
    static bool isWorkerThread();
};
}

#define ONLY_AUDIO_WORKER_THREAD assert(muse::audio::AudioSanitizer::isWorkerThread())
#define ONLY_AUDIO_MAIN_THREAD assert(muse::audio::AudioSanitizer::isMainThread())
#define ONLY_AUDIO_MAIN_OR_WORKER_THREAD assert((muse::audio::AudioSanitizer::isWorkerThread() \
                                                 || muse::audio::AudioSanitizer::isMainThread()))

#endif // MUSE_AUDIO_AUDIOSANITIZER_H
