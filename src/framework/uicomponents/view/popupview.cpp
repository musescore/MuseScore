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

#include "popupview.h"

#include <functional>
#include <QQuickView>
#include <QQuickWidget>
#include <QQmlEngine>
#include <QUrl>
#include <QQmlContext>
#include <QApplication>
#include <QMainWindow>
#include <QTimer>

#include "popupwindow/popupwindow_qquickview.h"

#include "log.h"

using namespace mu::uicomponents;

PopupView::PopupView(QQuickItem* parent)
    : QObject(parent)
{
    setObjectName("PopupView");
    setErrCode(Ret::Code::Ok);

    qApp->installEventFilter(this);
    connect(qApp, &QApplication::applicationStateChanged, this, &PopupView::onApplicationStateChanged);
}

QQuickItem* PopupView::parentItem() const
{
    return qobject_cast<QQuickItem*>(parent());
}

void PopupView::setParentItem(QQuickItem* parent)
{
    if (parentItem() == parent) {
        return;
    }

    QObject::setParent(parent);
    emit parentItemChanged();
}

void PopupView::forceActiveFocus()
{
    IF_ASSERT_FAILED(m_window) {
        return;
    }
    m_window->forceActiveFocus();
}

bool PopupView::isDialog() const
{
    return false;
}

void PopupView::classBegin()
{
}

void PopupView::componentComplete()
{
    QQmlEngine* engine = qmlEngine(this);
    IF_ASSERT_FAILED(engine) {
        return;
    }

    m_window = new PopupWindow_QQuickView();
    m_window->init(engine, uiConfiguration(), isDialog());
    m_window->setOnHidden([this]() { onHidden(); });
    m_window->setContent(m_contentItem);
}

bool PopupView::eventFilter(QObject* watched, QEvent* event)
{
    if (QEvent::MouseButtonPress == event->type()) {
        mousePressEvent(static_cast<QMouseEvent*>(event));
    } else if (QEvent::MouseButtonRelease == event->type()) {
        mouseReleaseEvent(static_cast<QMouseEvent*>(event));
    } else if (QEvent::Close == event->type() && watched == mainWindow()->qMainWindow()) {
        close();
    }

    return QObject::eventFilter(watched, event);
}

QWindow* PopupView::qWindow() const
{
    return m_window ? m_window->qWindow() : nullptr;
}

void PopupView::beforeShow()
{
}

void PopupView::open()
{
    if (isOpened()) {
        return;
    }

    IF_ASSERT_FAILED(m_window) {
        return;
    }

    beforeShow();

    if (m_globalPos.isNull()) {
        QQuickItem* prn = parentItem();
        IF_ASSERT_FAILED(prn) {
            return;
        }
        m_globalPos = prn->mapToGlobal(m_localPos);
    }

    if (isDialog()) {
        QWindow* qWindow = m_window->qWindow();
        IF_ASSERT_FAILED(qWindow) {
            return;
        }
        qWindow->setTitle(m_title);
        qWindow->setModality(m_modal ? Qt::ApplicationModal : Qt::NonModal);

        QRect winRect = m_window->geometry();
        qWindow->setMinimumSize(winRect.size());
        if (!m_resizable) {
            qWindow->setMaximumSize(winRect.size());
        }
    }

    m_window->show(m_globalPos.toPoint());

    m_globalPos = QPointF(); // invalidate

    if (!m_navigationParentControl) {
        ui::INavigationControl* ctrl = navigationController()->activeControl();
        //! NOTE At the moment we have only qml navigation controls
        QObject* qmlCtrl = dynamic_cast<QObject*>(ctrl);
        setNavigationParentControl(qmlCtrl);
    }

    emit isOpenedChanged();
    emit opened();
}

void PopupView::onHidden()
{
    emit isOpenedChanged();
    emit closed();
}

void PopupView::close()
{
    if (!isOpened()) {
        return;
    }

    IF_ASSERT_FAILED(m_window) {
        return;
    }

    m_window->hide();
}

void PopupView::toggleOpened()
{
    if (isOpened()) {
        close();
    } else {
        open();
    }
}

bool PopupView::isOpened() const
{
    return m_window ? m_window->isVisible() : false;
}

