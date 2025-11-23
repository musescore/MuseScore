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

#include "global/translation.h"
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

void GeneralSoundFontController::addSoundFont(const SoundFontUri& uri)
{
    io::path_t path = uri.toLocalFile();
    std::string title = muse::qtrc("audio", "Do you want to add SoundFont %1?")
                        .arg(io::filename(path).toQString()).toStdString();

    interactive()->question(title, "", {
        IInteractive::Button::No,
        IInteractive::Button::Yes
    })
    .onResolve(this, [this, path](const IInteractive::Result& res) {
        if (res.isButton(IInteractive::Button::No)) {
            LOGI() << "soundfont addition cancelled";
            return;
        }

        RetVal<io::path_t> newPath = resolveInstallationPath(path);
        if (!newPath.ret) {
            LOGE() << "failed resolve path, err: " << newPath.ret.toString();
            return;
        }

        if (fileSystem()->exists(newPath.val)) {
            std::string title = muse::trc("audio", "File already exists. Do you want to overwrite it?");

            std::string body = muse::qtrc("audio", "File path: %1")
                               .arg(newPath.val.toQString()).toStdString();

            interactive()->question(title, body, {
                IInteractive::Button::No,
                IInteractive::Button::Yes
            }, IInteractive::Button::Yes, IInteractive::WithIcon)
            .onResolve(this, [this, path, newPath](const IInteractive::Result& res) {
                if (res.isButton(IInteractive::Button::No)) {
                    LOGI() << "soundfont replacement cancelled";
                    return;
                }

                Ret ret = doAddSoundFont(path, newPath.val);
                if (ret) {
                    interactive()->info(muse::trc("audio", "SoundFont installed"),
                                        muse::trc("audio", "You can assign soundfonts to instruments using the mixer panel."),
                                        {}, 0, IInteractive::Option::WithIcon);
                } else {
                    LOGE() << "failed add soundfont, err: " << ret.toString();
                }
            });
        } else {
            Ret ret = doAddSoundFont(path, newPath.val);
            if (ret) {
                interactive()->info(muse::trc("audio", "SoundFont installed"),
                                    muse::trc("audio", "You can assign soundfonts to instruments using the mixer panel."),
                                    {}, 0, IInteractive::Option::WithIcon);
            } else {
                LOGE() << "failed add soundfont, err: " << ret.toString();
            }
        }
    });
}

Ret GeneralSoundFontController::doAddSoundFont(const io::path_t& src, const io::path_t& dst)
{
    Ret ret = fileSystem()->copy(src, dst, true /* replace */);

    if (ret) {
        synth::SoundFontUri uri = synth::SoundFontUri::fromLocalFile(dst);
        channel()->send(rpc::make_request(Method::AddSoundFont, RpcPacker::pack(uri)));
    }

    return ret;
}

RetVal<io::path_t> GeneralSoundFontController::resolveInstallationPath(const io::path_t& path) const
{
    io::paths_t dirs = configuration()->userSoundFontDirectories();

    for (const io::path_t& dir : dirs) {
        if (fileSystem()->isWritable(dir)) {
            io::path_t newPath = dir + "/" + io::filename(path);
            return RetVal<io::path_t>::make_ok(newPath);
        }
    }

    return RetVal<io::path_t>(make_ret(Ret::Code::UnknownError));
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
