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
#include "navigableappmenumodel.h"

#include <QApplication>
#include <QWindow>
#include <QKeyEvent>

#include <private/qkeymapper_p.h>

#include "log.h"

using namespace mu::appshell;
using namespace mu::uicomponents;

QSet<int> possibleKeys(int keyNativeScanCode)
{
    QKeyEvent fakeKey(QKeyEvent::KeyRelease, Qt::Key_unknown, Qt::AltModifier, keyNativeScanCode, -1, -1);
    auto keys = QKeyMapper::possibleKeys(&fakeKey);

    return QSet<int>(keys.cbegin(), keys.cend());
}

QSet<int> possibleKeys(const QChar& keySymbol)
{
    QKeyEvent fakeKey(QKeyEvent::KeyRelease, Qt::Key_unknown, Qt::AltModifier, keySymbol);
    auto keys = QKeyMapper::possibleKeys(&fakeKey);

    return QSet<int>(keys.cbegin(), keys.cend());
}

NavigableAppMenuModel::NavigableAppMenuModel(QObject* parent)
    : AppMenuModel(parent)
{
}

void NavigableAppMenuModel::load()
{
    AppMenuModel::load();

    connect(qApp, &QApplication::applicationStateChanged, this, [this](Qt::ApplicationState state){
        if (state != Qt::ApplicationActive) {
            resetNavigation();
        }
    });

    qApp->installEventFilter(this);
}

QWindow* NavigableAppMenuModel::appWindow() const
{
    return m_appWindow;
}

void NavigableAppMenuModel::setAppWindow(QWindow* appWindow)
{
    m_appWindow = appWindow;
}

void NavigableAppMenuModel::setHighlightedMenuId(QString highlightedMenuId)
{
    if (m_highlightedMenuId == highlightedMenuId) {
        return;
    }

    m_highlightedMenuId = highlightedMenuId;
    emit highlightedMenuIdChanged(m_highlightedMenuId);
}

bool NavigableAppMenuModel::eventFilter(QObject* watched, QEvent* event)
{
    if (watched != appWindow()) {
        return AbstractMenuModel::eventFilter(watched, event);
    }

    auto isAltKey = [](const QKeyEvent* keyEvent){
        return keyEvent->key() == Qt::Key_Alt && keyEvent->key() != Qt::Key_Shift && !(keyEvent->modifiers() & Qt::ShiftModifier);
    };

    switch (event->type()) {
    case QEvent::KeyPress: {
        QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (isAltKey(keyEvent)) {
            m_needActivateHighlight = true;
        }

        break;
    }
    case QEvent::KeyRelease: {
        QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (!isAltKey(keyEvent)) {
            break;
        }

        if (isNavigationStarted()) {
            resetNavigation();
            restoreMUNavigationSystemState();
        } else {
            if (m_needActivateHighlight) {
                saveMUNavigationSystemState();
                navigateToFirstMenu();
            } else {
                m_needActivateHighlight = true;
            }
        }

        break;
    }
    case QEvent::ShortcutOverride: {
        QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
        bool isNavigationStarted = this->isNavigationStarted();
        if (isNavigationStarted && isNavigateKey(keyEvent->key())) {
            navigateByKeyScanCode(keyEvent->key());
            m_needActivateHighlight = false;

            event->accept();
        } else if ((!keyEvent->modifiers() && isNavigationStarted)
                   || ((keyEvent->modifiers() & Qt::AltModifier) && !(keyEvent->modifiers() & Qt::ShiftModifier)
                       && keyEvent->text().length() == 1)) {
            if (hasItemByKeyScanCode(keyEvent->nativeScanCode())) {
                navigateByKey(keyEvent->nativeScanCode());
                m_needActivateHighlight = true;

                event->accept();
            } else {
                m_needActivateHighlight = false;
                resetNavigation();
                restoreMUNavigationSystemState();

                event->ignore();
            }
        } else {
            m_needActivateHighlight = false;

            event->ignore();
        }

        break;
    }
    case QEvent::MouseButtonPress: {
        resetNavigation();
        break;
    }
    default:
        break;
    }

    return AbstractMenuModel::eventFilter(watched, event);
}

bool NavigableAppMenuModel::isNavigationStarted() const
{
    return !m_highlightedMenuId.isEmpty();
}

bool NavigableAppMenuModel::isNavigateKey(int key) const
{
    static QList<Qt::Key> keys {
        Qt::Key_Left,
        Qt::Key_Right,
        Qt::Key_Down,
        Qt::Key_Space,
        Qt::Key_Escape,
        Qt::Key_Return
    };

    return keys.contains(static_cast<Qt::Key>(key));
}

void NavigableAppMenuModel::navigateByKeyScanCode(int scanCode)
{
    Qt::Key _key = static_cast<Qt::Key>(scanCode);
    switch (_key) {
    case Qt::Key_Left: {
        int newIndex = itemIndex(m_highlightedMenuId) - 1;
        if (newIndex < 0) {
            newIndex = rowCount() - 1;
        }

        setHighlightedMenuId(item(newIndex).id());
        break;
    }
    case Qt::Key_Right: {
        int newIndex = itemIndex(m_highlightedMenuId) + 1;
        if (newIndex > rowCount() - 1) {
            newIndex = 0;
        }

        setHighlightedMenuId(item(newIndex).id());
        break;
    }
    case Qt::Key_Down:
    case Qt::Key_Space:
    case Qt::Key_Return:
        activateHighlightedMenu();
        break;
    case Qt::Key_Escape:
        resetNavigation();
        restoreMUNavigationSystemState();
        break;
    default:
        break;
    }
}

bool NavigableAppMenuModel::hasItemByKeyScanCode(int scanCode)
{
    return !menuIdByKeyScanCode(scanCode).isEmpty();
}

void NavigableAppMenuModel::navigateByKey(int scanCode)
{
    saveMUNavigationSystemState();

    setHighlightedMenuId(menuIdByKeyScanCode(scanCode));
    activateHighlightedMenu();
}

void NavigableAppMenuModel::resetNavigation()
{
    setHighlightedMenuId("");
}

void NavigableAppMenuModel::navigateToFirstMenu()
{
    setHighlightedMenuId(item(0).id());
}

void NavigableAppMenuModel::saveMUNavigationSystemState()
{
    if (!navigationController()->isHighlight()) {
        return;
    }

    ui::INavigationControl* activeControl = navigationController()->activeControl();
    if (activeControl) {
        m_lastActiveNavigationControl = activeControl;
        activeControl->setActive(false);
    }
}

void NavigableAppMenuModel::restoreMUNavigationSystemState()
{
    if (m_lastActiveNavigationControl) {
        m_lastActiveNavigationControl->requestActive();
    }
}

void NavigableAppMenuModel::activateHighlightedMenu()
{
    emit openMenu(m_highlightedMenuId);
    actionsDispatcher()->dispatch("nav-first-control");
}

QString NavigableAppMenuModel::highlightedMenuId() const
{
    return m_highlightedMenuId;
}

QString NavigableAppMenuModel::menuIdByKeyScanCode(int scanCode)
{
    auto activatePossibleKeys = possibleKeys(scanCode);

    for (int i = 0; i < rowCount(); i++) {
        MenuItem& menuItem = item(i);
        QString title = menuItem.action().title;

        int activateKeyIndex = title.indexOf('&');
        if (activateKeyIndex == -1) {
            continue;
        }

        auto menuActivatePossibleKeys = possibleKeys(title[activateKeyIndex + 1].toUpper());
        if (menuActivatePossibleKeys.intersects(activatePossibleKeys)) {
            return menuItem.id();
        }
    }

    return QString();
}
