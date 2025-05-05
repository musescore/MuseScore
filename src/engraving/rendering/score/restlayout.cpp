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

#include "restlayout.h"

#include "beamlayout.h"
#include "chordlayout.h"
#include "tlayout.h"

#include "dom/beam.h"
#include "dom/system.h"

using namespace muse;
using namespace mu::engraving;

namespace mu::engraving::rendering::score {
void RestLayout::layoutRest(const Rest* item, Rest::LayoutData* ldata, const LayoutContext& ctx)
{
    if (item->isGap()) {
        return;
    }

    //! NOTE The types are listed here explicitly to show what types there are (see Rest::add method)
    //! and accordingly show what depends on.
    for (EngravingItem* e : item->el()) {
        switch (e->type()) {
        case ElementType::DEAD_SLAPPED: {
            DeadSlapped* ds = item_cast<DeadSlapped*>(e);
            LD_INDEPENDENT;
            TLayout::layoutDeadSlapped(ds, ds->mutldata());
        } break;
        case ElementType::SYMBOL: {
            Symbol* s = item_cast<Symbol*>(e);
            // LD_X not clear yet
            TLayout::layoutSymbol(s, s->mutldata(), ctx);
        } break;
        case ElementType::IMAGE: {
            Image* im = item_cast<Image*>(e);
            LD_INDEPENDENT;
            TLayout::layoutImage(im, im->mutldata());
        } break;
        default:
            UNREACHABLE;
        }
    }

    if (item->deadSlapped()) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);

    double spatium = item->spatium();

    ldata->setPosX(0.0);
    const StaffType* stt = item->staffType();
    if (stt && stt->isTabStaff()) {
        // if rests are shown and note values are shown as duration symbols
        if (stt->showRests() && stt->genDurations()) {
            DurationType type = item->durationType().type();
            int dots = item->durationType().dots();
            // if rest is whole measure, convert into actual type and dot values
            if (type == DurationType::V_MEASURE && item->measure()) {
                Fraction ticks = item->measure()->ticks();
                TDuration dur  = TDuration(ticks).type();
                type           = dur.type();
                dots           = dur.dots();
            }
            // symbol needed; if not exist, create, if exists, update duration
            if (!item->tabDur()) {
                const_cast<Rest*>(item)->setTabDur(new TabDurationSymbol(const_cast<Rest*>(item), stt, type, dots));
            } else {
                item->tabDur()->setDuration(type, dots, stt);
            }
            item->tabDur()->setParent(const_cast<Rest*>(item));
// needed?        _tabDur->setTrack(track());
            TLayout::layoutTabDurationSymbol(item->tabDur(), item->tabDur()->mutldata());
            ldata->setBbox(item->tabDur()->ldata()->bbox());
            ldata->setPos(0.0, 0.0);                   // no rest is drawn: reset any position might be set for it
            return;
        }
        // if no rests or no duration symbols, delete any dur. symbol and chain into standard staff mngmt
        // this is to ensure horiz space is reserved for rest, even if they are not displayed
        // Rest::draw() will skip their drawing, if not needed
        if (item->tabDur()) {
            delete item->tabDur();
            const_cast<Rest*>(item)->setTabDur(nullptr);
        }
    }

    const_cast<Rest*>(item)->setDotLine(Rest::getDotline(item->durationType().type()));

    double yOff = item->offset().y();
    const Staff* stf = item->staff();
    const StaffType* st = stf ? stf->staffTypeForElement(item) : 0;
    double lineDist = st ? st->lineDistance().val() : 1.0;
    int userLine   = RealIsNull(yOff) ? 0 : lrint(yOff / (lineDist * spatium));
    int lines      = st ? st->lines() : 5;

    int naturalLine = computeNaturalLine(lines); // Measured in 1sp steps
    int voiceOffset = computeVoiceOffset(item, ldata); // Measured in 1sp steps
    int wholeRestOffset = computeWholeOrBreveRestOffset(item, voiceOffset, lines);
    int finalLine = naturalLine + voiceOffset + wholeRestOffset;

