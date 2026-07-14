/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited and others
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
#include "dom/part.h"
#include "dom/parenthesis.h"
#include "dom/spacer.h"
#include "dom/score.h"
#include "dom/stafflines.h"
#include "dom/system.h"
#include "dom/tie.h"
#include "dom/timesig.h"
#include "dom/tremolosinglechord.h"
#include "dom/tremolotwochord.h"
#include "dom/utils.h"

#include "editing/addremoveelement.h"
#include "editing/editkeysig.h"
#include "editing/editmeasures.h"
#include "editing/editproperty.h"
#include "types/typesconv.h"

#include "arpeggiolayout.h"
#include "beamlayout.h"
#include "chordlayout.h"
#include "horizontalspacing.h"
#include "layoutcontext.h"
#include "mmrestlayout.h"
#include "modifydom.h"
#include "parenthesislayout.h"
#include "restlayout.h"
#include "segmentlayout.h"
#include "slurtielayout.h"
#include "tlayout.h"
#include "tremololayout.h"

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

    for (size_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
        if (Spacer* sp = item->vspacerDown(staffIdx)) {
            TLayout::layoutSpacer(sp, ctx);
            double y = item->system()->staff(staffIdx)->y() + sp->staffOffsetY();
            sp->setPos(item->spatium() * .5, y + sp->staff()->staffHeight(item->tick()));
        }
        if (Spacer* sp = item->vspacerUp(staffIdx)) {
            TLayout::layoutSpacer(sp, ctx);
            double y = item->system()->staff(staffIdx)->y() + sp->staffOffsetY();
            sp->setPos(item->spatium() * .5, y - sp->absoluteGap());
        }
    }

    // layout LAYOUT_BREAK elements
    TLayout::layoutBaseMeasureBase(item, item->mutldata(), ctx);

    //---------------------------------------------------
    //    layout cross-staff ties
    //---------------------------------------------------

    auto layoutCrossStaffTies = [](Chord* chord, Measure* item, const Fraction& stick, LayoutContext& ctx) ->void {
        for (Note* note : chord->notes()) {
            Tie* tieFor = note->tieFor();
            Tie* tieBack = note->tieBack();
            LaissezVib* lv = note->laissezVib();
            if (lv && lv->isCrossStaff()) {
                SlurTieLayout::layoutLaissezVibChord(chord, ctx);
            }
            if (tieFor && !lv && tieFor->isCrossStaff()) {
                SlurTieLayout::layoutTieFor(tieFor, item->system());
            }
            if (tieBack && tieBack->tick() < stick && tieBack->isCrossStaff()) {
                SlurTieLayout::layoutTieBack(tieBack, item->system(), ctx);
            }
        }
    };

    const Fraction stick = item->system()->measures().front()->tick();
    const size_t tracks = ctx.dom().ntracks();
    static const SegmentType st { SegmentType::ChordRest };
    for (track_idx_t track = 0; track < tracks; ++track) {
        if (!ctx.dom().staff(track2staff(track))->show()) {
            track += VOICES - 1;
            continue;
        }
        for (Segment* seg = item->first(st); seg; seg = seg->next(st)) {
            EngravingItem* element = seg->element(track);
            if (!element || !element->isChord()) {
                continue;
            }
            Chord* chord = toChord(element);
            layoutCrossStaffTies(chord, item, stick, ctx);

            for (Chord* graceChord : chord->graceNotes()) {
                layoutCrossStaffTies(graceChord, item, stick, ctx);
            }
        }
    }
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
    if (!currentMB || !currentMB->isMeasure()) {
        return;
    }

    Measure* measure = toMeasure(currentMB);

    if (!measure->ldata()->needLayout()) {
        return;
    }

    if (ctx.conf().isLinearMode() && (measure->tick() < ctx.state().startTick() || measure->tick() > ctx.state().endTick())) {
        return;
    }

    if (ctx.dom().allStavesInvisible()) {
        return;
    }

    measure->mutldata()->setNeedLayout(false);

    // Check if requested cross-staff is possible
    // This must happen before cmdUpdateNotes
    checkStaffMoveValidity(measure, ctx);

    // ---- Modify DOM ----
    ModifyDom::setCrossMeasure(measure, ctx);
    ModifyDom::connectTremolo(measure);
    ModifyDom::cmdUpdateNotes(measure, ctx.dom());
    ModifyDom::createStems(measure,  ctx);
    ModifyDom::setTrackForChordGraceNotes(measure, ctx.dom());
    ModifyDom::sortMeasureSegments(measure, ctx);
    // --------------------
    //
    // calculate accidentals and note lines,
    // create stem and set stem direction
    //

    const DomAccessor& dom = ctx.dom();
    const LayoutConfiguration& conf = ctx.conf();

    for (size_t staffIdx = 0; staffIdx < dom.nstaves(); ++staffIdx) {
        const Staff* staff = dom.staff(staffIdx);

        track_idx_t startTrack = staffIdx * VOICES;
        track_idx_t endTrack  = startTrack + VOICES;

        for (const Segment& segment : measure->segments()) {
            if (!staff->show() && !segment.isType(SegmentType::TimeSigTypes)) {
                continue;
            }

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
        for (EngravingItem* item : s.elist()) {
            if (!item || !item->isChordRest()) {
                continue;
            }
            if (const Staff* staff = ctx.dom().staff(item->vStaffIdx()); staff && staff->show()) {
                BeamLayout::layoutNonCrossBeams(toChordRest(item), ctx);
            }
        }
    }

    for (staff_idx_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
        const Staff* staff = ctx.dom().staff(staffIdx);
        if (!staff->show()) {
            continue;
        }

        for (Segment& segment : measure->segments()) {
            if (segment.isChordRestType()) {
                ChordLayout::layoutChords1(ctx, &segment, staffIdx);
                RestLayout::resolveVerticalRestConflicts(ctx, &segment, staffIdx);
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
                if (e->isSymbol() || e->isHarmony() || e->isFretDiagram()) {
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
            if (el && el->isChordRest() && !toChordRest(el)->graceNotes().empty()) {
                ChordLayout::appendGraceNotes(toChordRest(el));
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

    updateTicksMeasNumbersAndMMRests(ctx.mutState().curMeasure(), ctx);

    MeasureBase* mb = ctx.mutState().curMeasure();
    if (mb && mb->isMeasure()) {
        toMeasure(mb)->mutldata()->setNeedLayout(true);
    }
}

void MeasureLayout::updateTicksMeasNumbersAndMMRests(MeasureBase* currentMB, LayoutContext& ctx)
{
    IF_ASSERT_FAILED(currentMB == ctx.state().curMeasure()) {
        return;
    }

    if (!currentMB) {
        return;
    }

    if (!currentMB->isMeasure()) {
        assert(currentMB->tick() == ctx.state().tick());

        const LayoutBreak* layoutBreak = currentMB->sectionBreakElement();
        if (layoutBreak && layoutBreak->startWithMeasureOne()) {
            ctx.mutState().setMeasureNumber(0);
        }

        return;
    }

    Measure* measure = toMeasure(currentMB);

    int measureNumber = adjustMeasureNumber(measure, ctx.state().measureNumber());

    ctx.mutState().setMeasureNumber(measureNumber);

    MMRestLayout::createMultiMeasureRestsIfNeed(measure, ctx);

    currentMB = ctx.mutState().curMeasure();
    measure = toMeasure(currentMB);

    measure->moveTicks(ctx.state().tick() - measure->tick());

    ctx.mutState().setTick(ctx.state().tick() + measure->ticks());
}

//---------------------------------------------------------
//   adjustMeasureNumber
//---------------------------------------------------------

int MeasureLayout::adjustMeasureNumber(Measure* m, int measureNumber)
{
    measureNumber += m->measureNumberOffset();
    m->setMeasureNumber(measureNumber);
    if (!m->excludeFromNumbering()) {
        ++measureNumber;
    }

    const LayoutBreak* layoutBreak = m->sectionBreakElement();
    if (layoutBreak && layoutBreak->startWithMeasureOne()) {
        measureNumber = 0;
    }

    return measureNumber;
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
            ChordLayout::layoutArticulations2(chord, ctx);
            chord->computeKerningExceptions();
        }

        for (EngravingItem* annotation : seg.annotations()) {
            if (annotation->isFermata()) {
                TLayout::layoutItem(annotation, ctx);
            }
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
                                  ? m->width() - clefSeg->x() + clefSeg->minLeft() + ctx.conf().styleAbsolute(Sid::clefLeftMargin)
                                  * staffMag
                                  : 0.0;
            layoutPartialWidth(ms->lines(), ctx, m->width(), partialWidth / (m->spatium() * staffMag), true);
        } else {
            // normal staff lines
            TLayout::layoutStaffLines(ms->lines(), ctx);
        }
        staffIdx += 1;
    }
}

void MeasureLayout::layoutPlayCountText(Measure* m, LayoutContext& ctx)
{
    if (!m->repeatEnd()) {
        return;
    }

    Score* score = m->score();
    const std::vector<MStaff*>& measureStaves = m->mstaves();

    for (staff_idx_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
        if (staffIdx >= measureStaves.size()) {
            break;
        }
        track_idx_t track = staff2track(staffIdx);

        Segment* endBarSeg = m->last(SegmentType::BarLineTypes);
        PlayCountText* playCount
            = endBarSeg ? toPlayCountText(endBarSeg->findAnnotation(ElementType::PLAY_COUNT_TEXT, track, track)) : nullptr;
        if (!playCount) {
            continue;
        }

        String text;
        const int repeatCount = m->repeatCount();
        String defaultText = TConv::translatedUserName(ctx.conf().styleV(Sid::repeatPlayCountPreset).value<RepeatPlayCountPreset>()).arg(
            repeatCount);

        switch (playCount->playCountTextSetting()) {
        case AutoCustomHide::AUTO:
            text = defaultText;
            break;
        case AutoCustomHide::CUSTOM:
            text = playCount->playCountCustomText();
            if (text.empty()) {
                text = defaultText;
            }
            break;
        case AutoCustomHide::HIDE:
            playCount->mutldata()->setIsSkipDraw(true);
            break;
        }

        if (!playCount->cursor()->editing()) {
            playCount->setXmlText(text);
        }
    }
}

void MeasureLayout::layoutMeasureNumber(Measure* m, LayoutContext& ctx)
{
    MeasureNumberPlacement placementMode = ctx.conf().styleV(Sid::measureNumberPlacementMode).value<MeasureNumberPlacement>();

    String stringNum = String::number(m->measureNumber() + 1);

    Score* score = m->score();

    const std::vector<MStaff*>& measureStaves = m->mstaves();

    for (staff_idx_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
        if (staffIdx >= measureStaves.size()) {
            break;
        }

        if (m->showMeasureNumberOnStaff(staffIdx)) {
            MeasureNumber* measureNumber = measureStaves[staffIdx]->measureNumber();
            if (!measureNumber) {
                measureNumber = new MeasureNumber(m);
                measureNumber->setTrack(staff2track(staffIdx));
                measureNumber->setGenerated(true);
                measureNumber->setParent(m);
                m->add(measureNumber);
            }

            measureNumber->setXmlText(stringNum);
            measureNumber->setSystemFlag(placementMode != MeasureNumberPlacement::ON_ALL_STAVES);
            TLayout::layoutMeasureNumber(measureNumber, measureNumber->mutldata(), ctx);
        }
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

    for (s2 = firstCrSeg->nextActive(); s2; s2 = s2->nextActive()) {
        if (modernMMRest && !s2->isChordRestType() && !s2->isBreathType() && s2->element(staffIdx * VOICES)) {
            break;
        }

        if (!modernMMRest
            && (s2->isClefRepeatAnnounceType() || (s2->isTimeSigRepeatAnnounceType() && !s2->hasTimeSigAboveStaves())
                || s2->isKeySigRepeatAnnounceType() || s2->isEndBarLineType()) && s2->element(staffIdx * VOICES)) {
            break;
        }
    }

    double x1 = 0.0;
    while (s1) {
        if (!s1->hasTimeSigAboveStaves() && !s1->allElementsInvisible()) {
            x1 = std::max(x1, s1->x() + s1->minRight());
            if (s1->isCourtesySegment()) {
                break;
            }
        }
        s1 = s1->prevActive();
    }

    double x2 = s2 ? s2->x() - s2->minLeft() : measure->width();

    bool headerException = measure->header() && firstCrSeg->prev() && !firstCrSeg->prev()->isStartRepeatBarLineType()
                           && needsHeaderException;
    if (headerException) { //needs this exception on header bar
        // Set x1 to the imaginary barline located the minimum barline->note distance to the left of the rest's segment
        x1 = firstCrSeg->x() - ctx.conf().styleAbsolute(Sid::barNoteDistance);
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
                    double d = ctx.conf().styleAbsolute(Sid::multiMeasureRestMargin);
                    double w = x2 - x1 - 2 * d;
                    MMRest::LayoutData* mmrestLD = mmrest->mutldata();
                    mmrestLD->restWidth = w;
                    TLayout::layoutMMRest(mmrest, mmrest->mutldata(), ctx);
                    mmrestLD->setPosX(x1 - s.x() + d);
                } else if (e->isMeasureRepeat() && !(toMeasureRepeat(e)->numMeasures() % 2)) {
                    // two- or four-measure repeat, center on following barline
                    double measureWidth = x2 - s.x() + .5 * (m->styleP(Sid::barWidth));
                    e->mutldata()->setPosX(measureWidth - .5 * e->width());
                    if (toMeasureRepeat(e)->numMeasures() == 4 && ctx.conf().styleB(Sid::fourMeasureRepeatShowExtenders)) {
                        TLayout::layoutMeasureRepeatExtender(toMeasureRepeat(e), toMeasureRepeat(e)->mutldata(), ctx);
                    }
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
    const size_t nstaves = ctx.dom().nstaves();
    Segment* barlineSeg = m->findSegmentR(SegmentType::EndBarLine, m->ticks());
    const Measure* nextMeasure = m->nextMeasure();
    double barlineWidth = 0.0;
    bool sectionBreakHideCourtesy = false;
    if (LayoutBreak* sectionBreakElement = m->sectionBreakElement()) {
        sectionBreakHideCourtesy = !sectionBreakElement->showCourtesy();
    }

    m->setHasCourtesyKeySig(false);
    //  Set flag "hasCourtesyKeySig" if this measure needs a courtesy key sig.
    //  This flag is later used to set a double end bar line and to actually
    //  create the courtesy key sig.

    bool hasKeySig = false;
    bool hasTimeSig = false;
    bool hasCourtesyTimeSig = false;
    if (nextMeasure && !sectionBreakHideCourtesy) {
        //  Don't change barlines at the end of a section break,
        //  and don't create courtesy key/time signatures.
        const bool showCourtesyKeySig = (isLastMeasureInSystem && ctx.conf().styleB(Sid::genCourtesyKeysig))
                                        || (m->repeatJump() && ctx.conf().styleB(Sid::showCourtesiesOtherJumps));

        Fraction tick = m->endTick();
        for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            const Staff* staff     = ctx.dom().staff(staffIdx);
            const KeySigEvent key1 = staff->keySigEvent(tick - Fraction::eps());
            const KeySigEvent key2 = staff->keySigEvent(tick);
            if (key1 == key2) {
                continue;
            }
            // locate a key sig. in next measure and, if found,
            // check if it has court. sig turned off
            Segment* keySigSeg = nextMeasure->findSegment(SegmentType::KeySig, tick);
            if (keySigSeg) {
                hasKeySig = true;
                KeySig* keySig = toKeySig(keySigSeg->element(staffIdx * VOICES));
                if (keySig && !keySig->showCourtesy()) {
                    continue;
                }
            }
            if (showCourtesyKeySig) {
                m->setHasCourtesyKeySig(true);
            }
            break;
        }

        bool showCourtesyTimeSig = (isLastMeasureInSystem && ctx.conf().styleB(Sid::genCourtesyTimesig))
                                   || (m->repeatJump() && ctx.conf().styleB(Sid::showCourtesiesOtherJumps));

        Segment* timeSigSeg = nextMeasure->findSegmentR(SegmentType::TimeSig, Fraction(0, 1));
        if (timeSigSeg) {
            for (track_idx_t track = 0; track < nstaves * VOICES; track += VOICES) {
                TimeSig* timeSig = toTimeSig(timeSigSeg->element(track));
                if (!timeSig) {
                    continue;
                }
                hasTimeSig = true;
                if (timeSig->showCourtesySig() && showCourtesyTimeSig) {
                    hasCourtesyTimeSig = true;
                }
                break;
            }
        }
    }

    bool nextMeasRepeat = nextMeasure && nextMeasure->repeatStart() && !m->repeatEnd() && !isLastMeasureInSystem
                          && m->next() == nextMeasure;

    bool barlineBeforeRepeatSigs = ctx.conf().styleB(Sid::barlineBeforeSigChange) && (hasTimeSig || hasKeySig);

    if (nextMeasRepeat && !barlineBeforeRepeatSigs) {
        // we may skip barline at end of a measure immediately before a start repeat:
        // next measure is repeat start, this measure is not a repeat end,
        // this is not last measure of system, no intervening frame
        if (!barlineSeg) {
            return;
        }
        barlineSeg->setEnabled(false);
    } else {
        BarLineType blType = nextMeasure ? BarLineType::NORMAL : BarLineType::END;
        if (!barlineSeg) {
            barlineSeg = m->getSegmentR(SegmentType::EndBarLine, m->ticks());
        }
        barlineSeg->setEnabled(true);

        const bool isRepeatSigBarline = ctx.conf().styleB(Sid::barlineBeforeSigChange) && nextMeasRepeat;

        const int keySigBarlineMode = ctx.conf().styleI(Sid::keySigCourtesyBarlineMode);
        const bool beforeKSCourtesy = keySigBarlineMode == int(CourtesyBarlineMode::DOUBLE_BEFORE_COURTESY) && m->hasCourtesyKeySig();
        const bool alwaysDoubleKS = keySigBarlineMode == int(CourtesyBarlineMode::ALWAYS_DOUBLE) && hasKeySig;
        const bool beforeRepeatKS = isRepeatSigBarline && hasKeySig && ctx.conf().styleB(Sid::doubleBarlineBeforeKeySig);
        if (beforeKSCourtesy || alwaysDoubleKS || beforeRepeatKS) {
            blType = BarLineType::DOUBLE;
        }

        const int timeSigBarlineMode = ctx.conf().styleI(Sid::timeSigCourtesyBarlineMode);
        const bool beforeTSCourtesy = timeSigBarlineMode == int(CourtesyBarlineMode::DOUBLE_BEFORE_COURTESY) && hasCourtesyTimeSig;
        const bool alwaysDoubleTS = timeSigBarlineMode == int(CourtesyBarlineMode::ALWAYS_DOUBLE) && hasTimeSig;
        const bool beforeRepeatTS = isRepeatSigBarline && hasTimeSig && ctx.conf().styleB(Sid::doubleBarlineBeforeTimeSig);

        if (beforeTSCourtesy || alwaysDoubleTS || beforeRepeatTS) {
            blType = BarLineType::DOUBLE;
        }

        bool force = false;
        if (m->repeatEnd()) {
            blType = BarLineType::END_REPEAT;
            force = true;
        }

        for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            track_idx_t track = staffIdx * VOICES;
            BarLine* barLine  = toBarLine(barlineSeg->element(track));
            const Staff* staff = ctx.dom().staff(staffIdx);
            if (!barLine) {
                barLine = Factory::createBarLine(barlineSeg);
                barLine->setParent(barlineSeg);
                barLine->setTrack(track);
                barLine->setGenerated(true);
                barLine->setSpanStaff(staff->barLineSpan());
                barLine->setSpanFrom(staff->barLineFrom());
                barLine->setSpanTo(staff->barLineTo());
                barLine->setBarLineType(blType);
                ctx.mutDom().addElement(barLine);
            } else {
                // do not change bar line type if bar line is user modified
                // and its not a repeat start/end barline (forced)

                if (barLine->generated()) {
                    barLine->setSpanStaff(staff->barLineSpan());
                    barLine->setSpanFrom(staff->barLineFrom());
                    barLine->setSpanTo(staff->barLineTo());
                    barLine->setBarLineType(blType);
                } else if (barLine->barLineType() != blType && force) {
                    barLine->setBarLineType(blType);
                    barLine->setGenerated(true);
                }
            }

            TLayout::layoutBarLine(barLine, barLine->mutldata(), ctx);
            barlineWidth = std::max(barlineWidth, barLine->width());
        }
        // right align within segment
        for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            track_idx_t track = staffIdx * VOICES;
            BarLine* barLine = toBarLine(barlineSeg->element(track));
            if (!barLine) {
                continue;
            }
            barLine->mutldata()->moveX(barlineWidth - barLine->width());
        }
        barlineSeg->createShapes();
    }

    setClefSegVisibility(m, isLastMeasureInSystem, ctx);
}

void MeasureLayout::setClefSegVisibility(Measure* m, bool isLastMeasureInSystem, LayoutContext& ctx)
{
    const size_t nstaves  = ctx.dom().nstaves();
    bool sectionBreakHideCourtesy = false;
    if (LayoutBreak* sectionBreakElement = m->sectionBreakElement()) {
        sectionBreakHideCourtesy = !sectionBreakElement->showCourtesy();
    }
    const Measure* nextMeasure  = m->nextMeasure();

    Segment* clefSeg = m->findSegmentR(SegmentType::Clef, m->ticks());
    if (!clefSeg && nextMeasure) {
        clefSeg = nextMeasure->findSegmentR(SegmentType::Clef, Fraction(0, 1));
    }

    if (!clefSeg) {
        return;
    }

    bool visibleClef = false;
    bool clefFound = false;
    for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        if (!ctx.dom().staff(staffIdx)->show()) {
            continue;
        }

        const track_idx_t track = staffIdx * VOICES;
        Clef* clef = toClef(clefSeg->element(track));
        if (!clef) {
            continue;
        }

        clefFound = true;

        const bool showCourtesy = ctx.conf().styleB(Sid::genCourtesyClef) && clef->showCourtesy();             // normally show a courtesy clef
        // check if the measure is the last measure of the system or the last measure before a frame
        const bool lastMeasure = isLastMeasureInSystem || (nextMeasure ? !(m->next() == nextMeasure) : true);
        if (!nextMeasure || sectionBreakHideCourtesy || (lastMeasure && !showCourtesy)) {
            // hide the courtesy clef in the final measure of a section, or if the measure is the final measure of a system
            // and the score style or the clef style is set to "not show courtesy clef",
            // or if the clef is at the end of the very last measure of the score
            clef->clear();
            clefSeg->createShape(staffIdx);
        } else {
            TLayout::layoutClef(clef, clef->mutldata(), ctx.conf());
            clefSeg->createShape(staffIdx);
            visibleClef = true;
        }
    }

    if (visibleClef) {                       // there is at least one visible clef in the clef segment
        clefSeg->setVisible(true);
    } else if (!visibleClef && clefFound) {  // all (courtesy) clefs in the clef segment are not visible
        clefSeg->setVisible(false);
    } else { // should never happen
        LOGD("Clef Segment without Clef elements at tick %d/%d", clefSeg->tick().numerator(), clefSeg->tick().denominator());
        clefSeg->setVisible(false);
        return;
    }
}

void MeasureLayout::setCourtesyTimeSig(Measure* m, const Fraction& refSigTick, const Fraction& courtesySigTick,
                                       const SegmentType courtesySegType, LayoutContext& ctx)
{
    // Find original element
    const size_t nstaves = ctx.dom().nstaves();
    const Fraction courtesySigRTick = courtesySigTick - m->tick();
    const Measure* prevMeasure = m->prevMeasureMM();

    const bool isTrailer = courtesySegType == SegmentType::TimeSigAnnounce;
    const bool isContinuationCourtesy = courtesySegType == SegmentType::TimeSigStartRepeatAnnounce;

    const Segment* prevCourtesySegment
        = prevMeasure ? prevMeasure->findSegmentR(SegmentType::TimeSigRepeatAnnounce, prevMeasure->ticks()) : nullptr;

    // Cont. courtesies - check for sig at the start of the measure OR end of previous measure if changesBeforeBarline... is on
    const bool checkPrevMeasure = isContinuationCourtesy && prevMeasure
                                  && ((prevMeasure->repeatEnd() && ctx.conf().styleB(Sid::changesBeforeBarlineRepeats))
                                      || (prevMeasure->repeatJump()
                                          && ctx.conf().styleB(Sid::changesBeforeBarlineOtherJumps)));
    const Segment* tsSegAtCourtesyTick = m->findSegmentR(SegmentType::TimeSig, courtesySigRTick);
    tsSegAtCourtesyTick = !tsSegAtCourtesyTick && checkPrevMeasure ? prevMeasure->findSegmentR(SegmentType::TimeSig,
                                                                                               courtesySigTick
                                                                                               - prevMeasure->tick()) : tsSegAtCourtesyTick;
    const bool shouldShowContCourtesy = prevMeasure && prevMeasure->hasCourtesyTimeSig();

    Segment* courtesySigSeg = m->findSegmentR(courtesySegType, courtesySigRTick);
    for (track_idx_t track = 0; track < nstaves * VOICES; track += VOICES) {
        const Staff* staff = ctx.dom().staff(track2staff(track));
        const TimeSig* actualTimeSig = staff->timeSig(refSigTick);
        const TimeSig* curTimeSig = staff->timeSig(courtesySigTick - Fraction::eps());
        if (!actualTimeSig || !curTimeSig) {
            continue;
        }

        // Show repeat courtesy if the sigs are different
        // Show continuation courtesy if there's a repeat courtesy before it
        // Show trailer courtesy only when there's a time sig element in the next bar
        const bool sigsDifferent = actualTimeSig->sig() != curTimeSig->sig();
        const bool needsCourtesy = isContinuationCourtesy ? shouldShowContCourtesy && prevCourtesySegment && prevCourtesySegment->element(
            track) : isTrailer ? actualTimeSig->tick() == m->endTick() : sigsDifferent;
        // If there is a real key sig at this tick (in this bar or the previous), don't create a courtesy
        const bool hasSigAtTick = tsSegAtCourtesyTick && tsSegAtCourtesyTick->enabled() && tsSegAtCourtesyTick->element(track);
        // Only show courtesy if its real signature has courtesies enabled
        const bool actualShowCourtesy = actualTimeSig && actualTimeSig->showCourtesySig();
        const bool show = actualShowCourtesy && needsCourtesy && !hasSigAtTick && ctx.conf().styleB(Sid::genCourtesyTimesig);

        if (!courtesySigSeg) {
            if (!show) {
                continue;
            }

            // Create a segment if one doesn't already exist
            courtesySigSeg  = Factory::createSegment(m, courtesySegType, courtesySigRTick);
            m->add(courtesySigSeg);
            m->setTrailer(m->trailer() || isTrailer);
        }
        if (!isContinuationCourtesy) {
            m->setHasCourtesyTimeSig(true);
        }
        courtesySigSeg->setTrailer(isTrailer);
        courtesySigSeg->setEnabled(true);

        // Find courtesy element or create if it doesn't exist
        TimeSig* courtesyTimeSig = nullptr;
        EngravingItem* timeSigElem = courtesySigSeg->element(track);
        if (timeSigElem && timeSigElem->isTimeSig()) {
            courtesyTimeSig = toTimeSig(timeSigElem);
        }

        // this time sig shouldn't be shown, remove from segment
        if (!show) {
            if (courtesyTimeSig) {
                courtesySigSeg->remove(courtesyTimeSig);
            }
            continue;
        }

        if (!courtesyTimeSig) {
            courtesyTimeSig = Factory::createTimeSig(courtesySigSeg);
            courtesyTimeSig->setTrack(track);
            courtesyTimeSig->setGenerated(true);
            courtesyTimeSig->setParent(courtesySigSeg);
            courtesyTimeSig->setIsCourtesy(true);
            courtesySigSeg->add(courtesyTimeSig);
        }

        // Layout & create shapes
        courtesyTimeSig->setFrom(actualTimeSig);
        if (courtesyTimeSig->isStyled(Pid::SCALE)) {
            // If this courtesyTimeSig was previously disabled its scale style may have not been updated
            courtesyTimeSig->setScale(courtesyTimeSig->propertyDefault(Pid::SCALE).value<ScaleF>());
        }

        TLayout::layoutTimeSig(courtesyTimeSig, courtesyTimeSig->mutldata(), ctx);
    }

    if (courtesySigSeg && ((!isContinuationCourtesy && !m->hasCourtesyTimeSig()) || !ctx.conf().styleB(Sid::genCourtesyTimesig))) {
        // Whole segment shouldn't be shown, remove any existing courtesy signatures
        if (!isContinuationCourtesy) {
            m->setHasCourtesyTimeSig(false);
        }
        if (courtesySigSeg) {
            courtesySigSeg->setEnabled(false);
        }
    }

    if (courtesySigSeg && courtesySigSeg->enabled()) {
        if (courtesySigSeg->hasElements()) {
            courtesySigSeg->createShapes();
        } else {
            courtesySigSeg->setEnabled(false);
            if (!isContinuationCourtesy) {
                m->setHasCourtesyTimeSig(false);
            }
        }
    }
}

void MeasureLayout::setCourtesyKeySig(Measure* m, const Fraction& refSigTick, const Fraction& courtesySigTick,
                                      const SegmentType courtesySegType, LayoutContext& ctx)
{
    // Find original element
    const size_t nstaves = ctx.dom().nstaves();
    const Fraction courtesySigRTick = courtesySigTick - m->tick();
    const Measure* prevMeasure = m->prevMeasureMM();

    const bool isTrailer = courtesySegType == SegmentType::KeySigAnnounce;
    const bool isContinuationCourtesy = courtesySegType == SegmentType::KeySigStartRepeatAnnounce;

    const Segment* prevCourtesySegment
        = prevMeasure ? prevMeasure->findSegmentR(SegmentType::KeySigRepeatAnnounce, prevMeasure->ticks()) : nullptr;

    // Cont. courtesies - check for sig at the start of the measure OR end of previous measure if changesBeforeBarline... is on
    const bool checkPrevMeasure = isContinuationCourtesy && prevMeasure
                                  && ((prevMeasure->repeatEnd() && ctx.conf().styleB(Sid::changesBeforeBarlineRepeats))
                                      || (prevMeasure->repeatJump()
                                          && ctx.conf().styleB(Sid::changesBeforeBarlineOtherJumps)));
    const Segment* ksSegAtCourtesyTick = m->findSegmentR(SegmentType::KeySig, courtesySigRTick);
    ksSegAtCourtesyTick = !ksSegAtCourtesyTick && checkPrevMeasure ? prevMeasure->findSegmentR(SegmentType::KeySig,
                                                                                               courtesySigTick
                                                                                               - prevMeasure->tick()) : ksSegAtCourtesyTick;

    const bool shouldShowContCourtesy = prevMeasure && prevMeasure->hasCourtesyKeySig();

    Segment* courtesySigSeg = m->findSegmentR(courtesySegType, courtesySigRTick);
    for (track_idx_t track = 0; track < nstaves * VOICES; track += VOICES) {
        // Find reference signature
        const Staff* staff = ctx.dom().staff(track2staff(track));
        const Fraction refSigElementTick = staff->currentKeyTick(refSigTick);
        const Measure* refMeasure = ctx.dom().tick2measureMM(refSigElementTick);
        const Segment* actualKeySigSeg
            = refMeasure ? refMeasure->findSegmentR(SegmentType::KeySig, refSigElementTick - refMeasure->tick()) : nullptr;
        const EngravingItem* el = actualKeySigSeg ? actualKeySigSeg->element(track) : nullptr;
        const KeySig* actualKeySig = el ? toKeySig(el) : nullptr;

        const KeySigEvent refKey = staff->keySigEvent(refSigTick);
        // Get info from correct tick for repeats
        // For trailers and pre-repeat courtesies, signatures should be different
        const bool sigsDifferent = staff->key(m->endTick() - Fraction::eps()) != refKey.key();
        const bool needsCourtesy = isContinuationCourtesy ? shouldShowContCourtesy && prevCourtesySegment && prevCourtesySegment->element(
            track) : sigsDifferent;
        // Only show key sig changes on pitched staves
        const bool staffIsPitchedAtNextMeas = ctx.dom().lastMeasureMM() == m
                                              || (m->nextMeasureMM() && staff->isPitchedStaff(m->nextMeasureMM()->tick()));
        // If there is a real key sig at this tick (in this bar or the previous), don't create a courtesy
        const bool hasSigAtTick = ksSegAtCourtesyTick && ksSegAtCourtesyTick->enabled() && ksSegAtCourtesyTick->element(track);
        // Only show courtesy if its real signature has courtesies enabled
        const bool actualShowCourtesy = actualKeySig ? actualKeySig->showCourtesy() : true;
        const bool show = actualShowCourtesy && staffIsPitchedAtNextMeas && needsCourtesy && !hasSigAtTick && ctx.conf().styleB(
            Sid::genCourtesyKeysig);

        if (!courtesySigSeg) {
            if (!show) {
                continue;
            }

            // Create a segment if one doesn't already exist
            courtesySigSeg = Factory::createSegment(m, courtesySegType, courtesySigRTick);
            m->add(courtesySigSeg);
            m->setTrailer(m->trailer() || isTrailer);
        }
        if (!isContinuationCourtesy) {
            m->setHasCourtesyKeySig(true);
        }
        courtesySigSeg->setTrailer(isTrailer);
        courtesySigSeg->setEnabled(true);

        // Find courtesy element or create if it doesn't exist
        KeySig* courtesyKeySig = nullptr;
        EngravingItem* keySigElem = courtesySigSeg->element(track);
        if (keySigElem && keySigElem->isKeySig()) {
            courtesyKeySig = toKeySig(keySigElem);
        }

        // this key sig shouldn't be shown, remove from segment
        if (!show) {
            if (courtesyKeySig) {
                courtesySigSeg->remove(courtesyKeySig);
            }
            continue;
        }

        if (!courtesyKeySig) {
            courtesyKeySig = Factory::createKeySig(courtesySigSeg);
            courtesyKeySig->setTrack(track);
            courtesyKeySig->setGenerated(true);
            courtesyKeySig->setParent(courtesySigSeg);
            courtesyKeySig->setIsCourtesy(true);
            courtesySigSeg->add(courtesyKeySig);
        }
        courtesyKeySig->setKeySigEvent(refKey);

        // Layout & create shapes
        TLayout::layoutKeySig(courtesyKeySig, courtesyKeySig->mutldata(), ctx.conf());
    }

    if (courtesySigSeg && ((!isContinuationCourtesy && !m->hasCourtesyKeySig()) || !ctx.conf().styleB(Sid::genCourtesyKeysig))) {
        // Whole segment shouldn't be shown, remove any existing courtesy signatures
        if (!isContinuationCourtesy) {
            m->setHasCourtesyKeySig(false);
        }
        if (courtesySigSeg) {
            courtesySigSeg->setEnabled(false);
        }
    }

    if (courtesySigSeg && courtesySigSeg->enabled()) {
        if (courtesySigSeg->hasElements()) {
            courtesySigSeg->createShapes();
        } else {
            courtesySigSeg->setEnabled(false);
            if (!isContinuationCourtesy) {
                m->setHasCourtesyKeySig(false);
            }
        }
    }
}

void MeasureLayout::setCourtesyClef(Measure* m, const Fraction& refClefTick, const Fraction& courtesyClefTick,
                                    const SegmentType courtesySegType, LayoutContext& ctx)
{
    // Find original element
    const size_t nstaves = ctx.dom().nstaves();
    const Fraction courtesyClefRTick = courtesyClefTick - m->tick();
    const Measure* prevMeasure = m->prevMeasureMM();

    const bool isContinuationCourtesy = courtesySegType == SegmentType::ClefStartRepeatAnnounce;

    const Segment* prevCourtesySegment
        = prevMeasure ? prevMeasure->findSegmentR(SegmentType::ClefRepeatAnnounce, prevMeasure->ticks()) : nullptr;
    const Segment* clefSegAtCourtesyTick = m->findSegmentR(SegmentType::Clef | SegmentType::HeaderClef, courtesyClefRTick);
    clefSegAtCourtesyTick
        = !clefSegAtCourtesyTick
          && prevMeasure ? prevMeasure->findSegmentR(SegmentType::KeySig, courtesyClefTick - prevMeasure->tick()) : clefSegAtCourtesyTick;

    bool shouldShowContCourtesy = prevMeasure && prevMeasure->hasCourtesyClef();

    Segment* courtesyClefSeg = m->findSegmentR(courtesySegType, courtesyClefRTick);
    for (track_idx_t track = 0; track < nstaves * VOICES; track += VOICES) {
        const Staff* staff = ctx.dom().staff(track2staff(track));
        const Fraction refClefElementTick = staff->currentClefTick(refClefTick);
        const Measure* refMeasure = ctx.dom().tick2measureMM(refClefElementTick);
        const Segment* actualClefSeg
            = refMeasure ? refMeasure->findSegmentR(SegmentType::Clef | SegmentType::HeaderClef,
                                                    refClefElementTick - refMeasure->tick()) : nullptr;
        if (!actualClefSeg && refMeasure && refMeasure->prevMeasureMM()) {
            // Check previous measure
            Measure* refPrevMeasure = refMeasure->prevMeasureMM();
            actualClefSeg
                = refPrevMeasure->findSegmentR(SegmentType::Clef | SegmentType::HeaderClef, refClefElementTick - refPrevMeasure->tick());
        }
        const EngravingItem* el = actualClefSeg ? actualClefSeg->element(track) : nullptr;
        const Clef* actualClef = el ? toClef(el) : nullptr;

        const ClefType refClef = staff->clef(refClefTick);
        // For trailers and pre-repeat courtesies, clefs should be different
        const bool clefsDifferent = staff->clef(m->endTick() - Fraction::eps()) != refClef;
        const bool needsCourtesy = isContinuationCourtesy ? shouldShowContCourtesy && prevCourtesySegment && prevCourtesySegment->element(
            track) : clefsDifferent;
        // If there is a real clef at this tick (in this bar or the previous), don't create a courtesy
        const bool hasClefAtTick = clefSegAtCourtesyTick && clefSegAtCourtesyTick->enabled() && clefSegAtCourtesyTick->element(track);
        // Only show courtesy if its real clef has courtesies enabled
        const bool actualShowCourtesy = actualClef && actualClef->showCourtesy();
        const bool show = actualShowCourtesy && needsCourtesy && !hasClefAtTick && ctx.conf().styleB(Sid::genCourtesyClef);

        if (!courtesyClefSeg) {
            if (!show) {
                continue;
            }

            // Create a segment if one doesn't already exist
            courtesyClefSeg = Factory::createSegment(m, courtesySegType, courtesyClefRTick);
            m->add(courtesyClefSeg);
            courtesyClefSeg->setTrailer(false);
        }
        if (!isContinuationCourtesy) {
            m->setHasCourtesyClef(true);
        }
        courtesyClefSeg->setEnabled(true);

        // Find courtesy element or create if it doesn't exist
        Clef* courtesyClef = nullptr;
        EngravingItem* clefElem = courtesyClefSeg->element(track);
        if (clefElem && clefElem->isClef()) {
            courtesyClef = toClef(clefElem);
        }

        // this clef shouldn't be shown, remove from segment
        if (!show) {
            if (courtesyClef) {
                courtesyClefSeg->remove(courtesyClef);
            }
            continue;
        }

        if (!courtesyClef) {
            courtesyClef = Factory::createClef(courtesyClefSeg);
            courtesyClef->setTrack(track);
            courtesyClef->setGenerated(true);
            courtesyClef->setSmall(true);
            courtesyClef->setParent(courtesyClefSeg);
            courtesyClef->setClefType(actualClef->clefType());
            courtesyClef->setIsCourtesy(true);
            courtesyClefSeg->add(courtesyClef);
        }

        // Layout & create shapes
        TLayout::layoutClef(courtesyClef, courtesyClef->mutldata(), ctx.conf());
    }

    if (courtesyClefSeg && (!ctx.conf().styleB(Sid::genCourtesyClef) || (!isContinuationCourtesy && !m->hasCourtesyClef()))) {
        // Whole segment shouldn't be shown, remove any existing courtesy signatures
        if (!isContinuationCourtesy) {
            m->setHasCourtesyClef(false);
        }
        if (courtesyClefSeg) {
            courtesyClefSeg->setEnabled(false);
        }
    }

    if (courtesyClefSeg && courtesyClefSeg->enabled()) {
        if (courtesyClefSeg->hasElements()) {
            courtesyClefSeg->createShapes();
        } else {
            courtesyClefSeg->setEnabled(false);
            if (isContinuationCourtesy) {
                m->setHasCourtesyClef(false);
            }
        }
    }
}

static void calcParenTopBottom(Parenthesis* item, double& top, double& bottom, LayoutContext& ctx)
{
    EngravingItem* parent = item->parentItem();
    const double spatium = item->spatium();
    if (!parent) {
        return;
    }

    if (!parent->ldata()->isValid()) {
        TLayout::layoutItem(parent, ctx);
    }

    RectF bbox = parent->ldata()->bbox();

    top = std::min(top, bbox.top() + parent->pos().y() + spatium / 4);
    bottom = std::max(bottom, bbox.bottom() + parent->pos().y() - spatium / 4);
}

void MeasureLayout::addRepeatCourtesyParentheses(Measure* m, const bool continuation, LayoutContext& ctx)
{
    const SegmentType tsSegType = continuation ? SegmentType::TimeSigStartRepeatAnnounce : SegmentType::TimeSigRepeatAnnounce;
    const SegmentType ksSegType = continuation ? SegmentType::KeySigStartRepeatAnnounce : SegmentType::KeySigRepeatAnnounce;
    const SegmentType clefSegType = continuation ? SegmentType::ClefStartRepeatAnnounce : SegmentType::ClefRepeatAnnounce;

    const Fraction sigTick = continuation ? Fraction(0, 1) : m->ticks();

    auto elShouldHaveParenthesis = [&](const Segment* seg, const track_idx_t track) -> bool {
        if (!seg || !seg->enabled() || track > ctx.dom().ntracks()) {
            return false;
        }

        const Staff* staff = ctx.dom().staff(track2staff(track));
        if (!staff) {
            return false;
        }
        const StaffType* st = staff->staffType(seg->tick());
        const bool noTimesig = st && seg->isType(SegmentType::TimeSigTypes) && !st->genTimesig();
        const bool noKeysig = st && seg->isType(SegmentType::KeySigTypes) && !st->genKeysig();
        const bool noClef = st && seg->isType(SegmentType::ClefTypes) && !st->genClef();
        if (noTimesig || noKeysig || noClef) {
            return false;
        }

        const EngravingItem* el = seg->element(track);
        return el && el->visible();
    };

    auto timeSigShouldHaveOwnParentheses = [&](const Segment* seg, const track_idx_t track) -> bool {
        if (!elShouldHaveParenthesis(seg, track)) {
            return false;
        }

        const EngravingItem* el = seg ? seg->element(track) : nullptr;
        if (!el || !el->isTimeSig()) {
            return false;
        }

        return ctx.conf().styleV(Sid::timeSigPlacement).value<TimeSigPlacement>() != TimeSigPlacement::NORMAL;
    };

    auto createParenthesesForSegments = [&](Segment* leftSeg, Segment* rightSeg, track_idx_t track) -> void {
        double top = DBL_MAX;
        double bottom = -DBL_MAX;
        EngravingItem* leftItem = leftSeg ? leftSeg->element(track) : nullptr;
        EngravingItem* rightItem = rightSeg ? rightSeg->element(track) : nullptr;
        Parenthesis* leftParen = nullptr;
        if (leftItem) {
            leftItem->setParenthesesMode(leftItem->rightParen() ? ParenthesesMode::BOTH : ParenthesesMode::LEFT, true, true);
            leftParen = leftItem->leftParen();
        }
        Parenthesis* rightParen = nullptr;
        if (rightItem) {
            rightItem->setParenthesesMode(rightItem->leftParen() ? ParenthesesMode::BOTH : ParenthesesMode::RIGHT, true, true);
            rightParen = rightItem->rightParen();
        }
        bool needsBigTimeSigAdjust = leftSeg && rightSeg && leftSeg == rightSeg && leftSeg->isType(SegmentType::TimeSigTypes)
                                     && ctx.conf().styleV(Sid::timeSigPlacement).value<TimeSigPlacement>()
                                     != TimeSigPlacement::NORMAL;
        if ((ctx.conf().styleB(Sid::smallParens) || needsBigTimeSigAdjust) && leftParen && rightParen) {
            const double spatium = leftParen->spatium();
            calcParenTopBottom(leftParen, top, bottom, ctx);
            calcParenTopBottom(rightParen, top, bottom, ctx);

            double height = bottom - top;

            if (!ctx.conf().styleB(Sid::smallParens)) {
                top -= 1.25 * spatium;
                height += 2.5 * spatium;
            }

            rightParen->mutldata()->startY.set_value(top);
            rightParen->mutldata()->height.set_value(height);
            rightParen->mutldata()->midPointThickness.set_value(height / 60 * rightParen->ldata()->mag());  // 0.1sp for a height of 6sp
            leftParen->mutldata()->startY.set_value(top);
            leftParen->mutldata()->height.set_value(height);
            leftParen->mutldata()->midPointThickness.set_value(height / 60 * leftParen->ldata()->mag());  // 0.1sp for a height of 6sp

            double mag = std::max(leftParen->ldata()->mag(), rightParen->ldata()->mag());
            leftParen->mutldata()->setMag(mag);
            rightParen->mutldata()->setMag(mag);
        } else if (leftParen || rightParen) {
            const Staff* staff = leftParen ? leftParen->staff() : rightParen->staff();
            const double spatium = leftParen ? leftParen->spatium() : rightParen->spatium();
            const Fraction tick = leftParen ? leftParen->tick() : rightParen->tick();
            if (leftParen) {
                leftParen->mutldata()->startY.set_value(-spatium);
                leftParen->mutldata()->height.set_value(staff->staffHeight(tick) + 2 * spatium * leftParen->mag());
                leftParen->mutldata()->midPointThickness.set_value(leftParen->ldata()->height / 60 * leftParen->ldata()->mag());  // 0.1sp for a height of 6sp
            }
            if (rightParen) {
                rightParen->mutldata()->startY.set_value(-spatium);
                rightParen->mutldata()->height.set_value(staff->staffHeight(tick) + 2 * spatium * rightParen->mag());
                rightParen->mutldata()->midPointThickness.set_value(rightParen->ldata()->height / 60 * rightParen->ldata()->mag());  // 0.1sp for a height of 6sp
            }
        }

        if (leftParen) {
            ParenthesisLayout::layoutParentheses(leftItem, ctx);
        }

        if (rightParen && rightItem != leftItem) {
            ParenthesisLayout::layoutParentheses(rightItem, ctx);
        }
    };

    for (track_idx_t track = 0; track <= ctx.dom().nstaves() * VOICES; track += VOICES) {
        Segment* clefSeg = m->findSegmentR(clefSegType, sigTick);
        Segment* ksSeg = m->findSegmentR(ksSegType, sigTick);
        Segment* tsSeg = m->findSegmentR(tsSegType, sigTick);
        bool separateTsParens = timeSigShouldHaveOwnParentheses(tsSeg, track);

        Segment* leftMostSeg = clefSeg;

        if (!elShouldHaveParenthesis(leftMostSeg, track)) {
            leftMostSeg = ksSeg;
        }

        if (!separateTsParens && !elShouldHaveParenthesis(leftMostSeg, track)) {
            leftMostSeg = tsSeg;
        }

        if (!elShouldHaveParenthesis(leftMostSeg, track)) {
            leftMostSeg = nullptr;
        }

        // Remove stale parentheses
        for (Segment* seg : { clefSeg, ksSeg, tsSeg }) {
            if (seg == leftMostSeg || !seg || (seg == tsSeg && separateTsParens) || !seg->element(track)) {
                continue;
            }
            EngravingItem* item = seg->element(track);
            removeRepeatCourtesyParenthesis(item, DirectionH::LEFT);
        }

        Segment* rightMostSeg = separateTsParens ? nullptr : tsSeg;

        if (!elShouldHaveParenthesis(rightMostSeg, track)) {
            rightMostSeg = ksSeg;
        }

        if (!elShouldHaveParenthesis(rightMostSeg, track)) {
            rightMostSeg = clefSeg;
        }

        if (!elShouldHaveParenthesis(rightMostSeg, track)) {
            rightMostSeg = nullptr;
        }

        // Remove stale parentheses
        for (Segment* seg : { clefSeg, ksSeg, tsSeg }) {
            if (seg == rightMostSeg || !seg || (seg == tsSeg && separateTsParens) || !seg->element(track)) {
                continue;
            }
            EngravingItem* item = seg->element(track);
            removeRepeatCourtesyParenthesis(item, DirectionH::RIGHT);
        }

        createParenthesesForSegments(leftMostSeg, rightMostSeg, track);

        if (separateTsParens) {
            createParenthesesForSegments(tsSeg, tsSeg, track);
        }
    }
}

void MeasureLayout::removeRepeatCourtesyParenthesesMeasure(Measure* m, const bool continuation, LayoutContext& ctx)
{
    const SegmentType tsSegType = continuation ? SegmentType::TimeSigStartRepeatAnnounce : SegmentType::TimeSigRepeatAnnounce;
    const SegmentType ksSegType = continuation ? SegmentType::KeySigStartRepeatAnnounce : SegmentType::KeySigRepeatAnnounce;
    const SegmentType clefSegType = continuation ? SegmentType::ClefStartRepeatAnnounce : SegmentType::ClefRepeatAnnounce;

    const Fraction sigTick = continuation ? Fraction(0, 1) : m->ticks();

    Segment* clefSeg = m->findSegmentR(clefSegType, sigTick);
    Segment* ksSeg = m->findSegmentR(ksSegType, sigTick);
    Segment* tsSeg = m->findSegmentR(tsSegType, sigTick);

    for (Segment* seg : { clefSeg, ksSeg, tsSeg }) {
        if (!seg) {
            continue;
        }
        for (track_idx_t track = 0; track <= ctx.dom().nstaves() * VOICES; track += VOICES) {
            EngravingItem* item = seg->element(track);
            if (!item) {
                continue;
            }
            removeRepeatCourtesyParenthesis(item);
        }
    }
}

void MeasureLayout::removeRepeatCourtesyParenthesis(EngravingItem* item, const DirectionH direction)
{
    if (!item) {
        return;
    }
    if (direction == DirectionH::AUTO) {
        if (Parenthesis* leftParen = item->leftParen()) {
            item->remove(leftParen);
        }
        if (Parenthesis* rightParen = item->rightParen()) {
            item->remove(rightParen);
        }
        return;
    }

    Parenthesis* paren = item->paren(direction);
    if (!paren) {
        return;
    }

    item->remove(paren);
}

void MeasureLayout::setRepeatCourtesiesAndParens(Measure* m, LayoutContext& ctx)
{
    const bool showCourtesyRepeats = m->repeatEnd() && ctx.conf().styleB(Sid::showCourtesiesRepeats);
    const bool showCourtesyOtherJumps = m->repeatJump() && ctx.conf().styleB(Sid::showCourtesiesOtherJumps);

    if (showCourtesyRepeats) {
        MeasureLayout::addRepeatCourtesies(m, ctx);
        if (ctx.conf().styleB(Sid::useParensRepeatCourtesies)) {
            MeasureLayout::addRepeatCourtesyParentheses(m, false, ctx);
        } else {
            MeasureLayout::removeRepeatCourtesyParenthesesMeasure(m, false, ctx);
        }
    }

    if (showCourtesyOtherJumps) {
        MeasureLayout::addRepeatCourtesies(m, ctx);
        if (ctx.conf().styleB(Sid::useParensOtherJumpCourtesies)) {
            MeasureLayout::addRepeatCourtesyParentheses(m, false, ctx);
        } else {
            MeasureLayout::removeRepeatCourtesyParenthesesMeasure(m, false, ctx);
        }
    }

    if (!showCourtesyRepeats && !showCourtesyOtherJumps) {
        MeasureLayout::removeRepeatCourtesyParenthesesMeasure(m, false, ctx);
        removeRepeatCourtesies(m);
    }

    const bool courtesiesAfterCancellingRepeats = m->prevMeasureMM() && m->prevMeasureMM()->repeatEnd()
                                                  && ctx.conf().styleB(Sid::showCourtesiesAfterCancellingRepeats)
                                                  && ctx.conf().styleB(Sid::showCourtesiesRepeats);
    const bool courtesiesAfterCancellingOtherJumps = m->prevMeasureMM() && m->prevMeasureMM()->repeatJump()
                                                     && ctx.conf().styleB(Sid::showCourtesiesAfterCancellingOtherJumps)
                                                     && ctx.conf().styleB(Sid::showCourtesiesOtherJumps);
    if (courtesiesAfterCancellingRepeats) {
        MeasureLayout::addRepeatContinuationCourtesies(m, ctx);
        if (ctx.conf().styleB(Sid::useParensRepeatCourtesiesAfterCancelling)) {
            MeasureLayout::addRepeatCourtesyParentheses(m, true, ctx);
        } else {
            MeasureLayout::removeRepeatCourtesyParenthesesMeasure(m, true, ctx);
        }
    }

    if (courtesiesAfterCancellingOtherJumps) {
        MeasureLayout::addRepeatContinuationCourtesies(m, ctx);
        if (ctx.conf().styleB(Sid::useParensOtherJumpCourtesiesAfterCancelling)) {
            MeasureLayout::addRepeatCourtesyParentheses(m, true, ctx);
        } else {
            MeasureLayout::removeRepeatCourtesyParenthesesMeasure(m, true, ctx);
        }
    }

    if (!courtesiesAfterCancellingRepeats && !courtesiesAfterCancellingOtherJumps) {
        MeasureLayout::removeRepeatCourtesyParenthesesMeasure(m, true, ctx);
        removeRepeatContinuationCourtesies(m);
    }
}

void MeasureLayout::addRepeatCourtesies(Measure* m, LayoutContext& ctx)
{
    const std::vector<Measure*> measures = findFollowingRepeatMeasures(m);
    if (measures.empty()) {
        removeRepeatCourtesies(m);
        return;
    }

    bool hasCourtesies = false;
    for (Measure* repeatStartMeasure : measures) {
        // Follow section break courtesy property
        const Measure* prevMeasure = repeatStartMeasure->prevMeasureMM();
        const LayoutBreak* sectionBreak = prevMeasure ? prevMeasure->sectionBreakElement() : nullptr;
        const bool sectionBreakHideCourtesies = sectionBreak && !sectionBreak->showCourtesy();

        if (repeatStartMeasure == m->nextMeasureMM() || sectionBreakHideCourtesies) {
            continue;
        }
        setCourtesyClef(m, repeatStartMeasure->tick(), m->endTick(), SegmentType::ClefRepeatAnnounce, ctx);
        setCourtesyKeySig(m, repeatStartMeasure->tick(), m->endTick(), SegmentType::KeySigRepeatAnnounce, ctx);
        setCourtesyTimeSig(m, repeatStartMeasure->tick(), m->endTick(), SegmentType::TimeSigRepeatAnnounce, ctx);

        hasCourtesies = true;
    }

    if (!hasCourtesies) {
        removeRepeatCourtesies(m);
    }
}

void MeasureLayout::removeRepeatCourtesies(Measure* m)
{
    for (Segment* seg = m->last(); seg != m->first(); seg = seg->prev()) {
        if (seg->isChordRestType()) {
            break;
        }
        if (!seg->isType(SegmentType::ClefRepeatAnnounce | SegmentType::TimeSigRepeatAnnounce | SegmentType::KeySigRepeatAnnounce)) {
            continue;
        }
        seg->mutldata()->reset();

        if (seg->enabled()) {
            seg->setEnabled(false);
        }
    }
    m->setHasCourtesyClef(false);
    m->setHasCourtesyKeySig(false);
    m->setHasCourtesyTimeSig(false);
}

void MeasureLayout::addRepeatContinuationCourtesies(Measure* m, LayoutContext& ctx)
{
    setCourtesyTimeSig(m, m->tick(), m->tick(), SegmentType::TimeSigStartRepeatAnnounce, ctx);
    setCourtesyKeySig(m, m->tick(), m->tick(), SegmentType::KeySigStartRepeatAnnounce, ctx);
    setCourtesyClef(m, m->tick(), m->tick(), SegmentType::ClefStartRepeatAnnounce, ctx);
}

void MeasureLayout::removeRepeatContinuationCourtesies(Measure* m)
{
    for (Segment* seg = m->first(); seg != m->last(); seg = seg->next()) {
        if (seg->isChordRestType()) {
            break;
        }
        if (!seg->isType(SegmentType::ClefStartRepeatAnnounce | SegmentType::TimeSigStartRepeatAnnounce
                         | SegmentType::KeySigStartRepeatAnnounce)) {
            continue;
        }

        seg->mutldata()->reset();

        if (seg->enabled()) {
            seg->setEnabled(false);
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
    ClefTypeList cl = staff->clefType(m->tick() - Fraction::eps());
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
            m->add(cSegment);
        }
        cSegment->setHeader(true);
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
        Measure* pm = m->prevMeasureMM();
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
            m->add(kSegment);
        }
        kSegment->setHeader(true);
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
                    Segment* clefSeg = searchMeasure->findFirstR(SegmentType::Clef | SegmentType::HeaderClef, Fraction(0, 1));
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
        if (seg->isKeySigType()) {
            bool keySigChangeHappensHere = false;
            for (Staff* staff : m->score()->staves()) {
                if (staff->keyList()->count(m->tick().ticks()) > 0) {
                    keySigChangeHappensHere = true;
                    break;
                }
            }
            if (seg->header() && !keySigChangeHappensHere) {
                seg->setEnabled(false);
            }
        }
        if (!seg->header()) {
            break;
        }
        if (!seg->isKeySigType()) {
            seg->setEnabled(false);
        }
    }
    m->setHeader(false);
}

