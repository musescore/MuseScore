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
#include "dockmenubar.h"

#include <QMenu>

using namespace mu::dock;

DockMenuBar::DockMenuBar(QQuickItem* parent)
    : DockView(parent)
{
}

QVariantList DockMenuBar::items() const
{
    return m_items;
}

void DockMenuBar::setItems(QVariantList items)
{
    if (m_items == items) {
        return;
    }

    m_items = items;

    updateMenus();

    emit itemsChanged(m_items);
}

void DockMenuBar::onActionTriggered(QAction* action)
{
    emit actionTringgered(action->data().toString());
}

void DockMenuBar::updateMenus()
{
    QList<QMenu*> menus;
    for (const QVariant& item: m_items) {
        menus << makeMenu(item.toMap());
    }

    emit changed(menus);
}

QMenu* DockMenuBar::makeMenu(const QVariantMap& menuItem) const
{
    QMenu* menu = new QMenu();
    menu->setTitle(menuItem.value("title").toString());
    menu->setEnabled(menuItem.value("enabled").toBool());

    for (const QVariant& menuObj: menuItem.value("subitems").toList()) {
        QVariantMap menuMap = menuObj.toMap();
        if (menuMap.value("title").toString().isEmpty()) {
            menu->addSeparator();
        } else if (!menuMap.value("subitems").toList().empty()) {
            menu->addMenu(makeMenu(menuMap));
        } else {
            menu->addAction(makeAction(menuMap));
        }
    }

    return menu;
}

QAction* DockMenuBar::makeAction(const QVariantMap& menuItem) const
{
    QAction* action = new QAction();
    action->setData(menuItem.value("code").toString());
    action->setText(menuItem.value("title").toString());

    action->setShortcut(menuItem.value("shortcut").toString());
    // NOTE: Disable activate by hotkey
    action->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    action->setEnabled(menuItem.value("enabled").toBool());
    action->setChecked(menuItem.value("checked").toBool());
    return action;
}
