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
#ifndef MU_PLAYBACK_PLAYBACKSETTINGSMODEL_H
#define MU_PLAYBACK_PLAYBACKSETTINGSMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "ui/uitypes.h"
#include "playback/iplaybackcontroller.h"
#include "async/asyncable.h"

namespace mu::playback {
class PlaybackSettingsModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(playback, actions::IActionsDispatcher, dispatcher)
    INJECT(playback, playback::IPlaybackController, controller)

public:
    explicit PlaybackSettingsModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void handleAction(const QString& actionCode);

private:
    enum Roles {
        CodeRole = Qt::UserRole + 1,
        DescriptionRole,
        IconRole,
        CheckedRole,
        SectionRole
    };

    void updateCheckedState(const actions::ActionCode& actionCode);

    QString resolveSection(const actions::ActionCode& actionCode) const;
    bool isActionEnabled(const actions::ActionCode& actionCode) const;

    QList<ui::MenuItem> m_items;
};
}

#endif // MU_PLAYBACK_PLAYBACKSETTINGSMODEL_H
