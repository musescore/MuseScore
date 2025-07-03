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

#pragma once

#include "abstractstyledialogmodel.h"

namespace mu::notation {
class BarNumbersPageModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(StyleItem * showMeasureNumber READ showMeasureNumber CONSTANT)
    Q_PROPERTY(StyleItem * showMeasureNumberOne READ showMeasureNumberOne CONSTANT)
    Q_PROPERTY(StyleItem * measureNumberInterval READ measureNumberInterval CONSTANT)
    Q_PROPERTY(StyleItem * measureNumberSystem READ measureNumberSystem CONSTANT)
    Q_PROPERTY(StyleItem * measureNumberVPlacement READ measureNumberVPlacement CONSTANT)
    Q_PROPERTY(StyleItem * measureNumberHPlacement READ measureNumberHPlacement CONSTANT)
    Q_PROPERTY(StyleItem * measureNumberHPlacementInterval READ measureNumberHPlacementInterval CONSTANT)
    Q_PROPERTY(StyleItem * measureNumberAllStaves READ measureNumberAllStaves CONSTANT)
    Q_PROPERTY(StyleItem * measureNumberPosAbove READ measureNumberPosAbove CONSTANT)
    Q_PROPERTY(StyleItem * measureNumberPosBelow READ measureNumberPosBelow CONSTANT)
    Q_PROPERTY(StyleItem * measureNumberAlignToBarline READ measureNumberAlignToBarline CONSTANT)
    Q_PROPERTY(StyleItem * measureNumberAlignToBarlineInterval READ measureNumberAlignToBarlineInterval CONSTANT)

    Q_PROPERTY(StyleItem * mmRestShowMeasureNumberRange READ mmRestShowMeasureNumberRange CONSTANT)
    Q_PROPERTY(StyleItem * mmRestRangeBracketType READ mmRestRangeBracketType CONSTANT)
    Q_PROPERTY(StyleItem * mmRestRangePosAbove READ mmRestRangePosAbove CONSTANT)
    Q_PROPERTY(StyleItem * mmRestRangePosBelow READ mmRestRangePosBelow CONSTANT)
    Q_PROPERTY(StyleItem * mmRestRangeVPlacement READ mmRestRangeVPlacement CONSTANT)
    Q_PROPERTY(StyleItem * mmRestRangeHPlacement READ mmRestRangeHPlacement CONSTANT)

    Q_PROPERTY(QVariantList textStyles READ textStyles)
    Q_PROPERTY(StyleItem * measureNumberTextStyle READ measureNumberTextStyle CONSTANT)
    Q_PROPERTY(StyleItem * mmRestRangeTextStyle READ mmRestRangeTextStyle CONSTANT)

public:
    explicit BarNumbersPageModel(QObject* parent = nullptr);

    StyleItem* showMeasureNumber() const;
    StyleItem* showMeasureNumberOne() const;
    StyleItem* measureNumberInterval() const;
    StyleItem* measureNumberSystem() const;
    StyleItem* measureNumberVPlacement() const;
    StyleItem* measureNumberHPlacement() const;
    StyleItem* measureNumberHPlacementInterval() const;
    StyleItem* measureNumberAllStaves() const;
    StyleItem* measureNumberPosAbove() const;
    StyleItem* measureNumberPosBelow() const;
    StyleItem* measureNumberAlignToBarline() const;
    StyleItem* measureNumberAlignToBarlineInterval() const;

    StyleItem* mmRestShowMeasureNumberRange() const;
    StyleItem* mmRestRangeBracketType() const;
    StyleItem* mmRestRangePosAbove() const;
    StyleItem* mmRestRangePosBelow() const;
    StyleItem* mmRestRangeVPlacement() const;
    StyleItem* mmRestRangeHPlacement() const;

    QVariantList textStyles();

    StyleItem* measureNumberTextStyle() const;
    StyleItem* mmRestRangeTextStyle() const;
};
}
