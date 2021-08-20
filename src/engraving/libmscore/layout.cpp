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

#include <cmath>
#include <QtMath>

#include "style/style.h"

#include "accidental.h"
#include "barline.h"
#include "beam.h"
#include "box.h"
#include "chord.h"
#include "clef.h"
#include "element.h"
#include "fingering.h"
#include "glissando.h"
#include "harmony.h"
#include "key.h"
#include "keysig.h"
#include "layoutbreak.h"
#include "layout.h"
#include "lyrics.h"
#include "marker.h"
#include "measure.h"
#include "mmrestrange.h"
#include "mmrest.h"
#include "mscore.h"
#include "notedot.h"
#include "note.h"
#include "ottava.h"
#include "page.h"
#include "part.h"
#include "measurerepeat.h"
#include "score.h"
#include "scorefont.h"
#include "segment.h"
#include "sig.h"
#include "slur.h"
#include "staff.h"
#include "stem.h"
#include "stemslash.h"
#include "sticking.h"
#include "sym.h"
#include "system.h"
#include "text.h"
#include "tie.h"
#include "timesig.h"
#include "tremolo.h"
#include "tuplet.h"
#include "undo.h"
#include "utils.h"
#include "volta.h"
#include "breath.h"
#include "tempotext.h"
#include "systemdivider.h"
#include "hook.h"
#include "ambitus.h"
#include "hairpin.h"
#include "stafflines.h"
#include "articulation.h"
#include "bracket.h"
#include "spacer.h"
#include "fermata.h"
#include "measurenumber.h"

#include "masterscore.h"

using namespace mu;

namespace Ms {
// #define PAGE_DEBUG

#ifdef PAGE_DEBUG
#define PAGEDBG(...)  qDebug(__VA_ARGS__)
#else
#define PAGEDBG(...)  ;
#endif

//---------------------------------------------------------
//   almostZero
//---------------------------------------------------------

static bool inline almostZero(qreal value)
{
    // 1e-3 is close enough to zero to see it as zero.
    return value > -1e-3 && value < 1e-3;
}

//---------------------------------------------------------
//   relayoutForStyles
///   some styles can't properly apply if score hasn't been laid out yet,
///   so temporarily disable them and then reenable after layout
///   (called during score load)
//---------------------------------------------------------

void Score::relayoutForStyles()
{
    std::vector<Sid> stylesToTemporarilyDisable;

    for (Sid sid : { Sid::createMultiMeasureRests, Sid::mrNumberSeries }) {
        // only necessary if boolean style is true
        if (styleB(sid)) {
            stylesToTemporarilyDisable.push_back(sid);
        }
    }

    if (!stylesToTemporarilyDisable.empty()) {
        for (Sid sid : stylesToTemporarilyDisable) {
            style().set(sid, false); // temporarily disable
        }
        doLayout();
        for (Sid sid : stylesToTemporarilyDisable) {
            style().set(sid, true); // and immediately reenable
        }
    }
}

//---------------------------------------------------------
//   createMMRest
//    create a multimeasure rest
//    from firstMeasure to lastMeasure (inclusive)
//---------------------------------------------------------

void Score::createMMRest(Measure* firstMeasure, Measure* lastMeasure, const Fraction& len)
{
    int numMeasuresInMMRest = 1;
    if (firstMeasure != lastMeasure) {
        for (Measure* m = firstMeasure->nextMeasure(); m; m = m->nextMeasure()) {
            ++numMeasuresInMMRest;
            m->setMMRestCount(-1);
            if (m->mmRest()) {
                undo(new ChangeMMRest(m, 0));
            }
            if (m == lastMeasure) {
                break;
            }
        }
    }

    // mmrMeasure coexists with n undisplayed measures of rests
    Measure* mmrMeasure = firstMeasure->mmRest();
    if (mmrMeasure) {
        // reuse existing mmrest
        if (mmrMeasure->ticks() != len) {
            Segment* s = mmrMeasure->findSegmentR(SegmentType::EndBarLine, mmrMeasure->ticks());
            // adjust length
            mmrMeasure->setTicks(len);
            // move existing end barline
            if (s) {
                s->setRtick(len);
            }
        }
        mmrMeasure->removeSystemTrailer();
    } else {
        mmrMeasure = new Measure(this);
        mmrMeasure->setTicks(len);
        mmrMeasure->setTick(firstMeasure->tick());
        undo(new ChangeMMRest(firstMeasure, mmrMeasure));
    }
    mmrMeasure->setTimesig(firstMeasure->timesig());
    mmrMeasure->setPageBreak(lastMeasure->pageBreak());
    mmrMeasure->setLineBreak(lastMeasure->lineBreak());
    mmrMeasure->setMMRestCount(numMeasuresInMMRest);
    mmrMeasure->setNo(firstMeasure->no());

    //
    // set mmrMeasure with same barline as last underlying measure
    //
    Segment* lastMeasureEndBarlineSeg = lastMeasure->findSegmentR(SegmentType::EndBarLine, lastMeasure->ticks());
    if (lastMeasureEndBarlineSeg) {
        Segment* mmrEndBarlineSeg = mmrMeasure->undoGetSegmentR(SegmentType::EndBarLine, mmrMeasure->ticks());
        for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            Element* e = lastMeasureEndBarlineSeg->element(staffIdx * VOICES);
            if (e) {
                bool generated = e->generated();
                if (!mmrEndBarlineSeg->element(staffIdx * VOICES)) {
                    Element* eClone = generated ? e->clone() : e->linkedClone();
                    eClone->setGenerated(generated);
                    eClone->setParent(mmrEndBarlineSeg);
                    undoAddElement(eClone);
                } else {
                    BarLine* mmrEndBarline = toBarLine(mmrEndBarlineSeg->element(staffIdx * VOICES));
                    BarLine* lastMeasureEndBarline = toBarLine(e);
                    if (!generated && !mmrEndBarline->links()) {
                        undo(new Link(mmrEndBarline, lastMeasureEndBarline));
                    }
                    if (mmrEndBarline->barLineType() != lastMeasureEndBarline->barLineType()) {
                        // change directly when generating mmrests, do not change underlying measures or follow links
                        undo(new ChangeProperty(mmrEndBarline, Pid::BARLINE_TYPE,
                                                QVariant::fromValue(lastMeasureEndBarline->barLineType()),
                                                PropertyFlags::NOSTYLE));
                        undo(new ChangeProperty(mmrEndBarline, Pid::GENERATED, generated, PropertyFlags::NOSTYLE));
                    }
                }
            }
        }
    }

