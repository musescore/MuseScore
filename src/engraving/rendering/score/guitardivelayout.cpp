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
#include "guitardivelayout.h"

#include "dom/guitarbend.h"
#include "dom/note.h"
#include "dom/staff.h"
#include "dom/stafftype.h"
#include "dom/system.h"
#include "dom/whammybar.h"

#include "horizontalspacing.h"
#include "textlayout.h"
#include "tlayout.h"

using namespace mu::engraving;
using namespace muse::draw;
using namespace mu::engraving::rendering::score;

void GuitarDiveLayout::updateDiveSequences(const std::vector<GuitarBend*>& bends, const LayoutContext& ctx)
{
    std::vector<GuitarBend*> dives;
    for (GuitarBend* bend : bends) {
        if ((bend->bendType() == GuitarBendType::DIVE || bend->bendType() == GuitarBendType::PRE_DIVE) && bend->staffType()->isTabStaff()) {
            dives.push_back(bend);
        }
    }

    for (GuitarBend* dive : dives) {
        dive->computeBendAmount();
        dive->mutldata()->resetDiveLevels();
    }

    for (auto riter = dives.rbegin(); riter != dives.rend(); ++riter) {
        GuitarBend* dive = *riter;
        if (!dive->ldata()->diveLevels().empty()) {
            continue;
        }

        // On-staff vs above-staff layout
        DirectionV tabPos = dive->diveTabPos();

        std::set<int> diveLevels;
        std::unordered_set<GuitarBend*> diveSequence;

        diveLevels.insert(dive->totBendAmountIncludingPrecedingBends());
        diveSequence.insert(dive);

        // Consider dives part of the same sequence if they are directly connected
        // or if they are spanned by the same whammy bar line

        if (WhammyBar* whammyBar = dive->findOverlappingWhammyBar(dive->tick(), dive->tick2())) {
            for (auto riter2 = riter + 1; riter2 != dives.rend(); ++riter2) {
                GuitarBend* earlierDive = *riter2;
                if (earlierDive->tick() >= whammyBar->tick()) {
                    diveLevels.insert(earlierDive->totBendAmountIncludingPrecedingBends());
                    diveSequence.insert(earlierDive);
                    if (earlierDive->diveTabPos() != DirectionV::AUTO) {
                        tabPos = earlierDive->diveTabPos();
                    }
                }
            }
        }

        GuitarBend* prevDive = dive->findPrecedingBend();
        while (prevDive) {
            diveLevels.insert(prevDive->totBendAmountIncludingPrecedingBends());
            diveSequence.insert(prevDive);
            if (prevDive->diveTabPos() != DirectionV::AUTO) {
                tabPos = prevDive->diveTabPos();
            }
            prevDive = prevDive->findPrecedingBend();
        }

        bool layoutAboveStaff = tabPos == DirectionV::UP || (tabPos == DirectionV::AUTO && ctx.conf().styleB(Sid::guitarDivesAboveStaff));

        for (GuitarBend* d : diveSequence) {
            d->mutldata()->setDiveLevels(diveLevels);
            d->mutldata()->setAboveStaff(layoutAboveStaff);
        }
    }
}

