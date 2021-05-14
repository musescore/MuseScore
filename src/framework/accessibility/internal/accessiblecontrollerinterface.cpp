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
#include "accessiblecontrollerinterface.h"

#include "accessibilitycontroller.h"

#include "log.h"

using namespace mu::accessibility;

AccessibleControllerInterface::AccessibleControllerInterface(QObject* o)
{
    m_controller = qobject_cast<AccessibilityController*>(o);
}

bool AccessibleControllerInterface::isValid() const
{
    return m_controller != nullptr;
}

QObject* AccessibleControllerInterface::object() const
{
    return m_controller;
}

QWindow* AccessibleControllerInterface::window() const
{
    return nullptr;
}

QRect AccessibleControllerInterface::rect() const
{
    return QRect();
}

QAccessibleInterface* AccessibleControllerInterface::parent() const
{
    return nullptr;
}

int AccessibleControllerInterface::childCount() const
{
    return m_controller->childCount(nullptr);
}

QAccessibleInterface* AccessibleControllerInterface::child(int i) const
{
    return m_controller->child(nullptr, i);
}

QAccessibleInterface* AccessibleControllerInterface::childAt(int, int) const
{
    NOT_IMPLEMENTED;
    return nullptr;
}

int AccessibleControllerInterface::indexOfChild(const QAccessibleInterface* iface) const
{
    return m_controller->indexOfChild(nullptr, iface);
}

QAccessible::Role AccessibleControllerInterface::role() const
{
    return QAccessible::Application;
}

QAccessible::State AccessibleControllerInterface::state() const
{
    QAccessible::State state;
    state.active = 1;
    return state;
}

QString AccessibleControllerInterface::text(QAccessible::Text) const
{
    return QString();
}

void AccessibleControllerInterface::setText(QAccessible::Text, const QString&)
{
    NOT_IMPLEMENTED;
}
