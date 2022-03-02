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

static constexpr int INVALID_INDEX = -1;

MixerPanelModel::MixerPanelModel(QObject* parent)
    : QAbstractListModel(parent)
{
    controller()->currentTrackSequenceIdChanged().onNotify(this, [this]() {
        load(QVariant::fromValue(m_itemsNavigationSection));
    });
}

void MixerPanelModel::load(const QVariant& navigationSection)
{
    TRACEFUNC;

    TrackSequenceId sequenceId = controller()->currentTrackSequenceId();

    if (m_currentTrackSequenceId == sequenceId) {
        return;
    }

    m_itemsNavigationSection = navigationSection.value<ui::NavigationSection*>();
    m_currentTrackSequenceId = sequenceId;

    playback()->tracks()->trackAdded().onReceive(this, [this](const TrackSequenceId sequenceId, const TrackId trackId) {
        addItem(sequenceId, trackId);
    });

    playback()->tracks()->trackRemoved().onReceive(this, [this](const TrackSequenceId /*sequenceId*/, const TrackId trackId) {
        removeItem(trackId);
    });

    playback()->tracks()->trackIdList(sequenceId)
    .onResolve(this, [this, sequenceId](const TrackIdList& trackIdList) {
        loadItems(sequenceId, trackIdList);
    })
    .onReject(this, [sequenceId](int errCode, std::string text) {
        LOGE() << "unable to find track sequence:" << sequenceId << ", error code: " << errCode
               << ", " << text;
    });
}

QVariantMap MixerPanelModel::get(int index)
{
    QVariantMap result;

    QHash<int, QByteArray> names = roleNames();
    QHashIterator<int, QByteArray> i(names);
    while (i.hasNext()) {
        i.next();
        QModelIndex idx = this->index(index, 0);
        QVariant data = idx.data(i.key());
        result[i.value()] = data;
    }

    return result;
}

QVariant MixerPanelModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount() || role != ChannelItemRole) {
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
        { ChannelItemRole, "channelItem" }
    };

    return roles;
}

void MixerPanelModel::loadItems(const TrackSequenceId sequenceId, const TrackIdList& trackIdList)
{
    TRACEFUNC;

    beginResetModel();

    clear();

    TrackIdList sortedTrackIdList = trackIdList;
    std::sort(sortedTrackIdList.begin(), sortedTrackIdList.end(), [](const TrackId& f, const TrackId& s) {
        return f < s;
    });

    for (const TrackId& trackId : sortedTrackIdList) {
        m_mixerChannelList.append(buildTrackChannelItem(sequenceId, trackId));
    }

    m_mixerChannelList.append(buildMasterChannelItem());
    updateItemsPanelsOrder();

    endResetModel();
    emit rowCountChanged();
}

void MixerPanelModel::addItem(const audio::TrackSequenceId sequenceId, const audio::TrackId trackId)
{
    TRACEFUNC;

    int index = resolveInsertIndex(trackId);
    if (index == INVALID_INDEX) {
        return;
    }

    beginInsertRows(QModelIndex(), index, index);

    MixerChannelItem* item = buildTrackChannelItem(sequenceId, trackId);
    m_mixerChannelList.insert(index, item);
    updateItemsPanelsOrder();

    endInsertRows();

    emit rowCountChanged();
}

void MixerPanelModel::removeItem(const audio::TrackId trackId)
{
    TRACEFUNC;

    int index = indexOf(trackId);
    if (index == INVALID_INDEX) {
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);

    m_mixerChannelList.removeAt(index);
    updateItemsPanelsOrder();

    endRemoveRows();

    emit rowCountChanged();
}

void MixerPanelModel::updateItemsPanelsOrder()
{
    TRACEFUNC;

    for (int i = 0; i < m_mixerChannelList.size(); i++) {
        m_mixerChannelList[i]->setPanelOrder(i);
    }
}

void MixerPanelModel::clear()
{
    TRACEFUNC;

    qDeleteAll(m_mixerChannelList);
    m_mixerChannelList.clear();
}

