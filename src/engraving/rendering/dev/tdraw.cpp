/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#include "tdraw.h"

#include "style/style.h"
#include "types/typesconv.h"

#include "libmscore/accidental.h"
#include "libmscore/actionicon.h"
#include "libmscore/ambitus.h"
#include "libmscore/arpeggio.h"
#include "libmscore/articulation.h"

#include "libmscore/ornament.h"

#include "libmscore/score.h"
#include "libmscore/staff.h"

#include "infrastructure/rtti.h"

using namespace mu::engraving;
using namespace mu::engraving::rtti;
using namespace mu::engraving::rendering::dev;
using namespace mu::draw;

void TDraw::drawItem(const EngravingItem* item, draw::Painter* painter)
{
    switch (item->type()) {
    case ElementType::ACCIDENTAL:   draw(item_cast<const Accidental*>(item), painter);
        break;
    case ElementType::ACTION_ICON:  draw(item_cast<const ActionIcon*>(item), painter);
        break;
    case ElementType::AMBITUS:      draw(item_cast<const Ambitus*>(item), painter);
        break;
    case ElementType::ARPEGGIO:     draw(item_cast<const Arpeggio*>(item), painter);
        break;
    case ElementType::ARTICULATION: draw(item_cast<const Articulation*>(item), painter);
        break;
    case ElementType::ORNAMENT:     draw(item_cast<const Ornament*>(item), painter);
        break;
    default:
        item->draw(painter);
    }
}

void TDraw::draw(const Accidental* item, draw::Painter* painter)
{
    TRACE_DRAW_ITEM;
    if (item->layoutData().isSkipDraw) {
        return;
    }

    painter->setPen(item->curColor());
    for (const Accidental::LayoutData::Sym& e : item->layoutData().syms) {
        item->drawSymbol(e.sym, painter, PointF(e.x, e.y));
    }
}

void TDraw::draw(const ActionIcon* item, draw::Painter* painter)
{
    TRACE_DRAW_ITEM;
    painter->setFont(item->iconFont());
    painter->drawText(item->bbox(), draw::AlignCenter, Char(item->icon()));
}

void TDraw::draw(const Ambitus* item, draw::Painter* painter)
{
    TRACE_DRAW_ITEM;

    double spatium = item->spatium();
    double lw = item->lineWidth().val() * spatium;
    painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));

    const Ambitus::LayoutData& layoutData = item->layoutData();

    item->drawSymbol(item->noteHead(), painter, layoutData.topPos);
    item->drawSymbol(item->noteHead(), painter, layoutData.bottomPos);
    if (item->hasLine()) {
        painter->drawLine(layoutData.line);
    }

    // draw ledger lines (if not in a palette)
    if (item->segment() && item->track() != mu::nidx) {
        Fraction tick = item->segment()->tick();
        Staff* staff = item->score()->staff(item->staffIdx());
        double lineDist = staff->lineDistance(tick);
        int numOfLines = staff->lines(tick);
        double step = lineDist * spatium;
        double stepTolerance = step * 0.1;
        double ledgerLineLength = item->style().styleS(Sid::ledgerLineLength).val() * spatium;
        double ledgerLineWidth = item->style().styleS(Sid::ledgerLineWidth).val() * spatium;
        painter->setPen(Pen(item->curColor(), ledgerLineWidth, PenStyle::SolidLine, PenCapStyle::FlatCap));

        if (layoutData.topPos.y() - stepTolerance <= -step) {
            double xMin = layoutData.topPos.x() - ledgerLineLength;
            double xMax = layoutData.topPos.x() + item->headWidth() + ledgerLineLength;
            for (double y = -step; y >= layoutData.topPos.y() - stepTolerance; y -= step) {
                painter->drawLine(mu::PointF(xMin, y), mu::PointF(xMax, y));
            }
        }

        if (layoutData.bottomPos.y() + stepTolerance >= numOfLines * step) {
            double xMin = layoutData.bottomPos.x() - ledgerLineLength;
            double xMax = layoutData.bottomPos.x() + item->headWidth() + ledgerLineLength;
            for (double y = numOfLines * step; y <= layoutData.bottomPos.y() + stepTolerance; y += step) {
                painter->drawLine(mu::PointF(xMin, y), mu::PointF(xMax, y));
            }
        }
    }
}

void TDraw::draw(const Arpeggio* item, draw::Painter* painter)
{
    TRACE_DRAW_ITEM;

    const Arpeggio::LayoutData& layoutData = item->layoutData();

    const double y1 = layoutData.bbox.top();
    const double y2 = layoutData.bbox.bottom();
    const double lineWidth = item->style().styleMM(Sid::ArpeggioLineWidth);

    painter->setPen(Pen(item->curColor(), lineWidth, PenStyle::SolidLine, PenCapStyle::FlatCap));
    painter->save();

    switch (item->arpeggioType()) {
    case ArpeggioType::NORMAL:
    case ArpeggioType::UP:
    {
        const RectF& r = layoutData.symsBBox;
        painter->rotate(-90.0);
        item->drawSymbols(layoutData.symbols, painter, PointF(-r.right() - y1, -r.bottom() + r.height()));
    } break;

    case ArpeggioType::DOWN:
    {
        const RectF& r = layoutData.symsBBox;
        painter->rotate(90.0);
        item->drawSymbols(layoutData.symbols, painter, PointF(-r.left() + y1, -r.top() - r.height()));
    } break;

    case ArpeggioType::UP_STRAIGHT:
    {
        const RectF& r = layoutData.symsBBox;
        double x1 = item->spatium() * 0.5;
        item->drawSymbol(SymId::arrowheadBlackUp, painter, PointF(x1 - r.width() * 0.5, y1 - r.top()));
        double ny1 = y1 - r.top() * 0.5;
        painter->drawLine(LineF(x1, ny1, x1, y2));
    } break;

    case ArpeggioType::DOWN_STRAIGHT:
    {
        const RectF& r = layoutData.symsBBox;
        double x1 = item->spatium() * 0.5;
        item->drawSymbol(SymId::arrowheadBlackDown, painter, PointF(x1 - r.width() * 0.5, y2 - r.bottom()));
        double ny2 = y2 + r.top() * 0.5;
        painter->drawLine(LineF(x1, y1, x1, ny2));
    } break;

    case ArpeggioType::BRACKET:
    {
        double w = item->style().styleS(Sid::ArpeggioHookLen).val() * item->spatium();
        painter->drawLine(LineF(0.0, y1, w, y1));
        painter->drawLine(LineF(0.0, y2, w, y2));
        painter->drawLine(LineF(0.0, y1 - lineWidth / 2, 0.0, y2 + lineWidth / 2));
    } break;
    }
    painter->restore();
}

void TDraw::draw(const Articulation* item, draw::Painter* painter)
{
    TRACE_DRAW_ITEM;

    painter->setPen(item->curColor());

    if (item->textType() == ArticulationTextType::NO_TEXT) {
        item->drawSymbol(item->symId(), painter, PointF(-0.5 * item->width(), 0.0));
    } else {
        mu::draw::Font scaledFont(item->font());
        scaledFont.setPointSizeF(scaledFont.pointSizeF() * item->magS() * MScore::pixelRatio);
        painter->setFont(scaledFont);
        painter->drawText(item->bbox(), TextDontClip | AlignLeft | AlignTop, TConv::text(item->textType()));
    }
}

void TDraw::draw(const Ornament* item, draw::Painter* painter)
{
    draw(static_cast<const Articulation*>(item), painter);
}
