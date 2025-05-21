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

#include "global/translation.h"

#include "synthesizers/fluidsynth/fluidsoundfontparser.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::synth;
using namespace muse::async;

void SoundFontRepository::init()
{
    loadSoundFonts();
    configuration()->soundFontDirectoriesChanged().onReceive(this, [this](const io::paths_t&) {
        loadSoundFonts();
    });
}

void SoundFontRepository::loadSoundFonts()
{
    TRACEFUNC;

    m_soundFontPaths.clear();

    SoundFontsMap oldSoundFonts;
    m_soundFonts.swap(oldSoundFonts);

    static const std::vector<std::string> filters = { "*.sf2",  "*.sf3" };
    io::paths_t dirs = configuration()->soundFontDirectories();

    for (const io::path_t& dir : dirs) {
        RetVal<io::paths_t> soundFonts = fileSystem()->scanFiles(dir, filters);
        if (!soundFonts.ret) {
            LOGE() << soundFonts.ret.toString();
            continue;
        }

        for (const SoundFontPath& soundFont : soundFonts.val) {
            loadSoundFont(soundFont, oldSoundFonts);
        }
    }
}

void SoundFontRepository::loadSoundFont(const SoundFontPath& path, const SoundFontsMap& oldSoundFonts)
{
    m_soundFontPaths.push_back(path);

    auto it = oldSoundFonts.find(path);
    if (it != oldSoundFonts.cend()) {
        m_soundFonts.insert(*it);
        return;
    }

    RetVal<SoundFontMeta> meta = FluidSoundFontParser::parseSoundFont(path);

    if (!meta.ret) {
        LOGE() << "Failed parse SoundFont presets for " << path << ": " << meta.ret.toString();
        return;
    }

    m_soundFonts.insert_or_assign(path, std::move(meta.val));
}

const SoundFontPaths& SoundFontRepository::soundFontPaths() const
{
    return m_soundFontPaths;
}

const SoundFontsMap& SoundFontRepository::soundFonts() const
{
    return m_soundFonts;
}

Notification SoundFontRepository::soundFontsChanged() const
{
    return m_soundFontsChanged;
}

void SoundFontRepository::addSoundFont(const SoundFontPath& path)
{
    std::string title = muse::qtrc("audio", "Do you want to add the SoundFont: %1?")
                        .arg(io::filename(path).toQString()).toStdString();

    interactive()->questionAsync(title, "", {
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

            interactive()->questionAsync(title, body, {
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
                    interactive()->infoAsync(muse::trc("audio", "SoundFont installed"),
                                             muse::trc("audio", "You can assign soundfonts to instruments using the mixer panel."),
                                             {}, 0, IInteractive::Option::WithIcon);
                } else {
                    LOGE() << "failed add soundfont, err: " << ret.toString();
                }
            });
        }
    });
}

Ret SoundFontRepository::doAddSoundFont(const synth::SoundFontPath& src, const SoundFontPath& dst)
{
    Ret ret = fileSystem()->copy(src, dst, true /* replace */);

    if (ret) {
        loadSoundFont(dst);
        m_soundFontsChanged.notify();
    }

    return ret;
}

RetVal<SoundFontPath> SoundFontRepository::resolveInstallationPath(const SoundFontPath& path) const
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
