/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#pragma once

#include <QAbstractListModel>
#include <QQmlParserStatus>
#include <QList>
#include <QHash>
#include <qqmlintegration.h>

#include <optional>
#include <vector>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "audio/main/iplayback.h"
#include "project/iprojectsession.h"
#include "playback/iplaybackconfiguration.h"
#include "project/iprojectaudiosettings.h"
#include "ui/qml/Muse/Ui/navigationsection.h"

#include "iplaybackcontroller.h"
#include "mixerchannelitem.h"

namespace mu::playback {
class MixerPanelModel : public QAbstractListModel, public QQmlParserStatus, public muse::async::Asyncable, public muse::Contextable
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(
        muse::ui::NavigationSection * navigationSection READ navigationSection WRITE setNavigationSection NOTIFY navigationSectionChanged)
    Q_PROPERTY(int navigationOrderStart READ navigationOrderStart WRITE setNavigationOrderStart NOTIFY navigationOrderStartChanged)

    Q_PROPERTY(int count READ rowCount NOTIFY rowCountChanged)

    QML_ELEMENT

    muse::GlobalInject<IPlaybackConfiguration> configuration;
    muse::ContextInject<muse::audio::IPlayback> playback = { this };
    muse::ContextInject<IPlaybackController> controller = { this };
    muse::ContextInject<project::IProjectSession> context = { this };

public:
    explicit MixerPanelModel(QObject* parent = nullptr);

    Q_INVOKABLE QVariantMap get(int index);

    Q_INVOKABLE QVariantList channelContextMenuItems(int index) const;
    Q_INVOKABLE void handleChannelContextMenuItem(int index, const QString& itemId);
    Q_INVOKABLE void renameVcaGroup(int index, const QString& name);

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
    void classBegin() override {}
    void componentComplete() override;
    void init();

    enum Roles {
        ChannelItemRole = Qt::UserRole + 1
    };

    void reload();
    void reloadItems();
    void onTrackAdded(const muse::audio::TrackId& trackId);
    void addItem(MixerChannelItem* item, int index);
    void removeItem(const muse::audio::TrackId trackId);
    void updateItemsPanelsOrder();
    void clear();
    void setupConnections();

    void subscribeOnAutomationChanges();

    int resolveInsertIndex(const engraving::InstrumentTrackId& instrumentTrackId) const;
    int indexOf(const muse::audio::TrackId trackId) const;

    MixerChannelItem* buildInstrumentChannelItem(const muse::audio::TrackId trackId, const engraving::InstrumentTrackId& instrumentTrackId,
                                                 bool isPrimary = true);
    MixerChannelItem* buildAuxChannelItem(muse::audio::aux_channel_idx_t index, const muse::audio::TrackId trackId);
    MixerChannelItem* buildMasterChannelItem();
    MixerChannelItem* buildVcaChannelItem(size_t groupIndex, const project::VcaGroup& group);

    void reloadVcaItems();
    int vcaInsertIndex() const;
    void onVcaControlParamsChanged(MixerChannelItem* vcaItem);
    bool shiftVcaMembersVolume(project::VcaGroup& group, float requestedVolume) const;
    bool shiftVcaMembersBalance(project::VcaGroup& group, int requestedBalance, float balanceScale) const;
    void onVcaSoloMuteStateChanged(MixerChannelItem* vcaItem, const notation::INotationSoloMuteState::SoloMuteState& state);
    void onVcaAuxSendsChangeRequested(MixerChannelItem* vcaItem, const muse::audio::AuxSendsParams& requested);
    muse::audio::AuxSendsParams vcaAuxSends(const project::VcaGroup& group) const;
    void refreshVcaAuxSends(size_t groupIndex);
    std::vector<MixerChannelItem*> vcaMemberItems(const project::VcaGroup& group) const;
    MixerChannelItem* findVcaChannelItem(size_t groupIndex) const;

    std::optional<size_t> vcaGroupIndexOf(const engraving::InstrumentTrackId& instrumentTrackId) const;
    std::optional<size_t> vcaGroupIndexOf(const MixerChannelItem* vcaItem) const;
    void addToVcaGroup(const engraving::InstrumentTrackId& instrumentTrackId, std::optional<size_t> groupIndex);
    void removeFromVcaGroup(const engraving::InstrumentTrackId& instrumentTrackId);
    void deleteVcaGroup(size_t groupIndex);
    void applyVcaGroupsChange(const project::VcaGroupList& groups);
    QString newVcaGroupName(const project::VcaGroupList& groups) const;

    MixerChannelItem* findInstrumentChannelItem(const engraving::InstrumentTrackId& instrumentTrackId) const;

    int masterChannelIndex() const;

    MixerChannelItem* findChannelItem(const muse::audio::TrackId& trackId) const;

    void loadOutputParams(MixerChannelItem* item, const project::AudioOutputParams& params);
    void updateOutputResourceItemCount();

    project::INotationProjectPtr currentProject() const;
    project::IProjectAudioSettingsPtr audioSettings() const;
    notation::INotationPlaybackPtr notationPlayback() const;
    notation::INotationPartsPtr masterNotationParts() const;

    QList<MixerChannelItem*> m_mixerChannelList;
    MixerChannelItem* m_masterChannelItem = nullptr;
    QHash<const MixerChannelItem*, size_t> m_vcaGroupIndexByItem;

    muse::ui::NavigationSection* m_navigationSection = nullptr;
    int m_navigationOrderStart = 1;
};
}
