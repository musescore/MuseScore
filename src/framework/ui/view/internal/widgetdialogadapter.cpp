/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
#include "widgetdialogadapter.h"

#include <QGuiApplication>

#include "log.h"

using namespace muse::ui;

WidgetDialogAdapter::WidgetDialogAdapter(QDialog* parent, QWindow* window, bool staysOnTop)
    : QObject(parent), m_dialog(parent), m_window(window), m_staysOnTop(staysOnTop)
{
    IF_ASSERT_FAILED(m_dialog) {
        return;
    }

    m_dialog->setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    m_dialog->installEventFilter(this);

#ifdef Q_OS_MAC
    if (m_staysOnTop) {
        updateStayOnTopHint();
        connect(qApp, &QApplication::applicationStateChanged, this, &WidgetDialogAdapter::updateStayOnTopHint);
    }
#endif
}

// We want some windows to be on top of the main window.
// But not on top of all other applications when MuseScore isn't active.
// On Windows, we achieve this by setting the transient parent.
// On macOS, we have to use a workaround:
// When the application becomes active, the windows will get the StayOnTop hint.
// and when the application becomes inactive, the hint will be removed.
#ifdef Q_OS_MAC
void WidgetDialogAdapter::updateStayOnTopHint()
{
    IF_ASSERT_FAILED(m_dialog) {
        return;
    }

    bool stay = qApp->applicationState() == Qt::ApplicationActive;

    bool wasShown = m_dialog->isVisible();
    bool wasActive = m_dialog->isActiveWindow();

    m_dialog->setWindowFlag(Qt::WindowStaysOnTopHint, stay);
    if (wasShown) {
        if (!wasActive) {
            m_dialog->setAttribute(Qt::WA_ShowWithoutActivating, true);
        }
        m_dialog->show();
        m_dialog->setAttribute(Qt::WA_ShowWithoutActivating, false);
    }
}

#endif

bool WidgetDialogAdapter::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::Show && m_onShownCallBack) {
#ifndef Q_OS_MAC
        if (m_staysOnTop) {
            m_dialog->windowHandle()->setTransientParent(m_window);
        }
#endif

        m_onShownCallBack();
    }

    if (event->type() == QEvent::Hide && m_onHideCallBack) {
        m_onHideCallBack();
    }

    return QObject::eventFilter(watched, event);
}

WidgetDialogAdapter& WidgetDialogAdapter::onShow(const std::function<void()>& func)
{
    m_onShownCallBack = func;
    return *this;
}

WidgetDialogAdapter& WidgetDialogAdapter::onHide(const std::function<void()>& func)
{
    m_onHideCallBack = func;
    return *this;
}
