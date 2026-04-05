/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <gmock/gmock.h>

#include "framework/accessibility/iaccessibleapprootobject.h"

namespace muse::accessibility {
class AccessibleAppRootObjectMock : public IAccessibleAppRootObject
{
public:
    MOCK_METHOD(void, init, (), (override));
    MOCK_METHOD(QObject*, asQObject, (), (override));
    MOCK_METHOD(void, registerWindow, (QWindow*, AccessibleObject*), (override));
    MOCK_METHOD(void, unregisterWindow, (QWindow*), (override));
    MOCK_METHOD(int, windowCount, (), (const, override));
    MOCK_METHOD(QWindow*, windowAt, (int), (const, override));
    MOCK_METHOD(AccessibleObject*, windowRoot, (QWindow*), (const, override));
    MOCK_METHOD(QAccessibleInterface*, windowIface, (int), (const, override));
    MOCK_METHOD(bool, isAccessibilityActive, (), (const, override));
};
}
