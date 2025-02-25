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
#include "guitarbendimporter.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/note.h"
#include "engraving/dom/score.h"

using namespace mu::engraving;

namespace mu::iex::guitarpro {
GuitarBendImporter::GuitarBendImporter(mu::engraving::Score* score)
    : m_dataCollector(std::make_unique<BendDataCollector>()),
    m_dataProcessor(std::make_unique<BendDataProcessor>(score))
{
}

void GuitarBendImporter::collectBend(mu::engraving::Note* note, const mu::engraving::PitchValues& pitchValues)
{
    m_dataCollector->storeBendData(note, pitchValues);
}

void GuitarBendImporter::applyBendsToChords()
{
    m_dataProcessor->processBends(m_dataCollector->collectBendDataContext());
    // BendDataContext ctx;
    // {
    //     BendDataContext::GraceAfterBendData data;
    //     data.quarterTones = 4;
    //     ctx.graceAfterBendData[0][0][0].push_back(data);
    // }

    // {
    //     BendDataContext::GraceAfterBendData data;
    //     data.quarterTones = -4;
    //     ctx.graceAfterBendData[0][0][0].push_back(data);
    // }

    // m_dataProcessor->processBends(ctx);
}
} // namespace mu::iex::guitarpro