    ldata->sym = item->getSymbol(item->durationType().type(), finalLine + userLine, lines);

    ldata->setPosY(finalLine * lineDist * spatium);
    if (!item->shouldNotBeDrawn()) {
        fillShape(item, ldata, ctx.conf());
    }

    auto layoutRestDots = [](const Rest* item, const LayoutConfiguration& conf, Rest::LayoutData* ldata)
    {
        const_cast<Rest*>(item)->checkDots();
        double visibleX = item->symWidthNoLedgerLines(ldata) + conf.styleMM(Sid::dotNoteDistance) * item->mag();
        double visibleDX = conf.styleMM(Sid::dotDotDistance) * item->mag();
        double invisibleX = item->symWidthNoLedgerLines(ldata);
        double y = item->dotLine() * item->spatium() * .5;
        for (NoteDot* dot : item->dotList()) {
            NoteDot::LayoutData* dotldata = dot->mutldata();
            TLayout::layoutNoteDot(dot, dotldata);
            if (dot->visible()) {
                dotldata->setPos(visibleX, y);
                visibleX += visibleDX;
            } else {
                invisibleX +=  0.1 * item->spatium();
                dotldata->setPos(invisibleX, y);
                invisibleX += item->symWidth(SymId::augmentationDot) * dot->mag();
            }
        }
    };

    layoutRestDots(item, ctx.conf(), ldata);
}

void RestLayout::fillShape(const Rest* item, Rest::LayoutData* ldata, const LayoutConfiguration& conf)
{
    switch (item->type()) {
    case ElementType::REST:
        fillShape(static_cast<const Rest*>(item), static_cast<Rest::LayoutData*>(ldata));
        break;
    case ElementType::MMREST:
        fillShape(static_cast<const MMRest*>(item), static_cast<MMRest::LayoutData*>(ldata), conf);
        break;
    default:
        UNREACHABLE;
        return;
    }
}

void RestLayout::resolveVerticalRestConflicts(LayoutContext& ctx, Segment* segment, staff_idx_t staffIdx)
{
    std::vector<Rest*> rests;
    std::vector<Chord*> chords;

    collectChordsAndRest(segment, staffIdx, chords, rests);

    if (rests.empty()) {
        return;
    }

    collectChordsOverlappingRests(segment, staffIdx, chords);

    for (Rest* rest : rests) {
        rest->verticalClearance().reset();
    }

    const Staff* staff = ctx.dom().staff(staffIdx);
    if (!chords.empty()) {
        resolveRestVSChord(rests, chords, staff, segment);
    }

    if (rests.size() < 2) {
        return;
    }

    resolveRestVSRest(rests, staff, segment, ctx);
}

