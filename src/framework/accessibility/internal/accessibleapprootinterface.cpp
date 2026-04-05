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
#include "accessibleapprootinterface.h"

#include <QAccessible>
#include <QGuiApplication>

#include "accessibleapprootobject.h"

using namespace muse::accessibility;

AccessibleAppRootInterface::AccessibleAppRootInterface(AccessibleAppRootObject* root)
    : m_root(root)
{
}

bool AccessibleAppRootInterface::isValid() const
{
    return m_root != nullptr;
}

QObject* AccessibleAppRootInterface::object() const
{
    return m_root;
}

QWindow* AccessibleAppRootInterface::window() const
{
    return nullptr;
}

QRect AccessibleAppRootInterface::rect() const
{
    return {};
}

QAccessibleInterface* AccessibleAppRootInterface::parent() const
{
    return nullptr;
}

int AccessibleAppRootInterface::childCount() const
{
    return m_root ? m_root->windowCount() : 0;
}

QAccessibleInterface* AccessibleAppRootInterface::child(int index) const
{
    if (!m_root) {
        return nullptr;
    }

    return m_root->windowIface(index);
}

int AccessibleAppRootInterface::indexOfChild(const QAccessibleInterface* iface) const
{
    if (!m_root || !iface) {
        return -1;
    }

    QWindow* childWindow = iface->window();
    for (int i = 0; i < m_root->windowCount(); ++i) {
        if (m_root->windowAt(i) == childWindow) {
            return i;
        }
    }

    return -1;
}

QAccessibleInterface* AccessibleAppRootInterface::focusChild() const
{
    if (!m_root) {
        return nullptr;
    }

    QWindow* w = qApp->focusWindow();
    while (w) {
        for (int i = 0; i < m_root->windowCount(); ++i) {
            if (m_root->windowAt(i) == w) {
                QAccessibleInterface* windowIface = child(i);
                if (windowIface) {
                    QAccessibleInterface* focusedChild = windowIface->focusChild();
                    return focusedChild ? focusedChild : windowIface;
                }
                return nullptr;
            }
        }
        w = w->transientParent();
    }

    return nullptr;
}

QAccessibleInterface* AccessibleAppRootInterface::childAt(int, int) const
{
    return nullptr;
}

QAccessible::State AccessibleAppRootInterface::state() const
{
    QAccessible::State st;
    st.active = true;
    return st;
}

QAccessible::Role AccessibleAppRootInterface::role() const
{
    return QAccessible::Application;
}

QString AccessibleAppRootInterface::text(QAccessible::Text t) const
{
    if (t == QAccessible::Name) {
        return QGuiApplication::applicationName();
    }
    return {};
}

void AccessibleAppRootInterface::setText(QAccessible::Text, const QString&)
{
}

void* AccessibleAppRootInterface::interface_cast(QAccessible::InterfaceType)
{
    return nullptr;
}
