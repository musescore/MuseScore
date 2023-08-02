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
#include "singledraw.h"

#include "draw/painter.h"

#include "types/typesconv.h"
#include "style/style.h"
#include "style/defaultstyle.h"

#include "libmscore/accidental.h"
#include "libmscore/actionicon.h"
#include "libmscore/ambitus.h"
#include "libmscore/arpeggio.h"
#include "libmscore/articulation.h"

#include "libmscore/bagpembell.h"
#include "libmscore/barline.h"
#include "libmscore/beam.h"
#include "libmscore/bend.h"
#include "libmscore/bracket.h"
#include "libmscore/breath.h"

#include "libmscore/chordline.h"
#include "libmscore/clef.h"
#include "libmscore/capo.h"

#include "libmscore/deadslapped.h"
#include "libmscore/dynamic.h"

#include "libmscore/expression.h"

#include "libmscore/fermata.h"
#include "libmscore/figuredbass.h"
#include "libmscore/fingering.h"
#include "libmscore/fret.h"

#include "libmscore/glissando.h"

#include "libmscore/ornament.h"

#include "libmscore/stretchedbend.h"

#include "libmscore/textbase.h"

#include "infrastructure/rtti.h"

using namespace mu::engraving::rendering::single;
using namespace mu::engraving;
using namespace mu::engraving::rtti;
using namespace mu::draw;

void SingleDraw::drawItem(const EngravingItem* item, draw::Painter* painter)
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
    case ElementType::BAR_LINE:     draw(item_cast<const BarLine*>(item), painter);
        break;
    case ElementType::BEAM:         draw(item_cast<const Beam*>(item), painter);
        break;
    case ElementType::BEND:         draw(item_cast<const Bend*>(item), painter);
        break;
    case ElementType::BRACKET:      draw(item_cast<const Bracket*>(item), painter);
        break;
    case ElementType::BREATH:       draw(item_cast<const Breath*>(item), painter);
        break;

    case ElementType::CHORDLINE:    draw(item_cast<const ChordLine*>(item), painter);
        break;
    case ElementType::CLEF:         draw(item_cast<const Clef*>(item), painter);
        break;
    case ElementType::CAPO:         draw(item_cast<const Capo*>(item), painter);
        break;

    case ElementType::DEAD_SLAPPED: draw(item_cast<const DeadSlapped*>(item), painter);
        break;
    case ElementType::DYNAMIC:      draw(item_cast<const Dynamic*>(item), painter);
        break;

    case ElementType::EXPRESSION:   draw(item_cast<const Expression*>(item), painter);
        break;

    case ElementType::FERMATA:      draw(item_cast<const Fermata*>(item), painter);
        break;
    case ElementType::FIGURED_BASS: draw(item_cast<const FiguredBass*>(item), painter);
        break;
    case ElementType::FINGERING:    draw(item_cast<const Fingering*>(item), painter);
        break;
    case ElementType::FRET_DIAGRAM: draw(item_cast<const FretDiagram*>(item), painter);
        break;

    case ElementType::GLISSANDO_SEGMENT: draw(item_cast<const GlissandoSegment*>(item), painter);
        break;

    case ElementType::ORNAMENT:     draw(item_cast<const Ornament*>(item), painter);
        break;

    case ElementType::STRETCHED_BEND: draw(item_cast<const StretchedBend*>(item), painter);
        break;
    default:
        item->draw(painter);
    }
}

void SingleDraw::draw(const Accidental* item, draw::Painter* painter)
{
    painter->setPen(item->curColor());
    for (const Accidental::LayoutData::Sym& e : item->layoutData().syms) {
        item->drawSymbol(e.sym, painter, PointF(e.x, e.y));
    }
}

void SingleDraw::draw(const ActionIcon* item, draw::Painter* painter)
{
    TRACE_DRAW_ITEM;
    painter->setFont(item->iconFont());
    painter->drawText(item->bbox(), draw::AlignCenter, Char(item->icon()));
}