void RestLayout::resolveRestVSChord(std::vector<Rest*>& rests, std::vector<Chord*>& chords, const Staff* staff, Segment* segment)
{
    Fraction tick = segment->tick();
    int lines = staff->lines(tick);
    double spatium = staff->spatium(tick);
    double lineDistance = staff->lineDistance(tick) * spatium;

    for (Rest* rest : rests) {
        if (!rest->visible() || !rest->autoplace()) {
            continue;
        }
        bool isWholeOrHalf = rest->isWholeRest() || rest->durationType() == DurationType::V_HALF;
        double minRestToChordClearance = isWholeOrHalf ? 0.55 * spatium : 0.35 * spatium;
        RestVerticalClearance& restVerticalClearance = rest->verticalClearance();
        for (Chord* chord : chords) {
            if (!chord->visible() || !chord->autoplace()) {
                continue;
            }

            bool restAbove = rest->voice() < chord->voice() || (chord->slash() && !(rest->voice() % 2));
            int upSign = restAbove ? -1 : 1;
            double restYOffset = rest->offset().y();
            bool ignoreYOffset = (restAbove && restYOffset > 0) || (!restAbove && restYOffset < 0);
            PointF offset = ignoreYOffset ? PointF(0, restYOffset) : PointF(0, 0);

            Shape chordShape = chord->shape().translate(chord->pos());
            chordShape.removeInvisibles();
            chordShape.removeTypes({ ElementType::ARPEGGIO });
            if (chordShape.empty()) {
                continue;
            }

            double clearance = 0.0;
            Shape restShape = rest->shape().translate(rest->pos() - offset);
            if (chord->segment() == rest->segment()) {
                clearance = restAbove
                            ? restShape.verticalClearance(chordShape)
                            : chordShape.verticalClearance(restShape);
            } else {
                Note* limitNote = restAbove ? chord->upNote() : chord->downNote();
                RectF noteShape = limitNote->symBbox(limitNote->noteHead()).translated(limitNote->pos());
                clearance = restAbove ? noteShape.top() - restShape.bottom() : restShape.top() - noteShape.bottom();
                minRestToChordClearance = 0.0;
            }

            double margin = clearance - minRestToChordClearance;
            int marginInSteps = floor(margin / lineDistance);
            if (restAbove) {
                restVerticalClearance.setBelow(marginInSteps);
            } else {
                restVerticalClearance.setAbove(marginInSteps);
            }
            if (margin > 0) {
                continue;
            }

            rest->verticalClearance().setLocked(true);
            bool outAboveStaff = restAbove && restShape.bottom() + margin < minRestToChordClearance;
            bool outBelowStaff = !restAbove && restShape.top() - margin > (lines - 1) * lineDistance - minRestToChordClearance;
            bool useHalfSpaceSteps = (outAboveStaff || outBelowStaff) && !isWholeOrHalf;
            double yMove;
            if (useHalfSpaceSteps) {
                int steps = ceil(std::abs(margin) / (lineDistance / 2));
                yMove = steps * lineDistance / 2 * upSign;
                rest->mutldata()->moveY(yMove);
            } else {
                int steps = ceil(std::abs(margin) / lineDistance);
                yMove = steps * lineDistance * upSign;
                rest->mutldata()->moveY(yMove);
            }
            for (Rest* mergedRest : rest->ldata()->mergedRests) {
                mergedRest->mutldata()->moveY(yMove);
            }
            if (isWholeOrHalf) {
                updateSymbol(rest, rest->mutldata()); // Because it may need to use the symbol with ledger line now
            }
        }
    }
}

