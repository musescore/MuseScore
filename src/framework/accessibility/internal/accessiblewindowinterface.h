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
#ifndef MU_ACCESSIBILITY_ACCESSIBLEWINDOWINTERFACE_H
#define MU_ACCESSIBILITY_ACCESSIBLEWINDOWINTERFACE_H

#include <QAccessibleInterface>

#include "accessibleobject.h"

#include "modularity/ioc.h"
#include "ui/iinteractiveprovider.h"

namespace mu::accessibility {
class AccessibleWindowInterface : public QAccessibleInterface
{
public:
    AccessibleWindowInterface(QObject* window, AccessibleObject* children);

    bool isValid() const override;
    QObject* object() const override;
    QWindow* window() const override;
    QRect rect() const override;

    QAccessibleInterface* focusChild() const override;
    QAccessibleInterface* childAt(int x, int y) const override;

    QAccessibleInterface* parent() const override;
    QAccessibleInterface* child(int index) const override;
    int childCount() const override;
    int indexOfChild(const QAccessibleInterface* iface) const override;

    QAccessible::State state() const override;
    QAccessible::Role role() const override;
    QString text(QAccessible::Text) const override;
    void setText(QAccessible::Text, const QString& text) override;
protected:
    void* interface_cast(QAccessible::InterfaceType t) override;

private:
    QWindow* m_window = nullptr;
    AccessibleObject* m_children = nullptr;
};
}

#endif // MU_ACCESSIBILITY_ACCESSIBLEWINDOWINTERFACE_H
