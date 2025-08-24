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

#include "repeatplaycounttextmodel.h"

using namespace mu::notation;

RepeatPlayCountTextModel::RepeatPlayCountTextModel(QObject* parent)
    : AbstractStyleDialogModel(parent, {
    StyleId::repeatPlayCountPreset,
    StyleId::repeatPlayCountShow,
    StyleId::repeatPlayCountShowSingleRepeats
})
{
}

StyleItem* RepeatPlayCountTextModel::repeatTextPreset() const
{
    return styleItem(StyleId::repeatPlayCountPreset);
}

QVariantList RepeatPlayCountTextModel::textPresetOptions() const
{
    QVariantList options {
        QVariantMap{
            { "text", muse::qtrc("notation/editstyle/repeatplaycount", "x3") },
            { "value", mu::engraving::RepeatPlayCountPreset::X_N } },
        QVariantMap{
            { "text", muse::qtrc("notation/editstyle/repeatplaycount", "3x") },
            { "value", mu::engraving::RepeatPlayCountPreset::N_X } },
        QVariantMap{
            { "text", muse::qtrc("notation/editstyle/repeatplaycount", "Play 3 times") },
            { "value", mu::engraving::RepeatPlayCountPreset::PLAY_N_TIMES } },
        QVariantMap{
            { "text", muse::qtrc("notation/editstyle/repeatplaycount", "3 repeats") },
            { "value", mu::engraving::RepeatPlayCountPreset::N_REPEATS } },
    };

    return options;
}

StyleItem* RepeatPlayCountTextModel::repeatPlayCountShow() const
{
    return styleItem(StyleId::repeatPlayCountShow);
}

StyleItem* RepeatPlayCountTextModel::repeatPlayCountShowSingleRepeats() const
{
    return styleItem(StyleId::repeatPlayCountShowSingleRepeats);
}
