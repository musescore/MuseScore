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
#include "audioconfiguration.h"
#include "settings.h"
#include "stringutils.h"

#include "global/xmlreader.h"
#include "global/xmlwriter.h"

#include "log.h"

//TODO: remove with global clearing of Q_OS_*** defines
#include <QtGlobal>

using namespace mu;
using namespace mu::framework;
using namespace mu::audio;
using namespace mu::audio::synth;

//TODO: add other setting: audio device etc
static const Settings::Key AUDIO_API_KEY("audio", "io/audioApi");
static const Settings::Key AUDIO_BUFFER_SIZE("audio", "driver_buffer");

static const Settings::Key USER_SOUNDFONTS_PATH("midi", "application/paths/mySoundfonts");

static const Settings::Key SHOW_CONTROLS_IN_MIXER("midi", "io/midi/showControlsInMixer");

//! FIXME Temporary for tests
static const std::string DEFAULT_FLUID_SOUNDFONT = "MuseScore_General.sf3";     // "GeneralUser GS v1.471.sf2"; // "MuseScore_General.sf3";
static const std::string DEFAULT_ZERBERUS_SOUNDFONT = "FM-Piano1-20190916.sfz"; // "";

void AudioConfiguration::init()
{
    int defaultBufferSize = 0;
#ifdef Q_OS_WASM
    defaultBufferSize = 8192;
#else
    defaultBufferSize = 1024;
#endif
    settings()->setDefaultValue(AUDIO_BUFFER_SIZE, Val(defaultBufferSize));

    settings()->setDefaultValue(SHOW_CONTROLS_IN_MIXER, Val(true));
    settings()->setDefaultValue(AUDIO_API_KEY, Val("Core Audio"));
}

std::vector<std::string> AudioConfiguration::availableAudioApiList() const
{
    std::vector<std::string> names {
        "Core Audio",
        "ALSA Audio",
        "PulseAudio",
        "JACK Audio Server"
    };

    return names;
}

std::string AudioConfiguration::currentAudioApi() const
{
    return settings()->value(AUDIO_API_KEY).toString();
}

void AudioConfiguration::setCurrentAudioApi(const std::string& name)
{
    settings()->setValue(AUDIO_API_KEY, Val(name));
}

unsigned int AudioConfiguration::driverBufferSize() const
{
    return settings()->value(AUDIO_BUFFER_SIZE).toInt();
}

std::vector<io::path> AudioConfiguration::soundFontPaths() const
{
    std::string pathsStr = settings()->value(USER_SOUNDFONTS_PATH).toString();
    std::vector<io::path> paths = io::path::pathsFromString(pathsStr, ";");
    paths.push_back(globalConfiguration()->sharePath());

    //! TODO Implement me
    // append extensions directory
    //QStringList extensionsDir = Ms::Extension::getDirectoriesByType(Ms::Extension::soundfontsDir);

    return paths;
}

bool AudioConfiguration::isShowControlsInMixer() const
{
    return settings()->value(SHOW_CONTROLS_IN_MIXER).toBool();
}

void AudioConfiguration::setIsShowControlsInMixer(bool show)
{
    settings()->setValue(SHOW_CONTROLS_IN_MIXER, Val(show));
}

const SynthesizerState& AudioConfiguration::defaultSynthesizerState() const
{
    static SynthesizerState state;
    if (state.isNull()) {
        SynthesizerState::Group gf;
        gf.name = "Fluid";
        gf.vals.push_back(SynthesizerState::Val(SynthesizerState::ValID::SoundFontID, DEFAULT_FLUID_SOUNDFONT));
        state.groups.insert({ gf.name, std::move(gf) });

        SynthesizerState::Group gz;
        gz.name = "Zerberus";
        gz.vals.push_back(SynthesizerState::Val(SynthesizerState::ValID::SoundFontID, DEFAULT_ZERBERUS_SOUNDFONT));
        state.groups.insert({ gz.name, std::move(gz) });
    }

    return state;
}

