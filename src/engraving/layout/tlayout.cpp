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

#include "../iengravingfont.h"
#include "../libmscore/score.h"
#include "../libmscore/utils.h"

#include "../libmscore/accidental.h"
#include "../libmscore/actionicon.h"
#include "../libmscore/ambitus.h"
#include "../libmscore/arpeggio.h"

#include "../libmscore/note.h"

#include "../libmscore/staff.h"

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
