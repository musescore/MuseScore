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

#include "mixerchannelitem.h"

#include "translation.h"

#include "log.h"

using namespace mu::playback;
using namespace mu::audio;

static constexpr volume_dbfs_t MAX_DISPLAYED_DBFS = 0.f; // 100%
static constexpr volume_dbfs_t MIN_DISPLAYED_DBFS = -60.f; // 0%

static constexpr float BALANCE_SCALING_FACTOR = 100.f;

static constexpr int OUTPUT_RESOURCE_COUNT_LIMIT = 4;

static const std::string VSTFX_EDITOR_URI("musescore://vstfx/editor?sync=false&modal=false&floating=true");
static const std::string VSTI_EDITOR_URI("musescore://vsti/editor?sync=false&modal=false&floating=true");

static const std::string TRACK_ID_KEY("trackId");
static const std::string RESOURCE_ID_KEY("resourceId");
static const std::string CHAIN_ORDER_KEY("chainOrder");

MixerChannelItem::MixerChannelItem(QObject* parent, audio::TrackId trackId, bool isPrimary)
    : QObject(parent),
    m_trackId(trackId),
    m_isPrimary(isPrimary),
    m_leftChannelPressure(MIN_DISPLAYED_DBFS),
    m_rightChannelPressure(MIN_DISPLAYED_DBFS)
{
    m_panel = new ui::NavigationPanel(this);
    m_panel->setDirection(ui::NavigationPanel::Vertical);
    m_panel->setName("MixerChannelPanel " + QString::number(m_trackId));
    m_panel->accessible()->setName(qtrc("playback", "Mixer channel panel %1").arg(m_trackId));
    m_panel->componentComplete();

    connect(this, &MixerChannelItem::mutedChanged, this, [this]() {
        if (muted()) {
            resetAudioChannelsVolumePressure();
        }
    });
}

MixerChannelItem::~MixerChannelItem()
{
    m_audioSignalChanges.resetOnReceive(this);
}

TrackId MixerChannelItem::trackId() const
{
    return m_trackId;
}

bool MixerChannelItem::isMasterChannel() const
{
    return false;
}

QString MixerChannelItem::title() const
{
    return m_title;
}

bool MixerChannelItem::isPrimaryChannel() const
{
    return m_isPrimary;
}

float MixerChannelItem::leftChannelPressure() const
{
    return m_leftChannelPressure;
}

float MixerChannelItem::rightChannelPressure() const
{
    return m_rightChannelPressure;
}

float MixerChannelItem::volumeLevel() const
{
    return m_outParams.volume;
}

int MixerChannelItem::balance() const
{
    return m_outParams.balance * BALANCE_SCALING_FACTOR;
}

bool MixerChannelItem::muted() const
{
    return m_outParams.muted;
}

bool MixerChannelItem::mutedManually() const
{
    return m_soloMuteState.mute;
}

bool MixerChannelItem::solo() const
{
    return m_soloMuteState.solo;
}

mu::ui::NavigationPanel* MixerChannelItem::panel() const
{
    return m_panel;
}

void MixerChannelItem::setPanelOrder(int panelOrder)
{
    m_panel->setOrder(panelOrder);
}

void MixerChannelItem::setPanelSection(mu::ui::INavigationSection* section)
{
    m_panel->setSection(section);
}

void MixerChannelItem::loadOutputParams(AudioOutputParams&& newParams)
{
    if (m_outParams.volume != newParams.volume) {
        m_outParams.volume = newParams.volume;
        emit volumeLevelChanged(newParams.volume);
    }

    if (m_outParams.balance != newParams.balance) {
        m_outParams.balance = newParams.balance;
        emit balanceChanged(newParams.balance);
    }

    if (m_outParams.muted != newParams.muted) {
        m_outParams.muted = newParams.muted;
        emit mutedChanged();
    }

    m_outParams.fxChain = newParams.fxChain;

    QMap<AudioFxChainOrder, OutputResourceItem*> newItems = m_outputResourceItems;

    for (AudioFxChainOrder chainOrder : m_outputResourceItems.keys()) {
        OutputResourceItem* item = m_outputResourceItems.value(chainOrder);

        if (item->isBlank()) {
            continue;
        }

        if (newParams.fxChain.find(chainOrder) == newParams.fxChain.cend()) {
            item->disconnect();
            item->deleteLater();
            newItems.remove(chainOrder);
        }
    }

    for (const auto& pair : newParams.fxChain) {
        OutputResourceItem* item = newItems.value(pair.first, nullptr);
        if (!item) {
            newItems.insert(pair.first, buildOutputResourceItem(pair.second));
            continue;
        }

        if (item->params() != pair.second) {
            item->blockSignals(true);
            item->setParams(pair.second);
            item->blockSignals(false);
        }
    }

    if (m_outputResourceItems != newItems) {
        m_outputResourceItems = std::move(newItems);
        emit outputResourceItemListChanged();
    }

    ensureBlankOutputResourceSlot();
}

