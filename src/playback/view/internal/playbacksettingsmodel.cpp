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

#include "playbacksettingsmodel.h"

#include "playback/internal/playbackactions.h"

#include <QSet>

using namespace mu::playback;
using namespace mu::uicomponents;
using namespace mu::actions;

PlaybackSettingsModel::PlaybackSettingsModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant PlaybackSettingsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const MenuItem& item = m_items.at(index.row());

    switch (role) {
    case IconRole: return static_cast<int>(item.iconCode);
    case DescriptionRole: return QString::fromStdString(item.description);
    case CodeRole: return QString::fromStdString(item.code);
    case SectionRole: return QString::fromStdString(item.section);
    case CheckedRole: return item.checked;
    }

    return QVariant();
}

int PlaybackSettingsModel::rowCount(const QModelIndex&) const
{
    return m_items.size();
}

QHash<int, QByteArray> PlaybackSettingsModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { CodeRole, "code" },
        { DescriptionRole, "description" },
        { IconRole, "icon" },
        { CheckedRole, "checked" },
        { SectionRole, "section" },
    };

    return roles;
}

void PlaybackSettingsModel::load()
{
    beginResetModel();
    m_items.clear();

    for (const ActionItem& action : PlaybackActions::settingsActions()) {
        MenuItem item(action);
        item.section = resolveSection(action.code);
        item.checked = isActionEnabled(action.code);
        m_items.push_back(item);
    }

    controller()->actionEnabledChanged().onReceive(this, [this](const ActionCode& actionCode) {
        updateCheckedState(actionCode);
    });

    endResetModel();
}

void PlaybackSettingsModel::updateCheckedState(const ActionCode& actionCode)
{
    for (int i = 0; i < m_items.size(); ++i) {
        MenuItem& item = m_items[i];

        if (item.code == actionCode) {
            item.checked = isActionEnabled(actionCode);
            QModelIndex index = this->index(i);
            emit dataChanged(index, index, { CheckedRole });
            return;
        }
    }
}

std::string PlaybackSettingsModel::resolveSection(const ActionCode& actionCode) const
{
    return actionCode.find("loop") == std::string::npos ? "main" : "loop";
}

bool PlaybackSettingsModel::isActionEnabled(const ActionCode& actionCode) const
{
    return controller()->isActionEnabled(actionCode);
}

void PlaybackSettingsModel::handleAction(const QString& actionCode)
{
    dispatcher()->dispatch(actionCode.toStdString());
}
