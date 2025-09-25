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

#pragma once

#include "audio/engine/isynthresolver.h"
#include "imusesamplerinfo.h"

#include "modularity/ioc.h"
#include "imusesamplerconfiguration.h"

#include "libhandler.h"

namespace muse::musesampler {
class MuseSamplerResolver : public audio::synth::ISynthResolver::IResolver, public IMuseSamplerInfo, public Injectable
{
    Inject<IMuseSamplerConfiguration> configuration = { this };

public:
    MuseSamplerResolver(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();

    bool reloadAllInstruments();
    int buildNumber() const;

    audio::synth::ISynthesizerPtr resolveSynth(const audio::TrackId trackId, const audio::AudioInputParams& params,
                                               const audio::OutputSpec& outputSpec) const override;
    bool hasCompatibleResources(const audio::PlaybackSetupData& setup) const override;
    audio::AudioResourceMetaList resolveResources() const override;
    audio::SoundPresetList resolveSoundPresets(const audio::AudioResourceMeta& resourceMeta) const override;
    void refresh() override;
    void clearSources() override;

    const Version& version() const override;
    bool isLoaded() const override;

    float defaultReverbLevel(const String& instrumentSoundId) const override;

    ByteArray drumMapping(int instrumentId) const override;
    std::vector<Instrument> instruments() const override;

private:
    void loadSoundPresetAttributes(audio::SoundPresetAttributes& attributes, int instrumentId, const char* presetCode) const;

    String buildMuseInstrumentId(const String& category, const String& name, int uniqueId) const;

    MuseSamplerLibHandlerPtr m_libHandler = nullptr;

    Version m_samplerVersion;
    int m_samplerBuildNumber = -1;
};
}
