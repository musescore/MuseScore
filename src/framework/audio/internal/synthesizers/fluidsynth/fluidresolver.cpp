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

#include "fluidresolver.h"

#include "internal/audiosanitizer.h"

#include "log.h"

using namespace mu::audio;
using namespace mu::audio::synth;

static const std::map<SoundFontFormat, std::string> FLUID_SF_FILE_EXTENSIONS =
{
    { SoundFontFormat::SF2, "*.sf2" },
    { SoundFontFormat::SF3, "*.sf3" }
};

static const AudioResourceVendor FLUID_VENDOR_NAME = "Fluid";

FluidResolver::FluidResolver(const io::paths& soundFontDirs, async::Channel<io::paths> sfDirsChanges)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_soundFontDirs = soundFontDirs;
    refresh();

    sfDirsChanges.onReceive(this, [this](const io::paths& newSfDirs) {
        m_soundFontDirs = newSfDirs;
        refresh();
    });
}

ISynthesizerPtr FluidResolver::resolveSynth(const TrackId /*trackId*/, const AudioInputParams& params) const
{
    ONLY_AUDIO_WORKER_THREAD;

    FluidSynthPtr synth = std::make_shared<FluidSynth>(params);

    auto search = m_resourcesCache.find(params.resourceMeta.id);

    if (search == m_resourcesCache.end()) {
        LOGE() << "Not found: " << params.resourceMeta.id;
        return synth;
    }

    synth->addSoundFonts({ search->second });

    return synth;
}

AudioResourceMetaList FluidResolver::resolveResources() const
{
    ONLY_AUDIO_WORKER_THREAD;

    AudioResourceMetaList result;
    result.reserve(m_resourcesCache.size());

    for (const auto& pair : m_resourcesCache) {
        AudioResourceMeta meta;
        meta.id = pair.first;
        meta.type = AudioResourceType::FluidSoundfont;
        meta.vendor = FLUID_VENDOR_NAME;
        meta.hasNativeEditorSupport = false;

        result.push_back(std::move(meta));
    }

    return result;
}

void FluidResolver::refresh()
{
    ONLY_AUDIO_WORKER_THREAD;

    m_resourcesCache.clear();

    for (const auto& pair : FLUID_SF_FILE_EXTENSIONS) {
        updateCaches(pair.second);
    }
}

void FluidResolver::updateCaches(const std::string& fileExtension)
{
    for (const io::path& path : m_soundFontDirs) {
        RetVal<io::paths> files = fileSystem()->scanFiles(path, { QString::fromStdString(fileExtension) });
        if (!files.ret) {
            continue;
        }

        for (const io::path& filePath : files.val) {
            m_resourcesCache.emplace(io::basename(filePath).toStdString(), filePath);
        }
    }
}
