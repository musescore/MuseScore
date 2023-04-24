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

#include "defer.h"

#include "log.h"
#include "translation.h"

using namespace mu::playback;
using namespace mu::audio;
using namespace mu::engraving;
using namespace mu::notation;
using namespace mu::project;

static constexpr int INVALID_INDEX = -1;

MixerPanelModel::MixerPanelModel(QObject* parent)
    : QAbstractListModel(parent)
{
    controller()->currentTrackSequenceIdChanged().onNotify(this, [this]() {
        load(QVariant::fromValue(m_itemsNavigationSection), m_navigationPanelOrderStart);
    });
}

void MixerPanelModel::load(const QVariant& navigationSection, int navigationPanelOrderStart)
{
    TRACEFUNC;

    TrackSequenceId sequenceId = controller()->currentTrackSequenceId();

    if (m_currentTrackSequenceId == sequenceId) {
        return;
    }

    m_itemsNavigationSection = navigationSection.value<ui::NavigationSection*>();
    m_navigationPanelOrderStart = navigationPanelOrderStart;
    m_currentTrackSequenceId = sequenceId;

    controller()->trackAdded().onReceive(this, [this](const TrackId trackId) {
        const TrackIdList& auxTrackIdList = controller()->auxTrackIdList();
        if (mu::contains(auxTrackIdList, trackId)) {
            MixerChannelItem* item = buildAuxChannelItem(trackId);
            addItem(item, m_mixerChannelList.size() - 1);
            return;
        }

        const IPlaybackController::InstrumentTrackIdMap& instrumentTracks = controller()->instrumentTrackIdMap();
        InstrumentTrackId instrumentTrackId = mu::key(instrumentTracks, trackId);

        const Part* part = masterNotationParts()->part(instrumentTrackId.partId);
        bool isPrimary = part ? part->instrument()->id().toStdString() == instrumentTrackId.instrumentId : true;
        MixerChannelItem* item = buildInstrumentChannelItem(trackId, instrumentTrackId, isPrimary);
        int index = resolveInsertIndex(instrumentTrackId);

        addItem(item, index);
    });

    controller()->trackRemoved().onReceive(this, [this](const TrackId trackId) {
        removeItem(trackId);
    });

    loadItems();
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

void MixerPanelModel::loadItems()
{
    TRACEFUNC;

    beginResetModel();

    DEFER {
        endResetModel();
        emit rowCountChanged();
    };

    clear();

    if (m_currentTrackSequenceId == -1) {
        return;
    }

    const auto& instrumentTrackIdMap = controller()->instrumentTrackIdMap();

    auto addInstrumentTrack = [this, &instrumentTrackIdMap](const InstrumentTrackId& instrumentTrackId, bool isPrimary = true) {
        auto search = instrumentTrackIdMap.find(instrumentTrackId);
        if (search == instrumentTrackIdMap.cend()) {
            return;
        }

        m_mixerChannelList.push_back(buildInstrumentChannelItem(search->second, instrumentTrackId, isPrimary));
    };

    for (const Part* part : masterNotationParts()->partList()) {
        std::string primaryInstrId = part->instrument()->id().toStdString();

        for (const InstrumentTrackId& instrumentTrackId : part->instrumentTrackIdList()) {
            bool isPrimary = instrumentTrackId.instrumentId == primaryInstrId;
            addInstrumentTrack(instrumentTrackId, isPrimary);
        }
    }

    addInstrumentTrack(notationPlayback()->metronomeTrackId());

    for (auto it = instrumentTrackIdMap.cbegin(); it != instrumentTrackIdMap.cend(); ++it) {
        if (notationPlayback()->isChordSymbolsTrack(it->first)) {
            addInstrumentTrack(it->first);
        }
    }

    const TrackIdList& auxTrackIdList = controller()->auxTrackIdList();
    for (const TrackId& auxTrackId : auxTrackIdList) {
        m_mixerChannelList.push_back(buildAuxChannelItem(auxTrackId));
    }

    m_masterChannelItem = buildMasterChannelItem();
    m_mixerChannelList.append(m_masterChannelItem);

    updateItemsPanelsOrder();
    setupConnections();
}

void MixerPanelModel::addItem(MixerChannelItem* item, int index)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(item) {
        return;
    }

    beginInsertRows(QModelIndex(), index, index);
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
        m_mixerChannelList[i]->setPanelOrder(m_navigationPanelOrderStart + i + 1);
    }
}

void MixerPanelModel::clear()
{
    TRACEFUNC;

    m_masterChannelItem = nullptr;
    qDeleteAll(m_mixerChannelList);
    m_mixerChannelList.clear();
}

