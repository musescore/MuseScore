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

#include "draw/fontmetrics.h"

#include "style/style.h"
#include "types/typesconv.h"

#include "libmscore/accidental.h"
#include "libmscore/actionicon.h"
#include "libmscore/ambitus.h"
#include "libmscore/arpeggio.h"
#include "libmscore/articulation.h"

#include "libmscore/bagpembell.h"
#include "libmscore/barline.h"

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
    case ElementType::BAR_LINE:     draw(item_cast<const BarLine*>(item), painter);
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

static void drawDots(const BarLine* item, Painter* painter, double x)
{
    double _spatium = item->spatium();

    double y1l;
    double y2l;
    if (item->explicitParent() == 0) {      // for use in palette (always Bravura)
        //Bravura shifted repeatDot symbol 0.5sp upper in the font itself (1.272)
        y1l = 1.5 * _spatium;
        y2l = 2.5 * _spatium;
    } else {
        const StaffType* st = item->staffType();

        y1l = st->doty1() * _spatium;
        y2l = st->doty2() * _spatium;

        //workaround to make Emmentaler, Gonville and MuseJazz font work correctly with repeatDots
        if (item->score()->engravingFont()->name() == "Emmentaler"
            || item->score()->engravingFont()->name() == "Gonville"
            || item->score()->engravingFont()->name() == "MuseJazz") {
            double offset = 0.5 * item->style().spatium() * item->mag();
            y1l += offset;
            y2l += offset;
        }

        //adjust for staffType offset
        double stYOffset = st->yoffset().val() * _spatium;
        y1l += stYOffset;
        y2l += stYOffset;
    }

    item->drawSymbol(SymId::repeatDot, painter, PointF(x, y1l));
    item->drawSymbol(SymId::repeatDot, painter, PointF(x, y2l));
}

static void drawTips(const BarLine* item, Painter* painter, bool reversed, double x)
{
    if (reversed) {
        if (item->isTop()) {
            item->drawSymbol(SymId::reversedBracketTop, painter, PointF(x - item->symWidth(SymId::reversedBracketTop), item->y1()));
        }
        if (item->isBottom()) {
            item->drawSymbol(SymId::reversedBracketBottom, painter, PointF(x - item->symWidth(SymId::reversedBracketBottom), item->y2()));
        }
    } else {
        if (item->isTop()) {
            item->drawSymbol(SymId::bracketTop, painter, PointF(x, item->y1()));
        }
        if (item->isBottom()) {
            item->drawSymbol(SymId::bracketBottom, painter, PointF(x, item->y2()));
        }
    }
}