PopupView::ClosePolicy PopupView::closePolicy() const
{
    return m_closePolicy;
}

QObject* PopupView::navigationParentControl() const
{
    return m_navigationParentControl;
}

void PopupView::setNavigationParentControl(QObject* navigationParentControl)
{
    if (m_navigationParentControl == navigationParentControl) {
        return;
    }

    m_navigationParentControl = navigationParentControl;
    emit navigationParentControlChanged(m_navigationParentControl);
}

void PopupView::setContentItem(QQuickItem* content)
{
    if (m_contentItem == content) {
        return;
    }

    m_contentItem = content;
    emit contentItemChanged();
}

QQuickItem* PopupView::contentItem() const
{
    return m_contentItem;
}

qreal PopupView::localX() const
{
    return m_localPos.x();
}

qreal PopupView::localY() const
{
    return m_localPos.y();
}

QRect PopupView::geometry() const
{
    return m_window->geometry();
}

void PopupView::setLocalX(qreal x)
{
    if (qFuzzyCompare(m_localPos.x(), x)) {
        return;
    }

    m_localPos.setX(x);
    emit xChanged(x);
}

void PopupView::setLocalY(qreal y)
{
    if (qFuzzyCompare(m_localPos.y(), y)) {
        return;
    }

    m_localPos.setY(y);
    emit yChanged(y);
}

void PopupView::setClosePolicy(ClosePolicy closePolicy)
{
    if (m_closePolicy == closePolicy) {
        return;
    }

    m_closePolicy = closePolicy;
    emit closePolicyChanged(closePolicy);
}

void PopupView::onApplicationStateChanged(Qt::ApplicationState state)
{
    if (m_closePolicy == NoAutoClose) {
        return;
    }

    if (state != Qt::ApplicationActive) {
        close();
    }
}

void PopupView::mousePressEvent(QMouseEvent* event)
{
    if (!isOpened()) {
        return;
    }

    if (m_closePolicy == ClosePolicy::CloseOnPressOutsideParent) {
        if (!isMouseWithinBoundaries(event->globalPos())) {
            close();
        }
    }
}

void PopupView::mouseReleaseEvent(QMouseEvent* event)
{
    if (!isOpened()) {
        return;
    }

    if (m_closePolicy == ClosePolicy::CloseOnReleaseOutsideParent) {
        if (!isMouseWithinBoundaries(event->globalPos())) {
            close();
        }
    }
}

bool PopupView::isMouseWithinBoundaries(const QPoint& mousePos) const
{
    QRect viewRect = m_window->geometry();
    bool contains = viewRect.contains(mousePos);
    if (!contains) {
        //! NOTE We also check the parent because often clicking on the parent should toggle the popup,
        //! but if we don't check a parent here, the popup will be closed and reopened.
        QQuickItem* prn = parentItem();
        QPointF localPos = prn->mapFromGlobal(mousePos);
        QRectF parentRect = QRectF(prn->x(), prn->y(), prn->width(), prn->height());
        contains = parentRect.contains(localPos);
    }

    return contains;
}

void PopupView::setObjectID(QString objectID)
{
    if (m_objectID == objectID) {
        return;
    }

    m_objectID = objectID;
    emit objectIDChanged(m_objectID);
}

QString PopupView::objectID() const
{
    return m_objectID;
}

QString PopupView::title() const
{
    return m_title;
}

void PopupView::setTitle(QString title)
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

bool PopupView::modal() const
{
    return m_modal;
}

void PopupView::setModal(bool modal)
{
    if (m_modal == modal) {
        return;
    }

    m_modal = modal;
    emit modalChanged(m_modal);
}

void PopupView::setResizable(bool resizable)
{
    if (m_resizable == resizable) {
        return;
    }

    m_resizable = resizable;
    emit resizableChanged(m_resizable);
}

bool PopupView::resizable() const
{
    return m_resizable;
}

void PopupView::setRet(QVariantMap ret)
{
    if (m_ret == ret) {
        return;
    }

    m_ret = ret;
    emit retChanged(m_ret);
}

QVariantMap PopupView::ret() const
{
    return m_ret;
}

void PopupView::setErrCode(Ret::Code code)
{
    QVariantMap ret;
    ret["errcode"] = static_cast<int>(code);
    setRet(ret);
}
