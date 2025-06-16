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

#include "harmonylayout.h"
#include "tlayout.h"

#include "dom/fret.h"
#include "dom/harmony.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

void mu::engraving::rendering::score::HarmonyLayout::layoutHarmony(const Harmony* item, Harmony::LayoutData* ldata,
                                                                   const LayoutContext& ctx)
{
    if (!item->explicitParent()) {
        ldata->setPos(0.0, 0.0);
        const_cast<Harmony*>(item)->setOffset(0.0, 0.0);
    }

    if (ldata->layoutInvalid) {
        item->createBlocks(ldata);
    }

    if (ldata->blocks.empty()) {
        ldata->blocks.push_back(TextBlock());
    }

    auto calculateBoundingRect = [](const Harmony* item, Harmony::LayoutData* ldata, const LayoutContext& ctx) -> PointF
    {
        const double ypos = (item->placeBelow() && item->staff()) ? item->staff()->staffHeight(item->tick()) : 0.0;
        const FretDiagram* fd = (item->explicitParent() && item->explicitParent()->isFretDiagram())
                                ? toFretDiagram(item->explicitParent())
                                : nullptr;

        const double cw = item->symWidth(SymId::noteheadBlack);

        double newPosX = 0.0;
        double newPosY = 0.0;

        if (item->textList().empty()) {
            TLayout::layoutBaseTextBase1(item, ldata);

            if (fd) {
                newPosY = ldata->pos().y();
            } else {
                newPosY = ypos - ((item->align() == AlignV::BOTTOM) ? -ldata->bbox().height() : 0.0);
            }
        } else {
            RectF bb;
            RectF hAlignBox;
            for (TextSegment* ts : item->textList()) {
                RectF tsBbox = ts->tightBoundingRect().translated(ts->x(), ts->y());
                bb.unite(tsBbox);

                if (ts->align()) {
                    hAlignBox.unite(tsBbox);
                }
            }

            double xx = 0.0;
            if (fd) {
                switch (ctx.conf().styleV(Sid::chordAlignmentToFretboard).value<AlignH>()) {
                case AlignH::LEFT:
                    xx = -hAlignBox.left();
                    break;
                case AlignH::HCENTER:
                    xx = -(hAlignBox.center().x());
                    break;
                case AlignH::RIGHT:
                    xx = -hAlignBox.right();
                    break;
                }
            } else {
                switch (item->noteheadAlign()) {
                case AlignH::LEFT:
                    xx = -hAlignBox.left();
                    break;
                case AlignH::HCENTER:
                    xx = -(hAlignBox.center().x());
                    break;
                case AlignH::RIGHT:
                    xx = -hAlignBox.right();
                    break;
                }
            }

            double yy = -bb.y();      // Align::TOP
            if (item->align() == AlignV::VCENTER) {
                yy = -bb.y() / 2.0;
            } else if (item->align() == AlignV::BASELINE) {
                yy = item->baseLine();
            } else if (item->align() == AlignV::BOTTOM) {
                yy = -bb.height() - bb.y();
            }

            if (fd) {
                newPosY = ypos - yy - ctx.conf().styleMM(Sid::harmonyFretDist);
            } else {
                newPosY = ypos;
            }

            for (TextSegment* ts : item->textList()) {
                ts->setOffset(PointF(xx, yy));
            }

            ldata->setBbox(bb.translated(xx, yy));
            ldata->harmonyHeight = ldata->bbox().height();
        }

        if (fd) {
            switch (ctx.conf().styleV(Sid::chordAlignmentToFretboard).value<AlignH>()) {
            case AlignH::LEFT:
                newPosX = 0.0;
                break;
            case AlignH::HCENTER:
                newPosX = 0.5 * fd->mainWidth();
                break;
            case AlignH::RIGHT:
                newPosX = fd->mainWidth();
                break;
            }
        } else {
            switch (item->noteheadAlign()) {
            case AlignH::LEFT:
                newPosX = 0.0;
                break;
            case AlignH::HCENTER:
                newPosX = cw * 0.5;
                break;
            case AlignH::RIGHT:
                newPosX = cw;
                break;
            }
        }

        return PointF(newPosX, newPosY);
    };

    auto positionPoint = calculateBoundingRect(item, ldata, ctx);

    if (item->isPolychord()) {
        for (LineF& line : ldata->polychordDividerLines.mut_value()) {
            line.setP1(PointF(ldata->bbox().left(), line.y1()));
            line.setP2(PointF(ldata->bbox().right(), line.y2()));
        }
    }

    if (item->hasFrame()) {
        item->layoutFrame(ldata);
    }

    ldata->setPos(positionPoint);
}