void MixerPanelModel::setupConnections()
{
    audioSettings()->soloMuteStateChanged().onReceive(
        this, [this](const engraving::InstrumentTrackId& changedInstrumentTrackId,
                     project::IProjectAudioSettings::SoloMuteState newSoloMuteState) {
        const IPlaybackController::InstrumentTrackIdMap& instrumentTrackIdMap = controller()->instrumentTrackIdMap();
        TrackId trackId = mu::value(instrumentTrackIdMap, changedInstrumentTrackId);

        if (MixerChannelItem* item = findChannelItem(trackId)) {
            item->loadSoloMuteState(std::move(newSoloMuteState));
        }
    });

    playback()->tracks()->inputParamsChanged().onReceive(
        this, [this](const TrackSequenceId sequenceId, const TrackId trackId, AudioInputParams params) {
        if (m_currentTrackSequenceId != sequenceId) {
            return;
        }

        if (MixerChannelItem* item = findChannelItem(trackId)) {
            item->loadInputParams(std::move(params));
        }
    });

    playback()->audioOutput()->outputParamsChanged().onReceive(
        this, [this](const TrackSequenceId sequenceId, const TrackId trackId, AudioOutputParams params) {
        if (m_currentTrackSequenceId != sequenceId) {
            return;
        }

        if (MixerChannelItem* item = findChannelItem(trackId)) {
            item->loadOutputParams(std::move(params));
        }
    });

    playback()->audioOutput()->masterOutputParamsChanged().onReceive(this,
                                                                     [this](AudioOutputParams params) {
        if (m_masterChannelItem) {
            m_masterChannelItem->loadOutputParams(std::move(params));
        }
    }, AsyncMode::AsyncSetRepeat);
}

int MixerPanelModel::resolveInsertIndex(const engraving::InstrumentTrackId& newInstrumentTrackId) const
{
    const InstrumentTrackId& metronomeTrackId = notationPlayback()->metronomeTrackId();
    if (newInstrumentTrackId == metronomeTrackId) {
        return m_mixerChannelList.size() - 1;
    }

    if (notationPlayback()->isChordSymbolsTrack(newInstrumentTrackId)) {
        return m_mixerChannelList.size() - 1;
    }

    int mixerChannelListIdx = 0;

    // Assumptions:
    // - the last channel is always the master channel
    // - the InstrumentTrackIds from the mixer channel items are always a correctly
    //   sorted subset of the InstrumentTrackIds from NotationParts
    for (const Part* part : masterNotationParts()->partList()) {
        for (const InstrumentTrackId& instrumentTrackId : part->instrumentTrackIdList()) {
            if (instrumentTrackId == newInstrumentTrackId) {
                return mixerChannelListIdx;
            }

            const MixerChannelItem* mixerChannelItem = m_mixerChannelList[mixerChannelListIdx];
            MixerChannelItem::Type itemType = mixerChannelItem->type();

            if (itemType == MixerChannelItem::Type::Master) {
                return mixerChannelListIdx;
            }

            const InstrumentTrackId& itemInstrumentTrackId = mixerChannelItem->instrumentTrackId();

            if (itemInstrumentTrackId == metronomeTrackId) {
                return mixerChannelListIdx;
            }

            if (notationPlayback()->isChordSymbolsTrack(itemInstrumentTrackId)) {
                return mixerChannelListIdx;
            }

            if (itemInstrumentTrackId == instrumentTrackId) {
                if (instrumentTrackId == newInstrumentTrackId) {
                    return INVALID_INDEX;
                }

                ++mixerChannelListIdx;
            }
        }
    }

    return INVALID_INDEX;
}

int MixerPanelModel::indexOf(const audio::TrackId trackId) const
{
    for (int i = 0; i < m_mixerChannelList.size(); ++i) {
        if (trackId == m_mixerChannelList[i]->trackId()) {
            return i;
        }
    }

    return INVALID_INDEX;
}