void MeasureLayout::addSystemTrailer(Measure* m, Measure* nm, LayoutContext& ctx)
{
    const size_t nstaves = ctx.dom().nstaves();
    bool systemBreakHideCourtesy = false;
    if (LayoutBreak* sectionBreakElement = m->sectionBreakElement()) {
        systemBreakHideCourtesy = !sectionBreakElement->showCourtesy();
    }

    // locate a time sig. in the next measure and, if found,
    // check if it has court. sig. turned off
    if (nm && ctx.conf().styleB(Sid::genCourtesyTimesig) && !systemBreakHideCourtesy && !ctx.conf().isFloatMode()) {
        setCourtesyTimeSig(m, nm->tick(), m->endTick(), SegmentType::TimeSigAnnounce, ctx);
    }

    // courtesy key signatures, clefs
    if (nm && ctx.conf().styleB(Sid::genCourtesyKeysig) && !systemBreakHideCourtesy) {
        setCourtesyKeySig(m, nm->tick(), m->endTick(), SegmentType::KeySigAnnounce, ctx);
    }

    Segment* courtesyClefSeg = m->findSegmentR(SegmentType::Clef, m->ticks());
    if (courtesyClefSeg) {
        for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            const track_idx_t track = staffIdx * VOICES;

            Clef* courtesyClef = toClef(courtesyClefSeg->element(track));
            if (courtesyClef) {
                courtesyClef->setSmall(true);
                courtesyClef->setIsTrailer(true);
            }
        }
    }
    if (courtesyClefSeg) {
        courtesyClefSeg->createShapes();
    }

    m->checkTrailer();
}

