/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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
#include "pointfpropertyitem.h"

#include "dataformatter.h"

using namespace mu::inspector;

PointFPropertyItem::PointFPropertyItem(const mu::engraving::Pid propertyId, QObject* parent)
    : mu::inspector::PropertyItem(propertyId, parent)
{
}

qreal PointFPropertyItem::x_property() const
{
    return DataFormatter::roundDouble(value().toPointF().x());
}

qreal PointFPropertyItem::x() const
{
    return value().toPointF().x();
}

void PointFPropertyItem::setX(qreal newX)
{
    setValue(QPointF(newX, y()));
}

qreal PointFPropertyItem::y_property() const
{
    return DataFormatter::roundDouble(value().toPointF().y());
}

qreal PointFPropertyItem::y() const
{
    return value().toPointF().y();
}

void PointFPropertyItem::setY(qreal newY)
{
    setValue(QPointF(x(), newY));
}
