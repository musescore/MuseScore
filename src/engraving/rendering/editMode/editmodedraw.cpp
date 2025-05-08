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

#include "draw/painter.h"

#include "dom/engravingitem.h"
#include "dom/barline.h"
#include "dom/dynamic.h"
#include "dom/slurtie.h"
#include "dom/textbase.h"
#include "dom/textedit.h"

#include "infrastructure/rtti.h"

#include "editmodedraw.h"

using namespace mu::engraving::rendering::editmode;
using namespace mu::engraving;
using namespace mu::engraving::rtti;
using namespace muse;
using namespace muse::draw;

void EditModeDraw::drawItem(const EngravingItem* item, muse::draw::Painter* painter, EditData& ed, double currentViewScaling)
{
    switch (item->type()) {
    case ElementType::BAR_LINE:
        drawBarline(item_cast<const BarLine*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::CAPO:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::DYNAMIC:
        drawDynamic(item_cast<const Dynamic*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::EXPRESSION:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::FIGURED_BASS:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::FINGERING:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::GUITAR_BEND_TEXT:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::HARMONY:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::HARP_DIAGRAM:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::INSTRUMENT_CHANGE:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::INSTRUMENT_NAME:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::JUMP:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::LAISSEZ_VIB_SEGMENT:
        drawSlurTieSegment(item_cast<const SlurTieSegment*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::LYRICS:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::MMREST_RANGE:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::MARKER:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::MEASURE_NUMBER:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::PARTIAL_TIE_SEGMENT:
        drawSlurTieSegment(item_cast<const SlurTieSegment*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::PLAYTECH_ANNOTATION:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::REHEARSAL_MARK:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::SLUR_SEGMENT:
        drawSlurTieSegment(item_cast<const SlurTieSegment*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::STAFF_TEXT:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::STICKING:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::STRING_TUNINGS:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::SYSTEM_TEXT:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::TEMPO_TEXT:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::TEXT:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::TIE_SEGMENT:
        drawSlurTieSegment(item_cast<const SlurTieSegment*>(item), painter, ed, currentViewScaling);
        break;
    case ElementType::TRIPLET_FEEL:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling);
        break;
    default:
        drawEngravingItem(item, painter, ed, currentViewScaling);
    }
}

void EditModeDraw::drawEngravingItem(const EngravingItem* item, muse::draw::Painter* painter, EditData& ed, double currentViewScaling)
{
    UNUSED(currentViewScaling);

    Pen pen(item->configuration()->defaultColor(), 0.0);
    painter->setPen(pen);
    for (int i = 0; i < ed.grips; ++i) {
        if (Grip(i) == ed.curGrip) {
            painter->setBrush(item->configuration()->scoreGreyColor());
        } else {
            painter->setBrush(BrushStyle::NoBrush);
        }
        painter->drawRect(ed.grip[i]);
    }
}

static void drawDots(const BarLine* item, Painter* painter, double x)
{
    double spatium = item->spatium();

    double y1l;
    double y2l;
    if (item->explicitParent() == 0) {      // for use in palette (always Bravura)
        //Bravura shifted repeatDot symbol 0.5sp upper in the font itself (1.272)
        y1l = 1.5 * spatium;
        y2l = 2.5 * spatium;
    } else {
        const StaffType* st = item->staffType();

        y1l = st->doty1() * spatium;
        y2l = st->doty2() * spatium;

        //workaround to make Emmentaler, Gonville and MuseJazz font work correctly with repeatDots
        if (item->score()->engravingFont()->name() == "Emmentaler"
            || item->score()->engravingFont()->name() == "Gonville"
            || item->score()->engravingFont()->name() == "MuseJazz") {
            double offset = 0.5 * item->style().spatium() * item->mag();
            y1l += offset;
            y2l += offset;
        }

        //adjust for staffType offset
        double stYOffset = item->staffOffsetY();
        y1l += stYOffset;
        y2l += stYOffset;
    }

    item->drawSymbol(SymId::repeatDot, painter, PointF(x, y1l));
    item->drawSymbol(SymId::repeatDot, painter, PointF(x, y2l));
}

static void drawTips(const BarLine* item, const BarLine::LayoutData* data, Painter* painter, bool reversed, double x)
{
    if (reversed) {
        if (item->isTop()) {
            item->drawSymbol(SymId::reversedBracketTop, painter, PointF(x - item->symWidth(SymId::reversedBracketTop), data->y1));
        }
        if (item->isBottom()) {
            item->drawSymbol(SymId::reversedBracketBottom, painter, PointF(x - item->symWidth(SymId::reversedBracketBottom), data->y2));
        }
    } else {
        if (item->isTop()) {
            item->drawSymbol(SymId::bracketTop, painter, PointF(x, data->y1));
        }
        if (item->isBottom()) {
            item->drawSymbol(SymId::bracketBottom, painter, PointF(x, data->y2));
        }
    }
}

static void setMask(const EngravingItem* item, const EngravingItem::LayoutData* ldata, Painter* painter)
{
    const Shape& mask = ldata->mask();
    if (mask.empty()) {
        return;
    }

    RectF background = ldata->bbox().padded(item->spatium());

    painter->setMask(background, mask.toRects());
}

static void doDrawBarline(const BarLine* item, const BarLine::LayoutData* ldata, Painter* painter)
{
    TRACE_DRAW_ITEM;

    painter->save();
    setMask(item, ldata, painter);

    IF_ASSERT_FAILED(ldata) {
        return;
    }

    switch (item->barLineType()) {
    case BarLineType::NORMAL: {
        double lw = item->style().styleMM(Sid::barWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(lw * .5, ldata->y1, lw * .5, ldata->y2));
    }
    break;

    case BarLineType::BROKEN: {
        double lw = item->style().styleMM(Sid::barWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::DashLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(lw * .5, ldata->y1, lw * .5, ldata->y2));
    }
    break;

    case BarLineType::DOTTED: {
        double lw = item->style().styleMM(Sid::barWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::DotLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(lw * .5, ldata->y1, lw * .5, ldata->y2));
    }
    break;

    case BarLineType::END: {
        double lw = item->style().styleMM(Sid::barWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        double x  = lw * .5;
        painter->drawLine(LineF(x, ldata->y1, x, ldata->y2));

        double lw2 = item->style().styleMM(Sid::endBarWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        x += ((lw * .5) + item->style().styleMM(Sid::endBarDistance) + (lw2 * .5)) * item->mag();
        painter->drawLine(LineF(x, ldata->y1, x, ldata->y2));
    }
    break;

    case BarLineType::DOUBLE: {
        double lw = item->style().styleMM(Sid::doubleBarWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        double x = lw * .5;
        painter->drawLine(LineF(x, ldata->y1, x, ldata->y2));
        x += ((lw * .5) + item->style().styleMM(Sid::doubleBarDistance) + (lw * .5)) * item->mag();
        painter->drawLine(LineF(x, ldata->y1, x, ldata->y2));
    }
    break;

    case BarLineType::REVERSE_END: {
        double lw = item->style().styleMM(Sid::endBarWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        double x = lw * .5;
        painter->drawLine(LineF(x, ldata->y1, x, ldata->y2));

        double lw2 = item->style().styleMM(Sid::barWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        x += ((lw * .5) + item->style().styleMM(Sid::endBarDistance) + (lw2 * .5)) * item->mag();
        painter->drawLine(LineF(x, ldata->y1, x, ldata->y2));
    }
    break;

    case BarLineType::HEAVY: {
        double lw = item->style().styleMM(Sid::endBarWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(lw * .5, ldata->y1, lw * .5, ldata->y2));
    }
    break;

    case BarLineType::DOUBLE_HEAVY: {
        double lw2 = item->style().styleMM(Sid::endBarWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        double x = lw2 * .5;
        painter->drawLine(LineF(x, ldata->y1, x, ldata->y2));
        x += ((lw2 * .5) + item->style().styleMM(Sid::endBarDistance) + (lw2 * .5)) * item->mag();
        painter->drawLine(LineF(x, ldata->y1, x, ldata->y2));
    }
    break;

    case BarLineType::START_REPEAT: {
        double lw2 = item->style().styleMM(Sid::endBarWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        double x = lw2 * .5;
        painter->drawLine(LineF(x, ldata->y1, x, ldata->y2));

        double lw = item->style().styleMM(Sid::barWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        x += ((lw2 * .5) + item->style().styleMM(Sid::endBarDistance) + (lw * .5)) * item->mag();
        painter->drawLine(LineF(x, ldata->y1, x, ldata->y2));

        x += ((lw * .5) + item->style().styleMM(Sid::repeatBarlineDotSeparation)) * item->mag();
        drawDots(item, painter, x);

        if (item->style().styleB(Sid::repeatBarTips)) {
            drawTips(item, ldata, painter, false, 0.0);
        }
    }
    break;

    case BarLineType::END_REPEAT: {
        double lw = item->style().styleMM(Sid::barWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));

        double x = 0.0;
        drawDots(item, painter, x);

        x += item->symBbox(SymId::repeatDot).width();
        x += (item->style().styleMM(Sid::repeatBarlineDotSeparation) + (lw * .5)) * item->mag();
        painter->drawLine(LineF(x, ldata->y1, x, ldata->y2));

        double lw2 = item->style().styleMM(Sid::endBarWidth) * item->mag();
        x += ((lw * .5) + item->style().styleMM(Sid::endBarDistance) + (lw2 * .5)) * item->mag();
        painter->setPen(Pen(item->curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(x, ldata->y1, x, ldata->y2));

        if (item->style().styleB(Sid::repeatBarTips)) {
            drawTips(item, ldata, painter, true, x + lw2 * .5);
        }
    }
    break;
    case BarLineType::END_START_REPEAT: {
        double lw = item->style().styleMM(Sid::barWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));

        double x = 0.0;
        drawDots(item, painter, x);

        x += item->symBbox(SymId::repeatDot).width();
        x += (item->style().styleMM(Sid::repeatBarlineDotSeparation) + (lw * .5)) * item->mag();
        painter->drawLine(LineF(x, ldata->y1, x, ldata->y2));

        double lw2 = item->style().styleMM(Sid::endBarWidth) * item->mag();
        x += ((lw * .5) + item->style().styleMM(Sid::endBarDistance) + (lw2 * .5)) * item->mag();
        painter->setPen(Pen(item->curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(x, ldata->y1, x, ldata->y2));

        if (item->style().styleB(Sid::repeatBarTips)) {
            drawTips(item, ldata, painter, true, x + lw2 * .5);
        }

        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        x  += ((lw2 * .5) + item->style().styleMM(Sid::endBarDistance) + (lw * .5)) * item->mag();
        painter->drawLine(LineF(x, ldata->y1, x, ldata->y2));

        x += ((lw * .5) + item->style().styleMM(Sid::repeatBarlineDotSeparation)) * item->mag();
        drawDots(item, painter, x);

        if (item->style().styleB(Sid::repeatBarTips)) {
            drawTips(item, ldata, painter, false, 0.0);
        }
    }
    break;
    }
    Segment* s = item->segment();
    if (s && s->isEndBarLineType() && !item->score()->printing()) {
        Measure* m = s->measure();
        if (m->isIrregular() && item->score()->markIrregularMeasures() && !m->isMMRest()) {
            painter->setPen(item->configuration()->invisibleColor());

            Font f(u"Edwin", Font::Type::Text);
            f.setPointSizeF(12 * item->spatium() / SPATIUM20);
            f.setBold(true);
            Char ch = m->ticks() > m->timesig() ? u'+' : u'-';
            RectF r = FontMetrics(f).boundingRect(ch);

            Font scaledFont(f);
            scaledFont.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
            painter->setFont(scaledFont);

            painter->drawText(-r.width(), 0.0, ch);
        }
    }

    painter->restore();
}

void EditModeDraw::drawBarline(const BarLine* item, muse::draw::Painter* painter, EditData& ed, double currentViewScaling)
{
    drawEngravingItem(item, painter, ed, currentViewScaling);
    BarLineEditData* bed = static_cast<BarLineEditData*>(ed.getData(item).get());
    // Create a copy of the layout data
    BarLine::LayoutData* ldata = new BarLine::LayoutData(*item->ldata());
    ldata->y1 += bed->yoff1;
    ldata->y2 += bed->yoff2;
    PointF pos(item->canvasPos());
    painter->translate(pos);
    doDrawBarline(item, ldata, painter);
    painter->translate(-pos);

    delete ldata;
}

void EditModeDraw::drawDynamic(const Dynamic* item, muse::draw::Painter* painter, EditData& ed, double currentViewScaling)
{
    if (ed.editTextualProperties) {
        drawTextBase(item, painter, ed, currentViewScaling);
        return;
    }

    drawEngravingItem(item, painter, ed, currentViewScaling);
}

void EditModeDraw::drawSlurTieSegment(const SlurTieSegment* item, muse::draw::Painter* painter, EditData& ed, double currentViewScaling)
{
    UNUSED(currentViewScaling)

    PolygonF polygon(7);
    polygon[0] = PointF(ed.grip[int(Grip::START)].center());
    polygon[1] = PointF(ed.grip[int(Grip::BEZIER1)].center());
    polygon[2] = PointF(ed.grip[int(Grip::SHOULDER)].center());
    polygon[3] = PointF(ed.grip[int(Grip::BEZIER2)].center());
    polygon[4] = PointF(ed.grip[int(Grip::END)].center());
    polygon[5] = PointF(ed.grip[int(Grip::DRAG)].center());
    polygon[6] = PointF(ed.grip[int(Grip::START)].center());
    painter->setPen(Pen(item->configuration()->scoreGreyColor(), 0.0));
    painter->drawPolyline(polygon);

    painter->setPen(Pen(item->configuration()->defaultColor(), 0.0));
    for (int i = 0; i < ed.grips; ++i) {
        // Can't use ternary operator, because we want different overloads of `setBrush`
        if (Grip(i) == ed.curGrip) {
            painter->setBrush(item->configuration()->scoreGreyColor());
        } else {
            painter->setBrush(BrushStyle::NoBrush);
        }
        painter->drawRect(ed.grip[i]);
    }
}

static void drawTextBaseSelection(const TextBase* item, Painter* painter, const RectF& r)
{
    Brush bg(item->configuration()->selectionColor());
    painter->setCompositionMode(CompositionMode::HardLight);
    painter->setBrush(bg);
    painter->setNoPen();
    painter->drawRect(r);
    painter->setCompositionMode(CompositionMode::SourceOver);
    painter->setPen(item->textColor());
}

void EditModeDraw::drawTextBase(const TextBase* item, muse::draw::Painter* painter, EditData& ed, double currentViewScaling)
{
    PointF pos(item->canvasPos());
    painter->translate(pos);

    TextEditData* ted = static_cast<TextEditData*>(ed.getData(item).get());
    if (!ted) {
        LOGD("ted not found");
        return;
    }
    TextCursor* cursor = ted->cursor();

    const TextBase::LayoutData* ldata = item->ldata();
    IF_ASSERT_FAILED(ldata) {
        return;
    }

    if (cursor->hasSelection()) {
        painter->setBrush(BrushStyle::NoBrush);
        painter->setPen(item->textColor());
        size_t r1 = cursor->selectLine();
        size_t r2 = cursor->row();
        size_t c1 = cursor->selectColumn();
        size_t c2 = cursor->column();

        TextBase::sort(r1, c1, r2, c2);
        size_t row = 0;
        for (const TextBlock& t : ldata->blocks) {
            t.draw(painter, item);
            if (row >= r1 && row <= r2) {
                RectF br;
                if (row == r1 && r1 == r2) {
                    br = t.boundingRect(static_cast<int>(c1), static_cast<int>(c2), item);
                } else if (row == r1) {
                    br = t.boundingRect(static_cast<int>(c1), static_cast<int>(t.columns()), item);
                } else if (row == r2) {
                    br = t.boundingRect(0, static_cast<int>(c2), item);
                } else {
                    br = t.boundingRect();
                }
                br.translate(0.0, t.y());
                drawTextBaseSelection(item, painter, br);
            }
            ++row;
        }
    }
    painter->setBrush(item->curColor());
    Pen pen(item->curColor());
    pen.setJoinStyle(PenJoinStyle::MiterJoin);
    painter->setPen(pen);

    // Don't draw cursor if there is a selection
    if (!cursor->hasSelection()) {
        painter->drawRect(cursor->cursorRect());
    }

    painter->translate(-pos);
    painter->setPen(Pen(item->configuration()->frameColor(), 2.0 / currentViewScaling)); // 2 pixel pen size
    painter->setBrush(BrushStyle::NoBrush);

    double m = item->spatium();
    RectF r = item->canvasBoundingRect().adjusted(-m, -m, m, m);

    painter->drawRect(r);
    pen = Pen(item->configuration()->defaultColor(), 0.0);
}
