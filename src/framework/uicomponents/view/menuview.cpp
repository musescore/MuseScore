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

#include "menuview.h"

#include <QDateTime>

#include "log.h"

using namespace muse::uicomponents;

static const QString MENU_VIEW_CONTENT_OBJECT_NAME("_MenuViewContent");

MenuView::MenuView(QQuickItem* parent)
    : PopupView(parent)
{
    setObjectName("MenuView");

    setShowArrow(false);
    setPadding(8);

    // Initialize mouse stopped timer for amazon triangle
    m_mouseStoppedTimer = new QTimer(this);
    m_mouseStoppedTimer->setInterval(100); // Check every 100ms
    m_mouseStoppedTimer->setSingleShot(false);
    connect(m_mouseStoppedTimer, &QTimer::timeout, this, &MenuView::onMouseStoppedTimer);
    m_mouseStoppedTimer->start();
}

void MenuView::initCloseController()
{
    PopupView::initCloseController();

    // Set callback for mouse move tracking
    if (m_closeController) {
        m_closeController->setMouseMoveCallback([this](const QPointF& pos) {
            onMouseMove(pos);
        });
    }
}

int MenuView::viewVerticalMargin() const
{
    return 4;
}

Qt::AlignmentFlag MenuView::cascadeAlign() const
{
    return m_cascadeAlign;
}

void MenuView::setCascadeAlign(Qt::AlignmentFlag cascadeAlign)
{
    if (m_cascadeAlign == cascadeAlign) {
        return;
    }

    m_cascadeAlign = cascadeAlign;
    emit cascadeAlignChanged(m_cascadeAlign);
}

void MenuView::componentComplete()
{
    m_contentItem->setObjectName(m_contentItem->objectName() + MENU_VIEW_CONTENT_OBJECT_NAME);

    PopupView::componentComplete();
}

void MenuView::updateGeometry()
{
    const QQuickItem* parent = parentItem();
    IF_ASSERT_FAILED(parent) {
        return;
    }

    QPointF parentTopLeft = parent->mapToGlobal(QPoint(0, 0));

    if (m_globalPos.isNull()) {
        m_globalPos = parentTopLeft + m_localPos;
    }

    QRectF anchorRect = anchorGeometry();
    QRectF viewRect = viewGeometry();

    setPopupPosition(PopupPosition::Bottom);
    setCascadeAlign(Qt::AlignmentFlag::AlignRight);

    auto movePos = [this, &viewRect](qreal x, qreal y) {
        m_globalPos.setX(x);
        m_globalPos.setY(y);

        viewRect.moveTopLeft(m_globalPos);
    };

    const QQuickItem* parentMenuContentItem = this->parentMenuContentItem();
    bool isCascade = parentMenuContentItem != nullptr;

    if (isCascade) {
        movePos(parentTopLeft.x() + parent->width(), m_globalPos.y() - parent->height() - viewVerticalMargin());
    }

    if (viewRect.left() < anchorRect.left()) {
        // move to the right to an area that doesn't fit
        movePos(m_globalPos.x() + anchorRect.left() - viewRect.left(), m_globalPos.y());
    }

    if (viewRect.bottom() > anchorRect.bottom()) {
        if (isCascade) {
            // move to the top to an area that doesn't fit
            movePos(m_globalPos.x(), m_globalPos.y() - (viewRect.bottom() - anchorRect.bottom()));
        } else {
            qreal newY = parentTopLeft.y() - viewRect.height();
            if (anchorRect.top() < newY) {
                // move to the top of the parent
                movePos(m_globalPos.x(), newY);
                setPopupPosition(PopupPosition::Top);
            } else {
                // move to the right of the parent and move to top to an area that doesn't fit
                movePos(parentTopLeft.x() + parent->width(), m_globalPos.y() - (viewRect.bottom() - anchorRect.bottom()));
            }
        }
    }

    Qt::AlignmentFlag parentCascadeAlign = this->parentCascadeAlign(parentMenuContentItem);
    if (viewRect.right() > anchorRect.right() || parentCascadeAlign != Qt::AlignmentFlag::AlignRight) {
        if (isCascade) {
            // move to the right of the parent
            movePos(parentTopLeft.x() - viewRect.width() + padding() * 2, m_globalPos.y());
            setCascadeAlign(Qt::AlignmentFlag::AlignLeft);
        } else {
            // move to the left to an area that doesn't fit
            movePos(m_globalPos.x() - (viewRect.right() - anchorRect.right()) + padding() * 2, m_globalPos.y());
        }
    }

    // remove padding for arrow
    movePos(m_globalPos.x() - padding(), m_globalPos.y());

    updateContentPosition();
}

