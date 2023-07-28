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

#include "libmscore/bagpembell.h"

#include "libmscore/ornament.h"

#include "libmscore/note.h"

#include "libmscore/score.h"
#include "libmscore/staff.h"

#include "infrastructure/rtti.h"

using namespace mu::engraving;
using namespace mu::engraving::rtti;
using namespace mu::engraving::rendering::stable;
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
    case ElementType::BAGPIPE_EMBELLISHMENT: draw(item_cast<const BagpipeEmbellishment*>(item), painter);
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
    // don't show accidentals for tab or slash notation
    if (item->onTabStaff() || (item->note() && item->note()->fixed())) {
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

    double _spatium = item->spatium();

    double y1 = item->bbox().top();
    double y2 = item->bbox().bottom();

    double lineWidth = item->style().styleMM(Sid::ArpeggioLineWidth);

    painter->setPen(Pen(item->curColor(), lineWidth, PenStyle::SolidLine, PenCapStyle::FlatCap));
    painter->save();

    switch (item->arpeggioType()) {
    case ArpeggioType::NORMAL:
    case ArpeggioType::UP:
    {
        RectF r(item->symBbox(item->layoutData().symbols));
        painter->rotate(-90.0);
        item->drawSymbols(item->layoutData().symbols, painter, PointF(-r.right() - y1, -r.bottom() + r.height()));
    }
    break;

    case ArpeggioType::DOWN:
    {
        RectF r(item->symBbox(item->layoutData().symbols));
        painter->rotate(90.0);
        item->drawSymbols(item->layoutData().symbols, painter, PointF(-r.left() + y1, -r.top() - r.height()));
    }
    break;

    case ArpeggioType::UP_STRAIGHT:
    {
        RectF r(item->symBbox(SymId::arrowheadBlackUp));
        double x1 = _spatium * .5;
        item->drawSymbol(SymId::arrowheadBlackUp, painter, PointF(x1 - r.width() * .5, y1 - r.top()));
        y1 -= r.top() * .5;
        painter->drawLine(LineF(x1, y1, x1, y2));
    }
    break;

    case ArpeggioType::DOWN_STRAIGHT:
    {
        RectF r(item->symBbox(SymId::arrowheadBlackDown));
        double x1 = _spatium * .5;

        item->drawSymbol(SymId::arrowheadBlackDown, painter, PointF(x1 - r.width() * .5, y2 - r.bottom()));
        y2 += r.top() * .5;
        painter->drawLine(LineF(x1, y1, x1, y2));
    }
    break;

    case ArpeggioType::BRACKET:
    {
        double w = item->style().styleS(Sid::ArpeggioHookLen).val() * _spatium;
        painter->drawLine(LineF(0.0, y1, w, y1));
        painter->drawLine(LineF(0.0, y2, w, y2));
        painter->drawLine(LineF(0.0, y1 - lineWidth / 2, 0.0, y2 + lineWidth / 2));
    }
    break;
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

void TDraw::draw(const BagpipeEmbellishment* item, draw::Painter* painter)
{
    TRACE_DRAW_ITEM;
    using namespace mu::draw;
    SymId headsym = SymId::noteheadBlack;
    SymId flagsym = SymId::flag32ndUp;

    BagpipeNoteList nl = item->resolveNoteList();
    BagpipeEmbellishment::BEDrawingDataX dx(headsym, flagsym, item->magS(), item->style().spatium(), static_cast<int>(nl.size()));

    Pen pen(item->curColor(), dx.lw, PenStyle::SolidLine, PenCapStyle::FlatCap);
    painter->setPen(pen);

    bool drawBeam = nl.size() > 1;
    bool drawFlag = nl.size() == 1;

    auto drawGraceNote = [item](mu::draw::Painter* painter,
                                const BagpipeEmbellishment::BEDrawingDataX& dx,
                                const BagpipeEmbellishment::BEDrawingDataY& dy,
                                SymId flagsym, const double x, const bool drawFlag)
    {
        // draw head
        item->drawSymbol(dx.headsym, painter, mu::PointF(x - dx.headw, dy.y2));
        // draw stem
        double y1 =  drawFlag ? dy.y1f : dy.y1b;            // top of stems actually used
        painter->drawLine(mu::LineF(x - dx.lw * .5, y1, x - dx.lw * .5, dy.y2));
        if (drawFlag) {
            // draw flag
            item->drawSymbol(flagsym, painter, mu::PointF(x - dx.lw * .5 + dx.xcorr, y1 + dy.ycorr));
        }
    };

    // draw the notes including stem, (optional) flag and (optional) ledger line
    double x = dx.xl;
    for (int note : nl) {
        int line = BagpipeEmbellishment::BAGPIPE_NOTEINFO_LIST[note].line;
        BagpipeEmbellishment::BEDrawingDataY dy(line, item->style().spatium());
        drawGraceNote(painter, dx, dy, flagsym, x, drawFlag);

        // draw the ledger line for high A
        if (line == -2) {
            painter->drawLine(mu::LineF(x - dx.headw * 1.5 - dx.lw * .5, dy.y2, x + dx.headw * .5 - dx.lw * .5, dy.y2));
        }

        // move x to next note x position
        x += dx.headp;
    }

    if (drawBeam) {
        // beam drawing setup
        BagpipeEmbellishment::BEDrawingDataY dy(0, item->style().spatium());
        Pen beamPen(item->curColor(), dy.bw, PenStyle::SolidLine, PenCapStyle::FlatCap);
        painter->setPen(beamPen);
        // draw the beams
        auto drawBeams = [](mu::draw::Painter* painter, const double spatium,
                            const double x1, const double x2, double y)
        {
            // draw the beams
            painter->drawLine(mu::LineF(x1, y, x2, y));
            y += spatium / 1.5;
            painter->drawLine(mu::LineF(x1, y, x2, y));
            y += spatium / 1.5;
            painter->drawLine(mu::LineF(x1, y, x2, y));
        };

        drawBeams(painter, dx.spatium, dx.xl - dx.lw * .5, x - dx.headp - dx.lw * .5, dy.y1b);
    }
}
