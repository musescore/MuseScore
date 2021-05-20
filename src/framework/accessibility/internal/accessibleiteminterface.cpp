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

//#define ACCESSIBILITY_LOGGING_ENABLED

#ifdef ACCESSIBILITY_LOGGING_ENABLED
#define MYLOG() LOGI()
#else
#define MYLOG() LOGN()
#endif

using namespace mu::accessibility;

AccessibleItemInterface::AccessibleItemInterface(AccessibleObject* object)
{
    m_object = object;
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
    return m_object->item()->accessibleRect();
}

QAccessibleInterface* AccessibleItemInterface::parent() const
{
    QAccessibleInterface* iface = m_object->controller()->parentIface(m_object->item());
    MYLOG() << "item: " << m_object->item()->accessibleName() << ", parent: " << (iface ? iface->text(QAccessible::Name) : "null");
    return iface;
}

int AccessibleItemInterface::childCount() const
{
    int count = m_object->controller()->childCount(m_object->item());
    MYLOG() << "item: " << m_object->item()->accessibleName() << ", childCount: " << count;
    return count;
}

QAccessibleInterface* AccessibleItemInterface::child(int index) const
{
    QAccessibleInterface* iface = m_object->controller()->child(m_object->item(), index);
    MYLOG() << "item: " << m_object->item()->accessibleName() << ", child: " << index << " " << iface->text(QAccessible::Name);
    return iface;
}

int AccessibleItemInterface::indexOfChild(const QAccessibleInterface* iface) const
{
    int idx = m_object->controller()->indexOfChild(m_object->item(), iface);
    MYLOG() << "item: " << m_object->item()->accessibleName() << ", indexOfChild: " << iface->text(QAccessible::Name) << " = " << idx;
    return idx;
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
    case IAccessibility::Role::Application: {
        state.active = item->accessibleState(IAccessibility::State::Active);
    } break;
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
    case IAccessibility::Role::Application: return QAccessible::Application;
    case IAccessibility::Role::Panel: return QAccessible::Pane;
    case IAccessibility::Role::Button: return QAccessible::Button;
    }
    return QAccessible::NoRole;
}

QString AccessibleItemInterface::text(QAccessible::Text textType) const
{
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
