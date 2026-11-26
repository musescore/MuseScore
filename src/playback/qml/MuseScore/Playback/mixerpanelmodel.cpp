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

#include "mixerpanelmodel.h"

#include <algorithm>

#include "async/notifylist.h"
#include "defer.h"
#include "log.h"
#include "translation.h"

#include "notation/imasternotation.h"
#include "notation/inotationautomation.h"
#include "notation/inotationparts.h"
#include "notation/inotationplayback.h"

#include "project/inotationproject.h"

using namespace muse;
using namespace mu::playback;
using namespace muse::audio;
using namespace mu::engraving;
using namespace mu::notation;
using namespace mu::project;

static constexpr int INVALID_INDEX = -1;

static constexpr TrackId VCA_TRACK_ID_BASE = -1000;

static const QString VCA_NEW_MENU_ID("vca-new");
static const QString VCA_ADD_MENU_ID("vca-add");
static const QString VCA_ADD_MENU_ID_PREFIX("vca-add-");
static const QString VCA_REMOVE_MENU_ID("vca-remove");
static const QString VCA_RENAME_MENU_ID("vca-rename");
static const QString VCA_DELETE_MENU_ID("vca-delete");

static ControlParams withEnvelopesOf(ControlParams params, const ControlParams& current)
{
    // Automated controls are disabled, so an envelope is never the param being edited
    if (current.volume.hasAutomation()) {
        params.volume = current.volume;
    }
    if (current.balance.hasAutomation()) {
        params.balance = current.balance;
    }
    return params;
}

