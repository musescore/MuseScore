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

#include "../libmscore/note.h"

#include "../libmscore/staff.h"
#include "../libmscore/system.h"

#include "../libmscore/text.h"
#include "../libmscore/textframe.h"

#include "beamlayout.h"

using namespace mu::draw;
using namespace mu::engraving;

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
