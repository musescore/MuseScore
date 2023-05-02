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

#include "tlayout.h"

#include "draw/fontmetrics.h"

#include "../infrastructure/rtti.h"

#include "../iengravingfont.h"
#include "../types/typesconv.h"
#include "../types/symnames.h"
#include "../libmscore/score.h"
#include "../libmscore/utils.h"

#include "../libmscore/accidental.h"
#include "../libmscore/actionicon.h"
#include "../libmscore/ambitus.h"
#include "../libmscore/arpeggio.h"
#include "../libmscore/articulation.h"

#include "../libmscore/bagpembell.h"
#include "../libmscore/barline.h"
#include "../libmscore/beam.h"
#include "../libmscore/bend.h"
#include "../libmscore/box.h"
#include "../libmscore/bracket.h"
#include "../libmscore/breath.h"

#include "../libmscore/chord.h"
#include "../libmscore/chordline.h"
#include "../libmscore/clef.h"

#include "../libmscore/deadslapped.h"
#include "../libmscore/dynamic.h"

#include "../libmscore/expression.h"

#include "../libmscore/fermata.h"

#include "../libmscore/note.h"

#include "../libmscore/part.h"

#include "../libmscore/rest.h"

#include "../libmscore/staff.h"
#include "../libmscore/stem.h"
#include "../libmscore/system.h"

#include "../libmscore/text.h"
#include "../libmscore/textframe.h"

#include "beamlayout.h"
#include "chordlayout.h"

using namespace mu::draw;
using namespace mu::engraving;
using namespace mu::engraving::v0;

void TLayout::layout(Accidental* item, LayoutContext& ctx)
{
    item->clearElements();

    // TODO: remove Accidental in layout()
    // don't show accidentals for tab or slash notation
    if (item->onTabStaff() || (item->note() && item->note()->fixed())) {
        item->setbbox(RectF());
        return;
    }

    double m = item->explicitParent() ? item->parentItem()->mag() : 1.0;
    if (item->isSmall()) {
        m *= item->score()->styleD(Sid::smallNoteMag);
    }
    item->setMag(m);

    // if the accidental is standard (doubleflat, flat, natural, sharp or double sharp)
    // and it has either no bracket or parentheses, then we have glyphs straight from smufl.
    if (item->bracket() == AccidentalBracket::NONE
        || (item->bracket() == AccidentalBracket::PARENTHESIS
            && (item->accidentalType() == AccidentalType::FLAT
                || item->accidentalType() == AccidentalType::NATURAL
                || item->accidentalType() == AccidentalType::SHARP
                || item->accidentalType() == AccidentalType::SHARP2
                || item->accidentalType() == AccidentalType::FLAT2))) {
        layoutSingleGlyphAccidental(item, ctx);
    } else {
        layoutMultiGlyphAccidental(item, ctx);
    }
}

void TLayout::layoutSingleGlyphAccidental(Accidental* item, LayoutContext& ctx)
{
    RectF r;

    SymId s = item->symbol();
    if (item->bracket() == AccidentalBracket::PARENTHESIS) {
        switch (item->accidentalType()) {
        case AccidentalType::FLAT2:
            s = SymId::accidentalDoubleFlatParens;
            break;
        case AccidentalType::FLAT:
            s = SymId::accidentalFlatParens;
            break;
        case AccidentalType::NATURAL:
            s = SymId::accidentalNaturalParens;
            break;
        case AccidentalType::SHARP:
            s = SymId::accidentalSharpParens;
            break;
        case AccidentalType::SHARP2:
            s = SymId::accidentalDoubleSharpParens;
            break;
        default:
            break;
        }
        if (!item->score()->engravingFont()->isValid(s)) {
            layoutMultiGlyphAccidental(item, ctx);
            return;
        }
    }

    SymElement e(s, 0.0, 0.0);
    item->addElement(e);
    r.unite(item->symBbox(s));
    item->setbbox(r);
}

void TLayout::layoutMultiGlyphAccidental(Accidental* item, LayoutContext&)
{
    double margin = item->score()->styleMM(Sid::bracketedAccidentalPadding);
    RectF r;
    double x = 0.0;

    // should always be true
    if (item->bracket() != AccidentalBracket::NONE) {
        SymId id = SymId::noSym;
        switch (item->bracket()) {
        case AccidentalBracket::PARENTHESIS:
            id = SymId::accidentalParensLeft;
            break;
        case AccidentalBracket::BRACKET:
            id = SymId::accidentalBracketLeft;
            break;
        case AccidentalBracket::BRACE:
            id = SymId::accidentalCombiningOpenCurlyBrace;
            break;
        case AccidentalBracket::NONE: // can't happen
            break;
        }
        SymElement se(id, 0.0, item->bracket() == AccidentalBracket::BRACE ? item->spatium() * 0.4 : 0.0);
        item->addElement(se);
        r.unite(item->symBbox(id));
        x += item->symAdvance(id) + margin;
    }

    SymId s = item->symbol();
    SymElement e(s, x, 0.0);
    item->addElement(e);
    r.unite(item->symBbox(s).translated(x, 0.0));

    // should always be true
    if (item->bracket() != AccidentalBracket::NONE) {
        x += item->symAdvance(s) + margin;
        SymId id = SymId::noSym;
        switch (item->bracket()) {
        case AccidentalBracket::PARENTHESIS:
            id = SymId::accidentalParensRight;
            break;
        case AccidentalBracket::BRACKET:
            id = SymId::accidentalBracketRight;
            break;
        case AccidentalBracket::BRACE:
            id = SymId::accidentalCombiningCloseCurlyBrace;
            break;
        case AccidentalBracket::NONE: // can't happen
            break;
        }
        SymElement se(id, x, item->bracket() == AccidentalBracket::BRACE ? item->spatium() * 0.4 : 0.0);
        item->addElement(se);
        r.unite(item->symBbox(id).translated(x, 0.0));
    }
    item->setbbox(r);
}

void TLayout::layout(ActionIcon* item, LayoutContext&)
{
    FontMetrics fontMetrics(item->iconFont());
    item->setbbox(fontMetrics.boundingRect(Char(item->icon())));
}

