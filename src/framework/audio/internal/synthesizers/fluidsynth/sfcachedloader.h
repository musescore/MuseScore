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

#ifndef MU_AUDIO_SFCACHEDLOADER_H
#define MU_AUDIO_SFCACHEDLOADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cstdio>
#include <vector>
#include <map>
#include <string>

#include <sfloader/fluid_sfont.h>
#include <sfloader/fluid_defsfont.h>

namespace mu::audio::synth {
struct SoundFontCache : public std::map<std::string, fluid_sfont_t*> {
    static SoundFontCache* instance()
    {
        static SoundFontCache s;
        return &s;
    }

private:
    SoundFontCache() = default;
    ~SoundFontCache()
    {
        for (const auto& pair : *this) {
            fluid_defsfont_t* defsFont = static_cast<fluid_defsfont_t*>(fluid_sfont_get_data(pair.second));

            if (delete_fluid_defsfont(defsFont) != FLUID_OK) {
                continue;
            }

            delete_fluid_sfont(pair.second);
        }
    }
};

int deleteSoundFont(fluid_sfont_t* /*sfont*/)
{
    //!Note Prevent removal of sound-fonts by Fluid instances,
    //!     instead the actual removal of cached sound-fonts will happen in SoundFontCache.
    //!     However, we still need to provide "some" callback for Fluid's API

    return FLUID_OK;
}

fluid_sfont_t* loadSoundFont(fluid_sfloader_t* loader, const char* filename)
{
    auto search = SoundFontCache::instance()->find(filename);
    if (search != SoundFontCache::instance()->cend()) {
        return search->second;
    }

    fluid_defsfont_t* defsfont = nullptr;
    fluid_sfont_t* result = nullptr;

    defsfont = new_fluid_defsfont(static_cast<fluid_settings_t*>(fluid_sfloader_get_data(loader)));

    if (!defsfont) {
        return nullptr;
    }

    result = new_fluid_sfont(fluid_defsfont_sfont_get_name,
                             fluid_defsfont_sfont_get_preset,
                             fluid_defsfont_sfont_iteration_start,
                             fluid_defsfont_sfont_iteration_next,
                             deleteSoundFont);

    if (!result) {
        return result;
    }

    fluid_sfont_set_data(result, defsfont);
    defsfont->sfont = result;

    if (fluid_defsfont_load(defsfont, &loader->file_callbacks, filename) == FLUID_FAILED) {
        fluid_defsfont_sfont_delete(result);
        return nullptr;
    }

    SoundFontCache::instance()->emplace(filename, result);

    return result;
}
}

#ifdef __cplusplus
}
#endif

#endif // MU_AUDIO_SFCACHEDLOADER_H
