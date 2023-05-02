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
#include "context/iglobalcontext.h"
#include "ui/view/navigationsection.h"

#include "iplaybackcontroller.h"
#include "internal/mixerchannelitem.h"

namespace mu::playback {
class MixerPanelModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(audio::IPlayback, playback)
    INJECT(IPlaybackController, controller)
    INJECT(context::IGlobalContext, context)

    Q_PROPERTY(
        mu::ui::NavigationSection * navigationSection READ navigationSection WRITE setNavigationSection NOTIFY navigationSectionChanged)

    Q_PROPERTY(int count READ rowCount NOTIFY rowCountChanged)

public:
    explicit MixerPanelModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE QVariantMap get(int index);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    ui::NavigationSection* navigationSection() const;
    void setNavigationSection(ui::NavigationSection* navigationSection);

signals:
    void navigationSectionChanged();
    void rowCountChanged();

private:
    enum Roles {
        ChannelItemRole = Qt::UserRole + 1
    };

    void loadItems();
    void addItem(MixerChannelItem* item, int index);
    void removeItem(const audio::TrackId trackId);
    void updateItemsPanelsOrder();
    void clear();
    void setupConnections();

    int resolveInsertIndex(const engraving::InstrumentTrackId& instrumentTrackId) const;
    int indexOf(const audio::TrackId trackId) const;

    MixerChannelItem* buildInstrumentChannelItem(const audio::TrackId trackId, const engraving::InstrumentTrackId& instrumentTrackId,
                                                 bool isPrimary = true);
    MixerChannelItem* buildAuxChannelItem(const audio::TrackId trackId);
    MixerChannelItem* buildMasterChannelItem();

    MixerChannelItem* findChannelItem(const audio::TrackId& trackId) const;

    void loadOutputParams(MixerChannelItem* item, audio::AudioOutputParams&& params);
    void updateOutputResourceItemCount();

    project::INotationProjectPtr currentProject() const;
    project::IProjectAudioSettingsPtr audioSettings() const;
    notation::INotationPlaybackPtr notationPlayback() const;
    notation::INotationPartsPtr masterNotationParts() const;

    QList<MixerChannelItem*> m_mixerChannelList;
    MixerChannelItem* m_masterChannelItem = nullptr;
    audio::TrackSequenceId m_currentTrackSequenceId = -1;

    ui::NavigationSection* m_navigationSection = nullptr;
};
}

#endif // MU_PLAYBACK_MIXERPANELMODEL_H
