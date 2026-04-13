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

#include "generalsoundfontcontroller.h"

#include "global/io/path.h"

#include "audio/common/rpc/rpcpacker.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::rpc;
using namespace muse::audio::synth;

void GeneralSoundFontController::loadSoundFonts()
{
    configuration()->soundFontDirectoriesChanged().onReceive(this, [this](const io::paths_t&) {
        doLoadSoundFonts();
    });

    doLoadSoundFonts();
}

void GeneralSoundFontController::doLoadSoundFonts()
{
    TRACEFUNC;

    static const std::vector<std::string> filters = { "*.sf2",  "*.sf3" };
    io::paths_t dirs = configuration()->soundFontDirectories();

    std::vector<io::path_t> paths;
    for (const io::path_t& dir : dirs) {
        RetVal<io::paths_t> soundFonts = fileSystem()->scanFiles(dir, filters);
        if (!soundFonts.ret) {
            LOGE() << soundFonts.ret.toString();
            continue;
        }

        paths.insert(paths.end(), soundFonts.val.begin(), soundFonts.val.end());
    }

    loadSoundFonts(paths);
}

void GeneralSoundFontController::loadSoundFonts(const std::vector<io::path_t>& paths)
{
    std::vector<synth::SoundFontUri> uris;
    uris.reserve(paths.size());
    for (const io::path_t& p : paths) {
        uris.push_back(synth::SoundFontUri::fromLocalFile(p));
    }
    channel()->send(rpc::make_request(Method::LoadSoundFonts, RpcPacker::pack(uris)));
}

void GeneralSoundFontController::addSoundFont(const synth::SoundFontUri& uri)
{
    channel()->send(rpc::make_request(Method::AddSoundFont, RpcPacker::pack(uri)));
}
