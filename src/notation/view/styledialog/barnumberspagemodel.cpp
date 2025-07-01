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
#include "barnumberspagemodel.h"

using namespace mu::notation;
using namespace mu::engraving;

BarNumbersPageModel::BarNumbersPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, { StyleId::showMeasureNumber,
                                         StyleId::showMeasureNumberOne,
                                         StyleId::measureNumberInterval,
                                         StyleId::measureNumberSystem,
                                         StyleId::measureNumberHPlacement,
                                         StyleId::measureNumberHPlacementInterval,
                                         StyleId::measureNumberAllStaves,
                                         StyleId::measureNumberPosAbove,
                                         StyleId::measureNumberPosBelow,

                                         StyleId::mmRestShowMeasureNumberRange,
                                         StyleId::mmRestRangeBracketType,
                                         StyleId::mmRestRangePosAbove,
                                         StyleId::mmRestRangePosBelow,
                                         StyleId::mmRestRangeVPlacement,
                                         StyleId::mmRestRangeHPlacement,
                               })
{
}

StyleItem* BarNumbersPageModel::showMeasureNumber() const { return styleItem(StyleId::showMeasureNumber); }
StyleItem* BarNumbersPageModel::showMeasureNumberOne() const { return styleItem(StyleId::showMeasureNumberOne); }
StyleItem* BarNumbersPageModel::measureNumberInterval() const { return styleItem(StyleId::measureNumberInterval); }
StyleItem* BarNumbersPageModel::measureNumberSystem() const { return styleItem(StyleId::measureNumberSystem); }
StyleItem* BarNumbersPageModel::measureNumberHPlacement() const { return styleItem(StyleId::measureNumberHPlacement); }
StyleItem* BarNumbersPageModel::measureNumberHPlacementInterval() const { return styleItem(StyleId::measureNumberHPlacementInterval); }
StyleItem* BarNumbersPageModel::measureNumberAllStaves() const { return styleItem(StyleId::measureNumberAllStaves); }
StyleItem* BarNumbersPageModel::measureNumberPosAbove() const { return styleItem(StyleId::measureNumberPosAbove); }
StyleItem* BarNumbersPageModel::measureNumberPosBelow() const { return styleItem(StyleId::measureNumberPosBelow); }

StyleItem* BarNumbersPageModel::mmRestShowMeasureNumberRange() const { return styleItem(StyleId::mmRestShowMeasureNumberRange); }
StyleItem* BarNumbersPageModel::mmRestRangeBracketType() const { return styleItem(StyleId::mmRestRangeBracketType); }
StyleItem* BarNumbersPageModel::mmRestRangePosAbove() const { return styleItem(StyleId::mmRestRangePosAbove); }
StyleItem* BarNumbersPageModel::mmRestRangePosBelow() const { return styleItem(StyleId::mmRestRangePosBelow); }
StyleItem* BarNumbersPageModel::mmRestRangeVPlacement() const { return styleItem(StyleId::mmRestRangeVPlacement); }
StyleItem* BarNumbersPageModel::mmRestRangeHPlacement() const { return styleItem(StyleId::mmRestRangeHPlacement); }
