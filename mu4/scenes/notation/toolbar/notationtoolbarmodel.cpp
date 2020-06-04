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

#include "actions/actions.h"

using namespace mu::scene::notation;
using namespace mu::actions;

NotationToolBarModel::NotationToolBarModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

void NotationToolBarModel::load()
{
    beginResetModel();

    m_actions << action("file-open")
              << action("note-input")
              << action("pad-note-8");

    endResetModel();
}

QVariant NotationToolBarModel::data(const QModelIndex& index, int role) const
{
    const Action& action = m_actions.at(index.row());
    switch (role) {
    case TitleRole: return QString::fromStdString(action.title);
    case NameRole: return QString::fromStdString(action.name);
    }
    return QVariant();
}

int NotationToolBarModel::rowCount(const QModelIndex&) const
{
    return m_actions.count();
}

QHash<int,QByteArray> NotationToolBarModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { NameRole, "nameRole" },
        { TitleRole, "titleRole" }
    };
    return roles;
}

void NotationToolBarModel::click(const QString& action)
{
    dispatcher()->dispatch(actions::namefromQString(action));
}
