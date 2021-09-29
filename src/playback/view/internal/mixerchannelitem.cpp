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

using namespace mu::playback;
using namespace mu::audio;

static const volume_dbfs_t MAX_DISPLAYED_DBFS = 0.f; // 100%
static const volume_dbfs_t MIN_DISPLAYED_DBFS = -60.f; // 0%

static const float BALANCE_SCALING_FACTOR = 100.f;

static const int OUTPUT_RESOURCE_COUNT_LIMIT = 4;

static const QString MASTER_VSTFX_EDITOR_URI_BODY("musescore://vstfx/editor?sync=false&modal=false&resourceId=%1&chainOrder=%2");
static const QString TRACK_VSTFX_EDITOR_URI_BODY("musescore://vstfx/editor?sync=false&modal=false&trackId=%1&resourceId=%2&chainOrder=%3");
static const QString TRACK_VSTI_EDITOR_URI_BODY("musescore://vsti/editor?sync=false&modal=false&trackId=%1&resourceId=%2");

MixerChannelItem::MixerChannelItem(QObject* parent, const audio::TrackId id, const bool isMaster)
    : QObject(parent),
    m_id(id),
    m_isMaster(isMaster),
    m_leftChannelPressure(MIN_DISPLAYED_DBFS),
    m_rightChannelPressure(MIN_DISPLAYED_DBFS)
{
    m_inputResourceItem = buildInputResourceItem();
}

MixerChannelItem::~MixerChannelItem()
{
    m_audioSignalChanges.pressureChanges.resetOnReceive(this);
}

TrackId MixerChannelItem::id() const
{
    return m_id;
}

bool MixerChannelItem::isMasterChannel() const
{
    return m_isMaster;
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

bool MixerChannelItem::muted() const
{
    return m_mutedManually || m_mutedBySolo;
}

bool MixerChannelItem::solo() const
{
    return m_outParams.solo;
}

void MixerChannelItem::loadInputParams(AudioInputParams&& newParams)
{
    if (m_inputParams == newParams) {
        return;
    }

    m_inputParams = newParams;
    m_inputResourceItem->setParams(newParams);
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
        m_mutedManually = newParams.muted;
        emit mutedChanged(newParams.muted);
    }

    if (m_outParams.solo != newParams.solo) {
        m_outParams.solo = newParams.solo;
        emit soloChanged();
    }

    if (m_outParams.fxChain != newParams.fxChain) {
        qDeleteAll(m_outputResourceItemList);
        m_outputResourceItemList.clear();

        for (int i = 0; i < OUTPUT_RESOURCE_COUNT_LIMIT; ++i) {
            m_outputResourceItemList << buildOutputResourceItem(newParams.fxChain[i]);
        }

        emit outputResourceItemListChanged(m_outputResourceItemList);
    }

    ensureBlankOutputResourceSlot();
}

