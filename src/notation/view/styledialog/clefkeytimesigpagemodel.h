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
class ClefKeyTimeSigPageModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(StyleItem * genClef READ genClef CONSTANT)
    Q_PROPERTY(StyleItem * hideTabClefAfterFirst READ hideTabClefAfterFirst CONSTANT)
    Q_PROPERTY(StyleItem * genCourtesyClef READ genCourtesyClef CONSTANT)
    Q_PROPERTY(StyleItem * tabClef READ tabClef CONSTANT)

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
    Q_PROPERTY(StyleItem * timeSigVSMarginCentered READ timeSigVSMarginCentered CONSTANT)
    Q_PROPERTY(StyleItem * timeSigVSMarginNonCentered READ timeSigVSMarginNonCentered CONSTANT)
    Q_PROPERTY(StyleItem * timeSigCenterAcrossStaveGroup READ timeSigCenterAcrossStaveGroup CONSTANT)

    Q_PROPERTY(StyleItem * timeSigNormalScale READ timeSigNormalScale CONSTANT)
    Q_PROPERTY(StyleItem * timeSigAboveScale READ timeSigAboveScale CONSTANT)
    Q_PROPERTY(StyleItem * timeSigAcrossScale READ timeSigAcrossScale CONSTANT)

    Q_PROPERTY(StyleItem * timeSigNormalScaleLock READ timeSigNormalScaleLock CONSTANT)
    Q_PROPERTY(StyleItem * timeSigAboveScaleLock READ timeSigAboveScaleLock CONSTANT)
    Q_PROPERTY(StyleItem * timeSigAcrossScaleLock READ timeSigAcrossScaleLock CONSTANT)

    Q_PROPERTY(StyleItem * genCourtesyTimesig READ genCourtesyTimesig CONSTANT)

    Q_PROPERTY(StyleItem * genKeysig READ genKeysig CONSTANT)
    Q_PROPERTY(StyleItem * genCourtesyKeysig READ genCourtesyKeysig CONSTANT)

public:
    explicit ClefKeyTimeSigPageModel(QObject* parent = nullptr);

    StyleItem* genClef() const;
    StyleItem* hideTabClefAfterFirst() const;
    StyleItem* genCourtesyClef() const;
    StyleItem* tabClef() const;

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
    StyleItem* timeSigVSMarginCentered() const;
    StyleItem* timeSigVSMarginNonCentered() const;
    StyleItem* timeSigCenterAcrossStaveGroup() const;

    StyleItem* timeSigNormalScale() const;
    StyleItem* timeSigAboveScale() const;
    StyleItem* timeSigAcrossScale() const;

    StyleItem* timeSigNormalScaleLock() const;
    StyleItem* timeSigAboveScaleLock() const;
    StyleItem* timeSigAcrossScaleLock() const;

    Q_INVOKABLE void resetStyleAndSize() const;

    StyleItem* genCourtesyTimesig() const;

    StyleItem* genKeysig() const;
    StyleItem* genCourtesyKeysig() const;
};
}