void TLayout::layout(Ambitus* item, LayoutContext&)
{
    int bottomLine, topLine;
    ClefType clf;
    double headWdt     = item->headWidth();
    Key key;
    double lineDist;
    int numOfLines;
    Segment* segm        = item->segment();
    double _spatium    = item->spatium();
    Staff* stf         = nullptr;
    if (segm && item->track() != mu::nidx) {
        Fraction tick    = segm->tick();
        stf         = item->score()->staff(item->staffIdx());
        lineDist    = stf->lineDistance(tick) * _spatium;
        numOfLines  = stf->lines(tick);
        clf         = stf->clef(tick);
    } else {                              // for use in palettes
        lineDist    = _spatium;
        numOfLines  = 3;
        clf         = ClefType::G;
    }

    //
    // NOTEHEADS Y POS
    //
    // if pitch == INVALID_PITCH or tpc == Tpc::TPC_INVALID, set to some default:
    // for use in palettes and when actual range cannot be calculated (new ambitus or no notes in staff)
    //
    double xAccidOffTop    = 0;
    double xAccidOffBottom = 0;
    if (stf) {
        key = stf->key(segm->tick());
    } else {
        key = Key::C;
    }

    // top notehead
    if (item->topPitch() == INVALID_PITCH || item->topTpc() == Tpc::TPC_INVALID) {
        item->setTopPosY(0.0);  // if uninitialized, set to top staff line
    } else {
        topLine  = absStep(item->topTpc(), item->topPitch());
        topLine  = relStep(topLine, clf);
        item->setTopPosY(topLine * lineDist * 0.5);
        // compute accidental
        AccidentalType accidType;
        // if (13 <= (tpc - key) <= 19) there is no accidental)
        if (item->topTpc() - int(key) >= 13 && item->topTpc() - int(key) <= 19) {
            accidType = AccidentalType::NONE;
        } else {
            AccidentalVal accidVal = tpc2alter(item->topTpc());
            accidType = Accidental::value2subtype(accidVal);
            if (accidType == AccidentalType::NONE) {
                accidType = AccidentalType::NATURAL;
            }
        }
        item->topAccidental()->setAccidentalType(accidType);
        if (accidType != AccidentalType::NONE) {
            item->topAccidental()->layout();
        } else {
            item->topAccidental()->setbbox(RectF());
        }
        item->topAccidental()->setPosY(item->topPos().y());
    }

    // bottom notehead
    if (item->bottomPitch() == INVALID_PITCH || item->bottomTpc() == Tpc::TPC_INVALID) {
        item->setBottomPosY((numOfLines - 1) * lineDist);             // if uninitialized, set to last staff line
    } else {
        bottomLine  = absStep(item->bottomTpc(), item->bottomPitch());
        bottomLine  = relStep(bottomLine, clf);
        item->setBottomPosY(bottomLine * lineDist * 0.5);
        // compute accidental
        AccidentalType accidType;
        if (item->bottomTpc() - int(key) >= 13 && item->bottomTpc() - int(key) <= 19) {
            accidType = AccidentalType::NONE;
        } else {
            AccidentalVal accidVal = tpc2alter(item->bottomTpc());
            accidType = Accidental::value2subtype(accidVal);
            if (accidType == AccidentalType::NONE) {
                accidType = AccidentalType::NATURAL;
            }
        }
        item->bottomAccidental()->setAccidentalType(accidType);
        if (accidType != AccidentalType::NONE) {
            item->bottomAccidental()->layout();
        } else {
            item->bottomAccidental()->setbbox(RectF());
        }
        item->bottomAccidental()->setPosY(item->bottomPos().y());
    }

    //
    // NOTEHEAD X POS
    //
    // Note: manages colliding accidentals
    //
    double accNoteDist = item->point(item->score()->styleS(Sid::accidentalNoteDistance));
    xAccidOffTop      = item->topAccidental()->width() + accNoteDist;
    xAccidOffBottom   = item->bottomAccidental()->width() + accNoteDist;

    // if top accidental extends down more than bottom accidental extends up,
    // AND ambitus is not leaning right, bottom accidental needs to be displaced
    bool collision
        =(item->topAccidental()->ipos().y() + item->topAccidental()->bbox().y() + item->topAccidental()->height()
          > item->bottomAccidental()->ipos().y() + item->bottomAccidental()->bbox().y())
          && item->direction() != DirectionH::RIGHT;
    if (collision) {
        // displace bottom accidental (also attempting to 'undercut' flats)
        xAccidOffBottom = xAccidOffTop
                          + ((item->bottomAccidental()->accidentalType() == AccidentalType::FLAT
                              || item->bottomAccidental()->accidentalType() == AccidentalType::FLAT2
                              || item->bottomAccidental()->accidentalType() == AccidentalType::NATURAL)
                             ? item->bottomAccidental()->width() * 0.5 : item->bottomAccidental()->width());
    }

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
        item->bottomAccidental()->setPosX(collision ? -xAccidOffBottom : headWdt - xAccidOffBottom);
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
        double off = _spatium * Ambitus::LINEOFFSET_DEFAULT;
        PointF p1 = fullLine.pointAt(off / yDelta);
        PointF p2 = fullLine.pointAt(1 - (off / yDelta));
        item->setLine(LineF(p1, p2));
    } else {
        item->setLine(fullLine);
    }

    RectF headRect(0, -0.5 * _spatium, headWdt, 1 * _spatium);
    item->setbbox(headRect.translated(item->topPos()).united(headRect.translated(item->bottomPos()))
                  .united(item->topAccidental()->bbox().translated(item->topAccidental()->ipos()))
                  .united(item->bottomAccidental()->bbox().translated(item->bottomAccidental()->ipos()))
                  );
}

void TLayout::layout(Arpeggio* item, LayoutContext&)
{
    double top = item->calcTop();
    double bottom = item->calcBottom();
    if (item->score()->styleB(Sid::ArpeggioHiddenInStdIfTab)) {
        if (item->staff() && item->staff()->isPitchedStaff(item->tick())) {
            for (Staff* s : item->staff()->staffList()) {
                if (s->score() == item->score() && s->isTabStaff(item->tick()) && s->visible()) {
                    item->setbbox(RectF());
                    return;
                }
            }
        }
    }
    if (item->staff()) {
        item->setMag(item->staff()->staffMag(item->tick()));
    }
    switch (item->arpeggioType()) {
    case ArpeggioType::NORMAL: {
        item->symbolLine(SymId::wiggleArpeggiatoUp, SymId::wiggleArpeggiatoUp);
        // string is rotated -90 degrees
        RectF r(item->symBbox(item->symbols()));
        item->setbbox(RectF(0.0, -r.x() + top, r.height(), r.width()));
    }
    break;

    case ArpeggioType::UP: {
        item->symbolLine(SymId::wiggleArpeggiatoUpArrow, SymId::wiggleArpeggiatoUp);
        // string is rotated -90 degrees
        RectF r(item->symBbox(item->symbols()));
        item->setbbox(RectF(0.0, -r.x() + top, r.height(), r.width()));
    }
    break;

    case ArpeggioType::DOWN: {
        item->symbolLine(SymId::wiggleArpeggiatoUpArrow, SymId::wiggleArpeggiatoUp);
        // string is rotated +90 degrees (so that UpArrow turns into a DownArrow)
        RectF r(item->symBbox(item->symbols()));
        item->setbbox(RectF(0.0, r.x() + top, r.height(), r.width()));
    }
    break;

    case ArpeggioType::UP_STRAIGHT: {
        double _spatium = item->spatium();
        double x1 = _spatium * .5;
        double w  = item->symBbox(SymId::arrowheadBlackUp).width();
        item->setbbox(RectF(x1 - w * .5, top, w, bottom));
    }
    break;

    case ArpeggioType::DOWN_STRAIGHT: {
        double _spatium = item->spatium();
        double x1 = _spatium * .5;
        double w  = item->symBbox(SymId::arrowheadBlackDown).width();
        item->setbbox(RectF(x1 - w * .5, top, w, bottom));
    }
    break;

    case ArpeggioType::BRACKET: {
        double _spatium = item->spatium();
        double w  = item->score()->styleS(Sid::ArpeggioHookLen).val() * _spatium;
        item->setbbox(RectF(0.0, top, w, bottom));
        break;
    }
    }
}

