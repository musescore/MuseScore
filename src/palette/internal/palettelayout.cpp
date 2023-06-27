/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "palettelayout.h"

#include "draw/fontmetrics.h"

#include "engraving/types/typesconv.h"

#include "engraving/libmscore/engravingitem.h"
#include "engraving/libmscore/score.h"

#include "engraving/libmscore/accidental.h"
#include "engraving/libmscore/actionicon.h"
#include "engraving/libmscore/ambitus.h"
#include "engraving/libmscore/articulation.h"
#include "engraving/libmscore/bagpembell.h"
#include "engraving/libmscore/barline.h"
#include "engraving/libmscore/bracket.h"
#include "engraving/libmscore/clef.h"
#include "engraving/libmscore/fret.h"
#include "engraving/libmscore/keysig.h"
#include "engraving/libmscore/symbol.h"
#include "engraving/libmscore/timesig.h"

#include "engraving/libmscore/utils.h"

#include "engraving/layout/pal/tlayout.h"

#include "log.h"

using namespace mu::draw;
using namespace mu::engraving;
using namespace mu::palette;

void PaletteLayout::layoutItem(EngravingItem* item)
{
    layout::pal::LayoutContext ctxpal(item->score());
    Context ctx(item->score());

    switch (item->type()) {
    case ElementType::ACCIDENTAL:   layout(toAccidental(item), ctx);
        break;
    case ElementType::ACTION_ICON:  layout(toActionIcon(item), ctx);
        break;
    case ElementType::AMBITUS:      layout(toAmbitus(item), ctx);
        break;
    case ElementType::ARTICULATION: layout(toArticulation(item), ctx);
        break;
    case ElementType::BAGPIPE_EMBELLISHMENT: layout(toBagpipeEmbellishment(item), ctx);
        break;
    case ElementType::BAR_LINE:     layout(toBarLine(item), ctx);
        break;
    case ElementType::BRACKET:      layout(toBracket(item), ctx);
        break;
    case ElementType::CLEF:         layout(toClef(item), ctx);
        break;
    case ElementType::FRET_DIAGRAM: layout(toFretDiagram(item), ctx);
        break;
    case ElementType::KEYSIG:       layout(toKeySig(item), ctx);
        break;
    case ElementType::SYMBOL:       layout(toSymbol(item), ctx);
        break;
    case ElementType::TIMESIG:      layout(toTimeSig(item), ctx);
        break;
    default:
        LOGD() << item->typeName();
        if (std::string("Symbol") == item->typeName()) {
            int k = -1;
        }
        layout::pal::TLayout::layoutItem(item, ctxpal);
        break;
    }
}

const MStyle& PaletteLayout::Context::style() const
{
    return m_score->style();
}

std::shared_ptr<IEngravingFont> PaletteLayout::Context::engravingFont() const
{
    return m_score->engravingFont();
}

void PaletteLayout::layout(Accidental* item, const Context&)
{
    SymId s = item->symId();
    if (item->elements().empty()) {
        SymElement e(s, 0.0, 0.0);
        item->addElement(e);
    }

    RectF bbox = item->symBbox(s);
    item->setbbox(bbox);
}

void PaletteLayout::layout(ActionIcon* item, const Context&)
{
    FontMetrics fontMetrics(item->iconFont());
    item->setbbox(fontMetrics.boundingRect(Char(item->icon())));
}

