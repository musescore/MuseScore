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
    Q_PROPERTY(StyleItem * trailingDotOnMarginLabelsSingle READ trailingDotOnMarginLabelsSingle CONSTANT)
    Q_PROPERTY(StyleItem * trailingDotOnInStaffLabelsSingle READ trailingDotOnInStaffLabelsSingle CONSTANT)
    Q_PROPERTY(StyleItem * trailingDotOnMarginLabelsMultiple READ trailingDotOnMarginLabelsMultiple CONSTANT)
    Q_PROPERTY(StyleItem * trailingDotOnInStaffLabelsMultiple READ trailingDotOnInStaffLabelsMultiple CONSTANT)
    Q_PROPERTY(StyleItem * twoInstrLabelAlign READ twoInstrLabelAlign CONSTANT)
    Q_PROPERTY(StyleItem * compressWithHyphenMoreThan READ compressWithHyphenMoreThan CONSTANT)

public:
    explicit StaveSharingPageModel(QObject* parent = nullptr);

    bool isStaveSharingEnabled() const;
    void setIsStaveSharingEnabled(bool v);

signals:
    void isStaveSharingEnabledChanged(bool enabled);

public:
    StyleItem* allowVoiceCrossing() const;
    StyleItem* trailingDotOnMarginLabelsSingle() const;
    StyleItem* trailingDotOnInStaffLabelsSingle() const;
    StyleItem* trailingDotOnMarginLabelsMultiple() const;
    StyleItem* trailingDotOnInStaffLabelsMultiple() const;
    StyleItem* twoInstrLabelAlign() const;
    StyleItem* compressWithHyphenMoreThan() const;
};
}
