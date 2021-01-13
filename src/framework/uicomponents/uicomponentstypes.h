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
#ifndef MU_FRAMEWORK_UICOMPONENTSTYPES_H
#define MU_FRAMEWORK_UICOMPONENTSTYPES_H

#include <QString>
#include <QVariantMap>

#include "actions/actiontypes.h"

namespace mu::framework {
struct MenuItem : public actions::ActionItem
{
    std::string shortcut;
    std::string section;
    bool enabled = false;
    bool checked = false;

    MenuItem() = default;

    MenuItem(const actions::ActionItem& parent)
        : ActionItem(parent) {}

    QVariantMap toVariantMap() const
    {
        return {
            { "code", QString::fromStdString(code) },
            { "shortcut", QString::fromStdString(shortcut) },
            { "title", QString::fromStdString(title) },
            { "description", QString::fromStdString(description) },
            { "section", QString::fromStdString(section) },
            { "icon", static_cast<int>(iconCode) },
            { "enabled", enabled },
            { "checked", checked }
        };
    }
};
using MenuItemList = QList<MenuItem>;
}

#endif // MU_FRAMEWORK_UICOMPONENTSTYPES_H
