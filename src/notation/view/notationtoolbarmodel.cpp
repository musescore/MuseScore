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
using namespace mu::framework;

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

    ToolbarItem item = m_items[index.row()];

    switch (role) {
    case TitleRole: return item.title;
    case IconRole: return item.icon;
    case EnabledRole: return item.enabled;
    }

    return QVariant();
}

QHash<int, QByteArray> NotationToolBarModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { TitleRole, "title" },
        { IconRole, "icon" },
        { EnabledRole, "enabled" }
    };

    return roles;
}

void NotationToolBarModel::load()
{
    beginResetModel();

    m_items.clear();

    m_items << makeItem("Parts", IconCode::Code::NEW_FILE, "musescore://notation/parts", hasNotation());
    m_items << makeItem("Mixer", IconCode::Code::MIXER, "musescore://notation/mixer");

    endResetModel();

    context()->currentNotationChanged().onNotify(this, [this]() {
        load();
    });
}

void NotationToolBarModel::open(int index)
{
    if (index < 0 || index >= m_items.size()) {
        return;
    }

    Ret ret = interactive()->open(m_items[index].uri).ret;

    if (!ret) {
        LOGE() << ret.toString();
    }
}

NotationToolBarModel::ToolbarItem NotationToolBarModel::makeItem(std::string_view title, IconCode::Code icon, std::string uri,
                                                                 bool enabled) const
{
    ToolbarItem item;

    item.title = qtrc("notation", title.data());
    item.icon = static_cast<int>(icon);
    item.uri = std::move(uri);
    item.enabled = enabled;

    return item;
}

bool NotationToolBarModel::hasNotation() const
{
    return context()->currentNotation() != nullptr;
}
