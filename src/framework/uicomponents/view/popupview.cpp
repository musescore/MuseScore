//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "popupview.h"

#include <QQmlEngine>
#include <QUrl>
#include <QQmlContext>
#include <QApplication>

using namespace mu::uicomponents;

PopupView::PopupView(QQuickItem* parent)
    : QQuickItem(parent)
{
    setFlag(QQuickItem::ItemHasContents, true);

    qApp->installEventFilter(this);

    connect(qApp, &QApplication::applicationStateChanged, this, &PopupView::onApplicationStateChanged);
}

void PopupView::open()
{
    if (!m_containerView || m_isOpened) {
        return;
    }

    emit aboutToShow();

    m_containerView->show();

    setIsOpened(true);

    emit opened();
}

void PopupView::close()
{
    if (!m_containerView || !m_isOpened) {
        return;
    }

    emit aboutToHide();

    m_containerView->hide();
    setIsOpened(false);

    emit closed();
}

void PopupView::toggleOpened()
{
    if (m_isOpened) {
        close();
    } else {
        open();
    }
}

QQuickItem* PopupView::backgroundItem() const
{
    return m_backgroundItem;
}

QQuickItem* PopupView::contentItem() const
{
    return m_contentItem;
}

qreal PopupView::positionDisplacementX() const
{
    return m_positionDisplacementX;
}

qreal PopupView::positionDisplacementY() const
{
    return m_positionDisplacementY;
}

QPointF PopupView::globalPos() const
{
    return m_globalPos;
}

PopupView::ClosePolicy PopupView::closePolicy() const
{
    return m_closePolicy;
}

bool PopupView::isOpened() const
{
    return m_isOpened;
}

qreal PopupView::padding() const
{
    return m_padding;
}

void PopupView::setBackgroundItem(QQuickItem* backgroundItem)
{
    if (m_backgroundItem == backgroundItem) {
        return;
    }

    m_backgroundItem = backgroundItem;
    emit backgroundItemChanged(m_backgroundItem);
}

void PopupView::setContentItem(QQuickItem* contentItem)
{
    if (m_contentItem == contentItem) {
        return;
    }

    m_contentItem = contentItem;
    emit contentItemChanged(m_contentItem);
}

void PopupView::setClosePolicy(PopupView::ClosePolicy closePolicy)
{
    if (m_closePolicy == closePolicy) {
        return;
    }

    m_closePolicy = closePolicy;
    emit closePolicyChanged(m_closePolicy);
}

void PopupView::setIsOpened(bool isOpened)
{
    if (m_isOpened == isOpened) {
        return;
    }

    m_isOpened = isOpened;
    emit isOpenedChanged(m_isOpened);
}

void PopupView::setPadding(qreal padding)
{
    if (qFuzzyCompare(m_padding, padding)) {
        return;
    }

    m_padding = padding;
    emit paddingChanged(m_padding);
}

void PopupView::setPositionDisplacementX(qreal positionDisplacementX)
{
    if (qFuzzyCompare(m_positionDisplacementX, positionDisplacementX)) {
        return;
    }

    m_positionDisplacementX = positionDisplacementX;
    setX(positionDisplacementX);
    emit positionDisplacementXChanged(m_positionDisplacementX);
}

void PopupView::setPositionDisplacementY(qreal positionDisplacementY)
{
    if (qFuzzyCompare(m_positionDisplacementY, positionDisplacementY)) {
        return;
    }

    m_positionDisplacementY = positionDisplacementY;
    setY(positionDisplacementY);
    emit positionDisplacementYChanged(m_positionDisplacementY);
}

void PopupView::setGlobalPos(QPointF globalPos)
{
    if (m_globalPos == globalPos) {
        return;
    }

    m_globalPos = globalPos;
    m_containerView->setPosition(globalPos.toPoint());
    emit globalPosChanged(m_globalPos);
}

void PopupView::componentComplete()
{
    QQuickItem::componentComplete();

    QQmlEngine* engine = qmlEngine(this);
    QQmlContext* ctx = qmlContext(this);

    setupContentComponent(engine);
    setupContainerView(engine);
    QQuickItem* obj = loadContentItem(ctx);

    m_containerView->setContent(QUrl(), m_contentComponent, obj);
    setSize(obj->size());
}

bool PopupView::eventFilter(QObject* watched, QEvent* event)
{
    if (QEvent::MouseButtonPress == event->type()) {
        mousePressEvent(static_cast<QMouseEvent*>(event));
    } else if (QEvent::MouseButtonRelease == event->type()) {
        mouseReleaseEvent(static_cast<QMouseEvent*>(event));
    } else if (QEvent::Close == event->type()) {
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
    if (!m_isOpened) {
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
    if (!m_isOpened) {
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
    if (!parentItem()) {
        return false;
    }

    QRectF parentRect = QRectF(parentItem()->x(),
                               parentItem()->y(),
                               parentItem()->width(),
                               parentItem()->height());

    QRectF boundRect = QRectF(x(), y(), width(), height());
    QRectF contentRect = boundRect.united(parentRect);

    QPointF parentGlobalPos = parentItem()->mapToGlobal(parentItem()->position());

    QRectF globalContentRect = QRectF(qMin(globalPos().x(), parentGlobalPos.x()),
                                      qMin(globalPos().y(), parentGlobalPos.y()),
                                      contentRect.width(),
                                      contentRect.height());

    return globalContentRect.contains(mousePos);
}

void PopupView::setupContentComponent(QQmlEngine* engine)
{
    m_contentComponent = new QQmlComponent(engine, this);

    QString dataStr;
    dataStr.append("import QtQuick 2.15\n");
    dataStr.append("import QtQuick.Controls 2.15\n");
    dataStr.append("Control { }");
    m_contentComponent->setData(dataStr.toUtf8(), QUrl());
}

void PopupView::setupContainerView(QQmlEngine* engine)
{
    m_containerView = new QQuickView(engine, nullptr);
    m_containerView->setFlags(Qt::Popup | Qt::FramelessWindowHint);
    m_containerView->setResizeMode(QQuickView::SizeViewToRootObject);
    m_containerView->setColor("#00000000");
}

QQuickItem* PopupView::loadContentItem(QQmlContext* ctx)
{
    QVariantMap initialProperties;
    initialProperties.insert("background", QVariant::fromValue(m_backgroundItem));
    initialProperties.insert("contentItem", QVariant::fromValue(m_contentItem));
    initialProperties.insert("padding", m_padding);
    return qobject_cast<QQuickItem*>(m_contentComponent->createWithInitialProperties(initialProperties, ctx));
}
