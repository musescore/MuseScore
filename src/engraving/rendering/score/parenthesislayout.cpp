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

#include "parenthesislayout.h"

#include "dom/chord.h"
#include "dom/ledgerline.h"
#include "dom/measure.h"
#include "dom/note.h"
#include "dom/parenthesis.h"
#include "dom/segment.h"
#include "dom/slurtie.h"
#include "dom/staff.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

void ParenthesisLayout::layoutParenthesis(Parenthesis* item, Parenthesis::LayoutData* ldata, const LayoutContext& ctx)
{
    ldata->setPos(PointF());
    ldata->reset();
    ldata->path.reset();

    setLayoutValues(item, ldata, ctx);

    createCurveAndShape(item, ldata, ctx);
}

void ParenthesisLayout::createCurveAndShape(Parenthesis* item, Parenthesis::LayoutData* ldata, const LayoutContext& ctx)
{
    const double spatium = item->spatium();
    const double mag = item->mag();
    const bool leftBracket = item->direction() == DirectionH::LEFT;

    double startY = ldata->startY;
    double height = ldata->height;
    double xPos = ldata->pos().x();

    const double heightInSpatium = height / spatium;
    const double shoulderYOffset = 0.2 * height;
    const double thickness = height / 60 * mag; // 0.1sp for a height of 6sp
    ldata->thickness.set_value(thickness);
    const double shoulderX = 0.2 * height * mag;

    PointF start = PointF(xPos, startY);
    const PointF end = PointF(xPos, start.y() + height);
    const PointF endNormalised = end - start;

    const int direction = leftBracket ? -1 : 1;
    const double shoulderForX = direction * shoulderX + thickness * direction;
    const double shoulderBackX = direction * shoulderX + thickness * direction * -1;

    const PointF bezier1for = PointF(shoulderForX, shoulderYOffset);
    const PointF bezier2for = PointF(shoulderForX, endNormalised.y() - shoulderYOffset);
    const PointF bezier1back = PointF(shoulderBackX, endNormalised.y() - shoulderYOffset);
    const PointF bezier2back = PointF(shoulderBackX, shoulderYOffset);

    PainterPath path = PainterPath();
    path.moveTo(PointF());
    path.cubicTo(bezier1for, bezier2for, endNormalised);
    path.cubicTo(bezier1back, bezier2back, PointF());

    ldata->path = path;

    // Fill shape
    Shape shape(Shape::Type::Composite);

    PointF startPoint = PointF();
    double midThickness = 2 * thickness;
    int nbShapes = round(5.0 * heightInSpatium);
    nbShapes = std::clamp(nbShapes, 20, 50);
    PointF bezier1mid = bezier1for - PointF(thickness * direction, 0.0);
    PointF bezier2mid = bezier2for - PointF(thickness * direction, 0.0);
    const CubicBezier b(startPoint, bezier1mid, bezier2mid, endNormalised);
    for (int i = 1; i <= nbShapes; i++) {
        double percent = pow(sin(0.5 * M_PI * (double(i) / double(nbShapes))), 2);
        const PointF point = b.pointAtPercent(percent);
        RectF re = RectF(startPoint, point).normalized();
        double approxThicknessAtPercent = (1 - 2 * std::abs(0.5 - percent)) * midThickness;
        if (re.width() < approxThicknessAtPercent) {
            double adjust = (approxThicknessAtPercent - re.width()) * .5;
            re.adjust(-adjust, 0.0, adjust, 0.0);
        }
        shape.add(re, item);
        startPoint = point;
    }

    ldata->setShape(shape);

    item->setPos(start);
}

void ParenthesisLayout::setLayoutValues(Parenthesis* item, Parenthesis::LayoutData* ldata, const LayoutContext& ctx)
{
    if (!item->parentItem()) {
        return;
    }

    // Set ldata values based on parent
    switch (item->parentItem()->type()) {
    case ElementType::SEGMENT:
        segmentLayout(item, ldata, ctx);
        break;
    case ElementType::NOTE:
        noteLayout(item, ldata, ctx);
        break;
    default:
        defaultLayout(item, ldata, ctx);
        break;
    }
}

void ParenthesisLayout::segmentLayout(Parenthesis* item, Parenthesis::LayoutData* ldata, const LayoutContext& ctx)
{
    const Staff* staff = item->staff();
    const Fraction tick = item->tick();
    const Fraction tickPrev = tick - Fraction::eps();
    const StaffType* st = staff->staffType(tick);
    const StaffType* stPrev = !tickPrev.negative() ? item->staff()->staffType(tickPrev) : nullptr;
    const double spatium = item->spatium();

    const Segment* seg = item->segment();
    const bool isClefSeg = seg ? seg->isType(SegmentType::ClefType) : false;
    if (isClefSeg && seg->rtick() == seg->measure()->ticks()) {
        double offset = st->yoffset().val() - (stPrev ? stPrev->yoffset().val() : 0);
        ldata->startY.mut_value() += offset * spatium;
    }
}

void ParenthesisLayout::noteLayout(Parenthesis* item, Parenthesis::LayoutData* ldata, const LayoutContext& ctx)
{
    Note* note = toNote(item->parentItem());
    Chord* chord = note->chord();
    const Staff* staff = note->staff();
    const StaffType* staffType = staff->staffTypeForElement(note);
    bool isTabStaff = staffType && staffType->isTabStaff();

    ldata->setMag(note->mag());
    Shape noteShape = note->shape();
    noteShape.remove_if([item](ShapeElement& s) { return s.item() == item || s.item()->isBend() || s.item()->isParenthesis(); });
    LedgerLine* ledger = (note->line() < -1 || note->line() > note->staff()->lines(note->tick())) && !chord->ledgerLines().empty()
                         ? chord->ledgerLines().front() : nullptr;
    if (ledger) {
        noteShape.add(ledger->shape().translate(ledger->pos() - note->pos()));
    }
    double right = noteShape.right();
    double left = noteShape.left();

    ldata->startY = noteShape.top() - 0.25 * item->spatium();
    ldata->height = noteShape.bbox().height() + 0.5 * item->spatium();

    double parenthesisPadding = ctx.conf().styleMM(Sid::bracketedAccidentalPadding) * note->mag();

    if (item->direction() == DirectionH::LEFT) {
        ldata->setPosX(-left - item->width() - parenthesisPadding);
    } else if (item->direction() == DirectionH::RIGHT) {
        if (isTabStaff) {
            const Staff* st = note->staff();
            const StaffType* tab = st->staffTypeForElement(note);
            right = note->tabHeadWidth(tab);
        }

        ldata->setPosX(right + parenthesisPadding);
    }
}

void ParenthesisLayout::defaultLayout(Parenthesis* item, Parenthesis::LayoutData* ldata, const LayoutContext& ctx)
{
    const double spatium = item->spatium();
    EngravingItem* parent = item->parentItem();
    RectF bbox = parent->ldata()->bbox();

    ldata->startY = bbox.top() - 0.25 * spatium;
    ldata->height = bbox.height() + 0.5 * spatium;
    const double PADDING = spatium * 0.2;
    ldata->setPosX(item->direction() == DirectionH::RIGHT ? bbox.right() + PADDING : bbox.left() - PADDING);
}
