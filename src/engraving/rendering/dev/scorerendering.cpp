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
#include "scorerendering.h"

#include "libmscore/arpeggio.h"
#include "libmscore/beam.h"
#include "libmscore/textlinebase.h"
#include "libmscore/harmony.h"
#include "libmscore/chord.h"

#include "tdraw.h"
#include "tlayout.h"
#include "chordlayout.h"
#include "layoutcontext.h"
#include "scorelayout.h"
#include "arpeggiolayout.h"
#include "horizontalspacing.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::dev;

void ScoreRendering::layoutRange(Score* score, const Fraction& st, const Fraction& et)
{
    ScoreLayout::layoutRange(score, st, et);
}

void ScoreRendering::doLayoutItem(EngravingItem* item)
{
    LayoutContext ctx(item->score());
    TLayout::layoutItem(item, ctx);
}

void ScoreRendering::doDrawItem(const EngravingItem* item, draw::Painter* p)
{
    TDraw::drawItem(item, p);
}

void ScoreRendering::layoutText1(TextBase* item, bool base)
{
    LayoutContext ctx(item->score());
    if (base) {
        TLayout::layout1TextBase(item, ctx);
    } else if (Harmony::classof(item)) {
        TLayout::layout1(static_cast<Harmony*>(item), ctx);
    } else {
        TLayout::layout1TextBase(item, ctx);
    }
}

// ===============================================================
// Layout Elements on Edit
// ===============================================================
void ScoreRendering::layoutOnEdit(Arpeggio* item)
{
    LayoutContext ctx(item->score());
    ArpeggioLayout::layoutOnEdit(item, ctx);
}

double ScoreRendering::computePadding(const EngravingItem* item1, const EngravingItem* item2)
{
    return HorizontalSpacing::computePadding(item1, item2);
}

KerningType ScoreRendering::computeKerning(const EngravingItem* item1, const EngravingItem* item2)
{
    return HorizontalSpacing::computeKerning(item1, item2);
}

void ScoreRendering::layoutTextLineBaseSegment(TextLineBaseSegment* item)
{
    LayoutContext ctx(item->score());
    TLayout::layoutTextLineBaseSegment(item, ctx);
}

void ScoreRendering::layoutBeam1(Beam* item)
{
    LayoutContext ctx(item->score());
    TLayout::layout1(item, ctx);
}

void ScoreRendering::layoutStem(Chord* item)
{
    LayoutContext ctx(item->score());
    ChordLayout::layoutStem(item, ctx);
}
