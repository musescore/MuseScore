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

    QList<AccessibleItem*> children = m_children;
    for (AccessibleItem* ch : children) {
        ch->setAccessibleParent(nullptr);
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

const IAccessible* AccessibleItem::accessibleRoot() const
{
    return accessibilityController()->accessibleRoot();
}

const IAccessible* AccessibleItem::accessibleParent() const
{
    if (m_accessibleParent) {
        return static_cast<const IAccessible*>(m_accessibleParent);
    }

    return accessibleRoot();
}

IAccessible::Role AccessibleItem::accessibleRole() const
{
    return static_cast<IAccessible::Role>(m_role);
}

QString AccessibleItem::accessibleName() const
{
    return m_name;
}

QString AccessibleItem::accessibleDescription() const
{
    return m_description;
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

mu::async::Channel<IAccessible::Property> AccessibleItem::accessiblePropertyChanged() const
{
    return m_accessiblePropertyChanged;
}

mu::async::Channel<IAccessible::State, bool> AccessibleItem::accessibleStateChanged() const
{
    return m_accessibleStateChanged;
}

AccessibleItem* AccessibleItem::accessibleParent_property() const
{
    return m_accessibleParent;
}

void AccessibleItem::setAccessibleParent(AccessibleItem* p)
{
    if (m_accessibleParent == p) {
        return;
    }

    if (m_accessibleParent) {
        m_accessibleParent->removeChild(this);
    }

    m_accessibleParent = p;

    if (m_accessibleParent) {
        m_accessibleParent->addChild(this);
    }

    emit accessiblePrnChanged();
    m_accessiblePropertyChanged.send(IAccessible::Property::Parent);
}

void AccessibleItem::setState(IAccessible::State st, bool arg)
{
    if (m_state.value(st, false) == arg) {
        return;
    }

    m_state[st] = arg;
    emit stateChanged();

    if (!m_ignored) {
        m_accessibleStateChanged.send(st, arg);
    }
}

void AccessibleItem::setRole(MUAccessible::Role role)
{
    if (m_role == role) {
        return;
    }

    m_role = role;
    emit roleChanged(m_role);

    m_state[State::Enabled] = true;
}

MUAccessible::Role AccessibleItem::role() const
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
    m_accessiblePropertyChanged.send(IAccessible::Property::Name);
}

void AccessibleItem::setDescription(QString description)
{
    if (m_description == description) {
        return;
    }

    m_description = description;
    emit descriptionChanged(m_description);
    m_accessiblePropertyChanged.send(IAccessible::Property::Description);
}

QString AccessibleItem::name() const
{
    return m_name;
}

QString AccessibleItem::description() const
{
    return m_description;
}

void AccessibleItem::setIgnored(bool ignored)
{
    if (m_ignored == ignored) {
        return;
    }

    m_ignored = ignored;
    emit ignoredChanged(m_ignored);
}

bool AccessibleItem::ignored() const
{
    return m_ignored;
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