void TLayout::layout(Articulation* item, LayoutContext&)
{
    item->setSkipDraw(false);
    if (item->isHiddenOnTabStaff()) {
        item->setSkipDraw(true);
        return;
    }

    RectF bRect;

    if (item->textType() != ArticulationTextType::NO_TEXT) {
        mu::draw::Font scaledFont(item->font());
        scaledFont.setPointSizeF(item->font().pointSizeF() * item->magS());
        mu::draw::FontMetrics fm(scaledFont);

        bRect = fm.boundingRect(scaledFont, TConv::text(item->textType()));
    } else {
        bRect = item->symBbox(item->symId());
    }

    item->setbbox(bRect.translated(-0.5 * bRect.width(), 0.0));
}

void TLayout::layout(BagpipeEmbellishment* item, LayoutContext&)
{
    /*
    if (_embelType == 0 || _embelType == 8 || _embelType == 9) {
          LOGD("BagpipeEmbellishment::layout st %d", _embelType);
          }
     */
    SymId headsym = SymId::noteheadBlack;
    SymId flagsym = SymId::flag32ndUp;

    noteList nl = item->getNoteList();
    BagpipeEmbellishment::BEDrawingDataX dx(headsym, flagsym, item->magS(), item->score()->spatium(), static_cast<int>(nl.size()));

    item->setbbox(RectF());
    /*
    if (_embelType == 0 || _embelType == 8 || _embelType == 9) {
          symMetrics("headsym", headsym);
          symMetrics("flagsym", flagsym);
          LOGD("mags %f headw %f headp %f spatium %f xl %f",
                 dx.mags, dx.headw, dx.headp, dx.spatium, dx.xl);
          }
     */

    bool drawFlag = nl.size() == 1;

    // draw the notes including stem, (optional) flag and (optional) ledger line
    double x = dx.xl;
    for (int note : nl) {
        int line = BagpipeEmbellishment::BagpipeNoteInfoList[note].line;
        BagpipeEmbellishment::BEDrawingDataY dy(line, item->score()->spatium());

        // head
        item->addbbox(item->score()->engravingFont()->bbox(headsym, dx.mags).translated(PointF(x - dx.lw * .5 - dx.headw, dy.y2)));
        /*
        if (_embelType == 0 || _embelType == 8 || _embelType == 9) {
              printBBox(" notehead", bbox());
              }
         */

        // stem
        // highest top of stems actually used is y1b
        item->addbbox(RectF(x - dx.lw * .5 - dx.headw, dy.y1b, dx.lw, dy.y2 - dy.y1b));
        /*
        if (_embelType == 0 || _embelType == 8 || _embelType == 9) {
              printBBox(" notehead + stem", bbox());
              }
         */

        // flag
        if (drawFlag) {
            item->addbbox(item->score()->engravingFont()->bbox(flagsym,
                                                               dx.mags).translated(PointF(x - dx.lw * .5 + dx.xcorr, dy.y1f + dy.ycorr)));
            // printBBox(" notehead + stem + flag", bbox());
        }

        // draw the ledger line for high A
        if (line == -2) {
            item->addbbox(RectF(x - dx.headw * 1.5 - dx.lw * .5, dy.y2 - dx.lw * 2, dx.headw * 2, dx.lw));
            /*
            if (_embelType == 8) {
                  printBBox(" notehead + stem + ledger line", bbox());
                  }
             */
        }

        // move x to next note x position
        x += dx.headp;
    }
}

void TLayout::layout(BarLine* item, LayoutContext&)
{
    item->setPos(PointF());
    // barlines hidden on this staff
    if (item->staff() && item->segment()) {
        if ((!item->staff()->staffTypeForElement(item)->showBarlines() && item->segment()->segmentType() == SegmentType::EndBarLine)
            || (item->staff()->hideSystemBarLine() && item->segment()->segmentType() == SegmentType::BeginBarLine)) {
            item->setbbox(RectF());
            return;
        }
    }

    item->setMag(item->score()->styleB(Sid::scaleBarlines) && item->staff() ? item->staff()->staffMag(item->tick()) : 1.0);
    // Note: the true values of y1 and y2 are computed in layout2() (can be done only
    // after staff distances are known). This is a temporary layout.
    double _spatium = item->spatium();
    item->setY1(_spatium * .5 * item->spanFrom());
    if (RealIsEqual(item->y2(), 0.0)) {
        item->setY2(_spatium * .5 * (8.0 + item->spanTo()));
    }

    double w = item->layoutWidth() * item->mag();
    RectF r(0.0, item->y1(), w, item->y2() - item->y1());

    if (item->score()->styleB(Sid::repeatBarTips)) {
        switch (item->barLineType()) {
        case BarLineType::START_REPEAT:
            r.unite(item->symBbox(SymId::bracketTop).translated(0, item->y1()));
            // r |= symBbox(SymId::bracketBottom).translated(0, y2);
            break;
        case BarLineType::END_REPEAT: {
            double w1 = 0.0;               //symBbox(SymId::reversedBracketTop).width();
            r.unite(item->symBbox(SymId::reversedBracketTop).translated(-w1, item->y1()));
            // r |= symBbox(SymId::reversedBracketBottom).translated(0, y2);
        }
        break;
        case BarLineType::END_START_REPEAT: {
            double w1 = 0.0;               //symBbox(SymId::reversedBracketTop).width();
            r.unite(item->symBbox(SymId::reversedBracketTop).translated(-w1, item->y1()));
            r.unite(item->symBbox(SymId::bracketTop).translated(0, item->y1()));
            // r |= symBbox(SymId::reversedBracketBottom).translated(0, y2);
        }
        break;
        default:
            break;
        }
    }
    item->setbbox(r);

    for (EngravingItem* e : *item->el()) {
        e->layout();
        if (e->isArticulation()) {
            Articulation* a  = toArticulation(e);
            DirectionV dir    = a->direction();
            double distance   = 0.5 * item->spatium();
            double x          = item->width() * .5;
            if (dir == DirectionV::DOWN) {
                double botY = item->y2() + distance;
                a->setPos(PointF(x, botY));
            } else {
                double topY = item->y1() - distance;
                a->setPos(PointF(x, topY));
            }
        }
    }
}

