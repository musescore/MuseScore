/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "offsetSelect.h"

#include "engraving/dom/types.h"

using namespace mu::notation;

OffsetSelect::OffsetSelect(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);

    setFocusProxy(xVal);

    connect(xVal, &QDoubleSpinBox::valueChanged, this, &OffsetSelect::_offsetChanged);
    connect(yVal, &QDoubleSpinBox::valueChanged, this, &OffsetSelect::_offsetChanged);
}

void OffsetSelect::setSuffix(const QString& s)
{
    xVal->setSuffix(s);
    yVal->setSuffix(s);
}

void OffsetSelect::_offsetChanged()
{
    emit offsetChanged(QPointF(xVal->value(), yVal->value()));
}

QPointF OffsetSelect::offset() const
{
    return QPointF(xVal->value(), yVal->value());
}

void OffsetSelect::blockOffset(bool val)
{
    xVal->blockSignals(val);
    yVal->blockSignals(val);
}

void OffsetSelect::setOffset(const QPointF& o)
{
    blockOffset(true);
    xVal->setValue(o.x());
    yVal->setValue(o.y());
    blockOffset(false);
}
