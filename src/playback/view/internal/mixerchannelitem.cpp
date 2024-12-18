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

#include "mixerchannelitem.h"

#include "defer.h"
#include "translation.h"
#include "log.h"

using namespace mu::playback;
using namespace muse;
using namespace muse::audio;

static constexpr volume_dbfs_t MAX_DISPLAYED_DBFS = volume_dbfs_t::make(0.f);   // 100%
static constexpr volume_dbfs_t MIN_DISPLAYED_DBFS = volume_dbfs_t::make(-60.f); // 0%

static constexpr float BALANCE_SCALING_FACTOR = 100.f;

static constexpr int OUTPUT_RESOURCE_COUNT_LIMIT = 4;

static const std::string VSTFX_EDITOR_URI("muse://vstfx/editor?sync=false&modal=false&floating=true");
static const std::string VSTI_EDITOR_URI("muse://vsti/editor?sync=false&modal=false&floating=true");

static const std::string TRACK_ID_KEY("trackId");
static const std::string RESOURCE_ID_KEY("resourceId");
static const std::string CHAIN_ORDER_KEY("chainOrder");

MixerChannelItem::MixerChannelItem(QObject* parent, Type type, bool outputOnly, audio::TrackId trackId)
    : QObject(parent),
    m_type(type),
    m_trackId(trackId),
    m_outputOnly(outputOnly),
    m_leftChannelPressure(MIN_DISPLAYED_DBFS),
    m_rightChannelPressure(MIN_DISPLAYED_DBFS)
{
    if (!m_outputOnly) {
        m_inputResourceItem = buildInputResourceItem();
    }

    m_panel = new ui::NavigationPanel(this);
    m_panel->setDirection(ui::NavigationPanel::Vertical);
    m_panel->setName("MixerChannelPanel " + QString::number(m_trackId));
    m_panel->accessible()->setName(muse::qtrc("playback", "Mixer channel panel %1").arg(m_trackId));
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

MixerChannelItem::Type MixerChannelItem::type() const
{
    return m_type;
}

TrackId MixerChannelItem::trackId() const
{
    return m_trackId;
}

const mu::engraving::InstrumentTrackId& MixerChannelItem::instrumentTrackId() const
{
    return m_instrumentTrackId;
}

void MixerChannelItem::setInstrumentTrackId(const mu::engraving::InstrumentTrackId& instrumentTrackId)
{
    m_instrumentTrackId = instrumentTrackId;
}

QString MixerChannelItem::title() const
{
    return m_title;
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

bool MixerChannelItem::solo() const
{
    return m_outParams.solo;
}

bool MixerChannelItem::muted() const
{
    return m_outParams.muted;
}

bool MixerChannelItem::forceMute() const
{
    return m_outParams.forceMute;
}

muse::ui::NavigationPanel* MixerChannelItem::panel() const
{
    return m_panel;
}

void MixerChannelItem::setPanelOrder(int panelOrder)
{
    m_panel->setOrder(panelOrder);
}

void MixerChannelItem::setPanelSection(muse::ui::INavigationSection* section)
{
    m_panel->setSection(section);
}

void MixerChannelItem::setOutputResourceItemCount(size_t count)
{
    IF_ASSERT_FAILED(count >= m_outParams.fxChain.size()) {
        return;
    }

    count = std::min(count, static_cast<size_t>(OUTPUT_RESOURCE_COUNT_LIMIT));
    size_t itemsSize = static_cast<size_t>(m_outputResourceItems.size());

    if (itemsSize == count) {
        return;
    }

    if (itemsSize < count) {
        addBlankSlots(count - itemsSize);
    } else if (itemsSize > count) {
        removeBlankSlotsFromEnd(itemsSize - count);
    }
}

void MixerChannelItem::addBlankSlots(size_t count)
{
    TRACEFUNC;

    if (count == 0) {
        return;
    }

    for (size_t i = 0; i < count; ++i) {
        AudioFxParams params;
        params.chainOrder = resolveNewBlankOutputResourceItemOrder();
        m_outputResourceItems.insert(params.chainOrder, buildOutputResourceItem(std::move(params)));
    }

    emit outputResourceItemListChanged();
}

void MixerChannelItem::removeBlankSlotsFromEnd(size_t count)
{
    TRACEFUNC;

    bool itemsRemoved = false;
    DEFER {
        if (itemsRemoved) {
            emit outputResourceItemListChanged();
        }
    };

    for (size_t i = 0; i < count; ++i) {
        if (m_outputResourceItems.empty()) {
            return;
        }

        auto lastItemIt = std::prev(m_outputResourceItems.end());
        OutputResourceItem* item = lastItemIt.value();

        if (!item->isBlank()) {
            return;
        }

        m_outputResourceItems.erase(lastItemIt);
        closeEditor(item);
        item->disconnect();
        item->deleteLater();
        itemsRemoved = true;
    }
}

void MixerChannelItem::loadInputParams(const AudioInputParams& newParams)
{
    if (m_outputOnly) {
        return;
    }

    if (m_inputParams == newParams) {
        return;
    }

    m_inputParams = newParams;
    m_inputResourceItem->setParams(newParams);
}

void MixerChannelItem::loadOutputParams(const AudioOutputParams& newParams)
{
    if (!muse::RealIsEqual(m_outParams.volume, newParams.volume)) {
        m_outParams.volume = newParams.volume;
        emit volumeLevelChanged(newParams.volume);
    }

    if (!muse::RealIsEqual(m_outParams.balance, newParams.balance)) {
        m_outParams.balance = newParams.balance;
        emit balanceChanged(newParams.balance);
    }

    if (m_outParams.solo != newParams.solo) {
        m_outParams.solo = newParams.solo;
        emit soloChanged();
    }

    if (m_outParams.muted != newParams.muted) {
        m_outParams.muted = newParams.muted;
        emit mutedChanged();
    }

    if (m_outParams.forceMute != newParams.forceMute) {
        m_outParams.forceMute = newParams.forceMute;
        emit forceMuteChanged();
    }

    loadOutputResourceItems(newParams.fxChain);
    loadAuxSendItems(newParams.auxSends);
}

void MixerChannelItem::loadOutputResourceItems(const AudioFxChain& fxChain)
{
    m_outParams.fxChain = fxChain;

    m_outputResourceItemsLoading = true;
    DEFER {
        m_outputResourceItemsLoading = false;
    };

    QMap<AudioFxChainOrder, OutputResourceItem*> newItems = m_outputResourceItems;

    for (AudioFxChainOrder chainOrder : m_outputResourceItems.keys()) {
        OutputResourceItem* item = m_outputResourceItems.value(chainOrder);

        if (item->isBlank()) {
            continue;
        }

        if (fxChain.find(chainOrder) == fxChain.cend()) {
            item->disconnect();
            item->deleteLater();
            newItems.remove(chainOrder);
        }
    }

    for (const auto& pair : fxChain) {
        OutputResourceItem* item = newItems.value(pair.first, nullptr);

        if (item) {
            item->setParams(pair.second);
        } else {
            newItems.insert(pair.first, buildOutputResourceItem(pair.second));
        }
    }

    if (m_outputResourceItems != newItems) {
        m_outputResourceItems = std::move(newItems);
        emit outputResourceItemListChanged();
    }
}

void MixerChannelItem::loadAuxSendItems(const AuxSendsParams& auxSends)
{
    if (m_outParams.auxSends == auxSends) {
        return;
    }

    m_outParams.auxSends = auxSends;

    configuration()->isAuxSendVisibleChanged().onReceive(this, [this](aux_channel_idx_t index, bool visible) {
        if (visible) {
            IF_ASSERT_FAILED(index < m_outParams.auxSends.size()) {
                return;
            };

            m_auxSendItems.insert(index, buildAuxSendItem(index, m_outParams.auxSends[index]));
        } else {
            m_auxSendItems.remove(index);
        }

        emit auxSendItemListChanged();
    }, AsyncMode::AsyncSetOnce);

    if (m_auxSendItems.size() == static_cast<int>(auxSends.size())) {
        return;
    }

    QMap<aux_channel_idx_t, AuxSendItem*> newItems;

    for (aux_channel_idx_t i = 0; i < auxSends.size(); ++i) {
        if (!configuration()->isAuxSendVisible(i)) {
            continue;
        }

        auto it = m_auxSendItems.find(i);

        if (it != m_auxSendItems.end()) {
            newItems.insert(it.key(), it.value());
        } else {
            newItems.insert(i, buildAuxSendItem(i, auxSends[i]));
        }
    }

    if (m_auxSendItems != newItems) {
        m_auxSendItems = std::move(newItems);
        emit auxSendItemListChanged();
    }
}

void MixerChannelItem::loadSoloMuteState(const notation::INotationSoloMuteState::SoloMuteState& newState)
{
    if (m_outParams.muted != newState.mute) {
        m_outParams.muted = newState.mute;
        emit mutedChanged();
    }

    if (m_outParams.solo != newState.solo) {
        m_outParams.solo = newState.solo;
        emit soloChanged();
    }
}

void MixerChannelItem::subscribeOnAudioSignalChanges(AudioSignalChanges&& audioSignalChanges)
{
    m_audioSignalChanges = audioSignalChanges;

    m_audioSignalChanges.onReceive(this, [this](const AudioSignalValuesMap& signalValues) {
        //!Note There should be no signal changes when the mixer channel is muted.
        //!     But some audio signal changes still might be "on the way" from the times when the mixer channel wasn't muted
        //!     So that we have to just ignore them
        if (muted()) {
            return;
        }

        for (const auto& pair : signalValues) {
            audioch_t audioChNum = pair.first;
            volume_dbfs_t newPressure = pair.second.pressure;

            if (newPressure < MIN_DISPLAYED_DBFS) {
                setAudioChannelVolumePressure(audioChNum, MIN_DISPLAYED_DBFS);
            } else if (newPressure > MAX_DISPLAYED_DBFS) {
                setAudioChannelVolumePressure(audioChNum, MAX_DISPLAYED_DBFS);
            } else {
                setAudioChannelVolumePressure(audioChNum, newPressure);
            }
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

void MixerChannelItem::setSolo(bool solo)
{
    if (m_outParams.solo == solo) {
        return;
    }

    m_outParams.solo = solo;

    notation::INotationSoloMuteState::SoloMuteState soloMuteState;
    soloMuteState.mute = m_outParams.muted;
    soloMuteState.solo = m_outParams.solo;

    emit soloMuteStateChanged(soloMuteState);
    emit soloChanged();

    if (solo && m_outParams.muted) {
        setMuted(false);
    }
}

void MixerChannelItem::setMuted(bool mute)
{
    if (m_outParams.muted == mute) {
        return;
    }

    m_outParams.muted = mute;

    notation::INotationSoloMuteState::SoloMuteState soloMuteState;
    soloMuteState.mute = m_outParams.muted;
    soloMuteState.solo = m_outParams.solo;

    emit soloMuteStateChanged(soloMuteState);
    emit mutedChanged();

    if (mute && m_outParams.solo) {
        setSolo(false);
    }
}

mu::notation::INotationPlaybackPtr MixerChannelItem::notationPlayback() const
{
    project::INotationProjectPtr project = context()->currentProject();
    return project ? project->masterNotation()->playback() : nullptr;
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

InputResourceItem* MixerChannelItem::buildInputResourceItem()
{
    InputResourceItem* newItem = new InputResourceItem(this);

    connect(newItem, &InputResourceItem::inputParamsChangeRequested, this, [this, newItem](const AudioResourceMeta& newMeta) {
        if (askAboutChangingSound()) {
            newItem->setParamsRecourceMeta(newMeta);
        }
    });

    connect(newItem, &InputResourceItem::inputParamsChanged, this, [this, newItem]() {
        bool audioSourceChanged = m_inputParams.type() != newItem->params().type();

        m_inputParams = newItem->params();
        emit inputParamsChanged(m_inputParams);

        if (!audioSourceChanged) {
            return;
        }

        bool auxParamsChanged = false;
        for (aux_channel_idx_t idx = 0; idx < static_cast<size_t>(m_outParams.auxSends.size()); ++idx) {
            const muse::String& soundId = m_inputParams.resourceMeta.attributeVal(PLAYBACK_SETUP_DATA_ATTRIBUTE);
            gain_t newAudioSignalAmount = configuration()->defaultAuxSendValue(idx, m_inputParams.type(), soundId);

            auto it = m_auxSendItems.find(idx);
            if (it == m_auxSendItems.end()) {
                if (!muse::RealIsEqual(m_outParams.auxSends.at(idx).signalAmount, newAudioSignalAmount)) {
                    m_outParams.auxSends.at(idx).signalAmount = newAudioSignalAmount;
                    auxParamsChanged = true;
                }
            } else {
                it.value()->setAudioSignalPercentage(newAudioSignalAmount * 100.f);
            }
        }

        if (auxParamsChanged) {
            emit outputParamsChanged(m_outParams);
        }
    });

    connect(newItem, &InputResourceItem::isBlankChanged, this, &MixerChannelItem::inputResourceItemChanged);

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

OutputResourceItem* MixerChannelItem::buildOutputResourceItem(const audio::AudioFxParams& fxParams)
{
    OutputResourceItem* newItem = new OutputResourceItem(this, fxParams);

    connect(newItem, &OutputResourceItem::fxParamsChanged, this, [this]() {
        if (m_outputResourceItemsLoading) {
            return;
        }

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

        if (m_type != Type::Master) {
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

AuxSendItem* MixerChannelItem::buildAuxSendItem(aux_channel_idx_t index, const AuxSendParams& params)
{
    AuxSendItem* newItem = new AuxSendItem(this);
    newItem->blockSignals(true);
    newItem->setIsActive(params.active);
    newItem->setAudioSignalPercentage(params.signalAmount * 100.f);
    newItem->setTitle(muse::qtrc("playback", "Aux %1").arg(index + 1));
    newItem->blockSignals(false);

    connect(newItem, &AuxSendItem::isActiveChanged, this, [this, index](bool active) {
        IF_ASSERT_FAILED(index < m_outParams.auxSends.size()) {
            return;
        }

        m_outParams.auxSends[index].active = active;
        emit outputParamsChanged(m_outParams);
    });

    connect(newItem, &AuxSendItem::audioSignalPercentageChanged, this, [this, index](int percentage) {
        IF_ASSERT_FAILED(index < m_outParams.auxSends.size()) {
            return;
        }

        m_outParams.auxSends[index].signalAmount = static_cast<float>(percentage) / 100.f;
        emit outputParamsChanged(m_outParams);
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

bool MixerChannelItem::askAboutChangingSound()
{
    if (!configuration()->needToShowResetSoundFlagsWhenChangeSoundWarning()) {
        return true;
    }

    if (!notationPlayback()->hasSoundFlags({ m_instrumentTrackId })) {
        return true;
    }

    int changeBtn = int(IInteractive::Button::Apply);
    IInteractive::Options options = IInteractive::Option::WithIcon | IInteractive::Option::WithDontShowAgainCheckBox;
    IInteractive::ButtonDatas buttons = {
        interactive()->buttonData(IInteractive::Button::Cancel),
        IInteractive::ButtonData(changeBtn, muse::trc("playback", "Change sound"), true /*accent*/)
    };

    IInteractive::Result result = interactive()->warning(muse::trc("playback", "Are you sure you want to change this sound?"),
                                                         muse::trc("playback",
                                                                   "Sound flags on this instrument may be reset, but staff text will remain. This action can’t be undone."),
                                                         buttons, changeBtn, options);

    if (result.button() == changeBtn) {
        if (!result.showAgain()) {
            configuration()->setNeedToShowResetSoundFlagsWhenChangeSoundWarning(false);
        }

        return true;
    } else {
        return false;
    }
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
    return m_outputOnly;
}

const AudioInputParams& MixerChannelItem::inputParams() const
{
    return m_inputParams;
}

const AudioOutputParams& MixerChannelItem::outputParams() const
{
    return m_outParams;
}

InputResourceItem* MixerChannelItem::inputResourceItem() const
{
    return m_inputResourceItem;
}

QList<OutputResourceItem*> MixerChannelItem::outputResourceItemList() const
{
    return m_outputResourceItems.values();
}

QList<AuxSendItem*> MixerChannelItem::auxSendItemList() const
{
    return m_auxSendItems.values();
}

const QMap<aux_channel_idx_t, AuxSendItem*>& MixerChannelItem::auxSendItems() const
{
    return m_auxSendItems;
}
