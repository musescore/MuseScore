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
#include <QActionGroup>

using namespace mu::dock;

static int actionIndexInGroup(const QAction* action)
{
    QActionGroup* actionGroup = action->actionGroup();
    if (!actionGroup) {
        return -1;
    }

    QList<QAction*> actions = actionGroup->actions();
    for (int i = 0; i < actions.size(); ++i) {
        if (actions[i] == action) {
            return i;
        }
    }

    return -1;
}

static void addMenu(QMenu* child, QMenu* parent)
{
#ifdef Q_OS_MAC
    child->setParent(parent);
#endif
    parent->addMenu(child);
}

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
    QVariantMap data = action->data().toMap();
    int actionIndex = actionIndexInGroup(action);
    emit actionTriggered(data.value("code").toString(), actionIndex);
}

void DockMenuBar::updateMenus()
{
    QList<QMenu*> menus;
    for (const QVariant& item: m_items) {
        QMenu* menu = makeMenu(item.toMap());
        menus << menu;
    }

    emit changed(menus);
}

QMenu* DockMenuBar::makeMenu(const QVariantMap& menuItem) const
{
    QMenu* menu = new QMenu();
    menu->setTitle(menuItem.value("title").toString());
    menu->setEnabled(menuItem.value("enabled").toBool());

    QActionGroup* group = new QActionGroup(view());

    for (const QVariant& menuObj: menuItem.value("subitems").toList()) {
        QVariantMap menuMap = menuObj.toMap();
        if (menuMap.value("title").toString().isEmpty()) {
            menu->addSeparator();
        } else if (!menuMap.value("subitems").toList().empty()) {
            QMenu* subMenu = makeMenu(menuMap);
            addMenu(subMenu, menu);
        } else {
            bool isFromGroup = menuMap.value("selectable").toBool();
            menu->addAction(makeAction(menuMap, isFromGroup ? group : nullptr));
        }
    }

    return menu;
}

QAction* DockMenuBar::makeAction(const QVariantMap& menuItem, QActionGroup* group) const
{
    QAction* action = new QAction();

    if (group) {
        group->addAction(action);
    }

    QVariantMap data;
    data["code"] = menuItem.value("code").toString();
    action->setData(data);

    action->setText(menuItem.value("title").toString());

    action->setShortcut(menuItem.value("shortcut").toString());
    // NOTE: Disable activate by hotkey
    action->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    action->setEnabled(menuItem.value("enabled").toBool());

    if (menuItem.value("checkable").toBool()) {
        action->setCheckable(true);
        action->setChecked(menuItem.value("checked").toBool());
    }

    if (menuItem.value("selectable").toBool()) {
        action->setCheckable(true);
        action->setChecked(menuItem.value("selected").toBool());
    }

    return action;
}
