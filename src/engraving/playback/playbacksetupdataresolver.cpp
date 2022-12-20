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

#include "playbacksetupdataresolver.h"

#include "mapping/keyboardssetupdataresolver.h"
#include "mapping/stringssetupdataresolver.h"
#include "mapping/windssetupdataresolver.h"
#include "mapping/percussionssetupdataresolver.h"
#include "mapping/voicessetupdataresolver.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::mpe;

void PlaybackSetupDataResolver::resolveSetupData(const Instrument* instrument, mpe::PlaybackSetupData& result) const
{
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

    LOGE() << "Unable to resolve setup data for instrument, id: " << instrument->id()
           << ", family: " << instrument->family();
}

void PlaybackSetupDataResolver::resolveChordSymbolsSetupData(const Instrument* instrument, mpe::PlaybackSetupData& result) const
{
    if (instrument->hasStrings()) {
        static const mpe::PlaybackSetupData CHORD_SYMBOLS_SETUP_DATA = {
            SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                       SoundSubCategory::Nylon,
                                                       SoundSubCategory::Plucked }, {}
        };

        result = CHORD_SYMBOLS_SETUP_DATA;
    } else {
        static const mpe::PlaybackSetupData CHORD_SYMBOLS_SETUP_DATA = {
            SoundId::Piano, SoundCategory::Keyboards, {}, {}
        };

        result = CHORD_SYMBOLS_SETUP_DATA;
    }
}

void PlaybackSetupDataResolver::resolveMetronomeSetupData(mpe::PlaybackSetupData& result) const
{
    static const mpe::PlaybackSetupData METRONOME_SETUP_DATA = {
        SoundId::Block, SoundCategory::Percussions, { SoundSubCategory::Wooden }, {}
    };

    result = METRONOME_SETUP_DATA;
}