//---------------------------------------------------------
//    called after system layout; set vertical dimensions
//---------------------------------------------------------
void TLayout::layout2(BarLine* item, LayoutContext&)
{
    // barlines hidden on this staff
    if (item->staff() && item->segment()) {
        if ((!item->staff()->staffTypeForElement(item)->showBarlines() && item->segment()->segmentType() == SegmentType::EndBarLine)
            || (item->staff()->hideSystemBarLine() && item->segment()->segmentType() == SegmentType::BeginBarLine)) {
            item->setbbox(RectF());
            return;
        }
    }

    item->getY();
    item->bbox().setTop(item->y1());
    item->bbox().setBottom(item->y2());

    if (item->score()->styleB(Sid::repeatBarTips)) {
        switch (item->barLineType()) {
        case BarLineType::START_REPEAT:
            item->bbox().unite(item->symBbox(SymId::bracketTop).translated(0, item->y1()));
            item->bbox().unite(item->symBbox(SymId::bracketBottom).translated(0, item->y2()));
            break;
        case BarLineType::END_REPEAT:
        {
            double w1 = 0.0;               //symBbox(SymId::reversedBracketTop).width();
            item->bbox().unite(item->symBbox(SymId::reversedBracketTop).translated(-w1, item->y1()));
            item->bbox().unite(item->symBbox(SymId::reversedBracketBottom).translated(-w1, item->y2()));
            break;
        }
        case BarLineType::END_START_REPEAT:
        {
            double w1 = 0.0;               //symBbox(SymId::reversedBracketTop).width();
            item->bbox().unite(item->symBbox(SymId::reversedBracketTop).translated(-w1, item->y1()));
            item->bbox().unite(item->symBbox(SymId::reversedBracketBottom).translated(-w1, item->y2()));
            item->bbox().unite(item->symBbox(SymId::bracketTop).translated(0, item->y1()));
            item->bbox().unite(item->symBbox(SymId::bracketBottom).translated(0, item->y2()));
            break;
        }
        default:
            break;
        }
    }
}

void TLayout::layout(Beam* item, LayoutContext& ctx)
{
    BeamLayout::layout(item, ctx);
}

void TLayout::layout1(Beam* item, LayoutContext& ctx)
{
    BeamLayout::layout1(item, ctx);
}

void TLayout::layout(Bend* item, LayoutContext&)
{
    // during mtest, there may be no score. If so, exit.
    if (!item->score()) {
        return;
    }

    double _spatium = item->spatium();

    if (item->staff() && !item->staff()->isTabStaff(item->tick())) {
        if (!item->explicitParent()) {
            item->setNoteWidth(-_spatium * 2);
            item->setNotePos(PointF(0.0, _spatium * 3));
        }
    }

    double _lw = item->lineWidth();
    Note* note = toNote(item->explicitParent());
    if (note == 0) {
        item->setNoteWidth(0.0);
        item->setNotePos(PointF());
    } else {
        PointF notePos = note->pos();
        notePos.ry() = std::max(notePos.y(), 0.0);

        item->setNoteWidth(note->width());
        item->setNotePos(notePos);
    }
    RectF bb;

    mu::draw::FontMetrics fm(item->font(_spatium));

    size_t n   = item->points().size();
    double x = item->noteWidth();
    double y = -_spatium * .8;
    double x2, y2;

    double aw = _spatium * .5;
    PolygonF arrowUp;
    arrowUp << PointF(0, 0) << PointF(aw * .5, aw) << PointF(-aw * .5, aw);
    PolygonF arrowDown;
    arrowDown << PointF(0, 0) << PointF(aw * .5, -aw) << PointF(-aw * .5, -aw);

    for (size_t pt = 0; pt < n; ++pt) {
        if (pt == (n - 1)) {
            break;
        }
        int pitch = item->points().at(pt).pitch;
        if (pt == 0 && pitch) {
            y2 = -item->notePos().y() - _spatium * 2;
            x2 = x;
            bb.unite(RectF(x, y, x2 - x, y2 - y));

            bb.unite(arrowUp.translated(x2, y2 + _spatium * .2).boundingRect());

            int idx = (pitch + 12) / 25;
            const char* l = Bend::label[idx];
            bb.unite(fm.boundingRect(RectF(x2, y2, 0, 0),
                                     draw::AlignHCenter | draw::AlignBottom | draw::TextDontClip,
                                     String::fromAscii(l)));
            y = y2;
        }
        if (pitch == item->points().at(pt + 1).pitch) {
            if (pt == (n - 2)) {
                break;
            }
            x2 = x + _spatium;
            y2 = y;
            bb.unite(RectF(x, y, x2 - x, y2 - y));
        } else if (pitch < item->points().at(pt + 1).pitch) {
            // up
            x2 = x + _spatium * .5;
            y2 = -item->notePos().y() - _spatium * 2;
            double dx = x2 - x;
            double dy = y2 - y;

            PainterPath path;
            path.moveTo(x, y);
            path.cubicTo(x + dx / 2, y, x2, y + dy / 4, x2, y2);
            bb.unite(path.boundingRect());
            bb.unite(arrowUp.translated(x2, y2 + _spatium * .2).boundingRect());

            int idx = (item->points().at(pt + 1).pitch + 12) / 25;
            const char* l = Bend::label[idx];
            bb.unite(fm.boundingRect(RectF(x2, y2, 0, 0),
                                     draw::AlignHCenter | draw::AlignBottom | draw::TextDontClip,
                                     String::fromAscii(l)));
        } else {
            // down
            x2 = x + _spatium * .5;
            y2 = y + _spatium * 3;
            double dx = x2 - x;
            double dy = y2 - y;

            PainterPath path;
            path.moveTo(x, y);
            path.cubicTo(x + dx / 2, y, x2, y + dy / 4, x2, y2);
            bb.unite(path.boundingRect());

            bb.unite(arrowDown.translated(x2, y2 - _spatium * .2).boundingRect());
        }
        x = x2;
        y = y2;
    }
    bb.adjust(-_lw, -_lw, _lw, _lw);
    item->setbbox(bb);
    item->setPos(0.0, 0.0);
}

using BoxTypes = rtti::TypeList<HBox, VBox, FBox, TBox>;

class BoxVisitor : public rtti::Visitor<BoxVisitor>
{
public:
    template<typename T>
    static bool doVisit(EngravingItem* item, LayoutContext& ctx)
    {
        if (T::classof(item)) {
            TLayout::layout(static_cast<T*>(item), ctx);
            return true;
        }
        return false;
    }
};