void PaletteLayout::layout(Ambitus* item, const Context& ctx)
{
    double headWdt = item->headWidth();
    double spatium = item->spatium();

    double lineDist    = spatium;
    int numOfLines  = 3;

    //
    // NOTEHEADS Y POS
    //
    // if pitch == INVALID_PITCH or tpc == Tpc::TPC_INVALID, set to some default:
    // for use in palettes and when actual range cannot be calculated (new ambitus or no notes in staff)
    //

    // top notehead
    item->setTopPosY(0.0);

    // bottom notehead
    item->setBottomPosY((numOfLines - 1) * lineDist);

    //
    // NOTEHEAD X POS
    //
    // Note: manages colliding accidentals
    //
    double accNoteDist = item->point(ctx.style().styleS(Sid::accidentalNoteDistance));
    double xAccidOffTop = item->topAccidental()->width() + accNoteDist;
    double xAccidOffBottom = item->bottomAccidental()->width() + accNoteDist;

    switch (item->direction()) {
    case DirectionH::AUTO:                       // noteheads one above the other
        // left align noteheads and right align accidentals 'hanging' on the left
        item->setTopPosX(0.0);
        item->setBottomPosX(0.0);
        item->topAccidental()->setPosX(-xAccidOffTop);
        item->bottomAccidental()->setPosX(-xAccidOffBottom);
        break;
    case DirectionH::LEFT:                       // top notehead at the left of bottom notehead
        // place top notehead at left margin; bottom notehead at right of top head;
        // top accid. 'hanging' on left of top head and bottom accid. 'hanging' at left of bottom head
        item->setTopPosX(0.0);
        item->setBottomPosX(headWdt);
        item->topAccidental()->setPosX(-xAccidOffTop);
        item->bottomAccidental()->setPosX(headWdt - xAccidOffBottom);
        break;
    case DirectionH::RIGHT:                      // top notehead at the right of bottom notehead
        // bottom notehead at left margin; top notehead at right of bottomnotehead
        // top accid. 'hanging' on left of top head and bottom accid. 'hanging' at left of bottom head
        item->setBottomPosX(0.0);
        item->setTopPosX(headWdt);
        item->bottomAccidental()->setPosX(-xAccidOffBottom);
        item->topAccidental()->setPosX(headWdt - xAccidOffTop);
        break;
    }

    // compute line from top note centre to bottom note centre
    LineF fullLine(item->topPos().x() + headWdt * 0.5,
                   item->topPos().y(),
                   item->bottomPos().x() + headWdt * 0.5,
                   item->bottomPos().y());
    // shorten line on each side by offsets
    double yDelta = item->bottomPos().y() - item->topPos().y();
    if (yDelta != 0.0) {
        double off = spatium * Ambitus::LINEOFFSET_DEFAULT;
        PointF p1 = fullLine.pointAt(off / yDelta);
        PointF p2 = fullLine.pointAt(1 - (off / yDelta));
        item->setLine(LineF(p1, p2));
    } else {
        item->setLine(fullLine);
    }

    RectF headRect(0, -0.5 * spatium, headWdt, 1 * spatium);
    item->setbbox(headRect.translated(item->topPos()).united(headRect.translated(item->bottomPos()))
                  .united(item->topAccidental()->bbox().translated(item->topAccidental()->ipos()))
                  .united(item->bottomAccidental()->bbox().translated(item->bottomAccidental()->ipos()))
                  );
}

void PaletteLayout::layout(Articulation* item, const Context&)
{
    RectF bbox;

    if (item->textType() != ArticulationTextType::NO_TEXT) {
        mu::draw::Font scaledFont(item->font());
        scaledFont.setPointSizeF(item->font().pointSizeF() * item->magS());
        mu::draw::FontMetrics fm(scaledFont);
        bbox = fm.boundingRect(scaledFont, TConv::text(item->textType()));
    } else {
        bbox = item->symBbox(item->symId());
    }

    item->setbbox(bbox.translated(-0.5 * bbox.width(), 0.0));
}

void PaletteLayout::layout(BagpipeEmbellishment* item, const Context& ctx)
{
    SymId headsym = SymId::noteheadBlack;
    SymId flagsym = SymId::flag32ndUp;

    noteList nl = item->getNoteList();
    BagpipeEmbellishment::BEDrawingDataX dx(headsym, flagsym, item->magS(), ctx.style().spatium(), static_cast<int>(nl.size()));

    item->setbbox(RectF());

    bool drawFlag = nl.size() == 1;

    // draw the notes including stem, (optional) flag and (optional) ledger line
    double x = dx.xl;
    for (int note : nl) {
        int line = BagpipeEmbellishment::BagpipeNoteInfoList[note].line;
        BagpipeEmbellishment::BEDrawingDataY dy(line, ctx.style().spatium());

        // head
        RectF headBBox = ctx.engravingFont()->bbox(headsym, dx.mags);
        item->addbbox(headBBox.translated(PointF(x - dx.lw * .5 - dx.headw, dy.y2)));

        // stem
        // highest top of stems actually used is y1b
        item->addbbox(RectF(x - dx.lw * .5 - dx.headw, dy.y1b, dx.lw, dy.y2 - dy.y1b));

        // flag
        if (drawFlag) {
            RectF flagBBox = ctx.engravingFont()->bbox(flagsym, dx.mags);
            item->addbbox(flagBBox.translated(PointF(x - dx.lw * .5 + dx.xcorr, dy.y1f + dy.ycorr)));
            // printBBox(" notehead + stem + flag", bbox());
        }

        // draw the ledger line for high A
        if (line == -2) {
            item->addbbox(RectF(x - dx.headw * 1.5 - dx.lw * .5, dy.y2 - dx.lw * 2, dx.headw * 2, dx.lw));
        }

        // move x to next note x position
        x += dx.headp;
    }
}

