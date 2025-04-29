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

#pragma once

#include "layoutcontext.h"

namespace mu::engraving {
class BarLine;
class Page;
class System;
class StaffLines;
class TextBase;
}

namespace mu::engraving::rendering::score {
class MaskLayout
{
public:
    static void computeMasks(LayoutContext& ctx, Page* page);

    static void maskTABStringLinesForFrets(StaffLines* staffLines, const LayoutContext& ctx);

private:
    static void computeBarlineMasks(const Segment* barlineSement, const System* system, const std::vector<TextBase*>& allSystemText,
                                    LayoutContext& ctx);
    static void maskBarlineForText(BarLine* barline, const std::vector<TextBase*>& allSystemText);
    static std::vector<TextBase*> collectAllSystemText(const System* system);

    static void cleanupMask(const Shape& itemShape, Shape& mask, double minFragmentLength);
};
} // namespace mu::engraving::rendering::score
