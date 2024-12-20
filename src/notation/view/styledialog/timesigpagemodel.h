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
class TimeSigPageModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(StyleItem * timeSigPlacement READ timeSigPlacement CONSTANT)

    Q_PROPERTY(StyleItem * timeSigNormalStyle READ timeSigNormalStyle CONSTANT)
    Q_PROPERTY(StyleItem * timeSigAboveStyle READ timeSigAboveStyle CONSTANT)
    Q_PROPERTY(StyleItem * timeSigAcrossStyle READ timeSigAcrossStyle CONSTANT)

    Q_PROPERTY(StyleItem * timeSigNormalNumDist READ timeSigNormalNumDist CONSTANT)
    Q_PROPERTY(StyleItem * timeSigAboveNumDist READ timeSigAboveNumDist CONSTANT)
    Q_PROPERTY(StyleItem * timeSigAcrossNumDist READ timeSigAcrossNumDist CONSTANT)

    Q_PROPERTY(StyleItem * timeSigNormalY READ timeSigNormalY CONSTANT)
    Q_PROPERTY(StyleItem * timeSigAboveY READ timeSigAboveY CONSTANT)
    Q_PROPERTY(StyleItem * timeSigAcrossY READ timeSigAcrossY CONSTANT)

    Q_PROPERTY(StyleItem * timeSigCenterOnBarline READ timeSigCenterOnBarline CONSTANT)
    Q_PROPERTY(StyleItem * timeSigHangIntoMargin READ timeSigHangIntoMargin CONSTANT)
    Q_PROPERTY(StyleItem * timeSigCenterAcrossStaveGroup READ timeSigCenterAcrossStaveGroup CONSTANT)

    Q_PROPERTY(StyleItem * timeSigNormalScale READ timeSigNormalScale CONSTANT)
    Q_PROPERTY(StyleItem * timeSigAboveScale READ timeSigAboveScale CONSTANT)
    Q_PROPERTY(StyleItem * timeSigAcrossScale READ timeSigAcrossScale CONSTANT)

public:
    explicit TimeSigPageModel(QObject* parent = nullptr);

    StyleItem* timeSigPlacement() const;

    StyleItem* timeSigNormalStyle() const;
    StyleItem* timeSigAboveStyle() const;
    StyleItem* timeSigAcrossStyle() const;

    StyleItem* timeSigNormalNumDist() const;
    StyleItem* timeSigAboveNumDist() const;
    StyleItem* timeSigAcrossNumDist() const;

    StyleItem* timeSigNormalY() const;
    StyleItem* timeSigAboveY() const;
    StyleItem* timeSigAcrossY() const;

    StyleItem* timeSigCenterOnBarline() const;
    StyleItem* timeSigHangIntoMargin() const;
    StyleItem* timeSigCenterAcrossStaveGroup() const;

    StyleItem* timeSigNormalScale() const;
    StyleItem* timeSigAboveScale() const;
    StyleItem* timeSigAcrossScale() const;
};
}