void TLayout::layout(Box* item, LayoutContext& ctx)
{
    BoxVisitor::visit(BoxVisitor::ShouldBeFound, BoxTypes {}, item, ctx);
}

void TLayout::layoutBox(Box* item, LayoutContext&)
{
    item->MeasureBase::layout();
    for (EngravingItem* e : item->el()) {
        if (!e->isLayoutBreak()) {
            e->layout();
        }
    }
}

void TLayout::layout(HBox* item, LayoutContext& ctx)
{
    if (item->explicitParent() && item->explicitParent()->isVBox()) {
        VBox* vb = toVBox(item->explicitParent());
        double x = vb->leftMargin() * DPMM;
        double y = vb->topMargin() * DPMM;
        double w = item->point(item->boxWidth());
        double h = vb->height() - (vb->topMargin() + vb->bottomMargin()) * DPMM;
        item->setPos(x, y);
        item->bbox().setRect(0.0, 0.0, w, h);
    } else if (item->system()) {
        item->bbox().setRect(0.0, 0.0, item->point(item->boxWidth()), item->system()->height());
    } else {
        item->bbox().setRect(0.0, 0.0, 50, 50);
    }
    layoutBox(static_cast<Box*>(item), ctx);
}

void TLayout::layout(VBox* item, LayoutContext& ctx)
{
    item->setPos(PointF());

    if (item->system()) {
        item->bbox().setRect(0.0, 0.0, item->system()->width(), item->point(item->boxHeight()));
    } else {
        item->bbox().setRect(0.0, 0.0, 50, 50);
    }

    for (EngravingItem* e : item->el()) {
        if (!e->isLayoutBreak()) {
            e->layout();
        }
    }

    if (item->getProperty(Pid::BOX_AUTOSIZE).toBool()) {
        double contentHeight = item->contentRect().height();

        if (contentHeight < item->minHeight()) {
            contentHeight = item->minHeight();
        }

        item->setHeight(contentHeight);
    }

    item->MeasureBase::layout();

    if (MScore::noImages) {
        adjustLayoutWithoutImages(item, ctx);
    }
}

void TLayout::adjustLayoutWithoutImages(VBox* item, LayoutContext& ctx)
{
    double calculatedVBoxHeight = 0;
    const int padding = item->score()->spatium();
    auto elementList = item->el();

    for (auto pElement : elementList) {
        if (pElement->isText()) {
            Text* txt = toText(pElement);
            txt->bbox().moveTop(0);
            calculatedVBoxHeight += txt->height() + padding;
        }
    }

    item->setHeight(calculatedVBoxHeight);
    layoutBox(static_cast<Box*>(item), ctx);
}

void TLayout::layout(FBox* item, LayoutContext& ctx)
{
    item->bbox().setRect(0.0, 0.0, item->system()->width(), item->point(item->boxHeight()));
    layoutBox(static_cast<Box*>(item), ctx);
}

void TLayout::layout(TBox* item, LayoutContext&)
{
    item->setPos(PointF());        // !?
    item->bbox().setRect(0.0, 0.0, item->system()->width(), 0);
    item->text()->layout();

    double h = 0.;
    if (item->text()->empty()) {
        h = mu::draw::FontMetrics::ascent(item->text()->font());
    } else {
        h = item->text()->height();
    }
    double y = item->topMargin() * DPMM;
    item->text()->setPos(item->leftMargin() * DPMM, y);
    h += item->topMargin() * DPMM + item->bottomMargin() * DPMM;
    item->bbox().setRect(0.0, 0.0, item->system()->width(), h);

    item->MeasureBase::layout();    // layout LayoutBreak's
}

void TLayout::layout(Bracket* item, LayoutContext&)
{
    PainterPath& path = item->path;

    path = PainterPath();
    if (item->h2 == 0.0) {
        return;
    }

    item->setVisible(item->_bi->visible());
    item->_shape.clear();
    switch (item->bracketType()) {
    case BracketType::BRACE: {
        if (item->score()->styleSt(Sid::MusicalSymbolFont) == "Emmentaler"
            || item->score()->styleSt(Sid::MusicalSymbolFont) == "Gonville") {
            item->_braceSymbol = SymId::noSym;
            double w = item->score()->styleMM(Sid::akkoladeWidth);

#define XM(a) (a + 700) * w / 700
#define YM(a) (a + 7100) * item->h2 / 7100

            path.moveTo(XM(-8), YM(-2048));
            path.cubicTo(XM(-8), YM(-3192), XM(-360), YM(-4304), XM(-360), YM(-5400));                 // c 0
            path.cubicTo(XM(-360), YM(-5952), XM(-264), YM(-6488), XM(32), YM(-6968));                 // c 1
            path.cubicTo(XM(36), YM(-6974), XM(38), YM(-6984), XM(38), YM(-6990));                     // c 0
            path.cubicTo(XM(38), YM(-7008), XM(16), YM(-7024), XM(0), YM(-7024));                      // c 0
            path.cubicTo(XM(-8), YM(-7024), XM(-22), YM(-7022), XM(-32), YM(-7008));                   // c 1
            path.cubicTo(XM(-416), YM(-6392), XM(-544), YM(-5680), XM(-544), YM(-4960));               // c 0
            path.cubicTo(XM(-544), YM(-3800), XM(-168), YM(-2680), XM(-168), YM(-1568));               // c 0
            path.cubicTo(XM(-168), YM(-1016), XM(-264), YM(-496), XM(-560), YM(-16));                  // c 1
            path.lineTo(XM(-560), YM(0));                    //  l 1
            path.lineTo(XM(-560), YM(16));                   //  l 1
            path.cubicTo(XM(-264), YM(496), XM(-168), YM(1016), XM(-168), YM(1568));                   // c 0
            path.cubicTo(XM(-168), YM(2680), XM(-544), YM(3800), XM(-544), YM(4960));                  // c 0
            path.cubicTo(XM(-544), YM(5680), XM(-416), YM(6392), XM(-32), YM(7008));                   // c 1
            path.cubicTo(XM(-22), YM(7022), XM(-8), YM(7024), XM(0), YM(7024));                        // c 0
            path.cubicTo(XM(16), YM(7024), XM(38), YM(7008), XM(38), YM(6990));                        // c 0
            path.cubicTo(XM(38), YM(6984), XM(36), YM(6974), XM(32), YM(6968));                        // c 1
            path.cubicTo(XM(-264), YM(6488), XM(-360), YM(5952), XM(-360), YM(5400));                  // c 0
            path.cubicTo(XM(-360), YM(4304), XM(-8), YM(3192), XM(-8), YM(2048));                      // c 0
            path.cubicTo(XM(-8), YM(1320), XM(-136), YM(624), XM(-512), YM(0));                        // c 1
            path.cubicTo(XM(-136), YM(-624), XM(-8), YM(-1320), XM(-8), YM(-2048));                    // c 0*/
            item->setbbox(path.boundingRect());
            item->_shape.add(item->bbox());
        } else {
            if (item->_braceSymbol == SymId::noSym) {
                item->_braceSymbol = SymId::brace;
            }
            double h = item->h2 * 2;
            double w = item->symWidth(item->_braceSymbol) * item->_magx;
            item->bbox().setRect(0, 0, w, h);
            item->_shape.add(item->bbox());
        }
    }
    break;
    case BracketType::NORMAL: {
        double _spatium = item->spatium();
        double w = item->score()->styleMM(Sid::bracketWidth) * .5;
        double x = -w;

        double bd   = (item->score()->styleSt(Sid::MusicalSymbolFont) == "Leland") ? _spatium * .5 : _spatium * .25;
        item->_shape.add(RectF(x, -bd, w * 2, 2 * (item->h2 + bd)));
        item->_shape.add(item->symBbox(SymId::bracketTop).translated(PointF(-w, -bd)));
        item->_shape.add(item->symBbox(SymId::bracketBottom).translated(PointF(-w, bd + 2 * item->h2)));

        w      += item->symWidth(SymId::bracketTop);
        double y = -item->symHeight(SymId::bracketTop) - bd;
        double h = (-y + item->h2) * 2;
        item->bbox().setRect(x, y, w, h);
    }
    break;
    case BracketType::SQUARE: {
        double w = item->score()->styleMM(Sid::staffLineWidth) * .5;
        double x = -w;
        double y = -w;
        double h = (item->h2 + w) * 2;
        w      += (.5 * item->spatium() + 3 * w);
        item->bbox().setRect(x, y, w, h);
        item->_shape.add(item->bbox());
    }
    break;
    case BracketType::LINE: {
        double _spatium = item->spatium();
        double w = 0.67 * item->score()->styleMM(Sid::bracketWidth) * .5;
        double x = -w;
        double bd = _spatium * .25;
        double y = -bd;
        double h = (-y + item->h2) * 2;
        item->bbox().setRect(x, y, w, h);
        item->_shape.add(item->bbox());
    }
    break;
    case BracketType::NO_BRACKET:
        break;
    }
}

