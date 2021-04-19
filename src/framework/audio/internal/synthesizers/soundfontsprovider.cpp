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
#include "internal/audiosanitizer.h"

static const std::string SF2_FILTER("*.sf2");
static const std::string SF3_FILTER("*.sf3");
static const std::string SFZ_FILTER("*.sfz");

using namespace mu;
using namespace mu::audio::synth;
using namespace mu::framework;

std::vector<io::path> SoundFontsProvider::soundFontPathsForSynth(const SynthName& synthName) const
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;
    const SynthesizerState& state = configuration()->synthesizerState();

    auto it = state.groups.find(synthName);
    if (it == state.groups.end()) {
        LOGW() << "Synthesizer state not contains group: " << synthName;
        return std::vector<io::path>();
    }
    const SynthesizerState::Group& g = it->second;

    std::vector<std::string> fontNames;
    for (const SynthesizerState::Val& val : g.vals) {
        if (val.id == SynthesizerState::ValID::SoundFontID) {
            fontNames.push_back(val.val);
        }
    }

    if (fontNames.empty()) {
        LOGW() << "Synthesizer state not contains fonts for synth: " << synthName;
        return std::vector<io::path>();
    }

    auto synth = synthRegister()->synthesizer(synthName);
    IF_ASSERT_FAILED(synth) {
        return std::vector<io::path>();
    }

    std::vector<io::path> result;
    SoundFontFormats formats = synth->soundFontFormats();
    std::vector<io::path> fontsFiles = soundFontPaths(formats);
    for (const io::path& file : fontsFiles) {
        std::string name = io::filename(file).toStdString();

        if (std::find(fontNames.begin(), fontNames.end(), name) != fontNames.end()) {
            result.push_back(file);
        }
    }

    configuration()->synthesizerStateGroupChanged(synthName).onNotify(this, [this, synthName]() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_soundFontPathsForSynthChangedMap[synthName].notify();
    });

    return result;
}

async::Notification SoundFontsProvider::soundFontPathsForSynthChanged(const SynthName& synth) const
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;
    async::Notification n;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        n = m_soundFontPathsForSynthChangedMap[synth];
    }
    return n;
}

std::vector<io::path> SoundFontsProvider::soundFontPaths(SoundFontFormats formats) const
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;
    std::vector<io::path> paths = configuration()->soundFontPaths();

    QStringList filter;
    if (formats.find(SoundFontFormat::SF2) != formats.cend()) {
        filter << QString::fromStdString(SF2_FILTER);
    }

    if (formats.find(SoundFontFormat::SF3) != formats.cend()) {
        filter << QString::fromStdString(SF3_FILTER);
    }

    if (formats.find(SoundFontFormat::SFZ) != formats.cend()) {
        filter << QString::fromStdString(SFZ_FILTER);
    }

    IF_ASSERT_FAILED(!filter.isEmpty()) {
        return std::vector<io::path>();
    }

    std::vector<io::path> soundFonts;
    for (const io::path& path : paths) {
        RetVal<io::paths> files = fileSystem()->scanFiles(path, filter);
        if (!files.ret) {
            continue;
        }

        for (const io::path& filePath : files.val) {
            soundFonts.push_back(filePath);
        }
    }

    return soundFonts;
}
