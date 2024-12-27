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
#include "benddataprocessor.h"

#include "benddatacontext.h"
#include <engraving/dom/chord.h>
#include <engraving/dom/guitarbend.h>
#include <engraving/dom/factory.h>
#include <engraving/dom/measure.h>
#include <engraving/dom/note.h>
#include <engraving/dom/score.h>
#include <engraving/dom/tie.h>

using namespace mu::engraving;

namespace mu::iex::guitarpro {
static void createGuitarBends(const BendDataContext& bendDataCtx, const mu::engraving::Chord* chord);

BendDataProcessor::BendDataProcessor(mu::engraving::Score* score)
    : m_score(score)
{
}

void BendDataProcessor::processBends(const BendDataContext& bendDataCtx)
{
    for (const auto& [track, trackInfo] : bendDataCtx.bendChordDurations) {
        for (const auto& [mainTick, chordsDurations] : trackInfo) {
            if (chordsDurations.empty()) {
                LOGE() << "bend import error : no chord duration data for track " << track << ", tick " << mainTick;
                continue;
            }

            Fraction mainTickFr = Fraction::fromTicks(mainTick);
            const Measure* mainChordMeasure = m_score->tick2measure(mainTickFr);

            if (!mainChordMeasure) {
                LOGE() << "bend import error : no valid measure for track " << track << ", tick " << mainTick;
                return;
            }

            const Chord* mainChord = mainChordMeasure->findChord(mainTickFr, track);

            if (!mainChord) {
                LOGE() << "bend import error : no valid chord for track " << track << ", tick " << mainTick;
                return;
            }

            Fraction currentTick = mainChord->tick() + mainChord->ticks();
            createGuitarBends(bendDataCtx, mainChord);

            for (size_t i = 1; i < chordsDurations.size(); i++) {
                const Measure* currentMeasure = m_score->tick2measure(currentTick);
                const Segment* curSegment = currentMeasure->findSegment(SegmentType::ChordRest, currentTick);

                if (curSegment) {
                    // adding bend for existing chord
                    const Measure* startMeasure = m_score->tick2measure(currentTick);

                    if (!startMeasure) {
                        LOGE() << "bend import error : no valid measure for track " << track << ", tick " << currentTick;
                        return;
                    }

                    const Chord* existingChord = startMeasure->findChord(currentTick, track);

                    if (!existingChord) {
                        LOGE() << "bend import error : no valid chord for track " << track << ", tick " << currentTick;
                        return;
                    }

                    createGuitarBends(bendDataCtx, existingChord);
                    currentTick += existingChord->ticks();
                } else {
                    // TODO: create new segment and new chord on it
                    LOGE() << "bend wasn't added!";
                }
            }
        }
    }
}

static void createGuitarBends(const BendDataContext& bendDataCtx, const mu::engraving::Chord* chord)
{
    Score* score = chord->score();
    int chordTicks = chord->tick().ticks();

    if (bendDataCtx.bendDataByEndTick.find(chord->track()) == bendDataCtx.bendDataByEndTick.end()) {
        LOGE() << "bend import error: bends data on track " << chord->track() << " doesn't exist";
        return;
    }

    const auto& currentTrackData = bendDataCtx.bendDataByEndTick.at(chord->track());
    if (currentTrackData.find(chordTicks) == currentTrackData.end()) {
        LOGE() << "bend import error: bends data on track " << chord->track() << " doesn't exist for tick " << chordTicks;
        return;
    }

    const BendDataContext::BendChordData& bendChordData = currentTrackData.at(chordTicks);
    const Measure* startMeasure = score->tick2measure(bendChordData.startTick);

    if (!startMeasure) {
        LOGE() << "bend import error : no valid measure for track " << chord->track() << ", tick " << bendChordData.startTick.ticks();
        return;
    }

    const Chord* startChord = startMeasure->findChord(bendChordData.startTick, chord->track());

    if (!startChord) {
        LOGE() << "bend import error : no valid chord for track " << chord->track() << ", tick " << bendChordData.startTick.ticks();
        return;
    }

    std::vector<Note*> startChordNotes = startChord->notes();
    std::vector<Note*> endChordNotes = chord->notes();

    IF_ASSERT_FAILED(startChordNotes.size() == endChordNotes.size()) {
        LOGE() << "bend import error: start and end chord sizes don't match for track " << chord->track() << ", tick " <<
            bendChordData.startTick.ticks();
        return;
    }

    std::sort(startChordNotes.begin(), startChordNotes.end(), [](Note* l, Note* r) {
        return l->pitch() < r->pitch();
    });

    std::sort(endChordNotes.begin(), endChordNotes.end(), [](Note* l, Note* r) {
        return l->pitch() < r->pitch();
    });

    for (size_t noteIndex = 0; noteIndex < endChordNotes.size(); noteIndex++) {
        Note* note = endChordNotes[noteIndex];
        if (bendChordData.noteDataByPitch.find(note->pitch()) == bendChordData.noteDataByPitch.end()) {
            continue;
        }

        const BendDataContext::BendNoteData& bendNoteData = bendChordData.noteDataByPitch.at(note->pitch());
        Note* startNote = startChordNotes[noteIndex];
        int pitch = bendNoteData.quarterTones / 2;

        if (bendNoteData.type == GuitarBendType::PRE_BEND) {
            int pitch = bendNoteData.quarterTones / 2;
            note->setPitch(note->pitch() + pitch);
            note->setTpcFromPitch();
            GuitarBend* bend = score->addGuitarBend(bendNoteData.type, note);
            QuarterOffset quarterOff = bendNoteData.quarterTones % 2 ? QuarterOffset::QUARTER_SHARP : QuarterOffset::NONE;
            bend->setEndNotePitch(note->pitch(), quarterOff);
            Note* startNote = bend->startNote();
            if (startNote) {
                startNote->setPitch(note->pitch() - pitch);
                startNote->setTpcFromPitch();
            }
        } else if (bendNoteData.type == GuitarBendType::SLIGHT_BEND) {
            GuitarBend* bend = score->addGuitarBend(bendNoteData.type, note);
            bend->setStartTimeFactor(bendNoteData.startFactor);
            bend->setEndTimeFactor(bendNoteData.endFactor);
        } else {
            if (startChord == chord) {
                LOGE() << "bend import error : start and end chords are the same for track " << chord->track() << ", tick " <<
                    bendChordData.startTick.ticks();
                return;
            }

            GuitarBend* bend = score->addGuitarBend(bendNoteData.type, startNote, note);
            if (!bend) {
                LOGE() << "bend wasn't created for track " << chord->track() << ", tick " << startChord->tick().ticks();
                return;
            }

            QuarterOffset quarterOff = bendNoteData.quarterTones % 2 ? QuarterOffset::QUARTER_SHARP : QuarterOffset::NONE;
            bend->setEndNotePitch(startNote->pitch() + pitch, quarterOff);
            bend->setStartTimeFactor(bendNoteData.startFactor);
            bend->setEndTimeFactor(bendNoteData.endFactor);
        }

        int newPitch = note->pitch();
        Note* tiedNote = nullptr;

        Tie* tieFor = startNote->tieFor();
        if (tieFor) {
            tiedNote = tieFor->endNote();
            if (bendNoteData.type != GuitarBendType::PRE_BEND) {
                startNote->remove(tieFor);
            }
        }

        while (tiedNote) {
            tiedNote->setPitch(newPitch);
            tiedNote->setTpcFromPitch();
            Tie* tie = tiedNote->tieFor();
            if (!tie) {
                break;
            }

            tiedNote = tie->endNote();
        }
    }
}
} // namespace mu::iex::guitarpro
