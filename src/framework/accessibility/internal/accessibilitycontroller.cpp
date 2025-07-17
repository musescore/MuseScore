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
#include <QTimer>

#ifdef Q_OS_LINUX
#include <QKeyEvent>
#include <private/qcoreapplication_p.h>
#endif

#include "accessibleobject.h"
#include "accessiblestub.h"
#include "accessibleiteminterface.h"

#include "log.h"

#ifdef MUSE_MODULE_ACCESSIBILITY_TRACE
#define MYLOG() LOGI()
#else
#define MYLOG() LOGN()
#endif

using namespace muse;
using namespace muse::modularity;
using namespace muse::accessibility;

AccessibleObject* s_rootObject = nullptr;
std::shared_ptr<IQAccessibleInterfaceRegister> accessibleInterfaceRegister = nullptr;

static void updateHandlerNoop(QAccessibleEvent*)
{
}

AccessibilityController::~AccessibilityController()
{
    unreg(this);
}

QAccessibleInterface* AccessibilityController::accessibleInterface(QObject*)
{
    return static_cast<QAccessibleInterface*>(new AccessibleItemInterface(s_rootObject));
}

void AccessibilityController::setAccesibilityEnabled(bool enabled)
{
    m_enabled = enabled;
}

static QAccessibleInterface* muAccessibleFactory(const QString& classname, QObject* object)
{
    if (!accessibleInterfaceRegister) {
        accessibleInterfaceRegister = globalIoc()->resolve<IQAccessibleInterfaceRegister>("accessibility");
    }

    auto interfaceGetter = accessibleInterfaceRegister->interfaceGetter(classname);
    if (interfaceGetter) {
        return interfaceGetter(object);
    }

    return AccessibleStub::accessibleInterface(object);
}

void AccessibilityController::init()
{
    QAccessible::installFactory(muAccessibleFactory);

    reg(this);
    const Item& self = findItem(this);
    s_rootObject = self.object;

    QAccessible::installRootObjectHandler(nullptr);
    QAccessible::setRootObject(s_rootObject);
}

