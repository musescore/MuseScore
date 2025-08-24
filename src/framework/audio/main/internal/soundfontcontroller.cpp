/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#include "soundfontcontroller.h"

#include "global/translation.h"

#include "audio/common/rpc/rpcpacker.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::rpc;
using namespace muse::audio::synth;

void SoundFontController::init()
{
    configuration()->soundFontDirectoriesChanged().onReceive(this, [this](const io::paths_t&) {
        loadSoundFonts();
    });

    loadSoundFonts();
}

void SoundFontController::addSoundFont(const synth::SoundFontPath& path)
{
    std::string title = muse::qtrc("audio", "Do you want to add the SoundFont: %1?")
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

        RetVal<SoundFontPath> newPath = resolveInstallationPath(path);
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
        }
    });
}

Ret SoundFontController::doAddSoundFont(const synth::SoundFontPath& src, const SoundFontPath& dst)
{
    Ret ret = fileSystem()->copy(src, dst, true /* replace */);

    if (ret) {
        channel()->send(rpc::make_request(Method::AddSoundFont, RpcPacker::pack(dst)));
    }

    return ret;
}

RetVal<SoundFontPath> SoundFontController::resolveInstallationPath(const SoundFontPath& path) const
{
    io::paths_t dirs = configuration()->userSoundFontDirectories();

    for (const io::path_t& dir : dirs) {
        if (fileSystem()->isWritable(dir)) {
            SoundFontPath newPath = dir + "/" + io::filename(path);
            return RetVal<SoundFontPath>::make_ok(newPath);
        }
    }

    return RetVal<SoundFontPath>(make_ret(Ret::Code::UnknownError));
}

void SoundFontController::loadSoundFonts()
{
    TRACEFUNC;

    static const std::vector<std::string> filters = { "*.sf2",  "*.sf3" };
    io::paths_t dirs = configuration()->soundFontDirectories();

    synth::SoundFontPaths paths;
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

void SoundFontController::loadSoundFonts(const synth::SoundFontPaths& paths)
{
    channel()->send(rpc::make_request(Method::LoadSoundFonts, RpcPacker::pack(paths)));
}