void RestLayout::resolveRestVSRest(std::vector<Rest*>& rests, const Staff* staff,
                                   Segment* segment, LayoutContext& ctx,
                                   bool considerBeams)
{
    if (rests.empty()) {
        return;
    }

    Fraction tick = segment->tick();
    double spatium = staff->spatium(tick);
    double lineDistance = staff->lineDistance(tick) * spatium;
    int lines = staff->lines(tick);
    const double minRestToRestClearance = 0.55 * spatium;

    for (size_t i = 0; i < rests.size() - 1; ++i) {
        Rest* rest1 = rests[i];
        if (!rest1->visible() || !rest1->autoplace()) {
            continue;
        }

        RestVerticalClearance& rest1Clearance = rest1->verticalClearance();
        Shape shape1 = rest1->shape().translate(rest1->pos() - rest1->offset());

        Rest* rest2 = rests[i + 1];
        if (!rest2->visible() || !rest2->autoplace()) {
            continue;
        }

        if (muse::contains(rest1->ldata()->mergedRests, rest2) || muse::contains(rest2->ldata()->mergedRests, rest1)) {
            continue;
        }

        Shape shape2 = rest2->shape().translate(rest2->pos() - rest2->offset());
        RestVerticalClearance& rest2Clearance = rest2->verticalClearance();

        double clearance;
        bool firstAbove = rest1->voice() < rest2->voice();
        if (firstAbove) {
            clearance = shape1.verticalClearance(shape2);
        } else {
            clearance = shape2.verticalClearance(shape1);
        }
        double margin = clearance - minRestToRestClearance;
        int marginInSteps = floor(margin / lineDistance);
        if (firstAbove) {
            rest1Clearance.setBelow(marginInSteps);
            rest2Clearance.setAbove(marginInSteps);
        } else {
            rest1Clearance.setAbove(marginInSteps);
            rest2Clearance.setBelow(marginInSteps);
        }

        if (margin > 0) {
            continue;
        }

        int steps = ceil(std::abs(margin) / lineDistance);
        // Move the two rests away from each other
        int step1 = floor(double(steps) / 2);
        int step2 = ceil(double(steps) / 2);
        int maxStep1 = firstAbove ? rest1Clearance.above() : rest1Clearance.below();
        int maxStep2 = firstAbove ? rest2Clearance.below() : rest2Clearance.above();
        maxStep1 = std::max(maxStep1, 0);
        maxStep2 = std::max(maxStep2, 0);
        if (step1 > maxStep1) {
            step2 += step1 - maxStep1; // First rest is locked, try move the second more
        }
        if (step2 > maxStep2) {
            step1 += step2 - maxStep2; // Second rest is locked, try move the first more
        }
        step1 = std::min(step1, maxStep1);
        step2 = std::min(step2, maxStep2);
        rest1->mutldata()->moveY(step1 * lineDistance * (firstAbove ? -1 : 1));
        rest2->mutldata()->moveY(step2 * lineDistance * (firstAbove ? 1 : -1));

        Beam* beam1 = rest1->beam();
        Beam* beam2 = rest2->beam();
        if (beam1 && beam2 && considerBeams) {
            shape1 = rest1->shape().translate(rest1->pos() - rest1->offset());
            shape2 = rest2->shape().translate(rest2->pos() - rest2->offset());

            ChordRest* beam1Start = beam1->elements().front();
            ChordRest* beam1End = beam1->elements().back();
            double y1Start = BeamLayout::chordBeamAnchorY(beam1, beam1Start) - beam1Start->pagePos().y();
            double y1End = BeamLayout::chordBeamAnchorY(beam1, beam1End) - beam1End->pagePos().y();
            double beam1Ymid = 0.5 * (y1Start + y1End);

            ChordRest* beam2Start = beam2->elements().front();
            ChordRest* beam2End = beam2->elements().back();
            double y2Start = BeamLayout::chordBeamAnchorY(beam2, beam2Start) - beam2Start->pagePos().y();
            double y2End = BeamLayout::chordBeamAnchorY(beam2, beam2End) - beam2End->pagePos().y();
            double beam2Ymid = 0.5 * (y2Start + y2End);

            double centerY = 0.5 * (beam1Ymid + beam2Ymid);

            double upperBound = shape1.bottom();
            double lowerBound = shape2.top();
            int steps2 = 0;
            if (centerY < upperBound) {
                steps2 = floor((centerY - upperBound) / lineDistance);
            } else if (centerY > lowerBound) {
                steps2 = ceil((centerY - lowerBound) / lineDistance);
            }
            double moveY = steps2 * lineDistance;
            rest1->mutldata()->moveY(moveY);
            rest2->mutldata()->moveY(moveY);
            shape1.translate(PointF(0.0, moveY));
            shape2.translate(PointF(0.0, moveY));

            double halfLineDistance = 0.5 * lineDistance;
            if (shape1.bottom() < -halfLineDistance) {
                rest1->mutldata()->moveY(halfLineDistance);
            } else if (centerY >= (lines - 1) * lineDistance + halfLineDistance) {
                rest2->mutldata()->moveY(-halfLineDistance);
            }

            rest1->verticalClearance().setLocked(true);
            rest2->verticalClearance().setLocked(true);
            TLayout::layoutBeam(beam1, ctx);
            TLayout::layoutBeam(beam2, ctx);
        }

        bool rest1IsWholeOrHalf = rest1->isWholeRest() || rest1->durationType() == DurationType::V_HALF;
        bool rest2IsWholeOrHalf = rest2->isWholeRest() || rest2->durationType() == DurationType::V_HALF;

        if (rest1IsWholeOrHalf) {
            updateSymbol(rest1, rest1->mutldata());
        }
        if (rest2IsWholeOrHalf) {
            updateSymbol(rest2, rest2->mutldata());
        }
    }
}

