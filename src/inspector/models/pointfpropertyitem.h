/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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
#ifndef MU_INSPECTOR_POINTFPROPERTYITEM_H
#define MU_INSPECTOR_POINTFPROPERTYITEM_H

#include "propertyitem.h"

#if (defined(_MSCVER) || defined(_MSC_VER))
// unreferenced function with internal linkage has been removed
#pragma warning(disable: 4505)
#endif

namespace mu::inspector {
class PointFPropertyItem : public PropertyItem
{
    Q_OBJECT

    Q_PROPERTY(qreal x READ x_property WRITE setX NOTIFY valueChanged)
    Q_PROPERTY(qreal y READ y_property WRITE setY NOTIFY valueChanged)

public:
    explicit PointFPropertyItem(const mu::engraving::Pid propertyId, QObject* parent = nullptr);

    qreal x_property() const;
    qreal x() const;
    void setX(qreal newX);

    qreal y_property() const;
    qreal y() const;
    void setY(qreal newY);
};
}

#endif // MU_INSPECTOR_POINTFPROPERTYITEM_H
