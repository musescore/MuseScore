/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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

#include "benddatacollector.h"
#include "divedatacollector.h"
#include "bendbuilder.h"

namespace mu::engraving {
class Note;
class Chord;
class Score;
}

namespace mu::iex::guitarpro {
class GuitarBendImporter
{
public:

    GuitarBendImporter(mu::engraving::Score* score);

    void collectBend(mu::engraving::Note* note, const mu::engraving::PitchValues& pitchValues);
    void collectDive(const mu::engraving::Chord* chord, const mu::engraving::PitchValues& pitchValues);
    void addElementsToScore();

private:

    BendDataCollector m_bendCollector;
    DiveDataCollector m_diveCollector;
    BendBuilder m_bendBuilder;
    mu::engraving::Score* m_score = nullptr;
};
} // mu::iex::guitarpro
