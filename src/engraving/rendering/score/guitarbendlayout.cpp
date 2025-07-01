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

#include "tlayout.h"
#include "guitarbendlayout.h"

#include "../../dom/chord.h"
#include "../../dom/guitarbend.h"
#include "../../dom/measure.h"
#include "../../dom/note.h"
#include "../../dom/score.h"
#include "../../dom/segment.h"
#include "../../dom/staff.h"
#include "../../dom/system.h"
#include "../../dom/tie.h"

#include "../draw/types/transform.h"

using namespace mu::engraving;
using namespace muse::draw;
using namespace mu::engraving::rendering::score;

void GuitarBendLayout::updateSegmentsAndLayout(SLine* item, LayoutContext& ctx)
{
    Note* startNote = toNote(item->startElement());
    Note* endNote = toNote(item->endElement());
    if (!startNote || !endNote) {
        return;
    }

    System* system1 = startNote->findMeasure()->system();
    System* system2 = endNote->findMeasure()->system();

    if (!system1 || !system2) {
        return;
    }

    unsigned int segmentsNeeded = system1 == system2 ? 1 : 2;
    size_t segmentCount = item->spannerSegments().size();
    if (segmentCount != segmentsNeeded) {
        item->fixupSegments(segmentsNeeded, [item](System* parent) { return item->createLineSegment(parent); });
    }

    item->frontSegment()->setSystem(system1);
    if (segmentsNeeded == 1) {
        item->frontSegment()->setSpannerSegmentType(SpannerSegmentType::SINGLE);
    } else {
        item->frontSegment()->setSpannerSegmentType(SpannerSegmentType::BEGIN);
        item->backSegment()->setSystem(system2);
        item->backSegment()->setSpannerSegmentType(SpannerSegmentType::END);
    }

    for (SpannerSegment* segment : item->spannerSegments()) {
        segment->setTrack(item->track());
        TLayout::layoutLineSegment(toLineSegment(segment), ctx);
    }
}

void GuitarBendLayout::layoutStandardStaff(GuitarBendSegment* item, LayoutContext& ctx)
{
    GuitarBend* bend = item->guitarBend();

    if (bend->type() != GuitarBendType::SLIGHT_BEND) {
        layoutAngularBend(item, ctx);
    } else {
        layoutSlightBend(item, ctx);
    }
}

