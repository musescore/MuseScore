/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#if defined(Q_OS_MAC)
#include "internal/platform/macos/macospopupviewclosecontroller.h"
#elif defined(Q_OS_WIN)
#include "internal/platform/win/winpopupviewclosecontroller.h"
#endif

using namespace muse::uicomponents;

static Qt::WindowFlags resolveWindowFlags()
{
    Qt::WindowFlags flags;
    if (qGuiApp->platformName().contains("wayland")) {
        flags = Qt::Popup;
    } else {
        flags = Qt::Tool;
    }

    static int sIsKde = -1;
    if (sIsKde == -1) {
        QString desktop = qEnvironmentVariable("XDG_CURRENT_DESKTOP").toLower();
        QString session = qEnvironmentVariable("XDG_SESSION_DESKTOP").toLower();
        sIsKde = desktop.contains("kde") || session.contains("kde");
    }
    if (!sIsKde) {
        flags |= Qt::BypassWindowManagerHint;        // Otherwise, it does not work correctly on Gnome (Linux) when resizing)
    }

    flags |= Qt::FramelessWindowHint        // Without border
             | Qt::NoDropShadowWindowHint;   // Without system shadow

    return flags;
}

PopupView::PopupView(QQuickItem* parent)
    : WindowView(parent)
{
    setObjectName("PopupView");

    setShowArrow(true);
    setPadding(12);
}

PopupView::~PopupView()
{
    if (m_closeController) {
        delete m_closeController;
    }
}

void PopupView::initView()
{
    QQuickWindow::setDefaultAlphaBuffer(true);

    WindowView::initView();

    m_view->setFlags(resolveWindowFlags());
    m_view->setColor(Qt::transparent);
}

void PopupView::initCloseController()
{
#if defined(Q_OS_MAC)
    m_closeController = new MacOSPopupViewCloseController(muse::iocCtxForQmlEngine(this->engine()));
#elif defined(Q_OS_WIN)
    m_closeController = new WinPopupViewCloseController(muse::iocCtxForQmlEngine(this->engine()));
#else
    m_closeController = new PopupViewCloseController(muse::iocCtxForQmlEngine(this->engine()));
#endif

    m_closeController->init();

    m_closeController->setParentItem(parentItem());
    m_closeController->setWindow(window());
    m_closeController->setIsCloseOnPressOutsideParent(m_closePolicies & ClosePolicy::CloseOnPressOutsideParent);
    m_closeController->setCanClosed(!m_closePolicies.testFlag(ClosePolicy::NoAutoClose));

    m_closeController->closeNotification().onNotify(this, [this]() {
        close(true);
    });
}

void PopupView::setParentItem(QQuickItem* parent)
{
    WindowView::setParentItem(parent);

    if (m_closeController) {
        m_closeController->setParentItem(parent);
    }
}

void PopupView::beforeOpen()
{
    WindowView::beforeOpen();

    if (!m_closeController) {
        initCloseController();
    }

    m_closeController->setActive(true);

    qApp->installEventFilter(this);
}

void PopupView::onHidden()
{
    WindowView::onHidden();

    if (m_closeController) {
        m_closeController->setCanClosed(true);
        m_closeController->setActive(false);
    }

    qApp->removeEventFilter(this);
}

bool PopupView::eventFilter(QObject* watched, QEvent* event)
{
    if (QEvent::UpdateRequest == event->type()
        || (event->type() == QEvent::Move && watched == m_parentWindow)) {
        repositionWindowIfNeed();
    }

    return WindowView::eventFilter(watched, event);
}

PopupView::ClosePolicies PopupView::closePolicies() const
{
    return m_closePolicies;
}

void PopupView::setClosePolicies(ClosePolicies closePolicies)
{
    if (m_closePolicies == closePolicies) {
        return;
    }

    m_closePolicies = closePolicies;

    if (m_closeController) {
        m_closeController->setIsCloseOnPressOutsideParent(closePolicies & ClosePolicy::CloseOnPressOutsideParent);
    }

    emit closePoliciesChanged(closePolicies);
}

QQuickItem* PopupView::anchorItem() const
{
    return m_anchorItem;
}

