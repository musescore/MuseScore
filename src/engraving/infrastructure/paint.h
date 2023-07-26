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
#ifndef MU_ENGRAVING_PAINT_H
#define MU_ENGRAVING_PAINT_H

#include <functional>
#include <vector>
#include "draw/painter.h"

namespace mu::engraving {
class EngravingItem;
class Page;
class Score;

class Paint
{
public:

    struct Options
    {
        bool isSetViewport = true;
        bool isPrinting = false;
        bool isMultiPage = false;
        bool printPageBackground = true;
        RectF frameRect;
        int fromPage = -1; // 0 is first
        int toPage = -1;
        int copyCount = 1;
        int trimMarginPixelSize = -1;
        int deviceDpi = -1;

        std::function<void(draw::Painter* painter, const Page* page, const RectF& pageRect)> onPaintPageSheet;
        std::function<void()> onNewPage;
    };

    static void paintScore(draw::Painter* painter, Score* score, const Options& opt);
    static void paintElement(draw::Painter& painter, const EngravingItem* element);
    static void paintElements(draw::Painter& painter, const std::vector<EngravingItem*>& elements, bool isPrinting);

    static SizeF pageSizeInch(const Score* score);
    static SizeF pageSizeInch(const Score* score, const Options& opt);

private:
};
}

#endif // MU_ENGRAVING_PAINT_H
