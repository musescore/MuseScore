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
#include "playtoolbarmodel.h"

#include "log.h"

using namespace mu::scene::common;
using namespace mu::actions;
using namespace mu::domain::notation;

PlayToolBarModel::PlayToolBarModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant PlayToolBarModel::data(const QModelIndex& index, int role) const
{
    const ActionItem& item = m_items.at(index.row());
    switch (role) {
    case TitleRole: return QString::fromStdString(item.action.title);
    case NameRole: return QString::fromStdString(item.action.name);
    case EnabledRole: return item.enabled;
    case CheckedRole: return item.checked;
    }
    return QVariant();
}

int PlayToolBarModel::rowCount(const QModelIndex&) const
{
    return m_items.count();
}

QHash<int,QByteArray> PlayToolBarModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { NameRole, "nameRole" },
        { TitleRole, "titleRole" },
        { EnabledRole, "enabledRole" },
        { CheckedRole, "checkedRole" }
    };
    return roles;
}

void PlayToolBarModel::load()
{
    auto makeItem = [](const Action& action) {
                        ActionItem item;
                        item.action = action;
                        return item;
                    };

    beginResetModel();

    m_items << makeItem(Action { "domain/audio/play", "Play" });

    endResetModel();

    updateState();
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        updateState();
    });

    globalContext()->isPlayingChanged().onNotify(this, [this]() {
        updateState();
    });
}

void PlayToolBarModel::click(const QString& action)
{
    LOGI() << action;

    //! NOTE This is fake play, added for demonstration
    if (action == "domain/audio/play") {
        globalContext()->setIsPlaying(!globalContext()->isPlaying());
    }
}

void PlayToolBarModel::updateState()
{
    std::shared_ptr<INotation> notation = globalContext()->currentNotation();

    if (!notation) {
        for (ActionItem& item : m_items) {
            item.enabled = false;
            item.checked = false;
        }
    } else {
        for (ActionItem& item : m_items) {
            item.enabled = true;
            item.checked = false;
        }

        item("domain/audio/play").checked = globalContext()->isPlaying();
    }

    emit dataChanged(index(0), index(rowCount() - 1));
}

PlayToolBarModel::ActionItem& PlayToolBarModel::item(const actions::ActionName& name)
{
    for (ActionItem& item : m_items) {
        if (item.action.name == name) {
            return item;
        }
    }

    LOGE() << "item not found with name: " << name;
    static ActionItem null;
    return null;
}
