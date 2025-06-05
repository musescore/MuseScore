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

#ifndef MUSE_AUDIO_SFCACHEDLOADER_H
#define MUSE_AUDIO_SFCACHEDLOADER_H

#include <cstdio>

#include <sfloader/fluid_sfont.h>
#include <sfloader/fluid_defsfont.h>

namespace muse::audio::synth {
struct SoundFontData
{
    fluid_sfont_t* soundFontPtr = nullptr;
    std::FILE* fileStream = nullptr;
};

struct SoundFontCache : public std::map<std::string, SoundFontData> {
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
            fluid_defsfont_t* defsFont = static_cast<fluid_defsfont_t*>(fluid_sfont_get_data(pair.second.soundFontPtr));

            if (delete_fluid_defsfont(defsFont) != FLUID_OK) {
                continue;
            }

            delete_fluid_sfont(pair.second.soundFontPtr);

            if (pair.second.fileStream) {
                std::fclose(pair.second.fileStream);
            }
        }
    }
};

void* openSoundFont(const char* filename)
{
    auto search = SoundFontCache::instance()->find(filename);

    if (search != SoundFontCache::instance()->cend()
        && search->second.fileStream) {
        return search->second.fileStream;
    }

    std::FILE* stream = std::fopen(filename, "rb");

    SoundFontData sfData;
    sfData.fileStream = stream;

    SoundFontCache::instance()->emplace(filename, std::move(sfData));

    return stream;
}

int readSoundFont(void* buf, fluid_long_long_t count, void* handle)
{
    return static_cast<int>(std::fread(buf, count, 1, static_cast<std::FILE*>(handle)));
}

int seekSoundFont(void* handle, fluid_long_long_t offset, int origin)
{
    return std::fseek(static_cast<std::FILE*>(handle), offset, origin);
}

int closeSoundFont(void* /*handle*/)
{
    //!Note Prevent closing of sound-font files by Fluid instances,
    //!     instead the actual closing of cached sound-font files will happen in SoundFontCache.
    //!     However, we still need to provide "some" callback for Fluid's API

    return FLUID_OK;
}

fluid_long_long_t tellSoundFont(void* handle)
{
    return std::ftell(static_cast<std::FILE*>(handle));
}

int deleteSoundFont(fluid_sfont_t* /*sfont*/)
{
    //!Note Prevent removal of sound-fonts by Fluid instances,
    //!     instead the actual removal of cached sound-fonts will happen in SoundFontCache.
    //!     However, we still need to provide "some" callback for Fluid's API

    return FLUID_OK;
}

static fluid_file_callbacks_t FILE_CALLBACKS {
    openSoundFont,
    readSoundFont,
    seekSoundFont,
    closeSoundFont,
    tellSoundFont
};

fluid_sfont_t* loadSoundFont(fluid_sfloader_t* loader, const char* filename)
{
    auto search = SoundFontCache::instance()->find(filename);
    if (search != SoundFontCache::instance()->cend()) {
        return search->second.soundFontPtr;
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
    defsfont->fcbs = &FILE_CALLBACKS;

    if (fluid_defsfont_load(defsfont, &FILE_CALLBACKS, filename) == FLUID_FAILED) {
        fluid_defsfont_sfont_delete(result);
        return nullptr;
    }

    SoundFontData& sfData = SoundFontCache::instance()->operator[](filename);
    sfData.soundFontPtr = result;

    return result;
}
}

#endif // MUSE_AUDIO_SFCACHEDLOADER_H
