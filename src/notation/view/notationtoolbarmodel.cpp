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

#include "notationtoolbarmodel.h"

#include "log.h"

#include "translation.h"

using namespace mu::notation;
using namespace mu::ui;

NotationToolBarModel::NotationToolBarModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int NotationToolBarModel::rowCount(const QModelIndex&) const
{
    return m_items.size();
}

QVariant NotationToolBarModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    MenuItem item = m_items[index.row()];

    switch (role) {
    case TitleRole: return item.title;
    case CodeRole: return QString::fromStdString(item.code);
    case HintRole: return item.description;
    case IconRole: return static_cast<int>(item.iconCode);
    case EnabledRole: return item.state.enabled;
    }

    return QVariant();
}

QHash<int, QByteArray> NotationToolBarModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { TitleRole, "title" },
        { CodeRole, "code" },
        { IconRole, "icon" },
        { EnabledRole, "enabled" },
        { HintRole, "hint" }
    };

    return roles;
}

void NotationToolBarModel::load()
{
    beginResetModel();

    m_items.clear();

    m_items << makeItem("parts");
    m_items << makeItem("toggle-mixer");

    endResetModel();

    context()->currentNotationChanged().onNotify(this, [this]() {
        load();
    });
}

void NotationToolBarModel::handleAction(const QString& actionCode)
{
    dispatcher()->dispatch(actions::codeFromQString(actionCode));
}

MenuItem NotationToolBarModel::makeItem(const ActionCode& actionCode) const
{
    MenuItem item = actionsRegister()->action(actionCode);
    item.state.enabled = context()->currentNotation() != nullptr;

    return item;
}
