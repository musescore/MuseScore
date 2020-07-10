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

#ifndef MU_DOCK_DOCKPANEL_H
#define MU_DOCK_DOCKPANEL_H

#include "dockview.h"

class QDockWidget;

namespace mu {
namespace dock {
class DockPanel : public DockView
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(Qt::DockWidgetArea area READ area WRITE setArea NOTIFY areaChanged)
    Q_PROPERTY(QString tabifyObjectName READ tabifyObjectName WRITE setTabifyObjectName NOTIFY tabifyObjectNameChanged)

public:
    explicit DockPanel(QQuickItem* parent = nullptr);
    ~DockPanel();

    QString title() const;
    Qt::DockWidgetArea area() const;
    QString tabifyObjectName() const;

    struct Widget {
        QDockWidget* panel = nullptr;
        Qt::DockWidgetArea area{ Qt::LeftDockWidgetArea };
        QString tabifyObjectName;
    };

    Widget widget() const;

public slots:
    void setTitle(QString title);
    void setArea(Qt::DockWidgetArea area);
    void setTabifyObjectName(QString tabifyObjectName);

signals:
    void titleChanged(QString title);
    void areaChanged(Qt::DockWidgetArea area);
    void tabifyObjectNameChanged(QString tabifyObjectName);

protected:
    void onComponentCompleted() override;
    void updateStyle() override;

private:

    Widget m_dock;
    QString m_title;
};
}
}

#endif // MU_DOCK_DOCKPANEL_H
