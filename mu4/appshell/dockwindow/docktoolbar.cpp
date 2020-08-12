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

using namespace mu::dock;

static const QString qss = QString("QToolBar { background: %1; border: 0; padding: 0; }");

DockToolBar::DockToolBar(QQuickItem* parent)
    : DockView(parent)
{
    m_tool.bar = new QToolBar();
    m_tool.bar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
}

DockToolBar::~DockToolBar()
{
    delete m_tool.bar;
}

void DockToolBar::onComponentCompleted()
{
    m_tool.bar->setObjectName("w_" + objectName());
    m_tool.bar->setStyleSheet(qss.arg(color().name()));

    QWidget* w = view();
    w->setFixedHeight(height());
    w->setFixedWidth(width());
    m_tool.bar->addWidget(w);
}

void DockToolBar::updateStyle()
{
    m_tool.bar->setStyleSheet(qss.arg(color().name()));
}

DockToolBar::Widget DockToolBar::widget() const
{
    return m_tool;
}