void PaletteLayout::layout(BarLine* item, const Context& ctx)
{
    item->setPos(PointF());
    item->setMag(1.0);

    double spatium = item->spatium();
    item->setY1(spatium * .5 * item->spanFrom());
    if (RealIsEqual(item->y2(), 0.0)) {
        item->setY2(spatium * .5 * (8.0 + item->spanTo()));
    }

    auto layoutWidth = [](BarLine* item, const Context& ctx) {
        const double dotWidth = item->symWidth(SymId::repeatDot);

        double w = 0.0;
        switch (item->barLineType()) {
        case BarLineType::DOUBLE:
            w = ctx.style().styleMM(Sid::doubleBarWidth) * 2.0 + ctx.style().styleMM(Sid::doubleBarDistance);
            break;
        case BarLineType::DOUBLE_HEAVY:
            w = ctx.style().styleMM(Sid::endBarWidth) * 2.0 + ctx.style().styleMM(Sid::endBarDistance);
            break;
        case BarLineType::END_START_REPEAT:
            w = ctx.style().styleMM(Sid::endBarWidth)
                + ctx.style().styleMM(Sid::barWidth) * 2.0
                + ctx.style().styleMM(Sid::endBarDistance) * 2.0
                + ctx.style().styleMM(Sid::repeatBarlineDotSeparation) * 2.0
                + dotWidth * 2;
            break;
        case BarLineType::START_REPEAT:
        case BarLineType::END_REPEAT:
            w = ctx.style().styleMM(Sid::endBarWidth)
                + ctx.style().styleMM(Sid::barWidth)
                + ctx.style().styleMM(Sid::endBarDistance)
                + ctx.style().styleMM(Sid::repeatBarlineDotSeparation)
                + dotWidth;
            break;
        case BarLineType::END:
        case BarLineType::REVERSE_END:
            w = ctx.style().styleMM(Sid::endBarWidth)
                + ctx.style().styleMM(Sid::barWidth)
                + ctx.style().styleMM(Sid::endBarDistance);
            break;
        case BarLineType::BROKEN:
        case BarLineType::NORMAL:
        case BarLineType::DOTTED:
            w = ctx.style().styleMM(Sid::barWidth);
            break;
        case BarLineType::HEAVY:
            w = ctx.style().styleMM(Sid::endBarWidth);
            break;
        }
        return w;
    };

    double w = layoutWidth(item, ctx) * item->mag();
    RectF bbox(0.0, item->y1(), w, item->y2() - item->y1());
    item->setbbox(bbox);
}

