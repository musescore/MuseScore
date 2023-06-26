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
#include "engraving/libmscore/articulation.h"
#include "engraving/libmscore/clef.h"
#include "engraving/libmscore/keysig.h"
#include "engraving/libmscore/timesig.h"

#include "engraving/libmscore/utils.h"

#include "engraving/layout/pal/tlayout.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::palette;

void PaletteLayout::layoutItem(EngravingItem* item)
{
    layout::pal::LayoutContext ctxpal(item->score());
    Context ctx(item->score());

    switch (item->type()) {
    case ElementType::ACCIDENTAL:   layout(toAccidental(item), ctx);
        break;
    case ElementType::ARTICULATION: layout(toArticulation(item), ctx);
        break;
    case ElementType::CLEF:         layout(toClef(item), ctx);
        break;
    case ElementType::KEYSIG:       layout(toKeySig(item), ctx);
        break;
    case ElementType::TIMESIG:      layout(toTimeSig(item), ctx);
        break;
    default:
        LOGI() << item->typeName();
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
