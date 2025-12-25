/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

    Q_PROPERTY(mu::notation::StyleItem * genClef READ genClef CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * hideTabClefAfterFirst READ hideTabClefAfterFirst CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * genCourtesyClef READ genCourtesyClef CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * tabClef READ tabClef CONSTANT)

    Q_PROPERTY(mu::notation::StyleItem * timeSigPlacement READ timeSigPlacement CONSTANT)

    Q_PROPERTY(mu::notation::StyleItem * timeSigNormalStyle READ timeSigNormalStyle CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * timeSigAboveStyle READ timeSigAboveStyle CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * timeSigAcrossStyle READ timeSigAcrossStyle CONSTANT)

    Q_PROPERTY(mu::notation::StyleItem * timeSigNormalNumDist READ timeSigNormalNumDist CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * timeSigAboveNumDist READ timeSigAboveNumDist CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * timeSigAcrossNumDist READ timeSigAcrossNumDist CONSTANT)

    Q_PROPERTY(mu::notation::StyleItem * timeSigNormalY READ timeSigNormalY CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * timeSigAboveY READ timeSigAboveY CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * timeSigAcrossY READ timeSigAcrossY CONSTANT)

    Q_PROPERTY(mu::notation::StyleItem * timeSigCenterOnBarline READ timeSigCenterOnBarline CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * timeSigVSMarginCentered READ timeSigVSMarginCentered CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * timeSigVSMarginNonCentered READ timeSigVSMarginNonCentered CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * timeSigCenterAcrossStaveGroup READ timeSigCenterAcrossStaveGroup CONSTANT)

    Q_PROPERTY(mu::notation::StyleItem * timeSigNormalScale READ timeSigNormalScale CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * timeSigAboveScale READ timeSigAboveScale CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * timeSigAcrossScale READ timeSigAcrossScale CONSTANT)

    Q_PROPERTY(mu::notation::StyleItem * timeSigNormalScaleLock READ timeSigNormalScaleLock CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * timeSigAboveScaleLock READ timeSigAboveScaleLock CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * timeSigAcrossScaleLock READ timeSigAcrossScaleLock CONSTANT)

    Q_PROPERTY(mu::notation::StyleItem * genCourtesyTimesig READ genCourtesyTimesig CONSTANT)

    Q_PROPERTY(mu::notation::StyleItem * genKeysig READ genKeysig CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * genCourtesyKeysig READ genCourtesyKeysig CONSTANT)

    Q_PROPERTY(mu::notation::StyleItem * changesBeforeBarlineRepeats READ changesBeforeBarlineRepeats CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * changesBeforeBarlineOtherJumps READ changesBeforeBarlineOtherJumps CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * placeClefsBeforeRepeats READ placeClefsBeforeRepeats CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * changesBetweenEndStartRepeat READ changesBetweenEndStartRepeat CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * showCourtesiesRepeats READ showCourtesiesRepeats CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * useParensRepeatCourtesies READ useParensRepeatCourtesies CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * showCourtesiesOtherJumps READ showCourtesiesOtherJumps CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * useParensOtherJumpCourtesies READ useParensOtherJumpCourtesies CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * showCourtesiesAfterCancellingRepeats READ showCourtesiesAfterCancellingRepeats CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * useParensRepeatCourtesiesAfterCancelling READ useParensRepeatCourtesiesAfterCancelling CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * showCourtesiesAfterCancellingOtherJumps READ showCourtesiesAfterCancellingOtherJumps CONSTANT)
    Q_PROPERTY(
        mu::notation::StyleItem * useParensOtherJumpCourtesiesAfterCancelling READ useParensOtherJumpCourtesiesAfterCancelling CONSTANT)

    Q_PROPERTY(mu::notation::StyleItem * smallParens READ smallParens CONSTANT)

    Q_PROPERTY(mu::notation::StyleItem * keySigNaturals READ keySigNaturals CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * keySigShowNaturalsChangingSharpsFlats READ keySigShowNaturalsChangingSharpsFlats CONSTANT)

    QML_ELEMENT

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

    StyleItem* changesBeforeBarlineRepeats() const;
    StyleItem* changesBeforeBarlineOtherJumps() const;
    StyleItem* placeClefsBeforeRepeats() const;
    StyleItem* changesBetweenEndStartRepeat() const;
    StyleItem* showCourtesiesRepeats() const;
    StyleItem* useParensRepeatCourtesies() const;
    StyleItem* showCourtesiesOtherJumps() const;
    StyleItem* useParensOtherJumpCourtesies() const;
    StyleItem* showCourtesiesAfterCancellingRepeats() const;
    StyleItem* useParensRepeatCourtesiesAfterCancelling() const;
    StyleItem* showCourtesiesAfterCancellingOtherJumps() const;
    StyleItem* useParensOtherJumpCourtesiesAfterCancelling() const;

    StyleItem* smallParens() const;

    StyleItem* keySigNaturals() const;
    StyleItem* keySigShowNaturalsChangingSharpsFlats() const;
};
}
