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

#ifdef __cplusplus
extern "C" {
#endif

#include <sfloader/fluid_sfont.h>
#include <sfloader/fluid_defsfont.h>

#ifdef __cplusplus
}
#endif

#include "defer.h"
#include "translation.h"

#include "log.h"

using namespace mu::audio;
using namespace mu::audio::synth;
using namespace mu::framework;
using namespace mu::async;

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

    fluid_settings_t* settings = nullptr;
    fluid_sfloader_t* loader = nullptr;
    fluid_sfont_t* sfont = nullptr;

    DEFER {
        if (sfont) {
            fluid_defsfont_sfont_delete(sfont);
        }
        if (loader) {
            delete_fluid_sfloader(loader);
        }
        if (settings) {
            delete_fluid_settings(settings);
        }
    };

    settings = new_fluid_settings();
    if (!settings) {
        return;
    }

    fluid_settings_setint(settings, "synth.dynamic-sample-loading", 1);

    loader = new_fluid_defsfloader(settings);
    if (!loader) {
        return;
    }

    sfont = fluid_defsfloader_load(loader, path.c_str());
    if (!sfont) {
        return;
    }

    SoundFontMeta meta;
    meta.path = path;

    fluid_defsfont_sfont_iteration_start(sfont);

    fluid_preset_t* fluid_preset;
    while ((fluid_preset = fluid_defsfont_sfont_iteration_next(sfont))) {
        int bank = fluid_defpreset_preset_get_banknum(fluid_preset);
        int program = fluid_defpreset_preset_get_num(fluid_preset);
        const char* name = fluid_defpreset_preset_get_name(fluid_preset);

        SoundFontPreset preset;
        preset.program = midi::Program(bank, program);
        preset.name = name;
        meta.presets.push_back(preset);
    }

    m_soundFonts.insert_or_assign(path, meta);
}

SoundFontPaths SoundFontRepository::soundFontPaths() const
{
    return m_soundFontPaths;
}

SoundFontsMap SoundFontRepository::soundFonts() const
{
    return m_soundFonts;
}

Notification SoundFontRepository::soundFontsChanged() const
{
    return m_soundFontsChanged;
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
        loadSoundFont(newPath.val);
        m_soundFontsChanged.notify();

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