const SynthesizerState& AudioConfiguration::synthesizerState() const
{
    if (!m_state.isNull()) {
        return m_state;
    }

    bool ok = readState(stateFilePath(), m_state);
    if (!ok) {
        LOGW() << "failed read synthesizer state, file: " << stateFilePath();
        m_state = defaultSynthesizerState();
    }

    return m_state;
}

Ret AudioConfiguration::saveSynthesizerState(const SynthesizerState& state)
{
    std::list<std::string> changedGroups;
    for (auto it = m_state.groups.cbegin(); it != m_state.groups.cend(); ++it) {
        auto nit = state.groups.find(it->first);
        if (nit == state.groups.cend()) {
            continue;
        }

        if (it->second != nit->second) {
            changedGroups.push_back(it->first);
        }
    }

    Ret ret = writeState(stateFilePath(), state);
    if (!ret) {
        LOGE() << "failed write synthesizer state, file: " << stateFilePath();
        return ret;
    }

    m_state = state;
    m_synthesizerStateChanged.notify();
    for (const std::string& gname : changedGroups) {
        m_synthesizerStateGroupChanged[gname].notify();
    }

    return make_ret(Ret::Code::Ok);
}

async::Notification AudioConfiguration::synthesizerStateChanged() const
{
    return m_synthesizerStateChanged;
}

async::Notification AudioConfiguration::synthesizerStateGroupChanged(const std::string& gname) const
{
    return m_synthesizerStateGroupChanged[gname];
}

io::path AudioConfiguration::stateFilePath() const
{
    return globalConfiguration()->dataPath() + "/synthesizer.xml";
}

bool AudioConfiguration::readState(const io::path& path, SynthesizerState& state) const
{
    XmlReader xml(path);

    while (xml.canRead() && xml.success()) {
        XmlReader::TokenType token = xml.readNext();
        if (token == XmlReader::StartDocument) {
            continue;
        }

        if (token == XmlReader::StartElement) {
            if (xml.tagName() == "Synthesizer") {
                continue;
            }

            while (xml.tokenType() != XmlReader::EndElement) {
                SynthesizerState::Group group;
                group.name = xml.tagName();

                xml.readNext();

                while (xml.tokenType() != XmlReader::EndElement) {
                    if (xml.tokenType() == XmlReader::StartElement) {
                        if (xml.tagName() == "val") {
                            SynthesizerState::ValID id = static_cast<SynthesizerState::ValID>(xml.intAttribute("id"));
                            group.vals.push_back(SynthesizerState::Val(id, xml.readString()));
                        } else {
                            xml.skipCurrentElement();
                        }
                    }
                    xml.readNext();
                }

                state.groups.insert({ group.name, std::move(group) });
            }
        }
    }

    if (!xml.success()) {
        LOGE() << "failed parse xml, error: " << xml.error() << ", path: " << path;
    }

    return xml.success();
}

bool AudioConfiguration::writeState(const io::path& path, const SynthesizerState& state)
{
    XmlWriter xml(path);
    xml.writeStartDocument();

    xml.writeStartElement("Synthesizer");

    for (auto it = state.groups.cbegin(); it != state.groups.cend(); ++it) {
        const SynthesizerState::Group& group = it->second;

        if (group.name.empty()) {
            continue;
        }

        xml.writeStartElement(group.name);
        for (const SynthesizerState::Val& value : group.vals) {
            xml.writeStartElement("val");
            xml.writeAttribute("id", std::to_string(static_cast<int>(value.id)));
            xml.writeCharacters(value.val);
            xml.writeEndElement();
        }
        xml.writeEndElement();
    }

    xml.writeEndElement();
    xml.writeEndDocument();

    if (!xml.success()) {
        LOGE() << "failed write xml";
    }

    return xml.success();
}
