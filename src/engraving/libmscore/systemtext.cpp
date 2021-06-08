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

#include "systemtext.h"

using namespace mu;

namespace Ms {
//---------------------------------------------------------
//   systemStyle
//---------------------------------------------------------

static const ElementStyle systemStyle {
    { Sid::systemTextPlacement,                Pid::PLACEMENT },
    { Sid::systemTextMinDistance,              Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   SystemText
//---------------------------------------------------------

SystemText::SystemText(Score* s, Tid tid)
    : StaffTextBase(s, tid, ElementFlag::SYSTEM | ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&systemStyle);
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant SystemText::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::SUB_STYLE:
        return int(Tid::SYSTEM);
    default:
        return TextBase::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void SystemText::layout()
{
    TextBase::layout();
    autoplaceSegmentElement();
}
} // namespace Ms