void GuitarBendLayout::layoutAngularBend(GuitarBendSegment* item, LayoutContext& ctx)
{
    GuitarBend* bend = item->guitarBend();

    computeUp(bend);
    computeIsInside(bend);

    Note* startNote = bend->startNote();
    Note* endNote = bend->endNote();
    if (!startNote || !endNote) {
        return;
    }

    if (bend->type() == GuitarBendType::PRE_BEND && !startNote->headHasParentheses()) {
        startNote->setHeadHasParentheses(true, /* addToLinked= */ false, /* generated= */ true);
        startNote->mutldata()->reset();
        TLayout::layoutChord(startNote->chord(), ctx);
    }

    System* startSystem = startNote->chord()->segment()->system();
    System* endSystem = endNote->chord()->segment()->system();
    if (!startSystem || !endSystem) {
        return;
    }

    Chord* startChord = startNote->chord();
    PointF startNotePos = startNote->pos() + startChord->pos() + startChord->segment()->pos() + startChord->measure()->pos();
    Chord* endChord = endNote->chord();
    PointF endNotePos = endNote->pos() + endChord->pos() + endChord->segment()->pos() + endChord->measure()->pos();

    const bool up = bend->ldata()->up();
    const int upSign = up ? -1 : 1;
    const bool isInside = bend->ldata()->isInside();
    const double spatium = item->spatium();
    const double verticalPadding = 0.2 * spatium;
    const double horizontalIndent = 0.2 * spatium;
    double heightDiff = endNote->y() - startNote->y();

    PointF startPos;
    PointF endPos;

    if (item->isSingleBeginType()) {
        startPos = startNotePos;

        double xOff = (isInside ? 1 : 0.5) * startNote->width() + horizontalIndent * startNote->mag();
        double yOff = upSign * verticalPadding * startNote->mag();
        if (!isInside) {
            yOff += upSign * 0.5 * startNote->height();
        }

        startPos += PointF(xOff, yOff);
    } else {
        double x = endSystem->firstNoteRestSegmentX(true);
        double y = endNote->y() + upSign * 0.5 * endNote->height() - 0.5 * heightDiff;
        startPos = PointF(x, y);
    }

    if (item->isSingleEndType()) {
        endPos = endNotePos;

        double xOff = (isInside ? 0 : 0.5) * endNote->width() - horizontalIndent * endNote->mag();
        double yOff = upSign * verticalPadding * endNote->mag();
        if (!isInside) {
            yOff += upSign * 0.5 * endNote->height();
        }

        endPos += PointF(xOff, yOff);
    } else {
        double x = startSystem->endingXForOpenEndedLines();
        double y = startNote->y() + upSign * 0.5 * startNote->height() + 0.5 * heightDiff;
        endPos = PointF(x, y);
    }

    avoidBadStaffLineIntersection(item, startPos);
    avoidBadStaffLineIntersection(item, endPos);
    adjustX(item, startPos, endPos, startNote, endNote);

    item->setPos(startPos);
    item->setPos2(endPos - startPos);

    const double vertexHeightMin = 0.75 * spatium;
    const double vertexHeightMax = 2.0 * spatium;

    PointF relEndPoint = endPos - startPos;
    double angle = -atan(relEndPoint.y() / relEndPoint.x());

    double baseLength = relEndPoint.x();
    double minLength = spatium;
    double vertexHeight = vertexHeightMin + 0.1 * (baseLength - minLength);
    vertexHeight = std::min(vertexHeight, vertexHeightMax);

    PointF vertex = 0.5 * relEndPoint;
    PointF vertexDispl = PointF(upSign * vertexHeight * sin(angle), upSign * vertexHeight * cos(angle));
    vertex += vertexDispl;

    PointF vertexAbsCoord = vertex + startPos;
    avoidBadStaffLineIntersection(item, vertexAbsCoord);
    vertex = vertexAbsCoord - startPos;

    item->mutldata()->setVertexPoint(vertex);

    if (item->isSingleBeginType()) {
        startNote->addStartLineAttachPoint(item->pos(), bend);
    }
    if (item->isSingleEndType()) {
        endNote->addEndLineAttachPoint(item->pos() + item->pos2(), bend);
    }

    PainterPath path;
    path.lineTo(vertex + item->vertexPointOff());
    path.lineTo(item->pos2());
    item->mutldata()->setPath(path);

    RectF r = RectF(PointF(0.0, 0.0), vertex + item->vertexPointOff());
    r.unite(RectF(vertex + item->vertexPointOff(), item->pos2()));
    item->setbbox(r);
}

void GuitarBendLayout::computeUp(GuitarBend* item)
{
    // NOTE: at some point, it would be good to find a way to reuse the logic of Tie::computeUp() (M.S.)

    GuitarBend::LayoutData* layoutData = item->mutldata();

    if (item->direction() != DirectionV::AUTO) {
        layoutData->setUp(item->direction() == DirectionV::UP);
        return;
    }

    Measure* measure = item->score()->tick2measure(item->tick());
    if (!measure) {
        return;
    }
    if (measure->hasVoices(item->staffIdx())) {
        layoutData->setUp(item->track() % 2);
        return;
    }

    Note* startN = item->startNote();
    Chord* startChord = startN->chord();
    Note* endN = item->endNote();
    Chord* endChord = endN->chord();

    if (startChord->up() != endChord->up()) {
        layoutData->setUp(true);
        return;
    }

    if (startChord->notes().size() == 1 && startChord->stem()) {
        layoutData->setUp(!startChord->up());
        return;
    }

    if (endChord->notes().size() == 1 && endChord->stem()) {
        layoutData->setUp(!endChord->up());
        return;
    }

    int bendCount = 0;
    int indexOfThis = 0;
    for (Note* note : (startChord->notes().size() > 1 ? startChord->notes() : endChord->notes())) {
        for (Spanner* spanner : note->spannerFor()) {
            if (spanner->isGuitarBend()) {
                if (spanner == item) {
                    indexOfThis = bendCount;
                }
                ++bendCount;
            }
        }
    }

    bool up = indexOfThis >= floor(double(bendCount) / 2);
    layoutData->setUp(up);
}

