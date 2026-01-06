/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "abstractstyledialogmodel.h"

namespace mu::notation {
class BendsPageModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(mu::notation::StyleItem * guitarBendUseFull READ guitarBendUseFull CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * guitarBendLineWidth READ guitarBendLineWidth CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * guitarBendLineWidthTab READ guitarBendLineWidthTab CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * guitarBendArrowWidth READ guitarBendArrowWidth CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * guitarBendArrowHeight READ guitarBendArrowHeight CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * guitarDivesAboveStaff READ guitarDivesAboveStaff CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * useCueSizeFretForGraceBends READ useCueSizeFretForGraceBends CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * useFractionCharacters READ useFractionCharacters CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * alignPreBendAndPreDiveToGraceNote READ alignPreBendAndPreDiveToGraceNote CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * guitarDiveLineWidth READ guitarDiveLineWidth CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * guitarDiveLineWidthTab READ guitarDiveLineWidthTab CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * whammyBarText READ whammyBarText CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * whammyBarLineStyle READ whammyBarLineStyle CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * whammyBarDashLineLen READ whammyBarDashLineLen CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * whammyBarDashGapLen READ whammyBarDashGapLen CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * whammyBarLineWidth READ whammyBarLineWidth CONSTANT)

    QML_ELEMENT

public:
    explicit BendsPageModel(QObject* parent = nullptr);

    StyleItem* guitarBendUseFull() const;
    StyleItem* guitarBendLineWidth() const;
    StyleItem* guitarBendLineWidthTab() const;
    StyleItem* guitarBendArrowWidth() const;
    StyleItem* guitarBendArrowHeight() const;
    StyleItem* guitarDivesAboveStaff() const;
    StyleItem* useCueSizeFretForGraceBends() const;
    StyleItem* useFractionCharacters() const;
    StyleItem* alignPreBendAndPreDiveToGraceNote() const;
    StyleItem* guitarDiveLineWidth() const;
    StyleItem* guitarDiveLineWidthTab() const;
    StyleItem* whammyBarText() const;
    StyleItem* whammyBarLineStyle() const;
    StyleItem* whammyBarDashLineLen() const;
    StyleItem* whammyBarDashGapLen() const;
    StyleItem* whammyBarLineWidth() const;
};
}
