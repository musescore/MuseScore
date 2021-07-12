/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MU_PLAYBACK_MIXERPANELMODEL_H
#define MU_PLAYBACK_MIXERPANELMODEL_H

#include <QAbstractListModel>
#include <QList>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "audio/itracks.h"
#include "audio/iplayback.h"

#include "iplaybackcontroller.h"
#include "internal/mixerchannelitem.h"

namespace mu::playback {
class MixerPanelModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(playback, audio::IPlayback, playback)
    INJECT(playback, IPlaybackController, controller)

public:
    explicit MixerPanelModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    enum Roles {
        ItemRole = Qt::UserRole + 1
    };

    void loadItems(const audio::TrackSequenceId sequenceId, const audio::TrackIdList& trackIdList);
    void addItem(const audio::TrackSequenceId sequenceId, const audio::TrackId trackId);
    void removeItem(const audio::TrackId trackId);
    void sortItems();
    void clear();

    MixerChannelItem* buildChannelItem(const audio::TrackSequenceId& sequenceId, const audio::TrackId& trackId);
    MixerChannelItem* buildMasterChannelItem();

    QList<MixerChannelItem*> m_mixerChannelList;
};
}

#endif // MU_PLAYBACK_MIXERPANELMODEL_H
