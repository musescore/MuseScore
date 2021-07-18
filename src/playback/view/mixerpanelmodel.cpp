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

#include "mixerpanelmodel.h"

#include "log.h"
#include "translation.h"

using namespace mu::playback;
using namespace mu::audio;

MixerPanelModel::MixerPanelModel(QObject* parent)
    : QAbstractListModel(parent)
{
    controller()->currentTrackSequenceIdChanged().onNotify(this, [this]() {
        load();
    });
}

void MixerPanelModel::load()
{
    TrackSequenceId sequenceId = controller()->currentTrackSequenceId();

    playback()->tracks()->trackIdList(sequenceId)
    .onResolve(this, [this, sequenceId](const TrackIdList& trackIdList) {
        loadItems(sequenceId, trackIdList);
    })
    .onReject(this, [sequenceId](int errCode, std::string text) {
        LOGE() << "unable to find track sequence:" << sequenceId << ", error code: " << errCode
               << ", " << text;
    });

    playback()->tracks()->trackAdded().onReceive(this, [this](const TrackSequenceId sequenceId, const TrackId trackId) {
        addItem(sequenceId, trackId);
    });

    playback()->tracks()->trackRemoved().onReceive(this, [this](const TrackSequenceId /*sequenceId*/, const TrackId trackId) {
        removeItem(trackId);
    });
}

QVariant MixerPanelModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount() || role != ItemRole) {
        return QVariant();
    }

    return QVariant::fromValue(m_mixerChannelList.at(index.row()));
}

int MixerPanelModel::rowCount(const QModelIndex&) const
{
    return m_mixerChannelList.count();
}

QHash<int, QByteArray> MixerPanelModel::roleNames() const
{
    static QHash<int, QByteArray> roles = {
        { ItemRole, "item" }
    };

    return roles;
}

void MixerPanelModel::loadItems(const TrackSequenceId sequenceId, const TrackIdList& trackIdList)
{
    beginResetModel();

    clear();

    for (TrackId trackId : trackIdList) {
        m_mixerChannelList.append(buildChannelItem(sequenceId, trackId));
    }

    m_mixerChannelList.append(buildMasterChannelItem());
    sortItems();

    endResetModel();
}

void MixerPanelModel::addItem(const audio::TrackSequenceId sequenceId, const audio::TrackId trackId)
{
    beginResetModel();

    m_mixerChannelList.append(buildChannelItem(sequenceId, trackId));
    sortItems();

    endResetModel();
}

void MixerPanelModel::removeItem(const audio::TrackId trackId)
{
    beginResetModel();

    std::remove_if(m_mixerChannelList.begin(), m_mixerChannelList.end(), [&trackId](const MixerChannelItem* item) {
        return item->id() == trackId;
    });

    sortItems();

    endResetModel();
}

void MixerPanelModel::sortItems()
{
    std::sort(m_mixerChannelList.begin(), m_mixerChannelList.end(), [](const MixerChannelItem* f, const MixerChannelItem* s) {
        if (f->isMasterChannel()) {
            return false;
        }

        return f->id() < s->id();
    });
}

void MixerPanelModel::clear()
{
    qDeleteAll(m_mixerChannelList);
    m_mixerChannelList.clear();
}

MixerChannelItem* MixerPanelModel::buildChannelItem(const audio::TrackSequenceId& sequenceId, const audio::TrackId& trackId)
{
    MixerChannelItem* item = new MixerChannelItem(this, trackId);

    playback()->tracks()->inputParams(sequenceId, trackId)
    .onResolve(this, [item](AudioInputParams inParams) {
        item->setInputParams(std::move(inParams));
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to get track output parameters, error code: " << errCode
               << ", " << text;
    });

    playback()->audioOutput()->outputParams(sequenceId, trackId)
    .onResolve(this, [item](AudioOutputParams outParams) {
        item->setOutputParams(std::move(outParams));
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to get track output parameters, error code: " << errCode
               << ", " << text;
    });

    playback()->audioOutput()->signalChanges(sequenceId, trackId)
    .onResolve(this, [item](AudioSignalChanges signalChanges) {
        item->subscribeOnAudioSignalChanges(std::move(signalChanges));
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to subscribe on audio signal changes from mixer channel, error code: " << errCode
               << ", " << text;
    });

    connect(item, &MixerChannelItem::inputParamsChanged, this, [this, sequenceId, trackId](const AudioInputParams& params) {
        playback()->tracks()->setInputParams(sequenceId, trackId, params);
    });

    connect(item, &MixerChannelItem::outputParamsChanged, this, [this, sequenceId, trackId](const AudioOutputParams& params) {
        playback()->audioOutput()->setOutputParams(sequenceId, trackId, params);
    });

    connect(item, &MixerChannelItem::soloChanged, this, [this, item](const bool solo) {
        for (MixerChannelItem* ch : m_mixerChannelList) {
            if (item == ch || ch->isMasterChannel()) {
                continue;
            }

            ch->setMuted(!solo);
        }
    });

    return item;
}

MixerChannelItem* MixerPanelModel::buildMasterChannelItem()
{
    MixerChannelItem* item = new MixerChannelItem(this, /*trackId*/ -1, /*isMaster*/ true);

    item->setTitle(qtrc("playback", "Master"));

    playback()->audioOutput()->masterOutputParams()
    .onResolve(this, [item](AudioOutputParams outParams) {
        item->setOutputParams(std::move(outParams));
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to get master output parameters, error code: " << errCode
               << ", " << text;
    });

    playback()->audioOutput()->masterSignalChanges()
    .onResolve(this, [item](AudioSignalChanges signalChanges) {
        item->subscribeOnAudioSignalChanges(std::move(signalChanges));
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to subscribe on audio signal changes from master channel, error code: " << errCode
               << ", " << text;
    });

    connect(item, &MixerChannelItem::outputParamsChanged, this, [this](const AudioOutputParams& params) {
        playback()->audioOutput()->setMasterOutputParams(params);
    });

    return item;
}
