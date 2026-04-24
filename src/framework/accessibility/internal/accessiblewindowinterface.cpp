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

#include "accessiblewindowinterface.h"

#include <QWindow>

#include "accessibilitycontroller.h"

#include "modularity/ioc.h"
#include "../iaccessibleapprootobject.h"

#include "translation.h"
#include "log.h"

//#define MUE_ENABLE_ACCESSIBILITY_TRACE

#undef MYLOG
#ifdef MUE_ENABLE_ACCESSIBILITY_TRACE
#define MYLOG() LOGI()
#else
#define MYLOG() LOGN()
#endif

using namespace muse::accessibility;

AccessibleWindowInterface::AccessibleWindowInterface(QObject* window, AccessibleObject* children)
    : m_children(children)
{
    m_window = qobject_cast<QWindow*>(window);
}

bool AccessibleWindowInterface::isValid() const
{
    return m_window != nullptr;
}

QObject* AccessibleWindowInterface::object() const
{
    return m_window;
}

QWindow* AccessibleWindowInterface::window() const
{
    return m_window;
}

QRect AccessibleWindowInterface::rect() const
{
    if (!m_window) {
        return {};
    }
    return QRect(m_window->x(), m_window->y(), m_window->width(), m_window->height());
}

QAccessibleInterface* AccessibleWindowInterface::parent() const
{
    auto appRoot = muse::modularity::globalIoc()->resolve<IAccessibleAppRootObject>("accessibility");
    if (m_window && appRoot && appRoot->windowRoot(m_window)) {
        return QAccessible::queryAccessibleInterface(appRoot->asQObject());
    }
    return nullptr;
}

int AccessibleWindowInterface::childCount() const
{
    if (!m_children) {
        return 0;
    }

    auto controller = m_children->controller().lock();
    if (!controller) {
        return 0;
    }

    int count = controller->childCount(m_children->item());
    MYLOG() << "item: " << m_children->item()->accessibleName() << ", childCount: " << count;
    return count;
}

QAccessibleInterface* AccessibleWindowInterface::child(int index) const
{
    if (!m_children) {
        return nullptr;
    }

    auto controller = m_children->controller().lock();
    if (!controller) {
        return nullptr;
    }

    QAccessibleInterface* iface = controller->child(m_children->item(), index);
    MYLOG() << "item: " << m_children->item()->accessibleName() << ", child: " << index << " " <<
        (iface ? iface->text(QAccessible::Name) : "null");
    return iface;
}

int AccessibleWindowInterface::indexOfChild(const QAccessibleInterface* iface) const
{
    if (!m_children) {
        return -1;
    }

    auto controller = m_children->controller().lock();
    if (!controller) {
        return -1;
    }

    int idx = controller->indexOfChild(m_children->item(), iface);
    MYLOG() << "item: " << m_children->item()->accessibleName() << ", indexOfChild: " <<
        (iface ? iface->text(QAccessible::Name) : "null") << " = " << idx;
    return idx;
}

QAccessibleInterface* AccessibleWindowInterface::childAt(int, int) const
{
    NOT_IMPLEMENTED;
    return nullptr;
}

QAccessibleInterface* AccessibleWindowInterface::focusChild() const
{
    if (!m_children) {
        return nullptr;
    }

    auto controller = m_children->controller().lock();
    if (!controller) {
        return nullptr;
    }

    QAccessibleInterface* child = controller->focusedChild(m_children->item());
    MYLOG() << "item: " << m_children->item()->accessibleName() << ", focused child: " << (child ? child->text(QAccessible::Name) : "null");
    return child;
}

QAccessible::State AccessibleWindowInterface::state() const
{
    QAccessible::State st;
    st.active = true;
    return st;
}

QAccessible::Role AccessibleWindowInterface::role() const
{
    return QAccessible::Window;
}

QString AccessibleWindowInterface::text(QAccessible::Text) const
{
    if (!m_window) {
        return {};
    }
    return m_window->title();
}

void AccessibleWindowInterface::setText(QAccessible::Text, const QString&)
{
    NOT_IMPLEMENTED;
}

void* AccessibleWindowInterface::interface_cast(QAccessible::InterfaceType)
{
    //! NOTE Not implemented
    return nullptr;
}
