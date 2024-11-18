/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#include <cfloat>

#include "measurelayout.h"

#include "infrastructure/rtti.h"

#include "dom/ambitus.h"
#include "dom/barline.h"
#include "dom/beam.h"
#include "dom/factory.h"
#include "dom/keysig.h"
#include "dom/layoutbreak.h"
#include "dom/lyrics.h"
#include "dom/marker.h"
#include "dom/measure.h"
#include "dom/measurenumber.h"
#include "dom/measurerepeat.h"
#include "dom/mmrest.h"
#include "dom/mmrestrange.h"
#include "dom/ornament.h"
#include "dom/part.h"
#include "dom/spacer.h"
#include "dom/score.h"
#include "dom/stafflines.h"
#include "dom/system.h"
#include "dom/tie.h"
#include "dom/timesig.h"
#include "dom/tremolosinglechord.h"
#include "dom/tremolotwochord.h"
#include "dom/trill.h"
#include "dom/undo.h"
#include "dom/utils.h"

#include "tlayout.h"
#include "layoutcontext.h"
#include "arpeggiolayout.h"
#include "beamlayout.h"
#include "chordlayout.h"
#include "slurtielayout.h"
#include "horizontalspacing.h"
#include "tremololayout.h"
#include "segmentlayout.h"
#include "modifydom.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::rtti;
using namespace mu::engraving::rendering::score;

//---------------------------------------------------------
//   layout2
//    called after layout of page
//---------------------------------------------------------

void MeasureLayout::layout2(Measure* item, LayoutContext& ctx)
{
    assert(item->explicitParent());
    assert(ctx.dom().nstaves() == item->mstaves().size());

    double _spatium = item->spatium();

    for (size_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
        const MStaff* ms = item->mstaves().at(staffIdx);
        Spacer* sp = ms->vspacerDown();
        if (sp) {
            TLayout::layoutSpacer(sp, ctx);
            const Staff* staff = ctx.dom().staff(staffIdx);
            int n = staff->lines(item->tick()) - 1;
            double y = item->system()->staff(staffIdx)->y();
            sp->setPos(_spatium * .5, y + n * _spatium * staff->staffMag(item->tick()));
        }
        sp = ms->vspacerUp();
        if (sp) {
            TLayout::layoutSpacer(sp, ctx);
            double y = item->system()->staff(staffIdx)->y();
            sp->setPos(_spatium * .5, y - sp->gap());
        }
    }

    // layout LAYOUT_BREAK elements
    TLayout::layoutBaseMeasureBase(item, item->mutldata(), ctx);

    //---------------------------------------------------
    //    layout cross-staff ties
    //---------------------------------------------------

    const Fraction stick = item->system()->measures().front()->tick();
    const size_t tracks = ctx.dom().ntracks();
    static const SegmentType st { SegmentType::ChordRest };
    for (track_idx_t track = 0; track < tracks; ++track) {
        if (!ctx.dom().staff(track / VOICES)->show()) {
            track += VOICES - 1;
            continue;
        }
        for (Segment* seg = item->first(st); seg; seg = seg->next(st)) {
            EngravingItem* element = seg->elementAt(track);
            if (!element || !element->isChord()) {
                continue;
            }
            Chord* chord = toChord(element);
            for (Note* note : chord->notes()) {
                Tie* tieFor = note->tieFor();
                Tie* tieBack = note->tieBack();
                LaissezVib* lv = note->laissezVib();
                if (lv && lv->isCrossStaff()) {
                    SlurTieLayout::layoutLaissezVibChord(chord, ctx);
                }
                if (tieFor && !lv && tieFor->isCrossStaff()) {
                    SlurTieLayout::tieLayoutFor(tieFor, item->system());
                }
                if (tieBack && tieBack->tick() < stick && tieBack->isCrossStaff()) {
                    SlurTieLayout::tieLayoutBack(tieBack, item->system(), ctx);
                }
            }
        }
    }
}

static const std::unordered_set<ElementType> BREAK_TYPES {
    ElementType::TEMPO_TEXT,
    ElementType::REHEARSAL_MARK,
    ElementType::HARMONY,
    ElementType::STAFF_TEXT,
    ElementType::EXPRESSION,
    ElementType::SYSTEM_TEXT,
    ElementType::TRIPLET_FEEL,
    ElementType::PLAYTECH_ANNOTATION,
    ElementType::CAPO,
    ElementType::INSTRUMENT_CHANGE,
    ElementType::STRING_TUNINGS,
    ElementType::SYMBOL,
    ElementType::FRET_DIAGRAM,
    ElementType::HARP_DIAGRAM,
};

static const std::unordered_set<ElementType> ALWAYS_BREAK_TYPES {
    ElementType::TEMPO_TEXT,
    ElementType::REHEARSAL_MARK
};

static const std::unordered_set<ElementType> CONDITIONAL_BREAK_TYPES {
    ElementType::HARMONY,
    ElementType::STAFF_TEXT,
    ElementType::EXPRESSION,
    ElementType::SYSTEM_TEXT,
    ElementType::TRIPLET_FEEL,
    ElementType::PLAYTECH_ANNOTATION,
    ElementType::CAPO,
    ElementType::INSTRUMENT_CHANGE,
    ElementType::STRING_TUNINGS,
    ElementType::SYMBOL,
    ElementType::FRET_DIAGRAM,
    ElementType::HARP_DIAGRAM,
};

//---------------------------------------------------------
//   createMMRest
//    create a multimeasure rest
//    from firstMeasure to lastMeasure (inclusive)
//---------------------------------------------------------

