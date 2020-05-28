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

static const QString qss = QString("QDockWidget { border: 1; color: white; }"
                                   "QDockWidget::title { background: %1; } ");

DockPanel::DockPanel(QQuickItem *parent)
    : DockView(parent)
{
}

DockPanel::~DockPanel()
{
    delete _dock.panel;
}

void DockPanel::onComponentCompleted()
{
    _dock.panel = new QDockWidget();
    _dock.panel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    _dock.panel->setFeatures(QDockWidget::DockWidgetMovable);
    _dock.panel->setObjectName("w_"+objectName());
    _dock.panel->setWidget(view());
    _dock.panel->setMinimumWidth(width());
    _dock.panel->setWindowTitle(_title);
    //_dock.panel->setStyleSheet(qss.arg(color().name()));
}

DockPanel::Widget DockPanel::widget() const
{
    return _dock;
}

QString DockPanel::title() const
{
    return _title;
}

void DockPanel::setTitle(QString title)
{
    if (_title == title)
        return;

    _title = title;
    emit titleChanged(_title);
}

Qt::DockWidgetArea DockPanel::area() const
{
    return _dock.area;
}

void DockPanel::setArea(Qt::DockWidgetArea area)
{
    if (_dock.area == area)
        return;

    _dock.area = area;
    emit areaChanged(_dock.area);
}

QString DockPanel::tabifyObjectName() const
{
    return _dock.tabifyObjectName;
}

void DockPanel::setTabifyObjectName(QString tabify)
{
    if (_dock.tabifyObjectName == tabify)
        return;

    _dock.tabifyObjectName = tabify;
    emit tabifyObjectNameChanged(_dock.tabifyObjectName);
}
