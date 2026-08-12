/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
class StaveSharingPageModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    QML_ELEMENT

    Q_PROPERTY(
        bool isStaveSharingEnabled READ isStaveSharingEnabled WRITE setIsStaveSharingEnabled NOTIFY isStaveSharingEnabledChanged FINAL)

    Q_PROPERTY(StyleItem * allowVoiceCrossing READ allowVoiceCrossing CONSTANT)
    Q_PROPERTY(StyleItem * textForUnisonLabel READ textForUnisonLabel CONSTANT)
    Q_PROPERTY(StyleItem * sharedOnStaffNumeralsFollowInstrumentNumerals READ sharedOnStaffNumeralsFollowInstrumentNumerals CONSTANT)
    Q_PROPERTY(StyleItem * sharedOnStaffNumeralsTrailingDotSingle READ sharedOnStaffNumeralsTrailingDotSingle CONSTANT)
    Q_PROPERTY(StyleItem * sharedOnStaffNumeralsTrailingDotMultiple READ sharedOnStaffNumeralsTrailingDotMultiple CONSTANT)
    Q_PROPERTY(StyleItem * sharedOnStaffNumeralsHyphenEnable READ sharedOnStaffNumeralsHyphenEnable CONSTANT)
    Q_PROPERTY(StyleItem * sharedOnStaffNumeralsHyphenThreshold READ sharedOnStaffNumeralsHyphenThreshold CONSTANT)
    Q_PROPERTY(StyleItem * unisonLabelRestateOnNewSystem READ unisonLabelRestateOnNewSystem CONSTANT)

public:
    explicit StaveSharingPageModel(QObject* parent = nullptr);

    bool isStaveSharingEnabled() const;
    void setIsStaveSharingEnabled(bool v);

signals:
    void isStaveSharingEnabledChanged(bool enabled);

public:
    StyleItem* allowVoiceCrossing() const;
    StyleItem* textForUnisonLabel() const;
    StyleItem* sharedOnStaffNumeralsFollowInstrumentNumerals() const;
    StyleItem* sharedOnStaffNumeralsTrailingDotSingle() const;
    StyleItem* sharedOnStaffNumeralsTrailingDotMultiple() const;
    StyleItem* sharedOnStaffNumeralsHyphenEnable() const;
    StyleItem* sharedOnStaffNumeralsHyphenThreshold() const;
    StyleItem* unisonLabelRestateOnNewSystem() const;
};
}
