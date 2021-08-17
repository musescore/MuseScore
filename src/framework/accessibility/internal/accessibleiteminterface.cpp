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

#undef MYLOG
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
    //! NOTE Not worked at the moment
//    QWindow* w = m_object->item()->accessibleWindow();
//    if (w) {
//        return w;
//    }
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
    IAccessible* item = m_object->item();
    QAccessible::State state;
    state.invisible = false;
    state.invalid = false;
    state.disabled = !item->accessibleState(IAccessible::State::Enabled);

    if (state.disabled) {
        return state;
    }

    IAccessible::Role r = m_object->item()->accessibleRole();
    switch (r) {
    case IAccessible::Role::NoRole: break;
    case IAccessible::Role::Application: {
        state.active = true;
    } break;
    case IAccessible::Role::Dialog: {
        state.active = item->accessibleState(IAccessible::State::Active);
    } break;
    case IAccessible::Role::Panel: {
        state.active = item->accessibleState(IAccessible::State::Active);
    } break;
    case IAccessible::Role::Button: {
        state.focusable = true;
        state.focused = item->accessibleState(IAccessible::State::Focused);
    } break;
    case IAccessible::Role::RadioButton: {
        state.focusable = true;
        state.focused = item->accessibleState(IAccessible::State::Focused);

        state.checkable = true;
        state.checked = item->accessibleState(IAccessible::State::Selected);
    } break;
    case IAccessible::Role::EditableText: {
        state.focusable = true;
        state.focused = item->accessibleState(IAccessible::State::Focused);
    } break;
    case IAccessible::Role::StaticText: {
        state.focusable = true;
        state.focused = item->accessibleState(IAccessible::State::Focused);
    } break;
    case IAccessible::Role::ListItem: {
        state.focusable = true;
        state.focused = item->accessibleState(IAccessible::State::Focused);
    } break;
    case IAccessible::Role::Information: {
        state.focusable = true;
        state.focused = item->accessibleState(IAccessible::State::Focused);
    } break;
    case IAccessible::Role::ElementOnScore: {
        state.focusable = true;
        state.focused = item->accessibleState(IAccessible::State::Focused);

//        state.checkable = true;
//        state.checked = item->accessibleState(IAccessible::State::Selected);
    } break;
    default: {
        LOGW() << "not handled role: " << static_cast<int>(r);
    } break;
    }

    return state;
}

QAccessible::Role AccessibleItemInterface::role() const
{
    IAccessible::Role r = m_object->item()->accessibleRole();
    switch (r) {
    case IAccessible::Role::NoRole: return QAccessible::NoRole;
    case IAccessible::Role::Application: return QAccessible::Application;
    case IAccessible::Role::Dialog: return QAccessible::Dialog;
    case IAccessible::Role::Panel: return QAccessible::Pane;
    case IAccessible::Role::StaticText: return QAccessible::StaticText;
    case IAccessible::Role::EditableText: return QAccessible::EditableText;
    case IAccessible::Role::Button: return QAccessible::Button;
    case IAccessible::Role::CheckBox: return QAccessible::CheckBox;
    case IAccessible::Role::RadioButton: return QAccessible::RadioButton;
    case IAccessible::Role::ComboBox: return QAccessible::ComboBox;
    case IAccessible::Role::ListItem: return QAccessible::ListItem;
    case IAccessible::Role::Information: {
#ifdef Q_OS_WIN
        return QAccessible::StaticText;
#else
        return QAccessible::UserRole;
#endif
    }
    case IAccessible::Role::ElementOnScore: {
#ifdef Q_OS_WIN
        return QAccessible::StaticText;
#else
        return QAccessible::UserRole;
#endif
    }
    }

    LOGE() << "not handled role: " << static_cast<int>(r);
    return QAccessible::NoRole;
}

QString AccessibleItemInterface::text(QAccessible::Text textType) const
{
    switch (textType) {
    case QAccessible::Name: return m_object->item()->accessibleName();
    case QAccessible::Description: return m_object->item()->accessibleDescription();
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
