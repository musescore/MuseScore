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
#include "accidentalrenderer.h"

#include "infrastructure/rtti.h"

#include "libmscore/accidental.h"
#include "libmscore/note.h"

#include "layoutcontext.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::dev;
using namespace mu::engraving::rtti;

void AccidentalRenderer::layout(EngravingItem* item, LayoutContext& ctx) const
{
    layoutAccidental(item_cast<Accidental*>(item), ctx);
}

static SymId accidentalSingleSym(const Accidental* item)
{
    // if the accidental is standard (doubleflat, flat, natural, sharp or double sharp)
    // and it has either no bracket or parentheses, then we have glyphs straight from smufl.

    if (item->bracket() == AccidentalBracket::PARENTHESIS && !item->parentNoteHasParentheses()) {
        switch (item->accidentalType()) {
        case AccidentalType::FLAT:      return SymId::accidentalFlatParens;
        case AccidentalType::FLAT2:     return SymId::accidentalDoubleFlatParens;
        case AccidentalType::NATURAL:   return SymId::accidentalNaturalParens;
        case AccidentalType::SHARP:     return SymId::accidentalSharpParens;
        case AccidentalType::SHARP2:    return SymId::accidentalDoubleSharpParens;
        default:
            break;
        }
    }
    return SymId::noSym;
}

static std::pair<SymId, SymId> accidentalBracketSyms(AccidentalBracket type)
{
    switch (type) {
    case AccidentalBracket::PARENTHESIS: return { SymId::accidentalParensLeft, SymId::accidentalParensRight };
    case AccidentalBracket::BRACKET: return { SymId::accidentalBracketLeft, SymId::accidentalBracketRight };
    case AccidentalBracket::BRACE: return { SymId::accidentalCombiningOpenCurlyBrace, SymId::accidentalCombiningCloseCurlyBrace };
    case AccidentalBracket::NONE: return { SymId::noSym, SymId::noSym };
    }
    return { SymId::noSym, SymId::noSym };
}

void AccidentalRenderer::layoutAccidental(const Accidental* item, const LayoutContext& ctx, Accidental::LayoutData& data)
{
    // TODO: remove Accidental in layout
    // don't show accidentals for tab or slash notation
    if (item->onTabStaff() || (item->note() && item->note()->fixed())) {
        data.isSkipDraw = true;
        return;
    }

    if (item->accidentalType() == AccidentalType::NONE) {
        return;
    }

    // Single?
    SymId singleSym = accidentalSingleSym(item);
    if (singleSym != SymId::noSym && ctx.engravingFont()->isValid(singleSym)) {
        Accidental::LayoutData::Sym s(singleSym, 0.0, 0.0);
        data.syms.push_back(s);

        data.bbox.unite(item->symBbox(singleSym));
    }
    // Multi
    else {
        double margin = ctx.conf().styleMM(Sid::bracketedAccidentalPadding);
        double x = 0.0;

        std::pair<SymId, SymId> bracketSyms;
        bool isNeedBracket = item->bracket() != AccidentalBracket::NONE && !item->parentNoteHasParentheses();
        if (isNeedBracket) {
            bracketSyms = accidentalBracketSyms(item->bracket());
        }

        // Left
        if (bracketSyms.first != SymId::noSym) {
            Accidental::LayoutData::Sym ls(bracketSyms.first, 0.0,
                                           item->bracket() == AccidentalBracket::BRACE ? item->spatium() * 0.4 : 0.0);
            data.syms.push_back(ls);
            data.bbox.unite(item->symBbox(bracketSyms.first));

            x += item->symAdvance(bracketSyms.first) + margin;
        }

        // Main
        SymId mainSym = item->symId();
        Accidental::LayoutData::Sym ms(mainSym, x, 0.0);
        data.syms.push_back(ms);
        data.bbox.unite(item->symBbox(mainSym).translated(x, 0.0));

        // Right
        if (bracketSyms.second != SymId::noSym) {
            x += item->symAdvance(mainSym) + margin;

            Accidental::LayoutData::Sym rs(bracketSyms.second, x,
                                           item->bracket() == AccidentalBracket::BRACE ? item->spatium() * 0.4 : 0.0);
            data.syms.push_back(rs);
            data.bbox.unite(item->symBbox(bracketSyms.second).translated(x, 0.0));
        }
    }
}

void AccidentalRenderer::layoutAccidental(Accidental* item, const LayoutContext& ctx)
{
    Accidental::LayoutData data;
    layoutAccidental(item, ctx, data);
    item->setLayoutData(data);
}

void AccidentalRenderer::draw(const EngravingItem* item, draw::Painter* painter) const
{
    draw(item_cast<const Accidental*>(item), painter);
}

void AccidentalRenderer::draw(const Accidental* item, draw::Painter* painter)
{
    TRACE_DRAW_ITEM;
    if (item->layoutData().isSkipDraw) {
        return;
    }

    painter->setPen(item->curColor());
    for (const Accidental::LayoutData::Sym& e : item->layoutData().syms) {
        item->drawSymbol(e.sym, painter, PointF(e.x, e.y));
    }
}
