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

#include "layoutcontext.h"

#include "dom/box.h"

namespace mu::engraving::rendering::score {
class BoxLayout
{
public:
    static void layoutBox(const Box* item, Box::LayoutData* ldata, const LayoutContext& ctx); // factory

    static void layoutHBox(const HBox* item, HBox::LayoutData* ldata, const LayoutContext& ctx);
    static void layoutHBox2(HBox* item, const LayoutContext& ctx);

    static void layoutVBox(const VBox* item, VBox::LayoutData* ldata, const LayoutContext& ctx);

    static void layoutTBox(const TBox* item, TBox::LayoutData* ldata, const LayoutContext& ctx);

    static void layoutFBox(const FBox* item, FBox::LayoutData* ldata, const LayoutContext& ctx);

private:
    static void layoutBaseBox(const Box* item, Box::LayoutData* ldata, const LayoutContext& ctx); // base class
};
}
