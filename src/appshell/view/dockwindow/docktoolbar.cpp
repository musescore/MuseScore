//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

#include "docktoolbar.h"

#include <QToolBar>

#include "eventswatcher.h"

using namespace mu::dock;
using namespace mu::ui;

static const qreal TOOLBAR_GRIP_WIDTH(32);
static const qreal TOOLBAR_GRIP_HEIGHT(36);

static const QString TOOLBAR_QSS = QString("QToolBar { background: %4; border: 0; padding: 0; } "
                                           "QToolBar::handle::horizontal { image: url(\":/view/dockwindow/resources/toolbar_grip_%3_horizontal.svg\"); width: %1px; height: %2px; } "
                                           "QToolBar::handle::vertical { image: url(\":/view/dockwindow/resources/toolbar_grip_%3_vertical.svg\"); width: %2px; height: %1px; } "
                                           ).arg(TOOLBAR_GRIP_WIDTH).arg(TOOLBAR_GRIP_HEIGHT);

DockToolBar::DockToolBar(QQuickItem* parent)
    : DockView(parent)
{
    m_tool.bar = new QToolBar();
    m_tool.bar->setAllowedAreas(Qt::AllToolBarAreas);
    m_tool.bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(this, &DockToolBar::visibleEdited, this, [this](bool visible) {
        if (m_tool.bar->isVisible() != visible) {
            m_tool.bar->setVisible(visible);
        }
    });

    connect(m_tool.bar, &QToolBar::orientationChanged, [this](int orientation) {
        emit orientationChanged(orientation);
    });

    connect(m_tool.bar, &QToolBar::topLevelChanged, [this](bool floating) {
        setFloating(floating);
    });

    connect(m_tool.bar, &QToolBar::windowTitleChanged, this, &DockToolBar::titleChanged);

    m_eventsWatcher = new EventsWatcher(this);
    m_tool.bar->installEventFilter(m_eventsWatcher);
    connect(m_eventsWatcher, &EventsWatcher::eventReceived, this, &DockToolBar::onWidgetEvent);
}

DockToolBar::~DockToolBar()
{
    delete m_tool.bar;
}

QToolBar* DockToolBar::toolBar() const
{
    return m_tool.bar;
}

void DockToolBar::onComponentCompleted()
{
    toolBar()->setObjectName("w_" + objectName());
    updateStyle();

    QWidget* widget = view();
    widget->setMinimumWidth(minimumWidth());
    widget->setMinimumHeight(minimumHeight());
    toolBar()->addWidget(widget);
}

void DockToolBar::updateStyle()
{
    toolBar()->setStyleSheet(TOOLBAR_QSS.arg(QString::fromStdString(uiConfiguration()->currentTheme().codeKey), color().name()));
}

void DockToolBar::onWidgetEvent(QEvent* event)
{
    if (QEvent::Resize == event->type()) {
        QResizeEvent* resizeEvent = static_cast<QResizeEvent*>(event);
        resize(resizeEvent->size());
    } else if (QEvent::ShowToParent == event->type()) {
        resize(toolBar()->size());
    } else {
        DockView::onWidgetEvent(event);
    }
}

void DockToolBar::resize(const QSize& size)
{
    QSize newSize = size;
    if (toolBar()->orientation() == Qt::Horizontal) {
        newSize.setWidth(newSize.width() - TOOLBAR_GRIP_WIDTH);
    } else {
        newSize.setHeight(newSize.height() - TOOLBAR_GRIP_WIDTH);
    }
    if (view()) {
        view()->resize(newSize);
    }
}

void DockToolBar::setFloating(bool floating)
{
    if (m_floating == floating) {
        return;
    }

    m_floating = floating;
    emit floatingChanged(floating);
}

DockToolBar::Widget DockToolBar::widget() const
{
    return m_tool;
}

int DockToolBar::orientation() const
{
    return toolBar()->orientation();
}

int DockToolBar::minimumHeight() const
{
    return m_minimumHeight;
}

int DockToolBar::minimumWidth() const
{
    return m_minimumWidth;
}

Qt::ToolBarAreas DockToolBar::allowedAreas() const
{
    return toolBar()->allowedAreas();
}

bool DockToolBar::floating() const
{
    return m_floating;
}

bool DockToolBar::floatable() const
{
    return toolBar()->isFloatable();
}

bool DockToolBar::movable() const
{
    return toolBar()->isMovable();
}

bool DockToolBar::visible() const
{
    return toolBar()->isVisible();
}

QString DockToolBar::title() const
{
    return toolBar()->windowTitle();
}

void DockToolBar::setMinimumHeight(int minimumHeight)
{
    if (m_minimumHeight == minimumHeight) {
        return;
    }

    m_minimumHeight = minimumHeight;
    if (view()) {
        view()->setMinimumHeight(minimumHeight);
    }

    emit minimumHeightChanged(m_minimumHeight);
}

void DockToolBar::setMinimumWidth(int minimumWidth)
{
    if (m_minimumWidth == minimumWidth) {
        return;
    }

    m_minimumWidth = minimumWidth;
    if (view()) {
        view()->setMinimumWidth(minimumWidth);
    }
    emit minimumWidthChanged(m_minimumWidth);
}

void DockToolBar::setAllowedAreas(Qt::ToolBarAreas allowedAreas)
{
    if (allowedAreas == this->allowedAreas()) {
        return;
    }

    toolBar()->setAllowedAreas(allowedAreas);
    emit allowedAreasChanged(allowedAreas);
}

void DockToolBar::setFloatable(bool floatable)
{
    if (floatable == this->floatable()) {
        return;
    }

    toolBar()->setFloatable(floatable);
    emit floatableChanged(floatable);
}

void DockToolBar::setMovable(bool movable)
{
    if (movable == this->movable()) {
        return;
    }

    toolBar()->setMovable(movable);
    emit movableChanged(movable);
}

void DockToolBar::setVisible(bool visible)
{
    if (m_visible == visible) {
        return;
    }

    m_visible = visible;
    emit visibleEdited(m_visible);
}

void DockToolBar::setTitle(const QString& title)
{
    if (title == this->title()) {
        return;
    }

    toolBar()->setWindowTitle(title);
}
