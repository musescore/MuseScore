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
#include "guitarbendimporter.h"

using namespace mu::engraving;

namespace mu::iex::guitarpro {
GuitarBendImporter::GuitarBendImporter(Score* score)
    : m_bendCollector(score), m_score(score)
{
}

void GuitarBendImporter::collectBend(Note* note, const mu::engraving::PitchValues& pitchValues)
{
    m_bendCollector.storeBendData(note, pitchValues);
}

void GuitarBendImporter::collectDive(const mu::engraving::Chord* chord, const mu::engraving::PitchValues& pitchValues)
{
    m_diveCollector.collectDiveData(chord, pitchValues);
}

void GuitarBendImporter::addElementsToScore()
{
    m_bendBuilder.addElementsToScore(m_score, m_bendCollector.collectBendDataContext(), m_diveCollector.context());
}
} // namespace mu::iex::guitarpro
