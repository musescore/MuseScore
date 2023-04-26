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

#include "../libmscore/accidental.h"
#include "../libmscore/actionicon.h"

#include "../libmscore/note.h"

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
