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
#include "layoutindependent.h"

#include "global/async/asyncable.h"
#include "draw/fontmetrics.h"

#include "../../types/typesconv.h"
#include "../../infrastructure/rtti.h"

#include "../../dom/note.h"

using namespace mu::draw;
using namespace mu::engraving;
using namespace mu::engraving::rendering::dev;
using namespace mu::engraving::rtti;

bool LayoutIndependent::isItemIndepended(const EngravingItem* item)
{
    static LayoutContext _dummy(nullptr);
    return dolayoutItem(item, nullptr, _dummy, false);
}

bool LayoutIndependent::layoutItem(EngravingItem* item, const LayoutContext& ctx)
{
    return dolayoutItem(item, item->mutLayoutData(), ctx, true);
}

bool LayoutIndependent::layoutItem(const EngravingItem* item, EngravingItem::LayoutData* ldata, const LayoutContext& ctx)
{
    return dolayoutItem(item, ldata, ctx, true);
}

bool LayoutIndependent::dolayoutItem(const EngravingItem* item, EngravingItem::LayoutData* ldata, const LayoutContext& ctx, bool perform)
{
    switch (item->type()) {
    case ElementType::ACCIDENTAL:
        if (perform && !ldata->isValid()) {
            layout(item_cast<const Accidental*>(item), static_cast<Accidental::LayoutData*>(ldata), ctx.conf());
        }
        break;
    case ElementType::ACTION_ICON:
        if (perform && !ldata->isValid()) {
            layout(item_cast<const ActionIcon*>(item), static_cast<ActionIcon::LayoutData*>(ldata));
        }
        break;
    case ElementType::ARTICULATION:
        if (perform && !ldata->isValid()) {
            layout(item_cast<const Articulation*>(item), static_cast<Articulation::LayoutData*>(ldata));
        }
        break;
    case ElementType::ORNAMENT:
        if (perform && !ldata->isValid()) {
            layout(item_cast<const Ornament*>(item), static_cast<Ornament::LayoutData*>(ldata), ctx.conf());
        }
        break;
    default:
        return false;
    }

    return true;
}

void LayoutIndependent::layout(const Accidental* item, Accidental::LayoutData* ldata, const LayoutConfiguration& conf)
{
    ldata->syms.clear();

    // TODO: remove Accidental in layout
    // don't show accidentals for tab or slash notation
    if (item->onTabStaff() || (item->note() && item->note()->fixed())) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);

    if (item->accidentalType() == AccidentalType::NONE) {
        return;
    }

    auto accidentalSingleSym = [](const Accidental* item) -> SymId
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
    };

    auto accidentalBracketSyms = [](AccidentalBracket type) -> std::pair<SymId, SymId>
    {
        switch (type) {
        case AccidentalBracket::PARENTHESIS: return { SymId::accidentalParensLeft, SymId::accidentalParensRight };
        case AccidentalBracket::BRACKET: return { SymId::accidentalBracketLeft, SymId::accidentalBracketRight };
        case AccidentalBracket::BRACE: return { SymId::accidentalCombiningOpenCurlyBrace, SymId::accidentalCombiningCloseCurlyBrace };
        case AccidentalBracket::NONE: return { SymId::noSym, SymId::noSym };
        }
        return { SymId::noSym, SymId::noSym };
    };

    // Single?
    SymId singleSym = accidentalSingleSym(item);
    if (singleSym != SymId::noSym && conf.engravingFont()->isValid(singleSym)) {
        Accidental::LayoutData::Sym s(singleSym, 0.0, 0.0);
        ldata->syms.push_back(s);

        ldata->addBbox(item->symBbox(singleSym));
    }
    // Multi
    else {
        double margin = conf.styleMM(Sid::bracketedAccidentalPadding);
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
            ldata->syms.push_back(ls);
            ldata->addBbox(item->symBbox(bracketSyms.first));

            x += item->symAdvance(bracketSyms.first) + margin;
        }

        // Main
        SymId mainSym = item->symId();
        Accidental::LayoutData::Sym ms(mainSym, x, 0.0);
        ldata->syms.push_back(ms);
        ldata->addBbox(item->symBbox(mainSym).translated(x, 0.0));

        // Right
        if (bracketSyms.second != SymId::noSym) {
            x += item->symAdvance(mainSym) + margin;

            Accidental::LayoutData::Sym rs(bracketSyms.second, x,
                                           item->bracket() == AccidentalBracket::BRACE ? item->spatium() * 0.4 : 0.0);
            ldata->syms.push_back(rs);
            ldata->addBbox(item->symBbox(bracketSyms.second).translated(x, 0.0));
        }
    }
}

void LayoutIndependent::layout(const ActionIcon* item, ActionIcon::LayoutData* ldata)
{
    FontMetrics fontMetrics(item->iconFont());
    ldata->setBbox(fontMetrics.boundingRect(Char(item->icon())));
}

void LayoutIndependent::layout(const Articulation* item, Articulation::LayoutData* ldata)
{
    if (item->isHiddenOnTabStaff()) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);

    RectF bbox;

    if (item->textType() == ArticulationTextType::NO_TEXT) {
        bbox = item->symBbox(item->symId());

        //! NOTE symId can be changed during layout if anchor is auto
        static mu::async::Asyncable holder;
        item->symIdChanged().resetOnReceive(&holder);
        item->symIdChanged().onReceive(&holder, [item, ldata](SymId val) {
            RectF bbox = item->symBbox(val);
            ldata->setBbox(bbox.translated(-0.5 * bbox.width(), 0.0));
        });
    } else {
        Font scaledFont(item->font());
        scaledFont.setPointSizeF(item->font().pointSizeF() * item->magS());
        FontMetrics fm(scaledFont);
        bbox = fm.boundingRect(scaledFont, TConv::text(item->textType()));
    }

    ldata->setBbox(bbox.translated(-0.5 * bbox.width(), 0.0));
}

void LayoutIndependent::layout(const Ornament* item, Ornament::LayoutData* ldata, const LayoutConfiguration& conf)
{
    layout(static_cast<const Articulation*>(item), ldata);

    double _spatium = item->spatium();
    double vertMargin = 0.35 * _spatium;
    static constexpr double ornamentAccidentalMag = 0.6; // TODO: style?

    for (size_t i = 0; i < item->accidentalsAboveAndBelow().size(); ++i) {
        bool above = (i == 0);
        Accidental* accidental = item->accidentalsAboveAndBelow()[i];
        if (!accidental) {
            continue;
        }
        accidental->computeMag();
        Accidental::LayoutData* accLData = accidental->mutLayoutData();
        accLData->setMag(accLData->mag() * ornamentAccidentalMag);
        layout(accidental, accLData, conf);
        Shape accidentalShape = accidental->shape();
        double minVertDist = above
                             ? accidentalShape.minVerticalDistance(ldata->bbox())
                             : Shape(ldata->bbox()).minVerticalDistance(accidentalShape);

        accLData->setPos(-0.5 * accLData->bbox().width(), above ? (-minVertDist - vertMargin) : (minVertDist + vertMargin));
    }
}
