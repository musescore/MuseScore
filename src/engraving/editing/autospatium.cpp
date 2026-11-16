/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "autospatium.h"

#include "../dom/measurebase.h"
#include "../dom/score.h"

using namespace mu::engraving;

//--------------------------------------------------------
// update()
// Reduces the spatium size as necessary to accommodate all
// staves in the page. Caution: the spatium is expressed in
// DPI, page dimensions in inches, staff sizes in mm.
//--------------------------------------------------------
void AutoSpatium::update(Score* score)
{
    static constexpr double breathingSpace = 2.5; // allow breathing space between staves
    static constexpr double minStaffHeight = 2.0; // never make staff smaller than 2.0 mm
    static constexpr double maxStaffHeight = 7.0; // never make staff bigger than 7.0 mm (default value)

    const MStyle& style = score->style();
    double availableHeight = (style.styleD(Sid::pageHeight)
                              - style.styleD(Sid::pageOddTopMargin)
                              - style.styleD(Sid::pageOddBottomMargin)) * DPI; // convert from inches to DPI
    double titleHeight = (!score->measures()->empty() && score->measures()->first()->isVBox()) ? score->measures()->first()->height() : 0.0;
    availableHeight -= titleHeight;
    double totalNeededSpaces = 4 * score->nstaves() * breathingSpace;
    double targetSpatium = availableHeight / totalNeededSpaces;

    double resultingStaffHeight = 4 * targetSpatium / DPI * INCH; // conversion from DPI to mm
    resultingStaffHeight = round(resultingStaffHeight * 10) / 10; // round to nearest 0.1 mm
    if (resultingStaffHeight > maxStaffHeight) {
        return;
    }
    if (resultingStaffHeight < minStaffHeight) {
        resultingStaffHeight = minStaffHeight;
    }

    targetSpatium = (resultingStaffHeight / 4) * DPI / INCH;

    score->style().setSpatium(targetSpatium);
    score->updatePaddingTables();
}
