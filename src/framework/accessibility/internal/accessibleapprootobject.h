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

#include <QObject>
#include <QList>
#include <QWindow>

#include "../iaccessibleapprootobject.h"
#include "accessibleobject.h"

class QAccessibleInterface;

namespace muse::accessibility {
class AccessibleAppRootObject : public QObject, public IAccessibleAppRootObject
{
    Q_OBJECT
public:
    AccessibleAppRootObject();
    ~AccessibleAppRootObject() override;

    static QAccessibleInterface* accessibleInterface(QObject* object);

    void init() override;

    QObject* asQObject() override;

    void registerWindow(QWindow* window, AccessibleObject* windowRoot) override;
    void unregisterWindow(QWindow* window) override;

    int windowCount() const override;
    QWindow* windowAt(int index) const override;
    AccessibleObject* windowRoot(QWindow* window) const override;
    QAccessibleInterface* windowIface(int index) const override;

    bool isAccessibilityActive() const override;

private:

    struct WindowEntry {
        QWindow* window = nullptr;
        AccessibleObject* windowRoot = nullptr;
    };

    QList<WindowEntry> m_windows;
    bool m_inited = false;
};
}