void MixerChannelItem::subscribeOnAudioSignalChanges(AudioSignalChanges&& audioSignalChanges)
{
    m_audioSignalChanges = audioSignalChanges;

    m_audioSignalChanges.pressureChanges.onReceive(this, [this](const audioch_t audioChNum, const volume_dbfs_t newValue) {
        //!Note There should be no signal changes when the mixer channel is muted.
        //!     But some audio signal changes still might be "on the way" from the times when the mixer channel wasn't muted
        //!     So that we have to just ignore them
        if (muted()) {
            return;
        }

        if (newValue < MIN_DISPLAYED_DBFS) {
            setAudioChannelVolumePressure(audioChNum, MIN_DISPLAYED_DBFS);
        } else if (newValue > MAX_DISPLAYED_DBFS) {
            setAudioChannelVolumePressure(audioChNum, MAX_DISPLAYED_DBFS);
        } else {
            setAudioChannelVolumePressure(audioChNum, newValue);
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

void MixerChannelItem::setMuted(bool isMuted)
{
    if (muted() == isMuted) {
        return;
    }

    m_mutedManually = isMuted;
    applyMuteToOutputParams(muted());

    emit mutedChanged(isMuted);
}

void MixerChannelItem::setMutedBySolo(bool isMuted)
{
    if (m_mutedBySolo == isMuted) {
        return;
    }

    m_mutedBySolo = isMuted;
    applyMuteToOutputParams(muted());

    emit mutedChanged(isMuted);
}

void MixerChannelItem::setSolo(bool solo)
{
    if (m_outParams.solo == solo) {
        return;
    }

    if (solo) {
        setMutedBySolo(false);
    }

    m_outParams.solo = solo;
    emit outputParamsChanged(m_outParams);
    emit soloStateToggled(solo);
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

void MixerChannelItem::applyMuteToOutputParams(const bool isMuted)
{
    m_outParams.muted = isMuted;
    if (m_outParams.muted) {
        resetAudioChannelsVolumePressure();
    }

    emit outputParamsChanged(m_outParams);
}

InputResourceItem* MixerChannelItem::buildInputResourceItem()
{
    InputResourceItem* newItem = new InputResourceItem(this);

    connect(newItem, &InputResourceItem::inputParamsChanged, this, [this, newItem]() {
        m_inputParams = newItem->params();

        emit inputParamsChanged(m_inputParams);
    });

    connect(newItem, &InputResourceItem::isBlankChanged, this, &MixerChannelItem::inputResourceItemChanged);

    connect(newItem, &InputResourceItem::nativeEditorViewLaunchRequested, this, [this, newItem]() {
        QString uri;

        if (newItem->params().type() == AudioSourceType::Vsti) {
            uri = TRACK_VSTI_EDITOR_URI_BODY
                  .arg(m_id)
                  .arg(QString::fromStdString(newItem->params().resourceMeta.id));
        }

        interactive()->open(uri.toStdString());
    });

    return newItem;
}

OutputResourceItem* MixerChannelItem::buildOutputResourceItem(const audio::AudioFxParams& fxParams)
{
    OutputResourceItem* newItem = new OutputResourceItem(this, fxParams);

    connect(newItem, &OutputResourceItem::fxParamsChanged, this, [this]() {
        m_outParams.fxChain.clear();

        for (const OutputResourceItem* item : m_outputResourceItemList) {
            m_outParams.fxChain.insert({ item->params().chainOrder, item->params() });
        }

        emit outputParamsChanged(m_outParams);
    });

    connect(newItem, &OutputResourceItem::isBlankChanged, this, &MixerChannelItem::ensureBlankOutputResourceSlot);

    connect(newItem, &OutputResourceItem::nativeEditorViewLaunchRequested, this, [this, newItem]() {
        if (newItem->params().type() != AudioFxType::VstFx) {
            return;
        }

        QString uri;

        if (isMasterChannel()) {
            uri = MASTER_VSTFX_EDITOR_URI_BODY
                  .arg(QString::fromStdString(newItem->params().resourceMeta.id))
                  .arg(newItem->params().chainOrder);
        } else {
            uri = TRACK_VSTFX_EDITOR_URI_BODY
                  .arg(m_id)
                  .arg(QString::fromStdString(newItem->params().resourceMeta.id))
                  .arg(newItem->params().chainOrder);
        }

        interactive()->open(uri.toStdString());
    });

    return newItem;
}

void MixerChannelItem::ensureBlankOutputResourceSlot()
{
    removeRedundantEmptySlots();

    if (m_outputResourceItemList.count() >= OUTPUT_RESOURCE_COUNT_LIMIT) {
        return;
    }

    AudioFxParams params;
    params.chainOrder = m_outputResourceItemList.count();
    m_outputResourceItemList << buildOutputResourceItem(std::move(params));

    emit outputResourceItemListChanged(m_outputResourceItemList);
}

QList<OutputResourceItem*> MixerChannelItem::emptySlotsToRemove() const
{
    QList<OutputResourceItem*> result;

    for (OutputResourceItem* item : m_outputResourceItemList) {
        if (!item->isBlank()) {
            result.clear();
            continue;
        }

        result << item;
    }

    return result;
}

void MixerChannelItem::removeRedundantEmptySlots()
{
    for (OutputResourceItem* itemToRemove : emptySlotsToRemove()) {
        m_outputResourceItemList.removeOne(itemToRemove);
        itemToRemove->disconnect();
        itemToRemove->deleteLater();
    }
}

bool MixerChannelItem::outputOnly() const
{
    return m_isMaster;
}

const QList<OutputResourceItem*>& MixerChannelItem::outputResourceItemList() const
{
    return m_outputResourceItemList;
}

InputResourceItem* MixerChannelItem::inputResourceItem() const
{
    return m_inputResourceItem;
}
