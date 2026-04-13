/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "generalsoundfontinstallscenario.h"

#include "global/translation.h"

using namespace muse;
using namespace muse::audio;

void GeneralSoundFontInstallScenario::installSoundFont(const synth::SoundFontUri& uri)
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

RetVal<io::path_t> GeneralSoundFontInstallScenario::resolveInstallationPath(const io::path_t& path) const
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

Ret GeneralSoundFontInstallScenario::doAddSoundFont(const io::path_t& src, const io::path_t& dst)
{
    Ret ret = fileSystem()->copy(src, dst, true /* replace */);

    if (ret) {
        synth::SoundFontUri uri = synth::SoundFontUri::fromLocalFile(dst);
        soundFontController()->addSoundFont(uri);
    }

    return ret;
}
