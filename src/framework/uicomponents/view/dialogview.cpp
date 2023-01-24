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

#include "dialogview.h"

#include <QStyle>
#include <QWindow>
#include <QScreen>

#include "log.h"

using namespace mu::uicomponents;

static const int DIALOG_WINDOW_FRAME_HEIGHT(20);

DialogView::DialogView(QQuickItem* parent)
    : PopupView(parent)
{
    setObjectName("DialogView");
    setClosePolicy(NoAutoClose);
}

bool DialogView::isDialog() const
{
    return true;
}

void DialogView::onHidden()
{
    PopupView::onHidden();

    if (m_loop.isRunning()) {
        m_loop.exit();
    }
}

QScreen* DialogView::resolveScreen() const
{
    QWindow* qMainWindow = mainWindow()->qWindow();
    QScreen* mainWindowScreen = qMainWindow->screen();
    if (!mainWindowScreen) {
        mainWindowScreen = QGuiApplication::primaryScreen();
    }

    return mainWindowScreen;
}

void DialogView::updatePosition()
{
    QScreen* screen = resolveScreen();
    QRect anchorRect = screen->availableGeometry();

    QRect referenceRect = mainWindow()->qWindow()->geometry();
    if (referenceRect.isEmpty()) {
        referenceRect = anchorRect;
    }

    const QRect& dlgRect = geometry();

    m_globalPos.setX(referenceRect.x() + (referenceRect.width() - dlgRect.width()) / 2);
    m_globalPos.setY(referenceRect.y() + (referenceRect.height() - dlgRect.height()) / 2 + DIALOG_WINDOW_FRAME_HEIGHT);

    m_globalPos.setX(m_globalPos.x() + m_localPos.x());
    m_globalPos.setY(m_globalPos.y() + m_localPos.y());

    int titleBarHeight = QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight);

    if (m_globalPos.x() < anchorRect.x()) {
        m_globalPos.setX(anchorRect.x() + DIALOG_WINDOW_FRAME_HEIGHT);
    }

    if (m_globalPos.y() - titleBarHeight < anchorRect.y()) {
        m_globalPos.setY(anchorRect.y() + titleBarHeight + DIALOG_WINDOW_FRAME_HEIGHT);
    }

    if (m_globalPos.x() + dlgRect.width() > anchorRect.right()) {
        m_globalPos.setX(anchorRect.right() - dlgRect.width() - DIALOG_WINDOW_FRAME_HEIGHT);
    }

    if (m_globalPos.y() + dlgRect.height() > anchorRect.bottom()) {
        m_globalPos.setY(anchorRect.bottom() - dlgRect.height() - DIALOG_WINDOW_FRAME_HEIGHT);
    }

    //! NOTE ok will be if they call accept
    setErrCode(Ret::Code::Cancel);
}

void DialogView::exec()
{
    open();
    m_loop.exec();

    activateNavigationParentControl();
}

void DialogView::show()
{
    open();
}

void DialogView::hide()
{
    close();
}

void DialogView::raise()
{
    if (isOpened()) {
        m_window->raise();
    }
}

void DialogView::accept()
{
    setErrCode(Ret::Code::Ok);
    close();
}

void DialogView::reject(int code)
{
    if (code > 0) {
        setErrCode(static_cast<Ret::Code>(code));
    } else {
        setErrCode(Ret::Code::Cancel);
    }

    close();
}
