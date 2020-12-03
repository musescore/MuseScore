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

    Q_PROPERTY(int orientation READ orientation NOTIFY orientationChanged)
    Q_PROPERTY(int minimumHeight READ minimumHeight WRITE setMinimumHeight NOTIFY minimumHeightChanged)
    Q_PROPERTY(int minimumWidth READ minimumWidth WRITE setMinimumWidth NOTIFY minimumWidthChanged)
    Q_PROPERTY(Qt::ToolBarAreas allowedAreas READ allowedAreas WRITE setAllowedAreas NOTIFY allowedAreasChanged)

public:
    explicit DockToolBar(QQuickItem* parent = nullptr);
    ~DockToolBar();

    struct Widget {
        QToolBar* bar = nullptr;
        Qt::ToolBarArea breakArea { Qt::TopToolBarArea };
    };

    Widget widget() const;

    int orientation() const;
    int minimumHeight() const;
    int minimumWidth() const;

    Qt::ToolBarAreas allowedAreas() const;

public slots:
    void setMinimumHeight(int minimumHeight);
    void setMinimumWidth(int minimumWidth);
    void setAllowedAreas(Qt::ToolBarAreas allowedAreas);

signals:
    void orientationChanged(int orientation);
    void minimumHeightChanged(int minimumHeight);
    void minimumWidthChanged(int minimumWidth);
    void allowedAreasChanged(Qt::ToolBarAreas allowedAreas);

protected:
    void onComponentCompleted() override;
    void updateStyle() override;

private slots:
    void onToolbarEvent(QEvent* e);

private:
    void resize(const QSize& size);

    Widget m_tool;
    EventsWatcher* m_eventsWatcher = nullptr;
    int m_minimumHeight = 0;
    int m_minimumWidth = 0;
};
}
}

#endif // MU_DOCK_DOCKTOOLBAR_H
