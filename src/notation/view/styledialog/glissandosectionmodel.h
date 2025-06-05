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
class GlissandoSectionModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(StyleItem * glissandoLineStyle READ glissandoLineStyle CONSTANT)
    Q_PROPERTY(StyleItem * glissandoLineStyleDashSize READ glissandoLineStyleDashSize CONSTANT)
    Q_PROPERTY(StyleItem * glissandoLineStyleGapSize READ glissandoLineStyleGapSize CONSTANT)
    Q_PROPERTY(StyleItem * glissandoLineWidth READ glissandoLineWidth CONSTANT)
    Q_PROPERTY(StyleItem * glissandoShowText READ glissandoShowText CONSTANT)
    Q_PROPERTY(StyleItem * glissandoText READ glissandoText CONSTANT)
    Q_PROPERTY(StyleItem * glissandoLineType READ glissandoLineType CONSTANT)

public:
    explicit GlissandoSectionModel(QObject* parent = nullptr);

    StyleItem* glissandoLineStyle() const;
    StyleItem* glissandoLineStyleDashSize() const;
    StyleItem* glissandoLineStyleGapSize() const;
    StyleItem* glissandoLineWidth() const;
    StyleItem* glissandoShowText() const;
    StyleItem* glissandoText() const;
    StyleItem* glissandoLineType() const;
};
}
