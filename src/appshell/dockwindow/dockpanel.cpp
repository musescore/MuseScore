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

#include <QDockWidget>

using namespace mu::dock;

static const QString PANEL_QSS = QString("QDockWidget { border: 1; color: white; }"
                                         "QDockWidget::title { background: %1; } ");

DockPanel::DockPanel(QQuickItem* parent)
    : DockView(parent)
{
    m_dock.panel = new QDockWidget();
}

DockPanel::~DockPanel()
{
    delete m_dock.panel;
}

void DockPanel::onComponentCompleted()
{
    panel()->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    panel()->setFeatures(QDockWidget::DockWidgetMovable);
    panel()->setObjectName("w_" + objectName());
    panel()->setWidget(view());
    panel()->setWindowTitle(m_title);
    panel()->setStyleSheet(PANEL_QSS.arg(color().name()));

    m_preferedWidth = width();

    if (minimumWidth() == 0) {
        panel()->setMinimumWidth(width());
    }
}

void DockPanel::updateStyle()
{
    panel()->setStyleSheet(PANEL_QSS.arg(color().name()));
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

QDockWidget* DockPanel::panel() const
{
    return m_dock.panel;
}