void AccessibilityController::reg(IAccessible* item)
{
    if (!m_enabled) {
        return;
    }

    if (!m_inited) {
        //! This needed to be done here, because we need to init controller (register factory) after UI is start loaded,
        //! thus we register the factory after qt registers its factory so that our factory is called first
        m_inited = true;
        init();
    }

    if (findItem(item).isValid()) {
        LOGW() << "Already registered";
        return;
    }

    MYLOG() << "item: " << item->accessibleName();

    Item it;
    it.item = item;
    it.object = new AccessibleObject(item);
    it.object->setController(weak_from_this());
    it.iface = QAccessible::queryAccessibleInterface(it.object);

    m_allItems.insert(item, it);

    if (item->accessibleParent() == this) {
        m_children.append(item);
    }

    item->accessiblePropertyChanged().onReceive(this, [this, item](const IAccessible::Property& p, const Val value) {
        propertyChanged(item, p, value);
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

    if (m_lastFocused == item.item) {
        m_lastFocused = nullptr;
    }

    if (m_itemForRestoreFocus == item.item) {
        m_itemForRestoreFocus = nullptr;
    }

    if (m_children.contains(aitem)) {
        m_children.removeOne(aitem);
    }

    QAccessibleEvent ev(item.object, QAccessible::ObjectDestroyed);
    sendEvent(&ev);

    delete item.object;
}

void AccessibilityController::announce(const QString& announcement)
{
    m_announcement = announcement;

    if (m_lastFocused == nullptr || announcement.isEmpty()) {
        return;
    }

    const Item& focused = findItem(m_lastFocused);
    if (!focused.isValid()) {
        return;
    }


    // VoiceOver: Speech only works if you use a QObject rather than a
    // QAccessibleInterface to construct QAccessibleEvent() and subclasses.
#if 0
    // Proper solution, but it requires Qt 6.8+ and has these problems:
    // * VoiceOver doesn't interrupt prior speech to say the announcement.
    // * If you select a notation element, press Esc, then press other
    //   shortcuts, NVDA says "blank" before each announcement.
    QAccessibleAnnouncementEvent event(focused.object, announcement);
    event.setPoliteness(QAccessible::AnnouncementPoliteness::Assertive);
    sendEvent(&event);
#elif defined(Q_OS_MAC)
    // Item returns announcement as its name in accessibleiteminterface.cpp.
    // Let's prompt VoiceOver to query the new name.
    QAccessibleEvent event(focused.object, QAccessible::NameChanged);
    sendEvent(&event);
#else
    // Item returns announcement as its name in accessibleiteminterface.cpp.
    // NVDA, Narrator, Orca ignore name changes, so we pretend focus changed.
    triggerRevoicingOfChangedName(focused);
#endif
}

const IAccessible* AccessibilityController::accessibleRoot() const
{
    return this;
}

const IAccessible* AccessibilityController::lastFocused() const
{
    return m_lastFocused;
}

const QString& AccessibilityController::announcement() const
{
    return m_announcement;
}

bool AccessibilityController::needToVoicePanelInfo() const
{
    return m_needToVoicePanelInfo;
}

QString AccessibilityController::currentPanelAccessibleName() const
{
    const IAccessible* focusedItemPanel = panel(m_lastFocused);
    return focusedItemPanel ? focusedItemPanel->accessibleName() : "";
}

void AccessibilityController::setIgnoreQtAccessibilityEvents(bool ignore)
{
    if (ignore) {
        QAccessible::installUpdateHandler(updateHandlerNoop);
    } else {
        QAccessible::installUpdateHandler(nullptr);
    }
}

void AccessibilityController::propertyChanged(IAccessible* item, IAccessible::Property property, const Val& value)
{
    const Item& it = findItem(item);
    if (!it.isValid()) {
        return;
    }

    QAccessible::Event etype = QAccessible::InvalidEvent;
    switch (property) {
    case IAccessible::Property::Undefined:
        return;
    case IAccessible::Property::Parent: etype = QAccessible::ParentChanged;
        break;
    case IAccessible::Property::Name: {
        if (item == m_lastFocused) {
            m_pretendFocusItem = nullptr;
            m_announcement.clear();
        }
        bool triggerRevoicing = false;

#ifdef Q_OS_MAC
        triggerRevoicing = false;
#elif defined(Q_OS_WIN)
        triggerRevoicing = true;
#else
        //! if it is one character than we can send NameChange and don't use hack with revoicing
        triggerRevoicing = item->accessibleName().size() > 1;
#endif

        if (triggerRevoicing) {
            triggerRevoicingOfChangedName(it);
            return;
        } else {
            m_needToVoicePanelInfo = false;
            etype = QAccessible::NameChanged;
            break;
        }
    }
    case IAccessible::Property::Description: etype = QAccessible::DescriptionChanged;
        break;
    case IAccessible::Property::Value: {
        QAccessibleValueChangeEvent ev(it.object, it.item->accessibleValue());
        sendEvent(&ev);
        return;
    }
    case IAccessible::Property::TextCursor: {
        QAccessibleTextCursorEvent ev(it.object, it.item->accessibleCursorPosition());
        sendEvent(&ev);
        return;
    }
    case IAccessible::Property::TextInsert: {
        IAccessible::TextRange range(value.toQVariant().toMap());
        QAccessibleTextInsertEvent ev(it.object, range.startPosition,
                                      it.item->accessibleText(range.startPosition, range.endPosition));
        sendEvent(&ev);
        return;
    }
    case IAccessible::Property::TextRemove: {
        IAccessible::TextRange range(value.toQVariant().toMap());
        QAccessibleTextRemoveEvent ev(it.object, range.startPosition,
                                      it.item->accessibleText(range.startPosition, range.endPosition));
        sendEvent(&ev);
        return;
    }
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
        qstate.disabled = true;
    } break;
    case State::Active: {
        qstate.active = true;
    } break;
    case State::Focused: {
        qstate.focused = true;
        if (arg) {
            // Do this before we send the event to allow for in-process screen readers.
            m_pretendFocusItem = nullptr;
            m_announcement.clear();
        }
    } break;
    case State::Selected: {
        qstate.selected = true;
    } break;
    case State::Checked: {
        qstate.checked = true;
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
            cancelPreviousReading();
            savePanelAccessibleName(m_lastFocused, item.item);

            QAccessibleEvent ev2(item.object, QAccessible::Focus);
            sendEvent(&ev2);
            m_lastFocused = item.item;
        }
    }
}

void AccessibilityController::sendEvent(QAccessibleEvent* ev)
{
#ifdef MUSE_MODULE_ACCESSIBILITY_TRACE
    AccessibleObject* obj = qobject_cast<AccessibleObject*>(ev->object());
    MYLOG() << "object: " << obj->item()->accessibleName() << ", event: " << int(ev->type());
#endif

    QAccessible::updateAccessibility(ev);

    m_eventSent.send(ev);
}

void AccessibilityController::cancelPreviousReading()
{
    if (!configuration()->active()) {
        return;
    }

#ifdef Q_OS_LINUX
    //! HACK: it needs for canceling reading the name of previous control on accessibility
    QKeyEvent* keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Cancel, Qt::KeyboardModifier::NoModifier, 0, 1, 0);
    QCoreApplicationPrivate::setEventSpontaneous(keyEvent, true);
    application()->notify(mainWindow()->qWindow(), keyEvent);
#endif
}