void MixerChannelItem::loadSoloMuteState(project::IProjectAudioSettings::SoloMuteState&& newState)
{
    if (m_soloMuteState.mute != newState.mute) {
        m_soloMuteState.mute = newState.mute;
        emit mutedChanged();
    }

    if (m_soloMuteState.solo != newState.solo) {
        m_soloMuteState.solo = newState.solo;
        emit soloChanged();
    }
}

void MixerChannelItem::subscribeOnAudioSignalChanges(AudioSignalChanges&& audioSignalChanges)
{
    m_audioSignalChanges = audioSignalChanges;

    m_audioSignalChanges.onReceive(this, [this](const audioch_t audioChNum, const AudioSignalVal& newValue) {
        //!Note There should be no signal changes when the mixer channel is muted.
        //!     But some audio signal changes still might be "on the way" from the times when the mixer channel wasn't muted
        //!     So that we have to just ignore them
        if (muted()) {
            return;
        }

        if (newValue.pressure < MIN_DISPLAYED_DBFS) {
            setAudioChannelVolumePressure(audioChNum, MIN_DISPLAYED_DBFS);
        } else if (newValue.pressure > MAX_DISPLAYED_DBFS) {
            setAudioChannelVolumePressure(audioChNum, MAX_DISPLAYED_DBFS);
        } else {
            setAudioChannelVolumePressure(audioChNum, newValue.pressure);
        }
    });
}

void MixerChannelItem::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    emit titleChanged(m_title);
}

void MixerChannelItem::setLeftChannelPressure(float leftChannelPressure)
{
    if (qFuzzyCompare(m_leftChannelPressure, leftChannelPressure)) {
        return;
    }

    m_leftChannelPressure = leftChannelPressure;
    emit leftChannelPressureChanged(m_leftChannelPressure);
}

void MixerChannelItem::setRightChannelPressure(float rightChannelPressure)
{
    if (qFuzzyCompare(m_rightChannelPressure, rightChannelPressure)) {
        return;
    }

    m_rightChannelPressure = rightChannelPressure;
    emit rightChannelPressureChanged(m_rightChannelPressure);
}

void MixerChannelItem::setVolumeLevel(float volumeLevel)
{
    if (qFuzzyCompare(m_outParams.volume, volumeLevel)) {
        return;
    }

    m_outParams.volume = volumeLevel;
    emit volumeLevelChanged(m_outParams.volume);
    emit outputParamsChanged(m_outParams);
}

void MixerChannelItem::setBalance(int balance)
{
    if (m_outParams.balance * BALANCE_SCALING_FACTOR == balance) {
        return;
    }

    m_outParams.balance = balance / BALANCE_SCALING_FACTOR;
    emit balanceChanged(balance);
    emit outputParamsChanged(m_outParams);
}

void MixerChannelItem::setMutedManually(bool isMuted)
{
    if (m_soloMuteState.mute == isMuted) {
        return;
    }

    m_soloMuteState.mute = isMuted;
    emit soloMuteStateChanged(m_soloMuteState);
    emit mutedChanged();
}

void MixerChannelItem::setSolo(bool solo)
{
    if (m_soloMuteState.solo == solo) {
        return;
    }

    m_soloMuteState.solo = solo;
    emit soloMuteStateChanged(m_soloMuteState);
    emit soloChanged();
}

void MixerChannelItem::setAudioChannelVolumePressure(const audio::audioch_t chNum, const float newValue)
{
    if (chNum == 0) {
        setLeftChannelPressure(newValue);
    } else {
        setRightChannelPressure(newValue);
    }
}

void MixerChannelItem::resetAudioChannelsVolumePressure()
{
    setLeftChannelPressure(MIN_DISPLAYED_DBFS);
    setRightChannelPressure(MIN_DISPLAYED_DBFS);
}

OutputResourceItem* MixerChannelItem::buildOutputResourceItem(const audio::AudioFxParams& fxParams)
{
    OutputResourceItem* newItem = new OutputResourceItem(this, fxParams);

    connect(newItem, &OutputResourceItem::fxParamsChanged, this, [this]() {
        m_outParams.fxChain.clear();

        for (const OutputResourceItem* item : m_outputResourceItems) {
            m_outParams.fxChain.insert({ item->params().chainOrder, item->params() });
        }

        emit outputParamsChanged(m_outParams);
    });

    connect(newItem, &OutputResourceItem::nativeEditorViewLaunchRequested, this, [this, newItem]() {
        if (newItem->params().type() != AudioFxType::VstFx) {
            return;
        }

        UriQuery uri(VSTFX_EDITOR_URI);

        if (!isMasterChannel()) {
            uri.addParam(TRACK_ID_KEY, Val(m_trackId));
        }

        uri.addParam(RESOURCE_ID_KEY, Val(newItem->params().resourceMeta.id));
        uri.addParam(CHAIN_ORDER_KEY, Val(newItem->params().chainOrder));

        openEditor(newItem, uri);
    });

    connect(newItem, &OutputResourceItem::nativeEditorViewCloseRequested, this, [this, newItem]() {
        closeEditor(newItem);
    });

    return newItem;
}

