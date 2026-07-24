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
#include "bendbuilder.h"

#include <algorithm>

#include <engraving/dom/chord.h>
#include <engraving/dom/guitarbend.h>
#include <engraving/dom/factory.h>
#include <engraving/dom/measure.h>
#include <engraving/dom/note.h>
#include <engraving/dom/score.h>
#include <engraving/dom/part.h>
#include <engraving/dom/staff.h>
#include <engraving/dom/stringdata.h>
#include <engraving/dom/tie.h>
#include <engraving/dom/tuplet.h>

#include "../utils.h"

#include "global/log.h"

using namespace mu::engraving;

namespace mu::iex::guitarpro {
static std::vector<Chord*> createGraceChords(Chord* chord, const grace_segment_map_t& bendInfo);
static void reindexGraceNotes(Chord* chord);
static void createSlightBends(const BendDataContext& bendDataCtx, Score* score);
static void createPreBends(const BendDataContext& bendDataCtx, Score* score);
static void createPreDives(const DiveDataContext& diveDataCtx, Score* score);
static void createGraceAfterNotes(const GraceAfterTrackMap& data, GuitarBendType type, Score* score);
static void createTiedSpans(const TiedNotesTrackMap& data, GuitarBendType type, Score* score);

void BendBuilder::addElementsToScore(Score* score, const BendDataContext& bendCtx, const DiveDataContext& diveCtx)
{
    createPreBends(bendCtx, score);
    createSlightBends(bendCtx, score);
    createGraceAfterNotes(bendCtx.graceAfterBendData, GuitarBendType::BEND, score);
    createTiedSpans(bendCtx.tiedNotesBendsData, GuitarBendType::BEND, score);
    createPreDives(diveCtx, score);
    createTiedSpans(diveCtx.tiedNotesDivesData, GuitarBendType::DIVE, score);
    createGraceAfterNotes(diveCtx.graceAfterDiveData, GuitarBendType::DIVE, score);

    if (bendCtx.splitChordCtx) {
        m_bendDataProcessorSplitChord = std::make_unique<BendDataProcessorSplitChord>(score);
        m_bendDataProcessorSplitChord->processBends(*bendCtx.splitChordCtx);
    }
}

static void createSlightBends(const BendDataContext& bendDataCtx, mu::engraving::Score* score)
{
    for (const auto& [track, trackInfo] : bendDataCtx.slightBendData) {
        for (const auto& [tick, tickInfo] : trackInfo) {
            Chord* chord = utils::getLocatedChord(score, tick, track);
            for (size_t noteIndex = 0; noteIndex < chord->notes().size(); noteIndex++) {
                if (!muse::contains(tickInfo, noteIndex)) {
                    continue;
                }

                Note* note = chord->notes()[noteIndex];
                const auto& noteBendData = tickInfo.at(noteIndex);
                GuitarBend* bend = chord->score()->addGuitarBend(GuitarBendType::SLIGHT_BEND, note);
                IF_ASSERT_FAILED(bend) {
                    LOGE() << "bend wasn't created for track " << chord->track() << ", tick " << chord->tick().ticks();
                    continue;
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
            Chord* chord = utils::getLocatedChord(score, tick, track);
            for (size_t noteIndex = 0; noteIndex < chord->notes().size(); noteIndex++) {
                if (!muse::contains(tickInfo, noteIndex)) {
                    continue;
                }

                Note* note = chord->notes()[noteIndex];
                // NOTE: addGuitarBend will create grace note and transpose it down. We need the pitch of current note instead to properly fret the note.
                note->transposeDiatonic(1, true, false);
                const auto& noteBendData = tickInfo.at(noteIndex);
                GuitarBend* bend = chord->score()->addGuitarBend(GuitarBendType::PRE_BEND, note);
                IF_ASSERT_FAILED(bend) {
                    LOGE() << "prebend wasn't created for track " << chord->track() << ", tick " << chord->tick().ticks();
                    continue;
                }

                // NOTE: returning pitch.
                note->transposeDiatonic(-1, true, false);
                bend->setStartTimeFactor(noteBendData.startFactor);
                bend->setEndTimeFactor(noteBendData.endFactor);

                const int pitch = noteBendData.quarterTones / 2;
                note->setPitch(note->pitch() + pitch);
                note->setTpcFromPitch();
                int quarterOff = noteBendData.quarterTones % 2;
                bend->setEndNotePitch(note->pitch(), quarterOff);
                Note* startNote = bend->startNote();
                if (!startNote) {
                    return;
                }

                startNote->setPitch(note->pitch() - pitch);
                startNote->setTpcFromPitch();
                if (note->displayFret() == Note::DisplayFretOption::Hide) {
                    startNote->setDisplayFret(Note::DisplayFretOption::Hide);
                }

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

static void reindexGraceNotes(Chord* chord)
{
    int gi = 0;
    for (Chord* c : chord->graceNotes()) {
        c->setGraceIndex(gi++);
    }
}

static Chord* createGraceAfterChord(Chord* parent)
{
    Chord* graceChord = Factory::createChord(parent->score()->dummy()->segment());
    graceChord->setTrack(parent->track());
    graceChord->setNoteType(NoteType::GRACE8_AFTER);
    graceChord->setNoStem(true);
    graceChord->setBeamMode(BeamMode::NONE);

    TDuration dur;
    dur.setVal(mu::engraving::Constants::DIVISION / 2);
    graceChord->setDurationType(dur);
    graceChord->setTicks(dur.fraction());
    parent->add(graceChord);

    return graceChord;
}

static std::vector<Chord*> createGraceChords(Chord* chord, const grace_segment_map_t& bendInfo)
{
    std::vector<Chord*> graceChords;

    size_t maxGraceAmount = 0;
    for (size_t noteIndex = 0; noteIndex < chord->notes().size(); noteIndex++) {
        if (!muse::contains(bendInfo, noteIndex)) {
            continue;
        }
        maxGraceAmount = std::max(maxGraceAmount, bendInfo.at(noteIndex).data.size());
    }

    for (size_t i = 0; i < maxGraceAmount; i++) {
        graceChords.push_back(createGraceAfterChord(chord));
    }

    return graceChords;
}

static void createGraceAfterNotes(const GraceAfterTrackMap& data, GuitarBendType type, Score* score)
{
    for (const auto& [track, trackInfo] : data) {
        for (const auto& [tick, tickEntry] : trackInfo) {
            Chord* chord = utils::getLocatedChord(score, tick, track);
            std::vector<Chord*> graceChords = createGraceChords(chord, tickEntry);

            for (size_t noteIndex = 0; noteIndex < chord->notes().size(); noteIndex++) {
                if (!muse::contains(tickEntry, noteIndex)) {
                    continue;
                }

                Note* mainNote = chord->notes()[noteIndex];
                const auto& graceData = tickEntry.at(noteIndex);
                const auto& lastGraceData = graceData.lastNoteData;

                const bool isDive = (type == GuitarBendType::DIVE);

                Note* currentNote = mainNote;
                for (size_t graceIndex = 0; graceIndex < graceData.data.size(); graceIndex++) {
                    const auto& noteData = graceData.data[graceIndex];
                    Chord* graceChord = graceChords[graceIndex];

                    Note* graceNote = Factory::createNote(graceChord);
                    const int gracePitch = currentNote->pitch() + noteData.quarterTones / 2;
                    graceNote->setPitch(gracePitch);
                    graceNote->setTpcFromPitch();
                    graceChord->add(graceNote);

                    if (isDive) {
                        const Staff* staff = mainNote->staff();
                        const StringData* sd = mainNote->part()->stringData(mainNote->tick(), staff->idx());
                        const int offset = staff->pitchOffset(mainNote->tick());
                        const int fret = sd->fret(gracePitch + offset, mainNote->string(), staff);
                        if (fret >= 0) {
                            graceNote->setString(mainNote->string());
                            graceNote->setFret(fret);
                        }
                    }

                    GuitarBend* bend = score->addGuitarBend(type, currentNote, graceNote);
                    IF_ASSERT_FAILED(bend) {
                        LOGE() << "grace-after spanner not created for track " << track
                               << ", tick " << tick.ticks();
                        break;
                    }

                    // Bend quarterTones are absolute from chain start, dive quarterTones are deltas
                    const int endPitch = isDive
                                         ? currentNote->pitch() + noteData.quarterTones / 2
                                         : bend->startNoteOfChain()->pitch() + noteData.quarterTones / 2;

                    bend->setEndNotePitch(endPitch, noteData.quarterTones % 2);
                    bend->setStartTimeFactor(noteData.startFactor);
                    bend->setEndTimeFactor(noteData.endFactor);

                    currentNote = graceNote;
                }

                reindexGraceNotes(mainNote->chord());

                if (lastGraceData.shouldMoveTie) {
                    Tie* tieFor = mainNote->tieFor();
                    if (tieFor && tieFor->endNote()) {
                        Note* tiedNote = tieFor->endNote();
                        mainNote->remove(tieFor);
                        GuitarBend* tieBend = score->addGuitarBend(type, currentNote, tiedNote);
                        if (tieBend) {
                            tieBend->setEndTimeFactor(lastGraceData.endFactor);
                        }
                    }
                }
            }
        }
    }
}

static void createPreDives(const DiveDataContext& diveDataCtx, Score* score)
{
    for (const auto& [track, trackInfo] : diveDataCtx.preDiveData) {
        for (const auto& [tick, tickInfo] : trackInfo) {
            Chord* chord = utils::getLocatedChord(score, tick, track);
            if (!chord) {
                continue;
            }

            for (size_t noteIndex = 0; noteIndex < chord->notes().size(); noteIndex++) {
                if (!muse::contains(tickInfo, noteIndex)) {
                    continue;
                }

                Note* note = chord->notes()[noteIndex];
                const SegmentData& pd = tickInfo.at(noteIndex);

                const int writtenPitch = note->pitch();
                const int originalString = note->string();
                const int originalFret = note->fret();
                const int attackPitch = writtenPitch - pd.quarterTones / 2;

                note->transposeDiatonic(-1, true, false);
                GuitarBend* preDive = score->addGuitarBend(GuitarBendType::PRE_DIVE, note);
                note->transposeDiatonic(1, true, false);

                IF_ASSERT_FAILED(preDive) {
                    LOGE() << "pre-dive not created for track " << track << ", tick " << tick.ticks();
                    continue;
                }

                Note* ghostNote = preDive->startNote();
                if (ghostNote) {
                    ghostNote->setPitch(writtenPitch);
                    ghostNote->setTpcFromPitch();
                    ghostNote->setString(originalString);
                    ghostNote->setFret(originalFret);
                }

                note->setPitch(attackPitch);
                note->setTpcFromPitch();
                note->setString(originalString);
                note->setFret(originalFret);
            }
        }
    }
}

static void createTiedSpans(const TiedNotesTrackMap& data, GuitarBendType type, Score* score)
{
    const bool isDive = (type == GuitarBendType::DIVE);

    for (const auto& [track, trackInfo] : data) {
        for (const auto& [tick, tickEntry] : trackInfo) {
            Chord* chord = utils::getLocatedChord(score, tick, track);
            if (!chord) {
                continue;
            }

            for (size_t noteIndex = 0; noteIndex < chord->notes().size(); noteIndex++) {
                if (!muse::contains(tickEntry, noteIndex)) {
                    continue;
                }

                const auto& noteInfo = tickEntry.at(noteIndex);
                Note* startNote = chord->notes()[noteIndex];
                Note* endNote = startNote->tieFor() ? startNote->tieFor()->endNote() : nullptr;
                if (!endNote) {
                    LOGE() << "bend/dive import error: not found tied note for track " << track << ", tick " << tick.ticks();
                    continue;
                }

                Tie* tie = startNote->tieFor();
                if (tie) {
                    startNote->remove(tie);
                }

                GuitarBend* bend = score->addGuitarBend(type, startNote, endNote);
                IF_ASSERT_FAILED(bend) {
                    LOGE() << "bend/dive wasn't created for track " << chord->track() << ", tick " << chord->tick().ticks();
                    continue;
                }

                if (isDive && startNote->tieBack()) {
                    startNote->setPitch(startNote->tieBack()->startNote()->pitch());
                    startNote->setTpcFromPitch();
                }

                int rootPitch = isDive
                                ? startNote->pitch()
                                : bend->startNoteOfChain()->pitch();

                bend->setEndNotePitch(rootPitch + noteInfo.quarterTones / 2, noteInfo.quarterTones % 2);
                bend->setStartTimeFactor(noteInfo.startFactor);
                bend->setEndTimeFactor(noteInfo.endFactor);
                endNote->setParenthesesMode(ParenthesesMode::BOTH);

                Tie* endTie = endNote->tieFor();
                while (endTie) {
                    Note* nextNote = endTie->endNote();
                    IF_ASSERT_FAILED(nextNote) {
                        LOGE() << "bend/dive import error: not found tied note for track " << track << ", tick " << tick.ticks();
                        break;
                    }
                    nextNote->setPitch(endNote->pitch());
                    nextNote->setTpcFromPitch();
                    nextNote->setParenthesesMode(ParenthesesMode::BOTH);
                    endTie = nextNote->tieFor();
                }
            }
        }
    }
}
} // namespace mu::iex::guitarpro
