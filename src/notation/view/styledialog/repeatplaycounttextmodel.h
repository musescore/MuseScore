/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
class RepeatPlayCountTextModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(StyleItem * repeatPlayCountShow READ repeatPlayCountShow CONSTANT)
    Q_PROPERTY(StyleItem * repeatPlayCountShowSingleRepeats READ repeatPlayCountShowSingleRepeats CONSTANT)
    Q_PROPERTY(StyleItem * repeatTextPreset READ repeatTextPreset CONSTANT)

public:
    explicit RepeatPlayCountTextModel(QObject* parent = nullptr);

    StyleItem* repeatTextPreset() const;
    Q_INVOKABLE QVariantList textPresetOptions() const;
    StyleItem* repeatPlayCountShow() const;
    StyleItem* repeatPlayCountShowSingleRepeats() const;
};
}
