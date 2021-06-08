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
#include "accessibleobject.h"

#include "accessibilitycontroller.h"

using namespace mu::accessibility;

AccessibleObject::AccessibleObject(IAccessible* item)
    : QObject()
{
    setObjectName("AccessibleObject");
    m_item = item;
}

void AccessibleObject::setController(std::shared_ptr<AccessibilityController> controller)
{
    m_controller = controller;
}

const std::shared_ptr<AccessibilityController>& AccessibleObject::controller() const
{
    return m_controller;
}

IAccessible* AccessibleObject::item() const
{
    return m_item;
}