void PopupView::setAnchorItem(QQuickItem* anchorItem)
{
    if (m_anchorItem == anchorItem) {
        return;
    }

    m_anchorItem = anchorItem;
    emit anchorItemChanged(m_anchorItem);
}

QRectF PopupView::anchorGeometry() const
{
    QRectF geometry = currentScreenGeometry();
    if (m_anchorItem) {
        QPointF anchorItemTopLeft = m_anchorItem->mapToGlobal(QPoint(0, 0));
        geometry &= QRectF(anchorItemTopLeft, m_anchorItem->size());
    }

    return geometry;
}

PopupView::PlacementPolicies PopupView::placementPolicies() const
{
    return m_placementPolicies;
}

void PopupView::setPlacementPolicies(PlacementPolicies placementPolicies)
{
    if (m_placementPolicies == placementPolicies) {
        return;
    }

    m_placementPolicies = placementPolicies;
    emit placementPoliciesChanged(placementPolicies);
}

PopupPosition::Type PopupView::popupPosition() const
{
    return m_popupPosition;
}

void PopupView::setPopupPosition(PopupPosition::Type position)
{
    if (m_popupPosition == position) {
        return;
    }

    m_popupPosition = position;
    emit popupPositionChanged(m_popupPosition);
}

qreal PopupView::localX() const
{
    return m_localPos.x();
}

qreal PopupView::localY() const
{
    return m_localPos.y();
}

void PopupView::setLocalX(qreal x)
{
    if (qFuzzyCompare(m_localPos.x(), x)) {
        return;
    }

    m_localPos.setX(x);
    emit xChanged(m_localPos.x());

    repositionWindowIfNeed();
}

void PopupView::setLocalY(qreal y)
{
    if (qFuzzyCompare(m_localPos.y(), y)) {
        return;
    }

    m_localPos.setY(y);
    emit yChanged(m_localPos.y());

    repositionWindowIfNeed();
}

bool PopupView::showArrow() const
{
    return m_showArrow;
}

void PopupView::setShowArrow(bool showArrow)
{
    if (m_showArrow == showArrow) {
        return;
    }

    m_showArrow = showArrow;
    emit showArrowChanged(m_showArrow);
}

int PopupView::arrowX() const
{
    return m_arrowX;
}

int PopupView::arrowY() const
{
    return m_arrowY;
}

void PopupView::setArrowX(int arrowX)
{
    if (m_arrowX == arrowX) {
        return;
    }

    m_arrowX = arrowX;
    emit arrowXChanged(m_arrowX);
}

void PopupView::setArrowY(int arrowY)
{
    if (m_arrowY == arrowY) {
        return;
    }

    m_arrowY = arrowY;
    emit arrowYChanged(m_arrowY);
}

int PopupView::padding() const
{
    return m_padding;
}

void PopupView::setPadding(int padding)
{
    if (m_padding == padding) {
        return;
    }

    m_padding = padding;
    emit paddingChanged(m_padding);
}

void PopupView::repositionWindowIfNeed()
{
    if (!isOpened()) {
        return;
    }

    m_globalPos = QPointF();
    updateGeometry();
    m_view->setPosition(m_globalPos.toPoint());
    m_globalPos = QPoint();
}

