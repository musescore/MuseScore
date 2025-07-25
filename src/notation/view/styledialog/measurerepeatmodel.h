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

#pragma once

#include "abstractstyledialogmodel.h"

namespace mu::notation {
class MeasureRepeatModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(StyleItem * measureRepeatNumberPos READ measureRepeatNumberPos CONSTANT)
    Q_PROPERTY(StyleItem * mrNumberSeries READ mrNumberSeries CONSTANT)
    Q_PROPERTY(StyleItem * oneMeasureRepeatShow1 READ oneMeasureRepeatShow1 CONSTANT)
    Q_PROPERTY(StyleItem * fourMeasureRepeatShowExtenders READ fourMeasureRepeatShowExtenders CONSTANT)
    Q_PROPERTY(StyleItem * mrNumberEveryXMeasures READ mrNumberEveryXMeasures CONSTANT)
    Q_PROPERTY(StyleItem * mrNumberSeriesWithParentheses READ mrNumberSeriesWithParentheses CONSTANT)

public:
    explicit MeasureRepeatModel(QObject* parent = nullptr);

    StyleItem* measureRepeatNumberPos() const;
    StyleItem* mrNumberSeries() const;
    StyleItem* oneMeasureRepeatShow1() const;
    StyleItem* fourMeasureRepeatShowExtenders() const;
    StyleItem* mrNumberEveryXMeasures() const;
    StyleItem* mrNumberSeriesWithParentheses() const;
};
}
