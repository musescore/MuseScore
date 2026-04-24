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

#include "modularity/imoduleinterface.h"

class QAccessibleInterface;
class QObject;
class QWindow;

namespace muse::accessibility {
class AccessibleObject;

class IAccessibleAppRootObject : MODULE_GLOBAL_INTERFACE
{
    INTERFACE_ID(IAccessibleAppRootObject)
public:
    virtual ~IAccessibleAppRootObject() = default;

    virtual void init() = 0;

    virtual QObject* asQObject() = 0;

    virtual void registerWindow(QWindow* window, AccessibleObject* windowRoot) = 0;
    virtual void unregisterWindow(QWindow* window) = 0;

    virtual int windowCount() const = 0;
    virtual QWindow* windowAt(int index) const = 0;
    virtual AccessibleObject* windowRoot(QWindow* window) const = 0;
    virtual QAccessibleInterface* windowIface(int index) const = 0;

    virtual bool isAccessibilityActive() const = 0;
};
}
