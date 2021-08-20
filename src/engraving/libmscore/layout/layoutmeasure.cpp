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
#include "layoutmeasure.h"

#include "../score.h"
#include "../measure.h"

#include "layout.h"
#include "layoutcontext.h"
#include "layoutbeams.h"

using namespace mu::engraving;
using namespace Ms;

void LayoutMeasure::getNextMeasure(Ms::Score* score, LayoutContext& lc)
{
    lc.prevMeasure = lc.curMeasure;
    lc.curMeasure  = lc.nextMeasure;
    if (!lc.curMeasure) {
        lc.nextMeasure = score->_showVBox ? score->first() : score->firstMeasure();
    } else {
        lc.nextMeasure = score->_showVBox ? lc.curMeasure->next() : lc.curMeasure->nextMeasure();
    }
    if (!lc.curMeasure) {
        return;
    }

    int mno = lc.adjustMeasureNo(lc.curMeasure);

    if (lc.curMeasure->isMeasure()) {
        if (score->score()->styleB(Sid::createMultiMeasureRests)) {
            Measure* m = toMeasure(lc.curMeasure);
            Measure* nm = m;
            Measure* lm = nm;
            int n       = 0;
            Fraction len;

            while (validMMRestMeasure(nm)) {
                MeasureBase* mb = score->_showVBox ? nm->next() : nm->nextMeasure();
                if (breakMultiMeasureRest(nm) && n) {
                    break;
                }
                if (nm != m) {
                    lc.adjustMeasureNo(nm);
                }
                ++n;
                len += nm->ticks();
                lm = nm;
                if (!(mb && mb->isMeasure())) {
                    break;
                }
                nm = toMeasure(mb);
            }
            if (n >= score->styleI(Sid::minEmptyMeasures)) {
                score->createMMRest(m, lm, len);
                lc.curMeasure  = m->mmRest();
                lc.nextMeasure = score->_showVBox ? lm->next() : lm->nextMeasure();
            } else {
                if (m->mmRest()) {
                    score->undo(new ChangeMMRest(m, 0));
                }
                m->setMMRestCount(0);
                lc.measureNo = mno;
            }
        } else if (toMeasure(lc.curMeasure)->isMMRest()) {
            qDebug("mmrest: no %d += %d", lc.measureNo, toMeasure(lc.curMeasure)->mmRestCount());
            lc.measureNo += toMeasure(lc.curMeasure)->mmRestCount() - 1;
        }
    }
    if (!lc.curMeasure->isMeasure()) {
        lc.curMeasure->setTick(lc.tick);
        return;
    }

    //-----------------------------------------
    //    process one measure
    //-----------------------------------------

    Measure* measure = toMeasure(lc.curMeasure);
    measure->moveTicks(lc.tick - measure->tick());

    if (score->lineMode() && (measure->tick() < lc.startTick || measure->tick() > lc.endTick)) {
        // needed to reset segment widths if they can change after measure width is computed
        //for (Segment& s : measure->segments())
        //      s.createShapes();
        lc.tick += measure->ticks();
        return;
    }

    measure->connectTremolo();

    //
    // calculate accidentals and note lines,
    // create stem and set stem direction
    //
    for (int staffIdx = 0; staffIdx < score->score()->nstaves(); ++staffIdx) {
        const Staff* staff     = score->Score::staff(staffIdx);
        const Drumset* drumset
            = staff->part()->instrument(measure->tick())->useDrumset() ? staff->part()->instrument(measure->tick())->drumset() : 0;
        AccidentalState as;          // list of already set accidentals for this measure
        as.init(staff->keySigEvent(measure->tick()), staff->clef(measure->tick()));

        for (Segment& segment : measure->segments()) {
            // TODO? maybe we do need to process it here to make it possible to enable later
            //if (!segment.enabled())
            //      continue;
            if (segment.isKeySigType()) {
                KeySig* ks = toKeySig(segment.element(staffIdx * VOICES));
                if (!ks) {
                    continue;
                }
                Fraction tick = segment.tick();
                as.init(staff->keySigEvent(tick), staff->clef(tick));
                ks->layout();
            } else if (segment.isChordRestType()) {
                const StaffType* st = staff->staffTypeForElement(&segment);
                int track     = staffIdx * VOICES;
                int endTrack  = track + VOICES;

                for (int t = track; t < endTrack; ++t) {
                    ChordRest* cr = segment.cr(t);
                    if (!cr) {
                        continue;
                    }
                    qreal m = staff->staffMag(&segment);
                    if (cr->isSmall()) {
                        m *= score->score()->styleD(Sid::smallNoteMag);
                    }

                    if (cr->isChord()) {
                        Chord* chord = toChord(cr);
                        chord->cmdUpdateNotes(&as);
                        for (Chord* c : chord->graceNotes()) {
                            c->setMag(m * score->score()->styleD(Sid::graceNoteMag));
                            c->computeUp();
                            if (c->stemDirection() != Direction::AUTO) {
                                c->setUp(c->stemDirection() == Direction::UP);
                            } else {
                                c->setUp(!(t % 2));
                            }
                            if (drumset) {
                                layoutDrumsetChord(c, drumset, st, score->spatium());
                            }
                            c->layoutStem1();
                        }
                        if (drumset) {
                            layoutDrumsetChord(chord, drumset, st, score->spatium());
                        }
                        chord->computeUp();
                        chord->layoutStem1();               // create stems needed to calculate spacing
                                                            // stem direction can change later during beam processing

                        // if there is a two-note tremolo attached, and it is too steep,
                        // extend stem of one of the chords (if not cross-staff)
                        // or extend both stems (if cross-staff)
                        // this should be done after the stem lengths of two notes are both calculated
                        if (chord->tremolo() && chord == chord->tremolo()->chord2()) {
                            Stem* stem1 = chord->tremolo()->chord1()->stem();
                            Stem* stem2 = chord->tremolo()->chord2()->stem();
                            if (stem1 && stem2) {
                                std::pair<qreal, qreal> extendedLen = Layout::extendedStemLenWithTwoNoteTremolo(
                                    chord->tremolo(),
                                    stem1->p2().y(),
                                    stem2->p2().y());
                                stem1->setLen(extendedLen.first);
                                stem2->setLen(extendedLen.second);
                            }
                        }
                    }
                    cr->setMag(m);
                }
            } else if (segment.isClefType()) {
                Element* e = segment.element(staffIdx * VOICES);
                if (e) {
                    toClef(e)->setSmall(true);
                    e->layout();
                }
            } else if (segment.isType(SegmentType::TimeSig | SegmentType::Ambitus | SegmentType::HeaderClef)) {
                Element* e = segment.element(staffIdx * VOICES);
                if (e) {
                    e->layout();
                }
            }
        }
    }

    LayoutBeams::createBeams(score, lc, measure);

    for (int staffIdx = 0; staffIdx < score->score()->nstaves(); ++staffIdx) {
        for (Segment& segment : measure->segments()) {
            if (segment.isChordRestType()) {
                score->layoutChords1(&segment, staffIdx);
                for (int voice = 0; voice < VOICES; ++voice) {
                    ChordRest* cr = segment.cr(staffIdx * VOICES + voice);
                    if (cr) {
                        for (Lyrics* l : cr->lyrics()) {
                            if (l) {
                                l->layout();
                            }
                        }
                    }
                }
            }
        }
    }

    measure->computeTicks();

    for (Segment& segment : measure->segments()) {
        if (segment.isBreathType()) {
            for (Element* e : segment.elist()) {
                if (e && e->isBreath()) {
                    e->layout();
                }
            }
        } else if (segment.isChordRestType()) {
            for (Element* e : segment.annotations()) {
                if (e->isSymbol()) {
                    e->layout();
                }
            }
        }
    }

    score->rebuildTempoAndTimeSigMaps(measure);

    Segment* seg = measure->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
    if (measure->repeatStart()) {
        if (!seg) {
            seg = measure->getSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
        }
        measure->barLinesSetSpan(seg);          // this also creates necessary barlines
        for (int staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            BarLine* b = toBarLine(seg->element(staffIdx * VOICES));
            if (b) {
                b->setBarLineType(BarLineType::START_REPEAT);
                b->layout();
            }
        }
    } else if (seg) {
        score->score()->undoRemoveElement(seg);
    }

    for (Segment& s : measure->segments()) {
        // TODO? maybe we do need to process it here to make it possible to enable later
        //if (!s.enabled())
        //      continue;
        // DEBUG: relayout grace notes as beaming/flags may have changed
        if (s.isChordRestType()) {
            for (Element* e : s.elist()) {
                if (e && e->isChord()) {
                    Chord* chord = toChord(e);
                    chord->layout();
//                              if (chord->tremolo())            // debug
//                                    chord->tremolo()->layout();
                }
            }
        } else if (s.isEndBarLineType()) {
            continue;
        }
        s.createShapes();
    }

    lc.tick += measure->ticks();
}
