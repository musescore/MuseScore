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
#include "audioconfiguration.h"
#include "settings.h"
#include "stringutils.h"

#include "global/deprecated/xmlreader.h"
#include "global/deprecated/xmlwriter.h"

#include "log.h"

//TODO: remove with global clearing of Q_OS_*** defines
#include <QtGlobal>

using namespace mu;
using namespace mu::framework;
using namespace mu::audio;
using namespace mu::audio::synth;

static const audioch_t AUDIO_CHANNELS = 2;

//TODO: add other setting: audio device etc
static const Settings::Key AUDIO_API_KEY("audio", "io/audioApi");
static const Settings::Key AUDIO_OUTPUT_DEVICE_ID_KEY("audio", "io/outputDevice");
static const Settings::Key AUDIO_BUFFER_SIZE_KEY("audio", "io/bufferSize");
static const Settings::Key AUDIO_SAMPLE_RATE_KEY("audio", "io/sampleRate");

static const Settings::Key USER_SOUNDFONTS_PATHS("midi", "application/paths/mySoundfonts");

static const AudioResourceId DEFAULT_SOUND_FONT_NAME = "MS Basic";     // "GeneralUser GS v1.471.sf2"; // "MS Basic.sf3";
static const AudioResourceMeta DEFAULT_AUDIO_RESOURCE_META
    = { DEFAULT_SOUND_FONT_NAME, AudioResourceType::FluidSoundfont, "Fluid", {}, false /*hasNativeEditor*/ };

void AudioConfiguration::init()
{
    int defaultBufferSize = 0;
#ifdef Q_OS_WASM
    defaultBufferSize = 8192;
#else
    defaultBufferSize = 1024;
#endif
    settings()->setDefaultValue(AUDIO_BUFFER_SIZE_KEY, Val(defaultBufferSize));
    settings()->valueChanged(AUDIO_BUFFER_SIZE_KEY).onReceive(nullptr, [this](const Val&) {
        m_driverBufferSizeChanged.notify();
    });

    settings()->setDefaultValue(AUDIO_API_KEY, Val("Core Audio"));

    settings()->valueChanged(AUDIO_OUTPUT_DEVICE_ID_KEY).onReceive(nullptr, [this](const Val&) {
        m_audioOutputDeviceIdChanged.notify();
    });

    settings()->setDefaultValue(AUDIO_SAMPLE_RATE_KEY, Val(44100));
    settings()->valueChanged(AUDIO_SAMPLE_RATE_KEY).onReceive(nullptr, [this](const Val&) {
        m_driverSampleRateChanged.notify();
    });

    settings()->setDefaultValue(USER_SOUNDFONTS_PATHS, Val(globalConfiguration()->userDataPath() + "/SoundFonts"));
    settings()->valueChanged(USER_SOUNDFONTS_PATHS).onReceive(nullptr, [this](const Val&) {
        m_soundFontDirsChanged.send(soundFontDirectories());
    });

    for (const auto& path : userSoundFontDirectories()) {
        fileSystem()->makePath(path);
    }
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
    settings()->setSharedValue(AUDIO_API_KEY, Val(name));
}

std::string AudioConfiguration::audioOutputDeviceId() const
{
    return settings()->value(AUDIO_OUTPUT_DEVICE_ID_KEY).toString();
}

void AudioConfiguration::setAudioOutputDeviceId(const std::string& deviceId)
{
    settings()->setSharedValue(AUDIO_OUTPUT_DEVICE_ID_KEY, Val(deviceId));
}

async::Notification AudioConfiguration::audioOutputDeviceIdChanged() const
{
    return m_audioOutputDeviceIdChanged;
}

audioch_t AudioConfiguration::audioChannelsCount() const
{
    return AUDIO_CHANNELS;
}

unsigned int AudioConfiguration::driverBufferSize() const
{
    return settings()->value(AUDIO_BUFFER_SIZE_KEY).toInt();
}

void AudioConfiguration::setDriverBufferSize(unsigned int size)
{
    settings()->setSharedValue(AUDIO_BUFFER_SIZE_KEY, Val(static_cast<int>(size)));
}

async::Notification AudioConfiguration::driverBufferSizeChanged() const
{
    return m_driverBufferSizeChanged;
}

samples_t AudioConfiguration::renderStep() const
{
    return 512;
}

unsigned int AudioConfiguration::sampleRate() const
{
    return settings()->value(AUDIO_SAMPLE_RATE_KEY).toInt();
}

void AudioConfiguration::setSampleRate(unsigned int sampleRate)
{
    settings()->setSharedValue(AUDIO_SAMPLE_RATE_KEY, Val(static_cast<int>(sampleRate)));
}

async::Notification AudioConfiguration::sampleRateChanged() const
{
    return m_driverSampleRateChanged;
}

SoundFontPaths AudioConfiguration::soundFontDirectories() const
{
    SoundFontPaths paths = userSoundFontDirectories();
    paths.push_back(globalConfiguration()->appDataPath());

    return paths;
}

io::paths_t AudioConfiguration::userSoundFontDirectories() const
{
    std::string pathsStr = settings()->value(USER_SOUNDFONTS_PATHS).toString();
    return io::pathsFromString(pathsStr);
}

void AudioConfiguration::setUserSoundFontDirectories(const io::paths_t& paths)
{
    settings()->setSharedValue(USER_SOUNDFONTS_PATHS, Val(io::pathsToString(paths)));
}

async::Channel<io::paths_t> AudioConfiguration::soundFontDirectoriesChanged() const
{
    return m_soundFontDirsChanged;
}

AudioInputParams AudioConfiguration::defaultAudioInputParams() const
{
    AudioInputParams result;
    result.resourceMeta = DEFAULT_AUDIO_RESOURCE_META;

    return result;
}

const SynthesizerState& AudioConfiguration::defaultSynthesizerState() const
{
    static SynthesizerState state;
    if (state.isNull()) {
        SynthesizerState::Group gf;
        gf.name = "Fluid";
        gf.vals.push_back(SynthesizerState::Val(SynthesizerState::ValID::SoundFontID, DEFAULT_SOUND_FONT_NAME));
        state.groups.insert({ gf.name, std::move(gf) });
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

io::path_t AudioConfiguration::knownAudioPluginsDir() const
{
    return globalConfiguration()->userAppDataPath() + "/audio_plugins";
}

io::path_t AudioConfiguration::stateFilePath() const
{
    return globalConfiguration()->userAppDataPath() + "/synthesizer.xml";
}

bool AudioConfiguration::readState(const io::path_t& path, SynthesizerState& state) const
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

bool AudioConfiguration::writeState(const io::path_t& path, const SynthesizerState& state)
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