MixerChannelItem* MixerPanelModel::buildInstrumentChannelItem(const audio::TrackId trackId,
                                                              const engraving::InstrumentTrackId& instrumentTrackId,
                                                              bool isPrimary)
{
    MixerChannelItem::Type type = isPrimary ? MixerChannelItem::Type::PrimaryInstrument
                                  : MixerChannelItem::Type::SecondaryInstrument;

    MixerChannelItem* item = new MixerChannelItem(this, type, false /*outputOnly*/, trackId);
    item->setInstrumentTrackId(instrumentTrackId);
    item->setPanelSection(m_itemsNavigationSection);
    item->loadSoloMuteState(audioSettings()->soloMuteState(instrumentTrackId));

    playback()->tracks()->inputParams(m_currentTrackSequenceId, trackId)
    .onResolve(this, [this, trackId](AudioInputParams inParams) {
        if (MixerChannelItem* item = findChannelItem(trackId)) {
            item->loadInputParams(std::move(inParams));
        }
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to get track output parameters, error code: " << errCode
               << ", " << text;
    });

    playback()->tracks()->trackName(m_currentTrackSequenceId, trackId)
    .onResolve(this, [this, trackId](const TrackName& trackName) {
        if (MixerChannelItem* item = findChannelItem(trackId)) {
            item->setTitle(QString::fromStdString(trackName));
        }
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to get track name, error code: " << errCode
               << ", " << text;
    });

    playback()->audioOutput()->outputParams(m_currentTrackSequenceId, trackId)
    .onResolve(this, [this, trackId](AudioOutputParams outParams) {
        if (MixerChannelItem* item = findChannelItem(trackId)) {
            item->loadOutputParams(std::move(outParams));
        }
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to get track output parameters, error code: " << errCode
               << ", " << text;
    });

    playback()->audioOutput()->signalChanges(m_currentTrackSequenceId, trackId)
    .onResolve(this, [this, trackId](AudioSignalChanges signalChanges) {
        if (MixerChannelItem* item = findChannelItem(trackId)) {
            item->subscribeOnAudioSignalChanges(std::move(signalChanges));
        }
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to subscribe on audio signal changes from mixer channel, error code: " << errCode
               << ", " << text;
    });

    connect(item, &MixerChannelItem::inputParamsChanged, this, [this, trackId](const AudioInputParams& params) {
        playback()->tracks()->setInputParams(m_currentTrackSequenceId, trackId, params);
    });

    connect(item, &MixerChannelItem::outputParamsChanged, this, [this, trackId](const AudioOutputParams& params) {
        playback()->audioOutput()->setOutputParams(m_currentTrackSequenceId, trackId, params);
    });

    connect(item, &MixerChannelItem::soloMuteStateChanged, this,
            [this, instrumentTrackId](const project::IProjectAudioSettings::SoloMuteState& state) {
        audioSettings()->setSoloMuteState(instrumentTrackId, state);
    });

    return item;
}

MixerChannelItem* MixerPanelModel::buildAuxChannelItem(const TrackId trackId)
{
    MixerChannelItem* item = new MixerChannelItem(this, MixerChannelItem::Type::Aux, true /*outputOnly*/, trackId);
    item->setPanelSection(m_itemsNavigationSection);

    playback()->tracks()->trackName(m_currentTrackSequenceId, trackId)
    .onResolve(this, [this, trackId](const TrackName& trackName) {
        if (MixerChannelItem* item = findChannelItem(trackId)) {
            item->setTitle(QString::fromStdString(trackName));
        }
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to get track name, error code: " << errCode
               << ", " << text;
    });

    playback()->audioOutput()->outputParams(m_currentTrackSequenceId, trackId)
    .onResolve(this, [this, trackId](AudioOutputParams outParams) {
        if (MixerChannelItem* item = findChannelItem(trackId)) {
            item->loadOutputParams(std::move(outParams));
        }
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to get track output parameters, error code: " << errCode
               << ", " << text;
    });

    playback()->audioOutput()->signalChanges(m_currentTrackSequenceId, trackId)
    .onResolve(this, [this, trackId](AudioSignalChanges signalChanges) {
        if (MixerChannelItem* item = findChannelItem(trackId)) {
            item->subscribeOnAudioSignalChanges(std::move(signalChanges));
        }
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to subscribe on audio signal changes from mixer channel, error code: " << errCode
               << ", " << text;
    });

    connect(item, &MixerChannelItem::outputParamsChanged, this, [this, trackId](const AudioOutputParams& params) {
        playback()->audioOutput()->setOutputParams(m_currentTrackSequenceId, trackId, params);
    });

    return item;
}

MixerChannelItem* MixerPanelModel::buildMasterChannelItem()
{
    MixerChannelItem* item = new MixerChannelItem(this, MixerChannelItem::Type::Master, true /*outputOnly*/);
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

    return item;
}

MixerChannelItem* MixerPanelModel::findChannelItem(const audio::TrackId& trackId) const
{
    for (MixerChannelItem* item : m_mixerChannelList) {
        if (item->trackId() == trackId) {
            return item;
        }
    }

    return nullptr;
}

INotationProjectPtr MixerPanelModel::currentProject() const
{
    return context()->currentProject();
}

IProjectAudioSettingsPtr MixerPanelModel::audioSettings() const
{
    return currentProject() ? currentProject()->audioSettings() : nullptr;
}

INotationPlaybackPtr MixerPanelModel::notationPlayback() const
{
    return currentProject() ? currentProject()->masterNotation()->playback() : nullptr;
}

INotationPartsPtr MixerPanelModel::masterNotationParts() const
{
    return currentProject() ? currentProject()->masterNotation()->parts() : nullptr;
}
