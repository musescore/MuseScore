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
#include "accessibilitycontroller.h"

#include <QGuiApplication>
#include <QAccessible>

#include "accessibleobject.h"
#include "accessiblecontrollerinterface.h"
#include "accessibleiteminterface.h"

#include "async/async.h"
#include "log.h"

using namespace mu::accessibility;

AccessibilityController::~AccessibilityController()
{
}

static QAccessibleInterface* muAccessibleFactory(const QString& classname, QObject* object)
{
    if (classname == QLatin1String("mu::accessibility::AccessibleObject")) {
        return static_cast<QAccessibleInterface*>(new AccessibleItemInterface(object));
    }

    if (classname == QLatin1String("mu::accessibility::AccessibilityController")) {
        return static_cast<QAccessibleInterface*>(new AccessibleControllerInterface(object));
    }

    return nullptr;
}

static void updateHandlerNoop(QAccessibleEvent*)
{
}

void AccessibilityController::init()
{
    setObjectName("AccessibilityController");

    QAccessible::installFactory(muAccessibleFactory);

    QAccessible::setRootObject(this);
    QAccessible::installRootObjectHandler([](QObject*) {});

    //! NOTE Disabled any events from Qt
    QAccessible::installUpdateHandler(updateHandlerNoop);
}

void AccessibilityController::created(IAccessibility* parent, IAccessibility* item)
{
    //! TODO Not working yet
    parent = nullptr;

    QObject* prnObj = nullptr;
    if (parent) {
        prnObj = findItem(parent).object;
    } else {
        prnObj = this;
    }

    Item it;
    it.parent = parent;
    it.item = item;
    it.object = new AccessibleObject(item, prnObj);
    it.object->setController(shared_from_this());
    it.iface = QAccessible::queryAccessibleInterface(it.object);

    m_items.append(it);

    LOGI() << "parent: " << (parent ? parent->accessibleName() : "") << ", item: " << item->accessibleName();

    QAccessibleEvent ev(it.object, QAccessible::ObjectCreated);
    sendEvent(&ev);
}

void AccessibilityController::destroyed(IAccessibility* aitem)
{
    LOGI() << aitem->accessibleName();
    int idx = indexBy(aitem);
    IF_ASSERT_FAILED(idx >= 0) {
        return;
    }
    Item item = m_items.takeAt(idx);
    QAccessibleEvent ev(item.object, QAccessible::ObjectDestroyed);
    sendEvent(&ev);

    delete item.object;
}

void AccessibilityController::actived(IAccessibility* aitem, bool isActive)
{
    LOGI() << aitem->accessibleName() << " " << isActive;
    const Item& item = findItem(aitem);
    IF_ASSERT_FAILED(item.isValid()) {
        return;
    }

    QAccessible::State state;
    state.active = isActive;
    QAccessibleStateChangeEvent ev(item.object, state);
    sendEvent(&ev);
}

void AccessibilityController::focused(IAccessibility* aitem)
{
    LOGI() << aitem->accessibleName();
    const Item& item = findItem(aitem);
    IF_ASSERT_FAILED(item.isValid()) {
        return;
    }

    QAccessibleEvent ev(item.object, QAccessible::Focus);
    sendEvent(&ev);
}

void AccessibilityController::sendEvent(QAccessibleEvent* ev)
{
    QAccessible::installUpdateHandler(0);
    QAccessible::updateAccessibility(ev);
    //! NOTE Disabled any events from Qt
    QAccessible::installUpdateHandler(updateHandlerNoop);
}

const AccessibilityController::Item& AccessibilityController::findItem(IAccessibility* aitem) const
{
    for (int i = 0; i < m_items.count(); ++i) {
        if (m_items.at(i).item == aitem) {
            return m_items.at(i);
        }
    }

    static AccessibilityController::Item null;
    return null;
}

int AccessibilityController::indexBy(IAccessibility* aitem) const
{
    for (int i = 0; i < m_items.count(); ++i) {
        if (m_items.at(i).item == aitem) {
            return i;
        }
    }
    return -1;
}

int AccessibilityController::childCount(const IAccessibility* aitem) const
{
    int count = 0;
    for (const Item& item: m_items) {
        if (item.parent == aitem) {
            ++count;
        }
    }
    return count;
}

QAccessibleInterface* AccessibilityController::child(const IAccessibility* aitem, int i) const
{
    int idx = -1;
    for (const Item& item: m_items) {
        if (item.parent == aitem) {
            ++idx;
            if (idx == i) {
                return item.iface;
            }
        }
    }
    return nullptr;
}

int AccessibilityController::indexOfChild(const IAccessibility* aitem, const QAccessibleInterface* iface) const
{
    int idx = -1;
    for (const Item& item: m_items) {
        if (item.parent == aitem) {
            ++idx;
            if (item.iface == iface) {
                return idx;
            }
        }
    }

    return -1;
}