void GuitarBendLayout::computeIsInside(GuitarBend* item)
{
    GuitarBend::LayoutData* layoutData = item->mutldata();
    Note* startN = item->startNote();
    Chord* startChord = startN->chord();
    Note* endN = item->endNote();
    Chord* endChord = endN->chord();

    if (startChord->notes().size() <= 2 || endChord->notes().size() <= 2) {
        layoutData->setIsInside(false);
        return;
    }

    if (startN == startChord->upNote() || startN == startChord->downNote()
        || endN == endChord->upNote() || endN == endChord->downNote()) {
        layoutData->setIsInside(false);
        return;
    }

    layoutData->setIsInside(true);
}

void GuitarBendLayout::avoidBadStaffLineIntersection(GuitarBendSegment* item, PointF& point)
{
    int upSign = item->guitarBend()->ldata()->up() ? -1 : 1;
    double spatium = item->spatium();
    double lineDist = spatium * item->staff()->lineDistance(item->tick());
    double closestStaffLineY = lineDist * round(point.y() / lineDist);

    bool isInsideStaff = closestStaffLineY >= 0 && closestStaffLineY <= 4 * spatium;
    if (isInsideStaff && item->autoplace()) {
        const double minLineDist = 0.5 * item->style().styleMM(Sid::staffLineWidth) + 0.15 * spatium;
        double pointLineDist = point.y() - closestStaffLineY;
        if (std::abs(pointLineDist) < minLineDist) {
            point.setY(closestStaffLineY + upSign * minLineDist);
        }
    }
}

void GuitarBendLayout::adjustX(GuitarBendSegment* item, PointF& startPos, PointF& endPos, const Note* startNote, const Note* endNote)
{
    GuitarBend* bend = item->guitarBend();
    GuitarBend::LayoutData* layoutData = bend->mutldata();
    bool up = item->guitarBend()->ldata()->up();
    Chord* startChord = startNote->chord();
    Chord* endChord = endNote->chord();

    bool adjustStart = item->isSingleBeginType() && ((startChord->stem() && startChord->up() == up)
                                                     || (up && startNote != startChord->upNote())
                                                     || (!up && startNote != startChord->downNote()));
    bool adjustEnd = item->isSingleEndType() && ((endChord->stem() && endChord->up() == up)
                                                 || (up && endNote != endChord->upNote())
                                                 || (!up && endNote != endChord->downNote()));

    if (!layoutData->isInside() && !adjustStart && !adjustEnd) {
        return;
    }

    const double vertMargin = 0.1 * item->spatium();
    const double padding = 0.2 * item->spatium();

    if (item->isSingleBeginType()) {
        Shape startChordShape = startChord->shape().translate(
            startChord->pos() + startChord->segment()->pos() + startChord->measure()->pos());
        double pointToClear = startChordShape.rightMostEdgeAtHeight(startPos.y() + vertMargin, startPos.y() - vertMargin);
        pointToClear += padding;
        double resultingX = std::max(startPos.x(), pointToClear);
        startPos.setX(resultingX);
    }

    if (item->isSingleEndType()) {
        Shape endChordShape = endChord->shape().translate(
            endChord->pos() + endChord->segment()->pos() + endChord->measure()->pos());
        double pointToClear = endChordShape.leftMostEdgeAtHeight(endPos.y() + vertMargin, endPos.y() - vertMargin);
        pointToClear -= padding;
        double resultingX = std::min(endPos.x(), pointToClear);
        endPos.setX(resultingX);
    }
}

void GuitarBendLayout::layoutSlightBend(GuitarBendSegment* item, LayoutContext&)
{
    GuitarBend* bend = item->guitarBend();
    Note* startNote = bend->startNote();
    assert(startNote);

    Chord* startChord = startNote->chord();
    PointF startNotePos = startNote->pos() + startChord->pos() + startChord->segment()->pos() + startChord->measure()->pos();

    double spatium = item->spatium();
    PointF startPos = startNotePos + PointF(startNote->width() + 0.25 * spatium, -0.25 * spatium);
    PointF endPos(1.25 * spatium, -1.0 * spatium);
    PointF curve(endPos.x(), 0.0);

    item->setPos(startPos);
    item->setPos2(endPos);
    item->mutldata()->setVertexPoint(curve);

    item->mutldata()->setBbox(RectF(PointF(), item->pos2()).normalized());

    PainterPath path;
    path.cubicTo(PointF(0.0, 0.0), curve + item->vertexPointOff(), item->pos2());
    item->mutldata()->setPath(path);
}

