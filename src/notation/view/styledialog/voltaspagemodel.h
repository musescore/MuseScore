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
class VoltasPageModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(StyleItem * voltaPosAbove READ voltaPosAbove CONSTANT)
    Q_PROPERTY(StyleItem * voltaHook READ voltaHook CONSTANT)
    Q_PROPERTY(StyleItem * voltaLineWidth READ voltaLineWidth CONSTANT)
    Q_PROPERTY(StyleItem * voltaLineStyle READ voltaLineStyle CONSTANT)
    Q_PROPERTY(StyleItem * voltaDashLineLen READ voltaDashLineLen CONSTANT)
    Q_PROPERTY(StyleItem * voltaDashGapLen READ voltaDashGapLen CONSTANT)

    Q_PROPERTY(StyleItem * voltaAlignStartBeforeKeySig READ voltaAlignStartBeforeKeySig CONSTANT)
    Q_PROPERTY(StyleItem * voltaAlignEndLeftOfBarline READ voltaAlignEndLeftOfBarline CONSTANT)

public:
    explicit VoltasPageModel(QObject* parent = nullptr);

    StyleItem* voltaPosAbove() const;
    StyleItem* voltaHook() const;
    StyleItem* voltaLineWidth() const;
    StyleItem* voltaLineStyle() const;
    StyleItem* voltaDashLineLen() const;
    StyleItem* voltaDashGapLen() const;

    StyleItem* voltaAlignStartBeforeKeySig() const;
    StyleItem* voltaAlignEndLeftOfBarline() const;
};
}
