//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "soundfontsprovider.h"

#include "log.h"

static const std::string SF2_FILTER("*.sf2");
static const std::string SF3_FILTER("*.sf3");
static const std::string SFZ_FILTER("*.sfz");

using namespace mu;
using namespace mu::midi;
using namespace mu::framework;

std::vector<io::path> SoundFontsProvider::soundFontPathsForSynth(const SynthName& synthName) const
{
    const SynthesizerState& state = configuration()->synthesizerState();

    const SynthesizerState::Group& g = state.group(synthName);
    if (!g.isValid()) {
        LOGW() << "Synthesizer state not contains group: " << synthName;
        return std::vector<io::path>();
    }

    std::vector<std::string> fontNames;
    for (const SynthesizerState::Val& val : g.vals) {
        if (val.id == SynthesizerState::SoundFontID) {
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
        std::string name = io::filename(file);

        if (std::find(fontNames.begin(), fontNames.end(), name) != fontNames.end()) {
            result.push_back(file);
        }
    }

    return result;
}

std::vector<io::path> SoundFontsProvider::soundFontPaths(SoundFontFormats formats) const
{
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
        RetVal<QStringList> files = filesystem()->scanFiles(io::pathToQString(path), filter, IFsOperations::ScanMode::IncludeSubdirs);
        if (!files.ret) {
            continue;
        }

        for (const QString& path : files.val) {
            soundFonts.push_back(io::pathFromQString(path));
        }
    }

    return soundFonts;
}
