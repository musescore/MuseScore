/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "audio/main/iplayback.h"
#include "context/iglobalcontext.h"
#include "ui/view/navigationsection.h"
#include "playback/iplaybackconfiguration.h"

#include "iplaybackcontroller.h"
#include "internal/mixerchannelitem.h"

namespace mu::playback {
class MixerPanelModel : public QAbstractListModel, public muse::async::Asyncable
{
    Q_OBJECT

    INJECT(muse::audio::IPlayback, playback)
    INJECT(IPlaybackController, controller)
    INJECT(context::IGlobalContext, context)
    INJECT(IPlaybackConfiguration, configuration)

    Q_PROPERTY(
        muse::ui::NavigationSection * navigationSection READ navigationSection WRITE setNavigationSection NOTIFY navigationSectionChanged)
    Q_PROPERTY(int navigationOrderStart READ navigationOrderStart WRITE setNavigationOrderStart NOTIFY navigationOrderStartChanged)

    Q_PROPERTY(int count READ rowCount NOTIFY rowCountChanged)

public:
    explicit MixerPanelModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE QVariantMap get(int index);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    muse::ui::NavigationSection* navigationSection() const;
    void setNavigationSection(muse::ui::NavigationSection* navigationSection);

    int navigationOrderStart() const;
    void setNavigationOrderStart(int navigationOrderStart);

signals:
    void navigationSectionChanged();
    void navigationOrderStartChanged();
    void rowCountChanged();

private:
    enum Roles {
        ChannelItemRole = Qt::UserRole + 1
    };

    void loadItems();
    void onTrackAdded(const muse::audio::TrackId& trackId);
    void addItem(MixerChannelItem* item, int index);
    void removeItem(const muse::audio::TrackId trackId);
    void updateItemsPanelsOrder();
    void clear();
    void setupConnections();

    int resolveInsertIndex(const engraving::InstrumentTrackId& instrumentTrackId) const;
    int indexOf(const muse::audio::TrackId trackId) const;

    MixerChannelItem* buildInstrumentChannelItem(const muse::audio::TrackId trackId, const engraving::InstrumentTrackId& instrumentTrackId,
                                                 bool isPrimary = true);
    MixerChannelItem* buildAuxChannelItem(muse::audio::aux_channel_idx_t index, const muse::audio::TrackId trackId);
    MixerChannelItem* buildMasterChannelItem();

    int masterChannelIndex() const;

    MixerChannelItem* findChannelItem(const muse::audio::TrackId& trackId) const;

    void loadOutputParams(MixerChannelItem* item, muse::audio::AudioOutputParams&& params);
    void updateOutputResourceItemCount();

    project::INotationProjectPtr currentProject() const;
    project::IProjectAudioSettingsPtr audioSettings() const;
    notation::INotationPlaybackPtr notationPlayback() const;
    notation::INotationPartsPtr masterNotationParts() const;

    QList<MixerChannelItem*> m_mixerChannelList;
    MixerChannelItem* m_masterChannelItem = nullptr;
    muse::audio::TrackSequenceId m_currentTrackSequenceId = -1;

    muse::ui::NavigationSection* m_navigationSection = nullptr;
    int m_navigationOrderStart = 1;
};
}

#endif // MU_PLAYBACK_MIXERPANELMODEL_H
