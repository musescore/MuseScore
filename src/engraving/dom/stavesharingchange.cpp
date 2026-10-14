/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
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

#include "stavesharingchange.h"

#include "part.h"
#include "segment.h"
#include "sharedpart.h"

using namespace mu::engraving;

static const ElementStyle staveSharingChangeStyle {
    { Sid::staveSharingChangeMinDistance, Pid::MIN_DISTANCE },
};

StaveSharingChange::StaveSharingChange(DummyParentOr<Segment> parent)
    : IndicatorIcon(ElementType::STAVE_SHARING_CHANGE, parent, ElementFlag::GENERATED | ElementFlag::PLACE_ABOVE | ElementFlag::ON_STAFF)
{
    initElementStyle(&staveSharingChangeStyle);
}

char16_t StaveSharingChange::iconCode() const
{
    if (staveSharingState() && staveSharingState()->isReset()) {
        return 0xEF19;
    }

    if (staveSharingState() && !staveSharingState()->isUserMapValid()) {
        return 0xF3CE;
    }

    // TODO: placeholder icon
    return 0xF499;
}

const StaveSharingState* StaveSharingChange::staveSharingState() const
{
    SharedPart* sharedPart = part() && part()->isSharedPart() ? toSharedPart(part()) : nullptr;

    if (!sharedPart) {
        return nullptr;
    }

    return &sharedPart->staveSharingStateAtTick(tick());
}
