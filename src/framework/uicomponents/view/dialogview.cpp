/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include <QApplication>

#include "log.h"

using namespace Qt::Literals::StringLiterals;

using namespace muse::uicomponents;

static const int DIALOG_WINDOW_FRAME_HEIGHT(20);

DialogView::DialogView(QQuickItem* parent)
    : WindowView(parent)
{
    setObjectName("DialogView");
    setRetCode(Ret::Code::Ok);
}

bool DialogView::isDialog() const
{
    return true;
}

void DialogView::beforeOpen()
{
    QWindow* qWindow = m_window->qWindow();
    IF_ASSERT_FAILED(qWindow) {
        return;
    }

    //! NOTE Set default title
    if (m_title.isEmpty()) {
        setTitle(application()->title());
    }

    qWindow->setTitle(m_title);

    if (m_alwaysOnTop) {
#ifdef Q_OS_MAC
        auto updateStayOnTopHint = [this]() {
            bool stay = qApp->applicationState() == Qt::ApplicationActive;
            m_window->qWindow()->setFlag(Qt::WindowStaysOnTopHint, stay);
        };
        updateStayOnTopHint();
        connect(qApp, &QApplication::applicationStateChanged, this, updateStayOnTopHint);
#endif
    } else {
        qWindow->setModality(m_modal ? Qt::ApplicationModal : Qt::NonModal);
    }

    qWindow->setFlag(Qt::FramelessWindowHint, m_frameless);
#ifdef MUSE_MODULE_UI_DISABLE_MODALITY
    qWindow->setModality(Qt::NonModal);
#endif
    m_window->setResizable(m_resizable);

    //! NOTE ok will be if they call accept
    setRetCode(Ret::Code::Cancel);

    windowsController()->regWindow(qWindow->winId());
}

void DialogView::onHidden()
{
    WindowView::onHidden();

    activateNavigationParentControl();
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

void DialogView::updateGeometry()
{
    const QScreen* screen = resolveScreen();
    QRect anchorRect = screen->availableGeometry();

    const QWindow* qMainWindow = mainWindow()->qWindow();
    bool mainWindowVisible = qMainWindow->isVisible();
    QRect referenceRect = qMainWindow->geometry();

    if (referenceRect.isEmpty() || !mainWindowVisible) {
        referenceRect = anchorRect;
    }

    QRect dlgRect = viewGeometry();

    // position the dialog in the center of the main window
    dlgRect.moveLeft(referenceRect.x() + (referenceRect.width() - dlgRect.width()) / 2);
    dlgRect.moveTop(referenceRect.y() + (referenceRect.height() - dlgRect.height()) / 2 + DIALOG_WINDOW_FRAME_HEIGHT);

    dlgRect.moveLeft(dlgRect.x());
    dlgRect.moveTop(dlgRect.y());

    // try to move the dialog if it doesn't fit on the screen

    int titleBarHeight = QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight);

    if (dlgRect.left() <= anchorRect.left()) {
        dlgRect.moveLeft(anchorRect.left() + DIALOG_WINDOW_FRAME_HEIGHT);
    }

    if (dlgRect.top() - titleBarHeight <= anchorRect.top()) {
        dlgRect.moveTop(anchorRect.top() + titleBarHeight + DIALOG_WINDOW_FRAME_HEIGHT);
    }

    if (dlgRect.right() >= anchorRect.right()) {
        dlgRect.moveRight(anchorRect.right() - DIALOG_WINDOW_FRAME_HEIGHT);
    }

    if (dlgRect.bottom() >= anchorRect.bottom()) {
        dlgRect.moveBottom(anchorRect.bottom() - DIALOG_WINDOW_FRAME_HEIGHT);
    }

    // if after moving the dialog does not fit on the screen, then adjust the size of the dialog
    if (!anchorRect.contains(dlgRect)) {
        anchorRect -= QMargins(DIALOG_WINDOW_FRAME_HEIGHT, DIALOG_WINDOW_FRAME_HEIGHT + titleBarHeight,
                               DIALOG_WINDOW_FRAME_HEIGHT, DIALOG_WINDOW_FRAME_HEIGHT);
        dlgRect = anchorRect.intersected(dlgRect);
    }

    m_globalPos = dlgRect.topLeft();

    setContentWidth(dlgRect.width());
    setContentHeight(dlgRect.height());
}

QRect DialogView::viewGeometry() const
{
    return QRect(m_globalPos.toPoint(), QSize(contentWidth(), contentHeight()));
}

QString DialogView::title() const
{
    return m_title;
}

void DialogView::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    if (qWindow()) {
        qWindow()->setTitle(title);
    }

    emit titleChanged(m_title);
}

QString DialogView::objectId() const
{
    return m_objectId;
}

void DialogView::setObjectId(QString objectId)
{
    if (m_objectId == objectId) {
        return;
    }

    m_objectId = objectId;
    emit objectIdChanged(m_objectId);
}

bool DialogView::modal() const
{
    return m_modal;
}

void DialogView::setModal(bool modal)
{
    if (m_modal == modal) {
        return;
    }

    m_modal = modal;
    emit modalChanged(m_modal);
}

bool DialogView::frameless() const
{
    return m_frameless;
}

void DialogView::setFrameless(bool frameless)
{
    if (m_frameless == frameless) {
        return;
    }

    m_frameless = frameless;
    emit framelessChanged(m_frameless);
}

bool DialogView::resizable() const
{
    return m_window ? m_window->resizable() : m_resizable;
}

void DialogView::setResizable(bool resizable)
{
    if (this->resizable() == resizable) {
        return;
    }

    m_resizable = resizable;
    if (m_window) {
        m_window->setResizable(m_resizable);
    }
    emit resizableChanged(m_resizable);
}

bool DialogView::alwaysOnTop() const
{
    return m_alwaysOnTop;
}

void DialogView::setAlwaysOnTop(bool alwaysOnTop)
{
    if (m_alwaysOnTop == alwaysOnTop) {
        return;
    }

    m_alwaysOnTop = alwaysOnTop;
    emit alwaysOnTopChanged();
}

QVariantMap DialogView::ret() const
{
    return m_ret;
}

void DialogView::setRet(QVariantMap ret)
{
    if (m_ret == ret) {
        return;
    }

    m_ret = ret;
    emit retChanged(m_ret);
}

void DialogView::setRetCode(Ret::Code code)
{
    QVariantMap ret;
    ret[u"errcode"_s] = static_cast<int>(code);
    setRet(ret);
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
    setRetCode(Ret::Code::Ok);
    close();
}

void DialogView::reject(int code)
{
    if (code > 0) {
        setRetCode(static_cast<Ret::Code>(code));
    } else {
        setRetCode(Ret::Code::Cancel);
    }

    close();
}
