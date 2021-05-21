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

static const char* MU_ACCESSIBLE_PARENT = "mu_accessibleParent";

AccessibleAttached* AccessibleAttached::qmlAttachedProperties(QObject* object)
{
    return new AccessibleAttached(object);
}

// =======================================================
// AccessibleItem
// =======================================================
AccessibleItem::AccessibleItem(QObject* parent)
    : QObject(parent)
{
}

void AccessibleItem::classBegin()
{
}

void AccessibleItem::componentComplete()
{
    if (accessible()) {
        accessible()->init();
    }
}

bool AccessibleItem::event(QEvent* event)
{
    if (event->type() == QEvent::DynamicPropertyChange) {
        if (accessible()) {
            QDynamicPropertyChangeEvent* propEvent = static_cast<QDynamicPropertyChangeEvent*>(event);
            if (propEvent->propertyName() == MU_ACCESSIBLE_PARENT) {
                accessible()->notifyAboutParentChanged();
            }
        }
    }

    return QObject::event(event);
}

AccessibleAttached* AccessibleItem::accessible() const
{
    if (!m_attached) {
        m_attached = qobject_cast<AccessibleAttached*>(qmlAttachedPropertiesObject<AccessibleAttached>(this, false));
    }
    return m_attached;
}

size_t AccessibleItem::accessibleChildCount() const
{
    return 0;
}

const IAccessible* AccessibleItem::accessibleChild(size_t) const
{
    return nullptr;
}

void AccessibleItem::setAccessibleParent(AccessibleItem* item)
{
    setProperty(MU_ACCESSIBLE_PARENT, QVariant::fromValue(item));
}

void AccessibleItem::setAccessibleState(accessibility::IAccessible::State st, bool arg)
{
    if (accessible()) {
        accessible()->setAccessibleState(st, arg);
    }
}

// =======================================================
// AccessibleAttached
// =======================================================

AccessibleAttached::AccessibleAttached(QObject* object)
    : QObject(object)
{
    m_object = qobject_cast<AccessibleItem*>(object);
    IF_ASSERT_FAILED(m_object) {
    }
}

AccessibleAttached::~AccessibleAttached()
{
    accessibilityController()->unreg(this);
    m_registred = false;
}

void AccessibleAttached::init()
{
    accessibilityController()->reg(this);
    m_registred = true;
}

const IAccessible* AccessibleAttached::accessibleRoot() const
{
    return accessibilityController()->rootItem();
}

void AccessibleAttached::notifyAboutParentChanged()
{
    m_accessibleParentChanged.notify();
}

void AccessibleAttached::setAccessibleState(accessibility::IAccessible::State st, bool arg)
{
    if (m_state.value(st, false) == arg) {
        return;
    }

    m_state[st] = arg;
    emit stateChanged();
    onStateChanged(st, arg);
}

void AccessibleAttached::onStateChanged(State st, bool arg)
{
    if (m_registred) {
        accessibilityController()->stateChanged(this, st, arg);
    }
}

const IAccessible* AccessibleAttached::accessibleParent() const
{
    QObject* prnObj = m_object->property(MU_ACCESSIBLE_PARENT).value<QObject*>();
    if (!prnObj) {
        return accessibleRoot();
    }

    AccessibleItem* prnItem = qobject_cast<AccessibleItem*>(prnObj);
    IF_ASSERT_FAILED(prnItem) {
        return nullptr;
    }

    if (!prnItem->accessible()) {
        LOGW() << "parent item not accessible";
        return nullptr;
    }

    return static_cast<const IAccessible*>(prnItem->accessible());
}

mu::async::Notification AccessibleAttached::accessibleParentChanged() const
{
    return m_accessibleParentChanged;
}

size_t AccessibleAttached::accessibleChildCount() const
{
    return m_object->accessibleChildCount();
}

const IAccessible* AccessibleAttached::accessibleChild(size_t i) const
{
    return m_object->accessibleChild(i);
}

IAccessible::Role AccessibleAttached::accessibleRole() const
{
    return static_cast<IAccessible::Role>(m_role);
}

QString AccessibleAttached::accessibleName() const
{
    return m_name;
}

bool AccessibleAttached::accessibleState(State st) const
{
    return m_state.value(st, false);
}

QRect AccessibleAttached::accessibleRect() const
{
    if (!m_visualItem || !m_visualItem->window()) {
        return QRect();
    }

    QPointF scenePos = m_visualItem->mapToScene(QPointF(0, 0));
    QPoint globalPos = m_visualItem->window()->mapToGlobal(scenePos.toPoint());
    return QRect(globalPos.x(), globalPos.y(), m_visualItem->width(), m_visualItem->height());
}

QWindow* AccessibleAttached::accessibleWindow() const
{
    if (!m_visualItem || !m_visualItem->window()) {
        return nullptr;
    }
    return m_visualItem->window();
}

void AccessibleAttached::setRole(QmlRole role)
{
    if (m_role == role) {
        return;
    }

    m_role = role;
    emit roleChanged(m_role);

    m_state[State::Enabled] = true;
}

AccessibleAttached::QmlRole AccessibleAttached::role() const
{
    return m_role;
}

void AccessibleAttached::setName(QString name)
{
    if (m_name == name) {
        return;
    }

    m_name = name;
    emit nameChanged(m_name);
}

QString AccessibleAttached::name() const
{
    return m_name;
}

void AccessibleAttached::setVisualItem(QQuickItem* item)
{
    if (m_visualItem == item) {
        return;
    }

    m_visualItem = item;
    emit visualItemChanged(item);
}

QQuickItem* AccessibleAttached::visualItem() const
{
    return m_visualItem;
}
