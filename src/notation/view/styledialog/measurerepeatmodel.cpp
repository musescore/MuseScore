/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#include "measurerepeatmodel.h"

using namespace mu::notation;

MeasureRepeatModel::MeasureRepeatModel(QObject* parent)
    : AbstractStyleDialogModel(parent, {
    StyleId::measureRepeatNumberPos,
    StyleId::mrNumberSeries,
    StyleId::oneMeasureRepeatShow1,
    StyleId::fourMeasureRepeatShowExtenders,
    StyleId::mrNumberEveryXMeasures,
    StyleId::mrNumberSeriesWithParentheses
})
{
}

StyleItem* MeasureRepeatModel::measureRepeatNumberPos() const
{
    return styleItem(StyleId::measureRepeatNumberPos);
}

StyleItem* MeasureRepeatModel::mrNumberSeries() const
{
    return styleItem(StyleId::mrNumberSeries);
}

StyleItem* MeasureRepeatModel::oneMeasureRepeatShow1() const
{
    return styleItem(StyleId::oneMeasureRepeatShow1);
}

StyleItem* MeasureRepeatModel::fourMeasureRepeatShowExtenders() const
{
    return styleItem(StyleId::fourMeasureRepeatShowExtenders);
}

StyleItem* MeasureRepeatModel::mrNumberEveryXMeasures() const
{
    return styleItem(StyleId::mrNumberEveryXMeasures);
}

StyleItem* MeasureRepeatModel::mrNumberSeriesWithParentheses() const
{
    return styleItem(StyleId::mrNumberSeriesWithParentheses);
}