void GuitarDiveLayout::layoutDiveTabStaff(GuitarBendSegment* item, LayoutContext& ctx)
{
    GuitarBend* bend = item->guitarBend();
    if (bend->bendType() == GuitarBendType::DIP) {
        layoutDip(item, ctx);
        return;
    }
    if (bend->bendType() == GuitarBendType::SCOOP) {
        layoutScoop(item);
        return;
    }

    GuitarBendSegment::LayoutData* ldata = item->mutldata();

    Note* startNote = bend->startNote();
    Note* endNote = bend->endNote();
    if (!startNote || !endNote) {
        return;
    }

    Chord* startChord = startNote->chord();
    Chord* endChord = endNote->chord();
    if (startNote->string() != startChord->upString() || endNote->string() != endChord->upString()) {
        return; // On TAB only one dive line can exist and only from the top note
    }

    if (bend->bendType() == GuitarBendType::PRE_DIVE) {
        bool alignToGrace = ctx.conf().styleB(Sid::alignPreBendAndPreDiveToGraceNote);
        for (Note* note : startChord->notes()) {
            note->setVisible(alignToGrace);
        }
        for (Note* note : endChord->notes()) {
            note->setVisible(!alignToGrace);
        }
    } else if (bend->isFullReleaseDive() || item->isEndType()) {
        for (Note* note : endChord->notes()) {
            note->setVisible(true);
            note->setGhost(true);
            note->mutldata()->reset();
        }
        TLayout::layoutChord(endNote->chord(), ctx);
    } else if (!bend->isFullReleaseDive() || !ctx.conf().styleB(Sid::showFretOnFullBendRelease)) {
        for (Note* note : endChord->notes()) {
            note->setVisible(false);
        }
    }

    bool aboveStaff = bend->ldata()->aboveStaff();
    if (aboveStaff) {
        item->setPos(computeStartPosAboveStaff(item, ctx));
        item->setPos2(computeEndPosAboveStaff(item, ctx) - item->pos());
    } else {
        item->setPos(computeStartPosOnStaff(item, ctx));
        item->setPos2(computeEndPosOnStaff(item, ctx) - item->pos());
    }

    GuitarBendText* bendText = item->bendText();
    if (item->isSingleEndType()) {
        bendText->setParent(item);
        bendText->setXmlText(bend->ldata()->bendDigit());
        TextLayout::layoutBaseTextBase(bendText, ctx);
        double verticalTextPad = 0.35 * item->spatium();
        PointF bendTextPos = item->pos2();
        if (aboveStaff) {
            if (bend->bendType() == GuitarBendType::PRE_DIVE && bend->bendAmountInQuarterTones() < 0) {
                bendTextPos = PointF(); // If predive is downwards the text must be above the start
            }
            bendTextPos += PointF(-0.5 * bendText->width(), -bendText->height() - verticalTextPad);
        } else {
            bendTextPos += PointF(-0.5 * bendText->width(),
                                  item->pos().y() + item->pos2().y() > 0 ? verticalTextPad : -bendText->height() - verticalTextPad);
        }
        bendText->setPos(bendTextPos);
    } else {
        bendText->mutldata()->reset();
    }

    PainterPath path;
    if (bend->isSlack()) {
        const double slackLineLen = 2 * item->spatium();
        PointF vertexPoint = item->pos2() - PointF(0.5 * slackLineLen, 0.0);
        path.lineTo(vertexPoint);
        item->setPos2(item->pos2() + PointF(0.5 * slackLineLen, 0.0));
    }
    path.lineTo(item->pos2());
    ldata->setPath(path);

    const double lineWidth = item->lineWidth();
    RectF r = RectF(PointF(), item->pos2()).normalized();
    if (muse::RealIsNull(r.height())) {
        r.setHeight(lineWidth);
    }
    if (muse::RealIsNull(r.width())) {
        r.setWidth(lineWidth);
    }
    item->mutldata()->setBbox(r);
}

PointF GuitarDiveLayout::computeStartPosOnStaff(GuitarBendSegment* item, LayoutContext& ctx)
{
    GuitarBend* bend = item->guitarBend();

    if (bend->bendType() == GuitarBendType::PRE_DIVE) {
        Note* note = ctx.conf().styleB(Sid::alignPreBendAndPreDiveToGraceNote) ? bend->startNote() : bend->endNote();
        if (bend->bendAmountInQuarterTones() > 0) {
            note = note->chord()->upNote();
        } else {
            note = note->chord()->downNote();
        }
        PointF notePos = note->systemPos();
        RectF noteBbox = note->ldata()->bbox();
        double verticalPadding = 0.25 * item->spatium();
        notePos += PointF(0.5 * noteBbox.width(), bend->bendAmountInQuarterTones() > 0
                          ? noteBbox.top() - verticalPadding
                          : noteBbox.bottom() + verticalPadding);
        return notePos;
    }

    LineSegment* prevSegment = findPrevHoldOrBendSegment(item, /*excludeFullReleaseDive*/ true);
    if (prevSegment) {
        return prevSegment->pos() + prevSegment->pos2();
    }

    PointF startPos;
    Note* startNote = bend->startNote();
    if (item->isSingleBeginType()) {
        startPos = startNote->systemPos();
        double horizontalIndent = 0.25 * item->spatium();
        startPos += PointF(startNote->shape().right() + horizontalIndent, -0.5 * startNote->height() + 0.5 * item->lineWidth());
    } else {
        startPos.setX(item->system()->firstNoteRestSegmentX(true));
        startPos.setY(bend->frontSegment()->ldata()->pos().y() + bend->frontSegment()->ipos2().y());
    }

    return startPos;
}