void RestLayout::alignRests(const System* system, LayoutContext& ctx)
{
    using RestGroup = std::vector<Rest*>;
    using RestGroups = std::vector<RestGroup>;

    RestGroups restGroups;

    for (staff_idx_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
        for (const MeasureBase* mb : system->measures()) {
            if (!(mb->isMeasure() && toMeasure(mb)->hasVoices(staffIdx))) {
                continue;
            }

            const Measure* measure = toMeasure(mb);

            track_idx_t sTrack = staffIdx * VOICES;
            track_idx_t eTrack = sTrack + VOICES;

            std::vector<Fraction> voiceInterruptionPoints;
            for (const Segment* segment = measure->last(SegmentType::ChordRest); segment; segment = segment->prev(SegmentType::ChordRest)) {
                for (track_idx_t track = sTrack; track < eTrack; ++track) {
                    EngravingItem* item = segment->element(track);
                    if (item && item->isRest() && toRest(item)->isGap()) {
                        voiceInterruptionPoints.push_back(segment->rtick() + segment->ticks());
                        voiceInterruptionPoints.push_back(segment->rtick());
                        break;
                    }
                }
            }

            for (track_idx_t track = sTrack; track < eTrack; ++track) {
                restGroups.push_back(RestGroup());
                for (const Segment* segment = measure->first(SegmentType::ChordRest); segment;
                     segment = segment->next(SegmentType::ChordRest)) {
                    if (voiceInterruptionPoints.size() > 0 && segment->rtick() >= voiceInterruptionPoints.back()) {
                        restGroups.push_back(RestGroup());
                        voiceInterruptionPoints.pop_back();
                    }
                    EngravingItem* item = segment->element(track);
                    if (!(item && item->isRest() && item->visible())) {
                        continue;
                    }
                    Rest* rest = toRest(item);
                    if (rest->staffMove() == 0 && !rest->isGap() && rest->alignWithOtherRests()) {
                        restGroups.back().push_back(rest);
                    }
                }
            }
        }
    }

    for (RestGroup& group : restGroups) {
        if (group.size() == 0) {
            continue;
        }

        Rest* firstRest = group.front();
        const bool alignUpwards = firstRest->voice() == 0;
        const double lineDist = firstRest->staff()->lineDistance(firstRest->tick()) * firstRest->spatium();

        double yOuterRest = alignUpwards ? DBL_MAX : -DBL_MAX;
        for (Rest* rest : group) {
            double yRest = rest->ldata()->pos().y();
            yOuterRest = alignUpwards ? std::min(yOuterRest, yRest) : std::max(yOuterRest, yRest);
        }
        for (Rest* rest : group) {
            double yCur = rest->ldata()->pos().y();
            double yResult = yOuterRest;
            double restVertClearance = lineDist * (alignUpwards ? rest->verticalClearance().above() : rest->verticalClearance().below());
            double vertPadding = lineDist;
            restVertClearance = std::max(0.0, restVertClearance - vertPadding);
            double requiredMove = yOuterRest - yCur;
            if (std::abs(requiredMove) > restVertClearance) {
                yResult = alignUpwards ? yCur - restVertClearance : yCur + restVertClearance;
            }
            rest->mutldata()->setPosY(yResult);
            if (rest->isWholeRest() || rest->durationType() == DurationType::V_HALF) {
                updateSymbol(rest, rest->mutldata());
            }
        }
    }
}

