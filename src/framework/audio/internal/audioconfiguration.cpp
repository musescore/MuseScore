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

//TODO: remove with global clearing of Q_OS_*** defines
#include <QtGlobal>

#include "global/settings.h"

#include "soundfonttypes.h"

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
static const Settings::Key AUDIO_DESIRED_THREAD_NUMBER_KEY("audio", "io/audioThreads");

static const Settings::Key USER_SOUNDFONTS_PATHS("midi", "application/paths/mySoundfonts");

static const AudioResourceId DEFAULT_SOUND_FONT_NAME = "MS Basic";
static const AudioResourceAttributes DEFAULT_AUDIO_RESOURCE_ATTRIBUTES = {
    { PLAYBACK_SETUP_DATA_ATTRIBUTE, muse::mpe::GENERIC_SETUP_DATA_STRING },
    { SOUNDFONT_NAME_ATTRIBUTE, String::fromStdString(DEFAULT_SOUND_FONT_NAME) } };

static const AudioResourceMeta DEFAULT_AUDIO_RESOURCE_META
    = { DEFAULT_SOUND_FONT_NAME, AudioResourceType::FluidSoundfont, "Fluid", DEFAULT_AUDIO_RESOURCE_ATTRIBUTES, false /*hasNativeEditor*/ };

void AudioConfiguration::init()
{
    int defaultBufferSize = 0;
#if defined(Q_OS_WASM)
    defaultBufferSize = 8192;
#else
    defaultBufferSize = 1024;
#endif
    settings()->setDefaultValue(AUDIO_BUFFER_SIZE_KEY, Val(defaultBufferSize));
    settings()->valueChanged(AUDIO_BUFFER_SIZE_KEY).onReceive(nullptr, [this](const Val&) {
        m_driverBufferSizeChanged.notify();
        updateSamplesToPreallocate();
    });

    settings()->setDefaultValue(AUDIO_API_KEY, Val("Core Audio"));

    settings()->setDefaultValue(AUDIO_OUTPUT_DEVICE_ID_KEY, Val(DEFAULT_DEVICE_ID));
    settings()->valueChanged(AUDIO_OUTPUT_DEVICE_ID_KEY).onReceive(nullptr, [this](const Val&) {
        m_audioOutputDeviceIdChanged.notify();
    });

    settings()->setDefaultValue(AUDIO_SAMPLE_RATE_KEY, Val(48000));
    settings()->setCanBeManuallyEdited(AUDIO_SAMPLE_RATE_KEY, false, Val(48000), Val(192000));
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

    settings()->setDefaultValue(AUDIO_DESIRED_THREAD_NUMBER_KEY, Val(0));

    updateSamplesToPreallocate();
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

msecs_t AudioConfiguration::audioWorkerInterval(const samples_t samples, const sample_rate_t sampleRate) const
{
    msecs_t interval = float(samples) / 4.f / float(sampleRate) * 1000.f;
    interval = std::max(interval, msecs_t(1));

    // Found experementaly on a slow laptop (2 core) running on battery power
    interval = std::min(interval, msecs_t(10));

    return interval;
}

samples_t AudioConfiguration::minSamplesToReserve(RenderMode mode) const
{
    // Idle: render as little as possible for lower latency
    if (mode == RenderMode::IdleMode) {
        return 128;
    }

    // Active: render more for better quality (rendering is usually much heavier in this scenario)
    return 1024;
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

size_t AudioConfiguration::desiredAudioThreadNumber() const
{
    return settings()->value(AUDIO_DESIRED_THREAD_NUMBER_KEY).toInt();
}

int AudioConfiguration::audioDelayCompensate() const
{
    return m_audioDelayCompensate;
}

void AudioConfiguration::setAudioDelayCompensate(const int frames)
{
    m_audioDelayCompensate = frames;
}

size_t AudioConfiguration::minTrackCountForMultithreading() const
{
    // Start mutlithreading-processing only when there are more or equal number of tracks
    return 2;
}

AudioInputParams AudioConfiguration::defaultAudioInputParams() const
{
    AudioInputParams result;
    result.resourceMeta = DEFAULT_AUDIO_RESOURCE_META;

    return result;
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
