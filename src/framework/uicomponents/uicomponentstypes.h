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
#ifndef MU_UICOMPONENTS_UICOMPONENTSTYPES_H
#define MU_UICOMPONENTS_UICOMPONENTSTYPES_H

#include <QString>
#include <QVariantMap>

#include "actions/actiontypes.h"

namespace mu::uicomponents {
struct MenuItem : public actions::ActionItem
{
    std::string shortcut;
    std::string section;
    bool enabled = false;
    bool checked = false;

    actions::ActionData data;

    QList<MenuItem> subitems;

    MenuItem() = default;

    MenuItem(const actions::ActionItem& parent)
        : ActionItem(parent) {}

    QVariantMap toMap() const
    {
        QVariantList _subitems;
        for (const MenuItem& item: subitems) {
            _subitems << item.toMap();
        }

        return {
            { "code", QString::fromStdString(code) },
            { "shortcut", QString::fromStdString(shortcut) },
            { "title", QString::fromStdString(title) },
            { "description", QString::fromStdString(description) },
            { "section", QString::fromStdString(section) },
            { "icon", static_cast<int>(iconCode) },
            { "enabled", enabled },
            { "checked", checked },
            { "subitems", _subitems }
        };
    }

    static MenuItem fromMap(const QVariantMap& map)
    {
        MenuItem item;
        item.code = map.value("code").toString().toStdString();
        item.shortcut = map.value("shortcut").toString().toStdString();
        item.title = map.value("title").toString().toStdString();
        item.description = map.value("description").toString().toStdString();
        item.section = map.value("section").toString().toStdString();
        item.iconCode = static_cast<ui::IconCode::Code>(map.value("icon").toInt());
        item.enabled = map.value("enabled").toBool();
        item.checked = map.value("checked").toBool();

        for (const QVariant& _item: map.value("subitems").toList()) {
            item.subitems << fromMap(_item.toMap());
        }

        return item;
    }
};

using MenuItemList = QList<MenuItem>;
}

#endif // MU_UICOMPONENTS_UICOMPONENTSTYPES_H
