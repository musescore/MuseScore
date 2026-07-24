/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#include "divedatacollector.h"

#include <engraving/dom/chord.h>
#include <engraving/dom/note.h>

#include "diveinfoconverter.h"

using namespace mu::engraving;

namespace mu::iex::guitarpro {
void DiveDataCollector::collectDiveData(const Chord* chord, const PitchValues& pitchValues)
{
    if (pitchValues.size() < 2) {
        return;
    }

    const track_idx_t track = chord->track();
    const Fraction tick = chord->tick();

    bool hasPrecedingWhammy = false;
    if (auto it = m_lastWhammyByTrack.find(track); it != m_lastWhammyByTrack.end()) {
        const auto& prev = it->second;
        hasPrecedingWhammy = (prev.destPitch == pitchValues.front().pitch)
                             && (prev.endTick == tick);
    }
    m_lastWhammyByTrack[track] = { pitchValues.back().pitch, tick + chord->actualTicks() };

    for (const Note* note : chord->notes()) {
        if (note->displayFret() == Note::DisplayFretOption::Hide) {
            continue;
        }
        const bool isContinuedWhammy = hasPrecedingWhammy && pitchValues.front().pitch != 0 && note->tieBack();
        ImportedDiveInfo info = DiveInfoConverter::fillDiveInfo(note, pitchValues, isContinuedWhammy);
        if (info.type == DiveType::NONE || !info.note) {
            continue;
        }

        const size_t noteIdx = muse::indexOf(chord->notes(), note);

        const bool isCompoundPreDive = (info.type == DiveType::PRE_DIVE);

        if (isCompoundPreDive) {
            m_ctx.preDiveData[track][tick][noteIdx].quarterTones = info.quarterTones;
            if (info.graceDiveSegments.empty()) {
                continue;
            }
            // Compound PRE_DIVE: route grace segments through the same path as DIVE
        }

        if (info.graceDiveSegments.size() == 1 && note->tieFor()) {
            m_ctx.tiedNotesDivesData[track][tick][noteIdx] = std::move(info.graceDiveSegments.front());
            continue;
        }

        GraceNotesImportInfo& graceInfo = m_ctx.graceAfterDiveData[track][tick][noteIdx];

        if (note->tieFor() && !info.graceDiveSegments.empty()) {
            graceInfo.lastNoteData.shouldMoveTie = true;
            graceInfo.lastNoteData.endFactor = info.graceDiveSegments.back().endFactor;
            if (info.graceDiveSegments.size() > 1) {
                info.graceDiveSegments.pop_back();
            }
        }

        graceInfo.data = std::move(info.graceDiveSegments);
    }
}
} // namespace mu::iex::guitarpro
