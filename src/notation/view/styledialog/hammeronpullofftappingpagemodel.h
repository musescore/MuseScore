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
class HammerOnPullOffTappingPageModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(StyleItem * showOnStandardStaves READ showOnStandardStaves CONSTANT)
    Q_PROPERTY(StyleItem * showOnTabStaves READ showOnTabStaves CONSTANT)
    Q_PROPERTY(StyleItem * hopoUpperCase READ hopoUpperCase CONSTANT)
    Q_PROPERTY(StyleItem * hopoShowAll READ hopoShowAll CONSTANT)

public:
    explicit HammerOnPullOffTappingPageModel(QObject* parent = nullptr);

    StyleItem* showOnStandardStaves() const;
    StyleItem* showOnTabStaves() const;
    StyleItem* hopoUpperCase() const;
    StyleItem* hopoShowAll() const;
};
}
