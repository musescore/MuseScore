//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_DOCK_DOCKMENUBAR_H
#define MU_DOCK_DOCKMENUBAR_H

#include "dockview.h"

class QMenu;
class QActionGroup;
namespace mu::dock {
class DockMenuBar : public DockView
{
    Q_OBJECT

    Q_PROPERTY(QVariantList items READ items WRITE setItems NOTIFY itemsChanged)

public:
    explicit DockMenuBar(QQuickItem* parent = nullptr);

    QVariantList items() const;

public slots:
    void setItems(QVariantList items);
    void onActionTriggered(QAction* action);

signals:
    void itemsChanged(QVariantList items);
    void changed(const QList<QMenu*>& menus);
    void actionTringgered(const QString& actionCode, int actionIndex);

private:
    void updateMenus();

    QMenu* makeMenu(const QVariantMap& menuItem) const;
    QAction* makeAction(const QVariantMap& menuItem, QActionGroup* group) const;

    QVariantList m_items;
};
}

#endif // MU_DOCK_DOCKMENUBAR_H
