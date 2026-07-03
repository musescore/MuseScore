/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "clonevoice.h"

#include "../dom/beam.h"
#include "../dom/chord.h"
#include "../dom/factory.h"
#include "../dom/interval.h"
#include "../dom/linkedobjects.h"
#include "../dom/measure.h"
#include "../dom/note.h"
#include "../dom/rest.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/spanner.h"
#include "../dom/staff.h"
#include "../dom/tiemap.h"
#include "../dom/tremolotwochord.h"
#include "../dom/tupletmap.h"

#include "transpose.h"

using namespace mu::engraving;

static void doCloneVoice(Score* destScore, track_idx_t srcTrack, track_idx_t dstTrack, Segment* sourceSeg, const Fraction& lTick,
                         bool link = true)
{
    Score* sourceScore = sourceSeg->score();
    Fraction start = sourceSeg->tick();
    TieMap tieMap;
    TupletMap tupletMap;      // tuplets cannot cross measure boundaries
    TremoloTwoChord* tremolo = nullptr;

    auto maybeLinkedClone = [link](EngravingItem* item) -> EngravingItem* {
        // If we want to maintain the link (exchange voice) create a linked clone
        // If we want new, unlinked elements (implode/explode) create a clone
        if (link) {
            return item->linkedClone();
        } else {
            return item->clone();
        }
    };

    for (Segment* oseg = sourceSeg; oseg && oseg->tick() < lTick; oseg = oseg->next1()) {
        Segment* ns = 0;            //create segment later, on demand
        Measure* dm = destScore->tick2measure(oseg->tick());

        EngravingItem* oe = oseg->element(srcTrack);

        if (oe && !oe->generated() && oe->isChordRest()) {
            ChordRest* ocr = toChordRest(oe);
            ChordRest* ncr = toChordRest(maybeLinkedClone(ocr));
            ncr->setScore(destScore);
            ncr->setTrack(dstTrack);

            //Don't clone gaps to a first voice
            if (!(ncr->track() % VOICES) && ncr->isRest()) {
                toRest(ncr)->setGap(false);
            }

            //Handle beams
            if (ocr->beam() && !ocr->beam()->empty() && ocr->beam()->elements().front() == ocr) {
                Beam* nb = ocr->beam()->clone();
                nb->clear();
                nb->setTrack(dstTrack);
                nb->setScore(destScore);
                nb->add(ncr);
                ncr->setBeam(nb);
            }

            // clone Tuplets
            Tuplet* ot = ocr->tuplet();
            if (ot) {
                ot->setTrack(srcTrack);
                Tuplet* nt = tupletMap.findNew(ot);
                if (nt == 0) {
                    nt = toTuplet(maybeLinkedClone(ot));
                    nt->setTrack(dstTrack);
                    nt->setParent(dm);
                    tupletMap.add(ot, nt);

                    Tuplet* nt1 = nt;
                    while (ot->tuplet()) {
                        Tuplet* nt2 = tupletMap.findNew(ot->tuplet());
                        if (nt2 == 0) {
                            nt2 = toTuplet(maybeLinkedClone(ot->tuplet()));
                            nt2->setTrack(dstTrack);
                            nt2->setParent(dm);
                            tupletMap.add(ot->tuplet(), nt2);
                        }
                        nt2->add(nt1);
                        nt1->setTuplet(nt2);
                        ot = ot->tuplet();
                        nt1 = nt2;
                    }
                }
                nt->add(ncr);
                ncr->setTuplet(nt);
            }

            // clone additional settings
            if (oe->isRest()) {
                Rest* ore = toRest(ocr);
                // If we would clone a full measure rest just don't clone this rest
                if (ore->isFullMeasureRest() && (dstTrack % VOICES)) {
                    continue;
                }
            }

            auto cloneChord = [&](Chord* oldChord, Chord* newChord) {
                size_t n = oldChord->notes().size();
                for (size_t i = 0; i < n; ++i) {
                    Note* on = oldChord->notes().at(i);
                    Note* nn = newChord->notes().at(i);
                    staff_idx_t idx = track2staff(dstTrack);
                    Fraction tick = oseg->tick();
                    Interval v = destScore->staff(idx) ? destScore->staff(idx)->transpose(tick) : Interval();
                    nn->setTpc1(on->tpc1());
                    if (v.isZero()) {
                        nn->setTpc2(on->tpc1());
                    } else {
                        v.flip();
                        nn->setTpc2(Transpose::transposeTpc(nn->tpc1(), v, true));
                    }

                    if (on->tieFor()) {
                        Tie* tie = toTie(maybeLinkedClone(on->tieFor()));
                        tie->setScore(destScore);
                        nn->setTieFor(tie);
                        tie->setStartNote(nn);
                        tie->setTrack(nn->track());
                        tie->setEndNote(nn);
                        tieMap.add(on->tieFor(), tie);
                    }
                    if (on->tieBack()) {
                        Tie* tie = tieMap.findNew(on->tieBack());
                        if (tie) {
                            nn->setTieBack(tie);
                            tie->setEndNote(nn);
                        } else {
                            LOGD("cloneVoices: cannot find tie");
                        }
                    }
                    // add back spanners (going back from end to start spanner element
                    // makes sure the 'other' spanner anchor element is already set up)
                    // 'on' is the old spanner end note and 'nn' is the new spanner end note
                    for (Spanner* oldSp : on->spannerBack()) {
                        Note* newStart = Spanner::startElementFromSpanner(oldSp, nn);
                        if (newStart) {
                            Spanner* newSp = toSpanner(maybeLinkedClone(oldSp));
                            newSp->setNoteSpan(newStart, nn);
                            destScore->doUndoAddElement(newSp);
                        } else {
                            LOGD("cloneVoices: cannot find spanner start note");
                        }
                    }
                }
                // two note tremolo
                if (oldChord->tremoloTwoChord()) {
                    if (oldChord == oldChord->tremoloTwoChord()->chord1()) {
                        if (tremolo) {
                            LOGD("unconnected two note tremolo");
                        }
                        tremolo = item_cast<TremoloTwoChord*>(maybeLinkedClone(oldChord->tremoloTwoChord()));
                        tremolo->setScore(newChord->score());
                        tremolo->setParent(newChord);
                        tremolo->setTrack(newChord->track());
                        tremolo->setChords(newChord, nullptr);
                        newChord->setTremoloTwoChord(tremolo);
                    } else if (oldChord == oldChord->tremoloTwoChord()->chord2()) {
                        if (!tremolo) {
                            LOGD("first note for two note tremolo missing");
                        } else {
                            tremolo->setChords(tremolo->chord1(), newChord);
                            newChord->setTremoloTwoChord(tremolo);
                        }
                    } else {
                        LOGD("inconsistent two note tremolo");
                    }
                }
            };
            if (oe->isChord()) {
                cloneChord(toChord(ocr), toChord(ncr));
                for (size_t i = 0; i < toChord(ocr)->graceNotes().size(); ++i) {
                    Chord* ogc = toChord(ocr)->graceNotes().at(i);
                    Chord* ngc = toChord(ncr)->graceNotes().at(i);
                    cloneChord(ogc, ngc);
                }
            }

            // Add element
            if (link) {
                // To current staff only (exchange voice)
                if (!ns) {
                    ns = dm->undoGetSegment(oseg->segmentType(), oseg->tick());
                }
                ncr->setParent(ns);
                destScore->doUndoAddElement(ncr);
            } else {
                // To all linked staves (implode/explode)
                destScore->undoAddCR(ncr, dm, oseg->tick());
            }
        }

        Segment* tst = dm->segments().firstCRSegment();
        if (srcTrack % VOICES && !(dstTrack % VOICES) && (!tst || (!tst->element(dstTrack)))) {
            Rest* rest = Factory::createRest(destScore->dummy()->segment());
            rest->setTicks(dm->ticks());
            rest->setDurationType(DurationType::V_MEASURE);
            rest->setTrack(dstTrack);
            if (link) {
                Segment* segment = dm->undoGetSegment(SegmentType::ChordRest, dm->tick());
                rest->setParent(segment);
                destScore->doUndoAddElement(rest);
            } else {
                destScore->undoAddCR(rest, dm, dm->tick());
            }
        }

        const std::vector<EngravingItem*> annotations = oseg->annotations();
        for (EngravingItem* annotation : annotations) {
            if (!annotation->elementAppliesToTrack(srcTrack)) {
                continue;
            }

            EngravingItem* newAnnotation = maybeLinkedClone(annotation);
            newAnnotation->setTrack(dstTrack);

            if (!ns) {
                ns = dm->undoGetSegment(oseg->segmentType(), oseg->tick());
            }
            newAnnotation->setParent(ns);

            // Add element
            if (link) {
                // To current staff only (exchange voice)
                destScore->doUndoAddElement(newAnnotation);
            } else {
                // To all linked staves (implode/explode)
                destScore->undoAddElement(newAnnotation);
            }
        }
    }

    // Find and add corresponding slurs and hairpins
    static const std::set<ElementType> SPANNERS_TO_COPY { ElementType::SLUR, ElementType::HAMMER_ON_PULL_OFF, ElementType::HAIRPIN };
    auto spanners = sourceScore->spannerMap().findOverlapping(start.ticks(), lTick.ticks());
    for (auto i = spanners.begin(); i < spanners.end(); i++) {
        Spanner* sp      = i->value;
        Fraction spStart = sp->tick();
        Fraction spEnd = spStart + sp->ticks();

        if (muse::contains(SPANNERS_TO_COPY, sp->type()) && (spStart >= start && spEnd < lTick)) {
            if (!sp->elementAppliesToTrack(srcTrack)) {
                continue;
            }
            Spanner* ns = toSpanner(maybeLinkedClone(sp));

            ns->setScore(destScore);
            ns->setParent(0);
            ns->setTrack(dstTrack);
            ns->setTrack2(dstTrack);

            // set start/end element for slur
            ChordRest* cr1 = sp->startCR();
            ChordRest* cr2 = sp->endCR();

            ns->setStartElement(0);
            ns->setEndElement(0);
            if (cr1 && cr1->links()) {
                for (EngravingObject* e : *cr1->links()) {
                    ChordRest* cr = toChordRest(e);
                    if (cr == cr1) {
                        continue;
                    }
                    if ((cr->score() == destScore) && (cr->tick() == ns->tick()) && cr->track() == dstTrack) {
                        ns->setStartElement(cr);
                        break;
                    }
                }
            }
            if (cr2 && cr2->links()) {
                for (EngravingObject* e : *cr2->links()) {
                    ChordRest* cr = toChordRest(e);
                    if (cr == cr2) {
                        continue;
                    }
                    if ((cr->score() == destScore) && (cr->tick() == ns->tick2()) && cr->track() == dstTrack) {
                        ns->setEndElement(cr);
                        break;
                    }
                }
            }
            destScore->doUndoAddElement(ns);
        }
    }
}

