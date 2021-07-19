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

MixerChannelItem::MixerChannelItem(QObject* parent, const audio::TrackId id, const bool isMaster)
    : QObject(parent),
    m_id(id),
    m_isMaster(isMaster),
    m_leftChannelPressure(MIN_DISPLAYED_DBFS),
    m_rightChannelPressure(MIN_DISPLAYED_DBFS)
{
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

float MixerChannelItem::balance() const
{
    return m_outParams.balance;
}

bool MixerChannelItem::muted() const
{
    return m_outParams.isMuted;
}

bool MixerChannelItem::solo() const
{
    return m_solo;
}

void MixerChannelItem::setInputParams(AudioInputParams&& inputParams)
{
    m_inputParams = inputParams;
}

void MixerChannelItem::setOutputParams(AudioOutputParams&& outParams)
{
    m_outParams = outParams;
}

void MixerChannelItem::subscribeOnAudioSignalChanges(AudioSignalChanges&& audioSignalChanges)
{
    audioSignalChanges.pressureChanges.onReceive(this, [this](const audioch_t audioChNum, const volume_dbfs_t newValue) {
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

void MixerChannelItem::setBalance(float balance)
{
    if (qFuzzyCompare(m_outParams.balance, balance)) {
        return;
    }

    m_outParams.balance = balance;
    emit balanceChanged(balance);
    emit outputParamsChanged(m_outParams);
}

void MixerChannelItem::setMuted(bool muted)
{
    if (m_outParams.isMuted == muted) {
        return;
    }

    m_outParams.isMuted = muted;
    emit mutedChanged(muted);
    emit outputParamsChanged(m_outParams);
}

void MixerChannelItem::setSolo(bool solo)
{
    if (m_solo == solo) {
        return;
    }

    m_solo = solo;
    emit soloChanged(m_solo);
}

void MixerChannelItem::setAudioChannelVolumePressure(const audio::audioch_t chNum, const float newValue)
{
    if (chNum == 0) {
        setLeftChannelPressure(newValue);
    } else {
        setRightChannelPressure(newValue);
    }
}
