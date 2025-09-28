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
#include "editmoderenderer.h"

#include "draw/painter.h"

#include "dom/engravingitem.h"
#include "dom/barline.h"
#include "dom/dynamic.h"
#include "dom/slurtie.h"
#include "dom/textbase.h"
#include "dom/textedit.h"
#include "rendering/score/tdraw.h"

using namespace mu::engraving::rendering::editmode;
using namespace mu::engraving;
using namespace muse::draw;

void EditModeRenderer::drawItem(const EngravingItem* item, muse::draw::Painter* painter, const EditData& ed, double currentViewScaling,
                                const ElementPaintOptions& opt)
{
    switch (item->type()) {
    case ElementType::BAR_LINE:
        drawBarline(item_cast<const BarLine*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::CAPO:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::DYNAMIC:
        drawDynamic(item_cast<const Dynamic*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::EXPRESSION:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::FIGURED_BASS:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::FINGERING:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::GUITAR_BEND_TEXT:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::HARMONY:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::HARP_DIAGRAM:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::INSTRUMENT_CHANGE:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::INSTRUMENT_NAME:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::JUMP:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::LAISSEZ_VIB_SEGMENT:
        drawSlurTieSegment(item_cast<const SlurTieSegment*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::LYRICS:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::MMREST_RANGE:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::MARKER:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::MEASURE_NUMBER:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::PARTIAL_TIE_SEGMENT:
        drawSlurTieSegment(item_cast<const SlurTieSegment*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::PLAY_COUNT_TEXT:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::PLAYTECH_ANNOTATION:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::REHEARSAL_MARK:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::SLUR_SEGMENT:
    case ElementType::TIE_SEGMENT:
    case ElementType::HAMMER_ON_PULL_OFF_SEGMENT:
    case ElementType::TAPPING_HALF_SLUR_SEGMENT:
        drawSlurTieSegment(item_cast<const SlurTieSegment*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::STAFF_TEXT:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::STICKING:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::STRING_TUNINGS:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::SYSTEM_TEXT:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::TEMPO_TEXT:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::TEXT:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    case ElementType::TRIPLET_FEEL:
        drawTextBase(item_cast<const TextBase*>(item), painter, ed, currentViewScaling, opt);
        break;
    default:
        drawEngravingItem(item, painter, ed, currentViewScaling, opt);
    }
}

void EditModeRenderer::drawEngravingItem(const EngravingItem* item, muse::draw::Painter* painter, const EditData& ed,
                                         double currentViewScaling, const ElementPaintOptions&)
{
    UNUSED(currentViewScaling);

    Pen pen(item->configuration()->scoreInversionEnabled()
            ? item->configuration()->scoreInversionColor()
            : item->configuration()->defaultColor(), 0.0);
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

void EditModeRenderer::drawBarline(const BarLine* item, muse::draw::Painter* painter, const EditData& ed, double currentViewScaling,
                                   const ElementPaintOptions& opt)
{
    drawEngravingItem(item, painter, ed, currentViewScaling, opt);

    const BarLineEditData* bed = static_cast<BarLineEditData*>(ed.getData(item).get());
    BarLine::LayoutData* ldata = const_cast<BarLine::LayoutData*>(item->ldata());
    ldata->y1 += bed->yoff1;
    ldata->y2 += bed->yoff2;
    PointF pos(item->canvasPos());
    painter->translate(pos);

    score::TDraw::drawItem(item, painter, opt);

    ldata->y1 -= bed->yoff1;
    ldata->y2 -= bed->yoff2;
    painter->translate(-pos);
}

void EditModeRenderer::drawDynamic(const Dynamic* item, muse::draw::Painter* painter, const EditData& ed, double currentViewScaling,
                                   const ElementPaintOptions& opt)
{
    if (item->cursor() && item->cursor()->editing()) {
        drawTextBase(item, painter, ed, currentViewScaling, opt);
        return;
    }

    drawEngravingItem(item, painter, ed, currentViewScaling, opt);
}

void EditModeRenderer::drawSlurTieSegment(const SlurTieSegment* item, muse::draw::Painter* painter, const EditData& ed,
                                          double currentViewScaling, const ElementPaintOptions& opt)
{
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

    drawEngravingItem(item, painter, ed, currentViewScaling, opt);
}

static void drawTextBaseSelection(const TextBase* item, muse::draw::Painter* painter, const RectF& r)
{
    painter->save();
    Brush bg(item->configuration()->selectionColor());
    painter->setCompositionMode(CompositionMode::HardLight);
    painter->setBrush(bg);
    painter->setNoPen();
    painter->drawRect(r);
    painter->restore();
}

void EditModeRenderer::drawTextBase(const TextBase* item, muse::draw::Painter* painter, const EditData& ed, double currentViewScaling,
                                    const ElementPaintOptions& opt)
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
        painter->setPen(item->textColor(opt));
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
    painter->setBrush(item->curColor(opt));
    Pen pen(item->curColor(opt));
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
