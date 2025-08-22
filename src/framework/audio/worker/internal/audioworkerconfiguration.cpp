/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
#include "audioworkerconfiguration.h"

#include "audio/common/soundfonttypes.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::worker;

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

audioch_t AudioWorkerConfiguration::audioChannelsCount() const
{
    return configuration()->audioChannelsCount();
}

samples_t AudioWorkerConfiguration::samplesToPreallocate() const
{
    return configuration()->samplesToPreallocate();
}

async::Channel<samples_t> AudioWorkerConfiguration::samplesToPreallocateChanged() const
{
    return configuration()->samplesToPreallocateChanged();
}

bool AudioWorkerConfiguration::autoProcessOnlineSoundsInBackground() const
{
    return configuration()->autoProcessOnlineSoundsInBackground();
}

async::Channel<bool> AudioWorkerConfiguration::autoProcessOnlineSoundsInBackgroundChanged() const
{
    return configuration()->autoProcessOnlineSoundsInBackgroundChanged();
}

AudioInputParams AudioWorkerConfiguration::defaultAudioInputParams() const
{
    AudioInputParams result;
    result.resourceMeta = DEFAULT_AUDIO_RESOURCE_META;

    return result;
}

size_t AudioWorkerConfiguration::desiredAudioThreadNumber() const
{
    return 0;
}

size_t AudioWorkerConfiguration::minTrackCountForMultithreading() const
{
    // Start mutlithreading-processing only when there are more or equal number of tracks
    return 2;
}