void RestLayout::checkFullMeasureRestCollisions(const System* system, LayoutContext& ctx)
{
    for (staff_idx_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
        for (MeasureBase* mb : system->measures()) {
            if (!(mb->isMeasure() && toMeasure(mb)->hasVoices(staffIdx))) {
                continue;
            }

            Measure* measure = toMeasure(mb);
            Rest* fullMeasureRest = nullptr;

            Segment* firstCRSeg = measure->first(SegmentType::ChordRest);
            IF_ASSERT_FAILED(firstCRSeg) {
                continue;
            }
            track_idx_t startTrack = staff2track(staffIdx);
            track_idx_t endTrack = startTrack + VOICES;
            for (track_idx_t track = startTrack; track < endTrack; ++track) {
                EngravingItem* item = firstCRSeg->element(track);
                if (item && item->isRest() && toRest(item)->isFullMeasureRest()) {
                    fullMeasureRest = toRest(item);
                    break;
                }
            }

            if (!fullMeasureRest) {
                continue;
            }

            double xRest = fullMeasureRest->pagePos().x() - system->pagePos().x();
            Shape restShape = fullMeasureRest->shape().translate(PointF(xRest, fullMeasureRest->y()));

            Shape measureShape;
            for (const Segment& segment : measure->segments()) {
                double xSegment = segment.pagePos().x() - system->pagePos().x();
                measureShape.add(segment.staffShape(staffIdx).translated(PointF(xSegment, 0.0)));
            }
            measureShape.remove_if([fullMeasureRest] (const ShapeElement& shapeEl) {
                const EngravingItem* shapeItem = shapeEl.item();
                return shapeItem && (shapeItem == fullMeasureRest || shapeItem->isBarLine());
            });

            const double spatium = fullMeasureRest->spatium();
            const double lineDist = fullMeasureRest->staff()->lineDistance(fullMeasureRest->tick()) * spatium;
            const double minHorizontalDistance = 4 * spatium;
            const double minVertClearance = 0.75 * spatium;

            bool alignAbove = fullMeasureRest->voice() == 0;
            double verticalClearance = alignAbove ? restShape.verticalClearance(measureShape, minHorizontalDistance)
                                       : measureShape.verticalClearance(restShape, minHorizontalDistance);

            if (verticalClearance < minVertClearance) {
                double diff = minVertClearance - verticalClearance;
                int stepMoves = std::ceil(diff / lineDist);
                double yMove = stepMoves * lineDist;
                fullMeasureRest->mutldata()->moveY(alignAbove ? -yMove : yMove);
                updateSymbol(fullMeasureRest, fullMeasureRest->mutldata());
            }
        }
    }
}

void RestLayout::fillShape(const Rest* item, Rest::LayoutData* ldata)
{
    Shape shape(Shape::Type::Composite);

    if (!item->isGap() && !item->shouldNotBeDrawn()) {
        shape.add(ChordLayout::chordRestShape(item));
        shape.add(item->symBbox(ldata->sym), item);
        for (const NoteDot* dot : item->dotList()) {
            if (dot->addToSkyline()) {
                shape.add(item->symBbox(SymId::augmentationDot).translated(dot->pos()), dot);
            }
        }
    }

    for (const EngravingItem* e : item->el()) {
        if (e->addToSkyline()) {
            shape.add(e->shape().translate(e->pos()));
        }
    }

    ldata->setShape(shape);
}

void RestLayout::fillShape(const MMRest* item, MMRest::LayoutData* ldata, const LayoutConfiguration& conf)
{
    Shape shape(Shape::Type::Composite);

    double vStrokeHeight = conf.styleMM(Sid::mmRestHBarVStrokeHeight);
    shape.add(RectF(0.0, -(vStrokeHeight * .5), ldata->restWidth, vStrokeHeight), item);
    if (item->shouldShowNumber()) {
        shape.add(item->numberRect().translated(item->numberPos()), item);
    }

    ldata->setShape(shape);
}