void TLayout::layout(Breath* item, LayoutContext&)
{
    bool palette = (!item->staff() || item->track() == mu::nidx);
    if (!palette) {
        int voiceOffset = item->placeBelow() * (item->staff()->lines(item->tick()) - 1) * item->spatium();
        if (item->isCaesura()) {
            item->setPos(item->xpos(), item->spatium() + voiceOffset);
        } else if ((item->score()->styleSt(Sid::MusicalSymbolFont) == "Emmentaler")
                   && (item->symId() == SymId::breathMarkComma)) {
            item->setPos(item->xpos(), 0.5 * item->spatium() + voiceOffset);
        } else {
            item->setPos(item->xpos(), -0.5 * item->spatium() + voiceOffset);
        }
    }
    item->setbbox(item->symBbox(item->symId()));
}

void TLayout::layout(Chord* item, LayoutContext& ctx)
{
    ChordLayout::layout(item, ctx);
}

void TLayout::layout(ChordLine* item, LayoutContext&)
{
    item->setMag(item->chord() ? item->chord()->mag() : 1);
    if (!item->modified()) {
        double x2 = 0;
        double y2 = 0;
        double baseLength = item->spatium() * (item->chord() ? item->chord()->intrinsicMag() : 1);
        double horBaseLength = 1.2 * baseLength;     // let the symbols extend a bit more horizontally
        x2 += item->isToTheLeft() ? -horBaseLength : horBaseLength;
        y2 += item->isBelow() ? baseLength : -baseLength;
        if (item->chordLineType() != ChordLineType::NOTYPE && !item->isWavy()) {
            PainterPath path;
            if (!item->isToTheLeft()) {
                if (item->isStraight()) {
                    path.lineTo(x2, y2);
                } else {
                    path.cubicTo(x2 / 2, 0.0, x2, y2 / 2, x2, y2);
                }
            } else {
                if (item->isStraight()) {
                    path.lineTo(x2, y2);
                } else {
                    path.cubicTo(0.0, y2 / 2, x2 / 2, y2, x2, y2);
                }
            }
            item->setPath(path);
        }
    }

    if (item->explicitParent()) {
        Note* note = nullptr;

        if (item->note()) {
            note = item->chord()->findNote(item->note()->pitch());
        }

        if (!note) {
            note = item->chord()->upNote();
        }

        double x = 0.0;
        double y = note->pos().y();
        double horOffset = 0.33 * item->spatium();     // one third of a space away from the note
        double vertOffset = 0.25 * item->spatium();     // one quarter of a space from the center line
        // Get chord shape
        Shape chordShape = item->chord()->shape();
        // ...but remove from the shape items that the chordline shouldn't try to avoid
        // (especially the chordline itself)
        mu::remove_if(chordShape, [](ShapeElement& shapeEl){
            if (!shapeEl.toItem) {
                return true;
            }
            const EngravingItem* item = shapeEl.toItem;
            if (item->isChordLine() || item->isHarmony() || item->isLyrics()) {
                return true;
            }
            return false;
        });
        x += item->isToTheLeft() ? -chordShape.left() - horOffset : chordShape.right() + horOffset;
        y += item->isBelow() ? vertOffset : -vertOffset;

        /// TODO: calculate properly the position for wavy type
        if (item->isWavy()) {
            bool upDir = item->chordLineType() == ChordLineType::DOIT;
            y += note->height() * (upDir ? 0.8 : -0.3);
        }

        item->setPos(x, y);
    } else {
        item->setPos(0.0, 0.0);
    }

    if (!item->isWavy()) {
        RectF r = item->path().boundingRect();
        int x1 = 0, y1 = 0, width = 0, height = 0;

        x1 = r.x();
        y1 = r.y();
        width = r.width();
        height = r.height();
        item->bbox().setRect(x1, y1, width, height);
    } else {
        RectF r(item->score()->engravingFont()->bbox(ChordLine::WAVE_SYMBOLS, item->magS()));
        double angle = ChordLine::WAVE_ANGEL * M_PI / 180;

        r.setHeight(r.height() + r.width() * sin(angle));

        /// TODO: calculate properly the rect for wavy type
        if (item->chordLineType() == ChordLineType::DOIT) {
            r.setY(item->y() - r.height() * (item->onTabStaff() ? 1.25 : 1));
        }

        item->setbbox(r);
    }
}

