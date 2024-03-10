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

#include "isynthresolver.h"
#include "global/async/asyncable.h"

#ifndef MUSE_AUDIO_MIDIOUTPUTSYNTHCREATOR_H
#define MUSE_AUDIO_MIDIOUTPUTSYNTHCREATOR_H

namespace muse::audio::synth {
class MidiOutputResolver : public ISynthResolver::IResolver, public async::Asyncable
{
public:
    explicit MidiOutputResolver();

    ISynthesizerPtr resolveSynth(const audio::TrackId trackId, const audio::AudioInputParams& params) const override;
    bool hasCompatibleResources(const audio::PlaybackSetupData& setup) const override;

    audio::AudioResourceMetaList resolveResources() const override;
    audio::SoundPresetList resolveSoundPresets(const AudioResourceMeta& resourceMeta) const override;

    void refresh() override;
    void clearSources() override;
};
}

#endif // MUSE_AUDIO_MIDIOUTPUTSYNTHCREATOR_H
