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

#include "translation.h"

#include "log.h"

using namespace mu::audio;
using namespace mu::audio::synth;
using namespace mu::framework;
using namespace mu::async;
using namespace mu::io;

void SoundFontRepository::init()
{
    loadSoundFontPaths();
    configuration()->soundFontDirectoriesChanged().onReceive(this, [this](const io::paths_t&) {
        loadSoundFontPaths();
    });
}

void SoundFontRepository::loadSoundFontPaths()
{
    TRACEFUNC;

    m_soundFontPaths.clear();

    static const std::vector<std::string> filters = { "*.sf2",  "*.sf3" };
    io::paths_t dirs = configuration()->soundFontDirectories();

    for (const io::path_t& dir : dirs) {
        RetVal<io::paths_t> soundFonts = fileSystem()->scanFiles(dir, filters);
        if (!soundFonts.ret) {
            LOGE() << soundFonts.ret.toString();
            continue;
        }

        m_soundFontPaths.insert(m_soundFontPaths.end(), soundFonts.val.begin(), soundFonts.val.end());
    }
}

SoundFontPaths SoundFontRepository::soundFontPaths() const
{
    return m_soundFontPaths;
}

Notification SoundFontRepository::soundFontPathsChanged() const
{
    return m_soundFontPathsChanged;
}

mu::Ret SoundFontRepository::addSoundFont(const SoundFontPath& path)
{
    std::string title = qtrc("audio", "Do you want to add the SoundFont: %1?")
                        .arg(io::filename(path).toQString()).toStdString();

    IInteractive::Button btn = interactive()->question(title, "", {
        IInteractive::Button::No,
        IInteractive::Button::Yes
    }).standardButton();

    if (btn == IInteractive::Button::No) {
        return mu::make_ret(Ret::Code::Cancel);
    }

    RetVal<SoundFontPath> newPath = resolveInstallationPath(path);
    if (!newPath.ret) {
        return newPath.ret;
    }

    if (fileSystem()->exists(newPath.val)) {
        title = trc("audio", "File already exists. Do you want to overwrite it?");

        std::string body = qtrc("audio", "File path: %1")
                           .arg(newPath.val.toQString()).toStdString();

        btn = interactive()->question(title, body, {
            IInteractive::Button::No,
            IInteractive::Button::Yes
        }, IInteractive::Button::Yes, IInteractive::WithIcon).standardButton();

        if (btn == IInteractive::Button::No) {
            return mu::make_ret(Ret::Code::Cancel);
        }
    }

    Ret ret = fileSystem()->copy(path, newPath.val, true /* replace */);

    if (ret) {
        m_soundFontPaths.push_back(newPath.val);
        m_soundFontPathsChanged.notify();

        interactive()->info(trc("audio", "SoundFont installed"),
                            trc("audio", "You can assign soundfonts to instruments using the mixer panel."),
                            {}, 0, IInteractive::Option::WithIcon);
    }

    return ret;
}

mu::RetVal<SoundFontPath> SoundFontRepository::resolveInstallationPath(const SoundFontPath& path) const
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