void MeasureLayout::createMMRest(LayoutContext& ctx, Measure* firstMeasure, Measure* lastMeasure, const Fraction& len)
{
    int numMeasuresInMMRest = 1;
    if (firstMeasure != lastMeasure) {
        for (Measure* m = firstMeasure->nextMeasure(); m; m = m->nextMeasure()) {
            ++numMeasuresInMMRest;
            m->setMMRestCount(-1);
            if (m->mmRest()) {
                ctx.mutDom().undo(new ChangeMMRest(m, 0));
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
            Segment* bls = mmrMeasure->findSegmentR(SegmentType::EndBarLine, mmrMeasure->ticks());
            Segment* cs = mmrMeasure->findSegment(SegmentType::Clef | SegmentType::HeaderClef, mmrMeasure->ticks());
            // adjust length
            mmrMeasure->setTicks(len);
            // move existing end barline and clef
            if (bls) {
                bls->setRtick(len);
            }
            if (cs) {
                cs->setRtick(len);
            }
        }
        MeasureLayout::removeSystemTrailer(mmrMeasure);
    } else {
        mmrMeasure = Factory::createMeasure(ctx.mutDom().dummyParent()->system());
        mmrMeasure->setTicks(len);
        mmrMeasure->setTick(firstMeasure->tick());
        ctx.mutDom().undo(new ChangeMMRest(firstMeasure, mmrMeasure));
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
        for (size_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
            EngravingItem* e = lastMeasureEndBarlineSeg->element(staffIdx * VOICES);
            if (e) {
                bool generated = e->generated();
                if (!mmrEndBarlineSeg->element(staffIdx * VOICES)) {
                    EngravingItem* eClone = generated ? e->clone() : e->linkedClone();
                    eClone->setGenerated(generated);
                    eClone->setParent(mmrEndBarlineSeg);
                    ctx.mutDom().undoAddElement(eClone);// ???
                } else {
                    BarLine* mmrEndBarline = toBarLine(mmrEndBarlineSeg->element(staffIdx * VOICES));
                    BarLine* lastMeasureEndBarline = toBarLine(e);
                    if (!generated && !mmrEndBarline->links()) {
                        ctx.mutDom().undo(new Link(mmrEndBarline, lastMeasureEndBarline));
                    }
                    if (mmrEndBarline->barLineType() != lastMeasureEndBarline->barLineType()) {
                        // change directly when generating mmrests, do not change underlying measures or follow links
                        ctx.mutDom().undo(new ChangeProperty(mmrEndBarline, Pid::BARLINE_TYPE,
                                                             PropertyValue::fromValue(lastMeasureEndBarline->barLineType()),
                                                             PropertyFlags::NOSTYLE));
                        ctx.mutDom().undo(new ChangeProperty(mmrEndBarline, Pid::GENERATED, generated, PropertyFlags::NOSTYLE));
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
        for (size_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
            const track_idx_t track = staff2track(staffIdx);
            EngravingItem* e = lastMeasureClefSeg->element(track);
            if (e && e->isClef()) {
                Clef* lastMeasureClef = toClef(e);
                if (!mmrClefSeg->element(track)) {
                    Clef* mmrClef = lastMeasureClef->generated() ? lastMeasureClef->clone() : toClef(
                        lastMeasureClef->linkedClone());
                    mmrClef->setParent(mmrClefSeg);
                    ctx.mutDom().undoAddElement(mmrClef);
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
    for (EngravingItem* e : firstMeasure->el()) {
        if (e->isMarker() && firstMeasure != lastMeasure) {
            newList.push_back(e);
        }
    }
    for (EngravingItem* e : newList) {
        bool found = false;
        for (EngravingItem* ee : oldList) {
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
    for (EngravingItem* e : oldList) {
        delete e;
    }
    Segment* s = mmrMeasure->undoGetSegmentR(SegmentType::ChordRest, Fraction(0, 1));
    for (size_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
        track_idx_t track = staffIdx * VOICES;
        if (s->element(track) == 0) {
            MMRest* mmr = Factory::createMMRest(s);
            mmr->setDurationType(DurationType::V_MEASURE);
            mmr->setTicks(mmrMeasure->ticks());
            mmr->setTrack(track);
            mmr->setParent(s);
            ctx.mutDom().doUndoAddElement(mmr);
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
        for (size_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
            track_idx_t track = staffIdx * VOICES;
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
        ctx.mutDom().doUndoRemoveElement(mmrSeg);
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
        for (staff_idx_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
            track_idx_t track = staffIdx * VOICES;
            TimeSig* underlyingTimeSig = toTimeSig(underlyingSeg->element(track));
            if (underlyingTimeSig) {
                TimeSig* mmrTimeSig = toTimeSig(mmrSeg->element(track));
                if (!mmrTimeSig) {
                    mmrTimeSig = underlyingTimeSig->generated() ? underlyingTimeSig->clone() : toTimeSig(
                        underlyingTimeSig->linkedClone());
                    mmrTimeSig->setParent(mmrSeg);
                    ctx.mutDom().doUndoAddElement(mmrTimeSig);
                } else {
                    TLayout::layoutTimeSig(mmrTimeSig, mmrTimeSig->mutldata(), ctx);
                }
            }
        }
    } else if (mmrSeg) {
        // TODO: remove elements from mmrSeg?
        ctx.mutDom().doUndoRemoveElement(mmrSeg);
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
        for (size_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
            track_idx_t track = staffIdx * VOICES;
            Ambitus* underlyingAmbitus = toAmbitus(underlyingSeg->element(track));
            if (underlyingAmbitus) {
                Ambitus* mmrAmbitus = toAmbitus(mmrSeg->element(track));
                if (!mmrAmbitus) {
                    mmrAmbitus = underlyingAmbitus->clone();
                    mmrAmbitus->setParent(mmrSeg);
                    ctx.mutDom().doUndoAddElement(mmrAmbitus);
                } else {
                    mmrAmbitus->initFrom(underlyingAmbitus);
                    TLayout::layoutAmbitus(mmrAmbitus, mmrAmbitus->mutldata(), ctx);
                }
            }
        }
    } else if (mmrSeg) {
        // TODO: remove elements from mmrSeg?
        ctx.mutDom().doUndoRemoveElement(mmrSeg);
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
        for (size_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
            track_idx_t track = staffIdx * VOICES;
            KeySig* underlyingKeySig  = toKeySig(underlyingSeg->element(track));
            if (underlyingKeySig) {
                KeySig* mmrKeySig = toKeySig(mmrSeg->element(track));
                if (!mmrKeySig) {
                    mmrKeySig = underlyingKeySig->generated() ? underlyingKeySig->clone() : toKeySig(
                        underlyingKeySig->linkedClone());
                    mmrKeySig->setParent(mmrSeg);
                    mmrKeySig->setGenerated(true);
                    ctx.mutDom().doUndoAddElement(mmrKeySig);
                } else {
                    if (!(mmrKeySig->keySigEvent() == underlyingKeySig->keySigEvent())) {
                        bool addKey = underlyingKeySig->isChange();
                        ctx.mutDom().undo(new ChangeKeySig(mmrKeySig, underlyingKeySig->keySigEvent(), mmrKeySig->showCourtesy(),
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
        //doUndoRemoveElement(mmrSeg);
    }

    mmrMeasure->checkHeader();
    mmrMeasure->checkTrailer();

    //
    // check for rehearsal mark etc.
    //
    underlyingSeg = firstMeasure->findSegmentR(SegmentType::ChordRest, Fraction(0, 1));
    if (underlyingSeg) {
        // clone elements from underlying measure to mmr
        for (EngravingItem* e : underlyingSeg->annotations()) {
            // look at elements in underlying measure
            if (!muse::contains(BREAK_TYPES, e->type()) || !e->visible()) {
                continue;
            }
            // try to find a match in mmr
            bool found = false;
            for (EngravingItem* ee : s->annotations()) {
                if (muse::contains(e->linkList(), static_cast<EngravingObject*>(ee))) {
                    found = true;
                    break;
                }
            }
            // add to mmr if no match found
            if (!found) {
                EngravingItem* eClone = e->linkedClone();
                eClone->setParent(s);
                ctx.mutDom().doUndoAddElement(eClone);
            }
        }

        // remove stray elements (possibly leftover from a previous layout of this mmr)
        // this should not happen since the elements are linked?
        const auto annotations = s->annotations(); // make a copy since we alter the list
        for (EngravingItem* e : annotations) { // look at elements in mmr
            if (!muse::contains(BREAK_TYPES, e->type())) {
                continue;
            }
            // try to find a match in underlying measure
            bool found = false;
            for (EngravingItem* ee : underlyingSeg->annotations()) {
                if (muse::contains(e->linkList(), static_cast<EngravingObject*>(ee))) {
                    found = true;
                    break;
                }
            }
            // remove from mmr if no match found
            if (!found) {
                ctx.mutDom().doUndoRemoveElement(e);
            }
        }
    }

    MeasureBase* nm = ctx.conf().isShowVBox() ? lastMeasure->next() : lastMeasure->nextMeasure();
    mmrMeasure->setNext(nm);
    mmrMeasure->setPrev(firstMeasure->prev());
}

//---------------------------------------------------------
// validMMRestMeasure
//    return true if this might be a measure in a
//    multi measure rest
//---------------------------------------------------------

static bool validMMRestMeasure(const LayoutContext& ctx, const Measure* m)
{
    if (m->irregular()) {
        return false;
    }

    size_t nstaves = ctx.dom().nstaves();
    for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        if (m->isMeasureRepeatGroup(staffIdx)) {
            return false;
        }
    }

    int n = 0;
    for (const Segment* s = m->first(); s; s = s->next()) {
        for (const EngravingItem* e : s->annotations()) {
            if (!e->staff()->show() || !e->visible()) {
                continue;
            }
            if (!muse::contains(BREAK_TYPES, e->type())) {
                return false;
            }
        }
        if (s->isChordRestType()) {
            bool restFound = false;
            size_t tracks = ctx.dom().ntracks();
            for (track_idx_t track = 0; track < tracks; ++track) {
                if ((track % VOICES) == 0 && !ctx.dom().staff(track / VOICES)->show()) {
                    track += VOICES - 1;
                    continue;
                }
                if (s->element(track)) {
                    if (!s->element(track)->isRest()) {
                        return false;
                    }
                    restFound = true;
                }
            }
            for (const EngravingItem* e : s->annotations()) {
                if (e->isFermata()) {
                    return false;
                }
            }
            if (restFound) {
                ++n;
            }
            // measure is not empty if there is more than one rest
            if (n > 1) {
                return false;
            }
        }
    }
    return true;
}

//---------------------------------------------------------
//  breakMultiMeasureRest
//    return true if this measure should start a new
//    multi measure rest
//---------------------------------------------------------

static bool breakMultiMeasureRest(const LayoutContext& ctx, Measure* m)
{
    if (m->breakMultiMeasureRest()) {
        return true;
    }

    if (m->repeatStart()
        || (m->prevMeasure() && m->prevMeasure()->repeatEnd())
        || (m->isIrregular())
        || (m->prevMeasure() && m->prevMeasure()->isIrregular())
        || (m->prevMeasure() && (m->prevMeasure()->sectionBreak()))) {
        return true;
    }

    static const std::set<ElementType> breakSpannerTypes {
        ElementType::VOLTA,
        ElementType::GRADUAL_TEMPO_CHANGE,
        ElementType::TEXTLINE,
    };
    // Break for spanners/textLines in this measure
    auto sl = ctx.dom().spannerMap().findOverlapping(m->tick().ticks(), m->endTick().ticks());
    for (auto i : sl) {
        Spanner* s = i.value;
        if (!s->visible()) {
            continue;
        }
        Fraction spannerStart = s->tick();
        Fraction spannerEnd = s->tick2();
        Fraction measureStart = m->tick();
        Fraction measureEnd = m->endTick();
        bool spannerStartsInside = spannerStart >= measureStart && spannerStart < measureEnd;
        bool spannerEndsInside = spannerEnd >= measureStart && spannerEnd < measureEnd;
        if (muse::contains(breakSpannerTypes, s->type()) && (spannerStartsInside || spannerEndsInside)) {
            return true;
        }
    }
    // Break for spanners/textLines starting or ending mid-way inside the *previous* measure
    Measure* prevMeas = m->prevMeasure();
    if (prevMeas) {
        auto prevMeasSpanners = ctx.dom().spannerMap().findOverlapping(prevMeas->tick().ticks(), prevMeas->endTick().ticks());
        for (auto i : prevMeasSpanners) {
            Spanner* s = i.value;
            if (!s->visible()) {
                continue;
            }
            Fraction spannerStart = s->tick();
            Fraction spannerEnd = s->tick2();
            Fraction measureStart = prevMeas->tick();
            Fraction measureEnd = prevMeas->endTick();
            bool spannerStartsInside = spannerStart > measureStart && spannerStart < measureEnd;
            bool spannerEndsInside = spannerEnd > measureStart && spannerEnd < measureEnd;
            if (muse::contains(breakSpannerTypes, s->type()) && (spannerStartsInside || spannerEndsInside)) {
                return true;
            }
        }
    }

    // break for marker in this measure
    for (EngravingItem* e : m->el()) {
        if (e->isMarker()) {
            Marker* mark = toMarker(e);
            if (!(mark->align() == AlignH::RIGHT)) {
                return true;
            }
        }
    }

    // break for marker & jump in previous measure
    Measure* pm = m->prevMeasure();
    if (pm) {
        for (EngravingItem* e : pm->el()) {
            if (e->isJump()) {
                return true;
            } else if (e->isMarker()) {
                Marker* mark = toMarker(e);
                if (mark->align() == AlignH::RIGHT) {
                    return true;
                }
            }
        }
    }

    auto breakForAnnotation = [&](EngravingItem* e) {
        if (muse::contains(ALWAYS_BREAK_TYPES, e->type())) {
            return true;
        }
        bool breakForElement = e->systemFlag() || e->staff()->show();
        if (muse::contains(CONDITIONAL_BREAK_TYPES, e->type()) && breakForElement) {
            return true;
        }
        return false;
    };

    // Break for annotations found mid-way into the previous measure
    if (prevMeas) {
        for (Segment* s = prevMeas->first(); s; s = s->next()) {
            for (EngravingItem* e : s->annotations()) {
                if (!e->visible()) {
                    continue;
                }
                bool isInMidMeasure = e->rtick() > Fraction(0, 1);
                if (!isInMidMeasure) {
                    continue;
                }
                if (breakForAnnotation(e)) {
                    return true;
                }
            }
        }
    }

    for (Segment* s = m->first(); s; s = s->next()) {
        // Break for annotations in this measure
        for (EngravingItem* e : s->annotations()) {
            if (!e->visible()) {
                continue;
            }
            if (breakForAnnotation(e)) {
                return true;
            }
        }
        for (size_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
            if (!ctx.dom().staff(staffIdx)->show()) {
                continue;
            }
            EngravingItem* e = s->element(staffIdx * VOICES);
            if (!e || e->generated()) {
                continue;
            }
            if (s->isStartRepeatBarLineType()) {
                return true;
            }
            if (s->isType(SegmentType::KeySig | SegmentType::TimeSig) && m->tick().isNotZero()) {
                return true;
            }
            if (s->isClefType()) {
                if (s->tick() != m->endTick() && m->tick().isNotZero()) {
                    return true;
                }
            }
        }
    }
    if (pm) {
        Segment* s = pm->findSegmentR(SegmentType::EndBarLine, pm->ticks());
        if (s) {
            for (size_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
                BarLine* bl = toBarLine(s->element(staffIdx * VOICES));
                if (bl) {
                    BarLineType t = bl->barLineType();
                    if (t != BarLineType::NORMAL && t != BarLineType::BROKEN && t != BarLineType::DOTTED && !bl->generated()) {
                        return true;
                    } else {
                        break;
                    }
                }
            }
        }
        if (pm->findSegment(SegmentType::Clef, m->tick())) {
            return true;
        }
    }
    return false;
}

void MeasureLayout::moveToNextMeasure(LayoutContext& ctx)
{
    LAYOUT_CALL();
    LayoutState& state = ctx.mutState();

    state.setPrevMeasure(state.curMeasure());
    state.setCurMeasure(state.nextMeasure());
    if (!state.curMeasure()) {
        state.setNextMeasure(ctx.conf().isShowVBox() ? ctx.mutDom().first() : ctx.mutDom().firstMeasure());
    } else {
        MeasureBase* m = ctx.conf().isShowVBox() ? state.curMeasure()->next() : state.curMeasure()->nextMeasure();
        state.setNextMeasure(m);
    }
}

void MeasureLayout::createMultiMeasureRestsIfNeed(MeasureBase* currentMB, LayoutContext& ctx)
{
    LAYOUT_CALL() << LAYOUT_ITEM_INFO(currentMB);

    if (!currentMB->isMeasure()) {
        return;
    }

    int mno = ctx.state().measureNo();
    Measure* firstMeasure = toMeasure(currentMB);

    if (ctx.conf().styleB(Sid::createMultiMeasureRests)) {
        Measure* measureToBeChecked = firstMeasure;
        Measure* lastMeasure = measureToBeChecked;
        int n       = 0;
        Fraction len;

        while (validMMRestMeasure(ctx, measureToBeChecked)) {
            if (n && breakMultiMeasureRest(ctx, measureToBeChecked)) {
                break;
            }
            if (measureToBeChecked != firstMeasure) {
                int measureNo = adjustMeasureNo(measureToBeChecked, ctx.state().measureNo());
                ctx.mutState().setMeasureNo(measureNo);
            }
            ++n;
            len += measureToBeChecked->ticks();
            lastMeasure = measureToBeChecked;
            MeasureBase* nextMeasureBase = ctx.conf().isShowVBox() ? measureToBeChecked->next() : measureToBeChecked->nextMeasure();
            if (!(nextMeasureBase && nextMeasureBase->isMeasure())) {
                break;
            }
            measureToBeChecked = toMeasure(nextMeasureBase);
        }

        if (n >= ctx.conf().styleI(Sid::minEmptyMeasures)) {
            createMMRest(ctx, firstMeasure, lastMeasure, len);
            ctx.mutState().setCurMeasure(firstMeasure->mmRest());
            ctx.mutState().setNextMeasure(ctx.conf().isShowVBox() ? lastMeasure->next() : lastMeasure->nextMeasure());
        } else {
            if (firstMeasure->mmRest()) {
                ctx.mutDom().undo(new ChangeMMRest(firstMeasure, 0));
            }
            firstMeasure->setMMRestCount(0);
            ctx.mutState().setMeasureNo(mno);
        }
    } else if (firstMeasure->isMMRest()) {
        LOGD("mmrest: no %d += %d", ctx.state().measureNo(), firstMeasure->mmRestCount());
        int measureNo = ctx.state().measureNo() + firstMeasure->mmRestCount() - 1;
        ctx.mutState().setMeasureNo(measureNo);
    }
}

void MeasureLayout::checkStaffMoveValidity(Measure* measure, const LayoutContext& ctx)
{
    for (const Segment& segment : measure->segments()) {
        if (!segment.isJustType(SegmentType::ChordRest)) {
            continue;
        }

        for (track_idx_t t = 0; t < ctx.dom().nstaves() * VOICES; ++t) {
            ChordRest* cr = toChordRest(segment.element(t));
            if (cr) {
                // Check if requested cross-staff is possible
                if (cr->staffMove() || cr->storedStaffMove()) {
                    cr->checkStaffMoveValidity();
                }
            }
        }
    }
}

void MeasureLayout::layoutMeasure(MeasureBase* currentMB, LayoutContext& ctx)
{
    IF_ASSERT_FAILED(currentMB == ctx.state().curMeasure()) {
        return;
    }

    if (!currentMB) {
        return;
    }

    int measureNo = adjustMeasureNo(currentMB, ctx.state().measureNo());
    LAYOUT_CALL() << LAYOUT_ITEM_INFO(currentMB) << " measureNo: " << measureNo;

    ctx.mutState().setMeasureNo(measureNo);

    createMultiMeasureRestsIfNeed(currentMB, ctx);

    currentMB = ctx.mutState().curMeasure();

    if (!currentMB->isMeasure()) {
        currentMB->setTick(ctx.state().tick());
        return;
    }

    //-----------------------------------------
    //    process one measure
    //-----------------------------------------

    Measure* measure = toMeasure(currentMB);

    measure->moveTicks(ctx.state().tick() - measure->tick());

    if (ctx.conf().isLinearMode() && (measure->tick() < ctx.state().startTick() || measure->tick() > ctx.state().endTick())) {
        // needed to reset segment widths if they can change after measure width is computed
        //for (Segment& s : measure->segments())
        //      s.createShapes();
        ctx.mutState().setTick(ctx.state().tick() + measure->ticks());
        return;
    }

    // Check if requested cross-staff is possible
    // This must happen before cmdUpdateNotes
    checkStaffMoveValidity(measure, ctx);

    // ---- Modify DOM ----
    ModifyDom::setCrossMeasure(measure, ctx);
    ModifyDom::connectTremolo(measure);
    ModifyDom::cmdUpdateNotes(measure, ctx.dom());
    ModifyDom::createStems(measure,  ctx);
    ModifyDom::setTrackForChordGraceNotes(measure, ctx.dom());
    // --------------------
    //
    // calculate accidentals and note lines,
    // create stem and set stem direction
    //

    const DomAccessor& dom = ctx.dom();
    const LayoutConfiguration& conf = ctx.conf();

    for (size_t staffIdx = 0; staffIdx < dom.nstaves(); ++staffIdx) {
        const Staff* staff = dom.staff(staffIdx);
        if (!staff->show()) {
            continue;
        }

        track_idx_t startTrack = staffIdx * VOICES;
        track_idx_t endTrack  = startTrack + VOICES;

        for (const Segment& segment : measure->segments()) {
            SegmentLayout::layoutMeasureIndependentElements(segment, startTrack, ctx);

            if (!segment.isJustType(SegmentType::ChordRest)) {
                continue;
            }

            //! NOTE Maybe it makes sense to group these methods by chord

            SegmentLayout::setChordMag(staff, segment, startTrack, endTrack, conf);

            SegmentLayout::layoutChordDrumset(staff, segment, startTrack, endTrack, conf);

            SegmentLayout::computeChordsUp(segment, startTrack, endTrack, ctx);

            SegmentLayout::layoutChordsStem(segment, startTrack, endTrack, ctx);
        }
    }

    BeamLayout::createBeams(ctx, measure);

    /* HACK: The real beam layout is computed at much later stage (you can't do the beams until you know
     * horizontal spacing). However, horizontal spacing needs to know stems extensions to avoid collision
     * with stems, and stems extensions depend on beams. Solution: we compute dummy beams here, *before*
     * horizontal spacing. It is pointless for the beams themselves, but it *does* correctly extend the
     * stems, thus allowing to compute horizontal spacing correctly. (M.S.) */
    for (Segment& s : measure->segments()) {
        if (!s.isChordRestType()) {
            continue;
        }
        BeamLayout::layoutNonCrossBeams(&s, ctx);
    }

    for (staff_idx_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
        const Staff* staff = ctx.dom().staff(staffIdx);
        if (!staff->show()) {
            continue;
        }

        for (Segment& segment : measure->segments()) {
            if (segment.isChordRestType()) {
                ChordLayout::layoutChords1(ctx, &segment, staffIdx);
                ChordLayout::resolveVerticalRestConflicts(ctx, &segment, staffIdx);
                for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
                    ChordRest* cr = segment.cr(staffIdx * VOICES + voice);
                    if (cr) {
                        for (Lyrics* l : cr->lyrics()) {
                            if (l) {
                                TLayout::layoutLyrics(l, ctx);
                            }
                        }
                    }
                }
            }
        }
    }

    for (Segment& segment : measure->segments()) {
        if (segment.isBreathType()) {
            for (EngravingItem* e : segment.elist()) {
                if (e && e->isBreath()) {
                    TLayout::layoutItem(e, ctx);
                }
            }
        } else if (segment.isChordRestType()) {
            for (EngravingItem* e : segment.annotations()) {
                if (e->isSymbol()) {
                    TLayout::layoutItem(e, ctx);
                }
            }
        }
    }

    Segment* seg = measure->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
    if (measure->repeatStart()) {
        if (!seg) {
            seg = measure->getSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
        }
        barLinesSetSpan(seg, ctx);          // this also creates necessary barlines
        for (size_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
            BarLine* b = toBarLine(seg->element(staffIdx * VOICES));
            if (b) {
                b->setBarLineType(BarLineType::START_REPEAT);
                TLayout::layoutBarLine(b, b->mutldata(), ctx);
            }
        }
    } else if (seg) {
        ctx.mutDom().undoRemoveElement(seg);
    }

    for (Segment& s : measure->segments()) {
        if (s.isEndBarLineType()) {
            continue;
        }
        s.createShapes();
    }

    measure->computeTicks(); // Must be called *after* Segment::createShapes() because it relies on the
    // Segment::visible() property, which is determined by Segment::createShapes().

    ctx.mutState().setTick(ctx.state().tick() + measure->ticks());
}

void MeasureLayout::updateGraceNotes(Measure* measure, LayoutContext& ctx)
{
    // Clean everything
    for (Segment& s : measure->segments()) {
        for (track_idx_t track = 0; track < ctx.dom().ntracks(); ++track) {
            EngravingItem* e = s.preAppendedItem(track);
            if (e && e->isGraceNotesGroup()) {
                s.clearPreAppended(track);
                std::set<staff_idx_t> stavesToReShape;
                for (Chord* grace : *toGraceNotesGroup(e)) {
                    stavesToReShape.insert(grace->staffIdx());
                    stavesToReShape.insert(grace->vStaffIdx());
                }
                for (staff_idx_t staffToReshape : stavesToReShape) {
                    s.createShape(staffToReshape);
                }
            }
        }
    }

    // Append grace notes to appropriate segment
    for (Segment& s : measure->segments()) {
        if (!s.isChordRestType()) {
            continue;
        }
        for (auto el : s.elist()) {
            if (el && el->isChord() && !toChord(el)->graceNotes().empty()) {
                ChordLayout::appendGraceNotes(toChord(el));
            }
        }
    }

    // Layout grace note groups
    for (Segment& s : measure->segments()) {
        for (track_idx_t track = 0; track < ctx.dom().ntracks(); ++track) {
            EngravingItem* e = s.preAppendedItem(track);
            if (e && e->isGraceNotesGroup()) {
                GraceNotesGroup* gng = toGraceNotesGroup(e);
                TLayout::layoutGraceNotesGroup(gng, ctx);
                gng->addToShape();
            }
        }
    }
}

void MeasureLayout::getNextMeasure(LayoutContext& ctx)
{
    TRACEFUNC;
    LAYOUT_CALL();

    moveToNextMeasure(ctx);

    layoutMeasure(ctx.mutState().curMeasure(), ctx);
}

//---------------------------------------------------------
//   adjustMeasureNo
//---------------------------------------------------------

int MeasureLayout::adjustMeasureNo(MeasureBase* m, int measureNo)
{
    measureNo += m->noOffset();
    m->setNo(measureNo);
    if (!m->irregular()) {          // don’t count measure
        ++measureNo;
    }

    const LayoutBreak* layoutBreak = m->sectionBreakElement();
    if (layoutBreak && layoutBreak->startWithMeasureOne()) {
        measureNo = 0;
    }

    return measureNo;
}

/****************************************************************
 * computePreSpacingItems
 * Computes information that is needed before horizontal spacing.
 * Caution: assumes that the system is known! (which is why we
 * cannot compute this stuff in LayoutMeasure::getNextMeasure().)
 * **************************************************************/
void MeasureLayout::computePreSpacingItems(Measure* m, LayoutContext& ctx)
{
    // Compute chord properties
    bool isFirstChordInMeasure = true;
    ChordLayout::clearLineAttachPoints(m);
    for (Segment& seg : m->segments()) {
        if (!seg.isChordRestType()) {
            continue;
        }
        for (EngravingItem* e : seg.elist()) {
            if (!e || !e->isChord()) {
                continue;
            }
            Chord* chord = toChord(e);
            Staff* staff = chord->staff();
            if (staff && !staff->show()) {
                continue;
            }

            ChordLayout::updateLineAttachPoints(chord, isFirstChordInMeasure, ctx);
            for (Chord* gn : chord->graceNotes()) {
                ChordLayout::updateLineAttachPoints(gn, false, ctx);
            }
            if (chord->arpeggio()) {
                ArpeggioLayout::clearAccidentals(chord->arpeggio(), ctx);
            }

            ChordLayout::layoutArticulations(chord, ctx);
            ChordLayout::checkStartEndSlurs(chord, ctx);
            chord->computeKerningExceptions();
        }
        seg.createShapes();
        isFirstChordInMeasure = false;
    }
}

void MeasureLayout::layoutStaffLines(Measure* m, LayoutContext& ctx)
{
    int staffIdx = 0;
    for (const MStaff* ms : m->mstaves()) {
        if (m->isCutawayClef(staffIdx) && (ctx.dom().staff(staffIdx)->cutaway() || !m->visible(staffIdx))) {
            // draw short staff lines for a courtesy clef on a hidden measure
            Segment* clefSeg = m->findSegmentR(SegmentType::Clef, m->ticks());
            double staffMag = ctx.dom().staff(staffIdx)->staffMag(m->tick());
            double partialWidth = clefSeg
                                  ? m->width() - clefSeg->x() + clefSeg->minLeft() + ctx.conf().styleMM(Sid::clefLeftMargin) * staffMag
                                  : 0.0;
            layoutPartialWidth(ms->lines(), ctx, m->width(), partialWidth / (m->spatium() * staffMag), true);
        } else {
            // normal staff lines
            TLayout::layoutStaffLines(ms->lines(), ctx);
        }
        staffIdx += 1;
    }
}

void MeasureLayout::layoutMeasureNumber(Measure* m, LayoutContext& ctx)
{
    bool smn = m->showsMeasureNumber();

    String s;
    if (smn) {
        s = String::number(m->no() + 1);
    }

    unsigned nn = 1;
    bool nas = ctx.conf().styleB(Sid::measureNumberAllStaves);

    if (!nas) {
        //find first non invisible staff
        for (unsigned staffIdx = 0; staffIdx < m->mstaves().size(); ++staffIdx) {
            if (m->visible(staffIdx)) {
                nn = staffIdx;
                break;
            }
        }
    }
    for (unsigned staffIdx = 0; staffIdx < m->mstaves().size(); ++staffIdx) {
        const MStaff* ms = m->mstaves().at(staffIdx);
        MeasureNumber* t = ms->noText();
        if (t) {
            t->setTrack(staffIdx * VOICES);
        }
        if (smn && ((staffIdx == nn) || nas)) {
            if (t == 0) {
                t = new MeasureNumber(m);
                t->setTrack(staffIdx * VOICES);
                t->setGenerated(true);
                t->setParent(m);
                m->add(t);
            }
            t->setXmlText(s);
            TLayout::layoutMeasureNumber(t, t->mutldata(), ctx);
        } else {
            if (t) {
                ctx.mutDom().doUndoRemoveElement(t);
            }
        }
    }
}

void MeasureLayout::layoutMMRestRange(Measure* m, LayoutContext& ctx)
{
    if (!m->isMMRest() || !ctx.conf().styleB(Sid::mmRestShowMeasureNumberRange)) {
        // Remove existing
        for (unsigned staffIdx = 0; staffIdx < m->mstaves().size(); ++staffIdx) {
            const MStaff* ms = m->mstaves().at(staffIdx);
            MMRestRange* rr = ms->mmRangeText();
            if (rr) {
                ctx.mutDom().doUndoRemoveElement(rr);
            }
        }

        return;
    }

    String s;
    if (m->mmRestCount() > 1) {
        // middle char is an en dash (not em)
        s = String(u"%1–%2").arg(m->no() + 1).arg(m->no() + m->mmRestCount());
    } else {
        // If the minimum range to create a mmrest is set to 1,
        // then simply show the measure number as there is no range
        s = String::number(m->no() + 1);
    }

    for (unsigned staffIdx = 0; staffIdx < m->mstaves().size(); ++staffIdx) {
        const MStaff* ms = m->mstaves().at(staffIdx);
        MMRestRange* rr = ms->mmRangeText();
        if (!rr) {
            rr = new MMRestRange(m);
            rr->setTrack(staffIdx * VOICES);
            rr->setGenerated(true);
            rr->setParent(m);
            m->add(rr);
        }
        // setXmlText is reimplemented to take care of brackets
        rr->setXmlText(s);
        TLayout::layoutMMRestRange(rr, rr->mutldata(), ctx);
    }
}

MeasureLayout::MeasureStartEndPos MeasureLayout::getMeasureStartEndPos(const Measure* measure, const Segment* firstCrSeg,
                                                                       const staff_idx_t staffIdx, const bool needsHeaderException,
                                                                       const bool modernMMRest,
                                                                       const LayoutContext& ctx)
{
    if (!measure || !firstCrSeg) {
        return MeasureStartEndPos(0.0, 0.0);
    }

    Segment* s1;
    for (s1 = firstCrSeg->prevActive(); s1 && s1->allElementsInvisible(); s1 = s1->prevActive()) {
    }
    Segment* s2;
    if (modernMMRest) {
        for (s2 = firstCrSeg->nextActive(); s2; s2 = s2->nextActive()) {
            if (!s2->isChordRestType() && s2->element(staffIdx * VOICES)) {
                break;
            }
        }
    } else {
        // Ignore any stuff before the end bar line (such as courtesy clefs)
        s2 = measure->findSegment(SegmentType::EndBarLine, measure->endTick());
    }

    double x1 = 0.0;
    while (s1) {
        x1 = std::max(x1, s1->x() + s1->minRight());
        s1 = s1->prevActive();
    }

    double x2 = s2 ? s2->x() - s2->minLeft() : measure->width();

    bool headerException = measure->header() && firstCrSeg->prev() && !firstCrSeg->prev()->isStartRepeatBarLineType()
                           && needsHeaderException;
    if (headerException) { //needs this exception on header bar
        // Set x1 to the imaginary barline located the minimum barline->note distance to the left of the rest's segment
        x1 = firstCrSeg->x() - ctx.conf().styleMM(Sid::barNoteDistance);
    }

    return MeasureStartEndPos(x1, x2);
}

//-----------------------------------------------------------------------------
//  layoutMeasureElements()
//  lays out all the element of a measure
//  LEGACY: this method used to be called stretchMeasure() and was used to
//  distribute the remaining space at the end of a system. That task is now
//  performed elsewhere and only the layout tasks are kept.
//-----------------------------------------------------------------------------

void MeasureLayout::layoutMeasureElements(Measure* m, LayoutContext& ctx)
{
    //---------------------------------------------------
    //    layout individual elements
    //---------------------------------------------------

    for (Segment& s : m->segments()) {
        if (!s.enabled()) {
            continue;
        }

        // After the rest of the spacing is calculated we position grace-notes-after.
        ChordLayout::repositionGraceNotesAfter(&s, ctx.dom().ntracks());

        for (EngravingItem* e : s.elist()) {
            if (!e) {
                continue;
            }
            staff_idx_t staffIdx = e->staffIdx();
            bool modernMMRest = e->isMMRest() && !toMMRest(e)->isOldStyle();
            if ((e->isRest() && toRest(e)->isFullMeasureRest()) || e->isMMRest() || e->isMeasureRepeat()) {
                //
                // element has to be centered in free space
                //    x1 - left measure position of free space
                //    x2 - right measure position of free space

                const MeasureStartEndPos measureStartEnd = getMeasureStartEndPos(m, &s, staffIdx, e->isMMRest(), modernMMRest, ctx);

                const double x1 = measureStartEnd.x1;
                const double x2 = measureStartEnd.x2;

                if (e->isMMRest()) {
                    MMRest* mmrest = toMMRest(e);
                    // center multimeasure rest
                    double d = ctx.conf().styleMM(Sid::multiMeasureRestMargin);
                    double w = x2 - x1 - 2 * d;
                    MMRest::LayoutData* mmrestLD = mmrest->mutldata();
                    mmrestLD->restWidth = w;
                    TLayout::layoutMMRest(mmrest, mmrest->mutldata(), ctx);
                    mmrestLD->setPosX(x1 - s.x() + d);
                } else if (e->isMeasureRepeat() && !(toMeasureRepeat(e)->numMeasures() % 2)) {
                    // two- or four-measure repeat, center on following barline
                    double measureWidth = x2 - s.x() + .5 * (m->styleP(Sid::barWidth));
                    e->mutldata()->setPosX(measureWidth - .5 * e->width());
                } else {
                    // full measure rest or one-measure repeat, center within this measure
                    TLayout::layoutItem(e, ctx);
                    Shape sh = e->ldata()->shape();
                    auto shEL = sh.find_first(ElementType::REST);
                    //! HACK Previously, bbox of rest was used here
                    //! Now we are using a shape, and in the shape we need to find the part related to rest
                    //! But in some cases, the information about what the ShapeElement belongs to will disappear at the moment (item is null),
                    //! in this case, let's just take the first element, it currently corresponds to the rest
                    if (!shEL) {
                        shEL = sh.get_first();
                    }
                    if (shEL) {
                        e->mutldata()->setPosX((x2 - x1 - shEL->width()) * .5 + x1 - s.x() - shEL->x());
                    }
                }
                s.createShape(staffIdx);            // DEBUG
            } else if (e->isRest()) {
                e->mutldata()->setPosX(0);
            } else if (e->isChord()) {
                Chord* c = toChord(e);
                if (c->tremoloSingleChord()) {
                    TremoloLayout::layout(c->tremoloSingleChord(), ctx);
                }

                if (c->tremoloTwoChord()) {
                    TremoloTwoChord* tr = c->tremoloTwoChord();
                    Chord* c1 = tr->chord1();
                    Chord* c2 = tr->chord2();
                    if (c1 && !c1->staffMove() && c2 && !c2->staffMove()) {
                        TremoloLayout::layout(tr, ctx);
                    }
                }

                for (Chord* g : c->graceNotes()) {
                    if (g->tremoloSingleChord()) {
                        TremoloLayout::layout(g->tremoloSingleChord(), ctx);
                    }

                    if (g->tremoloTwoChord()) {
                        TremoloTwoChord* tr = g->tremoloTwoChord();
                        Chord* gc1 = tr->chord1();
                        Chord* gc2 = tr->chord2();
                        if (gc1 && !gc1->staffMove() && gc2 && !gc2->staffMove()) {
                            TremoloLayout::layout(tr, ctx);
                        }
                    }
                }
            } else if (e->isBarLine()) {
                e->mutldata()->setPosY(0.0);
                // for end barlines, x position was set in createEndBarLines
                if (s.segmentType() != SegmentType::EndBarLine) {
                    e->mutldata()->setPosX(0.0);
                }
            }
        }
    }
}

void MeasureLayout::barLinesSetSpan(Segment* seg, LayoutContext& ctx)
{
    int track = 0;
    for (Staff* staff : ctx.dom().staves()) {
        BarLine* bl = toBarLine(seg->element(track));      // get existing bar line for this staff, if any
        if (bl) {
            if (bl->generated()) {
                bl->setSpanStaff(staff->barLineSpan());
                bl->setSpanFrom(staff->barLineFrom());
                bl->setSpanTo(staff->barLineTo());
            }
        } else {
            bl = Factory::createBarLine(seg);
            bl->setParent(seg);
            bl->setTrack(track);
            bl->setGenerated(true);
            bl->setSpanStaff(staff->barLineSpan());
            bl->setSpanFrom(staff->barLineFrom());
            bl->setSpanTo(staff->barLineTo());
            TLayout::layoutBarLine(bl, bl->mutldata(), ctx);
            ctx.mutDom().addElement(bl);
        }
        track += VOICES;
    }
}

//---------------------------------------------------------
//   createEndBarLines
//    actually creates or modifies barlines
//    return the width change for measure
//---------------------------------------------------------

void MeasureLayout::createEndBarLines(Measure* m, bool isLastMeasureInSystem, LayoutContext& ctx)
{
    size_t nstaves  = ctx.dom().nstaves();
    Segment* seg = m->findSegmentR(SegmentType::EndBarLine, m->ticks());
    Measure* nm  = m->nextMeasure();
    double blw    = 0.0;

    if (nm && nm->repeatStart() && !m->repeatEnd() && !isLastMeasureInSystem && m->next() == nm) {
        // we may skip barline at end of a measure immediately before a start repeat:
        // next measure is repeat start, this measure is not a repeat end,
        // this is not last measure of system, no intervening frame
        if (!seg) {
            return;
        }
        seg->setEnabled(false);
    } else {
        BarLineType t = nm ? BarLineType::NORMAL : BarLineType::END;
        if (!seg) {
            seg = m->getSegmentR(SegmentType::EndBarLine, m->ticks());
        }
        seg->setEnabled(true);

        m->setHasCourtesyKeySig(false);
        //  Set flag "hasCourtesyKeySig" if this measure needs a courtesy key sig.
        //  This flag is later used to set a double end bar line and to actually
        //  create the courtesy key sig.

        if (nm && !m->sectionBreak()) {
            //  Don't change barlines at the end of a section break,
            //  and don't create courtesy key/time signatures.
            bool hasKeySig = false;
            bool showCourtesyKeySig = isLastMeasureInSystem && ctx.conf().styleB(Sid::genCourtesyKeysig);

            Fraction tick = m->endTick();
            for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                const Staff* staff     = ctx.dom().staff(staffIdx);
                KeySigEvent key1 = staff->keySigEvent(tick - Fraction::fromTicks(1));
                KeySigEvent key2 = staff->keySigEvent(tick);
                if (!(key1 == key2)) {
                    // locate a key sig. in next measure and, if found,
                    // check if it has court. sig turned off
                    Segment* s = nm->findSegment(SegmentType::KeySig, tick);
                    if (s) {
                        hasKeySig = true;
                        KeySig* ks = toKeySig(s->element(staffIdx * VOICES));
                        if (ks && !ks->showCourtesy()) {
                            continue;
                        }
                    }
                    if (showCourtesyKeySig) {
                        m->setHasCourtesyKeySig(true);
                    }
                    break;
                }
            }

            int keySigBarlineMode = ctx.conf().styleI(Sid::keySigCourtesyBarlineMode);
            if (keySigBarlineMode == int(CourtesyBarlineMode::DOUBLE_BEFORE_COURTESY)) {
                if (m->hasCourtesyKeySig()) {
                    t = BarLineType::DOUBLE;
                }
            } else if (keySigBarlineMode == int(CourtesyBarlineMode::ALWAYS_DOUBLE)) {
                if (hasKeySig) {
                    t = BarLineType::DOUBLE;
                }
            }

            bool hasTimeSig = false;
            bool hasCourtesyTimeSig = false;
            bool showCourtesyTimeSig = isLastMeasureInSystem && ctx.conf().styleB(Sid::genCourtesyTimesig);

            Segment* tss = nm->findSegmentR(SegmentType::TimeSig, Fraction(0, 1));
            if (tss) {
                for (track_idx_t track = 0; track < nstaves * VOICES; track += VOICES) {
                    TimeSig* ts = toTimeSig(tss->element(track));
                    if (ts) {
                        hasTimeSig = true;
                        if (ts->showCourtesySig() && showCourtesyTimeSig) {
                            hasCourtesyTimeSig = true;
                        }
                        break;
                    }
                }
            }

            int timeSigBarlineMode = ctx.conf().styleI(Sid::timeSigCourtesyBarlineMode);
            if (timeSigBarlineMode == int(CourtesyBarlineMode::DOUBLE_BEFORE_COURTESY)) {
                if (hasCourtesyTimeSig) {
                    t = BarLineType::DOUBLE;
                }
            } else if (timeSigBarlineMode == int(CourtesyBarlineMode::ALWAYS_DOUBLE)) {
                if (hasTimeSig) {
                    t = BarLineType::DOUBLE;
                }
            }
        }

        bool force = false;
        if (m->repeatEnd()) {
            t = BarLineType::END_REPEAT;
            force = true;
        }

        for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            track_idx_t track = staffIdx * VOICES;
            BarLine* bl  = toBarLine(seg->element(track));
            const Staff* staff = ctx.dom().staff(staffIdx);
            if (!bl) {
                bl = Factory::createBarLine(seg);
                bl->setParent(seg);
                bl->setTrack(track);
                bl->setGenerated(true);
                bl->setSpanStaff(staff->barLineSpan());
                bl->setSpanFrom(staff->barLineFrom());
                bl->setSpanTo(staff->barLineTo());
                bl->setBarLineType(t);
                ctx.mutDom().addElement(bl);
            } else {
                // do not change bar line type if bar line is user modified
                // and its not a repeat start/end barline (forced)

                if (bl->generated()) {
                    bl->setSpanStaff(staff->barLineSpan());
                    bl->setSpanFrom(staff->barLineFrom());
                    bl->setSpanTo(staff->barLineTo());
                    bl->setBarLineType(t);
                } else {
                    if (bl->barLineType() != t) {
                        if (force) {
                            bl->undoChangeProperty(Pid::BARLINE_TYPE, PropertyValue::fromValue(t));
                            bl->setGenerated(true);
                        }
                    }
                }
            }

            TLayout::layoutBarLine(bl, bl->mutldata(), ctx);
            blw = std::max(blw, bl->width());
        }
        // right align within segment
        for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            track_idx_t track = staffIdx * VOICES;
            BarLine* bl = toBarLine(seg->element(track));
            if (bl) {
                bl->mutldata()->moveX(blw - bl->width());
            }
        }
        seg->createShapes();
    }

    // set relative position of end barline and clef
    // if end repeat, clef goes after, otherwise clef goes before
    Segment* clefSeg = m->findSegmentR(SegmentType::Clef, m->ticks());
    ClefToBarlinePosition clefToBarlinePosition = ClefToBarlinePosition::AUTO;
    if (clefSeg) {
        int visibleInt = 0;
        for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            if (!ctx.dom().staff(staffIdx)->show()) {
                continue;
            }
            track_idx_t track = staffIdx * VOICES;
            Clef* clef = toClef(clefSeg->element(track));
            if (clef) {
                clefToBarlinePosition = clef->clefToBarlinePosition();
                bool showCourtesy = ctx.conf().styleB(Sid::genCourtesyClef) && clef->showCourtesy();         // normally show a courtesy clef
                // check if the measure is the last measure of the system or the last measure before a frame
                bool lastMeasure = isLastMeasureInSystem || (nm ? !(m->next() == nm) : true);
                if (!nm || m->isFinalMeasureOfSection() || (lastMeasure && !showCourtesy)) {
                    // hide the courtesy clef in the final measure of a section, or if the measure is the final measure of a system
                    // and the score style or the clef style is set to "not show courtesy clef",
                    // or if the clef is at the end of the very last measure of the score
                    clef->clear();
                    clefSeg->createShape(staffIdx);
                    if (visibleInt == 0) {
                        visibleInt = 1;
                    }
                } else {
                    TLayout::layoutClef(clef, clef->mutldata(), ctx.conf());
                    clefSeg->createShape(staffIdx);
                    visibleInt = 2;
                }
            }
        }
        if (visibleInt == 2) {         // there is at least one visible clef in the clef segment
            clefSeg->setVisible(true);
        } else if (visibleInt == 1) {  // all (courtesy) clefs in the clef segment are not visible
            clefSeg->setVisible(false);
        } else { // should never happen
            LOGD("Clef Segment without Clef elements at tick %d/%d", clefSeg->tick().numerator(), clefSeg->tick().denominator());
        }
        if (seg) {
            Segment* s1 = nullptr;
            Segment* s2 = nullptr;
            if (m->repeatEnd() && clefToBarlinePosition != ClefToBarlinePosition::BEFORE) {
                s1 = seg;
                s2 = clefSeg;
            } else {
                s1 = clefSeg;
                s2 = seg;
            }
            if (s1->next() != s2) {
                m->segments().remove(s1);
                m->segments().insert(s1, s2);
            }
        }
    }
}

Segment* MeasureLayout::addHeaderClef(Measure* m, bool isFirstClef, const Staff* staff, LayoutContext& ctx)
{
    const staff_idx_t staffIdx = staff->idx();
    const track_idx_t track = staffIdx * VOICES;
    Segment* cSegment = m->findFirstR(SegmentType::HeaderClef, Fraction(0, 1));
    const StaffType* staffType = staff->staffType(m->tick());

    const bool hideClef = staffType->isTabStaff() ? ctx.conf().styleB(Sid::hideTabClefAfterFirst) : !ctx.conf().styleB(Sid::genClef);

    // find the clef type at the previous tick
    ClefTypeList cl = staff->clefType(m->tick() - Fraction::fromTicks(1));
    bool showCourtesy = true;
    Segment* s = nullptr;
    if (m->prevMeasure()) {
        // look for a clef change at the end of the previous measure
        s = m->prevMeasure()->findSegment(SegmentType::Clef, m->tick());
    } else if (m->isMMRest()) {
        // look for a header clef at the beginning of the first underlying measure
        s = m->mmRestFirst()->findFirstR(SegmentType::HeaderClef, Fraction(0, 1));
    }
    if (s) {
        Clef* c = toClef(s->element(track));
        if (c) {
            cl = c->clefTypeList();
            showCourtesy = c->showCourtesy();
        }
    }
    Clef* clef = nullptr;
    if (cSegment) {
        clef = toClef(cSegment->element(track));
    }
    if (staff->staffTypeForElement(m)->genClef() && (isFirstClef || !hideClef)) {
        if (!cSegment) {
            cSegment = Factory::createSegment(m, SegmentType::HeaderClef, Fraction(0, 1));
            cSegment->setHeader(true);
            m->add(cSegment);
        }
        if (!clef) {
            //
            // create missing clef
            //
            clef = Factory::createClef(cSegment);
            clef->setTrack(track);
            clef->setGenerated(true);
            clef->setParent(cSegment);
            clef->setIsHeader(true);
            clef->setShowCourtesy(showCourtesy);
            cSegment->add(clef);
        }
        if (clef->generated()) {
            clef->setClefType(cl);
        }
        clef->setSmall(false);
        clef->mutldata()->reset();
        TLayout::layoutClef(clef, clef->mutldata(), ctx.conf());
        cSegment->setEnabled(true);
    } else if (clef) {
        clef->parentItem()->remove(clef);
        if (clef->generated()) {
            delete clef;
        }
    }

    return cSegment;
}

Segment* MeasureLayout::addHeaderKeySig(Measure* m, bool isFirstKeysig, const Staff* staff, LayoutContext& ctx)
{
    const staff_idx_t staffIdx = staff->idx();
    const track_idx_t track = staffIdx * VOICES;
    Segment* kSegment = m->findFirstR(SegmentType::KeySig, Fraction(0, 1));
    // If we need a Key::C KeySig (which would be invisible) and there is
    // a courtesy key sig, don’t create it and switch generated flags.
    // This avoids creating an invisible KeySig which can distort layout.

    KeySigEvent keyIdx = staff->keySigEvent(m->tick());
    KeySig* ksAnnounce = 0;
    if ((isFirstKeysig || ctx.conf().styleB(Sid::genKeysig)) && (keyIdx.key() == Key::C)) {
        Measure* pm = m->prevMeasure();
        if (pm && pm->hasCourtesyKeySig()) {
            Segment* ks = pm->first(SegmentType::KeySigAnnounce);
            if (ks) {
                ksAnnounce = toKeySig(ks->element(track));
                if (ksAnnounce) {
                    isFirstKeysig = false;
                    //                                    if (keysig) {
                    //                                          ksAnnounce->setGenerated(false);
                    //TODO                                      keysig->setGenerated(true);
                    //                                          }
                }
            }
        }
    }

    bool isPitchedStaff = staff->isPitchedStaff(m->tick());

    KeySig* keysig = nullptr;
    if (kSegment) {
        keysig = toKeySig(kSegment->element(track));
    }
    // keep key sigs in TABs: TABs themselves should hide them
    if ((isFirstKeysig || ctx.conf().styleB(Sid::genKeysig)) && isPitchedStaff) {
        if (!kSegment) {
            kSegment = Factory::createSegment(m, SegmentType::KeySig, Fraction(0, 1));
            kSegment->setHeader(true);
            m->add(kSegment);
        }
        if (!keysig) {
            //
            // create missing key signature
            //
            keysig = Factory::createKeySig(kSegment);
            keysig->setTrack(track);
            keysig->setGenerated(true);
            keysig->setParent(kSegment);
            kSegment->add(keysig);
        }
        keysig->setKeySigEvent(keyIdx);
        keysig->mutldata()->reset();
        TLayout::layoutKeySig(keysig, keysig->mutldata(), ctx.conf());
        kSegment->setEnabled(true);
    } else if (keysig && isPitchedStaff) {
        // do not remove user modified keysigs
        bool remove = true;
        EngravingItem* e = kSegment->element(staffIdx * VOICES);
        Key key = staff->key(m->tick());
        if ((e && !e->generated()) || (key != keyIdx.key())) {
            remove = false;
        }

        if (remove) {
            keysig->parentItem()->remove(keysig);
            if (keysig->generated()) {
                delete keysig;
            }
        }
    }

    return kSegment;
}

//-------------------------------------------------------------------
//   addSystemHeader
///   Add elements to make this measure suitable as the first measure
///   of a system.
//    The system header can contain a starting BarLine, a Clef,
//    and a KeySig
//-------------------------------------------------------------------

void MeasureLayout::addSystemHeader(Measure* m, bool isFirstSystem, LayoutContext& ctx)
{
    int staffIdx = 0;
    Segment* kSegment = nullptr;
    Segment* cSegment = nullptr;

    for (const Staff* staff : ctx.dom().staves()) {
        const int track = staffIdx * VOICES;

        // Check if this is the first VISIBLE appearance
        bool isFirstClef = true;
        bool isFirstKeySig = true;
        if (!isFirstSystem) {
            const Fraction clefTick = staff->currentClefTick(m->tick());
            const Fraction keySigTick = staff->currentKeyTick(m->tick());
            // Get first measure whether MMR or not
            Measure* searchMeasure = ctx.mutDom().tick2measure(std::min(clefTick, keySigTick));
            searchMeasure = searchMeasure->hasMMRest()
                            && ctx.conf().styleB(Sid::createMultiMeasureRests) ? searchMeasure->mmRest() : searchMeasure;
            while (searchMeasure->tick() < m->tick() && (isFirstClef || isFirstKeySig)) {
                const System* sys = searchMeasure->system();
                if (isFirstClef && searchMeasure->tick() >= clefTick) {
                    // Need to check previous measure for clef change if one not found in this measure
                    Segment* clefSeg = searchMeasure->findFirstR(SegmentType::Clef | SegmentType::HeaderClef, Fraction(0, 0));
                    Measure* prevMeas = searchMeasure->prevMeasure();
                    if (prevMeas && !clefSeg) {
                        clefSeg = prevMeas->findSegment(SegmentType::Clef, m->tick());
                    }
                    if (clefSeg && clefSeg->enabled()) {
                        const Clef* c = toClef(clefSeg->element(track));
                        if (c && sys && sys->staff(staffIdx)->show()) {
                            isFirstClef = false;
                        }
                    }
                }
                if (isFirstKeySig && searchMeasure->tick() >= keySigTick) {
                    const Segment* ksSeg = searchMeasure->findSegment(SegmentType::KeySig, searchMeasure->tick());
                    if (ksSeg && ksSeg->enabled()) {
                        const KeySig* ks = toKeySig(ksSeg->element(track));
                        if (ks && sys && sys->staff(staffIdx)->show()) {
                            isFirstKeySig = false;
                        }
                    }
                }
                searchMeasure = searchMeasure->nextMeasureMM();
            }
        }

        cSegment = addHeaderClef(m, isFirstSystem || isFirstClef, staff, ctx);

        kSegment = addHeaderKeySig(m, isFirstSystem || isFirstKeySig, staff, ctx);

        ++staffIdx;
    }
    if (cSegment) {
        cSegment->createShapes();
    }
    if (kSegment) {
        kSegment->createShapes();
    }

    MeasureLayout::createSystemBeginBarLine(m, ctx);

    m->checkHeader();
}

void MeasureLayout::removeSystemHeader(Measure* m)
{
    if (!m->header()) {
        return;
    }
    for (Segment* seg = m->first(); seg; seg = seg->next()) {
        if (!seg->header()) {
            break;
        }
        seg->setEnabled(false);
    }
    m->setHeader(false);
}

void MeasureLayout::addSystemTrailer(Measure* m, Measure* nm, LayoutContext& ctx)
{
    Fraction _rtick = m->ticks();
    bool isFinalMeasure = m->isFinalMeasureOfSection();

    // locate a time sig. in the next measure and, if found,
    // check if it has court. sig. turned off
    TimeSig* ts = nullptr;
    bool showCourtesySig = false;
    Segment* s = m->findSegmentR(SegmentType::TimeSigAnnounce, _rtick);
    if (s) {
        s->setTrailer(true);
    }
    if (nm && ctx.conf().styleB(Sid::genCourtesyTimesig) && !isFinalMeasure && !ctx.conf().isFloatMode()) {
        Segment* tss = nm->findSegmentR(SegmentType::TimeSig, Fraction(0, 1));
        if (tss) {
            size_t nstaves = ctx.dom().nstaves();
            for (track_idx_t track = 0; track < nstaves * VOICES; track += VOICES) {
                ts = toTimeSig(tss->element(track));
                if (ts) {
                    break;
                }
            }
            if (ts && ts->showCourtesySig()) {
                showCourtesySig = true;
                // if due, create a new courtesy time signature for each staff
                if (!s) {
                    s  = Factory::createSegment(m, SegmentType::TimeSigAnnounce, _rtick);
                    s->setTrailer(true);
                    m->add(s);
                }

                s->setEnabled(true);

                for (track_idx_t track = 0; track < nstaves * VOICES; track += VOICES) {
                    TimeSig* nts = toTimeSig(tss->element(track));
                    if (!nts) {
                        continue;
                    }
                    ts = toTimeSig(s->element(track));
                    if (!ts) {
                        ts = Factory::createTimeSig(s);
                        ts->setTrack(track);
                        ts->setGenerated(true);
                        ts->setParent(s);
                        ctx.mutDom().undoAddElement(ts);
                        s->setTrailer(true);
                    }
                    ts->setFrom(nts);
                    TLayout::layoutTimeSig(ts, ts->mutldata(), ctx);
                    //s->createShape(track / VOICES);
                }
                s->createShapes();
            }
        }
    }
    if (!showCourtesySig && s) {
        // remove any existing time signatures
        s->setEnabled(false);
    }

    // courtesy key signatures, clefs

    size_t n   = ctx.dom().nstaves();
    bool show  = m->hasCourtesyKeySig();
    s          = m->findSegmentR(SegmentType::KeySigAnnounce, _rtick);

    Segment* clefSegment = m->findSegmentR(SegmentType::Clef, m->ticks());

    for (staff_idx_t staffIdx = 0; staffIdx < n; ++staffIdx) {
        track_idx_t track = staffIdx * VOICES;
        const Staff* staff = ctx.dom().staff(staffIdx);
        bool staffIsPitchedAtNextMeas = m->nextMeasure() && staff->isPitchedStaff(m->nextMeasure()->tick());

        if (show) {
            if (!s) {
                s = Factory::createSegment(m, SegmentType::KeySigAnnounce, _rtick);
                s->setTrailer(true);
                m->add(s);
            }

            KeySig* keySig = nullptr;
            EngravingItem* keySigElem = s->element(track);
            if (keySigElem && keySigElem->isKeySig()) {
                keySig = toKeySig(keySigElem);
            }

            KeySigEvent key2 = staff->keySigEvent(m->endTick());
            bool needsCourtesy = staff->key(m->tick()) != key2.key();

            if (staffIsPitchedAtNextMeas && needsCourtesy) {
                if (!keySig) {
                    keySig = Factory::createKeySig(s);
                    keySig->setTrack(track);
                    keySig->setGenerated(true);
                    keySig->setParent(s);
                    s->add(keySig);
                    s->setTrailer(true);
                }
                keySig->setKeySigEvent(key2);
                TLayout::layoutKeySig(keySig, keySig->mutldata(), ctx.conf());
                //s->createShape(track / VOICES);
                s->setEnabled(true);
            } else { /// !staffIsPitchedAtNextMeas || !needsCourtesy
                if (keySig) {
                    s->remove(keySig);
                }
            }
        } else { /// !show
            // remove any existent courtesy key signature
            if (s) {
                s->setEnabled(false);
            }
        }
        if (clefSegment) {
            Clef* clef = toClef(clefSegment->element(track));
            if (clef) {
                clef->setSmall(true);
            }
        }
    }
    if (s) {
        s->createShapes();
    }
    if (clefSegment) {
        clefSegment->createShapes();
    }

    m->checkTrailer();
}

void MeasureLayout::removeSystemTrailer(Measure* m)
{
    for (Segment* seg = m->last(); seg != m->first(); seg = seg->prev()) {
        if (seg->isTimeTickType()) {
            continue;
        }
        if (!seg->trailer()) {
            break;
        }
        if (seg->enabled()) {
            seg->setEnabled(false);
        }
    }
    m->setTrailer(false);
}

void MeasureLayout::createSystemBeginBarLine(Measure* m, LayoutContext& ctx)
{
    if (!m->system()) {
        return;
    }
    Segment* s  = m->findSegment(SegmentType::BeginBarLine, m->tick());
    size_t n = 0;
    if (m->system()) {
        for (SysStaff* sysStaff : m->system()->staves()) {
            if (sysStaff->show()) {
                ++n;
            }
        }
    }
    if ((n > 1 && ctx.conf().styleB(Sid::startBarlineMultiple))
        || (n == 1 && (ctx.conf().styleB(Sid::startBarlineSingle) || m->system()->brackets().size()))) {
        if (!s) {
            s = Factory::createSegment(m, SegmentType::BeginBarLine, Fraction(0, 1));
            m->add(s);
        }
        for (track_idx_t track = 0; track < ctx.dom().ntracks(); track += VOICES) {
            BarLine* bl = toBarLine(s->element(track));
            if (!bl) {
                bl = Factory::createBarLine(s);
                bl->setTrack(track);
                bl->setGenerated(true);
                bl->setParent(s);
                bl->setBarLineType(BarLineType::NORMAL);
                bl->setSpanStaff(true);
                s->add(bl);
            }

            TLayout::layoutBarLine(bl, bl->mutldata(), ctx);
        }
        s->createShapes();
        s->setEnabled(true);
        s->setHeader(true);
        m->setHeader(true);
    } else if (s) {
        s->setEnabled(false);
    }
}

void MeasureLayout::stretchMeasureInPracticeMode(Measure* m, double targetWidth, LayoutContext& ctx)
{
    Measure::LayoutData* ldata = m->mutldata();
    ldata->setWidth(targetWidth);

    //---------------------------------------------------
    //    compute stretch
    //---------------------------------------------------

    std::multimap<double, Segment*> springs;

    Segment* seg = m->first();
    while (seg && !seg->enabled()) {
        seg = seg->next();
    }
    double minimumWidth = seg ? seg->x() : 0.0;
    for (Segment& s : m->segments()) {
        s.setStretch(1);

//        if (!s.enabled() || !s.visible())
//            ;continue;
        Fraction t = s.ticks();
        if (t.isNotZero()) {
            springs.insert(std::pair<double, Segment*>(0, &s));
        }
        minimumWidth += s.width() + s.spacing();
    }

    //---------------------------------------------------
    //    compute 1/Force for a given Extend
    //---------------------------------------------------

    if (targetWidth > minimumWidth) {
        double chordRestSegmentsWidth{ 0 };
        for (auto i = springs.begin(); i != springs.end(); i++) {
            chordRestSegmentsWidth += i->second->width() + i->second->spacing();
        }

        double stretch = 1 + (targetWidth - minimumWidth) / chordRestSegmentsWidth;

        //---------------------------------------------------
        //    distribute stretch to segments
        //---------------------------------------------------

        for (auto& i : springs) {
            i.second->setStretch(stretch);
        }

        //---------------------------------------------------
        //    move segments to final position
        //---------------------------------------------------

        Segment* s = m->first();
        while (s && !s->enabled()) {
            s = s->next();
        }

        double x = s ? s->pos().x() : 0.0;
        while (s) {
//            if (!s->enabled() || !s->visible()) {
//                s = s->nextEnabled();
//                continue;
//            }

            double spacing = s->spacing();
            double widthWithoutSpacing = s->width() - spacing;
            double segmentStretch = s->stretch();
            x += spacing * (muse::RealIsNull(segmentStretch) ? 1 : segmentStretch);
            s->mutldata()->setPosX(x);
            x += widthWithoutSpacing * (muse::RealIsNull(segmentStretch) ? 1 : segmentStretch);
            s = s->nextEnabled();
        }
    }

    //---------------------------------------------------
    //    layout individual elements
    //---------------------------------------------------

    for (Segment& s : m->segments()) {
        if (!s.enabled()) {
            continue;
        }
        // After the rest of the spacing is calculated we position grace-notes-after.
        ChordLayout::repositionGraceNotesAfter(&s, ctx.dom().ntracks());
        for (EngravingItem* e : s.elist()) {
            if (!e) {
                continue;
            }
            ElementType t = e->type();
            staff_idx_t staffIdx = e->staffIdx();
            if (t == ElementType::MEASURE_REPEAT || (t == ElementType::REST && (m->isMMRest() || toRest(e)->isFullMeasureRest()))) {
                //
                // element has to be centered in free space
                //    x1 - left measure position of free space
                //    x2 - right measure position of free space

                Segment* s1;
                for (s1 = s.prev(); s1 && !s1->enabled(); s1 = s1->prev()) {
                }
                Segment* s2;
                for (s2 = s.next(); s2; s2 = s2->next()) {
                    if (s2->enabled() && !s2->isChordRestType() && s2->element(staffIdx * VOICES)) {
                        break;
                    }
                }
                double x1 = s1 ? s1->x() + s1->minRight() : 0;
                double x2 = s2 ? s2->x() - s2->minLeft() : targetWidth;

                if (m->isMMRest()) {
                    MMRest* mmrest = toMMRest(e);
                    //
                    // center multi measure rest
                    //
                    double d = ctx.conf().styleMM(Sid::multiMeasureRestMargin);
                    double w = x2 - x1 - 2 * d;

                    mmrest->mutldata()->restWidth = w;
                    TLayout::layoutMMRest(mmrest, mmrest->mutldata(), ctx);
                    e->setPos(x1 - s.x() + d, e->staff()->staffHeight() * .5);   // center vertically in measure
                    s.createShape(staffIdx);
                } else { // if (rest->isFullMeasureRest()) {
                    //
                    // center full measure rest
                    //
                    e->mutldata()->setPosX((x2 - x1 - e->width()) * .5 + x1 - s.x() - e->ldata()->bbox().x());
                    s.createShape(staffIdx);  // DEBUG
                }
            } else if (t == ElementType::REST) {
                e->mutldata()->setPosX(0);
            } else if (t == ElementType::CHORD) {
                Chord* c = toChord(e);
                if (c->tremoloSingleChord()) {
                    TremoloLayout::layout(c->tremoloSingleChord(), ctx);
                } else if (c->tremoloTwoChord()) {
                    TremoloTwoChord* tr = c->tremoloTwoChord();
                    Chord* c1 = tr->chord1();
                    Chord* c2 = tr->chord2();
                    if (c1 && !c1->staffMove() && c2 && !c2->staffMove()) {
                        TremoloLayout::layout(tr, ctx);
                    }
                }
            } else if (t == ElementType::BAR_LINE) {
                e->mutldata()->setPosY(0.0);
                // for end barlines, x position was set in createEndBarLines
                if (s.segmentType() != SegmentType::EndBarLine) {
                    e->mutldata()->setPosX(0.0);
                }
            }
        }
    }
}

void MeasureLayout::layoutTimeTickAnchors(Measure* m, LayoutContext& ctx)
{
    bool darker = true;
    for (Segment& segment : m->segments()) {
        if (!segment.isTimeTickType()) {
            continue;
        }

        Segment* refCRSeg = m->findSegmentR(SegmentType::ChordRest, segment.rtick());
        if (!refCRSeg) {
            refCRSeg = segment.prev();
            while (refCRSeg && !refCRSeg->isChordRestType()) {
                refCRSeg = refCRSeg->prev();
            }
        }

        if (!refCRSeg || refCRSeg->ticks().isZero()) {
            continue;
        }

        Fraction refSegDuration = refCRSeg->ticks();
        Fraction thisDuration = segment.ticks();
        Fraction relativeTick = segment.rtick() - refCRSeg->rtick();

        Segment* nextCRSeg = m->findSegmentR(SegmentType::ChordRest, refCRSeg->rtick() + refCRSeg->ticks());
        double width = nextCRSeg ? nextCRSeg->x() - refCRSeg->x() : refCRSeg->width();

        double relativeX = width * (relativeTick.toDouble() / refCRSeg->ticks().toDouble());
        double relativeWidth = width * (thisDuration.toDouble() / refSegDuration.toDouble());

        segment.mutldata()->setPosX(refCRSeg->x() + relativeX);
        segment.setWidth(relativeWidth);

        for (EngravingItem* item : segment.elist()) {
            if (item) {
                TLayout::layoutItem(item, ctx);
                toTimeTickAnchor(item)->mutldata()->setDarker(darker);
            }
        }
        darker = !darker;
    }
}

//---------------------------------------------------------
//   layoutPartialWidth
///   Layout staff lines for the specified width only, aligned
///   to the left or right of the measure
//---------------------------------------------------------

void MeasureLayout::layoutPartialWidth(StaffLines* lines, LayoutContext& ctx, double w, double wPartial, bool alignRight)
{
    StaffLines::LayoutData* ldata = lines->mutldata();
    const Staff* s = lines->staff();
    double _spatium = lines->spatium();
    wPartial *= _spatium;
    double dist     = _spatium;
    lines->setPos(PointF(0.0, 0.0));
    int _lines;
    if (s) {
        lines->mutldata()->setMag(s->staffMag(lines->measure()->tick()));
        lines->setColor(s->color(lines->measure()->tick()));
        const StaffType* st = s->staffType(lines->measure()->tick());
        dist         *= st->lineDistance().val();
        _lines        = st->lines();
        lines->mutldata()->setPosY(lines->staffOffsetY());
    } else {
        _lines = 5;
        lines->setColor(lines->configuration()->defaultColor());
    }
    lines->setLw(ctx.conf().styleS(Sid::staffLineWidth).val() * _spatium);
    double x1 = lines->pos().x();
    double x2 = x1 + w;
    double y  = lines->pos().y();
    ldata->setBbox(x1, -lines->lw() * .5 + y, w, (_lines - 1) * dist + lines->lw());

    if (_lines == 1) {
        double extraSize = _spatium;
        ldata->setBbox(ldata->bbox().adjusted(0, -extraSize, 0, extraSize));
    }

    std::vector<LineF> ll;
    for (int i = 0; i < _lines; ++i) {
        if (alignRight) {
            ll.push_back(LineF(x2 - wPartial, y, x2, y));
        } else {
            ll.push_back(LineF(x1, y, x1 + wPartial, y));
        }
        y += dist;
    }
    lines->setLines(ll);
}
