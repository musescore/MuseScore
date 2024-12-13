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

#include <engraving/types/fraction.h>
#include <engraving/types/pitchvalue.h>
#include <engraving/dom/types.h>
#include "benddatacollector.h"
#include "benddataprocessor.h"

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
    void applyBendsToChords();

private:

    std::unique_ptr<BendDataCollector> m_dataCollector;
    std::unique_ptr<BendDataProcessor> m_dataProcessor;
};
} // mu::iex::guitarpro
