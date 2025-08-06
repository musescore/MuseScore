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
#include "iqaccessibleinterfaceregister.h"

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

AccessibilityController::AccessibilityController(const muse::modularity::ContextPtr& iocCtx)
    : muse::Injectable(iocCtx)
{
    m_pretendFocusTimer.setInterval(80); // Value found experimentally.
    m_pretendFocusTimer.setSingleShot(true);
    m_pretendFocusTimer.callOnTimeout([this]() {
        restoreFocus();
    });
}

AccessibilityController::~AccessibilityController()
{
    m_pretendFocusTimer.stop();
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

    auto dispatcher = actionsDispatcher();
    if (!dispatcher) {
        return;
    }

    dispatcher->preDispatch().onReceive(this, [this](actions::ActionCode) {
        // About to perform an action. Let's store some values to see if the action changes them.
        m_preDispatchFocus = m_lastFocused;
        m_preDispatchName = m_preDispatchFocus ? m_preDispatchFocus->accessibleName() : QString();
        m_announcement.clear(); // So we can detect if the action sets its own announcement.
    });

    dispatcher->postDispatch().onReceive(this, [this](actions::ActionCode actionCode) {
        // Just performed an action. Let's make sure the screen reader says something.

        if (!m_lastFocused || !m_announcement.isEmpty()) {
            return; // No focus item (prevents announcements), or the action set its own announcement.
        }

        if (m_lastFocused != m_preDispatchFocus || m_lastFocused->accessibleName() != m_preDispatchName) {
            return; // The screen reader will say something anyway.
        }

        const ui::UiAction action = actionsRegister()->action(actionCode);
        if (!action.isValid()) {
            return;
        }

        QString title = action.title.qTranslatedWithoutMnemonic();
        if (title.isEmpty()) {
            // E.g. UI navigation shortcuts: Tab, Space, Enter, Esc, etc.
            return; // Let the screen reader decide what to say.
        }

        announce(title); // Say the name of the action we just performed.
    });
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

    // VoiceOver: Use QObject not QAccessibleInterface in QAccessible…Event() constructors.
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

    if (item.item == m_lastFocused) {
        m_lastFocused = nullptr;
    }

    if (item.item == m_pretendFocus) {
        m_pretendFocus = nullptr;
    }

    if (m_children.contains(aitem)) {
        m_children.removeOne(aitem);
    }

    // VoiceOver: Use QObject not QAccessibleInterface in QAccessible…Event() constructors.
    QAccessibleEvent ev(item.object, QAccessible::ObjectDestroyed);
    sendEvent(&ev);

    delete item.object;
}

// Force the screen reader to speak an arbitrary message that isn't covered
// by standard accessibility events. For example, use this function to report
// that an action was performed, a different mode was entered, or a change
// occurred to an item that isn't the current focus item.

// NOTE: This function shouldn't be (ab)used to report changes in focus, or
// changes to the value, state, or text property of the focus item, unless
// such events are ignored or misreported by a particular screen reader.

// For standard events, see https://doc.qt.io/qt-6/qaccessible.html#Event-enum
// and classes that inherit from https://doc.qt.io/qt-6/qaccessibleevent.html

void AccessibilityController::announce(const QString& announcement)
{
    // Note: No early exit here if the announcement was already set.
    // If the user performs the same action multiple times then we
    // want to hear the same announcement multiple times.
    m_announcement = announcement;

    if (!m_lastFocused || announcement.isEmpty()) {
        return;
    }

    const Item& focused = findItem(m_lastFocused);
    if (!focused.isValid()) {
        return;
    }

#if 0
    // PROPER SOLUTION, but it requires Qt 6.8+ and has these problems:
    // * VoiceOver doesn't interrupt prior speech to say the announcement.
    // * When there's no selection, NVDA says "blank" before each announcement.
    // VoiceOver: Use QObject not QAccessibleInterface in QAccessible…Event() constructors.
    QAccessibleAnnouncementEvent event(focused.object, announcement);
    event.setPoliteness(QAccessible::AnnouncementPoliteness::Assertive);
    sendEvent(&event);
    return;
#endif

    // HACKY SOLUTION
    // Item returns announcement as its external name in accessibleiteminterface.cpp.
    // Note: Its *internal* name is not changed because that would notify subscribers
    // to IAccessible::accessiblePropertyChanged(), which has side effects.
    static constexpr QAccessible::Event eventType = QAccessible::NameChanged;

    if (focused.iface && needsRevoicing(*focused.iface, eventType)) {
        triggerRevoicing(focused);
        return;
    }

    // VoiceOver: Use QObject not QAccessibleInterface in QAccessible…Event() constructors.
    QAccessibleEvent event(focused.object, eventType);
    sendEvent(&event);
}