void CloneVoice::cloneVoice(
    Segment* sourceSeg, // first source segment
    const Fraction& lTick, // last tick to clone
    Segment* destSeg, // first destination segment
    track_idx_t strack,
    track_idx_t dtrack,
    bool deleteSource,
    bool link // if true  add elements in destination segment only
              // if false add elements in every linked staff
    )
{
    Score* s = destSeg->score();
    Fraction ticks = destSeg->tick() + lTick - sourceSeg->tick();

    // Clear destination voice (in case of not linked and otrack = -1 we would delete our source
    for (Segment* seg = destSeg; seg && seg->tick() < ticks; seg = seg->next1()) {
        EngravingItem* el = seg->element(dtrack);
        if (el && el->isChordRest()) {
            link ? s->doUndoRemoveElement(el) : s->undoRemoveElement(el);
        }
    }

    doCloneVoice(s, strack, dtrack, sourceSeg, ticks, link);

    if (deleteSource) {
        auto spanners = s->spannerMap().findOverlapping(sourceSeg->tick().ticks(), lTick.ticks());
        for (auto i = spanners.begin(); i < spanners.end(); i++) {
            Spanner* sp = i->value;
            if (sp->isSlur() && (sp->track() == strack || sp->track2() == strack)) {
                link ? s->doUndoRemoveElement(sp) : s->undoRemoveElement(sp);
            }
        }
        for (Segment* seg = destSeg; seg && seg->tick() < ticks; seg = seg->next1()) {
            EngravingItem* el = seg->element(strack);
            if (el && el->isChordRest()) {
                link ? s->doUndoRemoveElement(el) : s->undoRemoveElement(el);
            }

            const std::vector<EngravingItem*> annotations = seg->annotations();
            for (EngravingItem* annotation : annotations) {
                if (annotation && annotation->track() == strack) {
                    if (annotation->hasVoiceAssignmentProperties()) {
                        VoiceAssignment voiceAssignment = annotation->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>();
                        if (voiceAssignment != VoiceAssignment::CURRENT_VOICE_ONLY) {
                            continue;
                        }
                    }
                    link ? s->doUndoRemoveElement(annotation) : s->undoRemoveElement(annotation);
                }
            }
        }
        // Set rests if first voice in a staff
        if (!(strack % VOICES)) {
            // TODO: take `link` into account?
            s->setRest(destSeg->tick(), strack, ticks, false, nullptr);
        }
    }
}
