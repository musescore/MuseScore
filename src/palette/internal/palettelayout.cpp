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
#include "engraving/types/symnames.h"

#include "engraving/libmscore/engravingitem.h"
#include "engraving/libmscore/score.h"

#include "engraving/libmscore/accidental.h"
#include "engraving/libmscore/actionicon.h"
#include "engraving/libmscore/ambitus.h"
#include "engraving/libmscore/articulation.h"
#include "engraving/libmscore/bagpembell.h"
#include "engraving/libmscore/barline.h"
#include "engraving/libmscore/bracket.h"
#include "engraving/libmscore/capo.h"
#include "engraving/libmscore/clef.h"
#include "engraving/libmscore/dynamic.h"
#include "engraving/libmscore/expression.h"
#include "engraving/libmscore/fingering.h"
#include "engraving/libmscore/fret.h"
#include "engraving/libmscore/harppedaldiagram.h"
#include "engraving/libmscore/instrchange.h"
#include "engraving/libmscore/jump.h"
#include "engraving/libmscore/keysig.h"
#include "engraving/libmscore/letring.h"
#include "engraving/libmscore/line.h"
#include "engraving/libmscore/marker.h"
#include "engraving/libmscore/ottava.h"
#include "engraving/libmscore/palmmute.h"
#include "engraving/libmscore/pedal.h"
#include "engraving/libmscore/playtechannotation.h"
#include "engraving/libmscore/rehearsalmark.h"
#include "engraving/libmscore/stafftext.h"
#include "engraving/libmscore/symbol.h"
#include "engraving/libmscore/systemtext.h"
#include "engraving/libmscore/tempotext.h"
#include "engraving/libmscore/text.h"
#include "engraving/libmscore/textlinebase.h"
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
    case ElementType::CAPO:         layout(toCapo(item), ctx);
        break;
    case ElementType::CLEF:         layout(toClef(item), ctx);
        break;
    case ElementType::DYNAMIC:      layout(toDynamic(item), ctx);
        break;
    case ElementType::EXPRESSION:   layout(toExpression(item), ctx);
        break;
    case ElementType::FINGERING:    layout(toFingering(item), ctx);
        break;
    case ElementType::FRET_DIAGRAM: layout(toFretDiagram(item), ctx);
        break;
    case ElementType::HARP_DIAGRAM: layout(toHarpPedalDiagram(item), ctx);
        break;
    case ElementType::INSTRUMENT_CHANGE: layout(toInstrumentChange(item), ctx);
        break;
    case ElementType::JUMP:         layout(toJump(item), ctx);
        break;
    case ElementType::KEYSIG:       layout(toKeySig(item), ctx);
        break;
    case ElementType::LET_RING:     layout(toLetRing(item), ctx);
        break;
    case ElementType::MARKER:       layout(toMarker(item), ctx);
        break;
    case ElementType::OTTAVA:       layout(toOttava(item), ctx);
        break;
    case ElementType::PALM_MUTE:    layout(toPalmMute(item), ctx);
        break;
    case ElementType::PEDAL:        layout(toPedal(item), ctx);
        break;
    case ElementType::PLAYTECH_ANNOTATION: layout(toPlayTechAnnotation(item), ctx);
        break;
    case ElementType::REHEARSAL_MARK: layout(toRehearsalMark(item), ctx);
        break;
    case ElementType::STAFF_TEXT:   layout(toStaffText(item), ctx);
        break;
    case ElementType::SYMBOL:       layout(toSymbol(item), ctx);
        break;
    case ElementType::SYSTEM_TEXT:  layout(toSystemText(item), ctx);
        break;
    case ElementType::TEMPO_TEXT:   layout(toTempoText(item), ctx);
        break;
    case ElementType::TIMESIG:      layout(toTimeSig(item), ctx);
        break;
    default:
        //! TODO Still need
        //LOGD() << item->typeName();
        layout::pal::TLayout::layoutItem(item, ctxpal);
        break;
    }
}

