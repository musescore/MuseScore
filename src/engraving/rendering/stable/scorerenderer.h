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
#ifndef MU_ENGRAVING_SCORERENDERER_STABLE_H
#define MU_ENGRAVING_SCORERENDERER_STABLE_H

#include "../iscorerenderer.h"

namespace mu::engraving {
class Score;
}

namespace mu::engraving::rendering::stable  {
class ScoreRenderer : public IScoreRenderer
{
public:

    // Main interface
    void layoutScore(Score* score, const Fraction& st, const Fraction& et) const override;

    SizeF pageSizeInch(const Score* score) const override;
    SizeF pageSizeInch(const Score* score, const PaintOptions& opt) const override;
    void paintScore(draw::Painter* painter, Score* score, const IScoreRenderer::PaintOptions& opt) const override;
    void paintItem(draw::Painter& painter, const EngravingItem* item) const override;

    //! TODO Investigation is required, probably these functions or their calls should not be.
    // Other
    void layoutTextLineBaseSegment(TextLineBaseSegment* item) override;
    void layoutBeam1(Beam* item) override;
    void layoutStem(Chord* item) override;

    // Layout Text 1
    void layoutText1(TextBase* item, bool base = false) override;

    void computeBezier(TieSegment* tieSeg, PointF shoulderOffset = PointF()) override;
    void computeBezier(SlurSegment* slurSeg, PointF shoulderOffset = PointF()) override;

private:
    // Layout Single Item
    void doLayoutItem(EngravingItem* item) override;

    void doDrawItem(const EngravingItem* item, draw::Painter* p) override;
};
}

#endif // MU_ENGRAVING_SCORERENDERER_STABLE_H
