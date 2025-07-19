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
class NoteLineSectionModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(StyleItem * noteLineStyle READ noteLineStyle CONSTANT)
    Q_PROPERTY(StyleItem * noteLineStyleDashSize READ noteLineStyleDashSize CONSTANT)
    Q_PROPERTY(StyleItem * noteLineStyleGapSize READ noteLineStyleGapSize CONSTANT)
    Q_PROPERTY(StyleItem * noteLineWidth READ noteLineWidth CONSTANT)

public:
    explicit NoteLineSectionModel(QObject* parent = nullptr);

    StyleItem* noteLineStyle() const;
    StyleItem* noteLineStyleDashSize() const;
    StyleItem* noteLineStyleGapSize() const;
    StyleItem* noteLineWidth() const;
};
}
