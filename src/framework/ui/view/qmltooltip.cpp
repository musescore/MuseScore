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
#include "qmltooltip.h"

#include <QGuiApplication>
#include <QStyleHints>

using namespace mu::ui;

QmlToolTip::QmlToolTip(QObject* parent)
    : QObject(parent)
{
    connect(&m_openTimer, &QTimer::timeout, this, &QmlToolTip::doShow);

    m_closeTimer.setSingleShot(true);
    connect(&m_closeTimer, &QTimer::timeout, this, &QmlToolTip::doHide);
}

void QmlToolTip::show(QQuickItem* item, const QString& title, const QString& description, const QString& shortcut)
{
    if (item == m_item) {
        m_closeTimer.stop();
        return;
    }

    m_title = title;
    m_description = description;
    m_shortcut = shortcut;

    bool toolTipNotOpened = m_item == nullptr;
    bool openTimerStarted = m_openTimer.isActive();
    if (toolTipNotOpened || openTimerStarted) {
        m_item = item;
        connect(m_item, &QObject::destroyed, this, &QmlToolTip::doHide);

        const int interval = item ? qApp->styleHints()->mousePressAndHoldInterval() : 100;
        m_openTimer.start(interval);
    } else {
        m_item = item;
        doShow();
    }

    m_shouldBeClosed = false;
}

void QmlToolTip::hide(QQuickItem* item, bool force)
{
    if (m_item != item) {
        return;
    }

    m_shouldBeClosed = true;

    if (force) {
        doHide();
        return;
    }

    m_closeTimer.start(500);
}

void QmlToolTip::doShow()
{
    m_openTimer.stop();

    if (!m_item) {
        return;
    }

    emit showToolTip(m_item, m_title, m_description, m_shortcut);
}

void QmlToolTip::doHide()
{
    if (!m_shouldBeClosed) {
        return;
    }

    disconnect(m_item, &QObject::destroyed, this, &QmlToolTip::doHide);

    m_openTimer.stop();
    m_item = nullptr;
    m_title = QString();
    m_description = QString();
    m_shortcut = QString();

    emit hideToolTip();
}
