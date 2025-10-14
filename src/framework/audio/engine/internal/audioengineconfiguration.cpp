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
#include "audioengineconfiguration.h"

#include "audio/common/audiosanitizer.h"
#include "audio/common/soundfonttypes.h"
#include "audio/common/rpc/rpcpacker.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::engine;
using namespace muse::audio::rpc;

static const AudioResourceId DEFAULT_SOUND_FONT_NAME = "MS Basic";
static const AudioResourceAttributes DEFAULT_AUDIO_RESOURCE_ATTRIBUTES = {
    { PLAYBACK_SETUP_DATA_ATTRIBUTE, muse::mpe::GENERIC_SETUP_DATA_STRING },
    { synth::SOUNDFONT_NAME_ATTRIBUTE, String::fromStdString(DEFAULT_SOUND_FONT_NAME) } };

static const AudioResourceMeta DEFAULT_AUDIO_RESOURCE_META = {
    DEFAULT_SOUND_FONT_NAME,
    "Fluid",
    DEFAULT_AUDIO_RESOURCE_ATTRIBUTES,
    AudioResourceType::FluidSoundfont,
    false /*hasNativeEditor*/ };

void AudioEngineConfiguration::setConfig(const AudioEngineConfig& conf)
{
    ONLY_AUDIO_ENGINE_THREAD;

    if (conf.autoProcessOnlineSoundsInBackground != m_conf.autoProcessOnlineSoundsInBackground) {
        m_conf.autoProcessOnlineSoundsInBackground = conf.autoProcessOnlineSoundsInBackground;
        m_autoProcessOnlineSoundsInBackgroundChanged.send(m_conf.autoProcessOnlineSoundsInBackground);
    }
}

bool AudioEngineConfiguration::autoProcessOnlineSoundsInBackground() const
{
    return m_conf.autoProcessOnlineSoundsInBackground;
}

async::Channel<bool> AudioEngineConfiguration::autoProcessOnlineSoundsInBackgroundChanged() const
{
    return m_autoProcessOnlineSoundsInBackgroundChanged;
}

AudioInputParams AudioEngineConfiguration::defaultAudioInputParams() const
{
    AudioInputParams result;
    result.resourceMeta = DEFAULT_AUDIO_RESOURCE_META;

    return result;
}

size_t AudioEngineConfiguration::desiredAudioThreadNumber() const
{
    return 0;
}

size_t AudioEngineConfiguration::minTrackCountForMultithreading() const
{
    // Start mutlithreading-processing only when there are more or equal number of tracks
    return 2;
}
