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

#include "shortcutsmodel.h"

#include "ui/view/iconcodes.h"
#include "iinteractive.h"

using namespace mu::shortcuts;
using namespace mu::actions;
using namespace mu::ui;

ShortcutsModel::ShortcutsModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant ShortcutsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    Shortcut shortcut = m_shortcuts[index.row()];
    QString title = actionTitle(shortcut.action);
    QString sequence = QString::fromStdString(shortcut.sequence);

    switch (role) {
    case RoleTitle: return actionTitle(shortcut.action);
    case RoleSequence: return sequence;
    case RoleSearchKey: return title + sequence;
    }

    return QVariant();
}

QString ShortcutsModel::actionTitle(const std::string& actionCode) const
{
    ActionItem action = actionsRegister()->action(actionCode);
    QString title = QString::fromStdString(action.title);

    if (action.iconCode == IconCode::Code::NONE) {
        return title;
    }

    return QString(iconCodeToChar(action.iconCode)) + " " + title;
}

int ShortcutsModel::rowCount(const QModelIndex&) const
{
    return m_shortcuts.size();
}

QHash<int, QByteArray> ShortcutsModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { RoleTitle, "title" },
        { RoleSequence, "sequence" },
        { RoleSearchKey, "searchKey" }
    };

    return roles;
}

void ShortcutsModel::load()
{
    beginResetModel();

    for (const Shortcut& shortcut: shortcutsRegister()->shortcuts()) {
        m_shortcuts << shortcut;
    }

    endResetModel();
}

void ShortcutsModel::editShortcut(int index)
{
    Q_UNUSED(index);
    interactive()->open("musescore://shortcut/edit");
}