void MenuView::updateContentPosition()
{
    if (popupPosition() == PopupPosition::Top) {
        contentItem()->setY(padding());
    } else {
        contentItem()->setY(-padding());
    }
}

QRect MenuView::viewGeometry() const
{
    return QRect(m_globalPos.toPoint(), QSize(contentWidth() + padding() * 2, contentHeight() + padding() * 2));
}

Qt::AlignmentFlag MenuView::parentCascadeAlign(const QQuickItem* parent) const
{
    if (!parent) {
        return Qt::AlignmentFlag::AlignRight;
    }

    return static_cast<Qt::AlignmentFlag>(parent->property("cascadeAlign").toInt());
}

QQuickItem* MenuView::parentMenuContentItem() const
{
    QQuickItem* parent = parentItem();
    while (parent) {
        if (parent->objectName().contains(MENU_VIEW_CONTENT_OBJECT_NAME)) {
            return parent;
        }

        parent = parent->parentItem();
    }

    return nullptr;
}

int MenuView::contentWidth() const
{
    return m_contentWidth;
}

void MenuView::setContentWidth(int newContentWidth)
{
    if (m_contentWidth == newContentWidth) {
        return;
    }

    m_contentWidth = newContentWidth;
    emit contentWidthChanged();
}

int MenuView::contentHeight() const
{
    return m_contentHeight;
}

void MenuView::setContentHeight(int newContentHeight)
{
    if (m_contentHeight == newContentHeight) {
        return;
    }

    m_contentHeight = newContentHeight;
    emit contentHeightChanged();
}

QPointF MenuView::triangleP1() const
{
    return m_triangleP1;
}

void MenuView::setTriangleP1(const QPointF& point)
{
    if (m_triangleP1 == point) {
        return;
    }

    m_triangleP1 = point;
    emit triangleP1Changed();
    updateCloseControllerTriangle();
}

QPointF MenuView::triangleP2() const
{
    return m_triangleP2;
}

void MenuView::setTriangleP2(const QPointF& point)
{
    if (m_triangleP2 == point) {
        return;
    }

    m_triangleP2 = point;
    emit triangleP2Changed();
    updateCloseControllerTriangle();
}

QPointF MenuView::triangleP3() const
{
    return m_triangleP3;
}

void MenuView::setTriangleP3(const QPointF& point)
{
    if (m_triangleP3 == point) {
        return;
    }

    m_triangleP3 = point;
    emit triangleP3Changed();
    updateCloseControllerTriangle();
}

bool MenuView::amazonTriangleActive() const
{
    return m_amazonTriangleActive;
}

void MenuView::setAmazonTriangleActive(bool active)
{
    if (m_amazonTriangleActive == active) {
        return;
    }

    m_amazonTriangleActive = active;
    emit amazonTriangleActiveChanged();
    updateCloseControllerTriangle();
}

void MenuView::updateCloseControllerTriangle()
{
    // Update the close controller with current triangle state
    if (m_closeController) {
        m_closeController->setAmazonTriangle(m_triangleP1, m_triangleP2, m_triangleP3, m_amazonTriangleActive);
    }
}

