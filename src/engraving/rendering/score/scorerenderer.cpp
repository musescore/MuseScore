/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#include "scorerenderer.h"

#include "dom/arpeggio.h"
#include "dom/beam.h"
#include "dom/textlinebase.h"
#include "dom/harmony.h"
#include "dom/chord.h"

#include "tdraw.h"
#include "tlayout.h"
#include "chordlayout.h"
#include "layoutcontext.h"
#include "scorelayout.h"
#include "arpeggiolayout.h"
#include "horizontalspacing.h"
#include "slurtielayout.h"
#include "masklayout.h"

#include "paint.h"

#include "log.h"

using namespace muse::draw;
using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

void ScoreRenderer::layoutScore(Score* score, const Fraction& st, const Fraction& et) const
{
    ScoreLayout::layoutRange(score, st, et);
}

SizeF ScoreRenderer::pageSizeInch(const Score* score) const
{
    return Paint::pageSizeInch(score);
}

SizeF ScoreRenderer::pageSizeInch(const Score* score, const PaintOptions& opt) const
{
    return Paint::pageSizeInch(score, opt);
}

void ScoreRenderer::paintScore(Painter* painter, Score* score, const IScoreRenderer::PaintOptions& opt) const
{
    Paint::paintScore(painter, score, opt);
}

void ScoreRenderer::paintItem(Painter& painter, const EngravingItem* item) const
{
    Paint::paintItem(painter, item);
}

void ScoreRenderer::doLayoutItem(EngravingItem* item)
{
    LayoutContext ctx(item->score());
    TLayout::layoutItem(item, ctx);
}

void ScoreRenderer::doDrawItem(const EngravingItem* item, Painter* p)
{
    TDraw::drawItem(item, p);
}

void ScoreRenderer::layoutText1(TextBase* item, bool base)
{
    LayoutContext ctx(item->score());
    if (base) {
        TLayout::layoutBaseTextBase1(item, ctx);
    } else if (Harmony::classof(item)) {
        TLayout::layoutHarmony(static_cast<Harmony*>(item), static_cast<Harmony*>(item)->mutldata(), ctx);
    } else {
        TLayout::layoutBaseTextBase1(item, ctx);
    }
}

void ScoreRenderer::computeBezier(TieSegment* tieSeg, PointF shoulderOffset)
{
    SlurTieLayout::computeBezier(tieSeg, shoulderOffset);
}

void ScoreRenderer::computeBezier(SlurSegment* slurSeg, PointF shoulderOffset)
{
    SlurTieLayout::computeBezier(slurSeg, shoulderOffset);
}

void ScoreRenderer::computeMasks(Score* score)
{
    LayoutContext ctx(score);
    for (Page* page : score->pages()) {
        MaskLayout::computeMasks(ctx, page);
    }
}

void ScoreRenderer::layoutTextLineBaseSegment(TextLineBaseSegment* item)
{
    LayoutContext ctx(item->score());
    TLayout::layoutTextLineBaseSegment(item, ctx);
}

void ScoreRenderer::layoutBeam1(Beam* item)
{
    LayoutContext ctx(item->score());
    TLayout::layoutBeam1(item, ctx);
}

void ScoreRenderer::layoutStem(Chord* item)
{
    LayoutContext ctx(item->score());
    ChordLayout::layoutStem(item, ctx);
}
