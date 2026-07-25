/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "editvoice.h"
#include "editchord.h"

#include <vector>

#include "global/containers.h"

#include "../dom/articulation.h"
#include "../dom/chord.h"
#include "../dom/excerpt.h"
#include "../dom/factory.h"
#include "../dom/lyrics.h"
#include "../dom/masterscore.h"
#include "../dom/measure.h"
#include "../dom/note.h"
#include "../dom/partialtie.h"
#include "../dom/rest.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/select.h"
#include "../dom/slur.h"
#include "../dom/spanner.h"
#include "../dom/staff.h"
#include "../dom/tie.h"
#include "../dom/tuplet.h"

using namespace mu;
using namespace mu::engraving;

//---------------------------------------------------------
//   changeSelectedElementsVoice
//    moves selected notes into specified voice if possible
//---------------------------------------------------------

void EditVoice::changeSelectedElementsVoice(Transaction&, Score* score, voice_idx_t voice)
{
    auto isOperationValid = [](Score* score, track_idx_t origTrack, track_idx_t dstTrack) {
        Excerpt* excerpt = score->excerpt();
        if (!excerpt) {
            return true;
        }

        const track_idx_t dstTrackMapped = muse::key(excerpt->tracksMapping(), dstTrack, muse::nidx);
        if (dstTrackMapped == muse::nidx) {
            return false;
        }

        const track_idx_t origTrackMapped = muse::key(excerpt->tracksMapping(), origTrack, muse::nidx);
        if (origTrackMapped == dstTrack) {
            return false;
        }

        return true;
    };

    // Track groups of parentheses to recreate in new chord
    struct MovedParenGroup {
        Chord* dstChord = nullptr;
        bool generated = false;
        std::vector<Note*> notes;
    };
    std::map<const NoteParenthesisInfo*, MovedParenGroup> movedParenGroups;

    std::vector<EngravingItem*> newElements;
    std::vector<EngravingItem*> oel = score->selection().elements();       // make copy
    for (EngravingItem* e : oel) {
        if (e->isNote()) {
            Note* note   = toNote(e);
            Chord* chord = note->chord();

            // move grace notes with main chord only
            if (chord->isGrace() || chord->isTrillCueNote()) {
                continue;
            }

            if (chord->voice() == voice) {
                continue;
            }

            Score* elementScore = score;
            if (score->excerpt() && !chord->staff()->isVoiceVisible(voice)) {
                // We are on a linked stave with the desired voice not visible
                // Get note in main score to continue
                note = toNote(note->findLinkedInScore(score->masterScore()));
                chord = note->chord();
                elementScore = score->masterScore();
            }
            Segment* s       = chord->segment();
            Measure* m       = s->measure();
            size_t notes     = chord->notes().size();
            track_idx_t dstTrack = chord->staffIdx() * VOICES + voice;
            ChordRest* dstCR = toChordRest(s->element(dstTrack));
            Chord* dstChord  = nullptr;

            if (!isOperationValid(elementScore, chord->track(), dstTrack)) {
                continue;
            }

            // set up destination chord

            if (dstCR && dstCR->globalTicks() == chord->globalTicks()) {
                // existing chord/rest in destination with correct duration
                //   this case allows for tuplets, unlike the more general case below
                if (dstCR->isChord()) {
                    // can simply move note in
                    dstChord = toChord(dstCR);
                } else {
                    // replace rest with chord, then move note in
                    dstChord = Factory::createChord(s);
                    dstChord->setTrack(dstTrack);
                    dstChord->setDurationType(chord->durationType());
                    dstChord->setTicks(chord->ticks());
                    dstChord->setTuplet(dstCR->tuplet());
                    dstChord->setParent(s);
                    elementScore->undoRemoveElement(dstCR);
                }
            } else if (!chord->tuplet()) {
                // rests or gap in destination
                //   insert new chord if the rests / gap are long enough
                //   then move note in
                bool hasIncompatibleTuplet = false;
                const Chord* cBefore = nullptr;
                const Chord* cAfterStart = nullptr;
                for (const Segment* s2 = m->first(SegmentType::ChordRest); s2; s2 = s2->next(SegmentType::ChordRest)) {
                    const ChordRest* cr2 = toChordRest(s2->element(dstTrack));
                    if (!cr2) {
                        continue;
                    }
                    if (const Tuplet* topTuplet = cr2->topTuplet()) {
                        if (topTuplet->tick() < s->tick()
                            && topTuplet->endTick() > s->tick()) {
                            hasIncompatibleTuplet = true;
                            break;
                        }
                        if (topTuplet->tick() < chord->endTick()
                            && topTuplet->endTick() > chord->endTick()) {
                            hasIncompatibleTuplet = true;
                            break;
                        }
                    }
                    if (!cr2->isChord()) {
                        continue;
                    }
                    if (s2->tick() < s->tick()) {
                        cBefore = toChord(cr2);
                    }
                    if (s2->tick() >= s->tick()) {
                        cAfterStart = toChord(cr2);
                    }
                    if (s2->tick() >= chord->endTick()) {
                        break;
                    }
                }
                if (hasIncompatibleTuplet) {
                    continue;
                }
                if (cBefore && cBefore->endTick() > s->tick()) {
                    // previous chord overlaps
                    continue;
                }
                if (cAfterStart && cAfterStart->tick() < chord->endTick()) {
                    // next chord overlaps
                    continue;
                }
                // big enough gap found
                dstChord = Factory::createChord(s);
                dstChord->setTrack(dstTrack);
                dstChord->setDurationType(chord->durationType());
                dstChord->setTicks(chord->ticks());
                dstChord->setParent(s);
                // makeGapVoice will not back-fill an empty voice
                if (voice && !dstCR) {
                    elementScore->expandVoice(s, /*m->first(SegmentType::ChordRest,*/ dstTrack);
                }
                elementScore->makeGapVoice(s, dstTrack, chord->ticks(), s->tick());
            }

            if (!dstChord) {
                continue;
            }

            // move note to destination chord

            // create & add new note
            Note* newNote = Factory::copyNote(*note);
            newNote->setSelected(false);
            newNote->setParent(dstChord);
            elementScore->undoAddElement(newNote);
            newElements.push_back(newNote);
            // add new chord if one was created
            if (dstChord != dstCR) {
                elementScore->undoAddCR(dstChord, m, s->tick());
            }
            for (EngravingObject* linked : note->linkList()) {
                Note* linkedNote = toNote(linked);
                Note* linkedNewNote = linked == note ? newNote : toNote(newNote->findLinkedInStaff(linkedNote->staff()));
                // reconnect the tie to this note, if any
                Tie* tie = linkedNote->tieBack();
                if (tie) {
                    Note* startNote = tie->isPartialTie() && !toPartialTie(tie)->isOutgoing() ? nullptr : tie->startNote();
                    elementScore->undoChangeSpannerElements(tie, startNote, linkedNewNote);
                }
                // reconnect the tie from this note, if any
                tie = linkedNote->tieFor();
                if (tie) {
                    elementScore->undoChangeSpannerElements(tie, linkedNewNote, tie->endNote());
                }

                // Reconnect note anchored spanners
                for (EngravingItem* item : linkedNote->spannerBack()) {
                    if (!item || !item->isSpanner()) {
                        continue;
                    }
                    Spanner* spanner = toSpanner(item);
                    elementScore->undoChangeSpannerElements(spanner, spanner->startElement(), linkedNewNote);
                }

                for (EngravingItem* item : linkedNote->spannerFor()) {
                    if (!item || !item->isSpanner()) {
                        continue;
                    }
                    Spanner* spanner = toSpanner(item);
                    elementScore->undoChangeSpannerElements(spanner, linkedNewNote, spanner->endElement());
                }
            }

            // Move paren to corresponding group in new chord
            if (!chord->noteParentheses().empty()) {
                const NoteParenthesisInfo* noteParenInfo = chord->findNoteParenthesisInfo(note);
                if (noteParenInfo) {
                    bool generated = noteParenInfo->leftParen()->generated();
                    EditChord::removeChordParentheses(chord, { note }, /*addToLinked*/ true, generated);

                    MovedParenGroup& movedGroup = movedParenGroups[noteParenInfo];
                    movedGroup.dstChord = dstChord;
                    movedGroup.generated = generated;
                    movedGroup.notes.push_back(newNote);
                }
            }

            // remove original note
            if (notes > 1) {
                elementScore->undoRemoveElement(note);
            } else if (notes == 1) {
                // take care of slurs
                int currentTick = chord->tick().ticks();
                for (auto it : elementScore->spannerMap().findOverlapping(currentTick, currentTick + 1)) {
                    Spanner* spanner = it.value;
                    if (!spanner->isSlur()) {
                        continue;
                    }
                    Slur* slur = toSlur(spanner);
                    if (slur->startElement() == chord && slur->endElement() == chord) {
                        elementScore->undoChangeSpannerElements(slur, dstChord, dstChord);
                        slur->undoChangeProperty(Pid::VOICE, voice);
                    } else if (slur->startElement() == chord) {
                        elementScore->undoChangeSpannerElements(slur, dstChord, slur->endElement());
                    } else if (slur->endElement() == chord) {
                        elementScore->undoChangeSpannerElements(slur, slur->startElement(), dstChord);
                    }
                }
                // move articulations
                for (Articulation* artic : chord->articulations()) {
                    if (dstChord->hasArticulation(artic)) {
                        continue;
                    }
                    elementScore->undoChangeParent(artic, dstChord, dstChord->staffIdx());
                }
                // create rest to leave behind
                Rest* r = Factory::createRest(s);
                r->setTrack(chord->track());
                r->setDurationType(chord->durationType());
                r->setTicks(chord->ticks());
                r->setTuplet(chord->tuplet());
                r->setParent(s);
                // if there were grace notes, move them
                while (!chord->graceNotes().empty()) {
                    Chord* gc = chord->graceNotes().front();
                    Chord* ngc = Factory::copyChord(*gc);
                    elementScore->undoRemoveElement(gc);
                    ngc->setParent(dstChord);
                    ngc->setTrack(dstChord->track());
                    elementScore->undoAddElement(ngc);
                }
                // remove chord, replace with rest
                elementScore->undoRemoveElement(chord);
                elementScore->undoAddCR(r, m, s->tick());
            }

            // Move lyrics
            for (Lyrics* lyric : chord->lyrics()) {
                if (!lyric || dstChord->lyrics(lyric->verse(), lyric->placement())) {
                    continue;
                }

                Lyrics* newLyric = Factory::copyLyrics(*lyric);
                newLyric->setParent(dstChord);
                newLyric->setSelected(false);
                newLyric->setTrack(dstTrack);
                elementScore->undoAddElement(newLyric);
                newElements.push_back(newLyric);

                elementScore->undoRemoveElement(lyric);
            }
        } else if (e->hasVoiceAssignmentProperties()) {
            if (e->isSpannerSegment()) {
                e = toSpannerSegment(e)->spanner();
            }

            track_idx_t dstTrack = e->staffIdx() * VOICES + voice;
            if (!isOperationValid(score, e->track(), dstTrack)) {
                continue;
            }

            e->undoChangeProperty(Pid::VOICE, voice);
            e->undoChangeProperty(Pid::VOICE_ASSIGNMENT, VoiceAssignment::CURRENT_VOICE_ONLY);
            newElements.push_back(e);
        }
    }

    // Recreate parenthesis groups
    for (auto& pair : movedParenGroups) {
        MovedParenGroup& movedGroup = pair.second;
        EditChord::addChordParentheses(movedGroup.dstChord, movedGroup.notes, /*addToLinked*/ true, movedGroup.generated);
    }

    if (!newElements.empty()) {
        score->selection().clear();
        score->select(newElements, SelectType::ADD, muse::nidx);
    }

    score->setLayoutAll();
}

void EditVoice::changeSelectedElementsVoiceAssignment(Transaction&, Score* score, VoiceAssignment voiceAssignment)
{
    std::vector<EngravingItem*> newElements;

    for (EngravingItem* e : score->selection().elements()) {
        if (e->hasVoiceAssignmentProperties()) {
            e->undoChangeProperty(Pid::VOICE_ASSIGNMENT, voiceAssignment);
            newElements.push_back(e);
        }
    }

    if (!newElements.empty()) {
        score->selection().clear();
        score->select(newElements, SelectType::ADD, muse::nidx);
    }
}