void TDraw::draw(const BarLine* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    switch (item->barLineType()) {
    case BarLineType::NORMAL: {
        double lw = item->style().styleMM(Sid::barWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(lw * .5, item->y1(), lw * .5, item->y2()));
    }
    break;

    case BarLineType::BROKEN: {
        double lw = item->style().styleMM(Sid::barWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::DashLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(lw * .5, item->y1(), lw * .5, item->y2()));
    }
    break;

    case BarLineType::DOTTED: {
        double lw = item->style().styleMM(Sid::barWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::DotLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(lw * .5, item->y1(), lw * .5, item->y2()));
    }
    break;

    case BarLineType::END: {
        double lw = item->style().styleMM(Sid::barWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        double x  = lw * .5;
        painter->drawLine(LineF(x, item->y1(), x, item->y2()));

        double lw2 = item->style().styleMM(Sid::endBarWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        x += ((lw * .5) + item->style().styleMM(Sid::endBarDistance) + (lw2 * .5)) * item->mag();
        painter->drawLine(LineF(x, item->y1(), x, item->y2()));
    }
    break;

    case BarLineType::DOUBLE: {
        double lw = item->style().styleMM(Sid::doubleBarWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        double x = lw * .5;
        painter->drawLine(LineF(x, item->y1(), x, item->y2()));
        x += ((lw * .5) + item->style().styleMM(Sid::doubleBarDistance) + (lw * .5)) * item->mag();
        painter->drawLine(LineF(x, item->y1(), x, item->y2()));
    }
    break;

    case BarLineType::REVERSE_END: {
        double lw = item->style().styleMM(Sid::endBarWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        double x = lw * .5;
        painter->drawLine(LineF(x, item->y1(), x, item->y2()));

        double lw2 = item->style().styleMM(Sid::barWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        x += ((lw * .5) + item->style().styleMM(Sid::endBarDistance) + (lw2 * .5)) * item->mag();
        painter->drawLine(LineF(x, item->y1(), x, item->y2()));
    }
    break;

    case BarLineType::HEAVY: {
        double lw = item->style().styleMM(Sid::endBarWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(lw * .5, item->y1(), lw * .5, item->y2()));
    }
    break;

    case BarLineType::DOUBLE_HEAVY: {
        double lw2 = item->style().styleMM(Sid::endBarWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        double x = lw2 * .5;
        painter->drawLine(LineF(x, item->y1(), x, item->y2()));
        x += ((lw2 * .5) + item->style().styleMM(Sid::endBarDistance) + (lw2 * .5)) * item->mag();
        painter->drawLine(LineF(x, item->y1(), x, item->y2()));
    }
    break;

    case BarLineType::START_REPEAT: {
        double lw2 = item->style().styleMM(Sid::endBarWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        double x = lw2 * .5;
        painter->drawLine(LineF(x, item->y1(), x, item->y2()));

        double lw = item->style().styleMM(Sid::barWidth) * item->mag();
        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        x += ((lw2 * .5) + item->style().styleMM(Sid::endBarDistance) + (lw * .5)) * item->mag();
        painter->drawLine(LineF(x, item->y1(), x, item->y2()));

        x += ((lw * .5) + item->style().styleMM(Sid::repeatBarlineDotSeparation)) * item->mag();
        drawDots(item, painter, x);

        if (item->style().styleB(Sid::repeatBarTips)) {
            drawTips(item, painter, false, 0.0);
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
        painter->drawLine(LineF(x, item->y1(), x, item->y2()));

        double lw2 = item->style().styleMM(Sid::endBarWidth) * item->mag();
        x += ((lw * .5) + item->style().styleMM(Sid::endBarDistance) + (lw2 * .5)) * item->mag();
        painter->setPen(Pen(item->curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(x, item->y1(), x, item->y2()));

        if (item->style().styleB(Sid::repeatBarTips)) {
            drawTips(item, painter, true, x + lw2 * .5);
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
        painter->drawLine(LineF(x, item->y1(), x, item->y2()));

        double lw2 = item->style().styleMM(Sid::endBarWidth) * item->mag();
        x += ((lw * .5) + item->style().styleMM(Sid::endBarDistance) + (lw2 * .5)) * item->mag();
        painter->setPen(Pen(item->curColor(), lw2, PenStyle::SolidLine, PenCapStyle::FlatCap));
        painter->drawLine(LineF(x, item->y1(), x, item->y2()));

        if (item->style().styleB(Sid::repeatBarTips)) {
            drawTips(item, painter, true, x + lw2 * .5);
        }

        painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
        x  += ((lw2 * .5) + item->style().styleMM(Sid::endBarDistance) + (lw * .5)) * item->mag();
        painter->drawLine(LineF(x, item->y1(), x, item->y2()));

        x += ((lw * .5) + item->style().styleMM(Sid::repeatBarlineDotSeparation)) * item->mag();
        drawDots(item, painter, x);

        if (item->style().styleB(Sid::repeatBarTips)) {
            drawTips(item, painter, false, 0.0);
        }
    }
    break;
    }
    Segment* s = item->segment();
    if (s && s->isEndBarLineType() && !item->score()->printing() && item->score()->showUnprintable()) {
        Measure* m = s->measure();
        if (m->isIrregular() && item->score()->markIrregularMeasures() && !m->isMMRest()) {
            painter->setPen(EngravingItem::engravingConfiguration()->formattingMarksColor());
            draw::Font f(u"Edwin", Font::Type::Text);
            f.setPointSizeF(12 * item->spatium() / SPATIUM20);
            f.setBold(true);
            Char ch = m->ticks() > m->timesig() ? u'+' : u'-';
            RectF r = FontMetrics(f).boundingRect(ch);

            mu::draw::Font scaledFont(f);
            scaledFont.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
            painter->setFont(scaledFont);

            painter->drawText(-r.width(), 0.0, ch);
        }
    }
}