void MixerChannelItem::openEditor(AbstractAudioResourceItem* item, const UriQuery& editorUri)
{
    if (item->editorUri() != editorUri) {
        interactive()->close(item->editorUri());
        item->setEditorUri(editorUri);
    }

    if (interactive()->isOpened(editorUri).val) {
        interactive()->raise(editorUri);
    } else {
        interactive()->open(editorUri);
    }
}

void MixerChannelItem::closeEditor(AbstractAudioResourceItem* item)
{
    interactive()->close(item->editorUri());
    item->setEditorUri(UriQuery());
}

void MixerChannelItem::ensureBlankOutputResourceSlot()
{
    removeRedundantEmptySlots();

    if (m_outputResourceItems.count() >= OUTPUT_RESOURCE_COUNT_LIMIT) {
        return;
    }

    if (!m_outputResourceItems.empty() && m_outputResourceItems.last()->isBlank()) {
        return;
    }

    AudioFxParams params;
    params.chainOrder = resolveNewBlankOutputResourceItemOrder();
    m_outputResourceItems.insert(params.chainOrder, buildOutputResourceItem(std::move(params)));

    emit outputResourceItemListChanged();
}

void MixerChannelItem::removeRedundantEmptySlots()
{
    for (AudioFxChainOrder order : emptySlotsToRemove()) {
        OutputResourceItem* item = m_outputResourceItems.take(order);
        closeEditor(item);
        item->disconnect();
        item->deleteLater();
    }
}

QList<AudioFxChainOrder> MixerChannelItem::emptySlotsToRemove() const
{
    QList<AudioFxChainOrder> result;

    if (m_outputResourceItems.empty()) {
        return result;
    }

    AudioFxChainOrder lastChainOrder = m_outputResourceItems.lastKey();

    for (auto it = m_outputResourceItems.cbegin(); it != m_outputResourceItems.cend(); ++it) {
        if (!it.value()->isBlank()) {
            result.clear();
            continue;
        }

        if (result.empty() && it.key() == lastChainOrder) {
            continue;
        }

        result << it.key();
    }

    return result;
}

AudioFxChainOrder MixerChannelItem::resolveNewBlankOutputResourceItemOrder() const
{
    if (m_outputResourceItems.empty()) {
        return 0;
    }

    AudioFxChainOrder lastChainOrder = m_outputResourceItems.lastKey();
    if (lastChainOrder < OUTPUT_RESOURCE_COUNT_LIMIT - 1) {
        return lastChainOrder + 1;
    }

    for (AudioFxChainOrder order = 0; order < OUTPUT_RESOURCE_COUNT_LIMIT; ++order) {
        if (!m_outputResourceItems.contains(order)) {
            return order;
        }
    }

    return OUTPUT_RESOURCE_COUNT_LIMIT - 1;
}

bool MixerChannelItem::outputOnly() const
{
    return isMasterChannel();
}

QList<OutputResourceItem*> MixerChannelItem::outputResourceItemList() const
{
    return m_outputResourceItems.values();
}

TrackMixerChannelItem::TrackMixerChannelItem(QObject* parent, audio::TrackId trackId, const engraving::InstrumentTrackId& instrumentTrackId,
                                             bool isPrimary)
    : MixerChannelItem(parent, trackId, isPrimary), m_instrumentTrackId(instrumentTrackId)
{
    m_inputResourceItem = buildInputResourceItem();
}

const mu::engraving::InstrumentTrackId& TrackMixerChannelItem::instrumentTrackId() const
{
    return m_instrumentTrackId;
}

void TrackMixerChannelItem::loadInputParams(AudioInputParams&& newParams)
{
    if (m_inputParams == newParams) {
        return;
    }

    m_inputParams = newParams;
    m_inputResourceItem->setParams(newParams);
}

InputResourceItem* TrackMixerChannelItem::buildInputResourceItem()
{
    InputResourceItem* newItem = new InputResourceItem(this);

    connect(newItem, &InputResourceItem::inputParamsChanged, this, [this, newItem]() {
        m_inputParams = newItem->params();

        emit inputParamsChanged(m_inputParams);
    });

    connect(newItem, &InputResourceItem::isBlankChanged, this, &TrackMixerChannelItem::inputResourceItemChanged);

    connect(newItem, &InputResourceItem::nativeEditorViewLaunchRequested, this, [this, newItem]() {
        if (newItem->params().type() != AudioSourceType::Vsti) {
            return;
        }

        UriQuery uri(VSTI_EDITOR_URI);
        uri.addParam(TRACK_ID_KEY, Val(m_trackId));
        uri.addParam(RESOURCE_ID_KEY, Val(newItem->params().resourceMeta.id));

        openEditor(newItem, uri);
    });

    connect(newItem, &InputResourceItem::nativeEditorViewCloseRequested, this, [this, newItem]() {
        closeEditor(newItem);
    });

    return newItem;
}

InputResourceItem* TrackMixerChannelItem::inputResourceItem() const
{
    return m_inputResourceItem;
}

MasterMixerChannelItem::MasterMixerChannelItem(QObject* parent)
    : MixerChannelItem(parent)
{
}

bool MasterMixerChannelItem::isMasterChannel() const
{
    return true;
}
