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
#include <engraving/dom/tuplet.h>

using namespace mu::engraving;

namespace mu::iex::guitarpro {
static void createSlightBends(const BendDataContext& bendDataCtx, mu::engraving::Score* score);
static void createPreBends(const BendDataContext& bendDataCtx, mu::engraving::Score* score);
static Note* getLocatedNote(mu::engraving::Score* score, Fraction tickFr, track_idx_t track, size_t noteInChordIdx);
static Chord* getLocatedChord(mu::engraving::Score* score, Fraction tickFr, track_idx_t track);
static void createGraceAfterBends(const BendDataContext& bendDataCtx, mu::engraving::Score* score);
static void createTiedNotesBends(const BendDataContext& bendDataCtx, mu::engraving::Score* score);

BendDataProcessor::BendDataProcessor(mu::engraving::Score* score)
    : m_score(score)
{
}

void BendDataProcessor::processBends(const BendDataContext& bendDataCtx)
{
    createPreBends(bendDataCtx, m_score);
    createSlightBends(bendDataCtx, m_score);
    createGraceAfterBends(bendDataCtx, m_score);
    createTiedNotesBends(bendDataCtx, m_score);

    if (bendDataCtx.splitChordCtx) {
        m_bendDataProcessorSplitChord = std::make_unique<BendDataProcessorSplitChord>(m_score);
        m_bendDataProcessorSplitChord->processBends(*bendDataCtx.splitChordCtx);
    }
}

static void createSlightBends(const BendDataContext& bendDataCtx, mu::engraving::Score* score)
{
    for (const auto& [track, trackInfo] : bendDataCtx.slightBendData) {
        for (const auto& [tick, tickInfo] : trackInfo) {
            Chord* chord = getLocatedChord(score, tick, track);
            for (size_t noteIndex = 0; noteIndex < chord->notes().size(); noteIndex++) {
                if (!muse::contains(tickInfo, noteIndex)) {
                    continue;
                }

                Note* note = chord->notes()[noteIndex];
                const auto& noteBendData = tickInfo.at(noteIndex);
                GuitarBend* bend = chord->score()->addGuitarBend(GuitarBendType::SLIGHT_BEND, note);
                IF_ASSERT_FAILED(bend) {
                    LOGE() << "bend wasn't created for track " << chord->track() << ", tick " << chord->tick().ticks();
                    return;
                }

                bend->setStartTimeFactor(noteBendData.startFactor);
                bend->setEndTimeFactor(noteBendData.endFactor);
            }
        }
    }
}

static void createPreBends(const BendDataContext& bendDataCtx, mu::engraving::Score* score)
{
    for (const auto& [track, trackInfo] : bendDataCtx.prebendData) {
        for (const auto& [tick, tickInfo] : trackInfo) {
            Chord* chord = getLocatedChord(score, tick, track);
            for (size_t noteIndex = 0; noteIndex < chord->notes().size(); noteIndex++) {
                if (!muse::contains(tickInfo, noteIndex)) {
                    continue;
                }

                Note* note = chord->notes()[noteIndex];
                const auto& noteBendData = tickInfo.at(noteIndex);
                GuitarBend* bend = chord->score()->addGuitarBend(GuitarBendType::PRE_BEND, note);
                IF_ASSERT_FAILED(bend) {
                    LOGE() << "bend wasn't created for track " << chord->track() << ", tick " << chord->tick().ticks();
                    return;
                }

                bend->setStartTimeFactor(noteBendData.startFactor);
                bend->setEndTimeFactor(noteBendData.endFactor);

                const int pitch = noteBendData.quarterTones / 2;
                note->setPitch(note->pitch() + pitch);
                note->setTpcFromPitch();
                QuarterOffset quarterOff = noteBendData.quarterTones % 2 ? QuarterOffset::QUARTER_SHARP : QuarterOffset::NONE;
                bend->setEndNotePitch(note->pitch(), quarterOff);
                Note* startNote = bend->startNote();
                if (!startNote) {
                    return;
                }

                startNote->setPitch(note->pitch() - pitch);
                startNote->setTpcFromPitch();

                int newPitch = note->pitch();
                Note* tiedNote = nullptr;

                Tie* tieFor = startNote->tieFor();
                if (tieFor) {
                    tiedNote = tieFor->endNote();
                    startNote->remove(tieFor);
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
    }
}

static Chord* getLocatedChord(mu::engraving::Score* score, Fraction tickFr, track_idx_t track)
{
    const Measure* measure = score->tick2measure(tickFr);
    if (!measure) {
        LOGE() << "bend import error: no valid measure for track " << track << ", tick " << tickFr.ticks();
        return nullptr;
    }

    Chord* chord = measure->findChord(tickFr, track);
    if (!chord) {
        LOGE() << "bend import error: no valid chord for track " << track << ", tick " << tickFr.ticks();
        return nullptr;
    }

    return chord;
}

static Note* getLocatedNote(mu::engraving::Score* score, Fraction tickFr, track_idx_t track, size_t noteInChordIdx)
{
    Chord* chord = getLocatedChord(score, tickFr, track);
    if (chord && noteInChordIdx >= chord->notes().size()) {
        LOGE() << "bend import error: note index invalid for track " << track << ", tick " << tickFr.ticks();
        return nullptr;
    }

    return chord->notes()[noteInChordIdx];
}

static void createGraceAfterBends(const BendDataContext& bendDataCtx, mu::engraving::Score* score)
{
    for (const auto& [track, trackInfo] : bendDataCtx.graceAfterBendData) {
        for (const auto& [tick, tickInfo] : trackInfo) {
            for (const auto& [noteInChordIdx, graceData] : tickInfo) {
                Note* mainNote = getLocatedNote(score, tick, track, noteInChordIdx);
                if (!mainNote) {
                    continue;
                }

                const auto& graceVector = graceData.data;
                bool shouldMoveTie = graceData.shouldMoveTie;

                Note* currentNote = mainNote;
                for (const auto& graceInfo : graceVector) {
                    Chord* graceChord = Factory::createChord(score->dummy()->segment());
                    graceChord->setTrack(track);
                    graceChord->setNoteType(NoteType::GRACE8_AFTER);
                    graceChord->setNoStem(true);

                    TDuration dur;
                    dur.setVal(mu::engraving::Constants::DIVISION / 2);
                    graceChord->setDurationType(dur);
                    graceChord->setTicks(dur.fraction());

                    Note* graceNote = Factory::createNote(graceChord);
                    graceNote->setPitch(currentNote->pitch() + graceInfo.quarterTones / 2);
                    graceNote->setTpcFromPitch();
                    graceChord->add(graceNote);
                    mainNote->chord()->add(graceChord);

                    GuitarBend* bend = score->addGuitarBend(GuitarBendType::BEND, currentNote, graceNote);

                    QuarterOffset quarterOff = graceInfo.quarterTones % 2 ? QuarterOffset::QUARTER_SHARP : QuarterOffset::NONE;
                    bend->setEndNotePitch(bend->startNoteOfChain()->pitch() + graceInfo.quarterTones / 2, quarterOff);
                    bend->setStartTimeFactor(graceInfo.startFactor);
                    bend->setEndTimeFactor(graceInfo.endFactor);

                    currentNote = graceNote;
                }

                int gi = 0;
                for (Chord* c : mainNote->chord()->graceNotes()) {
                    c->setGraceIndex(gi++);
                }

                if (shouldMoveTie) {
                    Tie* tieFor = mainNote->tieFor();
                    if (tieFor && tieFor->endNote()) {
                        Note* tiedNote = tieFor->endNote();
                        mainNote->remove(tieFor);
                        score->addGuitarBend(GuitarBendType::BEND, currentNote, tiedNote);
                    }
                }
            }
        }
    }
}

static void createTiedNotesBends(const BendDataContext& bendDataCtx, mu::engraving::Score* score)
{
    for (const auto& [track, trackInfo] : bendDataCtx.tiedNotesBendsData) {
        for (const auto& [tick, tickInfo] : trackInfo) {
            Chord* chord = getLocatedChord(score, tick, track);
            if (!chord) {
                continue;
            }

            Chord* nextChord = chord->nextTiedChord();
            if (!nextChord) {
                continue;
            }

            for (size_t noteIndex = 0; noteIndex < chord->notes().size(); noteIndex++) {
                if (!muse::contains(tickInfo, noteIndex)) {
                    continue;
                }

                const auto& noteInfo = tickInfo.at(noteIndex);
                Note* startNote = chord->notes()[noteIndex];
                Note* endNote = nextChord->notes()[noteIndex];

                GuitarBend* bend = score->addGuitarBend(GuitarBendType::BEND, startNote, endNote);
                QuarterOffset quarterOff = noteInfo.quarterTones % 2 ? QuarterOffset::QUARTER_SHARP : QuarterOffset::NONE;
                bend->setEndNotePitch(bend->startNoteOfChain()->pitch() + noteInfo.quarterTones / 2, quarterOff);
                bend->setStartTimeFactor(noteInfo.startFactor);
                bend->setEndTimeFactor(noteInfo.endFactor);
                endNote->setHeadHasParentheses(true);

                Tie* tie = startNote->tieFor();
                if (tie) {
                    startNote->remove(tie);
                }

                tie = endNote->tieFor();
                while (tie) {
                    Note* nextNote = tie->endNote();
                    IF_ASSERT_FAILED(nextNote) {
                        LOGE() << "bend import error: not found tied note for track " << track << ", tick " << tick.ticks();
                        break;
                    }

                    nextNote->setPitch(endNote->pitch());
                    nextNote->setTpcFromPitch();
                    nextNote->setHeadHasParentheses(true);
                    tie = nextNote->tieFor();
                }
            }
        }
    }
}
} // namespace mu::iex::guitarpro
