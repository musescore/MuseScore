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

#include "playbacksetupdataresolver.h"

#include "mapping/keyboardssetupdataresolver.h"
#include "mapping/stringssetupdataresolver.h"
#include "mapping/windssetupdataresolver.h"
#include "mapping/percussionssetupdataresolver.h"
#include "mapping/voicessetupdataresolver.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse::mpe;

static const PlaybackSetupData PIANO_SETUP_DATA = {
    SoundId::Piano, SoundCategory::Keyboards
};

void PlaybackSetupDataResolver::resolveSetupData(const Instrument* instrument, PlaybackSetupData& result) const
{
    if (!instrument->soundId().empty()) {
        result = PlaybackSetupData::fromString(instrument->soundId());
        result.supportsSingleNoteDynamics = instrument->singleNoteDynamics();
        result.musicXmlSoundId = std::make_optional(instrument->musicXmlId().toStdString());
        return;
    }

    if (KeyboardsSetupDataResolver::resolve(instrument, result)) {
        return;
    }

    if (StringsSetupDataResolver::resolve(instrument, result)) {
        return;
    }

    if (WindsSetupDataResolver::resolve(instrument, result)) {
        return;
    }

    if (PercussionsSetupDataResolver::resolve(instrument, result)) {
        return;
    }

    if (VoicesSetupDataResolver::resolve(instrument, result)) {
        return;
    }

    LOGW() << "Unable to resolve setup data for instrument, id: " << instrument->id()
           << ", family: " << instrument->family() << "; fallback to piano";

    result = PIANO_SETUP_DATA;
}

void PlaybackSetupDataResolver::resolveChordSymbolsSetupData(const Instrument* instrument, PlaybackSetupData& result) const
{
    if (instrument->hasStrings()) {
        static const PlaybackSetupData GUITAR_SETUP_DATA = {
            SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                       SoundSubCategory::Nylon,
                                                       SoundSubCategory::Plucked }
        };

        result = GUITAR_SETUP_DATA;
    } else {
        result = PIANO_SETUP_DATA;
    }
}

void PlaybackSetupDataResolver::resolveMetronomeSetupData(PlaybackSetupData& result) const
{
    static const PlaybackSetupData METRONOME_SETUP_DATA = {
        SoundId::Block, SoundCategory::Percussions, { SoundSubCategory::Wooden }
    };

    result = METRONOME_SETUP_DATA;
}
