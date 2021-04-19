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

#include <QQmlEngine>
#include <QUrl>
#include <QQmlContext>
#include <QApplication>
#include <QMainWindow>

#include "log.h"

using namespace mu::uicomponents;

PopupView::PopupView(QQuickItem* parent)
    : QObject(parent)
{
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
    IF_ASSERT_FAILED(m_view) {
        return;
    }
    m_view->requestActivate();
}

void PopupView::open()
{
    if (isOpened()) {
        return;
    }

    IF_ASSERT_FAILED(m_view) {
        return;
    }

    QQuickItem* prn = parentItem();
    IF_ASSERT_FAILED(prn) {
        return;
    }

    QPointF pos = prn->position();
    pos.setY(pos.y() + m_localPos.y());
    pos.setX(pos.x() + m_localPos.x());

    QPointF global = prn->mapToGlobal(pos);

    m_view->setPosition(global.toPoint());
    m_view->show();
    m_view->requestActivate();
    m_view->setKeyboardGrabEnabled(true);

    emit isOpenedChanged();
    emit opened();
}

void PopupView::close()
{
    if (!isOpened()) {
        return;
    }

    IF_ASSERT_FAILED(m_view) {
        return;
    }

    m_view->hide();
    m_view->setKeyboardGrabEnabled(false);

    emit isOpenedChanged();
    emit closed();
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
    return m_view ? m_view->isVisible() : false;
}

PopupView::ClosePolicy PopupView::closePolicy() const
{
    return m_closePolicy;
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

qreal PopupView::x() const
{
    return m_localPos.x();
}

qreal PopupView::y() const
{
    return m_localPos.y();
}

void PopupView::setX(qreal x)
{
    if (qFuzzyCompare(m_localPos.x(), x)) {
        return;
    }

    m_localPos.setX(x);
    emit xChanged(x);
}

void PopupView::setY(qreal y)
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

void PopupView::classBegin()
{
}

void PopupView::componentComplete()
{
    QQmlEngine* engine = qmlEngine(this);
    IF_ASSERT_FAILED(engine) {
        return;
    }

    m_view = new QQuickView(engine, nullptr);
    m_view->setFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    m_view->setResizeMode(QQuickView::SizeViewToRootObject);
    m_view->setColor(QColor(0, 0, 0, 0)); // transparent
    m_view->setContent(QUrl(), nullptr, m_contentItem);
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

void PopupView::onApplicationStateChanged(Qt::ApplicationState state)
{
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
    QRect viewRect = m_view->geometry();
    return viewRect.contains(mousePos);
}