void PopupView::updateGeometry()
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

    auto movePos = [this, &viewRect](qreal x, qreal y) {
        m_globalPos.setX(x);
        m_globalPos.setY(y);

        viewRect.moveTopLeft(m_globalPos);
    };

    bool ignoreFit = m_placementPolicies.testFlag(PlacementPolicy::IgnoreFit);
    bool canFitAbove = !ignoreFit ? viewRect.height() < parentTopLeft.y() : true;
    bool canFitBelow = !ignoreFit ? viewRect.bottom() < anchorRect.bottom() : true;
    bool canFitLeft = !ignoreFit ? viewRect.width() < parentTopLeft.x() : true;
    bool canFitRight = !ignoreFit ? viewRect.right() < anchorRect.right() : true;

    auto moveBelow = [&]() {
        movePos(m_globalPos.x(), parentTopLeft.y() + parent->height());
        setPopupPosition(PopupPosition::Bottom);
    };

    auto moveAbove = [&]() {
        movePos(m_globalPos.x(), parentTopLeft.y() - viewRect.height());
        setPopupPosition(PopupPosition::Top);
    };

    auto moveLeft = [&]() {
        movePos(parentTopLeft.x() - viewRect.width(), m_globalPos.y());
        setPopupPosition(PopupPosition::Left);
    };

    auto moveRight = [&]() {
        movePos(parentTopLeft.x() + parent->width(), m_globalPos.y());
        setPopupPosition(PopupPosition::Right);
    };

    bool placementDefault = m_placementPolicies.testFlag(PlacementPolicy::Default);
    bool preferBelow = m_placementPolicies.testFlag(PlacementPolicy::PreferBelow);
    bool preferAbove = m_placementPolicies.testFlag(PlacementPolicy::PreferAbove);
    bool preferLeft = m_placementPolicies.testFlag(PlacementPolicy::PreferLeft);
    bool preferRight = m_placementPolicies.testFlag(PlacementPolicy::PreferRight);

    if ((preferBelow || placementDefault) && canFitBelow) {
        moveBelow();
    } else if ((preferAbove || placementDefault) && canFitAbove) {
        moveAbove();
    } else if (preferLeft && canFitLeft) {
        moveLeft();
    } else if (preferRight && canFitRight) {
        moveRight();
    } else if (!canFitBelow && canFitAbove && (preferBelow || placementDefault)) {
        moveAbove();
    } else if (!canFitAbove && canFitBelow && (preferAbove || placementDefault)) {
        moveBelow();
    } else if (!canFitLeft && canFitRight && preferLeft) {
        moveRight();
    } else if (!canFitRight && canFitLeft && preferRight) {
        moveLeft();
    } else {
        // move to the right of the parent and move to top to an area that doesn't fit
        movePos(parentTopLeft.x() + parent->width(), m_globalPos.y() - (viewRect.bottom() - anchorRect.bottom()) + padding());
        setPopupPosition(PopupPosition::Right);
    }

    if (viewRect.left() < anchorRect.left()) {
        // move to the right to an area that doesn't fit
        movePos(m_globalPos.x() + anchorRect.left() - viewRect.left(), m_globalPos.y());
    }

    if (viewRect.right() > anchorRect.right()) {
        // move to the left to an area that doesn't fit
        movePos(m_globalPos.x() - (viewRect.right() - anchorRect.right()), m_globalPos.y());
    }

    if (!showArrow()) {
        if (popupPosition() == PopupPosition::Bottom || popupPosition() == PopupPosition::Top) {
            movePos(m_globalPos.x() - padding(), m_globalPos.y());
        } else if (popupPosition() == PopupPosition::Left || popupPosition() == PopupPosition::Right) {
            movePos(m_globalPos.x(), m_globalPos.y() - padding());
        }
    }

    updateContentPosition();
}

void PopupView::updateContentPosition()
{
    if (!showArrow()) {
        return;
    }

    const QQuickItem* parent = parentItem();
    IF_ASSERT_FAILED(parent) {
        return;
    }

    QPointF parentTopLeft = parent->mapToGlobal(QPoint(0, 0));

    QRect viewGeometry = this->viewGeometry();
    QPointF viewTopLeft = QPointF(viewGeometry.x(), viewGeometry.y());
    QPointF viewTopRight = QPointF(viewGeometry.x() + viewGeometry.width(), viewGeometry.y());

    if (parentTopLeft.x() < viewTopLeft.x() || parentTopLeft.x() > viewTopRight.x()) {
        setArrowX(viewGeometry.width() / 2);
    } else {
        setArrowX(parentTopLeft.x() + (parent->width() / 2) - m_globalPos.x());
    }

    if (parentTopLeft.y() < viewTopLeft.y() || parentTopLeft.y() > viewGeometry.bottom()) {
        setArrowY(viewGeometry.height() / 2);
    } else {
        setArrowY(parentTopLeft.y() + (parent->height() / 2) - m_globalPos.y());
    }
}
