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
#include "playbacktoolbarmodel.h"

#include "log.h"
#include "translation.h"

using namespace mu::playback;
using namespace mu::actions;
using namespace mu::framework;

PlaybackToolBarModel::PlaybackToolBarModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant PlaybackToolBarModel::data(const QModelIndex& index, int role) const
{
    const MenuItem& item = m_items.at(index.row());
    switch (role) {
    case TitleRole: return QString::fromStdString(item.title);
    case CodeRole: return QString::fromStdString(item.code);
    case EnabledRole: return item.enabled;
    case CheckedRole: return item.checked;
    }
    return QVariant();
}

int PlaybackToolBarModel::rowCount(const QModelIndex&) const
{
    return m_items.count();
}

QHash<int,QByteArray> PlaybackToolBarModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { CodeRole, "codeRole" },
        { TitleRole, "titleRole" },
        { EnabledRole, "enabledRole" },
        { CheckedRole, "checkedRole" }
    };
    return roles;
}

void PlaybackToolBarModel::load()
{
    beginResetModel();

    m_items << ActionItem("play", shortcuts::ShortcutContext::Any, trc("playback", "Play"));

    endResetModel();

    updateState();

    playbackController()->isPlayAllowedChanged().onNotify(this, [this]() {
        updateState();
    });

    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        updateState();
    });
}

void PlaybackToolBarModel::click(const QString& action)
{
    LOGI() << action;

    dispatcher()->dispatch(actions::codeFromQString(action));
}

void PlaybackToolBarModel::updateState()
{
    bool isPlayAllowed = playbackController()->isPlayAllowed();

    if (!isPlayAllowed) {
        for (MenuItem& item : m_items) {
            item.enabled = false;
            item.checked = false;
        }
    } else {
        for (MenuItem& item : m_items) {
            item.enabled = true;
            item.checked = false;
        }

        item("play").checked = playbackController()->isPlaying();
    }

    emit dataChanged(index(0), index(rowCount() - 1));
}

MenuItem& PlaybackToolBarModel::item(const actions::ActionCode& actionCode)
{
    for (MenuItem& item : m_items) {
        if (item.code == actionCode) {
            return item;
        }
    }

    LOGE() << "item not found with name: " << actionCode;
    static MenuItem null;
    return null;
}
