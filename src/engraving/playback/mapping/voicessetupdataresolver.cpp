/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "voicessetupdataresolver.h"

using namespace mu::engraving;
using namespace muse::mpe;
using namespace muse;

PlaybackSetupData VoicesSetupDataResolver::doResolve(const Instrument* instrument)
{
    static const std::unordered_map<std::string, mpe::PlaybackSetupData> SETUP_DATA_MAP = {
        { "boy-soprano", { SoundId::Choir, SoundCategory::Voices, { SoundSubCategory::Soprano,
                                                                    SoundSubCategory::Boy } } },
        { "soprano", { SoundId::Choir, SoundCategory::Voices, { SoundSubCategory::Soprano } } },
        { "soprano-c-clef", { SoundId::Choir, SoundCategory::Voices, { SoundSubCategory::Soprano } } },
        { "mezzo-soprano", { SoundId::Choir, SoundCategory::Voices, { SoundSubCategory::Soprano } } },
        { "mezzo-soprano-c-clef", { SoundId::Choir, SoundCategory::Voices, { SoundSubCategory::Soprano } } },
        { "countertenor", { SoundId::Choir, SoundCategory::Voices, { SoundSubCategory::Counter_Tenor } } },
        { "alto", { SoundId::Choir, SoundCategory::Voices, { SoundSubCategory::Alto } } },
        { "alto-c-clef", { SoundId::Choir, SoundCategory::Voices, { SoundSubCategory::Alto } } },
        { "contralto", { SoundId::Choir, SoundCategory::Voices, { SoundSubCategory::Contra_Alto } } },
        { "tenor", { SoundId::Choir, SoundCategory::Voices, { SoundSubCategory::Tenor } } },
        { "tenor-c-clef", { SoundId::Choir, SoundCategory::Voices, { SoundSubCategory::Tenor } } },
        { "baritone", { SoundId::Choir, SoundCategory::Voices, { SoundSubCategory::Baritone } } },
        { "baritone-c-clef", { SoundId::Choir, SoundCategory::Voices, { SoundSubCategory::Baritone } } },
        { "bass", { SoundId::Choir, SoundCategory::Voices, { SoundSubCategory::Bass } } },
        { "voice", { SoundId::Choir, SoundCategory::Voices } },
        { "women", { SoundId::Choir, SoundCategory::Voices, { SoundSubCategory::Female } } },
        { "men", { SoundId::Choir, SoundCategory::Voices, { SoundSubCategory::Male } } },
    };

    auto search = SETUP_DATA_MAP.find(instrument->id().toStdString());
    if (search == SETUP_DATA_MAP.cend()) {
        static PlaybackSetupData empty;
        return empty;
    }

    return search->second;
}
