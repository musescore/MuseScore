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

#include "engraving/libmscore/engravingitem.h"
#include "engraving/libmscore/score.h"

#include "engraving/libmscore/accidental.h"
#include "engraving/libmscore/clef.h"

#include "engraving/layout/pal/tlayout.h"

using namespace mu::engraving;
using namespace mu::palette;

void PaletteLayout::layoutItem(EngravingItem* item)
{
    layout::pal::LayoutContext ctxpal(item->score());
    Context ctx(item->score());

    switch (item->type()) {
    case ElementType::ACCIDENTAL:   layout(toAccidental(item), ctx);
        break;
    case ElementType::CLEF:         layout(toClef(item), ctx);
        break;
    default:
        layout::pal::TLayout::layoutItem(item, ctxpal);
        break;
    }
}

const MStyle& PaletteLayout::Context::style() const
{
    return m_score->style();
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
