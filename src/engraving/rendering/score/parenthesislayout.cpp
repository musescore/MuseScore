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

#include "dom/chord.h"
#include "dom/harmony.h"
#include "dom/ledgerline.h"
#include "dom/measure.h"
#include "dom/note.h"
#include "dom/parenthesis.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/slurtie.h"
#include "dom/staff.h"
#include "dom/timesig.h"
#include "horizontalspacing.h"
#include "parenthesislayout.h"

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
        layoutParenthesis(leftParen, leftParen->mutldata(), ctx);
    }

    if (rightParen) {
        layoutParenthesis(rightParen, rightParen->mutldata(), ctx);
    }

    bool itemAddToSkyline = parent->autoplace() && !parent->ldata()->isSkipDraw();
    Shape dummyItemShape = getParentShape(parent);

    layoutParentheses(leftParen, rightParen, dummyItemShape, itemAddToSkyline, ctx);
}

void ParenthesisLayout::layoutChordParentheses(const Chord* chord, const LayoutContext& ctx)
{
    for (const NoteParenthesisInfo* parenNotesInfo : chord->noteParens()) {
        Parenthesis* leftParen = parenNotesInfo->leftParen();
        Parenthesis* rightParen = parenNotesInfo->rightParen();

        if (!(leftParen || rightParen)) {
            return;
        }
        if (leftParen) {
            layoutParenthesis(leftParen, leftParen->mutldata(), ctx);
        }

        if (rightParen) {
            layoutParenthesis(rightParen, rightParen->mutldata(), ctx);
        }

        bool itemAddToSkyline = chord->autoplace() && !chord->ldata()->isSkipDraw();
        Shape dummyItemShape = getParentShape(chord);

        layoutParentheses(leftParen, rightParen, dummyItemShape, itemAddToSkyline, ctx);
    }
}

void ParenthesisLayout::layoutParentheses(Parenthesis* leftParen, Parenthesis* rightParen, Shape& dummyItemShape, bool itemAddToSkyline,
                                          const LayoutContext& ctx)
{
    if (!leftParen || !rightParen) {
        // 1 parenthesis
        Parenthesis* paren = leftParen ? leftParen : rightParen;
        Parenthesis::LayoutData* ldata = paren->mutldata();
        const bool leftBracket = paren->direction() == DirectionH::LEFT;
        double minDist = 0.0;
        if (!leftBracket && itemAddToSkyline) {
            // Space against existing item shape
            minDist = HorizontalSpacing::minHorizontalDistance(dummyItemShape, paren->shape().translated(
                                                                   ldata->pos()), paren->spatium());
        } else if (itemAddToSkyline) {
            // Space following item shape against this
            minDist = -HorizontalSpacing::minHorizontalDistance(paren->shape().translated(
                                                                    ldata->pos()), dummyItemShape, paren->spatium());
        }
        paren->mutldata()->moveX(minDist);

        return;
    }

    // 2 parentheses
    assert(leftParen && rightParen);

    if (!itemAddToSkyline) {
        return;
    }

    Parenthesis::LayoutData* leftLdata = leftParen->mutldata();
    Parenthesis::LayoutData* rightLdata = rightParen->mutldata();

    const double itemLeftX = dummyItemShape.bbox().x();
    const double parentWidth = dummyItemShape.bbox().width();
    const double itemRightX = itemLeftX + parentWidth;

    const double leftParenPadding = HorizontalSpacing::minHorizontalDistance(leftParen->shape().translated(leftLdata->pos()),
                                                                             dummyItemShape, leftParen->spatium());
    leftLdata->moveX(-leftParenPadding);
    dummyItemShape.add(leftParen->shape().translate(leftLdata->pos() + leftParen->staffOffset()));

    const double rightParenPadding = HorizontalSpacing::minHorizontalDistance(dummyItemShape, rightParen->shape().translated(
                                                                                  rightLdata->pos()), rightParen->spatium());
    rightLdata->moveX(rightParenPadding);

    // If the right parenthesis has been padded against the left parenthesis, this means the parenthesis -> parenthesis padding distance
    // is larger than the width of the item the parentheses surrounds. In this case, the result is visually unbalanced.  Move both parens
    // to the left (relative to the segment) in order to centre the item: (b  ) -> ( b )
    const double itemWidth = parentWidth;
    double parenPadding = ctx.dom().paddingTable().at(ElementType::PARENTHESIS).at(ElementType::PARENTHESIS);
    parenPadding *= leftParen->ldata()->mag();

    if (itemWidth >= parenPadding) {
        return;
    }

    // Move parentheses to place item in the middle
    const double leftParenX = leftLdata->pos().x() + leftParen->ldata()->bbox().x() + leftParen->ldata()->midPointThickness;
    const double rightParenX = rightLdata->pos().x() + rightParen->ldata()->bbox().x() + rightParen->width()
                               - rightParen->ldata()->midPointThickness;

    const double leftParenToItem = itemLeftX - leftParenX;
    const double itemToRightParen = rightParenX - itemRightX;
    const double parenToItemDist = (leftParenToItem + itemToRightParen) / 2;

    leftLdata->moveX(-(parenToItemDist - leftParenToItem));
    rightLdata->moveX(-(itemToRightParen - parenToItemDist));
}

