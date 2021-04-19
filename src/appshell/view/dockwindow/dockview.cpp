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

#include "dockview.h"

#include <QQmlEngine>
#include <QQmlContext>
#include <QWidget>

#include "log.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

using namespace mu::dock;

DockView::DockView(QQuickItem* parent)
    : QQuickItem(parent)
{
    setFlag(QQuickItem::ItemHasContents, true);

    connect(this, &DockView::colorChanged, this, &DockView::updateStyle);
    connect(this, &DockView::borderColorChanged, this, &DockView::updateStyle);
}

void DockView::forceActiveFocus()
{
    view()->setFocus();
}

void DockView::componentComplete()
{
    QQuickItem::componentComplete();

    if (objectName().isEmpty()) {
        LOGE() << "not set objectName for " << this;
        Q_ASSERT(!objectName().isEmpty());
    }

    if (m_content) {
        QQmlEngine* engine = m_content->engine();

        m_view = new QQuickView(engine, nullptr);
        m_view->setResizeMode(QQuickView::SizeRootObjectToView);
        m_widget = QWidget::createWindowContainer(m_view, nullptr, Qt::Widget);
        m_widget->setObjectName("w_" + objectName());

        m_view->resize(width(), height());
        m_widget->resize(m_view->size());

        QQmlContext* ctx = QQmlEngine::contextForObject(this);
        QQuickItem* obj = qobject_cast<QQuickItem*>(m_content->create(ctx));
        m_view->setContent(QUrl(), m_content, obj);
    }

    onComponentCompleted();
}

void DockView::onWidgetEvent(QEvent* event)
{
    if (QEvent::Resize == event->type()) {
        QResizeEvent* re = static_cast<QResizeEvent*>(event);
        setSize(QSizeF(re->size()));
    } else if (QEvent::Move == event->type()) {
        QMoveEvent* me = static_cast<QMoveEvent*>(event);
        setPosition(me->pos());
        emit positionChanged(me->pos());
    }
}

QWidget* DockView::view() const
{
    return m_widget;
}

QQmlComponent* DockView::content() const
{
    return m_content;
}

void DockView::setContent(QQmlComponent* component)
{
    if (m_content == component) {
        return;
    }

    m_content = component;
    emit contentChanged();
}

QColor DockView::color() const
{
    return m_color;
}

void DockView::setColor(QColor color)
{
    if (m_color == color) {
        return;
    }

    m_color = color;
    emit colorChanged(m_color);
}

QColor DockView::borderColor() const
{
    return m_borderColor;
}

bool DockView::visible() const
{
    return view() ? view()->isVisible() : false;
}

void DockView::setBorderColor(QColor color)
{
    if (m_borderColor == color) {
        return;
    }

    m_borderColor = color;
    emit borderColorChanged(color);
}

void DockView::setVisible(bool value)
{
    if (view()) {
        view()->setVisible(value);
    }

    emit visibleEdited(value);
}
