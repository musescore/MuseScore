/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

//TODO: remove with global clearing of Q_OS_*** defines
#include <QtGlobal>

#include "global/settings.h"

#include "audio/common/soundfonttypes.h"
#include "audio/common/audioutils.h"
#include "audio/common/rpc/rpcpacker.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::synth;

static const audioch_t AUDIO_CHANNELS = 2;

//TODO: add other setting: audio device etc
static const Settings::Key AUDIO_API_KEY("audio", "io/audioApi");
static const Settings::Key AUDIO_OUTPUT_DEVICE_ID_KEY("audio", "io/outputDevice");
static const Settings::Key AUDIO_BUFFER_SIZE_KEY("audio", "io/bufferSize");
static const Settings::Key AUDIO_SAMPLE_RATE_KEY("audio", "io/sampleRate");
static const Settings::Key AUDIO_MEASURE_INPUT_LAG("audio", "io/measureInputLag");

static const Settings::Key ONLINE_SOUNDS_PROCESS_IN_BACKGROUND("audio", "io/onlineSounds/processInBackground");

static const Settings::Key USER_SOUNDFONTS_PATHS("midi", "application/paths/mySoundfonts");

void AudioConfiguration::init()
{
    settings()->setDefaultValue(AUDIO_BUFFER_SIZE_KEY, Val(1024));
    settings()->valueChanged(AUDIO_BUFFER_SIZE_KEY).onReceive(nullptr, [this](const Val&) {
        m_driverBufferSizeChanged.notify();
        updateSamplesToPreallocate();
    });

#if defined(Q_OS_WIN)
    settings()->setDefaultValue(AUDIO_API_KEY, Val("WASAPI"));
#elif defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    settings()->setDefaultValue(AUDIO_API_KEY, Val("ALSA"));
#endif
    settings()->valueChanged(AUDIO_API_KEY).onReceive(nullptr, [this](const Val&) {
        m_currentAudioApiChanged.notify();
    });

    settings()->setDefaultValue(AUDIO_OUTPUT_DEVICE_ID_KEY, Val(DEFAULT_DEVICE_ID));
    settings()->valueChanged(AUDIO_OUTPUT_DEVICE_ID_KEY).onReceive(nullptr, [this](const Val&) {
        m_audioOutputDeviceIdChanged.notify();
    });

    settings()->setDefaultValue(AUDIO_SAMPLE_RATE_KEY, Val(44100));
    settings()->setCanBeManuallyEdited(AUDIO_SAMPLE_RATE_KEY, false, Val(44100), Val(192000));
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

    settings()->setDefaultValue(AUDIO_MEASURE_INPUT_LAG, Val(false));

    settings()->setDefaultValue(ONLINE_SOUNDS_PROCESS_IN_BACKGROUND, Val(true));
    settings()->valueChanged(ONLINE_SOUNDS_PROCESS_IN_BACKGROUND).onReceive(nullptr, [this](const Val& val) {
        m_autoProcessOnlineSoundsInBackgroundChanged.send(val.toBool());
    });

    updateSamplesToPreallocate();
}

AudioEngineConfig AudioConfiguration::engineConfig() const
{
    AudioEngineConfig conf;
    conf.autoProcessOnlineSoundsInBackground = this->autoProcessOnlineSoundsInBackground();
    return conf;
}

void AudioConfiguration::onWorkerConfigChanged()
{
    rpcChannel()->send(rpc::make_notification(rpc::Method::EngineConfigChanged, rpc::RpcPacker::pack(engineConfig())));
}

std::string AudioConfiguration::currentAudioApi() const
{
    return settings()->value(AUDIO_API_KEY).toString();
}

void AudioConfiguration::setCurrentAudioApi(const std::string& name)
{
    settings()->setSharedValue(AUDIO_API_KEY, Val(name));
}

async::Notification AudioConfiguration::currentAudioApiChanged() const
{
    return m_currentAudioApiChanged;
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

samples_t AudioConfiguration::samplesToPreallocate() const
{
    return m_samplesToPreallocate;
}

async::Channel<samples_t> AudioConfiguration::samplesToPreallocateChanged() const
{
    return m_samplesToPreallocateChanged;
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

io::paths_t AudioConfiguration::soundFontDirectories() const
{
    io::paths_t paths = userSoundFontDirectories();
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

bool AudioConfiguration::autoProcessOnlineSoundsInBackground() const
{
    return settings()->value(ONLINE_SOUNDS_PROCESS_IN_BACKGROUND).toBool();
}

void AudioConfiguration::setAutoProcessOnlineSoundsInBackground(bool value)
{
    settings()->setSharedValue(ONLINE_SOUNDS_PROCESS_IN_BACKGROUND, Val(value));

    onWorkerConfigChanged();
}

async::Channel<bool> AudioConfiguration::autoProcessOnlineSoundsInBackgroundChanged() const
{
    return m_autoProcessOnlineSoundsInBackgroundChanged;
}

bool AudioConfiguration::shouldMeasureInputLag() const
{
    return settings()->value(AUDIO_MEASURE_INPUT_LAG).toBool();
}

void AudioConfiguration::updateSamplesToPreallocate()
{
    samples_t minToReserve = minSamplesToReserve(RenderMode::RealTimeMode);
    samples_t driverBufSize = driverBufferSize();
    samples_t newValue = std::max(minToReserve, driverBufSize);

    if (m_samplesToPreallocate != newValue) {
        m_samplesToPreallocate = newValue;
        m_samplesToPreallocateChanged.send(newValue);
    }
}
