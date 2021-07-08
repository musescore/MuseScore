#include "mixerchannelitem.h"

using namespace mu::playback;

MixerChannelItem::MixerChannelItem(QObject* parent)
    : QObject(parent)
{
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
    return m_volumeLevel;
}

float MixerChannelItem::balance() const
{
    return m_balance;
}

bool MixerChannelItem::muted() const
{
    return m_muted;
}

bool MixerChannelItem::solo() const
{
    return m_solo;
}

void MixerChannelItem::setInputParams(audio::AudioInputParams&& inputParams)
{
    m_inputParams = inputParams;
}

void MixerChannelItem::setOutputParams(audio::AudioOutputParams&& outParams)
{
    m_outParams = outParams;
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
    if (qFuzzyCompare(m_volumeLevel, volumeLevel)) {
        return;
    }

    m_volumeLevel = volumeLevel;
    emit volumeLevelChanged(m_volumeLevel);
}

void MixerChannelItem::setBalance(float balance)
{
    if (qFuzzyCompare(m_balance, balance)) {
        return;
    }

    m_balance = balance;
    emit balanceChanged(m_balance);
}

void MixerChannelItem::setMuted(bool muted)
{
    if (m_muted == muted) {
        return;
    }

    m_muted = muted;
    emit mutedChanged(m_muted);
}

void MixerChannelItem::setSolo(bool solo)
{
    if (m_solo == solo) {
        return;
    }

    m_solo = solo;
    emit soloChanged(m_solo);
}