QString AccessibilityController::announcement() const
{
    return m_announcement;
}

const IAccessible* AccessibilityController::accessibleRoot() const
{
    return this;
}

const IAccessible* AccessibilityController::lastFocused() const
{
    return m_lastFocused;
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
    // NOTE: Handling of properties must match accessibleiteminterface.cpp.
    // See AccessibleItemInterface::text(type) among others.
    const Item& it = findItem(item);
    if (!it.isValid()) {
        return;
    }

    QAccessible::Event etype = QAccessible::InvalidEvent;

    // VoiceOver: Use QObject not QAccessibleInterface in QAccessible…Event() constructors.
    switch (property) {
    case IAccessible::Property::Undefined:
        return;
    case IAccessible::Property::Parent: etype = QAccessible::ParentChanged;
        break;
    case IAccessible::Property::Name:
    case IAccessible::Property::Description: {
        if (item == m_lastFocused) {
            m_announcement.clear();
        }

#if defined(Q_OS_MAC)
        // Names and descriptions are combined in accessibleiteminterface.cpp.
        etype = QAccessible::NameChanged;
#elif defined(Q_OS_WIN)
        // Descriptions are converted to accelerators in accessibleiteminterface.cpp.
        etype = property == IAccessible::Property::Name
                ? QAccessible::NameChanged
                : QAccessible::AcceleratorChanged;
#else
        // No changes in accessibleiteminterface.cpp.
        etype = property == IAccessible::Property::Name
                ? QAccessible::NameChanged
                : QAccessible::DescriptionChanged;
#endif

        if (it.iface && needsRevoicing(*it.iface, etype)) {
            triggerRevoicing(it);
            return;
        }

        m_needToVoicePanelInfo = false;
        break;
    }
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
        LOGE() << "Null parent for item: " << aitem->accessibleName();
        return;
    }

    if (aitem->accessibleState(state) != arg) {
        // For Narrator and any in-process screen readers, an item's reported
        // state must change BEFORE we send a state changed event.
        LOGE() << "Inconsistent state for item: " << aitem->accessibleName();
        return;
    }

    // We can't tell external APIs the new value of a state; only that it has changed.
    QAccessible::State changedState; // True means it changed (to either true or false).
    switch (state) {
    case State::Enabled: {
        changedState.disabled = true;
    } break;
    case State::Active: {
        changedState.active = true;
    } break;
    case State::Focused: {
        changedState.focused = true;
        if (arg) {
            // Affects focused state reported by accessibleiteminterface.cpp.
            // Must do this before sending the event.
            m_pretendFocus = nullptr;
            m_announcement.clear();
        }
    } break;
    case State::Selected: {
        changedState.selected = true;
    } break;
    case State::Checked: {
        changedState.checked = true;
    } break;
    default: {
        LOGE() << "not handled state: " << int(state);
        return;
    }
    }

    // Prompt screen readers to fetch the new state from accessibleiteminterface.cpp.
    // VoiceOver: Use QObject not QAccessibleInterface in QAccessible…Event() constructors.
    QAccessibleStateChangeEvent ev(item.object, changedState);
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

bool AccessibilityController::needsRevoicing(const QAccessibleInterface& iface, QAccessible::Event eventType) const
{
    QAccessible::Text textType;

    switch (eventType) {
    case QAccessible::NameChanged:
        textType = QAccessible::Name;
        break;
    case QAccessible::DescriptionChanged:
        textType = QAccessible::Description;
        break;
    case QAccessible::AcceleratorChanged:
        textType = QAccessible::Accelerator;
        break;
    default:
        UNREACHABLE;
        return false;
    }

#if defined(Q_OS_WIN)
    UNUSED(iface);
    IF_ASSERT_FAILED(textType != QAccessible::Description) {
        return false; // Narrator doesn't read descriptions.
    }
    // Narrator & NVDA ignore name & accelerator changes.
    return true;
#elif defined(Q_OS_MAC)
    IF_ASSERT_FAILED(textType == QAccessible::Name) {
        return false; // VoiceOver only reads names.
    }
    // VoiceOver ignores name changes while editing text.
    return iface.role() == QAccessible::EditableText;
#else
    // Apparently Orca ignores name changes when new text is multiple characters!
    // TODO: Confirm this and check behavior for descriptions and accelerators.
    return iface.text(textType).length() > 1;
#endif
}

void AccessibilityController::triggerRevoicing(const Item& current)
{
    if (current.item != m_lastFocused || !configuration()->active()) {
        return;
    }

    IF_ASSERT_FAILED(current.isUsable()) {
        return;
    }

    // Objective: Find a "sibling" item to focus for a short period (not long
    // enough for the screen reader to say anything), then restore focus back
    // to the current item to make the screen reader speak its updated info.

    // This seems to work best when the sibling isn't a descendant of the
    // current item, and the sibling doesn't have any children of its own.

    const Item* ancestor = &current;
    const Item* traversed;

    do {
        traversed = ancestor;
        ancestor = &findItem(ancestor->item->accessibleParent());
        if (!ancestor->isUsable()) {
            LOGE() << "No usable ancestor for item: " << current.item->accessibleName();
            return;
        }
    } while (ancestor->item->accessibleChildCount() < 2);

    const Item& sibling = findSiblingItem(*ancestor, *traversed);
    const Item& pretend = sibling.isValid() ? sibling : *ancestor;

    // We'll inform external APIs that this item has focus. Internally,
    // we won't really focus it because that would notify subscribers to
    // IAccessible::accessibleStateChanged(), which has side effects.

    MYLOG() << "Pretend focus item: " << pretend.item->accessibleName();
    m_pretendFocus = pretend.item; // See state() in accessibleiteminterface.cpp.
    m_ignorePanelChangingVoice = true;
    setExternalFocus(pretend);
    m_pretendFocusTimer.start(); // Calls restoreFocus() after a short delay.
}

void AccessibilityController::restoreFocus()
{
    m_ignorePanelChangingVoice = false;

    if (!m_pretendFocus) {
        return;
    }

    const IAccessible* pretendItem = m_pretendFocus;

    m_pretendFocus = nullptr; // Stops pretending in accessibleiteminterface.cpp.
                              // Must do this before sending focus changed events.

    const Item& restore = findItem(m_lastFocused);
    IF_ASSERT_FAILED(restore.isValid() && restore.item != pretendItem) {
        return;
    }

    MYLOG() << "Restore focus item: " << restore.item->accessibleName();
    setExternalFocus(restore);
}

void AccessibilityController::setExternalFocus(const Item& other)
{
    // We can't tell external APIs the new value of a state; only that it has changed.
    QAccessible::State focusChanged;
    focusChanged.focused = true; // Means focus has changed (to either true or false).

    // Prompt the screen reader to fetch the new state from accessibleiteminterface.cpp.
    // VoiceOver: Use QObject not QAccessibleInterface in QAccessible…Event() constructors.
    QAccessibleStateChangeEvent event1(other.object, focusChanged);
    sendEvent(&event1);
    QAccessibleEvent event2(other.object, QAccessible::Focus);
    sendEvent(&event2);

    // NOTE: We don't need a StateChangeEvent for the item that lost external focus.
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

        if (sibling.item == current.item || !sibling.isUsable()) {
            continue;
        }

        const IAccessible::Role role = sibling.item->accessibleRole();

        if (role != IAccessible::Group
            && role != IAccessible::Panel
            && sibling.item->accessibleState(State::Enabled)
            && sibling.item->accessibleChildCount() == 0) {
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

IAccessible* AccessibilityController::pretendFocus() const
{
    return m_pretendFocus;
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
