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
#include "accessibleiteminterface.h"

#include <QWindow>

#include "accessibilitycontroller.h"

#include "log.h"

using namespace mu::accessibility;

AccessibleItemInterface::AccessibleItemInterface(QObject* object)
{
    m_object = qobject_cast<AccessibleObject*>(object);
}

bool AccessibleItemInterface::isValid() const
{
    return m_object != nullptr;
}

QObject* AccessibleItemInterface::object() const
{
    return m_object;
}

QWindow* AccessibleItemInterface::window() const
{
    //! TODO Need to add a current window
    return mainWindow()->qWindow();
}

QRect AccessibleItemInterface::rect() const
{
    NOT_IMPLEMENTED;
    return QRect();
}

QAccessibleInterface* AccessibleItemInterface::parent() const
{
    return QAccessible::queryAccessibleInterface(m_object->parent());
}

int AccessibleItemInterface::childCount() const
{
    return m_object->controller()->childCount(m_object->item());
}

QAccessibleInterface* AccessibleItemInterface::child(int index) const
{
    return m_object->controller()->child(m_object->item(), index);
}

int AccessibleItemInterface::indexOfChild(const QAccessibleInterface* iface) const
{
    return m_object->controller()->indexOfChild(m_object->item(), iface);
}

QAccessibleInterface* AccessibleItemInterface::childAt(int, int) const
{
    NOT_IMPLEMENTED;
    return nullptr;
}

QAccessibleInterface* AccessibleItemInterface::focusChild() const
{
    NOT_IMPLEMENTED;
    return nullptr;
}

QAccessible::State AccessibleItemInterface::state() const
{
    IAccessibility* item = m_object->item();
    QAccessible::State state;
    state.invisible = false;
    state.invalid = false;
    state.disabled = item->accessibleState(IAccessibility::State::Disabled);

    IAccessibility::Role r = m_object->item()->accessibleRole();
    switch (r) {
    case IAccessibility::Role::NoRole: break;
    case IAccessibility::Role::Panel: {
        state.active = item->accessibleState(IAccessibility::State::Active);
    } break;
    case IAccessibility::Role::Button: {
        state.focusable = true;
        state.focused = item->accessibleState(IAccessibility::State::Focused);
    } break;
    }

    return state;
}

QAccessible::Role AccessibleItemInterface::role() const
{
    IAccessibility::Role r = m_object->item()->accessibleRole();
    switch (r) {
    case IAccessibility::Role::NoRole: return QAccessible::NoRole;
    case IAccessibility::Role::Panel: return QAccessible::Pane;
    case IAccessibility::Role::Button: return QAccessible::Button;
    }
    return QAccessible::NoRole;
}

QString AccessibleItemInterface::text(QAccessible::Text textType) const
{
    return m_object->item()->accessibleName();

    switch (textType) {
    case QAccessible::Name: return m_object->item()->accessibleName();
    default: break;
    }

    return QString();
}

void AccessibleItemInterface::setText(QAccessible::Text, const QString&)
{
    NOT_IMPLEMENTED;
}

void* AccessibleItemInterface::interface_cast(QAccessible::InterfaceType)
{
    //! NOTE Not implemented
    return nullptr;
}
