/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
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

#include "stavesharingpagemodel.h"

#include "notation/inotationparts.h" // IWYU pragma: keep

using namespace mu::engraving;

namespace mu::notation {
StaveSharingPageModel::StaveSharingPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, { StyleId::enableStaveSharing,
                                         StyleId::allowVoiceCrossing,
                               })
{
}

bool StaveSharingPageModel::isStaveSharingEnabled() const
{
    bool val = styleItem(StyleId::enableStaveSharing)->value().toBool();
    return val;
}

void StaveSharingPageModel::setIsStaveSharingEnabled(bool v)
{
    styleItem(StyleId::enableStaveSharing)->setValue(v);

    context.get()->currentNotation()->parts()->toggleStaveSharing(v);

    emit isStaveSharingEnabledChanged(v);
}

StyleItem* StaveSharingPageModel::allowVoiceCrossing() const
{
    return styleItem(StyleId::allowVoiceCrossing);
}
}
