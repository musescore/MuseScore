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

#include "dropdownview.h"

#include "log.h"

using namespace muse::uicomponents;

DropdownView::DropdownView(QQuickItem* parent)
    : PopupView(parent)
{
}

bool DropdownView::eventFilter(QObject* watched, QEvent* event)
{
    static QSet<Qt::Key> NAVIGATION_KEYS = {
        Qt::Key_Enter,
        Qt::Key_Return,
        Qt::Key_Space,
        Qt::Key_Up,
        Qt::Key_Down,
        Qt::Key_Left,
        Qt::Key_Right,
        Qt::Key_Tab,
        Qt::Key_PageUp,
        Qt::Key_PageDown,
    };

    if (event && event->type() == QEvent::ShortcutOverride) {
        const QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        const Qt::Key key = (Qt::Key)keyEvent->key();
        if (!NAVIGATION_KEYS.contains(key)) {
            event->accept();
            return true;
        }
    }

    return PopupView::eventFilter(watched, event);
}

void DropdownView::updateGeometry()
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
    QRectF popupRect(m_globalPos, contentItem()->size());

    auto movePos = [this, &popupRect](qreal x, qreal y) {
        m_globalPos.setX(x);
        m_globalPos.setY(y);

        popupRect.moveTopLeft(m_globalPos);
    };

    // move to focus item
    movePos(m_globalPos.x(), m_globalPos.y() - focusItemY());

    if (popupRect.bottom() > anchorRect.bottom()) {
        // move to the right of the parent and move to top to an area that doesn't fit
        movePos(m_globalPos.x(), m_globalPos.y() - (popupRect.bottom() - anchorRect.bottom()) + padding());
    }

    if (popupRect.left() < anchorRect.left()) {
        // move to the right to an area that doesn't fit
        movePos(m_globalPos.x() + anchorRect.left() - popupRect.left(), m_globalPos.y());
    }

    if (popupRect.right() > anchorRect.right()) {
        // move to the left to an area that doesn't fit
        movePos(m_globalPos.x() - (popupRect.right() - anchorRect.right()), m_globalPos.y());
    }

    if (popupRect.top() < anchorRect.top()) {
        // move to the bottom to an area that doesn't fit
        movePos(m_globalPos.x(), anchorRect.top() + padding());
    }

    // remove padding for arrow
    movePos(m_globalPos.x() - padding(), m_globalPos.y());

    updateContentPosition();
}

int DropdownView::focusItemY() const
{
    return m_focusItemY;
}

void DropdownView::setFocusItemY(int newFocusItemY)
{
    if (m_focusItemY == newFocusItemY) {
        return;
    }

    m_focusItemY = newFocusItemY;
    emit focusItemYChanged();
}
