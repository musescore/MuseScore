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

#include "mixerpanelmodel.h"

#include "defer.h"

#include "log.h"
#include "translation.h"

using namespace muse;
using namespace mu::playback;
using namespace muse::audio;
using namespace mu::engraving;
using namespace mu::notation;
using namespace mu::project;

static constexpr int INVALID_INDEX = -1;

MixerPanelModel::MixerPanelModel(QObject* parent)
    : QAbstractListModel(parent)
{
    controller()->currentTrackSequenceIdChanged().onNotify(this, [this]() {
        load();
    });
}

void MixerPanelModel::load()
{
    TRACEFUNC;

    TrackSequenceId sequenceId = controller()->currentTrackSequenceId();

    if (m_currentTrackSequenceId == sequenceId) {
        return;
    }

    m_currentTrackSequenceId = sequenceId;

    controller()->trackAdded().onReceive(this, [this](const TrackId trackId) {
        onTrackAdded(trackId);
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
    static const QHash<int, QByteArray> roles = {
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
    for (auto it = instrumentTrackIdMap.cbegin(); it != instrumentTrackIdMap.cend(); ++it) {
        if (notationPlayback()->isChordSymbolsTrack(it->first)) {
            addInstrumentTrack(it->first);
        }
    }

    addInstrumentTrack(notationPlayback()->metronomeTrackId());

    const auto& auxTrackIdMap = controller()->auxTrackIdMap();
    for (auto it = auxTrackIdMap.cbegin(); it != auxTrackIdMap.cend(); ++it) {
        if (configuration()->isAuxChannelVisible(it->first)) {
            m_mixerChannelList.push_back(buildAuxChannelItem(it->first, it->second));
        }
    }

    m_masterChannelItem = buildMasterChannelItem();
    m_mixerChannelList.append(m_masterChannelItem);

    updateItemsPanelsOrder();
    setupConnections();
}

void MixerPanelModel::onTrackAdded(const TrackId& trackId)
{
    TRACEFUNC;

    const IPlaybackController::InstrumentTrackIdMap& instrumentTracks = controller()->instrumentTrackIdMap();
    auto instrumentIt = std::find_if(instrumentTracks.cbegin(), instrumentTracks.cend(), [trackId](const auto& pair) {
        return pair.second == trackId;
    });

    if (instrumentIt != instrumentTracks.end()) {
        const InstrumentTrackId& instrumentTrackId = instrumentIt->first;
        const Part* part = masterNotationParts()->part(instrumentTrackId.partId);
        bool isPrimary = part ? part->instrument()->id() == instrumentTrackId.instrumentId : true;
        MixerChannelItem* item = buildInstrumentChannelItem(trackId, instrumentTrackId, isPrimary);
        int index = resolveInsertIndex(instrumentTrackId);

        addItem(item, index);
        return;
    }

    const IPlaybackController::AuxTrackIdMap& auxTracks = controller()->auxTrackIdMap();
    auto auxIt = std::find_if(auxTracks.begin(), auxTracks.end(), [trackId](const auto& pair) {
        return pair.second == trackId;
    });

    if (auxIt != auxTracks.end()) {
        if (configuration()->isAuxChannelVisible(auxIt->first)) {
            addItem(buildAuxChannelItem(auxIt->first, trackId), m_mixerChannelList.size() - 1);
        }
    }
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

void MixerPanelModel::removeItem(const TrackId trackId)
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

    updateOutputResourceItemCount();

    emit rowCountChanged();
}

void MixerPanelModel::updateItemsPanelsOrder()
{
    TRACEFUNC;

    for (int i = 0; i < m_mixerChannelList.size(); i++) {
        m_mixerChannelList[i]->setPanelOrder(m_navigationOrderStart + i);
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
    audioSettings()->auxSoloMuteStateChanged().onReceive(
        this, [this](const aux_channel_idx_t index,
                     notation::INotationSoloMuteState::SoloMuteState newSoloMuteState) {
        const IPlaybackController::AuxTrackIdMap& auxTrackIdMap = controller()->auxTrackIdMap();
        TrackId trackId = muse::value(auxTrackIdMap, index);

        if (MixerChannelItem* item = findChannelItem(trackId)) {
            item->loadSoloMuteState(std::move(newSoloMuteState));
        }
    });

    playback()->inputParamsChanged().onReceive(
        this, [this](const TrackSequenceId sequenceId, const TrackId trackId, AudioInputParams params) {
        if (m_currentTrackSequenceId != sequenceId) {
            return;
        }

        if (MixerChannelItem* item = findChannelItem(trackId)) {
            item->loadInputParams(std::move(params));
        }
    });

    playback()->outputParamsChanged().onReceive(
        this, [this](const TrackSequenceId sequenceId, const TrackId trackId, AudioOutputParams params) {
        if (m_currentTrackSequenceId != sequenceId) {
            return;
        }

        if (MixerChannelItem* item = findChannelItem(trackId)) {
            loadOutputParams(item, std::move(params));
        }
    });

    playback()->masterOutputParamsChanged().onReceive(this,
                                                      [this](AudioOutputParams params) {
        if (m_masterChannelItem) {
            loadOutputParams(m_masterChannelItem, std::move(params));
        }
    }, AsyncMode::AsyncSetRepeat);

    controller()->auxChannelNameChanged().onReceive(this, [this](aux_channel_idx_t index, const std::string& name) {
        for (MixerChannelItem* item : m_mixerChannelList) {
            const QMap<aux_channel_idx_t, AuxSendItem*>& items = item->auxSendItems();
            auto it = items.find(index);

            if (it != items.end()) {
                it.value()->setTitle(QString::fromStdString(name));
            }
        }
    });

    configuration()->isAuxChannelVisibleChanged().onReceive(this, [this](aux_channel_idx_t index, bool visible) {
        const auto& auxMap = controller()->auxTrackIdMap();
        TrackId trackId = muse::value(auxMap, index);
        if (visible) {
            int visibleAuxesOnRight = 0;

            for (const auto& aux : auxMap) {
                if (configuration()->isAuxChannelVisible(aux.first) && (aux.first > index)) {
                    visibleAuxesOnRight++;
                }
            }
            addItem(buildAuxChannelItem(index, trackId), masterChannelIndex() - visibleAuxesOnRight);
        } else {
            removeItem(trackId);
        }
    });
}

int MixerPanelModel::resolveInsertIndex(const engraving::InstrumentTrackId& newInstrumentTrackId) const
{
    const InstrumentTrackId& metronomeTrackId = notationPlayback()->metronomeTrackId();
    if (newInstrumentTrackId == metronomeTrackId) {
        return masterChannelIndex();
    }

    // Assumptions:
    // - the last channel is always the master channel
    // - metronome channel is placed to the immediate left of the master (or auxes if visible)
    // - the InstrumentTrackIds from the mixer channel items are always a correctly
    //   sorted subset of the InstrumentTrackIds from NotationParts
    if (notationPlayback()->isChordSymbolsTrack(newInstrumentTrackId)) {
        int metronomeIdx = 0;
        for (const MixerChannelItem* channelItem : m_mixerChannelList) {
            const engraving::InstrumentTrackId& instrumentTrackId = channelItem->instrumentTrackId();
            if (instrumentTrackId.isValid() && instrumentTrackId != metronomeTrackId) {
                metronomeIdx++;
            }
        }
        return metronomeIdx;
    }

    int mixerChannelListIdx = 0;

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

int MixerPanelModel::indexOf(const TrackId trackId) const
{
    for (int i = 0; i < m_mixerChannelList.size(); ++i) {
        if (trackId == m_mixerChannelList[i]->trackId()) {
            return i;
        }
    }

    return INVALID_INDEX;
}

MixerChannelItem* MixerPanelModel::buildInstrumentChannelItem(const TrackId trackId,
                                                              const engraving::InstrumentTrackId& instrumentTrackId,
                                                              bool isPrimary)
{
    MixerChannelItem::Type type = isPrimary ? MixerChannelItem::Type::PrimaryInstrument
                                  : MixerChannelItem::Type::SecondaryInstrument;

    const InstrumentTrackId& metronomeTrackId = notationPlayback()->metronomeTrackId();
    if (instrumentTrackId == metronomeTrackId) {
        type = MixerChannelItem::Type::Metronome;
    }

    MixerChannelItem* item = new MixerChannelItem(this, type, false /*outputOnly*/, trackId);
    item->setInstrumentTrackId(instrumentTrackId);
    item->setPanelSection(m_navigationSection);
    item->loadSoloMuteState(controller()->trackSoloMuteState(instrumentTrackId));

    playback()->inputParams(m_currentTrackSequenceId, trackId)
    .onResolve(this, [this, trackId](AudioInputParams inParams) {
        if (MixerChannelItem* item = findChannelItem(trackId)) {
            item->loadInputParams(std::move(inParams));
        }
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to get track output parameters, error code: " << errCode
               << ", " << text;
    });

    playback()->trackName(m_currentTrackSequenceId, trackId)
    .onResolve(this, [this, trackId](const RetVal<TrackName>& trackName) {
        if (trackName.ret) {
            if (MixerChannelItem* item = findChannelItem(trackId)) {
                item->setTitle(QString::fromStdString(trackName.val));
            }
        } else {
            LOGE() << "unable to get track name, error: " << trackName.ret.toString();
        }
    });

    playback()->outputParams(m_currentTrackSequenceId, trackId)
    .onResolve(this, [this, trackId](AudioOutputParams outParams) {
        if (MixerChannelItem* item = findChannelItem(trackId)) {
            loadOutputParams(item, std::move(outParams));
        }
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to get track output parameters, error code: " << errCode
               << ", " << text;
    });

    playback()->signalChanges(m_currentTrackSequenceId, trackId)
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
        playback()->setInputParams(m_currentTrackSequenceId, trackId, params);
    });

    connect(item, &MixerChannelItem::outputParamsChanged, this, [this, trackId](const AudioOutputParams& params) {
        playback()->setOutputParams(m_currentTrackSequenceId, trackId, params);
    });

    connect(item, &MixerChannelItem::auxSendItemListChanged, this, [this, item]() {
        const QMap<aux_channel_idx_t, AuxSendItem*>& auxSendItems = item->auxSendItems();
        for (auto it = auxSendItems.begin(); it != auxSendItems.end(); ++it) {
            it.value()->setTitle(QString::fromStdString(controller()->auxChannelName(it.key())));
        }
    });

    connect(item, &MixerChannelItem::soloMuteStateChanged, this,
            [this, instrumentTrackId](const notation::INotationSoloMuteState::SoloMuteState& state) {
        controller()->setTrackSoloMuteState(instrumentTrackId, state);
    });

    return item;
}

MixerChannelItem* MixerPanelModel::buildAuxChannelItem(aux_channel_idx_t index, const TrackId trackId)
{
    MixerChannelItem* item = new MixerChannelItem(this, MixerChannelItem::Type::Aux, true /*outputOnly*/, trackId);
    item->setPanelSection(m_navigationSection);
    item->loadSoloMuteState(audioSettings()->auxSoloMuteState(index));

    playback()->trackName(m_currentTrackSequenceId, trackId)
    .onResolve(this, [this, trackId](const RetVal<TrackName>& trackName) {
        if (trackName.ret) {
            if (MixerChannelItem* item = findChannelItem(trackId)) {
                item->setTitle(QString::fromStdString(trackName.val));
            }
        } else {
            LOGE() << "unable to get track name, error: " << trackName.ret.toString();
        }
    });

    playback()->outputParams(m_currentTrackSequenceId, trackId)
    .onResolve(this, [this, trackId](AudioOutputParams outParams) {
        if (MixerChannelItem* item = findChannelItem(trackId)) {
            loadOutputParams(item, std::move(outParams));
        }
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to get track output parameters, error code: " << errCode
               << ", " << text;
    });

    playback()->signalChanges(m_currentTrackSequenceId, trackId)
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
        playback()->setOutputParams(m_currentTrackSequenceId, trackId, params);
    });

    connect(item, &MixerChannelItem::soloMuteStateChanged, this,
            [this, index](const notation::INotationSoloMuteState::SoloMuteState& state) {
        audioSettings()->setAuxSoloMuteState(index, state);
    });

    return item;
}

MixerChannelItem* MixerPanelModel::buildMasterChannelItem()
{
    MixerChannelItem* item = new MixerChannelItem(this, MixerChannelItem::Type::Master, true /*outputOnly*/);
    item->setPanelSection(m_navigationSection);
    item->setTitle(muse::qtrc("playback", "Master"));

    playback()->masterOutputParams()
    .onResolve(this, [this, item](AudioOutputParams outParams) {
        if (m_masterChannelItem && item == m_masterChannelItem) {
            loadOutputParams(item, std::move(outParams));
        }
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to get master output parameters, error code: " << errCode
               << ", " << text;
    });

    playback()->masterSignalChanges()
    .onResolve(this, [this, item](AudioSignalChanges signalChanges) {
        if (m_masterChannelItem && item == m_masterChannelItem) {
            item->subscribeOnAudioSignalChanges(std::move(signalChanges));
        }
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to subscribe on audio signal changes from master channel, error code: " << errCode
               << ", " << text;
    });

    connect(item, &MixerChannelItem::outputParamsChanged, this, [this](const AudioOutputParams& params) {
        playback()->setMasterOutputParams(params);
    });

    return item;
}

int MixerPanelModel::masterChannelIndex() const
{
    return m_mixerChannelList.size() - 1;
}

MixerChannelItem* MixerPanelModel::findChannelItem(const TrackId& trackId) const
{
    for (MixerChannelItem* item : m_mixerChannelList) {
        if (item->trackId() == trackId) {
            return item;
        }
    }

    return nullptr;
}

void MixerPanelModel::loadOutputParams(MixerChannelItem* item, AudioOutputParams&& params)
{
    IF_ASSERT_FAILED(item) {
        return;
    }

    item->loadOutputParams(std::move(params));
    updateOutputResourceItemCount();
}

void MixerPanelModel::updateOutputResourceItemCount()
{
    size_t maxFxCount = 0;

    for (const MixerChannelItem* item : m_mixerChannelList) {
        const AudioFxChain& chain = item->outputParams().fxChain;
        if (chain.empty()) {
            continue;
        }

        AudioFxChainOrder order = std::prev(chain.end())->first;
        maxFxCount = std::max(maxFxCount, static_cast<size_t>(order) + 1);
    }

    for (MixerChannelItem* item : m_mixerChannelList) {
        item->setOutputResourceItemCount(maxFxCount + 1 /* + 1 blank slot */);
    }
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

muse::ui::NavigationSection* MixerPanelModel::navigationSection() const
{
    return m_navigationSection;
}

void MixerPanelModel::setNavigationSection(muse::ui::NavigationSection* navigationSection)
{
    if (m_navigationSection == navigationSection) {
        return;
    }

    m_navigationSection = navigationSection;
    emit navigationSectionChanged();
}

int MixerPanelModel::navigationOrderStart() const
{
    return m_navigationOrderStart;
}

void MixerPanelModel::setNavigationOrderStart(int navigationOrderStart)
{
    if (m_navigationOrderStart == navigationOrderStart) {
        return;
    }

    m_navigationOrderStart = navigationOrderStart;
    emit navigationOrderStartChanged();

    updateItemsPanelsOrder();
}
