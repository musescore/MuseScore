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
#include "soundfontsprovider.h"

#include "log.h"

#include "async/async.h"

#include "audioerrors.h"
#include "internal/audiothread.h"
#include "internal/audiosanitizer.h"

using namespace mu;
using namespace mu::audio::synth;
using namespace mu::async;
using namespace mu::framework;

static const std::map<SoundFontFormat, std::string> SOUND_FONT_FILE_EXTENSIONS =
{
    { SoundFontFormat::SF2, "*.sf2" },
    { SoundFontFormat::SF3, "*.sf3" }
};

void SoundFontsProvider::refreshPaths()
{
    Async::call(this, [this]() {
        ONLY_AUDIO_WORKER_THREAD;

        m_soundFontPathsCache.clear();
        io::paths dirPaths = configuration()->soundFontDirectories();

        for (const auto& pair : SOUND_FONT_FILE_EXTENSIONS) {
            updateCaches(dirPaths, pair.first);
        }
    }, AudioThread::ID);
}

async::Promise<SoundFontPaths> SoundFontsProvider::soundFontPaths(SoundFontFormats formats) const
{
    return Promise<SoundFontPaths>([this, formats](Promise<SoundFontPaths>::Resolve resolve,
                                                   Promise<SoundFontPaths>::Reject /*reject*/) {
        ONLY_AUDIO_WORKER_THREAD;

        SoundFontPaths result;

        for (const SoundFontFormat& format : formats) {
            const auto& range = m_soundFontPathsCache.equal_range(format);

            for (auto i = range.first; i != range.second; ++i) {
                result.push_back(std::move(i->second));
            }
        }

        resolve(std::move(result));
    }, AudioThread::ID);
}

void SoundFontsProvider::updateCaches(const io::paths& dirPaths, const SoundFontFormat& format)
{
    for (const io::path& path : dirPaths) {
        RetVal<io::paths> files = fileSystem()->scanFiles(path, { QString::fromStdString(SOUND_FONT_FILE_EXTENSIONS.at(format)) });
        if (!files.ret) {
            continue;
        }

        for (const io::path& filePath : files.val) {
            m_soundFontPathsCache.insert({ format, filePath });
        }
    }
}
