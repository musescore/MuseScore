/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
class RestsPageModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(StyleItem * multiVoiceRestTwoSpaceOffset READ multiVoiceRestTwoSpaceOffset CONSTANT)
    Q_PROPERTY(StyleItem * mergeMatchingRests READ mergeMatchingRests CONSTANT)
    Q_PROPERTY(StyleItem * alignAdjacentRests READ alignAdjacentRests CONSTANT)

public:
    explicit RestsPageModel(QObject* parent = nullptr);

    StyleItem* multiVoiceRestTwoSpaceOffset() const;
    StyleItem* mergeMatchingRests() const;
    StyleItem* alignAdjacentRests() const;
};
}
