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

#include "dockbase.h"

#include <QRect>

#include "log.h"

#include "thirdparty/KDDockWidgets/src/DockWidgetQuick.h"
#include "thirdparty/KDDockWidgets/src/private/Frame_p.h"
#include "thirdparty/KDDockWidgets/src/private/FloatingWindow_p.h"

using namespace mu::dock;

DockBase::DockBase(QQuickItem* parent)
    : QQuickItem(parent)
{
    connect(this, &DockBase::minimumSizeChanged, this, &DockBase::resize);
    connect(this, &DockBase::maximumSizeChanged, this, &DockBase::resize);
}

QString DockBase::title() const
{
    return m_title;
}

int DockBase::minimumWidth() const
{
    return m_minimumWidth;
}

int DockBase::minimumHeight() const
{
    return m_minimumHeight;
}

int DockBase::maximumWidth() const
{
    return m_maximumWidth;
}

int DockBase::maximumHeight() const
{
    return m_maximumHeight;
}

QSize DockBase::preferredSize() const
{
    return QSize(width(), height());
}

Qt::DockWidgetAreas DockBase::allowedAreas() const
{
    return m_allowedAreas;
}

bool DockBase::floating() const
{
    return m_floating;
}

KDDockWidgets::DockWidgetQuick* DockBase::dockWidget() const
{
    return m_dockWidget;
}

void DockBase::setSizeConstraints(const QSize& minimumSize, const QSize& maximumSize)
{
    m_dockWidget->setMinimumSize(minimumSize);
    m_dockWidget->setMaximumSize(maximumSize);

    if (auto frame = m_dockWidget->frame()) {
        frame->setMinimumSize(minimumSize);
        frame->setMaximumSize(maximumSize);
    }

    if (auto floatingWindow = m_dockWidget->floatingWindow()) {
        QRect rect(floatingWindow->dragRect().topLeft(), minimumSize);
        floatingWindow->setGeometry(rect);
    }
}

void DockBase::setTitle(const QString& title)
{
    if (title == m_title) {
        return;
    }

    m_title = title;
    emit titleChanged();
}

void DockBase::setMinimumWidth(int width)
{
    if (width == minimumWidth()) {
        return;
    }

    m_minimumWidth = width;
    emit minimumSizeChanged();
}

void DockBase::setMinimumHeight(int height)
{
    if (height == minimumHeight()) {
        return;
    }

    m_minimumHeight = height;
    emit minimumSizeChanged();
}

void DockBase::setMaximumWidth(int width)
{
    if (width == maximumWidth()) {
        return;
    }

    m_maximumWidth = width;
    emit maximumSizeChanged();
}

void DockBase::setMaximumHeight(int height)
{
    if (height == maximumHeight()) {
        return;
    }

    m_maximumHeight = height;
    emit maximumSizeChanged();
}

void DockBase::setAllowedAreas(Qt::DockWidgetAreas areas)
{
    if (areas == allowedAreas()) {
        return;
    }

    m_allowedAreas = areas;
    emit allowedAreasChanged();
}

void DockBase::setFloating(bool floating)
{
    if (m_floating == floating) {
        return;
    }

    m_floating = floating;
    emit floatingChanged();
}

void DockBase::setLocation(DockLocation location)
{
    if (m_location == location) {
        return;
    }

    m_location = location;
    emit locationChanged(m_location);
}

DockType DockBase::type() const
{
    return DockType::Undefined;
}

void DockBase::resize()
{
    applySizeConstraints();
}

void DockBase::init()
{
    applySizeConstraints();
}

void DockBase::show()
{
    IF_ASSERT_FAILED(m_dockWidget) {
        return;
    }

    m_dockWidget->show();
}

void DockBase::hide()
{
    IF_ASSERT_FAILED(m_dockWidget) {
        return;
    }

    m_dockWidget->forceClose();
}

DockBase::DockLocation DockBase::location() const
{
    return m_location;
}

void DockBase::componentComplete()
{
    QQuickItem::componentComplete();

    auto children = childItems();
    IF_ASSERT_FAILED(!children.isEmpty()) {
        return;
    }

    m_dockWidget = new KDDockWidgets::DockWidgetQuick(objectName());
    m_dockWidget->setWidget(children.constFirst());
    m_dockWidget->setTitle(m_title);

    DockProperties properties;
    properties.type = type();
    properties.allowedAreas = allowedAreas();

    writePropertiesToObject(properties, *m_dockWidget);

    listenFloatingChanges();
}

void DockBase::applySizeConstraints()
{
    if (!m_dockWidget) {
        return;
    }

    int minWidth = m_minimumWidth > 0 ? m_minimumWidth : m_dockWidget->minimumWidth();
    int minHeight = m_minimumHeight > 0 ? m_minimumHeight : m_dockWidget->minimumHeight();
    int maxWidth = m_maximumWidth > 0 ? m_maximumWidth : m_dockWidget->maximumWidth();
    int maxHeight = m_maximumHeight > 0 ? m_maximumHeight : m_dockWidget->maximumHeight();

    QSize minimumSize(minWidth, minHeight);
    QSize maximumSize(maxWidth, maxHeight);

    setSizeConstraints(minimumSize, maximumSize);
}

void DockBase::listenFloatingChanges()
{
    connect(m_dockWidget, &KDDockWidgets::DockWidgetQuick::parentChanged, [this]() {
        if (!m_dockWidget || !m_dockWidget->parentItem()) {
            return;
        }

        KDDockWidgets::Frame* frame = m_dockWidget->frame();
        if (!frame) {
            return;
        }

        connect(frame, &KDDockWidgets::Frame::isInMainWindowChanged, this, [=]() {
            setFloating(!frame->isInMainWindow());
        }, Qt::UniqueConnection);
    });
}
