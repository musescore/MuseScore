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

#pragma once

#include "abstractstyledialogmodel.h"

namespace mu::notation {
class BeamsPageModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(mu::notation::StyleItem * useWideBeams READ useWideBeams CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * beamWidth READ beamWidth CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * beamMinLen READ beamMinLen CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * beamNoSlope READ beamNoSlope CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * frenchStyleBeams READ frenchStyleBeams CONSTANT)

    QML_ELEMENT

public:
    explicit BeamsPageModel(QObject* parent = nullptr);

    StyleItem* useWideBeams() const;
    StyleItem* beamWidth() const;
    StyleItem* beamMinLen() const;
    StyleItem* beamNoSlope() const;
    StyleItem* frenchStyleBeams() const;
};
}
