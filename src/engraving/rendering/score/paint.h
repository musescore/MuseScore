/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_ENGRAVING_PAINT_DEV_H
#define MU_ENGRAVING_PAINT_DEV_H

#include <vector>

#include "draw/painter.h"
#include "../iscorerenderer.h"

namespace mu::engraving {
class EngravingItem;
class Page;
class Score;
}

namespace mu::engraving::rendering::score {
class Paint
{
public:

    static void paintScore(muse::draw::Painter* painter, Score* score, const IScoreRenderer::PaintOptions& opt);
    static void paintItem(muse::draw::Painter& painter, const EngravingItem* item);
    static void paintItems(muse::draw::Painter& painter, const std::vector<EngravingItem*>& items);

    static SizeF pageSizeInch(const Score* score);
    static SizeF pageSizeInch(const Score* score, const IScoreRenderer::PaintOptions& opt);

private:
};
}

#endif // MU_ENGRAVING_PAINT_DEV_H
