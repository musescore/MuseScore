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
#include "measurenumberspagemodel.h"

#include "engraving/dom/textbase.h"
#include "engraving/types/typesconv.h"

using namespace mu::notation;
using namespace mu::engraving;

MeasureNumbersPageModel::MeasureNumbersPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, { StyleId::showMeasureNumber,
                                         StyleId::showMeasureNumberOne,
                                         StyleId::measureNumberInterval,
                                         StyleId::measureNumberSystem,
                                         StyleId::measureNumberVPlacement,
                                         StyleId::measureNumberHPlacement,
                                         StyleId::measureNumberAllStaves,
                                         StyleId::measureNumberPosAbove,
                                         StyleId::measureNumberPosBelow,
                                         StyleId::measureNumberAlignToBarline,

                                         StyleId::mmRestShowMeasureNumberRange,
                                         StyleId::mmRestRangeBracketType,
                                         StyleId::mmRestRangePosAbove,
                                         StyleId::mmRestRangePosBelow,
                                         StyleId::mmRestRangeVPlacement,
                                         StyleId::mmRestRangeHPlacement,

                                         StyleId::measureNumberTextStyle,
                                         StyleId::mmRestRangeTextStyle,
                               })
{
}

StyleItem* MeasureNumbersPageModel::showMeasureNumber() const { return styleItem(StyleId::showMeasureNumber); }
StyleItem* MeasureNumbersPageModel::showMeasureNumberOne() const { return styleItem(StyleId::showMeasureNumberOne); }
StyleItem* MeasureNumbersPageModel::measureNumberInterval() const { return styleItem(StyleId::measureNumberInterval); }
StyleItem* MeasureNumbersPageModel::measureNumberSystem() const { return styleItem(StyleId::measureNumberSystem); }
StyleItem* MeasureNumbersPageModel::measureNumberVPlacement() const { return styleItem(StyleId::measureNumberVPlacement); }
StyleItem* MeasureNumbersPageModel::measureNumberHPlacement() const { return styleItem(StyleId::measureNumberHPlacement); }
StyleItem* MeasureNumbersPageModel::measureNumberAllStaves() const { return styleItem(StyleId::measureNumberAllStaves); }
StyleItem* MeasureNumbersPageModel::measureNumberPosAbove() const { return styleItem(StyleId::measureNumberPosAbove); }
StyleItem* MeasureNumbersPageModel::measureNumberPosBelow() const { return styleItem(StyleId::measureNumberPosBelow); }
StyleItem* MeasureNumbersPageModel::measureNumberAlignToBarline() const { return styleItem(StyleId::measureNumberAlignToBarline); }

StyleItem* MeasureNumbersPageModel::mmRestShowMeasureNumberRange() const { return styleItem(StyleId::mmRestShowMeasureNumberRange); }
StyleItem* MeasureNumbersPageModel::mmRestRangeBracketType() const { return styleItem(StyleId::mmRestRangeBracketType); }
StyleItem* MeasureNumbersPageModel::mmRestRangePosAbove() const { return styleItem(StyleId::mmRestRangePosAbove); }
StyleItem* MeasureNumbersPageModel::mmRestRangePosBelow() const { return styleItem(StyleId::mmRestRangePosBelow); }
StyleItem* MeasureNumbersPageModel::mmRestRangeVPlacement() const { return styleItem(StyleId::mmRestRangeVPlacement); }
StyleItem* MeasureNumbersPageModel::mmRestRangeHPlacement() const { return styleItem(StyleId::mmRestRangeHPlacement); }

StyleItem* MeasureNumbersPageModel::measureNumberTextStyle() const { return styleItem(StyleId::measureNumberTextStyle); }
StyleItem* MeasureNumbersPageModel::mmRestRangeTextStyle() const { return styleItem(StyleId::mmRestRangeTextStyle); }

QVariantList mu::notation::MeasureNumbersPageModel::textStyles()
{
    QVariantList textStyles;
    textStyles.reserve(int(TextStyleType::TEXT_TYPES));

    for (int t = int(TextStyleType::DEFAULT) + 1; t < int(TextStyleType::TEXT_TYPES); ++t) {
        QVariantMap style;
        style["text"] = TConv::userName(static_cast<TextStyleType>(t)).qTranslated();
        style["value"] = t;
        textStyles << style;
    }

    return textStyles;
}
