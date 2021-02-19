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
#include "dockstatusbar.h"

#include <QWidget>

using namespace mu::dock;

DockStatusBar::DockStatusBar(QQuickItem* parent)
    : DockView(parent)
{
    setHeight(40);

    connect(this, &DockStatusBar::visibleEdited, this, [this](bool visible) {
        if (view() && view()->isVisible() != visible) {
            view()->setVisible(visible);
        }
    });
}

DockStatusBar::~DockStatusBar()
{
}

DockStatusBar::Widget DockStatusBar::widget() const
{
    return m_widget;
}

bool DockStatusBar::visible() const
{
    return m_visible;
}

void DockStatusBar::setVisible(bool visible)
{
    if (m_visible == visible) {
        return;
    }

    m_visible = visible;
    emit visibleEdited(m_visible);
}

void DockStatusBar::onComponentCompleted()
{
    m_widget.widget = view();
    m_widget.widget->setFixedHeight(height());
}