void ParenthesisLayout::layoutParenthesis(Parenthesis* item, Parenthesis::LayoutData* ldata, const LayoutContext& ctx)
{
    setLayoutValues(item, ldata, ctx);

    if (ldata->symId == SymId::noSym) {
        createPathAndShape(item, ldata);
    } else {
        createSmuflShape(item, ldata);
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
    case ElementType::HARMONY:
        padding = 0.2 * spatium;
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

    if (std::isinf(height)) {
        LOGE() << "Error: parenthesis height is infinite";
        return;
    }

    // Control width of parentheses. We don't want tall parens to be too wide, nor do we want parens at a small scale to lose their curve too much
    double shoulderX = !muse::RealIsNull(ldata->shoulderWidth()) ? ldata->shoulderWidth() : 0.2
                       * std::pow(height, 0.95) * std::pow(mag, 0.1);

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

void ParenthesisLayout::createSmuflShape(Parenthesis* item, Parenthesis::LayoutData* ldata)
{
    Shape bbox = Shape(item->symBbox(ldata->symId), item);
    double scale = item->ldata()->symScale;
    bbox.scale(SizeF(scale, scale));
    ldata->setShape(bbox);
}

void ParenthesisLayout::setLayoutValues(Parenthesis* item, Parenthesis::LayoutData* ldata, const LayoutContext& ctx)
{
    if (!item->parentItem()) {
        return;
    }

    ldata->setPos(PointF());
    ldata->reset();
    ldata->path.reset();
    ldata->symId.reset();

    // Set ldata values based on parent
    switch (item->parentItem()->type()) {
    case ElementType::CHORD:
        setChordValues(item, ldata);
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
    case ElementType::HARMONY:
        setHarmonyValues(item, ldata, ctx);
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

void ParenthesisLayout::setChordValues(Parenthesis* item, Parenthesis::LayoutData* ldata)
{
    Chord* chord = toChord(item->parentItem());

    ldata->setMag(chord->mag());

    Shape notesShape;

    const NoteParenthesisInfo* parenInfo = chord->findNoteParenInfo(item);
    IF_ASSERT_FAILED(parenInfo) {
        LOGD() << "No parenthesis info found for this chord";
        return;
    }

    const std::vector<Note*>& notes = parenInfo->notes();

    assert(!notes.empty());

    for (const Note* note : notes) {
        notesShape.add(getNoteShape(note, item).translated(note->pos()));
    }

    const StaffType* st = chord->staffType();
    if (st->isTabStaff()) {
        ldata->startY = notesShape.top();
        ldata->height = notesShape.bbox().height();
        ldata->midPointThickness.set_value(std::pow(ldata->height, 0.36) * ldata->mag());
        ldata->endPointThickness.set_value(0.05);
    } else {
        ldata->startY = notesShape.top() - 0.25 * item->spatium();
        ldata->height = notesShape.bbox().height() + 0.5 * item->spatium();
        ldata->midPointThickness.set_value(std::pow(ldata->height, 0.33) * ldata->mag());
        ldata->endPointThickness.set_value(0.05);
    }
}

void ParenthesisLayout::setHarmonyValues(Parenthesis* item, Parenthesis::LayoutData* ldata, const LayoutContext& ctx)
{
    const double spatium = item->spatium();
    Harmony* parent = toHarmony(item->parentItem());
    RectF bbox = parent->ldata()->bbox();

    double endPointThickness = 0.03;
    double extension = 0.1 * spatium * (parent->size() / 10.0);
    double bottom = parent->baseLine() + extension;

    double defaultCapHeight = muse::draw::FontMetrics::capHeight(parent->font());

    double topCapHeight = DBL_MAX;
    const TextSegment* rootTextSeg = nullptr;
    if (!parent->ldata()->renderItemList.value().empty()) {
        for (const HarmonyRenderItem* renderItem : parent->ldata()->renderItemList.value()) {
            if (const TextSegment* ts = dynamic_cast<const TextSegment*>(renderItem)) {
                topCapHeight = std::min(topCapHeight, ts->pos().y() - ts->capHeight());

                rootTextSeg = !rootTextSeg ? ts : rootTextSeg;
            }
        }
    } else {
        topCapHeight = defaultCapHeight;
    }

    double top = topCapHeight - extension;
    double height = bottom - top;

    if (ctx.conf().styleB(Sid::harmonyParenUseSmuflSym)) {
        ldata->symId = item->direction() == DirectionH::LEFT ? SymId::csymParensLeftTall : SymId::csymParensRightTall;
        double symHeight = item->symHeight(ldata->symId);
        ldata->symScale = height / symHeight;

        return;
    }

    double rootCapHeight = rootTextSeg ? rootTextSeg->capHeight() : defaultCapHeight;
    double scale = (height - 2 * extension) / rootCapHeight;

    ldata->setMag(parent->mag());
    ldata->startY = top;
    ldata->height = height;
    static constexpr double HEIGHT_TO_WIDTH_RATIO = 20;
    ldata->midPointThickness.set_value(ldata->height / HEIGHT_TO_WIDTH_RATIO * ldata->mag() * 1 / std::sqrt(scale));
    ldata->endPointThickness.set_value(endPointThickness);

    double shoulder = 0.2 * ldata->height * std::pow(ldata->mag(), 0.1) * 1 / std::sqrt(scale);
    ldata->shoulderWidth = shoulder;

    const double PADDING = spatium * 0.2;
    ldata->setPosX(item->direction() == DirectionH::RIGHT ? bbox.right() + PADDING : bbox.left() - PADDING);
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

Shape ParenthesisLayout::getParentShape(const EngravingItem* parent)
{
    Shape parentShape = parent->shape();

    bool isChord = parent->isChord();

    parentShape.remove_if([isChord](ShapeElement& s) {
        return !s.item() || s.item()->isParenthesis()
               || (s.item()->isLaissezVibSegment() && isChord)
               || (s.item()->isHook() && isChord)
               || (s.item()->isStem() && isChord);
    });

    return parentShape;
}

Shape ParenthesisLayout::getNoteShape(const Note* note, Parenthesis* paren)
{
    Shape noteShape = note->shape();
    noteShape.remove_if([paren](ShapeElement& s) {
        return s.item() == paren || s.item()->isBend() || s.item()->isParenthesis() || s.item()->isAccidental() || s.item()->isNoteDot() || s.item()->isLaissezVibSegment();
    });

    return noteShape;
}
