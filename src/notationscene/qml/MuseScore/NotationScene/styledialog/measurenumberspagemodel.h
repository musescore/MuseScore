/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
class MeasureNumbersPageModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(mu::notation::StyleItem * showMeasureNumber READ showMeasureNumber CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * showMeasureNumberOne READ showMeasureNumberOne CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * measureNumberInterval READ measureNumberInterval CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * measureNumberSystem READ measureNumberSystem CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * measureNumberVPlacement READ measureNumberVPlacement CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * measureNumberHPlacement READ measureNumberHPlacement CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * measureNumberPlacementMode READ measureNumberPlacementMode CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * measureNumberPosAbove READ measureNumberPosAbove NOTIFY measureNumberPosAboveChanged FINAL)
    Q_PROPERTY(mu::notation::StyleItem * measureNumberPosBelow READ measureNumberPosBelow CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * measureNumberAlignToBarline READ measureNumberAlignToBarline CONSTANT)

    Q_PROPERTY(mu::notation::StyleItem * mmRestShowMeasureNumberRange READ mmRestShowMeasureNumberRange CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * mmRestRangeBracketType READ mmRestRangeBracketType CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * mmRestRangePosAbove READ mmRestRangePosAbove CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * mmRestRangePosBelow READ mmRestRangePosBelow CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * mmRestRangeVPlacement READ mmRestRangeVPlacement CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * mmRestRangeHPlacement READ mmRestRangeHPlacement CONSTANT)

    Q_PROPERTY(QVariantList textStyles READ textStyles CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * measureNumberTextStyle READ measureNumberTextStyle CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * mmRestRangeTextStyle READ mmRestRangeTextStyle CONSTANT)

    QML_ELEMENT

public:
    explicit MeasureNumbersPageModel(QObject* parent = nullptr);

    StyleItem* showMeasureNumber() const;
    StyleItem* showMeasureNumberOne() const;
    StyleItem* measureNumberInterval() const;
    StyleItem* measureNumberSystem() const;
    StyleItem* measureNumberVPlacement() const;
    StyleItem* measureNumberHPlacement() const;
    StyleItem* measureNumberPlacementMode() const;
    StyleItem* measureNumberPosAbove() const;
    StyleItem* measureNumberPosBelow() const;
    StyleItem* measureNumberAlignToBarline() const;

    StyleItem* mmRestShowMeasureNumberRange() const;
    StyleItem* mmRestRangeBracketType() const;
    StyleItem* mmRestRangePosAbove() const;
    StyleItem* mmRestRangePosBelow() const;
    StyleItem* mmRestRangeVPlacement() const;
    StyleItem* mmRestRangeHPlacement() const;

    QVariantList textStyles();

    StyleItem* measureNumberTextStyle() const;
    StyleItem* mmRestRangeTextStyle() const;

signals:
    void measureNumberPosAboveChanged() const;

private:
    StyleItem* buildStyleItem(StyleId id) const override;
};
}