void SingleDraw::draw(const Ambitus* item, draw::Painter* painter)
{
    TRACE_DRAW_ITEM;
    using namespace mu::draw;
    double spatium = item->spatium();
    double lw = item->lineWidth().val() * spatium;
    painter->setPen(Pen(item->curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));

    const Ambitus::LayoutData& layoutData = item->layoutData();

    item->drawSymbol(item->noteHead(), painter, layoutData.topPos);
    item->drawSymbol(item->noteHead(), painter, layoutData.bottomPos);
    if (item->hasLine()) {
        painter->drawLine(layoutData.line);
    }
}

void SingleDraw::draw(const Arpeggio* item, draw::Painter* painter)
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

void SingleDraw::draw(const Articulation* item, draw::Painter* painter)
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

void SingleDraw::draw(const Ornament* item, draw::Painter* painter)
{
    draw(static_cast<const Articulation*>(item), painter);
}

void SingleDraw::draw(const BagpipeEmbellishment* item, draw::Painter* painter)
{
    TRACE_DRAW_ITEM;

    const BagpipeEmbellishment::LayoutData& data = item->layoutData();
    const BagpipeEmbellishment::LayoutData::BeamData& dataBeam = data.beamData;

    Pen pen(item->curColor(), data.stemLineW, PenStyle::SolidLine, PenCapStyle::FlatCap);
    painter->setPen(pen);

    // draw the notes including stem, (optional) flag and (optional) ledger line
    for (const auto& p : data.notesData) {
        const BagpipeEmbellishment::LayoutData::NoteData& noteData = p.second;

        // Draw Grace Note
        {
            // draw head
            item->drawSymbol(data.headsym, painter, noteData.headXY);

            // draw stem
            painter->drawLine(noteData.stemLine);

            if (data.isDrawFlag) {
                // draw flag
                item->drawSymbol(data.flagsym, painter, noteData.flagXY);
            }
        }

        // draw the ledger line for high A
        if (!noteData.ledgerLine.isNull()) {
            painter->drawLine(noteData.ledgerLine);
        }
    }

    if (data.isDrawBeam) {
        Pen beamPen(item->curColor(), dataBeam.width, PenStyle::SolidLine, PenCapStyle::FlatCap);
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

        drawBeams(painter, data.spatium, dataBeam.x1, dataBeam.x2, dataBeam.y);
    }
}

static void drawDots(const BarLine* item, Painter* painter, double x)
{
    double spatium = item->spatium();

    double y1l = 1.5 * spatium;
    double y2l = 2.5 * spatium;

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

void SingleDraw::draw(const BarLine* item, Painter* painter)
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
}

void SingleDraw::draw(const Beam* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    if (item->beamSegments().empty()) {
        return;
    }
    painter->setBrush(Brush(item->curColor()));
    painter->setNoPen();

    // make beam thickness independent of slant
    // (expression can be simplified?)

    const LineF bs = item->beamSegments().front()->line;
    double d  = (std::abs(bs.y2() - bs.y1())) / (bs.x2() - bs.x1());
    if (item->beamSegments().size() > 1 && d > M_PI / 6.0) {
        d = M_PI / 6.0;
    }
    double ww = (item->beamWidth() / 2.0) / sin(M_PI_2 - atan(d));

    for (const BeamSegment* bs1 : item->beamSegments()) {
        painter->drawPolygon(
            PolygonF({
            PointF(bs1->line.x1(), bs1->line.y1() - ww),
            PointF(bs1->line.x2(), bs1->line.y2() - ww),
            PointF(bs1->line.x2(), bs1->line.y2() + ww),
            PointF(bs1->line.x1(), bs1->line.y1() + ww),
        }),
            draw::FillRule::OddEvenFill);
    }
}

