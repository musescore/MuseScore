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
#ifndef MU_NOTATION_TIEPLACEMENTSELECTOR_H
#define MU_NOTATION_TIEPLACEMENTSELECTOR_H

#include "abstractstyledialogmodel.h"

namespace mu::notation {
class TiePlacementSelectorModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(StyleItem * placementSingleNotes READ placementSingleNotes CONSTANT)
    Q_PROPERTY(StyleItem * placementChords READ placementChords CONSTANT)
    Q_PROPERTY(StyleItem * placementDots READ placementDots CONSTANT)

public:
    explicit TiePlacementSelectorModel(QObject* parent = nullptr);

    StyleItem* placementSingleNotes() const;
    StyleItem* placementChords() const;
    StyleItem* placementDots() const;
};
}

#endif // MU_NOTATION_TIEPLACEMENTSELECTOR_H
