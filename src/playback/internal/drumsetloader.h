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

#pragma once

#include "async/asyncable.h"

#include "audio/common/audiotypes.h"

#include "notation/iinstrumentsrepository.h"
#include "notation/inotation.h"

#include "engraving/dom/drumset.h"

#include "musesampler/imusesamplerinfo.h"

namespace mu::playback {
class DrumsetLoader : public muse::async::Asyncable
{
    muse::Inject<notation::IInstrumentsRepository> instrumentsRepository;
    muse::Inject<muse::musesampler::IMuseSamplerInfo> museSampler;

public:
    void loadDrumset(notation::INotationPtr notation, const mu::engraving::InstrumentTrackId& trackId,
                     const muse::audio::AudioResourceMeta& resourceMeta);

private:
    void replaceDrumset(notation::INotationPtr notation, const mu::engraving::InstrumentTrackId& trackId,
                        const mu::engraving::Drumset& drumset);

    std::unordered_map<int /*instrumentId*/, std::optional<mu::engraving::Drumset> > m_drumsetCache;
};
}
