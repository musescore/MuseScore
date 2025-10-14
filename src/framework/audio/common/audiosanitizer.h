/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "muse_framework_config.h"

namespace muse::audio {
class AudioSanitizer
{
public:

    static void setupMainThread();
    static std::thread::id mainThread();
    static bool isMainThread();

    static void setupEngineThread();
    static void setMixerThreads(const std::set<std::thread::id>& threadIdSet);
    static std::thread::id engineThread();
    static bool isEngineThread();
};
}

#define AUDIO_SANITIZER_ENABLED

#ifdef Q_OS_WASM
#undef AUDIO_SANITIZER_ENABLED
#endif

#if MUSE_MODULE_AUDIO_WORKMODE == MUSE_MODULE_AUDIO_WORKERRPC_MODE
#undef AUDIO_SANITIZER_ENABLED
#endif

#ifdef AUDIO_SANITIZER_ENABLED
#define ONLY_AUDIO_ENGINE_THREAD assert(muse::audio::AudioSanitizer::isEngineThread())
#define ONLY_AUDIO_RPC_THREAD assert(muse::audio::AudioSanitizer::isEngineThread())
#define ONLY_AUDIO_PROC_THREAD assert(muse::audio::AudioSanitizer::isEngineThread())
#define ONLY_AUDIO_RPC_OR_PROC_THREAD assert(muse::audio::AudioSanitizer::isEngineThread())
#define ONLY_AUDIO_MAIN_THREAD assert(muse::audio::AudioSanitizer::isMainThread())
#define ONLY_AUDIO_MAIN_OR_ENGINE_THREAD assert((muse::audio::AudioSanitizer::isEngineThread() \
                                                 || muse::audio::AudioSanitizer::isMainThread()))
#else
#define ONLY_AUDIO_ENGINE_THREAD
#define ONLY_AUDIO_RPC_THREAD
#define ONLY_AUDIO_PROC_THREAD
#define ONLY_AUDIO_RPC_OR_PROC_THREAD
#define ONLY_AUDIO_MAIN_THREAD
#define ONLY_AUDIO_MAIN_OR_ENGINE_THREAD
#endif

#endif // MUSE_AUDIO_AUDIOSANITIZER_H
