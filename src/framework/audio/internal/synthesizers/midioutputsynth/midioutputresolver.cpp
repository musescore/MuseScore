/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "midioutputresolver.h"
#include "midioutputsynth.h"
#include "internal/audiosanitizer.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::synth;

MidiOutputResolver::MidiOutputResolver()
{
}

ISynthesizerPtr MidiOutputResolver::resolveSynth(const TrackId /*trackId*/, const AudioInputParams& params) const
{
    ONLY_AUDIO_WORKER_THREAD;

    return std::make_shared<MidiOutputSynth>(params);
}

bool MidiOutputResolver::hasCompatibleResources(const PlaybackSetupData& /*setup*/) const
{
    return true;
}

AudioResourceMetaList MidiOutputResolver::resolveResources() const
{
    return AudioResourceMetaList();
}

SoundPresetList MidiOutputResolver::resolveSoundPresets(const AudioResourceMeta&) const
{
    return SoundPresetList();
}

void MidiOutputResolver::refresh()
{
}

void MidiOutputResolver::clearSources()
{
}