void GuitarBendLayout::checkConflictWithOtherBends(GuitarBendSegment* item)
{
    GuitarBend* thisBend = item->guitarBend();
    Note* startNote = thisBend->startNote();
    Chord* startChord = startNote->chord();

    GuitarBend* otherBend = nullptr;
    for (Note* note : startChord->notes()) {
        if (note->string() >= startNote->string()) {
            continue;
        }
        GuitarBend* bendFor = note->bendFor();
        if (bendFor) {
            otherBend = bendFor;
            break;
        }
    }

    if (!otherBend) {
        return;
    }

    if (otherBend->ldata()->bendDigit() != thisBend->ldata()->bendDigit()) {
        if (thisBend->type() != GuitarBendType::PRE_BEND) {
            GuitarBendSegment::LayoutData* ldata = item->mutldata();
            double spatium = item->spatium();
            PointF endPointMove(spatium, spatium);
            item->setPos2(item->pos2() + endPointMove);
            item->mutldata()->arrow().translate(endPointMove);
            ldata->setVertexPoint(ldata->vertexPoint() + PointF(endPointMove.x(), 0.0));

            GuitarBendText* text = item->bendText();
            text->mutldata()->setPos(text->ldata()->pos() + endPointMove + PointF(0.35 * text->width(), 0.0));
        }
    }
}

void GuitarBendLayout::layoutTabStaff(GuitarBendSegment* item, LayoutContext& ctx)
{
    GuitarBend* bend = item->guitarBend();
    GuitarBendSegment::LayoutData* ldata = item->mutldata();

    Note* startNote = bend->startNote();
    Note* endNote = bend->endNote();
    if (!startNote || !endNote) {
        return;
    }

    double spatium = bend->spatium();
    const MStyle& style = item->style();
    const double verticalPad = 0.25 * spatium;
    const double distAboveTab = style.styleD(Sid::guitarBendHeightAboveTABStaff) * spatium * item->staff()->lineDistance(item->tick());
    const double arrowWidth = style.styleMM(Sid::guitarBendArrowWidth);
    const double arrowHeight = style.styleMM(Sid::guitarBendArrowHeight);
    const double lineWidth = item->lineWidth();

    PointF startPos = PointF(0.0, 0.0);
    PointF endPos = PointF(0.0, 0.0);
    PointF vertex = PointF(0.0, 0.0);
    PolygonF arrow;

    GuitarBend* prevBend = bend->findPrecedingBend();
    GuitarBendSegment* prevBendSeg = prevBend && !prevBend->segmentsEmpty() && !prevBend->isFullRelease()
                                     ? toGuitarBendSegment(prevBend->backSegment()) : nullptr;
    GuitarBendHoldSegment* prevHoldLine = prevBend && prevBend->holdLine() && !prevBend->holdLine()->segmentsEmpty()
                                          ? toGuitarBendHoldSegment(prevBend->holdLine()->backSegment()) : nullptr;

    PointF prevEndPoint = PointF(0.0, 0.0);
    if (prevHoldLine) {
        prevEndPoint = prevHoldLine->pos2() + prevHoldLine->pos();
    } else if (prevBendSeg) {
        const PolygonF& prevArrow = prevBendSeg->ldata()->arrow();
        if (!prevArrow.empty()) {
            prevEndPoint = prevArrow.front() + prevBendSeg->pos();
            prevEndPoint += PointF(0.0, (prevBend->isReleaseBend() ? -0.5 : 0.5) * lineWidth);
        }
    }

    if (item->isSingleBeginType() && !prevEndPoint.isNull()) {
        startPos = prevEndPoint;
    } else {
        startPos = computeStartPos(item, startNote, distAboveTab, verticalPad, arrowHeight);
    }

    endPos = computeEndPos(item, endNote, distAboveTab, verticalPad, arrowHeight, arrowWidth, startPos, prevEndPoint);

    vertex = bend->type() == GuitarBendType::PRE_BEND && !bend->angledPreBend()
             ? 0.5 * (startPos + endPos) : PointF(endPos.x(), startPos.y());

    arrow = bend->isReleaseBend()
            ? arrow << PointF(0, arrowHeight) << PointF(arrowWidth * .5, 0.0) << PointF(-arrowWidth * .5, 0.0)   // arrow-down
            : arrow << PointF(0, -arrowHeight) << PointF(arrowWidth * .5, 0.0) << PointF(-arrowWidth * .5, 0.0); // arrow-up

    if (bend->isReleaseBend() && !endNote->ghost()) {
        endNote->setGhost(true);
        endNote->mutldata()->reset();
        TLayout::layoutChord(endNote->chord(), ctx);
    }

    if (bend->type() != GuitarBendType::SLIGHT_BEND && !bend->isFullRelease()) {
        endNote->setVisible(false);
    }

    item->setPos(startPos);
    item->setPos2(endPos - startPos);
    ldata->setVertexPoint(vertex - startPos);

    arrow.translate(item->pos2());
    item->mutldata()->setArrow(arrow);

    GuitarBendText* guitarBendText = item->bendText();
    guitarBendText->setParent(item);
    guitarBendText->setXmlText(bend->ldata()->bendDigit());
    TLayout::layoutBaseTextBase(toTextBase(guitarBendText), ctx);
    double verticalTextPad = 0.2 * spatium;
    PointF centering(-0.5 * guitarBendText->width(),
                     (bend->isReleaseBend() ? verticalTextPad : -(guitarBendText->height() + verticalTextPad)));
    guitarBendText->mutldata()->setPos(arrow.front() + centering);

    GuitarBendLayout::checkConflictWithOtherBends(item);

    PainterPath path;
    if (bend->angledPreBend()) {
        path.lineTo(ldata->vertexPoint() + item->vertexPointOff());
        path.lineTo(item->pos2());
    } else {
        path.cubicTo(PointF(0.0, 0.0), ldata->vertexPoint() + item->vertexPointOff(), item->pos2());
    }
    ldata->setPath(path);

    RectF r;
    if (muse::RealIsNull(item->pos2().x())) {
        r = RectF(-0.5 * lineWidth, 0.0, lineWidth, arrow.front().y());
    } else {
        r = RectF(PointF(), PointF(arrow[1].x(), arrow[0].y()));
    }
    item->mutldata()->setBbox(r.normalized());
}