PointF GuitarDiveLayout::computeEndPosOnStaff(GuitarBendSegment* item, LayoutContext&)
{
    GuitarBend* bend = item->guitarBend();
    double spatium = item->spatium();
    Note* endNote = bend->endNote();
    PointF endNotePos = endNote->systemPos();

    if (bend->isFullReleaseDive()) {
        RectF endNoteBbox = endNote->ldata()->bbox();
        double y = endNotePos.y() + endNoteBbox.top() + 0.5 * item->lineWidth();
        double x = item->isSingleEndType() ? endNotePos.x() - endNote->ldata()->shape().left() - 0.25 * spatium
                   : item->system()->endingXForOpenEndedLines();
        return PointF(x, y);
    }

    double x = item->isSingleEndType() ? bend->bendType() == GuitarBendType::PRE_DIVE
               ? item->pos().x() : endNotePos.x() + 0.5 * endNote->width()
               : item->system()->endingXForOpenEndedLines();

    if (bend->bendType() == GuitarBendType::PRE_DIVE) {
        LineSegment* prevSegment = findPrevHoldOrBendSegment(item);
        if (prevSegment && prevSegment->isGuitarBendHoldSegment()) {
            double y = prevSegment->pos().y() + prevSegment->pos2().y();
            return PointF(x, y);
        }
    }

    int diveLevel = bend->totBendAmountIncludingPrecedingBends();
    bool aboveStaff = diveLevel > 0;

    const std::set<int>& diveLevels = bend->ldata()->diveLevels();
    std::deque<int> diveLevelsInThisDirection;
    for (int level : diveLevels) {
        if (aboveStaff && level > 0) {
            diveLevelsInThisDirection.push_back(level);
        } else if (!aboveStaff && level < 0) {
            diveLevelsInThisDirection.push_front(level);
        }
    }

    size_t levelNumber = muse::indexOf(diveLevelsInThisDirection, diveLevel);
    IF_ASSERT_FAILED(levelNumber != muse::nidx) {
        return PointF(x, 0.0);
    }

    const StaffType* staffType = item->staffType();
    IF_ASSERT_FAILED(staffType) {
        return PointF(x, 0.0);
    }
    int staffLines = staffType->lines();
    double lineDist = staffType->lineDistance().toMM(spatium);

    if (!item->isSingleEndType()) {
        double y = aboveStaff ? -lineDist : staffLines * lineDist;
        return PointF(x, y);
    }

    double zeroDiveLevel = aboveStaff ? -1.5 * lineDist : lineDist * (staffLines + 0.5);
    double increment = spatium;

    double y = zeroDiveLevel + (aboveStaff ? -1 : 1) * (increment * levelNumber);

    return PointF(x, y);
}

PointF GuitarDiveLayout::computeStartPosAboveStaff(GuitarBendSegment* item, LayoutContext& ctx)
{
    GuitarBend* bend = item->guitarBend();

    LineSegment* prevSegment = findPrevHoldOrBendSegment(item);
    if (prevSegment) {
        return prevSegment->pos() + prevSegment->pos2();
    }

    PointF startPos;

    if (item->isSingleBeginType()) {
        Note* note = bend->bendType() == GuitarBendType::PRE_BEND && !ctx.conf().styleB(Sid::alignPreBendAndPreDiveToGraceNote)
                     ? bend->endNote() : bend->startNote();
        startPos.setX(note->systemPos().x() + 0.5 * note->width());

        const StaffType* staffType = item->staffType();
        IF_ASSERT_FAILED(staffType) {
            startPos.setY(0.0);
        } else {
            double spatium = item->spatium();
            double lineDist = staffType->lineDistance().toMM(spatium);
            const double baseLine = -1.5 * lineDist;
            const double increment = spatium;
            std::set<int> diveLevels = bend->mutldata()->diveLevels();
            diveLevels.insert(0); // Because for dives above the staff we need the zero level too
            size_t zeroLevel = muse::indexOf(diveLevels, 0);
            double totIncrement = increment * zeroLevel;
            startPos.setY(baseLine - totIncrement);
        }
    } else {
        startPos.setX(item->system()->firstNoteRestSegmentX(true));
        startPos.setY(bend->frontSegment()->ldata()->pos().y());
    }

    return startPos;
}

PointF GuitarDiveLayout::computeEndPosAboveStaff(GuitarBendSegment* item, LayoutContext&)
{
    GuitarBend* bend = item->guitarBend();
    double spatium = item->spatium();
    Note* endNote = bend->endNote();
    PointF endNotePos = endNote->systemPos();

    double x = item->isSingleEndType() ? bend->bendType() == GuitarBendType::PRE_DIVE
               ? item->pos().x() : endNotePos.x() + 0.5 * endNote->width()
               : item->system()->endingXForOpenEndedLines();

    LineSegment* prevSegment = findPrevHoldOrBendSegment(item);
    if (bend->bendType() == GuitarBendType::PRE_DIVE) {
        if (prevSegment && prevSegment->isGuitarBendHoldSegment()) {
            double y = prevSegment->pos().y() + prevSegment->pos2().y();
            return PointF(x, y);
        }
    }

    int endingLevel = bend->totBendAmountIncludingPrecedingBends();
    int startingLevel = endingLevel - bend->bendAmountInQuarterTones();

    std::set<int> diveLevels = bend->ldata()->diveLevels();
    diveLevels.insert(0); // Because for dives above the staff we need the zero level too

    size_t endingIdx = muse::indexOf(diveLevels, endingLevel);
    size_t startingIdx = muse::indexOf(diveLevels, startingLevel);
    IF_ASSERT_FAILED(endingIdx != muse::nidx && startingIdx != muse::nidx && endingIdx != startingIdx) {
        return PointF(x, 0.0);
    }

    int steps = static_cast<int>(endingIdx) - static_cast<int>(startingIdx);
    double increment = spatium;

    double y = item->pos().y() - steps * increment;

    return PointF(x, y);
}

