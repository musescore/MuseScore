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
#include "engraving/style/textstyle.h"

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
    for (TextStyleType textStyleType : allTextStyles()) {
        const TextStyle* style = textStyle(textStyleType);
        int offsetPropertyIdx = static_cast<int>(TextStylePropertyType::Offset) - 1;
        addStyleId(style->at(offsetPropertyIdx).sid);
    }
}

StyleItem* MeasureNumbersPageModel::showMeasureNumber() const { return styleItem(StyleId::showMeasureNumber); }
StyleItem* MeasureNumbersPageModel::showMeasureNumberOne() const { return styleItem(StyleId::showMeasureNumberOne); }
StyleItem* MeasureNumbersPageModel::measureNumberInterval() const { return styleItem(StyleId::measureNumberInterval); }
StyleItem* MeasureNumbersPageModel::measureNumberSystem() const { return styleItem(StyleId::measureNumberSystem); }
StyleItem* MeasureNumbersPageModel::measureNumberVPlacement() const { return styleItem(StyleId::measureNumberVPlacement); }
StyleItem* MeasureNumbersPageModel::measureNumberHPlacement() const { return styleItem(StyleId::measureNumberHPlacement); }
StyleItem* MeasureNumbersPageModel::measureNumberAllStaves() const { return styleItem(StyleId::measureNumberAllStaves); }

StyleItem* MeasureNumbersPageModel::measureNumberPosAbove() const
{
    TextStyleType textStyleType = styleItem(StyleId::measureNumberTextStyle)->value().value<TextStyleType>();
    const TextStyle* ts = textStyle(textStyleType);
    int offsetPropertyIdx = static_cast<int>(TextStylePropertyType::Offset) - 1;
    return styleItem(ts->at(offsetPropertyIdx).sid);
}

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

StyleItem* MeasureNumbersPageModel::buildStyleItem(StyleId id) const
{
    StyleItem* item = AbstractStyleDialogModel::buildStyleItem(id);

    if (id == StyleId::measureNumberTextStyle) {
        connect(item, &StyleItem::valueChanged, this, [this]() { emit measureNumberPosAboveChanged(); });
    }

    return item;
}

QVariantList MeasureNumbersPageModel::textStyles()
{
    QVariantList textStyles;
    textStyles.reserve(int(TextStyleType::TEXT_TYPES));

    for (TextStyleType type : allTextStyles()) {
        QVariantMap style;
        style["text"] = TConv::userName(type).qTranslated();
        style["value"] = static_cast<int>(type);
        textStyles << style;
    }

    return textStyles;
}
