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

#include "widgetview.h"

using namespace mu::ui;

WidgetView::WidgetView(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setFlag(QQuickItem::ItemAcceptsDrops, true);
    setFlag(QQuickItem::ItemHasContents, true);

    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);
}

WidgetView::~WidgetView()
{
    delete m_widget;
}

void WidgetView::paint(QPainter* painter)
{
    if (!m_widget) {
        return;
    }

    m_widget->render(painter, QPoint(), QRegion(),
                       QWidget::DrawWindowBackground | QWidget::DrawChildren);
}

bool WidgetView::event(QEvent* event)
{
    if (!m_widget) {
        return QQuickItem::event(event);
    }

    static const QSet<QEvent::Type> clickEvents {
        QEvent::MouseButtonPress,
        QEvent::MouseButtonDblClick
    };

    if (clickEvents.contains(event->type())) {
        setFocus(true);
    }

    if (qApp->sendEvent(m_widget, event)) {
        update();
        return true;
    }

    return QQuickItem::event(event);
}

void WidgetView::setWidget(QWidget* widget)
{
    m_widget = widget;
}
