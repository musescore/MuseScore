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
static void createGuitarBends(const BendDataContext& bendDataCtx, mu::engraving::Chord* chord);

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

            Chord* mainChord = mainChordMeasure->findChord(mainTickFr, track);
            if (!mainChord) {
                LOGE() << "bend import error : no valid chord for track " << track << ", tick " << mainTick;
                return;
            }

            Fraction newMainChordDuration = chordsDurations.front();
            mainChord->setTicks(newMainChordDuration);
            mainChord->setDurationType(TDuration(newMainChordDuration));
            Fraction currentTick = mainChord->tick() + mainChord->ticks();
            createGuitarBends(bendDataCtx, mainChord);

            for (size_t i = 1; i < chordsDurations.size(); i++) {
                Measure* currentMeasure = m_score->tick2measure(currentTick);
                if (!currentMeasure) {
                    LOGE() << "bend import error : no valid measure for track " << track << ", tick " << currentTick;
                    return;
                }

                Segment* curSegment = currentMeasure->findSegment(SegmentType::ChordRest, currentTick);
                Chord* currentChord = nullptr;
                Fraction currentChordDuration = chordsDurations[i];
                if (curSegment) {
                    currentChord = currentMeasure->findChord(currentTick, track);
                    if (!currentChord) {
                        LOGE() << "bend import error : no valid chord for track " << track << ", tick " << currentTick;
                        return;
                    }
                } else {
                    curSegment = currentMeasure->getSegment(SegmentType::ChordRest, currentTick);
                    curSegment->setTicks(currentChordDuration);

                    currentChord = Factory::createChord(m_score->dummy()->segment());
                    currentChord->setTrack(track);
                    currentChord->setTicks(currentChordDuration);
                    currentChord->setDurationType(currentChordDuration);
                    curSegment->add(currentChord);

                    for (Note* note : mainChord->notes()) {
                        Note* newChordNote = Factory::createNote(currentChord);
                        newChordNote->setTrack(track);
                        currentChord->add(newChordNote);
                        newChordNote->setPitch(note->pitch());
                        newChordNote->setTpcFromPitch();
                    }
                }

                createGuitarBends(bendDataCtx, currentChord);
                currentTick += currentChordDuration;
            }
        }
    }
}

static void createGuitarBends(const BendDataContext& bendDataCtx, mu::engraving::Chord* chord)
{
    Score* score = chord->score();
    int chordTicks = chord->tick().ticks();

    if (bendDataCtx.bendDataByEndTick.find(chord->track()) == bendDataCtx.bendDataByEndTick.end()) {
        return;
    }

    const auto& currentTrackData = bendDataCtx.bendDataByEndTick.at(chord->track());
    if (currentTrackData.find(chordTicks) == currentTrackData.end()) {
        return;
    }

    const BendDataContext::BendChordData& bendChordData = currentTrackData.at(chordTicks);
    const Measure* startMeasure = score->tick2measure(bendChordData.startTick);
    if (!startMeasure) {
        LOGE() << "bend import error : no valid measure for track " << chord->track() << ", tick " << bendChordData.startTick.ticks();
        return;
    }

    Chord* startChord = startMeasure->findChord(bendChordData.startTick, chord->track());
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
        Note* startNote = startChordNotes[noteIndex];
        Note* note = endChordNotes[noteIndex];

        if (bendChordData.noteDataByIdx.find(noteIndex) == bendChordData.noteDataByIdx.end()) {
            Tie* tie = Factory::createTie(score->dummy());
            startNote->add(tie);
            tie->setEndNote(note);
            note->setTieBack(tie);
            continue;
        }

        const auto& bendNoteData = bendChordData.noteDataByIdx.at(noteIndex);
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
            bend->setEndNotePitch(bend->startNoteOfChain()->pitch() + pitch, quarterOff);
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