    //
    // if last underlying measure ends with clef change, show same at end of mmrest
    //
    Segment* lastMeasureClefSeg = lastMeasure->findSegmentR(SegmentType::Clef | SegmentType::HeaderClef,
                                                            lastMeasure->ticks());
    if (lastMeasureClefSeg) {
        Segment* mmrClefSeg = mmrMeasure->undoGetSegment(lastMeasureClefSeg->segmentType(), lastMeasure->endTick());
        for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            const int track = staff2track(staffIdx);
            Element* e = lastMeasureClefSeg->element(track);
            if (e && e->isClef()) {
                Clef* lastMeasureClef = toClef(e);
                if (!mmrClefSeg->element(track)) {
                    Clef* mmrClef = lastMeasureClef->generated() ? lastMeasureClef->clone() : toClef(
                        lastMeasureClef->linkedClone());
                    mmrClef->setParent(mmrClefSeg);
                    undoAddElement(mmrClef);
                } else {
                    Clef* mmrClef = toClef(mmrClefSeg->element(track));
                    mmrClef->setClefType(lastMeasureClef->clefType());
                    mmrClef->setShowCourtesy(lastMeasureClef->showCourtesy());
                }
            }
        }
    }

    mmrMeasure->setRepeatStart(firstMeasure->repeatStart() || lastMeasure->repeatStart());
    mmrMeasure->setRepeatEnd(firstMeasure->repeatEnd() || lastMeasure->repeatEnd());
    mmrMeasure->setSectionBreak(lastMeasure->sectionBreak());

    //
    // copy markers to mmrMeasure
    //
    ElementList oldList = mmrMeasure->takeElements();
    ElementList newList = lastMeasure->el();
    for (Element* e : firstMeasure->el()) {
        if (e->isMarker()) {
            newList.push_back(e);
        }
    }
    for (Element* e : newList) {
        bool found = false;
        for (Element* ee : oldList) {
            if (ee->type() == e->type() && ee->subtype() == e->subtype()) {
                mmrMeasure->add(ee);
                auto i = std::find(oldList.begin(), oldList.end(), ee);
                if (i != oldList.end()) {
                    oldList.erase(i);
                }
                found = true;
                break;
            }
        }
        if (!found) {
            mmrMeasure->add(e->clone());
        }
    }
    for (Element* e : oldList) {
        delete e;
    }
    Segment* s = mmrMeasure->undoGetSegmentR(SegmentType::ChordRest, Fraction(0, 1));
    for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
        int track = staffIdx * VOICES;
        if (s->element(track) == 0) {
            MMRest* mmr = new MMRest(this);
            mmr->setDurationType(TDuration::DurationType::V_MEASURE);
            mmr->setTicks(mmrMeasure->ticks());
            mmr->setTrack(track);
            mmr->setParent(s);
            undo(new AddElement(mmr));
        }
    }

    //
    // further check for clefs
    //
    Segment* underlyingSeg = lastMeasure->findSegmentR(SegmentType::Clef, lastMeasure->ticks());
    Segment* mmrSeg = mmrMeasure->findSegment(SegmentType::Clef, lastMeasure->endTick());
    if (underlyingSeg) {
        if (mmrSeg == 0) {
            mmrSeg = mmrMeasure->undoGetSegmentR(SegmentType::Clef, lastMeasure->ticks());
        }
        mmrSeg->setEnabled(underlyingSeg->enabled());
        mmrSeg->setTrailer(underlyingSeg->trailer());
        for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            int track = staffIdx * VOICES;
            Clef* clef = toClef(underlyingSeg->element(track));
            if (clef) {
                if (mmrSeg->element(track) == 0) {
                    mmrSeg->add(clef->clone());
                } else {
                    //TODO: check if same clef
                }
            }
        }
    } else if (mmrSeg) {
        // TODO: remove elements from mmrSeg?
        undo(new RemoveElement(mmrSeg));
    }

    //
    // check for time signature
    //
    underlyingSeg = firstMeasure->findSegmentR(SegmentType::TimeSig, Fraction(0, 1));
    mmrSeg = mmrMeasure->findSegment(SegmentType::TimeSig, firstMeasure->tick());
    if (underlyingSeg) {
        if (mmrSeg == 0) {
            mmrSeg = mmrMeasure->undoGetSegmentR(SegmentType::TimeSig, Fraction(0, 1));
        }
        mmrSeg->setEnabled(underlyingSeg->enabled());
        mmrSeg->setHeader(underlyingSeg->header());
        for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            int track = staffIdx * VOICES;
            TimeSig* underlyingTimeSig = toTimeSig(underlyingSeg->element(track));
            if (underlyingTimeSig) {
                TimeSig* mmrTimeSig = toTimeSig(mmrSeg->element(track));
                if (!mmrTimeSig) {
                    mmrTimeSig = underlyingTimeSig->generated() ? underlyingTimeSig->clone() : toTimeSig(
                        underlyingTimeSig->linkedClone());
                    mmrTimeSig->setParent(mmrSeg);
                    undo(new AddElement(mmrTimeSig));
                } else {
                    mmrTimeSig->setSig(underlyingTimeSig->sig(), underlyingTimeSig->timeSigType());
                    mmrTimeSig->layout();
                }
            }
        }
    } else if (mmrSeg) {
        // TODO: remove elements from mmrSeg?
        undo(new RemoveElement(mmrSeg));
    }

    //
    // check for ambitus
    //
    underlyingSeg = firstMeasure->findSegmentR(SegmentType::Ambitus, Fraction(0, 1));
    mmrSeg = mmrMeasure->findSegment(SegmentType::Ambitus, firstMeasure->tick());
    if (underlyingSeg) {
        if (mmrSeg == 0) {
            mmrSeg = mmrMeasure->undoGetSegmentR(SegmentType::Ambitus, Fraction(0, 1));
        }
        for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            int track = staffIdx * VOICES;
            Ambitus* underlyingAmbitus = toAmbitus(underlyingSeg->element(track));
            if (underlyingAmbitus) {
                Ambitus* mmrAmbitus = toAmbitus(mmrSeg->element(track));
                if (!mmrAmbitus) {
                    mmrAmbitus = underlyingAmbitus->clone();
                    mmrAmbitus->setParent(mmrSeg);
                    undo(new AddElement(mmrAmbitus));
                } else {
                    mmrAmbitus->initFrom(underlyingAmbitus);
                    mmrAmbitus->layout();
                }
            }
        }
    } else if (mmrSeg) {
        // TODO: remove elements from mmrSeg?
        undo(new RemoveElement(mmrSeg));
    }

    //
    // check for key signature
    //
    underlyingSeg = firstMeasure->findSegmentR(SegmentType::KeySig, Fraction(0, 1));
    mmrSeg = mmrMeasure->findSegmentR(SegmentType::KeySig, Fraction(0, 1));
    if (underlyingSeg) {
        if (mmrSeg == 0) {
            mmrSeg = mmrMeasure->undoGetSegmentR(SegmentType::KeySig, Fraction(0, 1));
        }
        mmrSeg->setEnabled(underlyingSeg->enabled());
        mmrSeg->setHeader(underlyingSeg->header());
        for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            int track = staffIdx * VOICES;
            KeySig* underlyingKeySig  = toKeySig(underlyingSeg->element(track));
            if (underlyingKeySig) {
                KeySig* mmrKeySig = toKeySig(mmrSeg->element(track));
                if (!mmrKeySig) {
                    mmrKeySig = underlyingKeySig->generated() ? underlyingKeySig->clone() : toKeySig(
                        underlyingKeySig->linkedClone());
                    mmrKeySig->setParent(mmrSeg);
                    mmrKeySig->setGenerated(true);
                    undo(new AddElement(mmrKeySig));
                } else {
                    if (!(mmrKeySig->keySigEvent() == underlyingKeySig->keySigEvent())) {
                        bool addKey = underlyingKeySig->isChange();
                        undo(new ChangeKeySig(mmrKeySig, underlyingKeySig->keySigEvent(), mmrKeySig->showCourtesy(),
                                              addKey));
                    }
                }
            }
        }
    } else if (mmrSeg) {
        mmrSeg->setEnabled(false);
        // TODO: remove elements from mmrSeg, then delete mmrSeg
        // previously we removed the segment if not empty,
        // but this resulted in "stale" keysig in mmrest after removed from underlying measure
        //undo(new RemoveElement(mmrSeg));
    }

    mmrMeasure->checkHeader();
    mmrMeasure->checkTrailer();

    //
    // check for rehearsal mark etc.
    //
    underlyingSeg = firstMeasure->findSegmentR(SegmentType::ChordRest, Fraction(0, 1));
    if (underlyingSeg) {
        // clone elements from underlying measure to mmr
        for (Element* e : underlyingSeg->annotations()) {
            // look at elements in underlying measure
            if (!(e->isRehearsalMark() || e->isTempoText() || e->isHarmony() || e->isStaffText() || e->isSystemText()
                  || e->isInstrumentChange())) {
                continue;
            }
            // try to find a match in mmr
            bool found = false;
            for (Element* ee : s->annotations()) {
                if (e->linkList().contains(ee)) {
                    found = true;
                    break;
                }
            }
            // add to mmr if no match found
            if (!found) {
                Element* eClone = e->linkedClone();
                eClone->setParent(s);
                undo(new AddElement(eClone));
            }
        }

        // remove stray elements (possibly leftover from a previous layout of this mmr)
        // this should not happen since the elements are linked?
        for (Element* e : s->annotations()) {
            // look at elements in mmr
            if (!(e->isRehearsalMark() || e->isTempoText() || e->isHarmony() || e->isStaffText() || e->isSystemText()
                  || e->isInstrumentChange())) {
                continue;
            }
            // try to find a match in underlying measure
            bool found = false;
            for (Element* ee : underlyingSeg->annotations()) {
                if (e->linkList().contains(ee)) {
                    found = true;
                    break;
                }
            }
            // remove from mmr if no match found
            if (!found) {
                undo(new RemoveElement(e));
            }
        }
    }

    MeasureBase* nm = _showVBox ? lastMeasure->next() : lastMeasure->nextMeasure();
    mmrMeasure->setNext(nm);
    mmrMeasure->setPrev(firstMeasure->prev());
}

//---------------------------------------------------------
//   doLayout
//    do a complete (re-) layout
//---------------------------------------------------------

void Score::doLayout()
{
    doLayoutRange(Fraction(0, 1), Fraction(-1, 1));
}

void Score::doLayoutRange(const Fraction& st, const Fraction& et)
{
    m_layout.doLayoutRange(st, et);
}
}