void PaletteLayout::layoutLineSegment(LineSegment* item, const Context& ctx)
{
    switch (item->type()) {
    case ElementType::LET_RING_SEGMENT:  layout(toLetRingSegment(item), ctx);
        break;
    case ElementType::OTTAVA_SEGMENT:    layout(toOttavaSegment(item), ctx);
        break;
    case ElementType::PALM_MUTE_SEGMENT: layout(toPalmMuteSegment(item), ctx);
        break;
    case ElementType::PEDAL_SEGMENT:     layout(toPedalSegment(item), ctx);
        break;
    default:
        UNREACHABLE;
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

    double lineDist = spatium;
    constexpr int numOfLines = 3;

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
        w += (0.5 * item->spatium() + 3 * w);
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

void PaletteLayout::layout(Capo* item, const Context& ctx)
{
    layoutTextBase(item, ctx);
}

void PaletteLayout::layout(Clef* item, const Context& ctx)
{
    constexpr int lines = 5;
    constexpr double lineDist = 1.0;
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

void PaletteLayout::layout(Expression* item, const Context& ctx)
{
    layoutTextBase(item, ctx);
}

void PaletteLayout::layout(Fingering* item, const Context& ctx)
{
    layoutTextBase(item, ctx);
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
        std::swap(w, h);
        std::swap(x, y);
    }

    item->bbox().setRect(x, y, w, h);
}

void PaletteLayout::layout(Dynamic* item, const Context& ctx)
{
    layoutTextBase(item, ctx);
}

void PaletteLayout::layout(HarpPedalDiagram* item, const Context& ctx)
{
    item->updateDiagramText();
    layoutTextBase(item, ctx);
}

void PaletteLayout::layout(InstrumentChange* item, const Context& ctx)
{
    layoutTextBase(item, ctx);
}

void PaletteLayout::layout(Jump* item, const Context& ctx)
{
    layoutTextBase(item, ctx);
}

void PaletteLayout::layout(KeySig* item, const Context& ctx)
{
    double spatium = item->spatium();
    double step = spatium * 0.5;

    item->setbbox(RectF());

    item->keySymbols().clear();

    // determine current clef for this staff
    ClefType clef = ClefType::G;

    int key = int(item->key());

    if (item->isCustom() && !item->isAtonal()) {
        double accidentalGap = ctx.style().styleS(Sid::keysigAccidentalDistance).val();
        // add standard key accidentals first, if necessary
        for (int i = 1; i <= abs(key) && abs(key) <= 7; ++i) {
            bool drop = false;
            for (const CustDef& cd: item->customKeyDefs()) {
                int degree = item->degInKey(cd.degree);
                // if custom keysig accidental takes place, don't create tonal accidental
                if ((degree * 2 + 2) % 7 == (key < 0 ? 8 - i : i) % 7) {
                    drop = true;
                    break;
                }
            }
            if (!drop) {
                KeySym ks;
                int lineIndexOffset = key > 0 ? -1 : 6;
                ks.sym = key > 0 ? SymId::accidentalSharp : SymId::accidentalFlat;
                ks.line = ClefInfo::lines(clef)[lineIndexOffset + i];
                if (item->keySymbols().size() > 0) {
                    KeySym& previous = item->keySymbols().back();
                    double previousWidth = item->symWidth(previous.sym) / spatium;
                    ks.xPos = previous.xPos + previousWidth + accidentalGap;
                } else {
                    ks.xPos = 0;
                }
                // TODO octave metters?
                item->keySymbols().push_back(ks);
            }
        }
        for (const CustDef& cd : item->customKeyDefs()) {
            SymId sym = item->symInKey(cd.sym, cd.degree);
            int degree = item->degInKey(cd.degree);
            bool flat = std::string(SymNames::nameForSymId(sym).ascii()).find("Flat") != std::string::npos;
            int accIdx = (degree * 2 + 1) % 7; // C D E F ... index to F C G D index
            accIdx = flat ? 13 - accIdx : accIdx;
            int line = ClefInfo::lines(clef)[accIdx] + cd.octAlt * 7;
            double xpos = cd.xAlt;
            if (item->keySymbols().size() > 0) {
                KeySym& previous = item->keySymbols().back();
                double previousWidth = item->symWidth(previous.sym) / spatium;
                xpos += previous.xPos + previousWidth + accidentalGap;
            }
            // if translated symbol if out of range, add key accidental followed by untranslated symbol
            if (sym == SymId::noSym) {
                KeySym ks;
                ks.line = line;
                ks.xPos = xpos;
                // for quadruple sharp use two double sharps
                if (cd.sym == SymId::accidentalTripleSharp) {
                    ks.sym = SymId::accidentalDoubleSharp;
                    sym = SymId::accidentalDoubleSharp;
                } else {
                    ks.sym = key > 0 ? SymId::accidentalSharp : SymId::accidentalFlat;
                    sym = cd.sym;
                }
                item->keySymbols().push_back(ks);
                xpos += key < 0 ? 0.7 : 1; // flats closer
            }
            // create symbol; natural only if is user defined
            if (sym != SymId::accidentalNatural || sym == cd.sym) {
                KeySym ks;
                ks.sym = sym;
                ks.line = line;
                ks.xPos = xpos;
                item->keySymbols().push_back(ks);
            }
        }
    } else {
        if (std::abs(key) <= 7) {
            const signed char* lines = ClefInfo::lines(clef);
            SymId sym = key > 0 ? SymId::accidentalSharp : SymId::accidentalFlat;
            double accidentalGap = ctx.style().styleS(Sid::keysigAccidentalDistance).val();
            double previousWidth = item->symWidth(sym) / spatium;
            int lineIndexOffset = key > 0 ? 0 : 7;
            for (int i = 0; i < std::abs(key); ++i) {
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
            LOGD() << "illegal key:" << key;
        }
    }

    // compute bbox
    for (const KeySym& ks : item->keySymbols()) {
        double x = ks.xPos * spatium;
        double y = ks.line * step;
        item->addbbox(item->symBbox(ks.sym).translated(x, y));
    }
}

void PaletteLayout::layout(LetRing* item, const Context& ctx)
{
    layoutLine(item, ctx);
}

void PaletteLayout::layout(LetRingSegment* item, const Context& ctx)
{
    layoutTextLineBaseSegment(item, ctx);
}

void PaletteLayout::layout(Marker* item, const Context& ctx)
{
    layoutTextBase(item, ctx);
}

void PaletteLayout::layout(Ottava* item, const Context& ctx)
{
    layoutLine(item, ctx);
}

void PaletteLayout::layout(OttavaSegment* item, const Context& ctx)
{
    layoutTextLineBaseSegment(item, ctx);
}

void PaletteLayout::layout(PalmMute* item, const Context& ctx)
{
    layoutLine(item, ctx);
}

void PaletteLayout::layout(PalmMuteSegment* item, const Context& ctx)
{
    layoutTextLineBaseSegment(item, ctx);
}

void PaletteLayout::layout(Pedal* item, const Context& ctx)
{
    layoutLine(item, ctx);
}

void PaletteLayout::layout(PedalSegment* item, const Context& ctx)
{
    layoutTextLineBaseSegment(item, ctx);
    item->setOffset(PointF());
}

void PaletteLayout::layout(PlayTechAnnotation* item, const Context& ctx)
{
    layoutTextBase(item, ctx);
}

void PaletteLayout::layout(RehearsalMark* item, const Context& ctx)
{
    layoutTextBase(item, ctx);
}

void PaletteLayout::layout(StaffText* item, const Context& ctx)
{
    layoutTextBase(item, ctx);
}

void PaletteLayout::layout(Symbol* item, const Context&)
{
    item->setbbox(item->scoreFont() ? item->scoreFont()->bbox(item->sym(), item->magS()) : item->symBbox(item->sym()));
    item->setOffset(0.0, 0.0);
    item->setPos(0.0, 0.0);
}

void PaletteLayout::layout(SystemText* item, const Context& ctx)
{
    layoutTextBase(item, ctx);
}

void PaletteLayout::layout(TempoText* item, const Context& ctx)
{
    layoutTextBase(item, ctx);
}

void PaletteLayout::layout(TimeSig* item, const Context& ctx)
{
    item->setPos(0.0, 0.0);
    double spatium = item->spatium();

    item->setbbox(RectF());                    // prepare for an empty time signature

    TimeSig::DrawArgs drawArgs;

    constexpr double lineDist = 1.0;
    constexpr int numOfLines = 5;
    TimeSigType sigType = item->timeSigType();

    // if some symbol
    // compute vert. displacement to center in the staff height
    // determine middle staff position:

    double yoff = spatium * (numOfLines - 1) * 0.5 * lineDist;

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
        double pzY = yoff - (denRect.width() < 0.01 ? 0.0 : (displ + numRect.height() * 0.5));
        double pnY = yoff + displ + denRect.height() * 0.5;

        if (numRect.width() >= denRect.width()) {
            // numerator: one space above centre line, unless denomin. is empty (if so, directly centre in the middle)
            drawArgs.pz = PointF(0.0, pzY);
            // denominator: horiz: centred around centre of numerator | vert: one space below centre line
            drawArgs.pn = PointF((numRect.width() - denRect.width()) * 0.5, pnY);
        } else {
            // numerator: one space above centre line, unless denomin. is empty (if so, directly centre in the middle)
            drawArgs.pz = PointF((denRect.width() - numRect.width()) * 0.5, pzY);
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

//! TODO
void PaletteLayout::layout(Volta*, const Context&)
{
    // layoutLine(item, ctx);
}

void PaletteLayout::layout(Text* item, const Context& ctx)
{
    layoutTextBase(static_cast<TextBase*>(item), ctx);
}

void PaletteLayout::layoutTextBase(TextBase* item, const Context& ctx)
{
    item->setPos(PointF());
    item->setOffset(0.0, 0.0);

    if (item->placeBelow()) {
        item->setPosY(0.0);
    }

    layout1TextBase(item, ctx);
}

void PaletteLayout::layout1TextBase(TextBase* item, const Context&)
{
    if (item->isBlockNotCreated()) {
        item->createBlocks();
    }
    if (item->blocksRef().empty()) {
        item->blocksRef().push_back(TextBlock());
    }

    RectF bb;
    double y = 0;

    // adjust the bounding box for the text item
    for (size_t i = 0; i < item->rows(); ++i) {
        TextBlock* t = &item->blocksRef()[i];
        t->layout(item);
        const RectF* r = &t->boundingRect();

        if (r->height() == 0) {
            r = &item->blocksRef()[i - i].boundingRect();
        }
        y += t->lineSpacing();
        t->setY(y);
        bb |= r->translated(0.0, y);
    }
    double yoff = 0;
    double h    = 0;

    item->setPos(PointF());

    if (item->align() == AlignV::BOTTOM) {
        yoff += h - bb.bottom();
    } else if (item->align() == AlignV::VCENTER) {
        yoff +=  (h - (bb.top() + bb.bottom())) * .5;
    } else if (item->align() == AlignV::BASELINE) {
        yoff += h * .5 - item->blocksRef().front().lineSpacing();
    } else {
        yoff += -bb.top();
    }

    for (TextBlock& t : item->blocksRef()) {
        t.setY(t.y() + yoff);
    }

    bb.translate(0.0, yoff);

    item->setbbox(bb);
    if (item->hasFrame()) {
        item->layoutFrame();
    }
}

void PaletteLayout::layoutLine(SLine* item, const Context& ctx)
{
    if (item->spannerSegments().empty()) {
        item->setLen(ctx.style().spatium() * 7);
    }

    LineSegment* lineSegm = item->frontSegment();
    layoutLineSegment(lineSegm, ctx);
    item->setbbox(lineSegm->bbox());
}

void PaletteLayout::layoutTextLineBaseSegment(TextLineBaseSegment* item, const Context& ctx)
{
    item->npointsRef() = 0;
    TextLineBase* tl = item->textLineBase();
    double spatium = tl->spatium();

    if (item->spanner()->placeBelow()) {
        item->setPosY(0.0);
    }

    if (!tl->diagonal()) {
        item->setUserYoffset2(0);
    }

    auto alignBaseLine = [tl](Text* text, PointF& pp1, PointF& pp2) {
        PointF widthCorrection(0.0, tl->lineWidth() / 2);
        switch (text->align().vertical) {
        case AlignV::TOP:
            pp1 += widthCorrection;
            pp2 += widthCorrection;
            break;
        case AlignV::VCENTER:
            break;
        case AlignV::BOTTOM:
            pp1 -= widthCorrection;
            pp2 -= widthCorrection;
            break;
        case AlignV::BASELINE:
            pp1 -= widthCorrection;
            pp2 -= widthCorrection;
            break;
        }
    };

    switch (item->spannerSegmentType()) {
    case SpannerSegmentType::SINGLE:
    case SpannerSegmentType::BEGIN:
        item->text()->setXmlText(tl->beginText());
        item->text()->setFamily(tl->beginFontFamily());
        item->text()->setSize(tl->beginFontSize());
        item->text()->setOffset(tl->beginTextOffset() * item->mag());
        item->text()->setAlign(tl->beginTextAlign());
        item->text()->setFontStyle(tl->beginFontStyle());
        break;
    case SpannerSegmentType::MIDDLE:
    case SpannerSegmentType::END:
        item->text()->setXmlText(tl->continueText());
        item->text()->setFamily(tl->continueFontFamily());
        item->text()->setSize(tl->continueFontSize());
        item->text()->setOffset(tl->continueTextOffset() * item->mag());
        item->text()->setAlign(tl->continueTextAlign());
        item->text()->setFontStyle(tl->continueFontStyle());
        break;
    }
    item->text()->setPlacement(PlacementV::ABOVE);

    layout(item->text(), ctx);

    if ((item->isSingleType() || item->isEndType())) {
        item->endText()->setXmlText(tl->endText());
        item->endText()->setFamily(tl->endFontFamily());
        item->endText()->setSize(tl->endFontSize());
        item->endText()->setOffset(tl->endTextOffset());
        item->endText()->setAlign(tl->endTextAlign());
        item->endText()->setFontStyle(tl->endFontStyle());
        item->endText()->setPlacement(PlacementV::ABOVE);
        item->endText()->setTrack(item->track());
        layout(item->endText(), ctx);
    } else {
        item->endText()->setXmlText(u"");
    }

    if (!item->textLineBase()->textSizeSpatiumDependent()) {
        item->text()->setSize(item->text()->size() * SPATIUM20 / item->spatium());
        item->endText()->setSize(item->endText()->size() * SPATIUM20 / item->spatium());
    }

    PointF pp1;
    PointF pp2(item->pos2());

    // line with no text or hooks - just use the basic rectangle for line
    if (item->text()->empty() && item->endText()->empty()
        && (!item->isSingleBeginType() || tl->beginHookType() == HookType::NONE)
        && (!item->isSingleEndType() || tl->endHookType() == HookType::NONE)) {
        item->npointsRef() = 2;
        item->pointsRef()[0] = pp1;
        item->pointsRef()[1] = pp2;
        item->setLineLength(sqrt(PointF::dotProduct(pp2 - pp1, pp2 - pp1)));

        item->setbbox(TextLineBaseSegment::boundingBoxOfLine(pp1, pp2, tl->lineWidth() / 2, tl->lineStyle() == LineType::DOTTED));
        return;
    }

    // line has text or hooks or is not diagonal - calculate reasonable bbox

    double x1 = std::min(0.0, pp2.x());
    double x2 = std::max(0.0, pp2.x());
    double y0 = -tl->lineWidth();
    double y1 = std::min(0.0, pp2.y()) + y0;
    double y2 = std::max(0.0, pp2.y()) - y0;

    double l = 0.0;
    if (!item->text()->empty()) {
        double gapBetweenTextAndLine = spatium * tl->gapBetweenTextAndLine().val();
        if ((item->isSingleBeginType() && (tl->beginTextPlace() == TextPlace::LEFT || tl->beginTextPlace() == TextPlace::AUTO))
            || (!item->isSingleBeginType() && (tl->continueTextPlace() == TextPlace::LEFT || tl->continueTextPlace() == TextPlace::AUTO))) {
            l = item->text()->pos().x() + item->text()->bbox().width() + gapBetweenTextAndLine;
        }

        double h = item->text()->height();
        if (tl->beginTextPlace() == TextPlace::ABOVE) {
            y1 = std::min(y1, -h);
        } else if (tl->beginTextPlace() == TextPlace::BELOW) {
            y2 = std::max(y2, h);
        } else {
            y1 = std::min(y1, -h * .5);
            y2 = std::max(y2, h * .5);
        }
        x2 = std::max(x2, item->text()->width());
    }

    if (tl->endHookType() != HookType::NONE) {
        double h = pp2.y() + tl->endHookHeight().val() * spatium;
        if (h > y2) {
            y2 = h;
        } else if (h < y1) {
            y1 = h;
        }
    }

    if (tl->beginHookType() != HookType::NONE) {
        double h = tl->beginHookHeight().val() * spatium;
        if (h > y2) {
            y2 = h;
        } else if (h < y1) {
            y1 = h;
        }
    }
    item->bbox().setRect(x1, y1, x2 - x1, y2 - y1);
    if (!item->text()->empty()) {
        item->bbox() |= item->text()->bbox().translated(item->text()->pos());      // DEBUG
    }
    // set end text position and extend bbox
    if (!item->endText()->empty()) {
        item->endText()->movePosX(item->bbox().right());
        item->bbox() |= item->endText()->bbox().translated(item->endText()->pos());
    }

    if (tl->lineVisible()) {
        // Extends lines to fill the corner between them.
        // Assumes that l1p2 == l2p1 is the intersection between the lines.
        // If checkAngle is false, assumes that the lines are perpendicular,
        // and some calculations are saved.
        auto extendLines = [](const PointF& l1p1, PointF& l1p2, PointF& l2p1, const PointF& l2p2, double lineWidth, bool checkAngle)
        {
            PointF l1UnitVector = (l1p2 - l1p1).normalized();
            PointF l2UnitVector = (l2p1 - l2p2).normalized();

            double addedLength = lineWidth * 0.5;

            if (checkAngle) {
                double angle = M_PI - acos(PointF::dotProduct(l1UnitVector, l2UnitVector));

                if (angle <= M_PI_2) {
                    addedLength *= tan(0.5 * angle);
                }
            }

            l1p2 += l1UnitVector * addedLength;
            l2p1 += l2UnitVector * addedLength;
        };

        pp1 = PointF(l, 0.0);

        // Make sure baseline of text and line are properly aligned (accounting for line thickness)
        bool alignBeginText = tl->beginTextPlace() == TextPlace::LEFT || tl->beginTextPlace() == TextPlace::AUTO;
        bool alignContinueText = tl->continueTextPlace() == TextPlace::LEFT || tl->continueTextPlace() == TextPlace::AUTO;
        bool alignEndText = tl->endTextPlace() == TextPlace::LEFT || tl->endTextPlace() == TextPlace::AUTO;
        bool isSingleOrBegin = item->isSingleBeginType();
        bool hasBeginText = !item->text()->empty() && isSingleOrBegin;
        bool hasContinueText = !item->text()->empty() && !isSingleOrBegin;
        bool hasEndText = !item->endText()->empty() && item->isSingleEndType();
        if ((hasBeginText && alignBeginText) || (hasContinueText && alignContinueText)) {
            alignBaseLine(item->text(), pp1, pp2);
        } else if (hasEndText && alignEndText) {
            alignBaseLine(item->endText(), pp1, pp2);
        }

        double beginHookHeight = tl->beginHookHeight().val() * spatium;
        double endHookHeight = tl->endHookHeight().val() * spatium;
        double beginHookWidth = 0.0;
        double endHookWidth = 0.0;

        if (tl->beginHookType() == HookType::HOOK_45) {
            beginHookWidth = fabs(beginHookHeight * .4);
            pp1.rx() += beginHookWidth;
        }

        if (tl->endHookType() == HookType::HOOK_45) {
            endHookWidth = fabs(endHookHeight * .4);
            pp2.rx() -= endHookWidth;
        }

        // don't draw backwards lines (or hooks) if text is longer than nominal line length
        if (!item->text()->empty() && pp1.x() > pp2.x() && !tl->diagonal()) {
            return;
        }

        if (item->isSingleBeginType() && tl->beginHookType() != HookType::NONE) {
            // We use the term "endpoint" for the point that does not touch the main line.
            const PointF& beginHookEndpoint = item->pointsRef()[item->npointsRef()++]
                                                  = PointF(pp1.x() - beginHookWidth, pp1.y() + beginHookHeight);

            if (tl->beginHookType() == HookType::HOOK_90T) {
                // A T-hook needs to be drawn separately, so we add an extra point
                item->pointsRef()[item->npointsRef()++] = PointF(pp1.x() - beginHookWidth, pp1.y() - beginHookHeight);
            } else if (tl->lineStyle() != LineType::SOLID) {
                // For non-solid lines, we also draw the hook separately,
                // so that we can distribute the dashes/dots for each linepiece individually
                PointF& beginHookStartpoint = item->pointsRef()[item->npointsRef()++] = pp1;

                if (tl->lineStyle() == LineType::DASHED) {
                    // For dashes lines, we extend the lines somewhat,
                    // so that the corner between them gets filled
                    bool checkAngle = tl->beginHookType() == HookType::HOOK_45 || tl->diagonal();
                    extendLines(beginHookEndpoint, beginHookStartpoint, pp1, pp2, tl->lineWidth() * item->mag(), checkAngle);
                }
            }
        }

        item->pointsRef()[item->npointsRef()++] = pp1;
        PointF& pp22 = item->pointsRef()[item->npointsRef()++] = pp2; // Keep a reference so that we can modify later

        if (item->isSingleEndType() && tl->endHookType() != HookType::NONE) {
            const PointF endHookEndpoint = PointF(pp2.x() + endHookWidth, pp2.y() + endHookHeight);

            if (tl->endHookType() == HookType::HOOK_90T) {
                // A T-hook needs to be drawn separately, so we add an extra point
                item->pointsRef()[item->npointsRef()++] = PointF(pp2.x() + endHookWidth, pp2.y() - endHookHeight);
            } else if (tl->lineStyle() != LineType::SOLID) {
                // For non-solid lines, we also draw the hook separately,
                // so that we can distribute the dashes/dots for each linepiece individually
                PointF& endHookStartpoint = item->pointsRef()[item->npointsRef()++] = pp2;

                if (tl->lineStyle() == LineType::DASHED) {
                    bool checkAngle = tl->endHookType() == HookType::HOOK_45 || tl->diagonal();

                    // For dashes lines, we extend the lines somewhat,
                    // so that the corner between them gets filled
                    extendLines(pp1, pp22, endHookStartpoint, endHookEndpoint, tl->lineWidth() * item->mag(), checkAngle);
                }
            }

            item->pointsRef()[item->npointsRef()++] = endHookEndpoint;
        }

        item->setLineLength(sqrt(PointF::dotProduct(pp22 - pp1, pp22 - pp1)));
    }
}
