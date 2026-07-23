/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#include "mmrestlayout.h"

#include "dom/barline.h"
#include "dom/factory.h"
#include "dom/layoutbreak.h"
#include "dom/marker.h"
#include "dom/measure.h"
#include "dom/mmrest.h"
#include "dom/mmrestrange.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/staff.h"

#include "editing/editmeasures.h"

#include "measurelayout.h"
#include "tlayout.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

static const std::unordered_set<ElementType> BREAK_TYPES {
    ElementType::TEMPO_TEXT,
    ElementType::REHEARSAL_MARK,
    ElementType::HARMONY,
    ElementType::STAFF_TEXT,
    ElementType::EXPRESSION,
    ElementType::DYNAMIC,
    ElementType::SYSTEM_TEXT,
    ElementType::TRIPLET_FEEL,
    ElementType::PLAYTECH_ANNOTATION,
    ElementType::CAPO,
    ElementType::INSTRUMENT_CHANGE,
    ElementType::STRING_TUNINGS,
    ElementType::SYMBOL,
    ElementType::FRET_DIAGRAM,
    ElementType::HARP_DIAGRAM,
    ElementType::PLAY_COUNT_TEXT,
    ElementType::FERMATA,
};

static const std::unordered_set<ElementType> ALWAYS_BREAK_TYPES {
    ElementType::TEMPO_TEXT,
    ElementType::REHEARSAL_MARK
};

