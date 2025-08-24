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
#include "soundfontrepository.h"

#include "audio/common/rpc/rpcpacker.h"
#include "audio/common/audiosanitizer.h"

#include "fluidsynth/fluidsoundfontparser.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::synth;
using namespace muse::audio::rpc;
using namespace muse::async;

void SoundFontRepository::init()
{
    ONLY_AUDIO_WORKER_THREAD;

    channel()->onMethod(Method::LoadSoundFonts, [this](const Msg& msg) {
        synth::SoundFontPaths paths;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, paths)) {
            return;
        }

        this->loadSoundFonts(paths);
    });

    channel()->onMethod(Method::AddSoundFont, [this](const Msg& msg) {
        synth::SoundFontPath path;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, path)) {
            return;
        }

        this->addSoundFont(path);
    });
}

const SoundFontPaths& SoundFontRepository::soundFontPaths() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_soundFontPaths;
}

const SoundFontsMap& SoundFontRepository::soundFonts() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_soundFonts;
}

Notification SoundFontRepository::soundFontsChanged() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_soundFontsChanged;
}

void SoundFontRepository::addSoundFont(const SoundFontPath& path)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_soundFontPaths.push_back(path);

    RetVal<SoundFontMeta> meta = FluidSoundFontParser::parseSoundFont(path);

    if (!meta.ret) {
        LOGE() << "Failed parse SoundFont presets for " << path << ": " << meta.ret.toString();
        return;
    }

    m_soundFonts.insert_or_assign(path, std::move(meta.val));

    m_soundFontsChanged.notify();
}

void SoundFontRepository::loadSoundFonts(const SoundFontPaths& paths)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_soundFontPaths.clear();

    SoundFontsMap oldSoundFonts;
    m_soundFonts.swap(oldSoundFonts);

    for (const synth::SoundFontPath& path : paths) {
        m_soundFontPaths.push_back(path);

        auto it = oldSoundFonts.find(path);
        if (it != oldSoundFonts.cend()) {
            m_soundFonts.insert(*it);
            continue;
        }

        RetVal<SoundFontMeta> meta = FluidSoundFontParser::parseSoundFont(path);

        if (!meta.ret) {
            LOGE() << "Failed parse SoundFont presets for " << path << ": " << meta.ret.toString();
            continue;
        }

        m_soundFonts.insert_or_assign(path, std::move(meta.val));
    }

    m_soundFontsChanged.notify();
}
