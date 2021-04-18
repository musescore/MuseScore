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

#include "dockpanel.h"

#include "log.h"
#include "eventswatcher.h"

using namespace mu::dock;

static const QString PANEL_QSS = QString("QDockWidget { border: 1 solid %2; color: transparent; }"
                                         "QDockWidget::title { background: %1; } ");

DockPanel::DockPanel(QQuickItem* parent)
    : DockView(parent)
{
    m_dock.panel = new QDockWidget();
    m_dock.panel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_dock.panel->setFeatures(QDockWidget::DockWidgetMovable);

    connect(this, &DockPanel::visibleEdited, this, [this](bool visible) {
        if (m_dock.panel->isVisible() != visible) {
            m_dock.panel->setVisible(visible);
            if (!visible) {
                m_isShown = false;
            }
        }
    });

    connect(m_dock.panel, &QDockWidget::visibilityChanged, this, [this](bool vsbl) {
        m_isShown = vsbl;
        emit isShownChanged(vsbl);
    });

    m_eventsWatcher = new EventsWatcher(this);
    m_dock.panel->installEventFilter(m_eventsWatcher);
    connect(m_eventsWatcher, &EventsWatcher::eventReceived, this, &DockPanel::onWidgetEvent);
}

DockPanel::~DockPanel()
{
    delete m_dock.panel;
}

void DockPanel::onComponentCompleted()
{
    panel()->setObjectName("w_" + objectName());
    panel()->setWidget(view());
    panel()->setWindowTitle(m_title);
    updateStyle();

    m_preferedWidth = width();
    m_isShown = panel()->isVisible();

    if (minimumWidth() == 0) {
        panel()->setMinimumWidth(width());
    }
}

void DockPanel::updateStyle()
{
    panel()->setStyleSheet(PANEL_QSS.arg(color().name(), borderColor().name()));
}

DockPanel::Widget DockPanel::widget() const
{
    return m_dock;
}

QString DockPanel::title() const
{
    return m_title;
}

void DockPanel::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    panel()->setWindowTitle(m_title);
    emit titleChanged(m_title);
}

Qt::DockWidgetArea DockPanel::area() const
{
    return m_dock.area;
}

void DockPanel::setArea(Qt::DockWidgetArea area)
{
    if (m_dock.area == area) {
        return;
    }

    m_dock.area = area;
    emit areaChanged(m_dock.area);
}

QString DockPanel::tabifyObjectName() const
{
    return m_dock.tabifyObjectName;
}

void DockPanel::setTabifyObjectName(QString tabify)
{
    if (m_dock.tabifyObjectName == tabify) {
        return;
    }

    m_dock.tabifyObjectName = tabify;
    emit tabifyObjectNameChanged(m_dock.tabifyObjectName);
}

int DockPanel::minimumWidth() const
{
    return panel()->minimumWidth();
}

int DockPanel::preferedWidth() const
{
    return m_preferedWidth;
}

void DockPanel::setMinimumWidth(int width)
{
    if (panel()->minimumWidth() == width) {
        return;
    }

    panel()->setMinimumWidth(width);
    emit minimumWidthChanged(width);
}

bool DockPanel::floatable() const
{
    return featureEnabled(QDockWidget::DockWidgetFloatable);
}

void DockPanel::setFloatable(bool floatable)
{
    if (floatable == this->floatable()) {
        return;
    }

    setFeature(QDockWidget::DockWidgetFloatable, floatable);

    emit floatableChanged(floatable);
}

bool DockPanel::closable() const
{
    return featureEnabled(QDockWidget::DockWidgetClosable);
}

bool DockPanel::isShown() const
{
    return m_isShown;
}

bool DockPanel::visible() const
{
    return m_dock.panel ? m_dock.panel->isVisible() : false;
}

void DockPanel::setClosable(bool closable)
{
    if (closable == this->closable()) {
        return;
    }

    setFeature(QDockWidget::DockWidgetClosable, closable);

    emit closableChanged(closable);
}

void DockPanel::onWidgetEvent(QEvent* event)
{
    if (QEvent::Close == event->type()) {
        emit closed();
    } else {
        DockView::onWidgetEvent(event);
    }
}

void DockPanel::setFeature(QDockWidget::DockWidgetFeature feature, bool value)
{
    QDockWidget::DockWidgetFeatures features = panel()->features();
    features.setFlag(feature, value);

    panel()->setFeatures(features);
}

bool DockPanel::featureEnabled(QDockWidget::DockWidgetFeature feature) const
{
    return panel()->features().testFlag(feature);
}

QDockWidget* DockPanel::panel() const
{
    return m_dock.panel;
}
