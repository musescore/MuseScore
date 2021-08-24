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
#include <QWindow>

#include "accessibleobject.h"
#include "accessibleiteminterface.h"
#include "async/async.h"

#include "log.h"

//#define ACCESSIBILITY_LOGGING_ENABLED

#ifdef ACCESSIBILITY_LOGGING_ENABLED
#define MYLOG() LOGI()
#else
#define MYLOG() LOGN()
#endif

using namespace mu::accessibility;

AccessibilityController::~AccessibilityController()
{
    unreg(this);
}

static QAccessibleInterface* muAccessibleFactory(const QString& classname, QObject* object)
{
    if (classname == QLatin1String("mu::accessibility::AccessibleObject")) {
        AccessibleObject* aobj = qobject_cast<AccessibleObject*>(object);
        IF_ASSERT_FAILED(aobj) {
            return nullptr;
        }
        return static_cast<QAccessibleInterface*>(new AccessibleItemInterface(aobj));
    }

    return nullptr;
}

static void updateHandlerNoop(QAccessibleEvent*)
{
}

static void rootObjectHandlerNoop(QObject*)
{
}

void AccessibilityController::init()
{
    QAccessible::installFactory(muAccessibleFactory);

    reg(this);
    const Item& self = findItem(this);

    QAccessible::installRootObjectHandler(nullptr);
    QAccessible::setRootObject(self.object);

    //! NOTE Disabled any events from Qt
    QAccessible::installRootObjectHandler(rootObjectHandlerNoop);
    QAccessible::installUpdateHandler(updateHandlerNoop);
}

const IAccessible* AccessibilityController::accessibleRoot() const
{
    return this;
}

void AccessibilityController::reg(IAccessible* item)
{
    if (findItem(item).isValid()) {
        LOGW() << "Already registered";
        return;
    }

    MYLOG() << "item: " << item->accessibleName();

    Item it;
    it.item = item;
    it.object = new AccessibleObject(item);
    it.object->setController(shared_from_this());
    it.iface = QAccessible::queryAccessibleInterface(it.object);

    m_allItems.insert(item, it);

    if (item->accessibleParent() == this) {
        m_children.append(item);
    }

    item->accessiblePropertyChanged().onReceive(this, [this, item](const IAccessible::Property& p) {
        propertyChanged(item, p);
    });

    item->accessibleStateChanged().onReceive(this, [this, item](const State& state, bool arg) {
        stateChanged(item, state, arg);
    });

    QAccessibleEvent ev(it.object, QAccessible::ObjectCreated);
    sendEvent(&ev);
}

void AccessibilityController::unreg(IAccessible* aitem)
{
    MYLOG() << aitem->accessibleName();

    Item item = m_allItems.take(aitem);
    if (!item.isValid()) {
        return;
    }

    if (m_children.contains(aitem)) {
        m_children.removeOne(aitem);
    }

    QAccessibleEvent ev(item.object, QAccessible::ObjectDestroyed);
    sendEvent(&ev);

    delete item.object;
}

void AccessibilityController::propertyChanged(IAccessible* item, IAccessible::Property p)
{
    const Item& it = findItem(item);
    if (!it.isValid()) {
        return;
    }

    QAccessible::Event etype = QAccessible::InvalidEvent;
    switch (p) {
    case IAccessible::Property::Undefined:
        return;
    case IAccessible::Property::Parent: etype = QAccessible::ParentChanged;
        break;
    case IAccessible::Property::Name: etype = QAccessible::NameChanged;
        break;
    case IAccessible::Property::Description: etype = QAccessible::DescriptionChanged;
        break;
    }

    QAccessibleEvent ev(it.object, etype);
    sendEvent(&ev);
}

void AccessibilityController::stateChanged(IAccessible* aitem, State state, bool arg)
{
    if (!configuration()->enabled()) {
        return;
    }

    MYLOG() << aitem->accessibleName() << ", state: " << int(state) << ", arg: " << arg;
    const Item& item = findItem(aitem);
    IF_ASSERT_FAILED(item.isValid()) {
        return;
    }

    if (!item.item->accessibleParent()) {
        LOGE() << "for item: " << aitem->accessibleName() << " parent is null";
        return;
    }

    QAccessible::State qstate;
    switch (state) {
    case State::Enabled: {
        qstate.disabled = !arg;
    } break;
    case State::Active: {
        qstate.active = arg;
    } break;
    case State::Focused: {
        qstate.focused = arg;
    } break;
    case State::Selected: {
        qstate.selected = arg;
    } break;
    default: {
        LOGE() << "not handled state: " << int(state);
        return;
    }
    }

    QAccessibleStateChangeEvent ev(item.object, qstate);
    sendEvent(&ev);

    if (state == State::Focused) {
        if (arg) {
            QAccessibleEvent ev(item.object, QAccessible::Focus);
            sendEvent(&ev);
        }
    }
}

