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

    if (parent->isNote()) {
        const Note* note = toNote(parent);
        const Chord* chord = note->chord();
        LedgerLine* ledger = (note->line() < -1 || note->line() > note->staff()->lines(note->tick())) && !chord->ledgerLines().empty()
                             ? chord->ledgerLines().front() : nullptr;
        if (ledger) {
            dummyItemShape.add(ledger->shape().translate(ledger->pos() - note->pos() - ledger->staffOffset()));
        }
    }

    if (!leftParen || !rightParen) {
        // 1 parenthesis
        Parenthesis* paren = leftParen ? leftParen : rightParen;
        const bool leftBracket = paren->direction() == DirectionH::LEFT;
        double minDist = 0.0;
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
    double parenPadding = parent->score()->paddingTable().at(ElementType::PARENTHESIS).at(ElementType::PARENTHESIS);
    parenPadding *= leftParen->ldata()->mag();

    if (itemWidth >= parenPadding) {
        return;
    }

    // Move parentheses to place item in the middle
    const double leftParenX = leftParen->pos().x() + leftParen->ldata()->bbox().x() + leftParen->ldata()->midPointThickness;
    const double rightParenX = rightParen->pos().x() + rightParen->ldata()->bbox().x() + rightParen->width()
                               - rightParen->ldata()->midPointThickness;

    const double leftParenToItem = itemLeftX - leftParenX;
    const double itemToRightParen = rightParenX - itemRightX;
    const double parenToItemDist = (leftParenToItem + itemToRightParen) / 2;

    leftParen->mutldata()->moveX(-(parenToItemDist - leftParenToItem));
    rightParen->mutldata()->moveX(-(itemToRightParen - parenToItemDist));
}

void ParenthesisLayout::layoutParenthesis(Parenthesis* item, Parenthesis::LayoutData* ldata, const LayoutContext& ctx)
{
    ldata->setPos(PointF());
    ldata->reset();
    ldata->path.reset();
    ldata->symId.reset();

    setLayoutValues(item, ldata, ctx);

    if (ldata->symId == SymId::noSym) {
        createPathAndShape(item, ldata);
    } else {
        ldata->setShape(Shape(item->symBbox(ldata->symId), item));
    }
}

bool ParenthesisLayout::isInternalParenPadding(const EngravingItem* item1, const EngravingItem* item2)
{
    const Chord* chord1 = toChord(item1->findAncestor(ElementType::CHORD));
    const Chord* chord2 = toChord(item2->findAncestor(ElementType::CHORD));
    const EngravingItem* parent1 = item1->parentItem();
    const EngravingItem* parent2 = item2->parentItem();

    bool internalPadding = (item1->isParenthesis() && parent1 == item2)
                           || (item2->isParenthesis() && parent2 == item1)
                           || parent1 == parent2
                           || (chord1 && chord2 && chord1 == chord2);

    return internalPadding;
}

double ParenthesisLayout::computeExternalParenthesisPadding(const EngravingItem* item1, const EngravingItem* item2)
{
    EngravingItem* parent = item1->isParenthesis() ? item1->parentItem() : item2->parentItem();

    ElementType type1 = item1->type();
    ElementType type2 = item2->type();

    ParenPaddingTablePtr paddingTable = ParenPaddingTable::getPaddingTable(parent);

    double padding = paddingTable->padding(type1, type2);

    double scaling = (item1->mag() + item2->mag()) / 2;
    padding *= scaling;
    return padding;
}

double ParenthesisLayout::computeParenthesisPadding(const EngravingItem* item1, const EngravingItem* item2)
{
    if (isInternalParenPadding(item1, item2)) {
        return computeInternalParenthesisPadding(item1, item2);
    }

    return computeExternalParenthesisPadding(item1, item2);
}

double ParenthesisLayout::computeInternalParenthesisPadding(const EngravingItem* item1, const EngravingItem* item2)
{
    bool parenFirst = item1->isParenthesis();
    const EngravingItem* other = item1->isParenthesis() ? item2 : item1;
    const double spatium = item1->spatium();
    double padding = 0.0;

    switch (other->type()) {
    case ElementType::NOTE:
        if (item1->staffType() && item1->staffType()->isTabStaff()) {
            padding = item1->style().styleS(Sid::tabFretPadding).val() * spatium;
        } else {
            padding = 0.3 * spatium;
        }
        break;
    case ElementType::ARTICULATION:
    case ElementType::ACCIDENTAL:
    case ElementType::STEM:
        padding = 0.3 * spatium;
        break;
    case ElementType::NOTEDOT:
        padding = 0.15 * spatium;
        break;
    case ElementType::LEDGER_LINE:
        padding = 0.3 * spatium;
        break;
    case ElementType::HOOK:
        padding = 0.3 * spatium;
        break;
    case ElementType::KEYSIG:
        padding = (parenFirst ? 0.35 : 0.25) * spatium;
        break;
    case ElementType::TIMESIG:
        padding = (parenFirst ? 0.2 : 0.25) * spatium;
        break;
    case ElementType::CLEF:
        padding = (parenFirst ? 0.2 : 0.25) * spatium;
        break;
    default:
        padding = 0.1 * spatium;
        break;
    }

    double scaling = (item1->mag() + item2->mag()) / 2;
    padding *= scaling;
    return padding;
}