void SingleDraw::draw(const Bend* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    double _spatium = item->spatium();
    double _lw = item->lineWidth();

    Pen pen(item->curColor(), _lw, PenStyle::SolidLine, PenCapStyle::RoundCap, PenJoinStyle::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(Brush(item->curColor()));

    mu::draw::Font f = item->font(_spatium * MScore::pixelRatio);
    painter->setFont(f);

    double x  = item->noteWidth() + _spatium * .2;
    double y  = -_spatium * .8;
    double x2, y2;

    double aw = item->style().styleMM(Sid::bendArrowWidth);
    PolygonF arrowUp;
    arrowUp << PointF(0, 0) << PointF(aw * .5, aw) << PointF(-aw * .5, aw);
    PolygonF arrowDown;
    arrowDown << PointF(0, 0) << PointF(aw * .5, -aw) << PointF(-aw * .5, -aw);

    size_t n = item->points().size();
    for (size_t pt = 0; pt < n - 1; ++pt) {
        int pitch = item->points()[pt].pitch;
        if (pt == 0 && pitch) {
            y2 = -item->notePos().y() - _spatium * 2;
            x2 = x;
            painter->drawLine(LineF(x, y, x2, y2));

            painter->setBrush(item->curColor());
            painter->drawPolygon(arrowUp.translated(x2, y2));

            int idx = (pitch + 12) / 25;
            const char* l = item->label[idx];
            painter->drawText(RectF(x2, y2, .0, .0),
                              draw::AlignHCenter | draw::AlignBottom | draw::TextDontClip,
                              String::fromAscii(l));

            y = y2;
        }
        if (pitch == item->points()[pt + 1].pitch) {
            if (pt == (n - 2)) {
                break;
            }
            x2 = x + _spatium;
            y2 = y;
            painter->drawLine(LineF(x, y, x2, y2));
        } else if (pitch < item->points()[pt + 1].pitch) {
            // up
            x2 = x + _spatium * .5;
            y2 = -item->notePos().y() - _spatium * 2;
            double dx = x2 - x;
            double dy = y2 - y;

            PainterPath path;
            path.moveTo(x, y);
            path.cubicTo(x + dx / 2, y, x2, y + dy / 4, x2, y2);
            painter->setBrush(BrushStyle::NoBrush);
            painter->drawPath(path);

            painter->setBrush(item->curColor());
            painter->drawPolygon(arrowUp.translated(x2, y2));

            int idx = (item->points()[pt + 1].pitch + 12) / 25;
            const char* l = item->label[idx];
            double ty = y2;       // - _spatium;
            painter->drawText(RectF(x2, ty, .0, .0),
                              draw::AlignHCenter | draw::AlignBottom | draw::TextDontClip,
                              String::fromAscii(l));
        } else {
            // down
            x2 = x + _spatium * .5;
            y2 = y + _spatium * 3;
            double dx = x2 - x;
            double dy = y2 - y;

            PainterPath path;
            path.moveTo(x, y);
            path.cubicTo(x + dx / 2, y, x2, y + dy / 4, x2, y2);
            painter->setBrush(BrushStyle::NoBrush);
            painter->drawPath(path);

            painter->setBrush(item->curColor());
            painter->drawPolygon(arrowDown.translated(x2, y2));
        }
        x = x2;
        y = y2;
    }
}

void SingleDraw::draw(const Bracket* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    if (RealIsNull(item->h2())) {
        return;
    }
    switch (item->bracketType()) {
    case BracketType::BRACE: {
        if (item->braceSymbol() == SymId::noSym) {
            painter->setNoPen();
            painter->setBrush(Brush(item->curColor()));
            painter->drawPath(item->path());
        } else {
            double h        = 2 * item->h2();
            double mag      = h / (100 * item->magS());
            painter->setPen(item->curColor());
            painter->save();
            painter->scale(item->magx(), mag);
            item->drawSymbol(item->braceSymbol(), painter, PointF(0, 100 * item->magS()));
            painter->restore();
        }
    }
    break;
    case BracketType::NORMAL: {
        double h        = 2 * item->h2();
        double _spatium = item->spatium();
        double w        = item->style().styleMM(Sid::bracketWidth);
        double bd       = (item->style().styleSt(Sid::MusicalSymbolFont) == "Leland") ? _spatium * .5 : _spatium * .25;
        Pen pen(item->curColor(), w, PenStyle::SolidLine, PenCapStyle::FlatCap);
        painter->setPen(pen);
        painter->drawLine(LineF(0.0, -bd - w * .5, 0.0, h + bd + w * .5));
        double x    =  -w * .5;
        double y1   = -bd;
        double y2   = h + bd;
        item->drawSymbol(SymId::bracketTop, painter, PointF(x, y1));
        item->drawSymbol(SymId::bracketBottom, painter, PointF(x, y2));
    }
    break;
    case BracketType::SQUARE: {
        double h = 2 * item->h2();
        double lineW = item->style().styleMM(Sid::staffLineWidth);
        double bracketWidth = item->width() - lineW / 2;
        Pen pen(item->curColor(), lineW, PenStyle::SolidLine, PenCapStyle::FlatCap);
        painter->setPen(pen);
        painter->drawLine(LineF(0.0, 0.0, 0.0, h));
        painter->drawLine(LineF(-lineW / 2, 0.0, lineW / 2 + bracketWidth, 0.0));
        painter->drawLine(LineF(-lineW / 2, h, lineW / 2 + bracketWidth, h));
    }
    break;
    case BracketType::LINE: {
        double h = 2 * item->h2();
        double w = 0.67 * item->style().styleMM(Sid::bracketWidth);
        Pen pen(item->curColor(), w, PenStyle::SolidLine, PenCapStyle::FlatCap);
        painter->setPen(pen);
        double bd = item->style().styleMM(Sid::staffLineWidth) * 0.5;
        painter->drawLine(LineF(0.0, -bd, 0.0, h + bd));
    }
    break;
    case BracketType::NO_BRACKET:
        break;
    }
}

void SingleDraw::draw(const Breath* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    painter->setPen(item->curColor());
    item->drawSymbol(item->symId(), painter);
}

void SingleDraw::draw(const ChordLine* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    if (!item->isWavy()) {
        painter->setPen(Pen(item->curColor(), item->style().styleMM(Sid::chordlineThickness) * item->mag(), PenStyle::SolidLine));
        painter->setBrush(BrushStyle::NoBrush);
        painter->drawPath(item->path());
    } else {
        painter->save();
        painter->rotate((item->chordLineType() == ChordLineType::FALL ? 1 : -1) * ChordLine::WAVE_ANGEL);
        item->drawSymbols(ChordLine::WAVE_SYMBOLS, painter);
        painter->restore();
    }
}

void SingleDraw::draw(const Clef* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    if (item->symId() == SymId::noSym) {
        return;
    }
    painter->setPen(item->curColor());
    item->drawSymbol(item->symId(), painter);
}

void SingleDraw::draw(const Capo* item, Painter* painter)
{
    drawTextBase(item, painter);
}

void SingleDraw::draw(const DeadSlapped* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    painter->setPen(draw::PenStyle::NoPen);
    painter->setBrush(item->curColor());
    painter->drawPath(item->path1());
    painter->drawPath(item->path2());
}

void SingleDraw::draw(const Dynamic* item, Painter* painter)
{
    drawTextBase(item, painter);
}

void SingleDraw::draw(const Expression* item, Painter* painter)
{
    drawTextBase(item, painter);
}

void SingleDraw::draw(const Fermata* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    painter->setPen(item->curColor());
    item->drawSymbol(item->symId(), painter, PointF(-0.5 * item->width(), 0.0));
}

void SingleDraw::draw(const FiguredBass* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    if (item->items().size() < 1) {                                 // if not parseable into f.b. items
        drawTextBase(item, painter);                                // draw as standard text
    } else {
        for (FiguredBassItem* fi : item->items()) {               // if parseable into f.b. items
            painter->translate(fi->pos());                // draw each item in its proper position
            fi->draw(painter);
            painter->translate(-fi->pos());
        }
    }
}

void SingleDraw::draw(const Fingering* item, Painter* painter)
{
    TRACE_DRAW_ITEM;
    drawTextBase(item, painter);
}

void SingleDraw::draw(const FretDiagram* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    PointF translation = -PointF(item->stringDist() * (item->strings() - 1), 0);
    if (item->orientation() == Orientation::HORIZONTAL) {
        painter->save();
        painter->rotate(-90);
        painter->translate(translation);
    }

    // Init pen and other values
    double _spatium = item->spatium() * item->userMag();
    Pen pen(item->curColor());
    pen.setCapStyle(PenCapStyle::FlatCap);
    painter->setBrush(Brush(Color(painter->pen().color())));

    // x2 is the x val of the rightmost string
    double x2 = (item->strings() - 1) * item->stringDist();

    // Draw the nut
    pen.setWidthF(item->nutLw());
    painter->setPen(pen);
    painter->drawLine(LineF(-item->stringLw() * .5, 0.0, x2 + item->stringLw() * .5, 0.0));

    // Draw strings and frets
    pen.setWidthF(item->stringLw());
    painter->setPen(pen);

    // y2 is the y val of the bottom fretline
    double y2 = item->fretDist() * (item->frets() + .5);
    for (int i = 0; i < item->strings(); ++i) {
        double x = item->stringDist() * i;
        painter->drawLine(LineF(x, item->fretOffset() ? -_spatium * .2 : 0.0, x, y2));
    }
    for (int i = 1; i <= item->frets(); ++i) {
        double y = item->fretDist() * i;
        painter->drawLine(LineF(0.0, y, x2, y));
    }

    // dotd is the diameter of a dot
    double dotd = _spatium * .49 * item->style().styleD(Sid::fretDotSize);

    // Draw dots, sym pen is used to draw them (and markers)
    Pen symPen(pen);
    symPen.setCapStyle(PenCapStyle::RoundCap);
    double symPenWidth = item->stringLw() * 1.2;
    symPen.setWidthF(symPenWidth);

    for (auto const& i : item->dots()) {
        for (auto const& d : i.second) {
            if (!d.exists()) {
                continue;
            }

            int string = i.first;
            int fret = d.fret - 1;

            // Calculate coords of the top left corner of the dot
            double x = item->stringDist() * string - dotd * .5;
            double y = item->fretDist() * fret + item->fretDist() * .5 - dotd * .5;

            // Draw different symbols
            painter->setPen(symPen);
            switch (d.dtype) {
            case FretDotType::CROSS:
                // Give the cross a slightly larger width
                symPen.setWidthF(symPenWidth * 1.5);
                painter->setPen(symPen);
                painter->drawLine(LineF(x, y, x + dotd, y + dotd));
                painter->drawLine(LineF(x + dotd, y, x, y + dotd));
                symPen.setWidthF(symPenWidth);
                break;
            case FretDotType::SQUARE:
                painter->setBrush(BrushStyle::NoBrush);
                painter->drawRect(RectF(x, y, dotd, dotd));
                break;
            case FretDotType::TRIANGLE:
                painter->drawLine(LineF(x, y + dotd, x + .5 * dotd, y));
                painter->drawLine(LineF(x + .5 * dotd, y, x + dotd, y + dotd));
                painter->drawLine(LineF(x + dotd, y + dotd, x, y + dotd));
                break;
            case FretDotType::NORMAL:
            default:
                painter->setBrush(symPen.color());
                painter->setNoPen();
                painter->drawEllipse(RectF(x, y, dotd, dotd));
                break;
            }
        }
    }

    // Draw markers
    symPen.setWidthF(symPenWidth * 1.2);
    painter->setBrush(BrushStyle::NoBrush);
    painter->setPen(symPen);
    for (auto const& i : item->markers()) {
        int string = i.first;
        FretItem::Marker marker = i.second;
        if (!marker.exists()) {
            continue;
        }

        double x = item->stringDist() * string - item->markerSize() * .5;
        double y = -item->fretDist() - item->markerSize() * .5;
        if (marker.mtype == FretMarkerType::CIRCLE) {
            painter->drawEllipse(RectF(x, y, item->markerSize(), item->markerSize()));
        } else if (marker.mtype == FretMarkerType::CROSS) {
            painter->drawLine(PointF(x, y), PointF(x + item->markerSize(), y + item->markerSize()));
            painter->drawLine(PointF(x, y + item->markerSize()), PointF(x + item->markerSize(), y));
        }
    }

    // Draw barres
    for (auto const& i : item->barres()) {
        int fret        = i.first;
        int startString = i.second.startString;
        int endString   = i.second.endString;

        double x1    = item->stringDist() * startString;
        double newX2 = endString == -1 ? x2 : item->stringDist() * endString;
        double y     = item->fretDist() * (fret - 1) + item->fretDist() * .5;
        pen.setWidthF(dotd * item->style().styleD(Sid::barreLineWidth));
        pen.setCapStyle(PenCapStyle::RoundCap);
        painter->setPen(pen);
        painter->drawLine(LineF(x1, y, newX2, y));
    }

    // Draw fret offset number
    if (item->fretOffset() > 0) {
        double fretNumMag = item->style().styleD(Sid::fretNumMag);
        mu::draw::Font scaledFont(item->font());
        scaledFont.setPointSizeF(item->font().pointSizeF()
                                 * item->userMag()
                                 * (item->spatium() / SPATIUM20)
                                 * MScore::pixelRatio
                                 * fretNumMag);
        painter->setFont(scaledFont);
        String text = String::number(item->fretOffset() + 1);

        if (item->orientation() == Orientation::VERTICAL) {
            if (item->numPos() == 0) {
                painter->drawText(RectF(-item->stringDist() * .4, .0, .0, item->fretDist()),
                                  draw::AlignVCenter | draw::AlignRight | draw::TextDontClip, text);
            } else {
                painter->drawText(RectF(x2 + (item->stringDist() * .4), .0, .0, item->fretDist()),
                                  draw::AlignVCenter | draw::AlignLeft | draw::TextDontClip,
                                  String::number(item->fretOffset() + 1));
            }
        } else if (item->orientation() == Orientation::HORIZONTAL) {
            painter->save();
            painter->translate(-translation);
            painter->rotate(90);
            if (item->numPos() == 0) {
                painter->drawText(RectF(.0, item->stringDist() * (item->strings() - 1), .0, .0),
                                  draw::AlignLeft | draw::TextDontClip, text);
            } else {
                painter->drawText(RectF(.0, .0, .0, .0), draw::AlignBottom | draw::AlignLeft | draw::TextDontClip, text);
            }
            painter->restore();
        }
        painter->setFont(item->font());
    }

    // NOTE:JT possible future todo - draw fingerings

    if (item->orientation() == Orientation::HORIZONTAL) {
        painter->restore();
    }
}

void SingleDraw::draw(const GlissandoSegment* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    if (item->pos2().x() <= 0) {
        return;
    }

    painter->save();
    double _spatium = item->spatium();
    const Glissando* glissando = item->glissando();

    Pen pen(item->curColor(item->visible(), glissando->lineColor()));
    pen.setWidthF(glissando->lineWidth());
    pen.setCapStyle(PenCapStyle::RoundCap);
    painter->setPen(pen);

    // rotate painter so that the line become horizontal
    double w     = item->pos2().x();
    double h     = item->pos2().y();
    double l     = sqrt(w * w + h * h);
    double wi    = asin(-h / l) * 180.0 / M_PI;
    painter->rotate(-wi);

    if (glissando->glissandoType() == GlissandoType::STRAIGHT) {
        painter->drawLine(LineF(0.0, 0.0, l, 0.0));
    } else if (glissando->glissandoType() == GlissandoType::WAVY) {
        RectF b = item->symBbox(SymId::wiggleTrill);
        double a  = item->symAdvance(SymId::wiggleTrill);
        int n    = static_cast<int>(l / a);          // always round down (truncate) to avoid overlap
        double x  = (l - n * a) * 0.5;     // centre line in available space
        SymIdList ids;
        for (int i = 0; i < n; ++i) {
            ids.push_back(SymId::wiggleTrill);
        }

        item->drawSymbols(ids, painter, PointF(x, -(b.y() + b.height() * 0.5)));
    }

    if (glissando->showText()) {
        mu::draw::Font f(glissando->fontFace(), draw::Font::Type::Unknown);
        f.setPointSizeF(glissando->fontSize() * _spatium / SPATIUM20);
        f.setBold(glissando->fontStyle() & FontStyle::Bold);
        f.setItalic(glissando->fontStyle() & FontStyle::Italic);
        f.setUnderline(glissando->fontStyle() & FontStyle::Underline);
        f.setStrike(glissando->fontStyle() & FontStyle::Strike);
        mu::draw::FontMetrics fm(f);
        RectF r = fm.boundingRect(glissando->text());

        // if text longer than available space, skip it
        if (r.width() < l) {
            double yOffset = r.height() + r.y();             // find text descender height
            // raise text slightly above line and slightly more with WAVY than with STRAIGHT
            yOffset += _spatium * (glissando->glissandoType() == GlissandoType::WAVY ? 0.4 : 0.1);

            mu::draw::Font scaledFont(f);
            scaledFont.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
            painter->setFont(scaledFont);

            double x = (l - r.width()) * 0.5;
            painter->drawText(PointF(x, -yOffset), glissando->text());
        }
    }
    painter->restore();
}

void SingleDraw::draw(const StretchedBend* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    double sp = item->spatium();
    const mu::draw::Color& color = item->curColor();
    const int textFlags = item->textFlags();

    Pen pen(color, item->lineWidth(), PenStyle::SolidLine, PenCapStyle::RoundCap, PenJoinStyle::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(Brush(color));
    mu::draw::Font f = item->font(sp * MScore::pixelRatio);
    painter->setFont(f);

    bool isTextDrawn = false;

    for (const StretchedBend::BendSegment& bendSegment : item->bendSegmentsStretched()) {
        if (!bendSegment.visible) {
            continue;
        }

        const PointF& src = bendSegment.src;
        const PointF& dest = bendSegment.dest;
        const String& text = item->toneToLabel(bendSegment.tone);

        switch (bendSegment.type) {
        case StretchedBend::BendSegmentType::LINE_UP:
        {
            painter->drawLine(LineF(src, dest));
            painter->setBrush(color);
            painter->drawPolygon(item->arrows().up.translated(dest));
            /// TODO: remove substraction after fixing bRect
            PointF pos = dest - PointF(0, sp * 0.5);
            painter->drawText(RectF(pos.x(), pos.y(), .0, .0), textFlags, text);
            break;
        }

        case StretchedBend::BendSegmentType::CURVE_UP:
        case StretchedBend::BendSegmentType::CURVE_DOWN:
        {
            bool bendUp = (bendSegment.type == StretchedBend::BendSegmentType::CURVE_UP);
            double endY = dest.y() + item->arrows().width * (bendUp ? 1 : -1);

            PainterPath path = item->bendCurveFromPoints(src, PointF(dest.x(), endY));
            const auto& arrowPath = (bendUp ? item->arrows().up : item->arrows().down);

            painter->setBrush(BrushStyle::NoBrush);
            painter->drawPath(path);
            painter->setBrush(color);
            painter->drawPolygon(arrowPath.translated(dest));

            if (bendUp && !isTextDrawn) {
                /// TODO: remove subtraction after fixing bRect
                PointF pos = dest - PointF(0, sp * 0.5);
                painter->drawText(RectF(pos.x(), pos.y(), .0, .0), textFlags, text);
                isTextDrawn = true;
            }

            break;
        }

        case StretchedBend::BendSegmentType::LINE_STROKED:
        {
            PainterPath path;
            path.moveTo(src + PointF(item->arrows().width, 0));
            path.lineTo(dest);
            Pen p(painter->pen());
            p.setStyle(PenStyle::DashLine);
            painter->strokePath(path, p);
            break;
        }

        default:
            break;
        }
    }
}

void SingleDraw::drawTextBase(const TextBase* item, Painter* painter)
{
    TRACE_DRAW_ITEM;

    if (item->hasFrame()) {
        double baseSpatium = DefaultStyle::baseStyle().value(Sid::spatium).toReal();
        if (item->frameWidth().val() != 0.0) {
            Color fColor = item->curColor(item->visible(), item->frameColor());
            double frameWidthVal = item->frameWidth().val() * (item->sizeIsSpatiumDependent() ? item->spatium() : baseSpatium);

            Pen pen(fColor, frameWidthVal, PenStyle::SolidLine, PenCapStyle::SquareCap, PenJoinStyle::MiterJoin);
            painter->setPen(pen);
        } else {
            painter->setNoPen();
        }
        Color bg(item->bgColor());
        painter->setBrush(bg.alpha() ? Brush(bg) : BrushStyle::NoBrush);
        if (item->circle()) {
            painter->drawEllipse(item->frame());
        } else {
            double frameRoundFactor = (item->sizeIsSpatiumDependent() ? (item->spatium() / baseSpatium) / 2 : 0.5f);

            int r2 = item->frameRound() * frameRoundFactor;
            if (r2 > 99) {
                r2 = 99;
            }
            painter->drawRoundedRect(item->frame(), item->frameRound() * frameRoundFactor, r2);
        }
    }
    painter->setBrush(BrushStyle::NoBrush);
    painter->setPen(item->textColor());
    for (const TextBlock& t : item->blocks()) {
        t.draw(painter, item);
    }
}
