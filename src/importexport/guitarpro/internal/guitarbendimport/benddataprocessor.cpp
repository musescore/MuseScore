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

#ifdef SPLIT_CHORD_DURATIONS
static void createSplitDurationBends(const BendDataContext& bendDataCtx, mu::engraving::Score* score);
static void createSplitDurationBendsForChord(const BendDataContext& bendDataCtx, mu::engraving::Chord* chord);
static void removeReduntantChords(const BendDataContext& bendDataCtx, const mu::engraving::Score* score);
#else
static void createGraceAfterBends(const BendDataContext& bendDataCtx, mu::engraving::Score* score);
static void createTiedNotesBends(const BendDataContext& bendDataCtx, mu::engraving::Score* score);
#endif

BendDataProcessor::BendDataProcessor(mu::engraving::Score* score)
    : m_score(score)
{
}

void BendDataProcessor::processBends(const BendDataContext& bendDataCtx)
{
    createPreBends(bendDataCtx, m_score);
    createSlightBends(bendDataCtx, m_score);

#ifdef SPLIT_CHORD_DURATIONS
    createSplitDurationBends(bendDataCtx, m_score);
    removeReduntantChords(bendDataCtx, m_score);
#else
    createGraceAfterBends(bendDataCtx, m_score);
    createTiedNotesBends(bendDataCtx, m_score);
#endif
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

#ifdef SPLIT_CHORD_DURATIONS
                auto tieBackInfoForTrackIt = bendDataCtx.chordTicksForTieBack.find(chord->track());
                if (tieBackInfoForTrackIt != bendDataCtx.chordTicksForTieBack.end()
                    && (tieBackInfoForTrackIt->second.find(chord->tick()) != tieBackInfoForTrackIt->second.end())) {
                    const Chord* prevChord = chord->prev();
                    IF_ASSERT_FAILED(prevChord) {
                        LOGE() << "bend import error : couldn't add tie, previous chord was null for track " << chord->track() <<
                            ", tick " <<
                            chord->tick().ticks();
                        continue;
                    }

                    // TODO: adapt to creating ties for each note if needed (now only checking up note)
                    Note* endNote = chord->upNote();
                    Note* prevStartNote = prevChord->upNote();

                    Tie* tie = Factory::createTie(chord->score()->dummy());
                    prevStartNote->add(tie);
                    tie->setEndNote(endNote);
                    endNote->setTieBack(tie);

                    continue;
                }
#endif

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

#ifndef SPLIT_CHORD_DURATIONS

static void createGraceAfterBends(const BendDataContext& bendDataCtx, mu::engraving::Score* score)
{
    for (const auto& [track, trackInfo] : bendDataCtx.graceAfterBendData) {
        for (const auto& [tick, tickInfo] : trackInfo) {
            for (const auto& [noteInChordIdx, graceVector] : tickInfo) {
                Note* note = getLocatedNote(score, tick, track, noteInChordIdx);
                if (!note) {
                    continue;
                }

                Note* startNote = note;
                for (const auto& graceInfo : graceVector) {
                    Chord* graceChord = Factory::createChord(score->dummy()->segment());
                    graceChord->setTrack(track);
                    graceChord->setNoteType(NoteType::GRACE8_AFTER);

                    Note* graceNote = Factory::createNote(graceChord);
                    graceNote->setPitch(startNote->pitch() + graceInfo.quarterTones / 2);
                    graceNote->setTpcFromPitch();
                    graceChord->add(graceNote);
                    note->chord()->add(graceChord);

                    GuitarBend* bend = score->addGuitarBend(GuitarBendType::BEND, startNote, graceNote);

                    QuarterOffset quarterOff = graceInfo.quarterTones % 2 ? QuarterOffset::QUARTER_SHARP : QuarterOffset::NONE;
                    bend->setEndNotePitch(bend->startNoteOfChain()->pitch() + graceInfo.quarterTones / 2, quarterOff);
                    bend->setStartTimeFactor(graceInfo.startFactor);
                    bend->setEndTimeFactor(graceInfo.endFactor);

                    startNote = graceNote;
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

#else
static void createSplitDurationBends(const BendDataContext& bendDataCtx, mu::engraving::Score* score)
{
    for (const auto& [track, trackInfo] : bendDataCtx.bendChordDurations) {
        for (const auto& [mainTick, chordsDurations] : trackInfo) {
            if (chordsDurations.empty()) {
                LOGE() << "bend import error : no chord duration data for track " << track << ", tick " << mainTick;
                continue;
            }

            Fraction mainTickFr = Fraction::fromTicks(mainTick);
            const Measure* mainChordMeasure = score->tick2measure(mainTickFr);

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
            Fraction currentActualTick = mainChord->tick() + mainChord->actualTicks();
            createSplitDurationBendsForChord(bendDataCtx, mainChord);

            Tuplet* tuplet = mainChord->tuplet();
            for (size_t i = 1; i < chordsDurations.size(); i++) {
                Measure* currentMeasure = score->tick2measure(currentTick);
                if (!currentMeasure) {
                    LOGE() << "bend import error : no valid measure for track " << track << ", tick " << currentTick.ticks();
                    return;
                }

                Segment* curSegment = currentMeasure->findSegment(SegmentType::ChordRest, currentActualTick);
                Chord* currentChord = nullptr;
                Fraction currentChordDuration = chordsDurations[i];
                Fraction actualDuration = (tuplet ? currentChordDuration / tuplet->ratio() : currentChordDuration);

                if (curSegment) {
                    currentChord = currentMeasure->findChord(currentTick, track);
                    if (!currentChord) {
                        LOGE() << "bend import error : no valid chord for track " << track << ", tick " << currentTick.ticks();
                        return;
                    }

                    currentChord->setTicks(currentChordDuration);
                    currentChord->setDurationType(currentChordDuration);
                } else {
                    curSegment = currentMeasure->getSegment(SegmentType::ChordRest, currentActualTick);
                    currentChord = Factory::createChord(score->dummy()->segment());
                    currentChord->setTrack(track);
                    currentChord->setTicks(currentChordDuration);
                    currentChord->setDurationType(currentChordDuration);
                    if (tuplet) {
                        tuplet->add(currentChord);
                    }

                    curSegment->add(currentChord);

                    for (Note* note : mainChord->notes()) {
                        Note* newChordNote = Factory::createNote(currentChord);
                        newChordNote->setTrack(track);
                        currentChord->add(newChordNote);
                        newChordNote->setPitch(note->pitch());
                        newChordNote->setTpcFromPitch();
                    }
                }

                createSplitDurationBendsForChord(bendDataCtx, currentChord);
                currentTick += currentChordDuration;
                currentActualTick += actualDuration;
            }
        }
    }
}

static void createSplitDurationBendsForChord(const BendDataContext& bendDataCtx, mu::engraving::Chord* chord)
{
    Score* score = chord->score();
    int chordTicks = chord->tick().ticks();

    if (!muse::contains(bendDataCtx.bendDataByEndTick, chord->track())) {
        return;
    }

    const auto& currentTrackData = bendDataCtx.bendDataByEndTick.at(chord->track());
    if (!muse::contains(currentTrackData, chordTicks)) {
        return;
    }

    const BendDataContext::BendChordData& bendChordData = currentTrackData.at(chordTicks);
    const Measure* startMeasure = score->tick2measure(bendChordData.startTick);
    IF_ASSERT_FAILED(startMeasure) {
        LOGE() << "bend import error : no valid measure for track " << chord->track() << ", tick " << bendChordData.startTick.ticks();
        return;
    }

    Chord* startChord = startMeasure->findChord(bendChordData.startTick, chord->track());
    IF_ASSERT_FAILED(startChord) {
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

        if (bendChordData.noteDataByIdx.find(static_cast<int>(noteIndex)) == bendChordData.noteDataByIdx.end()) {
            Tie* tie = Factory::createTie(score->dummy());
            startNote->add(tie);
            tie->setEndNote(note);
            note->setTieBack(tie);
            continue;
        }

        const auto& bendNoteData = bendChordData.noteDataByIdx.at(static_cast<int>(noteIndex));
        const int pitch = bendNoteData.quarterTones / 2;

        IF_ASSERT_FAILED(startChord != chord) {
            LOGE() << "bend import error : start and end chords are the same for track " << chord->track() << ", tick " <<
                bendChordData.startTick.ticks();
            return;
        }

        GuitarBend* bend = score->addGuitarBend(GuitarBendType::BEND, startNote, note);
        IF_ASSERT_FAILED(bend) {
            LOGE() << "bend wasn't created for track " << chord->track() << ", tick " << startChord->tick().ticks();
            return;
        }

        QuarterOffset quarterOff = bendNoteData.quarterTones % 2 ? QuarterOffset::QUARTER_SHARP : QuarterOffset::NONE;
        bend->setEndNotePitch(bend->startNoteOfChain()->pitch() + pitch, quarterOff);
        bend->setStartTimeFactor(bendNoteData.startFactor);
        bend->setEndTimeFactor(bendNoteData.endFactor);

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

static void removeReduntantChords(const BendDataContext& bendDataCtx, const mu::engraving::Score* score)
{
    for (const auto& [track, trackInfo] : bendDataCtx.reduntantChordTicks) {
        for (const Fraction& tick : trackInfo) {
            const Measure* currentMeasure = score->tick2measure(tick);
            IF_ASSERT_FAILED(currentMeasure) {
                LOGE() << "bend import error : couldn't remove invalid chord, no valid measure for track " << track << ", tick " <<
                    tick.ticks();
                continue;
            }

            Segment* curSegment = currentMeasure->findSegment(SegmentType::ChordRest, tick);
            Chord* currentChord = currentMeasure->findChord(tick, track);

            IF_ASSERT_FAILED(curSegment && currentChord) {
                LOGE() << "bend import error : couldn't remove invalid chord, segment or chord was null for track " << track << ", tick " <<
                    tick.ticks();
                continue;
            }

            curSegment->remove(currentChord);
        }
    }
}

#endif
} // namespace mu::iex::guitarpro
