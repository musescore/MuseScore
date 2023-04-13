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

static constexpr int DEFAULT_OPEN_INTERVAL = 500;
static constexpr int DEFAULT_CLOSE_INTERVAL = 500;
static constexpr int FILETOOLTIP_OPEN_INTERVAL = 500;
static constexpr int FILETOOLTIP_CLOSE_INTERVAL = 100;

using namespace mu::ui;

QmlToolTip::QmlToolTip(QObject* parent)
    : QObject(parent)
{
    connect(&m_openTimer, &QTimer::timeout, this, &QmlToolTip::doShow);

    m_closeTimer.setSingleShot(true);
    connect(&m_closeTimer, &QTimer::timeout, this, &QmlToolTip::doHide);

    qApp->installEventFilter(this);
}

void QmlToolTip::show(QQuickItem* item, const QString& title, const QString& description, const QString& shortcut,
                      const ToolTipType& toolTipType)
{
    if (item == m_item) {
        m_closeTimer.stop();
        return;
    }

    m_title = title;
    m_description = description;
    m_shortcut = shortcut;
    m_toolTipType = toolTipType;

    bool toolTipNotOpened = m_item == nullptr;
    bool openTimerStarted = m_openTimer.isActive();

    m_item = item;
    m_shouldBeClosed = false;

    if (toolTipNotOpened || !openTimerStarted) {
        connect(m_item, &QObject::destroyed, this, &QmlToolTip::doHide);

        m_openTimer.start(m_toolTipType == Default ? DEFAULT_OPEN_INTERVAL : FILETOOLTIP_OPEN_INTERVAL);
    } else {
        doShow();
    }
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

    m_closeTimer.start(m_toolTipType == Default ? DEFAULT_CLOSE_INTERVAL : FILETOOLTIP_CLOSE_INTERVAL);
}

void QmlToolTip::doShow()
{
    m_openTimer.stop();
    m_closeTimer.stop();

    if (!m_item) {
        return;
    }

    if (m_shouldBeClosed) {
        m_item = nullptr;
        m_shouldBeClosed = false;
        return;
    }

    emit showToolTip(m_item, m_title, m_description, m_shortcut, m_toolTipType);
}

void QmlToolTip::doHide()
{
    if (!m_shouldBeClosed) {
        return;
    }

    if (m_item) {
        disconnect(m_item, &QObject::destroyed, this, &QmlToolTip::doHide);
    }

    m_openTimer.stop();
    m_closeTimer.stop();

    m_item = nullptr;
    m_title = QString();
    m_description = QString();
    m_shortcut = QString();
    m_toolTipType = Default;

    emit hideToolTip();
}

bool QmlToolTip::eventFilter(QObject*, QEvent* event)
{
    if (event->type() == QEvent::Wheel) {
        m_shouldBeClosed = true;
        doHide();
    }

    return false;
}