LineSegment* GuitarDiveLayout::findPrevHoldOrBendSegment(GuitarBendSegment* item, bool excludeFullReleaseDive)
{
    GuitarBend* bend = item->guitarBend();
    GuitarBend* prevBend = item->guitarBend()->findPrecedingBend();
    if (!prevBend) {
        return nullptr;
    }

    GuitarBendHold* hold = prevBend->holdLine();
    if (hold && !hold->segmentsEmpty() && prevBend->endNote() != bend->startNote() && hold->backSegment()->system() == item->system()) {
        return hold->backSegment();
    }

    if (!prevBend->segmentsEmpty() && !(excludeFullReleaseDive && prevBend->isFullReleaseDive())
        && prevBend->backSegment()->system() == item->system()) {
        return prevBend->backSegment();
    }

    return nullptr;
}

void GuitarDiveLayout::layoutDip(GuitarBendSegment* item, LayoutContext& ctx)
{
    GuitarBend* bend = item->guitarBend();
    Note* startNote = bend->startNote();
    IF_ASSERT_FAILED(startNote) {
        return;
    }

    PointF notePos = startNote->systemPos();
    double centerX = notePos.x() + 0.5 * startNote->width();
    double spatium = item->spatium();
    double baseY = std::min(-spatium, notePos.y() - 0.5 * startNote->height() - spatium);

    const double vHeight = 2 * spatium;
    const double vWidth = 2 * spatium;

    bool upwards = bend->bendAmountInQuarterTones() > 0;
    PointF start = PointF(centerX - 0.5 * vWidth, upwards ? baseY : baseY - vHeight);
    PointF vertex = PointF(centerX, upwards ? baseY - vHeight : baseY) - start;
    PointF end = PointF(centerX + 0.5 * vWidth, upwards ? baseY : baseY - vHeight) - start;

    item->setPos(start);
    item->setPos2(end);

    PainterPath path;
    path.lineTo(vertex);
    path.lineTo(end);
    item->mutldata()->setPath(path);

    RectF bbox = RectF(PointF(), vertex).normalized().united(RectF(vertex, end).normalized());
    item->mutldata()->setBbox(bbox);

    GuitarBendText* bendText = item->bendText();
    bendText->setParent(item);
    bendText->setXmlText(bend->ldata()->bendDigit());
    TextLayout::layoutBaseTextBase(bendText, ctx);
    double verticalTextPad = 0.35 * item->spatium();
    bendText->setPos(PointF(vertex.x() - 0.5 * bendText->width(), bbox.top() - verticalTextPad - bendText->height()));
}

void GuitarDiveLayout::layoutScoop(GuitarBendSegment* item)
{
    GuitarBend* bend = item->guitarBend();
    Note* startNote = bend->startNote();
    IF_ASSERT_FAILED(startNote) {
        return;
    }

    const SymId scoopSym = SymId::guitarVibratoBarScoop;
    RectF bbox = item->symBbox(scoopSym);
    item->setbbox(bbox);

    PointF pos = startNote->systemPos();
    const double spatium = item->spatium();
    const double leftPad = 0.25 * spatium;
    const double downPad = 0.35 * spatium;

    double yMove = bbox.height();
    if (item->staffType() && item->staffType()->isTabStaff()) {
        yMove -= 0.5 * startNote->height();
    } else {
        yMove += startNote->line() % 2 ? 0.0 : downPad;
    }

    Shape chordShape = startNote->chord()->shape();
    chordShape.removeTypes({ ElementType::GUITAR_BEND, ElementType::GUITAR_BEND_SEGMENT, ElementType::BEAM });

    double xMove = -HorizontalSpacing::minHorizontalDistance(Shape(bbox.translated(0.0, startNote->y())), chordShape, spatium) - leftPad;

    startNote->chord()->segment()->staffShape(startNote->staffIdx()).add(bbox.translated(xMove, yMove + startNote->y()));

    item->setPos(pos + PointF(xMove, yMove));
    item->setPos2(PointF());
}
