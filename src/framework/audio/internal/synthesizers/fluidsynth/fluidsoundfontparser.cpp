/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#include "fluidsoundfontparser.h"

#include <fluidsynth.h>
#include <sfloader/fluid_sfont.h>
#include <sfloader/fluid_defsfont.h>

#include "defer.h"

using namespace muse;
using namespace muse::audio::synth;

RetVal<SoundFontMeta> FluidSoundFontParser::parseSoundFont(const SoundFontPath& path)
{
    fluid_settings_t* settings = nullptr;
    fluid_sfloader_t* loader = nullptr;
    fluid_sfont_t* sfont = nullptr;

    DEFER {
        if (sfont) {
            sfont->free(sfont);
        }
        if (loader) {
            loader->free(loader);
        }
        delete_fluid_settings(settings);
    };

    settings = new_fluid_settings();
    if (!settings) {
        return make_ret(Ret::Code::UnknownError);
    }

    fluid_settings_setint(settings, "synth.dynamic-sample-loading", 1);

    loader = new_fluid_defsfloader(settings);
    if (!loader) {
        return make_ret(Ret::Code::UnknownError);
    }

    sfont = fluid_defsfloader_load(loader, path.c_str());
    if (!sfont) {
        return make_ret(Ret::Code::UnknownError);
    }

    SoundFontMeta meta;
    meta.path = path;

    sfont->iteration_start(sfont);

    fluid_preset_t* fluid_preset;
    while ((fluid_preset = sfont->iteration_next(sfont))) {
        int bank = fluid_preset->get_banknum(fluid_preset);
        int program = fluid_preset->get_num(fluid_preset);
        const char* name = fluid_preset->get_name(fluid_preset);

        SoundFontPreset preset;
        preset.program = midi::Program(bank, program);
        preset.name = name;
        meta.presets.emplace_back(std::move(preset));
    }

    return RetVal<SoundFontMeta>::make_ok(meta);
}