void TLayout::layout(Clef* item, LayoutContext&)
{
    // determine current number of lines and line distance
    int lines;
    double lineDist;
    Segment* clefSeg  = item->segment();
    int stepOffset;

    // check clef visibility and type compatibility
    if (clefSeg && item->staff()) {
        Fraction tick = clefSeg->tick();
        const StaffType* st = item->staff()->staffType(tick);
        bool show     = st->genClef();            // check staff type allows clef display
        StaffGroup staffGroup = st->group();

        // if not tab, use instrument->useDrumset to set staffGroup (to allow pitched to unpitched in same staff)
        if (staffGroup != StaffGroup::TAB) {
            staffGroup = item->staff()->part()->instrument(item->tick())->useDrumset() ? StaffGroup::PERCUSSION : StaffGroup::STANDARD;
        }

        // check clef is compatible with staff type group:
        if (ClefInfo::staffGroup(item->clefType()) != staffGroup) {
            if (tick > Fraction(0, 1) && !item->generated()) {     // if clef is not generated, hide it
                show = false;
            } else {                            // if generated, replace with initial clef type
                // TODO : instead of initial staff clef (which is assumed to be compatible)
                // use the last compatible clef previously found in staff
                item->setClefType(item->staff()->clefType(Fraction(0, 1)));
            }
        }

        // if clef not to show or not compatible with staff group
        if (!show) {
            item->setbbox(RectF());
            item->setSymId(SymId::noSym);
            LOGD("Clef::layout(): invisible clef at tick %d(%d) staff %zu",
                 item->segment()->tick().ticks(), item->segment()->tick().ticks() / 1920, item->staffIdx());
            return;
        }
        lines      = st->lines();             // init values from staff type
        lineDist   = st->lineDistance().val();
        stepOffset = st->stepOffset();
    } else {
        lines      = 5;
        lineDist   = 1.0;
        stepOffset = 0;
    }

    double _spatium = item->spatium();
    double yoff     = 0.0;
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
    case ClefType::TAB:                                    // TAB clef
        // on tablature, position clef at half the number of spaces * line distance
        yoff = lineDist * (lines - 1) * .5;
        stepOffset = 0;           //  ignore stepOffset for TAB and percussion clefs
        break;
    case ClefType::TAB4:                                    // TAB clef 4 strings
        // on tablature, position clef at half the number of spaces * line distance
        yoff = lineDist * (lines - 1) * .5;
        stepOffset = 0;
        break;
    case ClefType::TAB_SERIF:                                   // TAB clef alternate style
        // on tablature, position clef at half the number of spaces * line distance
        yoff = lineDist * (lines - 1) * .5;
        stepOffset = 0;
        break;
    case ClefType::TAB4_SERIF:                                   // TAB clef alternate style
        // on tablature, position clef at half the number of spaces * line distance
        yoff = lineDist * (lines - 1) * .5;
        stepOffset = 0;
        break;
    case ClefType::PERC:                                   // percussion clefs
        yoff = lineDist * (lines - 1) * 0.5;
        stepOffset = 0;
        break;
    case ClefType::PERC2:
        yoff = lineDist * (lines - 1) * 0.5;
        stepOffset = 0;
        break;
    case ClefType::INVALID:
    case ClefType::MAX:
        LOGD("Clef::layout: invalid type");
        return;
    default:
        break;
    }
    // clefs on palette or at start of system/measure are left aligned
    // other clefs are right aligned
    RectF r(item->symBbox(item->symId()));
    double x = item->segment() && item->segment()->rtick().isNotZero() ? -r.right() : 0.0;
    item->setPos(x, yoff * _spatium + (stepOffset * 0.5 * _spatium));

    item->setbbox(r);
}

void TLayout::layout(DeadSlapped* item, LayoutContext&)
{
    const double deadSlappedWidth = item->spatium() * 2;
    RectF rect = RectF(0, 0, deadSlappedWidth, item->staff()->height());
    item->setbbox(rect);

    // fillPath
    {
        constexpr double crossThinknessPercentage = 0.1;
        double height = rect.height();
        double width = rect.width();
        double crossThickness = width * crossThinknessPercentage;

        PointF topLeft = PointF(rect.x(), rect.y());
        PointF bottomRight = topLeft + PointF(width, height);
        PointF topRight = topLeft + PointF(width, 0);
        PointF bottomLeft = topLeft + PointF(0, height);
        PointF offsetX = PointF(crossThickness, 0);

        item->m_path1 = mu::draw::PainterPath();

        item->m_path1.moveTo(topLeft);
        item->m_path1.lineTo(topLeft + offsetX);
        item->m_path1.lineTo(bottomRight);
        item->m_path1.lineTo(bottomRight - offsetX);
        item->m_path1.lineTo(topLeft);

        item->m_path2 = mu::draw::PainterPath();

        item->m_path2.moveTo(topRight);
        item->m_path2.lineTo(topRight - offsetX);
        item->m_path2.lineTo(bottomLeft);
        item->m_path2.lineTo(bottomLeft + offsetX);
        item->m_path2.lineTo(topRight);
    }
}

void TLayout::layout(Dynamic* item, LayoutContext&)
{
    item->_snappedExpression = nullptr; // Here we reset it. It will become known again when we layout expression

    const StaffType* stType = item->staffType();

    item->_skipDraw = false;
    if (stType && stType->isHiddenElementOnTab(item->score(), Sid::dynamicsShowTabCommon, Sid::dynamicsShowTabSimple)) {
        item->_skipDraw = true;
        return;
    }

    item->TextBase::layout();

    Segment* s = item->segment();
    if (!s || (!item->_centerOnNotehead && item->align().horizontal == AlignH::LEFT)) {
        return;
    }

    EngravingItem* itemToAlign = nullptr;
    track_idx_t startTrack = staff2track(item->staffIdx());
    track_idx_t endTrack = startTrack + VOICES;
    for (track_idx_t track = startTrack; track < endTrack; ++track) {
        EngravingItem* e = s->elementAt(track);
        if (!e || (e->isRest() && toRest(e)->ticks() >= item->measure()->ticks() && item->measure()->hasVoices(e->staffIdx()))) {
            continue;
        }
        itemToAlign = e;
        break;
    }

    if (!itemToAlign->isChord()) {
        item->movePosX(itemToAlign->width() * 0.5);
        return;
    }

    Chord* chord = toChord(itemToAlign);
    bool centerOnNote = item->_centerOnNotehead || (!item->_centerOnNotehead && item->align().horizontal == AlignH::HCENTER);

    // Move to center of notehead width
    Note* note = chord->notes().at(0);
    double noteHeadWidth = note->headWidth();
    item->movePosX(noteHeadWidth * (centerOnNote ? 0.5 : 1));

    if (!item->_centerOnNotehead) {
        return;
    }

    // Use Smufl optical center for dynamic if available
    SymId symId = TConv::symId(item->dynamicType());
    double opticalCenter = item->symSmuflAnchor(symId, SmuflAnchorId::opticalCenter).x();
    if (symId != SymId::noSym && opticalCenter) {
        double symWidth = item->symBbox(symId).width();
        double offset = symWidth / 2 - opticalCenter + item->symBbox(symId).left();
        double spatiumScaling = item->spatium() / item->score()->spatium();
        offset *= spatiumScaling;
        item->movePosX(offset);
    }

    // If the dynamic contains custom text, keep it aligned
    item->movePosX(-item->customTextOffset());
}

