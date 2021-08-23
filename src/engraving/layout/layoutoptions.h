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
#ifndef MU_ENGRAVING_LAYOUTOPTIONS_H
#define MU_ENGRAVING_LAYOUTOPTIONS_H

#include "style/styledef.h"
#include "style/style.h"
#include "libmscore/mscore.h"

namespace mu::engraving {
//---------------------------------------------------------
//   LayoutMode
//    PAGE   The normal page view, honors page and line breaks.
//    LINE   The panoramic view, one long system
//    FLOAT  The "reflow" mode, ignore page and line breaks
//    SYSTEM The "never ending page", page break are turned into line break
//---------------------------------------------------------

enum class LayoutMode : char {
    PAGE, FLOAT, LINE, SYSTEM
};

struct LayoutOptions
{
    LayoutMode mode = LayoutMode::PAGE;

    bool showVBox = true;

    // from style
    qreal loWidth = 0;
    qreal loHeight = 0;
    bool firstSystemIndent = true;

    qreal maxFretShiftAbove = 0;
    qreal maxFretShiftBelow = 0;

    Ms::VerticalAlignRange verticalAlignRange = Ms::VerticalAlignRange::SEGMENT;

    bool isMode(LayoutMode m) const { return mode == m; }

    void updateFromStyle(const Ms::MStyle& style)
    {
        loWidth = style.styleD(Ms::Sid::pageWidth) * Ms::DPI;
        loHeight = style.styleD(Ms::Sid::pageHeight) * Ms::DPI;

        firstSystemIndent = style.styleB(Ms::Sid::enableIndentationOnFirstSystem);

        maxFretShiftAbove = style.styleP(Ms::Sid::maxFretShiftAbove);
        maxFretShiftBelow = style.styleP(Ms::Sid::maxFretShiftBelow);

        verticalAlignRange = Ms::VerticalAlignRange(style.styleI(Ms::Sid::autoplaceVerticalAlignRange));
    }
};
}

#endif // MU_ENGRAVING_LAYOUTOPTIONS_H