PointF GuitarBendLayout::computeStartPos(GuitarBendSegment* item, Note* startNote, double distAboveTab, double verticalPad,
                                         double arrowHeight)
{
    GuitarBend* bend = item->guitarBend();
    Chord* startChord = startNote->chord();
    PointF startNotePos = startNote->pos() + startChord->pos() + startChord->segment()->pos() + startChord->measure()->pos();

    const double spatium = item->spatium();
    const double lineWidth = item->lineWidth();
    const double horizontalIndent = 0.25 * spatium;
    System* system = item->system();

    PointF startPos;

    if (bend->type() == GuitarBendType::PRE_BEND && !bend->angledPreBend()) {
        startPos = startNotePos;
        startPos += PointF(0.5 * startNote->width(), -0.5 * startNote->height() - verticalPad);
        return startPos;
    }

    if (bend->isReleaseBend()) {
        if (item->isSingleBeginType()) {
            startPos.setX(startNotePos.x());
            startPos.setY(-distAboveTab - arrowHeight + 0.5 * lineWidth);
        } else {
            startPos.setX(system->firstNoteRestSegmentX(true));
            startPos.setY(-distAboveTab);
        }
        return startPos;
    }

    if (item->isSingleBeginType()) {
        startPos = startNotePos;
        startPos += PointF(startNote->width() + horizontalIndent, -0.5 * startNote->height() + 0.5 * lineWidth);
    } else {
        startPos.setX(system->firstNoteRestSegmentX(true));
        startPos.setY(0.5 * spatium);
    }

    return startPos;
}

