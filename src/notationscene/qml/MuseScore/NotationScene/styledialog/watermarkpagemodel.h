/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
class WatermarkPageModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(mu::notation::StyleItem* watermarkEnabled READ watermarkEnabled CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem* watermarkType READ watermarkType CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem* watermarkText READ watermarkText CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem* watermarkOpacity READ watermarkOpacity CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem* watermarkAngle READ watermarkAngle CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem* watermarkImagePath READ watermarkImagePath CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem* watermarkImageScale READ watermarkImageScale CONSTANT)

    QML_ELEMENT

public:
    explicit WatermarkPageModel(QObject* parent = nullptr);

    StyleItem* watermarkEnabled() const;
    StyleItem* watermarkType() const;
    StyleItem* watermarkText() const;
    StyleItem* watermarkOpacity() const;
    StyleItem* watermarkAngle() const;
    StyleItem* watermarkImagePath() const;
    StyleItem* watermarkImageScale() const;
};
}
