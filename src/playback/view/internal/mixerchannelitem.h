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

#ifndef MU_PLAYBACK_MIXERCHANNELITEM_H
#define MU_PLAYBACK_MIXERCHANNELITEM_H

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "audio/audiotypes.h"
#include "iinteractive.h"

#include "inputresourceitem.h"
#include "outputresourceitem.h"

namespace mu::playback {
class MixerChannelItem : public QObject, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title NOTIFY titleChanged)

    Q_PROPERTY(bool outputOnly READ outputOnly CONSTANT)
    Q_PROPERTY(InputResourceItem * inputResourceItem READ inputResourceItem NOTIFY inputResourceItemChanged)
    Q_PROPERTY(QList<OutputResourceItem*> outputResourceItemList READ outputResourceItemList NOTIFY outputResourceItemListChanged)

    Q_PROPERTY(float leftChannelPressure READ leftChannelPressure NOTIFY leftChannelPressureChanged)
    Q_PROPERTY(float rightChannelPressure READ rightChannelPressure NOTIFY rightChannelPressureChanged)

    Q_PROPERTY(float volumeLevel READ volumeLevel WRITE setVolumeLevel NOTIFY volumeLevelChanged)
    Q_PROPERTY(float balance READ balance WRITE setBalance NOTIFY balanceChanged)

    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(bool solo READ solo WRITE setSolo NOTIFY soloChanged)

    INJECT(playback, framework::IInteractive, interactive)

public:
    explicit MixerChannelItem(QObject* parent, const audio::TrackId id = -1, const bool isMaster = false);

    ~MixerChannelItem();

    audio::TrackId id() const;

    bool isMasterChannel() const;

    QString title() const;

    float leftChannelPressure() const;
    float rightChannelPressure() const;

    float volumeLevel() const;
    float balance() const;

    bool muted() const;
    bool solo() const;

    void loadInputParams(const audio::AudioInputParams& newParams);
    void loadOutputParams(const audio::AudioOutputParams& newParams);

    void subscribeOnAudioSignalChanges(audio::AudioSignalChanges&& audioSignalChanges);

    bool outputOnly() const;

    const QList<OutputResourceItem*>& outputResourceItemList() const;

    InputResourceItem* inputResourceItem() const;

public slots:
    void setTitle(QString title);

    void setLeftChannelPressure(float leftChannelPressure);
    void setRightChannelPressure(float rightChannelPressure);

    void setVolumeLevel(float volumeLevel);
    void setBalance(float balance);

    void setMuted(bool muted);
    void setSolo(bool solo);

signals:
    void titleChanged(QString title);

    void leftChannelPressureChanged(float leftChannelPressure);
    void rightChannelPressureChanged(float rightChannelPressure);

    void volumeLevelChanged(float volumeLevel);
    void balanceChanged(float balance);

    void mutedChanged(bool muted);
    void soloChanged(bool solo);

    void inputParamsChanged(const audio::AudioInputParams& params);
    void outputParamsChanged(const audio::AudioOutputParams& params);

    void outputResourceItemListChanged(QList<OutputResourceItem*> itemList);

    void inputResourceItemChanged();

private:
    void setAudioChannelVolumePressure(const audio::audioch_t chNum, const float newValue);
    InputResourceItem* buildInputResourceItem();
    OutputResourceItem* buildOutputResourceItem(const audio::AudioFxParams& fxParams);
    void ensureBlankOutputResourceSlot();

    audio::TrackId m_id = -1;

    audio::AudioInputParams m_inputParams;
    audio::AudioOutputParams m_outParams;

    InputResourceItem* m_inputResourceItem = nullptr;
    QList<OutputResourceItem*> m_outputResourceItemList;

    audio::AudioSignalChanges m_audioSignalChanges;

    bool m_isMaster = false;
    QString m_title;

    float m_leftChannelPressure = 0.0;
    float m_rightChannelPressure = 0.0;

    bool m_solo = false;
};
}

#endif // MU_PLAYBACK_MIXERCHANNELITEM_H