void AccessibilityController::savePanelAccessibleName(const IAccessible* oldItem, const IAccessible* newItem)
{
    if (m_ignorePanelChangingVoice) {
        m_needToVoicePanelInfo = false;
        return;
    }

    const IAccessible* oldItemPanel = panel(oldItem);
    QString oldItemPanelName = oldItemPanel ? oldItemPanel->accessibleName() : "";

    const IAccessible* newItemPanel = panel(newItem);
    QString newItemPanelName = newItemPanel ? newItemPanel->accessibleName() : "";

    m_needToVoicePanelInfo = oldItemPanelName != newItemPanelName;
}

void AccessibilityController::triggerRevoicingOfChangedName(const Item& current)
{
    if (!configuration()->active()) {
        return;
    }

    if (current.item != m_lastFocused) {
        return;
    }

    const Item& currentPanel = findItem(panel(current.item));
    if (!currentPanel.isValid()) {
        return;
    }

    m_ignorePanelChangingVoice = true;

    const Item& sibling = findSiblingItem(currentPanel, current);
    const Item& pretend = sibling.isValid() ? sibling : currentPanel;

    // Inform the accessibility backend that focus has changed. We can't tell it the new state
    // here; only that it has changed. The accessibility backend should then query the changed
    // item on its own to determine the new state.
    QAccessible::State focusedChangedState;
    focusedChangedState.focused = true; // means focus has changed (to either true or false)

    m_pretendFocusItem = pretend.item;
    QAccessibleStateChangeEvent currentChanged(current.object, focusedChangedState);
    QAccessibleStateChangeEvent pretendChanged(pretend.object, focusedChangedState);
    QAccessibleEvent pretendFocused(pretend.object, QAccessible::Focus);
    sendEvent(&currentChanged);
    sendEvent(&pretendChanged);
    sendEvent(&pretendFocused);

    //! NOTE: Restore the focused element after some delay(this value was found experimentally)
    QTimer::singleShot(100, [&]() {
        m_pretendFocusItem = nullptr;
        m_ignorePanelChangingVoice = false;
        QAccessibleStateChangeEvent pretendChanged(pretend.object, focusedChangedState);
        QAccessibleStateChangeEvent currentChanged(current.object, focusedChangedState);
        QAccessibleEvent currentFocused(current.object, QAccessible::Focus);
        sendEvent(&pretendChanged);
        sendEvent(&currentChanged);
        sendEvent(&currentFocused);
    });
}

const IAccessible* AccessibilityController::panel(const IAccessible* item) const
{
    if (!item) {
        return nullptr;
    }

    const IAccessible* parent = item->accessibleParent();
    while (parent) {
        if (parent->accessibleRole() == IAccessible::Panel) {
            return parent;
        }

        parent = parent->accessibleParent();
    }

    return nullptr;
}