void MenuView::setSubMenuGeometry(const QRectF& geometry)
{
    m_subMenuGeometry = geometry;
    m_hasSubMenuOpen = true;

    // Recalculate triangle with current mouse position
    if (!m_lastMousePos.isNull()) {
        updateTriangle(m_lastMousePos);
    }
}

void MenuView::clearSubMenuGeometry()
{
    m_hasSubMenuOpen = false;
    m_subMenuGeometry = QRectF();

    // Deactivate triangle when submenu closes
    if (m_amazonTriangleActive) {
        m_amazonTriangleActive = false;
        emit amazonTriangleActiveChanged();
        updateCloseControllerTriangle();
    }
}

bool MenuView::isMouseInsideTriangle(const QPointF& mousePos) const
{
    if (!m_amazonTriangleActive) {
        return false;
    }

    // Helper lambda to calculate cross product sign
    auto sign = [](const QPointF& p1, const QPointF& p2, const QPointF& p3) -> qreal {
        return (p1.x() - p3.x()) * (p2.y() - p3.y()) - (p2.x() - p3.x()) * (p1.y() - p3.y());
    };

    qreal d1 = sign(mousePos, m_triangleP1, m_triangleP2);
    qreal d2 = sign(mousePos, m_triangleP2, m_triangleP3);
    qreal d3 = sign(mousePos, m_triangleP3, m_triangleP1);

    bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    // Point is inside if all cross products have the same sign
    return !(hasNeg && hasPos);
}

void MenuView::onMouseMove(const QPointF& position)
{
    if (position == m_lastMousePos) {
        return;
    }

    m_lastMousePos = position;
    m_lastMouseMoveTime = QDateTime::currentMSecsSinceEpoch();
    m_mouseStopped = false;
}

void MenuView::onMouseStoppedTimer()
{
    if (m_lastMouseMoveTime == 0) {
        return;
    }

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 timeSinceLastMove = now - m_lastMouseMoveTime;

    if (timeSinceLastMove > 350 && !m_mouseStopped) {
        m_mouseStopped = true;
        updateTriangle(m_lastMousePos);
    } else if (timeSinceLastMove <= 350 && m_mouseStopped) {
        m_mouseStopped = false;
    }
}

void MenuView::updateTriangle(const QPointF& mousePos)
{
    if (!m_hasSubMenuOpen || !m_mouseStopped) {
        return;
    }

    calculateTriangleVertices(mousePos);

    // Activate triangle
    if (!m_amazonTriangleActive) {
        m_amazonTriangleActive = true;
        emit amazonTriangleActiveChanged();
    }

    updateCloseControllerTriangle();
}

void MenuView::calculateTriangleVertices(const QPointF& p1)
{
    // Set P1 to mouse position
    if (m_triangleP1 != p1) {
        m_triangleP1 = p1;
        emit triangleP1Changed();
    }

    if (m_hasSubMenuOpen && !m_subMenuGeometry.isEmpty()) {
        // Calculate P2 and P3 based on submenu position
        // P2 is top-left corner, P3 is bottom-left corner
        QPointF p2(m_subMenuGeometry.left(), m_subMenuGeometry.top());
        QPointF p3(m_subMenuGeometry.left(), m_subMenuGeometry.bottom());

        if (m_triangleP2 != p2) {
            m_triangleP2 = p2;
            emit triangleP2Changed();
        }

        if (m_triangleP3 != p3) {
            m_triangleP3 = p3;
            emit triangleP3Changed();
        }
    } else {
        // No submenu open, use default points (small triangle around cursor)
        QPointF p2(p1.x() + 1, p1.y() + 1);
        QPointF p3(p1.x() + 1, p1.y() - 1);

        if (m_triangleP2 != p2) {
            m_triangleP2 = p2;
            emit triangleP2Changed();
        }

        if (m_triangleP3 != p3) {
            m_triangleP3 = p3;
            emit triangleP3Changed();
        }
    }
}
