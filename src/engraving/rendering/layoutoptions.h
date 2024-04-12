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
#ifndef MU_ENGRAVING_LAYOUTOPTIONS_H
#define MU_ENGRAVING_LAYOUTOPTIONS_H

namespace mu::engraving {
//---------------------------------------------------------
//   LayoutMode
//    PAGE   The normal page view, honors page and line breaks.
//    LINE   The panoramic view, one long system
//    FLOAT  The "reflow" mode, ignore page and line breaks, stave spacer up, fixed and down
//    SYSTEM The "never ending page", page break are turned into line break
//---------------------------------------------------------

enum class LayoutMode : char {
    PAGE, FLOAT, LINE, SYSTEM, HORIZONTAL_FIXED
};

struct LayoutOptions
{
    LayoutMode mode = LayoutMode::PAGE;

    bool isShowVBox = true;
    double noteHeadWidth = 0.0;

    bool isMode(LayoutMode m) const { return mode == m; }
    bool isLinearMode() const { return mode == LayoutMode::LINE || mode == LayoutMode::HORIZONTAL_FIXED; }
};
}

#endif // MU_ENGRAVING_LAYOUTOPTIONS_H