void TLayout::layout(Expression* item, LayoutContext&)
{
    item->TextBase::layout();

    Segment* segment = item->explicitParent() ? toSegment(item->explicitParent()) : nullptr;
    if (!segment) {
        return;
    }

    if (item->align().horizontal != AlignH::LEFT) {
        Chord* chordToAlign = nullptr;
        // Look for chord in this staff
        track_idx_t startTrack = track2staff(item->staffIdx());
        track_idx_t endTrack = startTrack + VOICES;
        for (track_idx_t track = startTrack; track < endTrack; ++track) {
            EngravingItem* item = segment->elementAt(track);
            if (item && item->isChord()) {
                chordToAlign = toChord(item);
                break;
            }
        }

        if (chordToAlign) {
            Note* note = chordToAlign->notes().at(0);
            double headWidth = note->headWidth();
            bool center = item->align().horizontal == AlignH::HCENTER;
            item->movePosX(headWidth * (center ? 0.5 : 1));
        }
    }

    item->_snappedDynamic = nullptr;
    if (!item->_snapToDynamics) {
        item->autoplaceSegmentElement();
        return;
    }

    Dynamic* dynamic = toDynamic(segment->findAnnotation(ElementType::DYNAMIC, item->track(), item->track()));
    if (!dynamic || dynamic->placeAbove() != item->placeAbove()) {
        item->autoplaceSegmentElement();
        return;
    }

    item->_snappedDynamic = dynamic;
    dynamic->setSnappedExpression(item);

    // If there is a dynamic on same segment and track, lock this expression to it
    double padding = item->computeDynamicExpressionDistance();
    double dynamicRight = dynamic->shape().translate(dynamic->pos()).right();
    double expressionLeft = item->bbox().translated(item->pos()).left();
    double difference = expressionLeft - dynamicRight - padding;
    item->movePosX(-difference);

    // Keep expression and dynamic vertically aligned
    item->autoplaceSegmentElement();
    bool above = item->placeAbove();
    double yExpression = item->pos().y();
    double yDynamic = dynamic->pos().y();
    bool expressionIsOuter = above ? yExpression < yDynamic : yExpression > yDynamic;
    if (expressionIsOuter) {
        dynamic->movePosY((yExpression - yDynamic));
    } else {
        item->movePosY((yDynamic - yExpression));
    }
}

void TLayout::layout(Fermata* item, LayoutContext&)
{
    const StaffType* stType = item->staffType();

    item->_skipDraw = false;
    if (stType && stType->isHiddenElementOnTab(item->score(), Sid::fermataShowTabCommon, Sid::fermataShowTabSimple)) {
        item->_skipDraw = true;
        return;
    }

    Segment* s = item->segment();
    item->setPos(PointF());
    if (!s) {            // for use in palette
        item->setOffset(0.0, 0.0);
        RectF b(item->symBbox(item->symId()));
        item->setbbox(b.translated(-0.5 * b.width(), 0.0));
        return;
    }

    if (item->isStyled(Pid::OFFSET)) {
        item->setOffset(item->propertyDefault(Pid::OFFSET).value<PointF>());
    }
    EngravingItem* e = s->element(item->track());
    if (e) {
        if (e->isChord()) {
            Chord* chord = toChord(e);
            Note* note = chord->up() ? chord->downNote() : chord->upNote();
            double offset = chord->xpos() + note->xpos() + note->headWidth() / 2;
            item->movePosX(offset);
        } else {
            item->movePosX(e->x() - e->shape().left() + e->width() * item->staff()->staffMag(Fraction(0, 1)) * .5);
        }
    }

    String name = String::fromAscii(SymNames::nameForSymId(item->_symId).ascii());
    if (item->placeAbove()) {
        if (name.endsWith(u"Below")) {
            item->_symId = SymNames::symIdByName(name.left(name.size() - 5) + u"Above");
        }
    } else {
        item->movePosY(item->staff()->height());
        if (name.endsWith(u"Above")) {
            item->_symId = SymNames::symIdByName(name.left(name.size() - 5) + u"Below");
        }
    }
    RectF b(item->symBbox(item->_symId));
    item->setbbox(b.translated(-0.5 * b.width(), 0.0));
    item->autoplaceSegmentElement();
}

void TLayout::layout(GraceNotesGroup* item, LayoutContext&)
{
    Shape _shape;
    for (size_t i = item->size() - 1; i != mu::nidx; --i) {
        Chord* grace = item->at(i);
        Shape graceShape = grace->shape();
        Shape groupShape = _shape;
        mu::remove_if(groupShape, [grace](ShapeElement& s) {
            if (!s.toItem || (s.toItem->isStem() && s.toItem->vStaffIdx() != grace->vStaffIdx())) {
                return true;
            }
            return false;
        });
        double offset;
        offset = -std::max(graceShape.minHorizontalDistance(groupShape), 0.0);
        // Adjust spacing for cross-beam situations
        if (i < item->size() - 1) {
            Chord* prevGrace = item->at(i + 1);
            if (prevGrace->up() != grace->up()) {
                double crossCorrection = grace->notes().front()->headWidth() - grace->stem()->width();
                if (prevGrace->up() && !grace->up()) {
                    offset += crossCorrection;
                } else {
                    offset -= crossCorrection;
                }
            }
        }
        _shape.add(graceShape.translated(mu::PointF(offset, 0.0)));
        double xpos = offset - item->parent()->rxoffset() - item->parent()->xpos();
        grace->setPos(xpos, 0.0);
    }
    double xPos = -_shape.minHorizontalDistance(item->appendedSegment()->staffShape(item->parent()->staffIdx()));
    // If the parent chord is cross-staff, also check against shape in the other staff and take the minimum
    if (item->parent()->staffMove() != 0) {
        double xPosCross = -_shape.minHorizontalDistance(item->appendedSegment()->staffShape(item->parent()->vStaffIdx()));
        xPos = std::min(xPos, xPosCross);
    }
    // Same if the grace note itself is cross-staff
    Chord* firstGN = item->back();
    if (firstGN->staffMove() != 0) {
        double xPosCross = -_shape.minHorizontalDistance(item->appendedSegment()->staffShape(firstGN->vStaffIdx()));
        xPos = std::min(xPos, xPosCross);
    }
    // Safety net in case the shape checks don't succeed
    xPos = std::min(xPos, -double(item->score()->styleMM(Sid::graceToMainNoteDist) + firstGN->notes().front()->headWidth() / 2));
    item->setPos(xPos, 0.0);
}