void PaletteLayout::layout(Bracket* item, const Context& ctx)
{
    Shape shape;

    switch (item->bracketType()) {
    case BracketType::BRACE: {
        if (item->braceSymbol() == SymId::noSym) {
            item->setBraceSymbol(SymId::brace);
        }
        double h = item->h2() * 2;
        double w = item->symWidth(item->braceSymbol()) * item->magx();
        item->bbox().setRect(0, 0, w, h);
        shape.add(item->bbox());
    }
    break;
    case BracketType::NORMAL: {
        double spatium = item->spatium();
        double w = ctx.style().styleMM(Sid::bracketWidth) * 0.5;
        double x = -w;

        double bd = spatium * 0.5;
        shape.add(RectF(x, -bd, w * 2, 2 * (item->h2() + bd)));
        shape.add(item->symBbox(SymId::bracketTop).translated(PointF(-w, -bd)));
        shape.add(item->symBbox(SymId::bracketBottom).translated(PointF(-w, bd + 2 * item->h2())));

        w += item->symWidth(SymId::bracketTop);
        double y = -item->symHeight(SymId::bracketTop) - bd;
        double h = (-y + item->h2()) * 2;
        item->bbox().setRect(x, y, w, h);
    }
    break;
    case BracketType::SQUARE: {
        double w = ctx.style().styleMM(Sid::staffLineWidth) * .5;
        double x = -w;
        double y = -w;
        double h = (item->h2() + w) * 2;
        w += (.5 * item->spatium() + 3 * w);
        item->bbox().setRect(x, y, w, h);
        shape.add(item->bbox());
    }
    break;
    case BracketType::LINE: {
        double spatium = item->spatium();
        double w = 0.67 * ctx.style().styleMM(Sid::bracketWidth) * 0.5;
        double x = -w;
        double bd = spatium * 0.25;
        double y = -bd;
        double h = (-y + item->h2()) * 2;
        item->bbox().setRect(x, y, w, h);
        shape.add(item->bbox());
    }
    break;
    case BracketType::NO_BRACKET:
        break;
    }

    item->setShape(shape);
}

void PaletteLayout::layout(Clef* item, const Context& ctx)
{
    int lines = 5;
    double lineDist = 1.0;
    double spatium = ctx.style().spatium();
    double yoff = 0.0;

    if (item->clefType() != ClefType::INVALID && item->clefType() != ClefType::MAX) {
        item->setSymId(ClefInfo::symId(item->clefType()));
        yoff = lineDist * (5 - ClefInfo::line(item->clefType()));
    } else {
        item->setSymId(SymId::noSym);
    }

    switch (item->clefType()) {
    case ClefType::C_19C:                                    // 19th C clef is like a G clef
        yoff = lineDist * 1.5;
        break;
    case ClefType::TAB:         // TAB clef
    case ClefType::TAB4:        // TAB clef 4 strings
    case ClefType::TAB_SERIF:   // TAB clef alternate style
    case ClefType::TAB4_SERIF:  // TAB clef alternate style
        yoff = lineDist * (lines - 1) * 0.5;
        break;
    case ClefType::PERC:        // percussion clefs
    case ClefType::PERC2:
        yoff = lineDist * (lines - 1) * 0.5;
        break;
    default:
        break;
    }

    item->setPos(0.0, yoff * spatium);

    RectF bbox = item->symBbox(item->symId());
    item->setbbox(bbox);
}

void PaletteLayout::layout(FretDiagram* item, const Context& ctx)
{
    double spatium  = item->spatium();
    item->setStringLw(spatium * 0.08);
    item->setNutLw((item->fretOffset() || !item->showNut()) ? item->stringLw() : spatium * 0.2);
    item->setStringDist(ctx.style().styleMM(Sid::fretStringSpacing));
    item->setFretDist(ctx.style().styleMM(Sid::fretFretSpacing));
    item->setMarkerSize(item->stringDist() * 0.8);

    double w = item->stringDist() * (item->strings() - 1) + item->markerSize();
    double h = (item->frets() + 1) * item->fretDist() + item->markerSize();
    double y = -(item->markerSize() * 0.5 + item->fretDist());
    double x = -(item->markerSize() * 0.5);

    // Allocate space for fret offset number
    if (item->fretOffset() > 0) {
        mu::draw::Font scaledFont(item->font());
        scaledFont.setPointSizeF(item->font().pointSizeF() * item->userMag());

        double fretNumMag = ctx.style().styleD(Sid::fretNumMag);
        scaledFont.setPointSizeF(scaledFont.pointSizeF() * fretNumMag);
        mu::draw::FontMetrics fm2(scaledFont);
        double numw = fm2.width(String::number(item->fretOffset() + 1));
        double xdiff = numw + item->stringDist() * .4;
        w += xdiff;
        x += (item->numPos() == 0) == (item->orientation() == engraving::Orientation::VERTICAL) ? -xdiff : 0;
    }

    if (item->orientation() == engraving::Orientation::HORIZONTAL) {
        double tempW = w;
        double tempX = x;
        w = h;
        h = tempW;
        x = y;
        y = tempX;
    }

    item->bbox().setRect(x, y, w, h);
}