int MixerPanelModel::resolveInsertIndex(const audio::TrackId trackId) const
{
    for (int i = 0; i < m_mixerChannelList.size(); ++i) {
        TrackId id = m_mixerChannelList[i]->id();

        if (trackId == id) {
            return INVALID_INDEX;
        }

        if (trackId < m_mixerChannelList[i]->id()) {
            return i;
        }
    }

    //! NOTE: the last index is always reserved for the master channel
    return m_mixerChannelList.size() - 1;
}

int MixerPanelModel::indexOf(const audio::TrackId trackId) const
{
    for (int i = 0; i < m_mixerChannelList.size(); ++i) {
        if (trackId == m_mixerChannelList[i]->id()) {
            return i;
        }
    }

    return INVALID_INDEX;
}

MixerChannelItem* MixerPanelModel::buildTrackChannelItem(const audio::TrackSequenceId& sequenceId, const audio::TrackId& trackId)
{
    MixerChannelItem* item = new MixerChannelItem(this, trackId);
    item->setPanelSection(m_itemsNavigationSection);

    playback()->tracks()->inputParams(sequenceId, trackId)
    .onResolve(this, [item](AudioInputParams inParams) {
        item->loadInputParams(std::move(inParams));
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to get track output parameters, error code: " << errCode
               << ", " << text;
    });

    playback()->tracks()->inputParamsChanged().onReceive(this,
                                                         [this](const TrackSequenceId sequenceId,
                                                                const TrackId trackId,
                                                                AudioInputParams params) {
        if (m_currentTrackSequenceId != sequenceId) {
            return;
        }

        MixerChannelItem* item = trackChannelItem(trackId);

        if (!item) {
            return;
        }

        item->loadInputParams(std::move(params));
    });

    playback()->tracks()->trackName(sequenceId, trackId)
    .onResolve(this, [item](const TrackName& trackName) {
        item->setTitle(QString::fromStdString(trackName));
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to get track name, error code: " << errCode
               << ", " << text;
    });

    playback()->audioOutput()->outputParams(sequenceId, trackId)
    .onResolve(this, [item](AudioOutputParams outParams) {
        item->loadOutputParams(std::move(outParams));
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to get track output parameters, error code: " << errCode
               << ", " << text;
    });

    playback()->audioOutput()->outputParamsChanged().onReceive(this,
                                                               [this](const TrackSequenceId sequenceId,
                                                                      const TrackId trackId,
                                                                      AudioOutputParams params) {
        if (m_currentTrackSequenceId != sequenceId) {
            return;
        }

        MixerChannelItem* item = trackChannelItem(trackId);

        if (!item) {
            return;
        }

        item->loadOutputParams(std::move(params));
    }, AsyncMode::AsyncSetRepeat);

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

    connect(item, &MixerChannelItem::soloStateToggled, this, [this, item](const bool solo) {
        for (MixerChannelItem* ch : m_mixerChannelList) {
            if (item == ch || ch->isMasterChannel() || ch->solo()) {
                continue;
            }

            ch->setMutedBySolo(solo);
        }
    });

    return item;
}

MixerChannelItem* MixerPanelModel::buildMasterChannelItem()
{
    MixerChannelItem* item = new MixerChannelItem(this, /*trackId*/ -1, /*isMaster*/ true);
    item->setPanelSection(m_itemsNavigationSection);

    item->setTitle(qtrc("playback", "Master"));

    playback()->audioOutput()->masterOutputParams()
    .onResolve(this, [item](AudioOutputParams outParams) {
        item->loadOutputParams(std::move(outParams));
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

    connect(item, &MixerChannelItem::soloStateToggled, this, [this, item](const bool solo) {
        for (MixerChannelItem* ch : m_mixerChannelList) {
            if (item == ch || ch->solo()) {
                continue;
            }

            ch->setMutedBySolo(solo);
        }
    });

    return item;
}

MixerChannelItem* MixerPanelModel::trackChannelItem(const audio::TrackId& trackId) const
{
    for (MixerChannelItem* item : m_mixerChannelList) {
        if (item->id() == trackId) {
            return item;
        }
    }

    return nullptr;
}
