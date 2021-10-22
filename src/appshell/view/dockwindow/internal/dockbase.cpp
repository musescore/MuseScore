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

namespace mu::dock {
class DockWidgetImpl : public KDDockWidgets::DockWidgetQuick
{
public:
    DockWidgetImpl(const QString& uniqueName)
        : KDDockWidgets::DockWidgetQuick(uniqueName)
    {
        setObjectName(uniqueName);
    }

    QSize minimumSize() const override
    {
        return DockWidgetBase::minimumSize();
    }

    QSize maximumSize() const override
    {
        return DockWidgetBase::maximumSize();
    }
};
}

using namespace mu::dock;

DockBase::DockBase(QQuickItem* parent)
    : QQuickItem(parent), m_resizable(true)
{
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

bool DockBase::resizable() const
{
    return m_resizable;
}

KDDockWidgets::DockWidgetQuick* DockBase::dockWidget() const
{
    return m_dockWidget;
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
    if (floating == m_floating) {
        return;
    }

    m_dockWidget->setFloating(floating);
    doSetFloating(floating);
}

void DockBase::setLocation(DockLocation location)
{
    if (location == m_location) {
        return;
    }

    m_location = location;
    emit locationChanged(m_location);
}

void DockBase::setResizable(bool resizable)
{
    if (resizable == m_resizable) {
        return;
    }

    m_resizable = resizable;
    emit resizableChanged();
}

DockType DockBase::type() const
{
    return DockType::Undefined;
}

void DockBase::init()
{
    applySizeConstraints();
}

bool DockBase::isOpen() const
{
    IF_ASSERT_FAILED(m_dockWidget) {
        return false;
    }

    return m_dockWidget->isOpen();
}

void DockBase::open()
{
    IF_ASSERT_FAILED(m_dockWidget) {
        return;
    }

    m_dockWidget->show();
    setVisible(true);
}

void DockBase::close()
{
    IF_ASSERT_FAILED(m_dockWidget) {
        return;
    }

    m_dockWidget->forceClose();
    setVisible(false);
}

DockBase::DockLocation DockBase::location() const
{
    return m_location;
}

void DockBase::componentComplete()
{
    QQuickItem::componentComplete();

    auto children = childItems();
    IF_ASSERT_FAILED_X(children.size() == 1, "Dock must have only one child as its content!") {
        return;
    }

    QQuickItem* content = childItems().first();
    IF_ASSERT_FAILED(content) {
        return;
    }

    if (content->objectName().isEmpty()) {
        content->setObjectName(objectName() + "_content");
    }

    m_dockWidget = new DockWidgetImpl(objectName());
    m_dockWidget->setWidget(content);
    m_dockWidget->setTitle(m_title);

    DockProperties properties;
    properties.type = type();
    properties.allowedAreas = allowedAreas();

    writePropertiesToObject(properties, *m_dockWidget);

    listenFloatingChanges();

    connect(m_dockWidget, &KDDockWidgets::DockWidgetQuick::widthChanged, this, [this]() {
        if (m_dockWidget) {
            setWidth(m_dockWidget->width());
        }
    });

    connect(m_dockWidget, &KDDockWidgets::DockWidgetQuick::heightChanged, this, [this]() {
        if (m_dockWidget) {
            setHeight(m_dockWidget->height());
        }
    });

    connect(this, &DockBase::minimumSizeChanged, this, &DockBase::applySizeConstraints);
    connect(this, &DockBase::maximumSizeChanged, this, &DockBase::applySizeConstraints);
}

void DockBase::applySizeConstraints()
{
    if (!m_dockWidget) {
        return;
    }

    TRACEFUNC;

    int minWidth = m_minimumWidth > 0 ? m_minimumWidth : m_dockWidget->minimumWidth();
    int minHeight = m_minimumHeight > 0 ? m_minimumHeight : m_dockWidget->minimumHeight();
    int maxWidth = m_maximumWidth > 0 ? m_maximumWidth : m_dockWidget->maximumWidth();
    int maxHeight = m_maximumHeight > 0 ? m_maximumHeight : m_dockWidget->maximumHeight();

    QSize minimumSize(minWidth, minHeight);
    QSize maximumSize(maxWidth, maxHeight);

    if (!m_resizable) {
        maximumSize = minimumSize;
    }

    if (auto frame = m_dockWidget->frame()) {
        frame->setMinimumSize(minimumSize);
        frame->setMaximumSize(maximumSize);
    }

    m_dockWidget->setMinimumSize(minimumSize);
    m_dockWidget->setMaximumSize(maximumSize);

    if (auto floatingWindow = m_dockWidget->floatingWindow()) {
        QRect rect(floatingWindow->dragRect().topLeft(), minimumSize);

        floatingWindow->setGeometry(rect);
        floatingWindow->setMinimumSize(minimumSize);
        floatingWindow->setMaximumSize(maximumSize);
    }
}

void DockBase::listenFloatingChanges()
{
    IF_ASSERT_FAILED(m_dockWidget) {
        return;
    }

    connect(m_dockWidget, &KDDockWidgets::DockWidgetQuick::parentChanged, this, [this]() {
        if (!m_dockWidget || !m_dockWidget->parentItem()) {
            return;
        }

        const KDDockWidgets::Frame* frame = m_dockWidget->frame();
        if (!frame) {
            return;
        }

        connect(frame, &KDDockWidgets::Frame::isInMainWindowChanged, this, [=]() {
            doSetFloating(!frame->isInMainWindow());
            applySizeConstraints();
        }, Qt::UniqueConnection);
    });
}

void DockBase::doSetFloating(bool floating)
{
    m_floating = floating;
    emit floatingChanged();
}