MixerPanelModel::MixerPanelModel(QObject* parent)
    : QAbstractListModel(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void MixerPanelModel::componentComplete()
{
    init();
}

void MixerPanelModel::init()
{
    //! NOTE Must be set from Qml
    DO_ASSERT(m_navigationSection);

    controller()->playbackInitedChanged().onReceive(this, [this](bool) {
        reload();
    });

    controller()->trackAdded().onReceive(this, [this](const TrackId trackId) {
        onTrackAdded(trackId);
    });

    controller()->trackRemoved().onReceive(this, [this](const TrackId trackId) {
        removeItem(trackId);
    });

    reload();
}

void MixerPanelModel::reload()
{
    TRACEFUNC;

    reloadItems();
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

void MixerPanelModel::reloadItems()
{
    TRACEFUNC;

    beginResetModel();

    DEFER {
        endResetModel();
        emit rowCountChanged();
    };

    clear();

    if (!controller()->isPlaybackInited()) {
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

    async::NotifyList<const Part*> partList = masterNotationParts()->partList();
    for (const Part* part : partList) {
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

    if (audioSettings()) {
        const VcaGroupList& vcaGroups = audioSettings()->vcaGroups();
        for (size_t i = 0; i < vcaGroups.size(); ++i) {
            m_mixerChannelList.push_back(buildVcaChannelItem(i, vcaGroups[i]));
        }
    }

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
    m_vcaGroupIndexByItem.clear();
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
            item->loadSoloMuteState(newSoloMuteState);
        }
    });

    playback()->sourceParamsChanged().onReceive(this, [this](const TrackId trackId, const AudioSourceParams& params) {
        if (MixerChannelItem* item = findChannelItem(trackId)) {
            item->loadInputParams(params);
        }
    });

    playback()->fxChainParamsChanged().onReceive(this, [this](const TrackId trackId, const AudioFxChain& params) {
        if (MixerChannelItem* item = findChannelItem(trackId)) {
            AudioOutputParams outParams = audioSettings()->trackOutputParams(item->instrumentTrackId());
            outParams.fxChain = params;
            loadOutputParams(item, outParams);
        }
    });

    playback()->masterFxChainParamsChanged().onReceive(this, [this](const AudioFxChain& params) {
        if (m_masterChannelItem) {
            AudioOutputParams outParams = audioSettings()->masterAudioOutputParams();
            outParams.fxChain = params;
            loadOutputParams(m_masterChannelItem, outParams);
        }
    }, Asyncable::Mode::SetReplace);

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

    subscribeOnAutomationChanges();
}

void MixerPanelModel::subscribeOnAutomationChanges()
{
    if (!currentProject()) {
        return;
    }

    AutomationDataConstPtr automation = currentProject()->masterNotation()->automation()->automationData();
    if (!automation) {
        return;
    }

    automation->changed().onReceive(this, [this](const AutomationChanges& changes) {
        if (changes.isFullReset) {
            for (MixerChannelItem* item : m_mixerChannelList) {
                item->updateHasAutomationFlags();
            }
            return;
        }

        InstrumentTrackIdSet affectedTrackIds;
        bool masterAffected = false;

        for (const AutomationCurveKey& key : changes.affectedKeys) {
            if (key.type != AutomationType::Volume && key.type != AutomationType::Pan) {
                continue;
            }

            if (key.isGlobal()) {
                masterAffected = true;
            } else if (const std::optional<InstrumentTrackId> trackId = key.trackId()) {
                affectedTrackIds.insert(*trackId);
            }
        }

        for (MixerChannelItem* item : m_mixerChannelList) {
            const bool affected = item->type() == MixerChannelItem::Type::Master
                                  ? masterAffected
                                  : muse::contains(affectedTrackIds, item->instrumentTrackId());
            if (affected) {
                item->updateHasAutomationFlags();
            }
        }
    });
}

int MixerPanelModel::resolveInsertIndex(const engraving::InstrumentTrackId& newInstrumentTrackId) const
{
    const InstrumentTrackId& metronomeTrackId = notationPlayback()->metronomeTrackId();
    if (newInstrumentTrackId == metronomeTrackId) {
        // Keep the metronome to the left of any VCA strips
        for (int i = 0; i < m_mixerChannelList.size(); ++i) {
            if (m_mixerChannelList.at(i)->type() == MixerChannelItem::Type::Vca) {
                return i;
            }
        }

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

    async::NotifyList<const Part*> partList = masterNotationParts()->partList();
    for (const Part* part : partList) {
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

    playback()->params(trackId)
    .onResolve(this, [this, trackId, instrumentTrackId](const TrackParams& params) {
        if (MixerChannelItem* item = findChannelItem(trackId)) {
            item->loadInputParams(params.source);

            AudioOutputParams outParams = audioSettings()->trackOutputParams(instrumentTrackId);
            outParams.fxChain = params.fxChain;
            outParams.auxSends = params.auxSends;
            outParams.setControl(params.control);
            loadOutputParams(item, outParams);
        }
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to get track output parameters, error code: " << errCode
               << ", " << text;
    });

    playback()->trackName(trackId)
    .onResolve(this, [this, trackId](const RetVal<TrackName>& trackName) {
        if (trackName.ret) {
            if (MixerChannelItem* item = findChannelItem(trackId)) {
                item->setTitle(QString::fromStdString(trackName.val));
            }
        } else {
            LOGE() << "unable to get track name, error: " << trackName.ret.toString();
        }
    });

    playback()->signalChanges(trackId)
    .onResolve(this, [this, trackId](AudioSignalChanges signalChanges) {
        if (MixerChannelItem* item = findChannelItem(trackId)) {
            item->subscribeOnAudioSignalChanges(signalChanges);
        }
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to subscribe on audio signal changes from mixer channel, error code: " << errCode
               << ", " << text;
    });

    playback()->automatedControlParamsChanges(trackId)
    .onResolve(this, [this, trackId](AutomatedControlParamsChanges changes) {
        if (MixerChannelItem* item = findChannelItem(trackId)) {
            item->subscribeOnAutomatedControlParamsChanges(changes);
        }
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to subscribe on automated control params changes from mixer channel, error code: " << errCode
               << ", " << text;
    });

    connect(item, &MixerChannelItem::inputParamsChanged, this, [this, trackId](const AudioInputParams& params) {
        playback()->setSourceParams(trackId, params);
    });

    connect(item, &MixerChannelItem::controlParamsChanged, this,
            [this, trackId, instrumentTrackId](const AudioOutputParams& params) {
        const ControlParams current = controller()->currentTrackControlParams(instrumentTrackId);
        playback()->setControlParams(trackId, withEnvelopesOf(params.control(), current));
    });

    connect(item, &MixerChannelItem::fxChainParamsChanged, this, [this, trackId](const AudioOutputParams& params) {
        playback()->setFxChainParams(trackId, params.fxChain);
    });

    connect(item, &MixerChannelItem::auxSendsParamsChanged, this, [this, trackId](const AudioOutputParams& params) {
        playback()->setAuxSendsParams(trackId, params.auxSends);
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

    playback()->trackName(trackId)
    .onResolve(this, [this, trackId](const RetVal<TrackName>& trackName) {
        if (trackName.ret) {
            if (MixerChannelItem* item = findChannelItem(trackId)) {
                item->setTitle(QString::fromStdString(trackName.val));
            }
        } else {
            LOGE() << "unable to get track name, error: " << trackName.ret.toString();
        }
    });

    AudioOutputParams outParams = audioSettings()->auxOutputParams(index);
    loadOutputParams(item, outParams);

    playback()->signalChanges(trackId)
    .onResolve(this, [this, trackId](AudioSignalChanges signalChanges) {
        if (MixerChannelItem* item = findChannelItem(trackId)) {
            item->subscribeOnAudioSignalChanges(signalChanges);
        }
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to subscribe on audio signal changes from mixer channel, error code: " << errCode
               << ", " << text;
    });

    connect(item, &MixerChannelItem::controlParamsChanged, this, [this, trackId](const AudioOutputParams& params) {
        playback()->setControlParams(trackId, params.control());
    });
    connect(item, &MixerChannelItem::fxChainParamsChanged, this, [this, trackId](const AudioOutputParams& params) {
        playback()->setFxChainParams(trackId, params.fxChain);
    });
    connect(item, &MixerChannelItem::auxSendsParamsChanged, this, [this, trackId](const AudioOutputParams& params) {
        playback()->setAuxSendsParams(trackId, params.auxSends);
    });

    connect(item, &MixerChannelItem::soloMuteStateChanged, this,
            [this, index](const notation::INotationSoloMuteState::SoloMuteState& state) {
        audioSettings()->setAuxSoloMuteState(index, state);
    });

    return item;
}

MixerChannelItem* MixerPanelModel::buildMasterChannelItem()
{
    MixerChannelItem* item = new MixerChannelItem(this, MixerChannelItem::Type::Master, true /*outputOnly*/, MASTER_TRACK_ID);
    item->setPanelSection(m_navigationSection);
    item->setTitle(muse::qtrc("playback", "Master"));

    AudioOutputParams outParams = audioSettings()->masterAudioOutputParams();
    item->updateHasAutomationFlags();
    loadOutputParams(item, outParams);

    playback()->masterSignalChanges()
    .onResolve(this, [this, item](AudioSignalChanges signalChanges) {
        if (m_masterChannelItem && item == m_masterChannelItem) {
            item->subscribeOnAudioSignalChanges(signalChanges);
        }
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to subscribe on audio signal changes from master channel, error code: " << errCode
               << ", " << text;
    });

    playback()->automatedControlParamsChanges(MASTER_TRACK_ID)
    .onResolve(this, [this, item](AutomatedControlParamsChanges changes) {
        if (m_masterChannelItem && item == m_masterChannelItem) {
            item->subscribeOnAutomatedControlParamsChanges(changes);
        }
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to subscribe on automated control params changes from master channel, error code: " << errCode
               << ", " << text;
    });

    connect(item, &MixerChannelItem::controlParamsChanged, this, [this](const AudioOutputParams& params) {
        const ControlParams current = controller()->currentMasterControlParams();
        playback()->setMasterControlParams(withEnvelopesOf(params.control(), current));
    });

    connect(item, &MixerChannelItem::fxChainParamsChanged, this, [this](const AudioOutputParams& params) {
        playback()->setMasterFxChainParams(params.fxChain);
    });

    connect(item, &MixerChannelItem::auxSendsParamsChanged, this, [this](const AudioOutputParams& params) {
        playback()->setMasterAuxSendsParams(params.auxSends);
    });

    return item;
}

MixerChannelItem* MixerPanelModel::buildVcaChannelItem(size_t groupIndex, const VcaGroup& group)
{
    // VCA strips have no audio track, so give each a negative id that can't collide with a real one
    const TrackId trackId = VCA_TRACK_ID_BASE - static_cast<TrackId>(groupIndex);

    MixerChannelItem* item = new MixerChannelItem(this, MixerChannelItem::Type::Vca, true /*outputOnly*/, trackId);
    item->setPanelSection(m_navigationSection);
    item->setTitle(group.name.toQString());

    AudioOutputParams outParams;
    outParams.volume = group.volume;
    item->loadOutputParams(outParams);

    m_vcaGroupIndexByItem.insert(item, groupIndex);

    connect(item, &MixerChannelItem::controlParamsChanged, this, [this, item](const AudioOutputParams& params) {
        onVcaVolumeChangeRequested(item, params.volume.raw());
    });

    return item;
}

void MixerPanelModel::reloadVcaItems()
{
    TRACEFUNC;

    for (int i = m_mixerChannelList.size() - 1; i >= 0; --i) {
        MixerChannelItem* item = m_mixerChannelList.at(i);
        if (item->type() != MixerChannelItem::Type::Vca) {
            continue;
        }

        beginRemoveRows(QModelIndex(), i, i);
        m_mixerChannelList.removeAt(i);
        endRemoveRows();

        item->deleteLater();
    }

    m_vcaGroupIndexByItem.clear();

    if (!audioSettings()) {
        updateItemsPanelsOrder();
        emit rowCountChanged();
        return;
    }

    const VcaGroupList& groups = audioSettings()->vcaGroups();
    if (!groups.empty()) {
        const int insertIndex = vcaInsertIndex();
        const int count = static_cast<int>(groups.size());

        beginInsertRows(QModelIndex(), insertIndex, insertIndex + count - 1);
        for (int i = 0; i < count; ++i) {
            m_mixerChannelList.insert(insertIndex + i, buildVcaChannelItem(static_cast<size_t>(i), groups.at(i)));
        }
        endInsertRows();
    }

    updateItemsPanelsOrder();
    emit rowCountChanged();
}

int MixerPanelModel::vcaInsertIndex() const
{
    // VCA strips go to the immediate left of the aux channels and the master channel
    for (int i = 0; i < m_mixerChannelList.size(); ++i) {
        const MixerChannelItem::Type type = m_mixerChannelList.at(i)->type();
        if (type == MixerChannelItem::Type::Aux || type == MixerChannelItem::Type::Master) {
            return i;
        }
    }

    return m_mixerChannelList.size();
}

void MixerPanelModel::onVcaVolumeChangeRequested(MixerChannelItem* vcaItem, float requestedVolume)
{
    const std::optional<size_t> groupIndex = vcaGroupIndexOf(vcaItem);
    if (!groupIndex || !audioSettings()) {
        return;
    }

    VcaGroupList groups = audioSettings()->vcaGroups();
    IF_ASSERT_FAILED(*groupIndex < groups.size()) {
        return;
    }

    VcaGroup& group = groups.at(*groupIndex);

    const float minDb = VOLUME_DB_MIN.raw();
    const float maxDb = VOLUME_DB_MAX.raw();
    const float currentVcaVolume = group.volume.raw();

    float delta = std::clamp(requestedVolume - currentVcaVolume,
                             std::min(0.f, minDb - currentVcaVolume), std::max(0.f, maxDb - currentVcaVolume));

    // Stop the VCA fader as soon as any member fader reaches the end of its range,
    // so that the balance between the members is never lost.
    // Members with volume automation are controlled by their automation curve, so the VCA leaves them alone.
    std::vector<MixerChannelItem*> members;
    for (const InstrumentTrackId& instrumentTrackId : group.members) {
        MixerChannelItem* member = findInstrumentChannelItem(instrumentTrackId);
        if (!member || member->hasVolumeAutomation()) {
            continue;
        }

        const float memberVolume = member->outputParams().volume.raw();
        delta = std::clamp(delta, std::min(0.f, minDb - memberVolume), std::max(0.f, maxDb - memberVolume));
        members.push_back(member);
    }

    if (!muse::RealIsNull(delta)) {
        for (MixerChannelItem* member : members) {
            member->setVolumeLevel(member->outputParams().volume.raw() + delta);
        }
    }

    group.volume = currentVcaVolume + delta;
    audioSettings()->setVcaGroups(groups);

    // If the fader had to stop early, move the VCA strip back to where the group actually is
    if (!muse::RealIsEqual(group.volume.raw(), requestedVolume)) {
        AudioOutputParams outParams = vcaItem->outputParams();
        outParams.volume = group.volume;
        vcaItem->loadOutputParams(outParams);
    }
}

std::optional<size_t> MixerPanelModel::vcaGroupIndexOf(const InstrumentTrackId& instrumentTrackId) const
{
    if (!audioSettings()) {
        return std::nullopt;
    }

    const VcaGroupList& groups = audioSettings()->vcaGroups();
    for (size_t i = 0; i < groups.size(); ++i) {
        if (muse::contains(groups.at(i).members, instrumentTrackId)) {
            return i;
        }
    }

    return std::nullopt;
}

std::optional<size_t> MixerPanelModel::vcaGroupIndexOf(const MixerChannelItem* vcaItem) const
{
    auto it = m_vcaGroupIndexByItem.constFind(vcaItem);
    if (it == m_vcaGroupIndexByItem.cend()) {
        return std::nullopt;
    }

    return it.value();
}

void MixerPanelModel::addToVcaGroup(const InstrumentTrackId& instrumentTrackId, std::optional<size_t> groupIndex)
{
    VcaGroupList groups = audioSettings()->vcaGroups();

    // A channel belongs to at most one VCA group
    for (VcaGroup& group : groups) {
        muse::remove(group.members, instrumentTrackId);
    }

    if (groupIndex) {
        IF_ASSERT_FAILED(*groupIndex < groups.size()) {
            return;
        }

        groups.at(*groupIndex).members.push_back(instrumentTrackId);
    } else {
        VcaGroup group;
        group.name = newVcaGroupName(groups);
        group.members.push_back(instrumentTrackId);
        groups.push_back(std::move(group));
    }

    applyVcaGroupsChange(groups);
}

void MixerPanelModel::removeFromVcaGroup(const InstrumentTrackId& instrumentTrackId)
{
    VcaGroupList groups = audioSettings()->vcaGroups();
    for (VcaGroup& group : groups) {
        muse::remove(group.members, instrumentTrackId);
    }

    applyVcaGroupsChange(groups);
}

void MixerPanelModel::deleteVcaGroup(size_t groupIndex)
{
    VcaGroupList groups = audioSettings()->vcaGroups();
    IF_ASSERT_FAILED(groupIndex < groups.size()) {
        return;
    }

    // The members keep their current volumes, which already include the group's offset
    groups.erase(groups.begin() + static_cast<std::ptrdiff_t>(groupIndex));

    applyVcaGroupsChange(groups);
}

void MixerPanelModel::applyVcaGroupsChange(const VcaGroupList& groups)
{
    audioSettings()->setVcaGroups(groups);
    reloadVcaItems();
}

QString MixerPanelModel::newVcaGroupName(const VcaGroupList& groups) const
{
    for (int number = 1;; ++number) {
        //: The default name of a new VCA group in the mixer. %1 is a number. VCA stands for "voltage-controlled amplifier".
        const QString name = muse::qtrc("playback", "VCA %1").arg(number);

        const bool nameTaken = std::any_of(groups.cbegin(), groups.cend(), [&name](const VcaGroup& group) {
            return group.name.toQString() == name;
        });

        if (!nameTaken) {
            return name;
        }
    }
}

MixerChannelItem* MixerPanelModel::findInstrumentChannelItem(const InstrumentTrackId& instrumentTrackId) const
{
    for (MixerChannelItem* item : m_mixerChannelList) {
        const MixerChannelItem::Type type = item->type();
        if ((type == MixerChannelItem::Type::PrimaryInstrument || type == MixerChannelItem::Type::SecondaryInstrument)
            && item->instrumentTrackId() == instrumentTrackId) {
            return item;
        }
    }

    return nullptr;
}

QVariantList MixerPanelModel::channelContextMenuItems(int index) const
{
    QVariantList result;

    if (index < 0 || index >= m_mixerChannelList.size() || !audioSettings()) {
        return result;
    }

    auto buildMenuItem = [](const QString& id, const QString& title, const QVariantList& subitems = QVariantList()) {
        QVariantMap item;
        item["id"] = id;
        item["title"] = title;
        item["subitems"] = subitems;
        return item;
    };

    const MixerChannelItem* item = m_mixerChannelList.at(index);
    const VcaGroupList& groups = audioSettings()->vcaGroups();

    switch (item->type()) {
    case MixerChannelItem::Type::PrimaryInstrument:
    case MixerChannelItem::Type::SecondaryInstrument: {
        const std::optional<size_t> currentGroup = vcaGroupIndexOf(item->instrumentTrackId());

        result << buildMenuItem(VCA_NEW_MENU_ID, muse::qtrc("playback", "Add to new VCA group"));

        QVariantList groupItems;
        for (size_t i = 0; i < groups.size(); ++i) {
            if (currentGroup && *currentGroup == i) {
                continue;
            }

            groupItems << buildMenuItem(VCA_ADD_MENU_ID_PREFIX + QString::number(i), groups.at(i).name.toQString());
        }

        if (!groupItems.empty()) {
            result << buildMenuItem(VCA_ADD_MENU_ID, muse::qtrc("playback", "Add to VCA group"), groupItems);
        }

        if (currentGroup) {
            //: %1 is the name of a VCA group in the mixer
            result << buildMenuItem(VCA_REMOVE_MENU_ID, muse::qtrc("playback", "Remove from %1")
                                    .arg(groups.at(*currentGroup).name.toQString()));
        }
        break;
    }
    case MixerChannelItem::Type::Vca:
        result << buildMenuItem(VCA_RENAME_MENU_ID, muse::qtrc("playback", "Rename VCA group"));
        result << buildMenuItem(VCA_DELETE_MENU_ID, muse::qtrc("playback", "Delete VCA group"));
        break;
    default:
        break;
    }

    return result;
}

void MixerPanelModel::handleChannelContextMenuItem(int index, const QString& itemId)
{
    if (index < 0 || index >= m_mixerChannelList.size() || !audioSettings()) {
        return;
    }

    const MixerChannelItem* item = m_mixerChannelList.at(index);

    if (itemId == VCA_NEW_MENU_ID) {
        addToVcaGroup(item->instrumentTrackId(), std::nullopt);
    } else if (itemId.startsWith(VCA_ADD_MENU_ID_PREFIX)) {
        bool ok = false;
        const size_t groupIndex = itemId.mid(VCA_ADD_MENU_ID_PREFIX.size()).toULongLong(&ok);
        if (ok) {
            addToVcaGroup(item->instrumentTrackId(), groupIndex);
        }
    } else if (itemId == VCA_REMOVE_MENU_ID) {
        removeFromVcaGroup(item->instrumentTrackId());
    } else if (itemId == VCA_DELETE_MENU_ID) {
        if (const std::optional<size_t> groupIndex = vcaGroupIndexOf(item)) {
            deleteVcaGroup(*groupIndex);
        }
    }
}

void MixerPanelModel::renameVcaGroup(int index, const QString& name)
{
    const QString trimmedName = name.trimmed();
    if (index < 0 || index >= m_mixerChannelList.size() || trimmedName.isEmpty() || !audioSettings()) {
        return;
    }

    MixerChannelItem* item = m_mixerChannelList.at(index);
    const std::optional<size_t> groupIndex = vcaGroupIndexOf(item);
    if (!groupIndex) {
        return;
    }

    VcaGroupList groups = audioSettings()->vcaGroups();
    IF_ASSERT_FAILED(*groupIndex < groups.size()) {
        return;
    }

    groups.at(*groupIndex).name = trimmedName;
    audioSettings()->setVcaGroups(groups);

    item->setTitle(trimmedName);
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

void MixerPanelModel::loadOutputParams(MixerChannelItem* item, const AudioOutputParams& params)
{
    IF_ASSERT_FAILED(item) {
        return;
    }

    item->loadOutputParams(params);
    updateOutputResourceItemCount();
}

void MixerPanelModel::updateOutputResourceItemCount()
{
    size_t maxFxCount = 0;

    for (const MixerChannelItem* item : m_mixerChannelList) {
        if (item->type() == MixerChannelItem::Type::Vca) {
            continue;
        }

        const AudioFxChain& chain = item->outputParams().fxChain;
        if (chain.empty()) {
            continue;
        }

        AudioFxChainOrder order = std::prev(chain.end())->first;
        maxFxCount = std::max(maxFxCount, static_cast<size_t>(order) + 1);
    }

    for (MixerChannelItem* item : m_mixerChannelList) {
        // VCA strips carry no audio, so they have no effects slots
        if (item->type() != MixerChannelItem::Type::Vca) {
            item->setOutputResourceItemCount(maxFxCount + 1 /* + 1 blank slot */);
        }
    }
}

INotationProjectPtr MixerPanelModel::currentProject() const
{
    return context()->project();
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
