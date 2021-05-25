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

#include "qmlaccessible.h"

#include <QQuickWindow>

#include "log.h"

using namespace mu::ui;
using namespace mu::accessibility;

AccessibleItem::AccessibleItem(QObject* parent)
    : QObject(parent)
{
}

AccessibleItem::~AccessibleItem()
{
    if (m_accessibleParent) {
        m_accessibleParent->removeChild(this);
    }

    if (m_registred) {
        accessibilityController()->unreg(this);
        m_registred = false;
    }
}

void AccessibleItem::classBegin()
{
}

void AccessibleItem::componentComplete()
{
    accessibilityController()->reg(this);
    m_registred = true;
}

bool AccessibleItem::event(QEvent* event)
{
    if (event->type() == QEvent::ParentChange) {
        LOGI() << "parent: " << parent();
    }

    return QObject::event(event);
}

const IAccessible* AccessibleItem::accessibleRoot() const
{
    return accessibilityController()->rootItem();
}

const IAccessible* AccessibleItem::accessibleParent() const
{
    if (m_accessibleParent) {
        return static_cast<const IAccessible*>(m_accessibleParent);
    }

    return accessibleRoot();
}

mu::async::Notification AccessibleItem::accessibleParentChanged() const
{
    return m_accessibleParentChanged;
}

IAccessible::Role AccessibleItem::accessibleRole() const
{
    return static_cast<IAccessible::Role>(m_role);
}

QString AccessibleItem::accessibleName() const
{
    return m_name;
}

bool AccessibleItem::accessibleState(State st) const
{
    return m_state.value(st, false);
}

void AccessibleItem::addChild(AccessibleItem* item)
{
    m_children.append(item);
}

void AccessibleItem::removeChild(AccessibleItem* item)
{
    m_children.removeOne(item);
}

size_t AccessibleItem::accessibleChildCount() const
{
    return static_cast<size_t>(m_children.size());
}

const IAccessible* AccessibleItem::accessibleChild(size_t i) const
{
    return static_cast<const IAccessible*>(m_children.value(static_cast<int>(i), nullptr));
}

QQuickItem* AccessibleItem::resolveVisualItem() const
{
    if (m_visualItem) {
        return m_visualItem;
    }

    QObject* prn = parent();
    while (prn) {
        QQuickItem* vitem = qobject_cast<QQuickItem*>(prn);
        if (vitem) {
            return vitem;
        }
        prn = prn->parent();
    }

    return nullptr;
}

QRect AccessibleItem::accessibleRect() const
{
    QQuickItem* vitem = resolveVisualItem();
    if (!vitem || !vitem->window()) {
        return QRect();
    }

    QPointF scenePos = vitem->mapToScene(QPointF(0, 0));
    QPoint globalPos = vitem->window()->mapToGlobal(scenePos.toPoint());
    return QRect(globalPos.x(), globalPos.y(), vitem->width(), vitem->height());
}

QWindow* AccessibleItem::accessibleWindow() const
{
    QQuickItem* vitem = resolveVisualItem();
    if (!vitem || !vitem->window()) {
        return nullptr;
    }
    return vitem->window();
}

void AccessibleItem::setAccessibleParent(AccessibleItem* p)
{
    if (m_accessibleParent) {
        m_accessibleParent->removeChild(this);
    }

    m_accessibleParent = p;

    if (m_accessibleParent) {
        m_accessibleParent->addChild(this);
    }
}

void AccessibleItem::setState(accessibility::IAccessible::State st, bool arg)
{
    if (m_state.value(st, false) == arg) {
        return;
    }

    m_state[st] = arg;
    emit stateChanged();

    if (m_registred) {
        accessibilityController()->stateChanged(this, st, arg);
    }
}

void AccessibleItem::setRole(QAccessible::Role role)
{
    if (m_role == role) {
        return;
    }

    m_role = role;
    emit roleChanged(m_role);

    m_state[State::Enabled] = true;
}

QAccessible::Role AccessibleItem::role() const
{
    return m_role;
}

void AccessibleItem::setName(QString name)
{
    if (m_name == name) {
        return;
    }

    m_name = name;
    emit nameChanged(m_name);
}

QString AccessibleItem::name() const
{
    return m_name;
}

void AccessibleItem::setVisualItem(QQuickItem* item)
{
    if (m_visualItem == item) {
        return;
    }

    m_visualItem = item;
    emit visualItemChanged(item);
}

QQuickItem* AccessibleItem::visualItem() const
{
    return m_visualItem;
}
