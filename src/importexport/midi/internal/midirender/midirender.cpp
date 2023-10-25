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

#include "midirender.h"
#include "renderstrategy.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/part.h"
#include "engraving/dom/score.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/note.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/navigate.h"
#include "log.h"

namespace mu::iex::midi {
void MidiRender::render(mu::engraving::Score* score, RenderContext& ctx, MidiRenderOutData& outData,
                        RenderStrategy& strategy)
{
    m_ctx = ctx;
    m_score = score;
    for (const auto p: m_score->parts()) {
        // TODO: Figure out what the diff between idx and id and why the differ
        m_ctx.parts().insert({ p->id().toUint64(), {} });
    }
    collectNotes();
    strategy.render(m_ctx, outData, m_renderMeta);
}

void MidiRender::collectNotes()
{
    track_idx_t numTracks = m_score->staves().size() * VOICES;
    for (auto m = m_score->firstMeasure(); m; m = m->nextMeasure()) {
        for (auto s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            for (track_idx_t track = 0; track < numTracks; ++track) {
                Staff* staff = m_score->staff(track / VOICES);
                if (!staff->isPrimaryStaff()) {
                    track += VOICES - 1;
                    continue;
                }
                auto cr = s->element(track);
                if (!cr || !cr->isChord()) [[likely]] {
                    continue;
                }
                Chord* chord = toChord(cr);
                const uint64_t partID = chord->part()->id().toUint64();
                auto graceBefore = chord->graceNotesBefore();
                auto graceAfter = chord->graceNotesBefore();
                auto startTick = chord->tick().ticks();
                if (!graceBefore.empty()) [[unlikely]] {
                    for (const Chord* ch: graceBefore) {
                        auto prevChord = prevChordRest(chord, true, true);
                        collectGraceNotes(true, ch, startTick, partID,
                                          prevChord->isChord() ? toChord(prevChord) : nullptr);
                    }
                }
                if (!graceAfter.empty()) [[unlikely]] {
                    for (const Chord* ch: graceAfter) {
                        collectGraceNotes(false, ch, startTick, partID, chord);
                    }
                }
                for (const auto n: chord->notes()) {
                    // TODO: This code for collecting spanners should be refactored.
                    // Need to figure out why cant use n->isPalmMute;
                    for (auto it: m_score->spannerMap().findOverlapping(startTick + 1, startTick + 2)) {
                        Spanner* spanner = it.value;
                        if (spanner->track() != chord->track()) [[likely]] {
                            continue;
                        }

                        if (spanner->isLetRing()) {
//                                LetRing* letRing = toLetRing(spanner);
                        } else if (spanner->isPalmMute()) {
                            m_ctx.part(partID).palmMuteNotes.insert({ startTick, n });
                            Meta meta;
                            meta.isPalmMute = true;
                            m_renderMeta.insert({ toEngravingItem(spanner), meta });
                        }
                    }
                    if (n->isPalmMute()) {
                        m_ctx.part(partID).palmMuteNotes.insert({ startTick, n });
                    } else if (n->isHammerOn()) {
                        LOGI() << "Hammer on";
                    } else [[likely]] {
                        m_renderMeta.insert({ toEngravingItem(n), {} });
                        m_ctx.part(partID).regularNotes.insert({ startTick, n });
                    }
                }
            }
        }
    }
}

void
MidiRender::collectGraceNotes(bool isBefore, const mu::engraving::Chord* chord, int mainChordStartTick,
                              uint64_t partID,
                              mu::engraving::Chord* dependentItem)
{
    // Collect grace notes
    // Give them the same start tick as root note has
    // We will handle that during events generating (merging)
    for (const auto n: chord->notes()) {
        if (isBefore) [[likely]] {
            m_ctx.part(partID).graceNotesBefore.insert({ mainChordStartTick, n });
        } else {
            m_ctx.part(partID).graceNotesAfter.insert({ mainChordStartTick, n });
        }
        Meta meta;
        meta.isGraceBefore = isBefore;
        meta.isGraceAfter = !isBefore;
        if (dependentItem) {
            for (const auto dependentNote: dependentItem->notes()) {
                meta.dependentItems.push_back(toEngravingItem(dependentNote));
            }
        }
        m_renderMeta.insert({ toEngravingItem(n), meta });
    }
}
}