static const std::unordered_set<ElementType> CONDITIONAL_BREAK_TYPES {
    ElementType::HARMONY,
    ElementType::STAFF_TEXT,
    ElementType::EXPRESSION,
    ElementType::DYNAMIC,
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

/*
    MMRests are measures which overlay existing notation
    When they are created, any item which should be visible is moved from the underlying measure to the MMRest measure.
    Items which are invisible remain in the underlying measure.
    This means that when deciding where in the score to break MMRests, we must check both the underlying measures and any MMRests connected
    to them
*/

void MMRestLayout::reuseExistingMMRest(LayoutContext& ctx, Measure* mmrMeasure, Measure* lastMeasure, Fraction len)
{
    if (mmrMeasure->ticks() == len) {
        MeasureLayout::removeSystemTrailer(mmrMeasure, ctx);
        return;
    }

    Fraction remainingMMRDuration = std::min(mmrMeasure->ticks() - len, ctx.dom().lastMeasure()->endTick() - len);

    Segment* barLineSeg = mmrMeasure->findSegmentR(SegmentType::EndBarLine, mmrMeasure->ticks());
    Segment* clefSeg = mmrMeasure->findSegmentR(SegmentType::Clef, mmrMeasure->ticks());
    Segment* breathSeg = mmrMeasure->findSegmentR(SegmentType::Breath, mmrMeasure->ticks());

    if (!remainingMMRDuration.negative() && remainingMMRDuration.isNotZero()) {
        // Time remaining. Create another mmrest to fill the time
        Measure* nextFirstMeasure = ctx.mutDom().tick2measure(lastMeasure->endTick());
        if (!nextFirstMeasure->mmRest()) {
            Measure* nextLastMeasure = ctx.mutDom().tick2measure(lastMeasure->endTick() + remainingMMRDuration - Fraction::eps());
            nextLastMeasure = nextLastMeasure ? nextLastMeasure : ctx.mutDom().lastMeasure();
            const int numMeasuresInNewMMRest = nextLastMeasure->measureIndex() - nextFirstMeasure->measureIndex() + 1;

            Measure* nextMMRMeasure = Factory::createMeasure(ctx.mutDom().dummyParent()->system());
            nextMMRMeasure->setTicks(remainingMMRDuration);
            nextMMRMeasure->setTick(nextFirstMeasure->tick());
            ctx.mutDom().undo(new ChangeMMRest(nextFirstMeasure, nextMMRMeasure));

            nextMMRMeasure->setTimesig(nextFirstMeasure->timesig());
            nextMMRMeasure->setRepeatCount(nextLastMeasure->repeatCount());
            nextMMRMeasure->setMMRestCount(numMeasuresInNewMMRest);
            nextMMRMeasure->setMeasureNumber(nextFirstMeasure->measureNumber());

            ctx.mutDom().updateLocksOnCreateMMRest(nextFirstMeasure, nextLastMeasure);

            nextMMRMeasure->setRepeatStart(nextFirstMeasure->repeatStart() || nextLastMeasure->repeatStart());
            nextMMRMeasure->setRepeatEnd(nextFirstMeasure->repeatEnd() || nextLastMeasure->repeatEnd());

            if (barLineSeg) {
                changeElementsParent(barLineSeg, nextMMRMeasure, remainingMMRDuration, ctx, false);
            }
            if (clefSeg) {
                changeElementsParent(clefSeg, nextMMRMeasure, remainingMMRDuration, ctx, false);
            }
            if (breathSeg) {
                changeElementsParent(breathSeg, nextMMRMeasure, remainingMMRDuration, ctx, false);
            }
        }
    }

    // adjust length
    mmrMeasure->setTicks(len);
    // move existing end barline, clef and breath
    if (barLineSeg) {
        barLineSeg->setRtick(len);
    }
    if (clefSeg) {
        clefSeg->setRtick(len);
    }
    if (breathSeg) {
        breathSeg->setRtick(len);
    }
    MeasureLayout::removeSystemTrailer(mmrMeasure, ctx);
}

//---------------------------------------------------------
//   createMMRest
//    create a multimeasure rest
//    from firstMeasure to lastMeasure (inclusive)
//---------------------------------------------------------

void MMRestLayout::createMMRest(LayoutContext& ctx, Measure* firstMeasure, Measure* lastMeasure, const Fraction& len)
{
    int numMeasuresInMMRest = 1;
    if (firstMeasure != lastMeasure) {
        for (Measure* m = firstMeasure->nextMeasure(); m; m = m->nextMeasure()) {
            ++numMeasuresInMMRest;
            m->setMMRestCount(0);
            if (Measure* oldMMRest = m->mmRest()) {
                ctx.mutDom().undo(new ChangeMMRest(m, nullptr));
                // These MMRests are being removed. Move all elements back to underlying measures.
                // Any elements that need moving back to the MMRest (in lastUnderlyingMeasure) will be handled by changeMeasureElParents below
                Measure* lastUnderlyingMeasure = oldMMRest->mmRestLast();
                restoreMeasureElParents(m, lastUnderlyingMeasure, oldMMRest, ctx);
            }
            if (m == lastMeasure) {
                break;
            }
        }
    }

    // mmrMeasure coexists with n undisplayed measures of rests
    Measure* mmrMeasure = firstMeasure->mmRest();
    if (mmrMeasure) {
        reuseExistingMMRest(ctx, mmrMeasure, lastMeasure, len);
    } else {
        mmrMeasure = Factory::createMeasure(ctx.mutDom().dummyParent()->system());
        mmrMeasure->setTicks(len);
        mmrMeasure->setTick(firstMeasure->tick());
        ctx.mutDom().undo(new ChangeMMRest(firstMeasure, mmrMeasure));
    }
    mmrMeasure->setTimesig(firstMeasure->timesig());
    mmrMeasure->setRepeatCount(lastMeasure->repeatCount());
    mmrMeasure->setMMRestCount(numMeasuresInMMRest);
    mmrMeasure->setMeasureNumber(firstMeasure->measureNumber());

    ctx.mutDom().updateLocksOnCreateMMRest(firstMeasure, lastMeasure);

    mmrMeasure->setRepeatStart(firstMeasure->repeatStart() || lastMeasure->repeatStart());
    mmrMeasure->setRepeatEnd(firstMeasure->repeatEnd() || lastMeasure->repeatEnd());

    Segment* chordRestSeg = mmrMeasure->undoGetSegmentR(SegmentType::ChordRest, Fraction(0, 1));
    for (size_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
        track_idx_t track = staffIdx * VOICES;
        if (chordRestSeg->element(track) == 0) {
            MMRest* mmr = Factory::createMMRest(chordRestSeg);
            mmr->setDurationType(DurationType::V_MEASURE);
            mmr->setTicks(mmrMeasure->ticks());
            mmr->setTrack(track);
            mmr->setParent(chordRestSeg);
            ctx.mutDom().doUndoAddElement(mmr);
        }
    }

    changeMeasureElParents(firstMeasure, lastMeasure, mmrMeasure, ctx);

    MeasureBase* nm = ctx.conf().isShowVBox() ? lastMeasure->next() : lastMeasure->nextMeasure();
    ctx.mutDom().undo(new ChangeMMRestNext(mmrMeasure, nm));
    ctx.mutDom().undo(new ChangeMMRestPrev(mmrMeasure, firstMeasure->prev()));
}

void MMRestLayout::changeMeasureElParents(Measure* firstMeasure, Measure* lastMeasure, Measure* mmrMeasure, LayoutContext& ctx)
{
    // copy markers & jumps to mmrMeasure
    // jumps come from lastMeasure, markers come from firstMeasure
    ElementList newList = lastMeasure->el();
    for (EngravingItem* e : firstMeasure->el()) {
        if (e->isMarker() && firstMeasure != lastMeasure) {
            newList.push_back(e);
        }
    }
    for (EngravingItem* e : newList) {
        ctx.mutDom().undoChangeParent(e, mmrMeasure, e->staffIdx(), false);
    }

    // set mmrMeasure with same barline as last underlying measure
    Segment* lastMeasureEndBarlineSeg = lastMeasure->findSegmentR(SegmentType::EndBarLine, lastMeasure->ticks());
    changeElementsParent(lastMeasureEndBarlineSeg, mmrMeasure, mmrMeasure->ticks(), ctx, false);

    // if last underlying measure ends with clef change, show same at end of mmrest
    Segment* lastMeasureClefSeg = lastMeasure->findSegmentR(SegmentType::Clef | SegmentType::HeaderClef,
                                                            lastMeasure->ticks());
    changeElementsParent(lastMeasureClefSeg, mmrMeasure, mmrMeasure->ticks(), ctx, false);

    // further check for clefs
    Segment* underlyingClefSeg = lastMeasure->findSegmentR(SegmentType::Clef, lastMeasure->ticks());
    Segment* mmrClefSeg = changeElementsParent(underlyingClefSeg, mmrMeasure, mmrMeasure->ticks(), ctx, false);

    if (underlyingClefSeg && mmrClefSeg) {
        mmrClefSeg->setEnabled(underlyingClefSeg->enabled());
        mmrClefSeg->setTrailer(underlyingClefSeg->trailer());
    }

    // check for time signature
    Segment* underlyingTimeSigSeg = firstMeasure->findSegmentR(SegmentType::TimeSig, Fraction(0, 1));
    Segment* mmrTimeSigSeg = changeElementsParent(underlyingTimeSigSeg, mmrMeasure, Fraction(0, 1), ctx, false);

    if (underlyingTimeSigSeg && mmrTimeSigSeg) {
        mmrTimeSigSeg->setEnabled(underlyingTimeSigSeg->enabled());
        mmrTimeSigSeg->setHeader(underlyingTimeSigSeg->header());
    }

    // check for end of measure time signature
    Segment* underlyingEndTimeSigSeg = lastMeasure->findSegmentR(SegmentType::TimeSig, lastMeasure->ticks());
    Segment* mmrEndTimeSigSeg = changeElementsParent(underlyingEndTimeSigSeg, mmrMeasure, mmrMeasure->ticks(), ctx, false);

    if (underlyingEndTimeSigSeg && mmrEndTimeSigSeg) {
        mmrEndTimeSigSeg->setEnabled(underlyingEndTimeSigSeg->enabled());
        mmrEndTimeSigSeg->setHeader(underlyingEndTimeSigSeg->header());
        mmrEndTimeSigSeg->setEndOfMeasureChange(underlyingEndTimeSigSeg->endOfMeasureChange());
    }

    // check for end of measure breaths
    Segment* underlyingBreathSeg = lastMeasure->findSegmentR(SegmentType::Breath, lastMeasure->ticks());
    changeElementsParent(underlyingBreathSeg, mmrMeasure, mmrMeasure->ticks(), ctx, false);

    // check for ambitus
    Segment* underlyingAmbitusSeg = firstMeasure->findSegmentR(SegmentType::Ambitus, Fraction(0, 1));
    changeElementsParent(underlyingAmbitusSeg, mmrMeasure, Fraction(0, 1), ctx, false);

    // check for key signature
    Segment* underlyingKeySigSeg = firstMeasure->findSegmentR(SegmentType::KeySig, Fraction(0, 1));
    Segment* mmrKeySigSeg = changeElementsParent(underlyingKeySigSeg, mmrMeasure, Fraction(0, 1), ctx, false);

    if (underlyingKeySigSeg && mmrKeySigSeg) {
        mmrKeySigSeg->setEnabled(underlyingKeySigSeg->enabled());
        mmrKeySigSeg->setHeader(underlyingKeySigSeg->header());
    }

    mmrMeasure->checkHeader();
    mmrMeasure->checkTrailer();

    // check for rehearsal mark etc.
    Segment* mmrChordRestSeg = mmrMeasure->undoGetSegmentR(SegmentType::ChordRest, Fraction(0, 1));
    Segment* underlyingCRSeg = firstMeasure->findSegmentR(SegmentType::ChordRest, Fraction(0, 1));
    changeAnnotationsParent(underlyingCRSeg, mmrChordRestSeg, ctx, false);
}

void MMRestLayout::restoreMeasureElParents(Measure* firstMeasure, Measure* lastMeasure, Measure* mmrMeasure, LayoutContext& ctx)
{
    // copy markers & jumps back to underlying measures
    ElementList mmrMeasureEls = mmrMeasure->el();
    for (EngravingItem* e : mmrMeasureEls) {
        Measure* newMeasure = e->isMarker() ? firstMeasure : lastMeasure;
        ctx.mutDom().undoChangeParent(e, newMeasure, e->staffIdx(), false);
    }

    // restore the barline from mmrMeasure to the last underlying measure
    Segment* mmrEndBarlineSeg = mmrMeasure->findSegmentR(SegmentType::EndBarLine, mmrMeasure->ticks());
    changeElementsParent(mmrEndBarlineSeg, lastMeasure, lastMeasure->ticks(), ctx, false);

    // restore clef segments from mmrMeasure to the last underlying measure
    Segment* mmrClefOrHeaderSeg = mmrMeasure->findSegmentR(SegmentType::Clef | SegmentType::HeaderClef, mmrMeasure->ticks());
    changeElementsParent(mmrClefOrHeaderSeg, lastMeasure, lastMeasure->ticks(), ctx, false);

    Segment* mmrClefSeg = mmrMeasure->findSegmentR(SegmentType::Clef, mmrMeasure->ticks());
    Segment* lastMeasureClefSeg = changeElementsParent(mmrClefSeg, lastMeasure, lastMeasure->ticks(), ctx, false);

    if (mmrClefSeg && lastMeasureClefSeg) {
        lastMeasureClefSeg->setEnabled(mmrClefSeg->enabled());
        lastMeasureClefSeg->setTrailer(mmrClefSeg->trailer());
    }

    // restore time signature segments from mmrMeasure to the first underlying measure
    Segment* mmrTimeSigSeg = mmrMeasure->findSegmentR(SegmentType::TimeSig, Fraction(0, 1));
    Segment* firstMeasureTimeSigSeg = changeElementsParent(mmrTimeSigSeg, firstMeasure, Fraction(0, 1), ctx, false);

    if (mmrTimeSigSeg && firstMeasureTimeSigSeg) {
        firstMeasureTimeSigSeg->setEnabled(mmrTimeSigSeg->enabled());
        firstMeasureTimeSigSeg->setHeader(mmrTimeSigSeg->header());
    }

    // restore end-of-measure time signature segment from mmrMeasure to the last underlying measure
    Segment* mmrEndTimeSigSeg = mmrMeasure->findSegmentR(SegmentType::TimeSig, mmrMeasure->ticks());
    Segment* lastMeasureEndTimeSigSeg = changeElementsParent(mmrEndTimeSigSeg, lastMeasure, lastMeasure->ticks(), ctx, false);

    if (mmrEndTimeSigSeg && lastMeasureEndTimeSigSeg) {
        lastMeasureEndTimeSigSeg->setEnabled(mmrEndTimeSigSeg->enabled());
        lastMeasureEndTimeSigSeg->setHeader(mmrEndTimeSigSeg->header());
        lastMeasureEndTimeSigSeg->setEndOfMeasureChange(mmrEndTimeSigSeg->endOfMeasureChange());
    }

    // restore breath segment from mmrMeasure to the last underlying measure
    Segment* mmrBreathSeg = mmrMeasure->findSegmentR(SegmentType::Breath, mmrMeasure->ticks());
    changeElementsParent(mmrBreathSeg, lastMeasure, lastMeasure->ticks(), ctx, false);

    // restore ambitus segment from mmrMeasure to the first underlying measure
    Segment* mmrAmbitusSeg = mmrMeasure->findSegmentR(SegmentType::Ambitus, Fraction(0, 1));
    changeElementsParent(mmrAmbitusSeg, firstMeasure, Fraction(0, 1), ctx, false);

    // restore key signature segment from mmrMeasure to the first underlying measure
    Segment* mmrKeySigSeg = mmrMeasure->findSegmentR(SegmentType::KeySig, Fraction(0, 1));
    Segment* firstMeasureKeySigSeg = changeElementsParent(mmrKeySigSeg, firstMeasure, Fraction(0, 1), ctx, false);

    if (mmrKeySigSeg && firstMeasureKeySigSeg) {
        firstMeasureKeySigSeg->setEnabled(mmrKeySigSeg->enabled());
        firstMeasureKeySigSeg->setHeader(mmrKeySigSeg->header());
    }

    firstMeasure->checkHeader();
    lastMeasure->checkTrailer();

    // restore annotations from the mmr chord/rest segment back to the first underlying measure
    Segment* mmrChordRestSeg = mmrMeasure->undoGetSegmentR(SegmentType::ChordRest, Fraction(0, 1));
    Segment* underlyingCRSeg = firstMeasure->findSegmentR(SegmentType::ChordRest, Fraction(0, 1));
    changeAnnotationsParent(mmrChordRestSeg, underlyingCRSeg, ctx, false);
}

void MMRestLayout::changeAnnotationsParent(Segment* oldParent, Segment* newParent, const LayoutContext& ctx, bool checkVisibility)
{
    if (!oldParent || !newParent) {
        return;
    }
    std::vector<EngravingItem*> annotations = oldParent->annotations();
    for (EngravingItem* e : annotations) {
        // look at elements in underlying measure
        if (!muse::contains(BREAK_TYPES, e->type())) {
            continue;
        }
        if (checkVisibility && (!e->visible() || !ctx.dom().staff(e->staffIdx())->show())) {
            continue;
        }

        e->score()->undoChangeParent(e, newParent, e->staffIdx(), false);
    }
}

Segment* MMRestLayout::changeElementsParent(Segment* oldSeg, Measure* newMeasure, const Fraction& newSegTick,
                                            LayoutContext& ctx, bool checkVisibility)
{
    // Moves elements in oldSeg to newSeg at newSegTick in newMeasure. Creates a new segment if no new seg exists
    if (!oldSeg) {
        return nullptr;
    }
    Segment* newSeg = newMeasure->undoGetSegmentR(oldSeg->segmentType(), newSegTick);
    for (size_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
        const track_idx_t track = staff2track(staffIdx);
        EngravingItem* el = oldSeg->element(track);
        if (!el || !el->visible()) {
            continue;
        }
        if (checkVisibility && (!el->visible() || !ctx.dom().staff(staffIdx)->show())) {
            continue;
        }
        ctx.mutDom().undoChangeParent(el, newSeg, el->staffIdx(), false);
    }

    changeAnnotationsParent(oldSeg, newSeg, ctx, checkVisibility);

    return newSeg;
}

//---------------------------------------------------------
// validMMRestMeasure
//    return true if this might be a measure in a
//    multi measure rest
//---------------------------------------------------------

bool MMRestLayout::validMMRestMeasure(const LayoutContext& ctx, const Measure* m)
{
    if (m->excludeFromNumbering()) {
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
            if (e->staffIdx() >= nstaves || !e->staff()->show() || !e->visible()) {
                continue;
            }
            if (muse::contains(BREAK_TYPES, e->type()) && !s->rtick().isZero()) {
                // play count text and fermatas are permitted at the end of a measure
                if (!e->isPlayCountText() && !e->isFermata()) {
                    return false;
                }
            }
        }
        size_t tracks = ctx.dom().ntracks();
        if (!s->isChordRestType()) {
            continue;
        }
        bool restFound = false;
        for (track_idx_t track = 0; track < tracks; ++track) {
            if ((track % VOICES) == 0 && !ctx.dom().staff(track / VOICES)->show()) {
                track += VOICES - 1;
                continue;
            }
            if (!s->element(track)) {
                continue;
            }
            if (!s->element(track)->isRest()) {
                return false;
            } else {
                bool isNonEmptyIrregular = m->isIrregular() && !toRest(s->element(track))->isFullMeasureRest();
                if (isNonEmptyIrregular) {
                    return false;
                }
            }
            restFound = true;
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
    return true;
}

bool MMRestLayout::breakMMRForElement(Measure* measure, Measure* prevMeasure, const LayoutContext& ctx)
{
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

    // break for marker in this measure
    for (EngravingItem* e : measure->el()) {
        if (e->isMarker()) {
            Marker* mark = toMarker(e);
            if (!(mark->align() == AlignH::RIGHT)) {
                return true;
            }
        }
    }

    for (Segment* s = measure->first(); s; s = s->next()) {
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
            if (s->isType(SegmentType::KeySig | SegmentType::TimeSig) && measure->tick().isNotZero()) {
                return true;
            }
            if (s->isClefType()) {
                if (s->tick() != measure->endTick() && measure->tick().isNotZero()) {
                    return true;
                }
            }
        }
    }

    if (!prevMeasure) {
        return false;
    }

    // break for marker & jump in previous measure
    for (EngravingItem* e : prevMeasure->el()) {
        if (e->isJump()) {
            return true;
        } else if (e->isMarker()) {
            Marker* mark = toMarker(e);
            if (mark->align() == AlignH::RIGHT) {
                return true;
            }
        }
    }

    // Break for breaths and annotations found mid-way into the previous measure
    for (Segment* s = prevMeasure->first(); s; s = s->next()) {
        for (size_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
            if (!ctx.dom().staff(staffIdx)->show()) {
                continue;
            }
            EngravingItem* e = s->element(staffIdx * VOICES);
            if (!e || e->generated()) {
                continue;
            }
            if (s->isBreathType()) {
                return true;
            }
        }

        for (EngravingItem* e : s->annotations()) {
            if (!e->visible()) {
                continue;
            }
            if (e->rtick() == Fraction(0, 1)) {
                continue;
            }
            if (breakForAnnotation(e) || e->isFermata()) {
                return true;
            }
        }
    }

    // Break for non-normal end barline in previous measure
    Segment* endBarSeg = prevMeasure->findSegmentR(SegmentType::EndBarLine, prevMeasure->ticks());
    if (endBarSeg) {
        for (size_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
            BarLine* bl = toBarLine(endBarSeg->element(staffIdx * VOICES));
            if (!bl) {
                continue;
            }
            BarLineType t = bl->barLineType();
            if (t != BarLineType::NORMAL && t != BarLineType::BROKEN && t != BarLineType::DOTTED && !bl->generated()) {
                return true;
            } else {
                break;
            }
        }
    }

    // Break for courtesy clefs/sigs at the end of the previous measure,
    // only if the element is on a visible staff
    auto hasVisibleElement = [&ctx](Segment* seg) -> bool {
        for (size_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
            if (!ctx.dom().staff(staffIdx)->show()) {
                continue;
            }
            EngravingItem* e = seg->element(staffIdx * VOICES);
            if (e && !e->generated()) {
                return true;
            }
        }
        return false;
    };

    if (Segment* clefSeg = prevMeasure->findSegment(SegmentType::Clef, measure->tick())) {
        if (hasVisibleElement(clefSeg)) {
            return true;
        }
    }
    if (Segment* tsSeg = prevMeasure->findSegment(SegmentType::TimeSigTypes, measure->tick())) {
        if (hasVisibleElement(tsSeg)) {
            return true;
        }
    }
    if (Segment* ksSeg = prevMeasure->findSegment(SegmentType::KeySigTypes, measure->tick())) {
        if (hasVisibleElement(ksSeg)) {
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------
//  breakMultiMeasureRest
//    return true if this measure should start a new
//    multi measure rest
//---------------------------------------------------------

bool MMRestLayout::breakMultiMeasureRest(const LayoutContext& ctx, Measure* m)
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

    bool breakRest = breakMMRForElement(m, prevMeas, ctx);

    Measure* mmrest = m->mmRest();
    Measure* mmrestPrev = mmrest ? m->mmRest()->prevMeasureMM() : nullptr;

    // If we are creating MMRests from expanded notation, checking underlying measures is sufficient.
    // If we are updating MMRests which are already being shown (any layout update) we must also check that the elements dictating their boundaries are still present
    // This is because elements are moved between underlying and covering measures
    if (mmrest && (mmrest->mmRestFirst() == m || mmrest->mmRestLast() == m)) {
        breakRest |= breakMMRForElement(mmrest, mmrestPrev, ctx);
    }

    return breakRest;
}

void MMRestLayout::createMultiMeasureRestsIfNeed(Measure* firstMeasure, LayoutContext& ctx)
{
    LAYOUT_CALL() << LAYOUT_ITEM_INFO(firstMeasure);

    if (ctx.dom().nstaves() == 0) {
        return;
    }

    int mn = ctx.state().measureNumber();

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
                int measureNumber = MeasureLayout::adjustMeasureNumber(measureToBeChecked, ctx.state().measureNumber());
                ctx.mutState().setMeasureNumber(measureNumber);
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
                removeMMRestElements(firstMeasure->mmRest(), ctx);
                ctx.mutDom().undo(new ChangeMMRest(firstMeasure, nullptr));
            }
            ctx.mutState().setMeasureNumber(mn);
        }
    } else if (firstMeasure->mmRest()) {
        removeMMRestElements(firstMeasure->mmRest(), ctx);

        if (firstMeasure->mmRestCount() > 0) {
            LOGD("mmrest: measureNumber %d += %d", ctx.state().measureNumber(), firstMeasure->mmRestCount());
            int measureNumber = ctx.state().measureNumber() + firstMeasure->mmRestCount() - 1;
            ctx.mutState().setMeasureNumber(measureNumber);
        }
    }
}

void MMRestLayout::removeMMRestElements(Measure* mmrMeasure, LayoutContext& ctx)
{
    // Move elements from MMRest measure back to underlying
    Measure* firstMeasure = mmrMeasure->mmRestFirst();
    Measure* lastMeasure = mmrMeasure->mmRestLast();
    IF_ASSERT_FAILED(firstMeasure && lastMeasure) {
        return;
    }

    restoreMeasureElParents(firstMeasure, lastMeasure, mmrMeasure, ctx);
}

void MMRestLayout::layoutMMRestRange(Measure* m, LayoutContext& ctx)
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
        s = String(u"%1–%2").arg(m->measureNumber() + 1).arg(m->measureNumber() + m->mmRestCount());
    } else {
        // If the minimum range to create a mmrest is set to 1,
        // then simply show the measure number as there is no range
        s = String::number(m->measureNumber() + 1);
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