const AccessibilityController::Item& AccessibilityController::findSiblingItem(const Item& parent, const Item& current) const
{
    TRACEFUNC;
    static constexpr Item null;
    const size_t count = parent.item->accessibleChildCount();

    for (size_t i = 0; i < count; ++i) {
        const Item& sibling = findItem(parent.item->accessibleChild(i));

        if (sibling.item == current.item
            || !sibling.isValid()
            || !sibling.iface
            || sibling.item->accessibleIgnored()) {
            continue;
        }

        const IAccessible::Role role = sibling.item->accessibleRole();

        if (sibling.item->accessibleState(State::Enabled)
            && role != IAccessible::Group
            && role != IAccessible::Panel) {
            return sibling;
        }

        if (sibling.item->accessibleChildCount() > 0) {
            const Item& subItem = findSiblingItem(sibling, current);
            if (subItem.isValid()) {
                return subItem;
            }
        }
    }

    return null;
}

IAccessible* AccessibilityController::pretendFocusItem() const
{
    return m_pretendFocusItem;
}

async::Channel<QAccessibleEvent*> AccessibilityController::eventSent() const
{
    return m_eventSent;
}

const AccessibilityController::Item& AccessibilityController::findItem(const IAccessible* aitem) const
{
    static constexpr Item null;

    if (!aitem) {
        return null;
    }

    auto it = m_allItems.find(aitem);

    if (it == m_allItems.end()) {
        return null;
    }

    return it.value();
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

    if (it.item->accessibleRole() == IAccessible::Role::Application) {
        if (!qApp->isQuitLockEnabled()) {
            return QAccessible::queryAccessibleInterface(interactiveProvider()->topWindow());
        } else {
            return QAccessible::queryAccessibleInterface(qApp->focusWindow());
        }
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

QAccessibleInterface* AccessibilityController::focusedChild(const IAccessible* item) const
{
    TRACEFUNC;
    size_t count = item->accessibleChildCount();
    for (size_t i = 0; i < count; ++i) {
        const IAccessible* ch = item->accessibleChild(i);
        const Item& chIt = findItem(ch);
        if (!chIt.isValid() || !chIt.iface || chIt.item->accessibleIgnored()) {
            continue;
        }

        if (chIt.iface->state().focused) {
            return chIt.iface;
        }

        if (chIt.item->accessibleChildCount() > 0) {
            QAccessibleInterface* subItem = focusedChild(chIt.item);
            if (subItem) {
                return subItem;
            }
        }
    }

    return nullptr;
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

QWindow* AccessibilityController::accessibleWindow() const
{
    return mainWindow()->qWindow();
}

muse::modularity::ContextPtr AccessibilityController::iocContext() const
{
    return Injectable::iocContext();
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

bool AccessibilityController::accessibleIgnored() const
{
    return false;
}

QVariant AccessibilityController::accessibleValue() const
{
    return QVariant();
}

QVariant AccessibilityController::accessibleMaximumValue() const
{
    return QVariant();
}

QVariant AccessibilityController::accessibleMinimumValue() const
{
    return QVariant();
}

QVariant AccessibilityController::accessibleValueStepSize() const
{
    return QVariant();
}

void AccessibilityController::accessibleSelection(int, int*, int*) const
{
}

int AccessibilityController::accessibleSelectionCount() const
{
    return 0;
}

int AccessibilityController::accessibleCursorPosition() const
{
    return 0;
}

QString AccessibilityController::accessibleText(int, int) const
{
    return QString();
}

QString AccessibilityController::accessibleTextBeforeOffset(int, TextBoundaryType, int*, int*) const
{
    return QString();
}

QString AccessibilityController::accessibleTextAfterOffset(int, TextBoundaryType, int*, int*) const
{
    return QString();
}

QString AccessibilityController::accessibleTextAtOffset(int, TextBoundaryType, int*, int*) const
{
    return QString();
}

int AccessibilityController::accessibleCharacterCount() const
{
    return 0;
}

int AccessibilityController::accessibleRowIndex() const
{
    return 0;
}

async::Channel<IAccessible::Property, Val> AccessibilityController::accessiblePropertyChanged() const
{
    static async::Channel<IAccessible::Property, Val> ch;
    return ch;
}

async::Channel<IAccessible::State, bool> AccessibilityController::accessibleStateChanged() const
{
    static async::Channel<IAccessible::State, bool> ch;
    return ch;
}

void AccessibilityController::setState(State, bool)
{
}
