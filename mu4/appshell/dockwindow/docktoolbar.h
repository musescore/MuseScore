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

#ifndef MU_DOCK_DOCKTOOLBAR_H
#define MU_DOCK_DOCKTOOLBAR_H

#include "dockview.h"

class QToolBar;

namespace mu {
namespace dock {
class DockToolBar : public DockView
{
    Q_OBJECT

public:
    explicit DockToolBar(QQuickItem* parent = nullptr);
    ~DockToolBar();

    struct Widget {
        QToolBar* bar = nullptr;
        Qt::ToolBarArea breakArea{ Qt::TopToolBarArea };
    };

    Widget widget() const;

protected:
    void onComponentCompleted() override;
    void updateStyle() override;

private:

    Widget m_tool;
};
}
}

#endif // MU_DOCK_DOCKTOOLBAR_H