void ParenthesisLayout::createPathAndShape(Parenthesis* item, Parenthesis::LayoutData* ldata)
{
    const double spatium = item->spatium();
    const bool leftBracket = item->direction() == DirectionH::LEFT;

    const double startY = ldata->startY;
    const double height = ldata->height;
    const double xPos = ldata->pos().x();
    const double mag = ldata->mag();
    const double maxMidPointThickness = 0.2 * spatium;
    const double midPointThickness = std::min(ldata->midPointThickness.value(), maxMidPointThickness);

    const double heightInSpatium = height / spatium;
    const double shoulderYOffset = 0.2 * height;

    // Control width of parentheses. We don't want tall parens to be too wide, nor do we want parens at a small scale to lose their curve too much
    double shoulderX = 0.2 * std::pow(height, 0.95) * std::pow(mag, 0.1);
    const double minShoulderX = 0.25 * spatium;
    shoulderX = std::max(shoulderX, minShoulderX);

    PointF start = PointF(xPos, startY);
    const PointF end = PointF(xPos, start.y() + height);
    const PointF endNormalised = end - start;

    const int direction = leftBracket ? -1 : 1;
    const double shoulderForX = direction * shoulderX + midPointThickness * direction;
    const double shoulderBackX = direction * shoulderX + midPointThickness * direction * -1;

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
    double midThickness = 2 * midPointThickness;
    int nbShapes = round(5.0 * heightInSpatium);
    nbShapes = std::clamp(nbShapes, 20, 50);
    PointF bezier1mid = bezier1for - PointF(midPointThickness * direction, 0.0);
    PointF bezier2mid = bezier2for - PointF(midPointThickness * direction, 0.0);
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
        setNoteValues(item, ldata);
        break;
    // height & startY are set in MeasureLayout for clef, timesig and keysig
    // TODO: create generic way of matching height & startY between parentheses on different items
    case ElementType::CLEF:
        setClefValues(item, ldata);
        break;
    case ElementType::TIMESIG:
        setTimeSigValues(item, ldata, ctx);
        break;
    case ElementType::KEYSIG:
        break;
    default:
        setDefaultValues(item, ldata);
        break;
    }
}

void ParenthesisLayout::setClefValues(Parenthesis* item, Parenthesis::LayoutData* ldata)
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

void ParenthesisLayout::setNoteValues(Parenthesis* item, Parenthesis::LayoutData* ldata)
{
    Note* note = toNote(item->parentItem());

    const StaffType* st = note->staffType();

    ldata->setMag(note->mag());
    Shape noteShape = note->shape();
    noteShape.remove_if([item](ShapeElement& s) {
        return s.item() == item || s.item()->isBend() || s.item()->isParenthesis() || s.item()->isAccidental() || s.item()->isNoteDot();
    });

    if (st->isTabStaff()) {
        ldata->startY = noteShape.top();
        ldata->height = noteShape.bbox().height();
        ldata->midPointThickness.set_value(ldata->height / 20 * ldata->mag());
        ldata->endPointThickness.set_value(0.05);
    } else {
        ldata->startY = noteShape.top() - 0.25 * item->spatium();
        ldata->height = noteShape.bbox().height() + 0.5 * item->spatium();
        ldata->midPointThickness.set_value(ldata->height / 30 * ldata->mag());
        ldata->endPointThickness.set_value(0.05);
    }
}

void ParenthesisLayout::setDefaultValues(Parenthesis* item, Parenthesis::LayoutData* ldata)
{
    const double spatium = item->spatium();
    EngravingItem* parent = item->parentItem();
    RectF bbox = parent->ldata()->bbox();

    ldata->setMag(item->parentItem()->mag());
    ldata->startY = bbox.top() - 0.25 * spatium;
    ldata->height = bbox.height() + 0.5 * spatium;
    ldata->midPointThickness.set_value(ldata->height / 60 * ldata->mag());  // 0.1sp for a height of 6sp
    const double PADDING = spatium * 0.2;
    ldata->setPosX(item->direction() == DirectionH::RIGHT ? bbox.right() + PADDING : bbox.left() - PADDING);
}