void MeasureLayout::removeSystemTrailer(Measure* m, LayoutContext& ctx)
{
    for (Segment* seg = m->last(); seg != m->first(); seg = seg->prev()) {
        if (seg->isChordRestType()) {
            break;
        }

        if (seg->isClefType()) {
            for (EngravingItem* el : seg->elist()) {
                if (!el) {
                    continue;
                }
                Clef* clef = toClef(el);
                clef->setIsTrailer(false);
                TLayout::layoutClef(clef, clef->mutldata(), ctx.conf());
            }
            seg->createShapes();
        }

        if (seg->isTimeTickType() || !seg->trailer()) {
            continue;
        }

        if (seg->enabled()) {
            seg->setEnabled(false);
            seg->setTrailer(false);
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
                    double d = ctx.conf().styleAbsolute(Sid::multiMeasureRestMargin);
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
        if (!(refCRSeg && refCRSeg->isActive())) {
            refCRSeg = segment.prevActive();
            while (refCRSeg && !refCRSeg->isChordRestType()) {
                refCRSeg = refCRSeg->prevActive();
            }
        }

        if (!refCRSeg || refCRSeg->ticks().isZero()) {
            continue;
        }

        Fraction refSegDuration = refCRSeg->ticks();
        Fraction thisDuration = segment.ticks();
        Fraction relativeTick = segment.rtick() - refCRSeg->rtick();

        Segment* nextSeg = m->findSegmentR(SegmentType::ChordRest, refCRSeg->rtick() + refCRSeg->ticks());
        if (!(nextSeg && nextSeg->isActive())) {
            nextSeg = m->findSegmentR(SegmentType::BarLineTypes, refCRSeg->rtick() + refCRSeg->ticks());
        }
        double width = nextSeg ? nextSeg->x() - refCRSeg->x() : refCRSeg->width();

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

void MeasureLayout::updateKeySignatures(const Measure* measure, LayoutContext& ctx)
{
    Measure* prevMeasure = measure->prevMeasure();
    if (!prevMeasure || !prevMeasure->repeatEnd()) {
        return;
    }
    for (const Segment& seg : measure->segments()) {
        if (!seg.isType(SegmentType::KeySigTypes)) {
            continue;
        }

        for (EngravingItem* el : seg.elist()) {
            if (!el) {
                continue;
            }

            KeySig* ks = toKeySig(el);

            TLayout::layoutKeySig(ks, ks->mutldata(), ctx.conf());
        }
    }
}
