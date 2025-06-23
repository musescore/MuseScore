/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "dom/parenthesis.h"
#include "layoutcontext.h"

namespace mu::engraving {
class Segment;
class System;
}

namespace mu::engraving::rendering::score {
class ParenthesisLayout
{
public:
    static void layoutParentheses(const EngravingItem* parent, const LayoutContext& ctx);
    static void layoutParenthesis(Parenthesis* item, Parenthesis::LayoutData* ldata, const LayoutContext& ctx);

    static double computeParenthesisPadding(const EngravingItem* item1, const EngravingItem* item2);
    static double computeInternalParenthesisPadding(const EngravingItem* item1, const EngravingItem* item2);

private:
    static void createPathAndShape(Parenthesis* item, Parenthesis::LayoutData* ldata);
    static bool isInternalParenPadding(const EngravingItem* item1, const EngravingItem* item2);

    static void setLayoutValues(Parenthesis* item, Parenthesis::LayoutData* ldata, const LayoutContext& ctx);
    static void setClefValues(Parenthesis* item, Parenthesis::LayoutData* ldata);
    static void setTimeSigValues(Parenthesis* item, Parenthesis::LayoutData* ldata, const LayoutContext& ctx);
    static void setNoteValues(Parenthesis* item, Parenthesis::LayoutData* ldata);
    static void setDefaultValues(Parenthesis* item, Parenthesis::LayoutData* ldata);
};
}
