/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#ifndef MU_MUSESAMPLER_MUSESAMPLERRESOLVER_H
#define MU_MUSESAMPLER_MUSESAMPLERRESOLVER_H

#include "audio/isynthresolver.h"
#include "modularity/ioc.h"

#include "libhandler.h"
#include "imusesamplerconfiguration.h"
#include "imusesamplerinfo.h"

namespace mu::musesampler {
class MuseSamplerResolver : public audio::synth::ISynthResolver::IResolver, public IMuseSamplerInfo
{
    INJECT(musesampler, IMuseSamplerConfiguration, configuration)

public:
    void init();

    audio::synth::ISynthesizerPtr resolveSynth(const audio::TrackId trackId, const audio::AudioInputParams& params) const override;
    bool hasCompatibleResources(const audio::PlaybackSetupData& setup) const override;
    audio::AudioResourceMetaList resolveResources() const override;
    void refresh() override;
    void clearSources() override;

    std::string version() const override;
    bool isInstalled() const override;

private:
    bool checkLibrary() const;
    bool isVersionSupported() const;
    bool isVersionAboveMinSupported() const;
    bool isVersionBelowMaxSupported() const;

    String buildMuseInstrumentId(const String& category, const String& name, int uniqueId) const;

    MuseSamplerLibHandlerPtr m_libHandler = nullptr;
};
}

#endif // MU_MUSESAMPLER_MUSESAMPLERRESOLVER_H