void PaletteLayout::layout(KeySig* item, const Context& ctx)
{
    double spatium = item->spatium();
    double step = spatium * 0.5;

    item->setbbox(RectF());
    item->keySymbols().clear();

    Key key = item->key();
    const signed char* lines = ClefInfo::lines(ClefType::G);

    if (std::abs(int(key)) <= 7) {
        SymId sym = key > 0 ? SymId::accidentalSharp : SymId::accidentalFlat;
        double accidentalGap = ctx.style().styleS(Sid::keysigAccidentalDistance).val();
        double previousWidth = item->symWidth(sym) / spatium;
        int lineIndexOffset = int(key) > 0 ? 0 : 7;
        for (int i = 0; i < std::abs(int(key)); ++i) {
            int line = lines[lineIndexOffset + i];
            KeySym ks;
            ks.sym = sym;
            double x = 0.0;
            if (item->keySymbols().size() > 0) {
                const KeySym& previous = item->keySymbols().back();
                x = previous.xPos + previousWidth + accidentalGap;
                bool isAscending = line < previous.line;
                SmuflAnchorId currentCutout = isAscending ? SmuflAnchorId::cutOutSW : SmuflAnchorId::cutOutNW;
                SmuflAnchorId previousCutout = isAscending ? SmuflAnchorId::cutOutNE : SmuflAnchorId::cutOutSE;
                PointF cutout = item->symSmuflAnchor(sym, currentCutout);
                double currentCutoutY = line * step + cutout.y();
                double previousCutoutY = previous.line * step + item->symSmuflAnchor(previous.sym, previousCutout).y();
                if ((isAscending && currentCutoutY < previousCutoutY) || (!isAscending && currentCutoutY > previousCutoutY)) {
                    x -= cutout.x() / spatium;
                }
            }
            ks.xPos = x;
            ks.line = line;
            item->keySymbols().push_back(ks);
        }
    } else {
        LOGD() << "illegal key:" << int(key);
    }

    // compute bbox
    for (const KeySym& ks : item->keySymbols()) {
        double x = ks.xPos * spatium;
        double y = ks.line * step;
        item->addbbox(item->symBbox(ks.sym).translated(x, y));
    }
}

void PaletteLayout::layout(Symbol* item, const Context&)
{
    item->setbbox(item->scoreFont() ? item->scoreFont()->bbox(item->sym(), item->magS()) : item->symBbox(item->sym()));
    item->setOffset(0.0, 0.0);
    item->setPos(0.0, 0.0);
}

