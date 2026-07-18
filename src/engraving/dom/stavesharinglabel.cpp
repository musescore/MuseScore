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
#include "stavesharinglabel.h"

namespace mu::engraving {
static const ElementStyle STAVE_SHARING_LABEL_STYLE {
    { Sid::staveSharingLabelPlacement, Pid::PLACEMENT },
    { Sid::staveSharingLabelMinDistance, Pid::MIN_DISTANCE },
};

StaveSharingLabel::StaveSharingLabel(Segment* parent, TextStyleType tid)
    : StaffTextBase(ElementType::STAVE_SHARING_LABEL, parent, tid, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&STAVE_SHARING_LABEL_STYLE);
}
} // namespace mu::engraving