void AccessibilityController::sendEvent(QAccessibleEvent* ev)
{
#ifdef ACCESSIBILITY_LOGGING_ENABLED
    AccessibleObject* obj = qobject_cast<AccessibleObject*>(ev->object());
    MYLOG() << "object: " << obj->item()->accessibleName() << ", event: " << int(ev->type());
#endif

    QAccessible::installUpdateHandler(0);
    QAccessible::updateAccessibility(ev);
    //! NOTE Disabled any events from Qt
    QAccessible::installUpdateHandler(updateHandlerNoop);

    m_eventSent.send(ev);
}

mu::async::Channel<QAccessibleEvent*> AccessibilityController::eventSent() const
{
    return m_eventSent;
}

const AccessibilityController::Item& AccessibilityController::findItem(const IAccessible* aitem) const
{
    auto it = m_allItems.find(aitem);
    if (it != m_allItems.end()) {
        return it.value();
    }

    static AccessibilityController::Item null;
    return null;
}

QAccessibleInterface* AccessibilityController::parentIface(const IAccessible* item) const
{
    IF_ASSERT_FAILED(item) {
        return nullptr;
    }

    const IAccessible* parent = item->accessibleParent();
    if (!parent) {
        return nullptr;
    }

    const Item& it = findItem(parent);
    if (!it.isValid()) {
        return nullptr;
    }
    return it.iface;
}

int AccessibilityController::childCount(const IAccessible* item) const
{
    IF_ASSERT_FAILED(item) {
        return 0;
    }

    const Item& it = findItem(item);
    IF_ASSERT_FAILED(it.isValid()) {
        return 0;
    }
    return static_cast<int>(it.item->accessibleChildCount());
}

QAccessibleInterface* AccessibilityController::child(const IAccessible* item, int i) const
{
    IF_ASSERT_FAILED(item) {
        return nullptr;
    }

    const IAccessible* chld = item->accessibleChild(static_cast<size_t>(i));
    IF_ASSERT_FAILED(chld) {
        return nullptr;
    }

    const Item& chldIt = findItem(chld);
    IF_ASSERT_FAILED(chldIt.isValid()) {
        return nullptr;
    }

    return chldIt.iface;
}

int AccessibilityController::indexOfChild(const IAccessible* item, const QAccessibleInterface* iface) const
{
    TRACEFUNC;
    size_t count = item->accessibleChildCount();
    for (size_t i = 0; i < count; ++i) {
        const IAccessible* ch = item->accessibleChild(i);
        const Item& chIt = findItem(ch);
        IF_ASSERT_FAILED(chIt.isValid()) {
            continue;
        }

        if (chIt.iface == iface) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

IAccessible* AccessibilityController::accessibleParent() const
{
    return nullptr;
}

size_t AccessibilityController::accessibleChildCount() const
{
    return static_cast<size_t>(m_children.size());
}

IAccessible* AccessibilityController::accessibleChild(size_t i) const
{
    return m_children.at(static_cast<int>(i));
}

IAccessible::Role AccessibilityController::accessibleRole() const
{
    return IAccessible::Role::Application;
}

QString AccessibilityController::accessibleName() const
{
    return QString("AccessibilityController");
}

QString AccessibilityController::accessibleDescription() const
{
    return QString();
}

bool AccessibilityController::accessibleState(State st) const
{
    switch (st) {
    case State::Undefined: return false;
    case State::Enabled: return true;
    case State::Active: return true;
    default: {
        LOGW() << "not handled state: " << static_cast<int>(st);
    }
    }

    return false;
}

QRect AccessibilityController::accessibleRect() const
{
    return mainWindow()->qWindow()->geometry();
}

mu::async::Channel<IAccessible::Property> AccessibilityController::accessiblePropertyChanged() const
{
    static async::Channel<IAccessible::Property> ch;
    return ch;
}

mu::async::Channel<IAccessible::State, bool> AccessibilityController::accessibleStateChanged() const
{
    static async::Channel<IAccessible::State, bool> ch;
    return ch;
}