void PaletteLayout::layout(TimeSig* item, const Context& ctx)
{
    item->setPos(0.0, 0.0);
    double spatium = item->spatium();

    item->setbbox(RectF());                    // prepare for an empty time signature

    TimeSig::DrawArgs drawArgs;

    double lineDist = 1.0;
    int numOfLines = 5;
    TimeSigType sigType = item->timeSigType();

    // if some symbol
    // compute vert. displacement to center in the staff height
    // determine middle staff position:

    double yoff = spatium * (numOfLines - 1) * .5 * lineDist;

    // C and Ccut are placed at the middle of the staff: use yoff directly
    IEngravingFontPtr font = ctx.engravingFont();
    SizeF mag(item->magS() * item->scale());

    if (sigType == TimeSigType::FOUR_FOUR) {
        drawArgs.pz = PointF(0.0, yoff);
        RectF bbox = font->bbox(SymId::timeSigCommon, mag);
        item->setbbox(bbox.translated(drawArgs.pz));
        drawArgs.ns.clear();
        drawArgs.ns.push_back(SymId::timeSigCommon);
        drawArgs.ds.clear();
    } else if (sigType == TimeSigType::ALLA_BREVE) {
        drawArgs.pz = PointF(0.0, yoff);
        RectF bbox = font->bbox(SymId::timeSigCutCommon, mag);
        item->setbbox(bbox.translated(drawArgs.pz));
        drawArgs.ns.clear();
        drawArgs.ns.push_back(SymId::timeSigCutCommon);
        drawArgs.ds.clear();
    } else if (sigType == TimeSigType::CUT_BACH) {
        drawArgs.pz = PointF(0.0, yoff);
        RectF bbox = font->bbox(SymId::timeSigCut2, mag);
        item->setbbox(bbox.translated(drawArgs.pz));
        drawArgs.ns.clear();
        drawArgs.ns.push_back(SymId::timeSigCut2);
        drawArgs.ds.clear();
    } else if (sigType == TimeSigType::CUT_TRIPLE) {
        drawArgs.pz = PointF(0.0, yoff);
        RectF bbox = font->bbox(SymId::timeSigCut3, mag);
        item->setbbox(bbox.translated(drawArgs.pz));
        drawArgs.ns.clear();
        drawArgs.ns.push_back(SymId::timeSigCut3);
        drawArgs.ds.clear();
    } else {
        if (item->numeratorString().isEmpty()) {
            drawArgs.ns = timeSigSymIdsFromString(item->numeratorString().isEmpty()
                                                  ? String::number(item->sig().numerator())
                                                  : item->numeratorString());

            drawArgs.ds = timeSigSymIdsFromString(item->denominatorString().isEmpty()
                                                  ? String::number(item->sig().denominator())
                                                  : item->denominatorString());
        } else {
            drawArgs.ns = timeSigSymIdsFromString(item->numeratorString());
            drawArgs.ds = timeSigSymIdsFromString(item->denominatorString());
        }

        RectF numRect = font->bbox(drawArgs.ns, mag);
        RectF denRect = font->bbox(drawArgs.ds, mag);

        // position numerator and denominator; vertical displacement:
        // number of lines is odd: 0.0 (strings are directly above and below the middle line)
        // number of lines even:   0.05 (strings are moved up/down to leave 1/10sp between them)

        double displ = (numOfLines & 1) ? 0.0 : (0.05 * spatium);

        //align on the wider
        double pzY = yoff - (denRect.width() < 0.01 ? 0.0 : (displ + numRect.height() * .5));
        double pnY = yoff + displ + denRect.height() * .5;

        if (numRect.width() >= denRect.width()) {
            // numerator: one space above centre line, unless denomin. is empty (if so, directly centre in the middle)
            drawArgs.pz = PointF(0.0, pzY);
            // denominator: horiz: centred around centre of numerator | vert: one space below centre line
            drawArgs.pn = PointF((numRect.width() - denRect.width()) * .5, pnY);
        } else {
            // numerator: one space above centre line, unless denomin. is empty (if so, directly centre in the middle)
            drawArgs.pz = PointF((denRect.width() - numRect.width()) * .5, pzY);
            // denominator: horiz: centred around centre of numerator | vert: one space below centre line
            drawArgs.pn = PointF(0.0, pnY);
        }

        // centering of parenthesis so the middle of the parenthesis is at the divisor marking level
        int centerY = yoff / 2 + spatium;
        int widestPortion = numRect.width() > denRect.width() ? numRect.width() : denRect.width();
        drawArgs.pointLargeLeftParen = PointF(-spatium, centerY);
        drawArgs.pointLargeRightParen = PointF(widestPortion + spatium, centerY);

        item->setbbox(numRect.translated(drawArgs.pz));       // translate bounding boxes to actual string positions
        item->addbbox(denRect.translated(drawArgs.pn));
        if (item->largeParentheses()) {
            item->addbbox(RectF(drawArgs.pointLargeLeftParen.x(), drawArgs.pointLargeLeftParen.y() - denRect.height(), spatium / 2,
                                numRect.height() + denRect.height()));
            item->addbbox(RectF(drawArgs.pointLargeRightParen.x(), drawArgs.pointLargeRightParen.y() - denRect.height(),  spatium / 2,
                                numRect.height() + denRect.height()));
        }
    }

    item->setDrawArgs(drawArgs);
}

void PaletteLayout::layout(Volta*, const Context&)
{
    // layoutLine(item, ctx);
}