PointF GuitarBendLayout::computeEndPos(GuitarBendSegment* item, Note* endNote, double distAboveTab, double verticalPad, double arrowHeight,
                                       double arrowWidth, const PointF& startPos, const PointF& prevEndPoint)
{
    GuitarBend* bend = item->guitarBend();
    Chord* endChord = endNote->chord();
    PointF endNotePos = endNote->pos() + endChord->pos() + endChord->segment()->pos() + endChord->measure()->pos();

    const MStyle& style = item->style();
    const double partialBendHeight = style.styleMM(Sid::guitarBendPartialBendHeight);
    const double spatium = item->spatium();
    System* system = item->system();

    PointF endPos;

    if (bend->type() == GuitarBendType::PRE_BEND && !bend->angledPreBend()) {
        endPos = PointF(startPos.x(), -distAboveTab);        // TODO: style
        return endPos;
    }

    if (bend->isReleaseBend()) {
        if (item->isSingleEndType()) {
            endPos = endNotePos;
            endPos += PointF(0.5 * endNote->width(), 0.0);
            if (bend->isFullRelease()) {
                endPos.setY(endPos.y() - 0.5 * endNote->height() - arrowHeight - verticalPad);
            } else {
                endPos.setY(startPos.y() + partialBendHeight);
            }
        } else {
            endPos.setX(system->endingXForOpenEndedLines() - 0.5 * arrowWidth);
            endPos.setY(-arrowHeight);
        }
        return endPos;
    }

    if (bend->type() == GuitarBendType::SLIGHT_BEND) {
        const double slightBendWidth = 1.25 * spatium;
        endPos = startPos + PointF(slightBendWidth, 0.0);
    } else {
        if (item->isSingleEndType()) {
            endPos = endNotePos;
            endPos += PointF(0.5 * endNote->width(), 0.0);
        } else {
            endPos.setX(system->endingXForOpenEndedLines() - 0.5 * arrowWidth);
        }
    }
    endPos = PointF(endPos.x(), prevEndPoint.isNull() ? -distAboveTab : startPos.y() - partialBendHeight);             // TODO: style
    endPos.setX(std::max(endPos.x(), startPos.x() + 0.75 * spatium));

    return endPos;
}

void GuitarBendLayout::layoutHoldLine(GuitarBendHoldSegment* item)
{
    if (!item->system()) {
        return;
    }

    GuitarBend* startBend = item->guitarBendHold()->guitarBend();
    if (!startBend || startBend->segmentsEmpty()) {
        return;
    }

    const double spatium = item->spatium();
    const double lineWidth = item->lineWidth();

    PointF startPos = PointF();
    PointF endPos = PointF();

    Note* endNote = item->guitarBendHold()->endNote();

    Chord* endChord = endNote->chord();
    PointF endNotePos = endNote->pos() + endChord->pos() + endChord->segment()->pos() + endChord->measure()->pos();

    GuitarBendSegment* startBendSegment = nullptr;
    if (!startBend->isFullRelease() && !startBend->segmentsEmpty()) {
        startBendSegment = toGuitarBendSegment(startBend->backSegment());
    } else {
        return;
    }
    item->setStartBendSeg(startBendSegment);

    GuitarBendSegment* endBendSegment = nullptr;
    if (item->isSingleEndType()) {
        GuitarBend* bend = endNote->bendFor();
        if (bend && !bend->segmentsEmpty()) {
            endBendSegment = toGuitarBendSegment(bend->frontSegment());
        }
    }
    item->setEndBendSeg(endBendSegment);

    if (item->isSingleBeginType()) {
        startPos = startBendSegment->pos() + startBendSegment->ldata()->arrow().front();
        double gap = item->dashLength() * lineWidth;
        startPos += PointF(gap, (startBend->isReleaseBend() ? -0.5 : +0.5) * lineWidth);
    } else {
        startPos.setX(item->system()->firstNoteRestSegmentX(true));
        startPos.setY(item->guitarBendHold()->frontSegment()->pos().y());
    }

    if (item->isSingleEndType()) {
        endPos.setX(endNotePos.x() + (endBendSegment ? 0.0 : endNote->chord()->segment()->width() - 1.5 * spatium));
    } else {
        endPos.setX(item->system()->endingXForOpenEndedLines());
    }

    endPos.setY(startPos.y());

    item->setPos(startPos);
    item->setPos2(endPos - startPos);

    RectF r(0.0, -0.5 * lineWidth, item->pos2().x(), lineWidth);
    item->mutldata()->setBbox(r);
}
