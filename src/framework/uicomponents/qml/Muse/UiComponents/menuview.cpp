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

#include "log.h"
#include "defer.h"

using namespace muse::uicomponents;

static const QString MENU_VIEW_CONTENT_OBJECT_NAME("_MenuViewContent");

// Padding so that our menus don't "collide" with the top/bottom of our screen...
static const int TOP_BOTTOM_EDGE_PADDING = 16;

MenuView::MenuView(QQuickItem* parent)
    : PopupView(parent)
{
    setObjectName("MenuView");

    setShowArrow(false);
    setPadding(8);
}

bool MenuView::isSearchable() const
{
    return m_isSearchable;
}

void MenuView::setIsSearchable(bool isSearchable)
{
    if (m_isSearchable == isSearchable) {
        return;
    }
    m_isSearchable = isSearchable;
    emit isSearchableChanged();
}

bool MenuView::isSearching() const
{
    return m_isSearching;
}

void MenuView::setIsSearching(bool isSearching)
{
    IF_ASSERT_FAILED(m_isSearchable || !isSearching) {
        return;
    }
    if (m_isSearching == isSearching) {
        return;
    }
    m_isSearching = isSearching;
    emit isSearchingChanged();
}

int MenuView::viewMargins() const
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
    setContentHeight(m_desiredHeight);
    setContentWidth(m_desiredWidth);

    const QQuickItem* parent = parentItem();
    IF_ASSERT_FAILED(parent) {
        return;
    }

    setLocalX(0);
    setLocalY(parent->height());

    const QPointF parentTopLeft = parent->mapToGlobal(QPoint(0, 0));

    if (m_globalPos.isNull()) {
        m_globalPos = parentTopLeft;
    }

    const QRectF paddedAnchorRect = anchorGeometry().adjusted(0, TOP_BOTTOM_EDGE_PADDING, 0, -TOP_BOTTOM_EDGE_PADDING);
    QRectF viewRect = viewGeometry();

    //! NOTE: should be after resolving anchor geometry
    //! because we can move out of the screen
    m_globalPos += m_localPos;

    PopupPosition::Type newPopupPos = popupPosition();
    setCascadeAlign(Qt::AlignmentFlag::AlignRight);

    const auto movePos = [this, &viewRect](qreal x, qreal y) {
        m_globalPos.setX(x);
        m_globalPos.setY(y);

        viewRect.moveTopLeft(m_globalPos);
    };

    const QQuickItem* parentMenuContentItem = this->parentMenuContentItem();
    const bool isCascade = parentMenuContentItem != nullptr;

    if (isCascade) {
        movePos(parentTopLeft.x() + parent->width(), m_globalPos.y() - parent->height() - viewMargins());
    }

    if (viewRect.left() < paddedAnchorRect.left()) { // The left of this menu overlaps the left of the anchor (doesn't fit)...
        movePos(m_globalPos.x() + paddedAnchorRect.left() - viewRect.left(), m_globalPos.y()); // Move to the right
    }

    const bool isSearchingAbove = isSearching() && popupPosition() == PopupPosition::Top;
    const bool isSearchingRight = isSearching() && popupPosition() == PopupPosition::Right;
    const bool isSearchingBottom = isSearching() && popupPosition() == PopupPosition::Bottom;

    const auto doRepositionResize = [&]() { // This gets quite complicated - lambda avoids nesting...
        if (isCascade) {
            // If this is a submenu - move it up...
            movePos(m_globalPos.x(), m_globalPos.y() - (viewRect.bottom() - paddedAnchorRect.bottom()));
            return;
        }

        if (isSearchingBottom) {
            const qreal bottomOverlap = viewRect.bottom() - paddedAnchorRect.bottom();
            if (bottomOverlap > 0) {
                // Resize the popup so that it doesn't extend beyond the bottom of the screen...
                setContentHeight(m_desiredHeight - bottomOverlap);
                viewRect.setHeight(viewRect.height() - bottomOverlap);
            }
            return; // We're searching, so don't reposition the popup...
        }

        qreal desiredY = parentTopLeft.y() - viewRect.height();
        const qreal topOverlap = paddedAnchorRect.top() - desiredY;
        if (isSearchingAbove && topOverlap > 0) {
            setContentHeight(m_desiredHeight - topOverlap);
            viewRect.setHeight(viewRect.height() - topOverlap);

            desiredY = parentTopLeft.y() - viewRect.height(); // height changed - recompute...
            // No early return - we still need to actively position above...
        }

        // Searching right is an intermediate state that should never actually happen. When the popup is initially
        // positioned to the right and we start a search, we immediately reposition above...
        if (isSearchingAbove || isSearchingRight || paddedAnchorRect.top() < desiredY) {
            // Place above...
            movePos(m_globalPos.x(), desiredY);
            newPopupPos = PopupPosition::Top;
            return;
        }

        if (!isSearching()) {
            // Place to the right...
            movePos(parentTopLeft.x() + parent->width(), m_globalPos.y() - (viewRect.bottom() - paddedAnchorRect.bottom()));
            newPopupPos = PopupPosition::Right;
        }
    };

    // When searching a menu - we need to actively preserve the position / resize...
    const bool overlapsBottom = viewRect.bottom() > paddedAnchorRect.bottom();
    if (isSearching() || overlapsBottom) {
        doRepositionResize();
    }

    const Qt::AlignmentFlag parentCascadeAlign = this->parentCascadeAlign(parentMenuContentItem);
    if (viewRect.right() > paddedAnchorRect.right() || parentCascadeAlign != Qt::AlignmentFlag::AlignRight) {
        if (isCascade) {
            // move to the right of the parent
            movePos(parentTopLeft.x() - viewRect.width() + padding() * 2, m_globalPos.y());
            setCascadeAlign(Qt::AlignmentFlag::AlignLeft);
            newPopupPos = PopupPosition::Right;
        } else {
            // move to the left to an area that doesn't fit
            movePos(m_globalPos.x() - (viewRect.right() - paddedAnchorRect.right()) + padding() * 2, m_globalPos.y());
            newPopupPos = PopupPosition::Left;
        }
    }

    // remove padding for arrow
    setPopupPosition(newPopupPos);
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

int MenuView::desiredHeight() const
{
    return m_desiredHeight;
}

void MenuView::setDesiredHeight(int desiredHeight)
{
    if (m_desiredHeight == desiredHeight) {
        return;
    }

    m_desiredHeight = desiredHeight;
    emit desiredHeightChanged();

    QMetaObject::invokeMethod(this, [this] {
        updateGeometry();
        repositionWindowIfNeed();
    }, Qt::QueuedConnection);
}

int MenuView::desiredWidth() const
{
    return m_desiredWidth;
}

void MenuView::setDesiredWidth(int desiredWidth)
{
    if (m_desiredWidth == desiredWidth) {
        return;
    }

    m_desiredWidth = desiredWidth;
    emit desiredWidthChanged();

    QMetaObject::invokeMethod(this, [this] {
        updateGeometry();
        repositionWindowIfNeed();
    }, Qt::QueuedConnection);
}
