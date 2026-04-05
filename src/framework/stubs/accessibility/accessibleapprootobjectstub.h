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

#include "accessibility/iaccessibleapprootobject.h"

namespace muse::accessibility {
class AccessibleAppRootObjectStub : public IAccessibleAppRootObject
{
public:
    void init() override {}
    QObject* asQObject() override { return nullptr; }
    void registerWindow(QWindow*, AccessibleObject*) override {}
    void unregisterWindow(QWindow*) override {}
    int windowCount() const override { return 0; }
    QWindow* windowAt(int) const override { return nullptr; }
    AccessibleObject* windowRoot(QWindow*) const override { return nullptr; }
    QAccessibleInterface* windowIface(int) const override { return nullptr; }
    bool isAccessibilityActive() const override { return false; }
};
}
