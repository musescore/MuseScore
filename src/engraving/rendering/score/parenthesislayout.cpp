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
#include "dom/score.h"
#include "dom/timesig.h"
#include "horizontalspacing.h"

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

void ParenthesisLayout::layoutParentheses(const EngravingItem* parent, const LayoutContext& ctx)
{
    // Layout parentheses surrounding an engraving item. Handle padding and placement
    Parenthesis* leftParen = parent->leftParen();
    Parenthesis* rightParen = parent->rightParen();
    if (!(leftParen || rightParen)) {
        return;
    }
    if (leftParen) {
        layoutParenthesis(parent->leftParen(), parent->leftParen()->mutldata(), ctx);
    }

    if (rightParen) {
        layoutParenthesis(parent->rightParen(), parent->rightParen()->mutldata(), ctx);
    }

    bool itemAddToSkyline = parent->addToSkyline();
    Shape dummyItemShape = parent->shape();
    dummyItemShape.remove_if([](ShapeElement& shapeEl) {
        return shapeEl.item() && shapeEl.item()->isParenthesis();
    });

    if (!leftParen || !rightParen) {
        // 1 parenthesis
        Parenthesis* paren = leftParen ? leftParen : rightParen;
        const bool leftBracket = paren->direction() == DirectionH::LEFT;
        double minDist = 0.0;
        // INTERNAL PAREN SPACING - TODO INCORRECT.
        // We are spacing the parent and the paren here. We should not replace the paren type with the parent type
        if (!leftBracket && itemAddToSkyline) {
            // Space against existing item shape
            minDist = HorizontalSpacing::minHorizontalDistance(dummyItemShape, paren->shape().translated(
                                                                   paren->pos()), paren->spatium());
        } else if (itemAddToSkyline) {
            // Space following item shape against this
            minDist = -HorizontalSpacing::minHorizontalDistance(paren->shape().translated(
                                                                    paren->pos()), dummyItemShape, paren->spatium());
        }
        paren->mutldata()->moveX(minDist);

        return;
    }

    // 2 parentheses
    assert(leftParen && rightParen);

    if (!itemAddToSkyline) {
        return;
    }

    const double itemLeftX = parent->pos().x();
    const double itemRightX = itemLeftX + parent->width();

    // INTERNAL PAREN SPACING - TODO INCORRECT
    // We are spacing the parent and the paren here. We should not replace the paren type with the parent type
    const double leftParenPadding = HorizontalSpacing::minHorizontalDistance(leftParen->shape().translated(leftParen->pos()),
                                                                             dummyItemShape, leftParen->spatium());
    leftParen->mutldata()->moveX(-leftParenPadding);
    dummyItemShape.add(leftParen->shape().translate(leftParen->pos() + leftParen->staffOffset()));

    const double rightParenPadding = HorizontalSpacing::minHorizontalDistance(dummyItemShape, rightParen->shape().translated(
                                                                                  rightParen->pos()), rightParen->spatium());
    rightParen->mutldata()->moveX(rightParenPadding);

    // If the right parenthesis has been padded against the left parenthesis, this means the parenthesis -> parenthesis padding distance
    // is larger than the width of the item the parentheses surrounds. In this case, the result is visually unbalanced.  Move both parens
    // to the left (relative to the segment) in order to centre the item: (b  ) -> ( b )
    const double itemWidth = parent->width();
    const double parenPadding = parent->score()->paddingTable()->at(ElementType::PARENTHESIS).at(ElementType::PARENTHESIS);

    if (itemWidth >= parenPadding) {
        return;
    }

    // Move parentheses to place item in the middle
    const double leftParenX = leftParen->pos().x() + leftParen->ldata()->bbox().x() + leftParen->ldata()->thickness;
    const double rightParenX = rightParen->pos().x() + rightParen->ldata()->bbox().x() + rightParen->width()
                               - rightParen->ldata()->thickness;

    const double leftParenToItem = itemLeftX - leftParenX;
    const double itemToRightParen = rightParenX - itemRightX;
    const double parenToItemDist = (leftParenToItem + itemToRightParen) / 2;

    leftParen->mutldata()->moveX(-(parenToItemDist - leftParenToItem));
    rightParen->mutldata()->moveX(-(itemToRightParen - parenToItemDist));
}

void ParenthesisLayout::layoutParenthesis(Parenthesis* item, Parenthesis::LayoutData* ldata, const LayoutContext& ctx)
{
    ldata->setPos(PointF());
    ldata->reset();     // Shouldn't reset startY, height, thickness
    ldata->path.reset();

    setLayoutValues(item, ldata, ctx);

    createPathAndShape(item, ldata);
}

void ParenthesisLayout::createPathAndShape(Parenthesis* item, Parenthesis::LayoutData* ldata)
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
    case ElementType::NOTE:
        setNoteValues(item, ldata, ctx);
        break;
    // height & startY are set in MeasureLayout for clef, timesig and keysig
    // TODO: create generic way of matching height & startY between parentheses on different items
    case ElementType::CLEF:
        setClefValues(item, ldata, ctx);
        break;
    case ElementType::TIMESIG:
        setTimeSigValues(item, ldata, ctx);
        break;
    case ElementType::KEYSIG:
        break;
    default:
        setDefaultValues(item, ldata, ctx);
        break;
    }
}

void ParenthesisLayout::setClefValues(Parenthesis* item, Parenthesis::LayoutData* ldata, const LayoutContext& ctx)
{
    const EngravingItem* parent = item->parentItem();
    const Fraction tick = parent->tick();
    const Measure* measure = item->findMeasure();

    double offset = -parent->pos().y();

    if (!measure || tick != measure->endTick()) {
        ldata->startY.mut_value() += offset;
        return;
    }

    const double spatium = item->spatium();
    const Staff* staff = parent->staff();
    const Fraction tickPrev = tick - Fraction::eps();
    const StaffType* st = staff->staffType(tick);
    const StaffType* stPrev = !tickPrev.negative() ? item->staff()->staffType(tickPrev) : nullptr;

    offset += (st->yoffset().val() - (stPrev ? stPrev->yoffset().val() : 0)) * spatium;
    ldata->startY.mut_value() += offset;
}

void ParenthesisLayout::setTimeSigValues(Parenthesis* item, Parenthesis::LayoutData* ldata, const LayoutContext& ctx)
{
    const TimeSig* parentTs = toTimeSig(item->parentItem());
    if (ctx.conf().styleV(Sid::timeSigPlacement).value<TimeSigPlacement>() == TimeSigPlacement::NORMAL) {
        return;
    }

    double yOffset = -parentTs->pos().y();
    ldata->startY.mut_value() += yOffset;
}

void ParenthesisLayout::setNoteValues(Parenthesis* item, Parenthesis::LayoutData* ldata, const LayoutContext& ctx)
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

void ParenthesisLayout::setDefaultValues(Parenthesis* item, Parenthesis::LayoutData* ldata, const LayoutContext& ctx)
{
    const double spatium = item->spatium();
    EngravingItem* parent = item->parentItem();
    RectF bbox = parent->ldata()->bbox();

    ldata->startY = bbox.top() - 0.25 * spatium;
    ldata->height = bbox.height() + 0.5 * spatium;
    const double PADDING = spatium * 0.2;
    ldata->setPosX(item->direction() == DirectionH::RIGHT ? bbox.right() + PADDING : bbox.left() - PADDING);
}