int RestLayout::computeNaturalLine(int lines)
{
    int line = (lines % 2) ? floor(double(lines) / 2) : ceil(double(lines) / 2);
    return line;
}

int RestLayout::computeVoiceOffset(const Rest* item, Rest::LayoutData* ldata)
{
    ldata->mergedRests.clear();
    Segment* s = item->segment();
    Measure* measure = item->measure();
    bool offsetVoices = s && measure && (item->voice() > 0 || measure->hasVoices(item->staffIdx(), item->tick(), item->actualTicks()));
    if (offsetVoices && item->voice() == 0) {
        // do not offset voice 1 rest if there exists a matching invisible rest in voice 2;
        EngravingItem* e = s->element(item->track() + 1);
        if (e && e->isRest() && !e->visible() && !toRest(e)->isGap()) {
            Rest* r = toRest(e);
            if (r->globalTicks() == item->globalTicks()) {
                offsetVoices = false;
            }
        }
    }

    if (offsetVoices && item->voice() < 2) {
        // in slash notation voices 1 and 2 are not offset outside the staff
        // if the staff contains slash notation then only offset rests in voices 3 and 4
        track_idx_t baseTrack = item->staffIdx() * VOICES;
        for (voice_idx_t v = 0; v < VOICES; ++v) {
            EngravingItem* e = s->element(baseTrack + v);
            if (e && e->isChord() && toChord(e)->slash()) {
                offsetVoices = false;
                break;
            }
        }
    }

    if (offsetVoices && item->staff()->shouldMergeMatchingRests()) {
        // automatically merge matching rests if nothing in any other voice
        // this is not always the right thing to do do, but is useful in choral music
        // and can be enabled via a staff property
        bool matchFound = false;
        track_idx_t baseTrack = item->staffIdx() * VOICES;
        for (voice_idx_t v = 0; v < VOICES; ++v) {
            if (v == item->voice()) {
                continue;
            }
            EngravingItem* e = s->element(baseTrack + v);
            // try to find match in any other voice
            if (e) {
                if (e->isRest()) {
                    Rest* r = toRest(e);
                    if (r->globalTicks() == item->globalTicks() && r->durationType() == item->durationType()) {
                        matchFound = true;
                        ldata->mergedRests.push_back(r);
                        continue;
                    }
                }
                // no match found; no sense looking for anything else
                matchFound = false;
                break;
            }
        }
        if (matchFound) {
            offsetVoices = false;
        }
    }

    if (!offsetVoices) {
        return 0;
    }

    bool up = item->voice() == 0 || item->voice() == 2;
    int upSign = up ? -1 : 1;
    int voiceLineOffset = item->style().styleB(Sid::multiVoiceRestTwoSpaceOffset) ? 2 : 1;

    return voiceLineOffset * upSign;
}

int RestLayout::computeWholeOrBreveRestOffset(const Rest* item, int voiceOffset, int lines)
{
    int lineMove = 0;
    if (item->isWholeRest()) {
        bool moveToLineAbove = (lines > 5)
                               || ((lines > 1 || voiceOffset == -1 || voiceOffset == 2) && !(voiceOffset == -2 || voiceOffset == 1));
        if (moveToLineAbove) {
            lineMove = -1;
        }
    } else if (item->isBreveRest() && lines == 1) {
        lineMove = 1;
    }

    return lineMove;
}

void RestLayout::updateSymbol(const Rest* item, Rest::LayoutData* ldata)
{
    Fraction t = item->tick();
    Staff* st = item->staff();
    double lineDistance = st->lineDistance(t) * item->spatium();
    int lines = st->lines(t);

    double y = item->pos().y();
    int line = floor(y / lineDistance);

    ldata->sym = item->getSymbol(item->durationType().type(), line, lines);
}
} // namespace mu::engraving::rendering::score
