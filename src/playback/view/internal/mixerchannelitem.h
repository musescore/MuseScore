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
#include "ui/view/navigationpanel.h"
#include "project/iprojectaudiosettings.h"

#include "inputresourceitem.h"
#include "outputresourceitem.h"
#include "auxsenditem.h"

namespace mu::playback {
class MixerChannelItem : public QObject, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(Type type READ type CONSTANT)
    Q_PROPERTY(bool outputOnly READ outputOnly CONSTANT)

    Q_PROPERTY(QString title READ title NOTIFY titleChanged)

    Q_PROPERTY(InputResourceItem * inputResourceItem READ inputResourceItem NOTIFY inputResourceItemChanged)
    Q_PROPERTY(QList<OutputResourceItem*> outputResourceItemList READ outputResourceItemList NOTIFY outputResourceItemListChanged)
    Q_PROPERTY(QList<AuxSendItem*> auxSendItemList READ auxSendItemList NOTIFY auxSendItemListChanged)

    Q_PROPERTY(float leftChannelPressure READ leftChannelPressure NOTIFY leftChannelPressureChanged)
    Q_PROPERTY(float rightChannelPressure READ rightChannelPressure NOTIFY rightChannelPressureChanged)

    Q_PROPERTY(float volumeLevel READ volumeLevel WRITE setVolumeLevel NOTIFY volumeLevelChanged)
    Q_PROPERTY(int balance READ balance WRITE setBalance NOTIFY balanceChanged)

    Q_PROPERTY(bool muted READ muted NOTIFY mutedChanged)
    Q_PROPERTY(bool mutedManually READ mutedManually WRITE setMutedManually NOTIFY mutedChanged)
    Q_PROPERTY(bool solo READ solo WRITE setSolo NOTIFY soloChanged)

    Q_PROPERTY(mu::ui::NavigationPanel * panel READ panel NOTIFY panelChanged)

    INJECT(playback, framework::IInteractive, interactive)

public:
    enum class Type {
        Unknown,
        PrimaryInstrument,
        SecondaryInstrument,
        Aux,
        Master,
    };
    Q_ENUM(Type)

    MixerChannelItem() = default;
    MixerChannelItem(QObject* parent, Type type, bool outputOnly = false, audio::TrackId trackId = -1);

    ~MixerChannelItem() override;

    Type type() const;

    audio::TrackId trackId() const;

    const engraving::InstrumentTrackId& instrumentTrackId() const;
    void setInstrumentTrackId(const engraving::InstrumentTrackId& instrumentTrackId);

    QString title() const;

    float leftChannelPressure() const;
    float rightChannelPressure() const;

    float volumeLevel() const;
    int balance() const;

    bool muted() const;
    bool mutedManually() const;
    bool solo() const;

    ui::NavigationPanel* panel() const;
    void setPanelOrder(int panelOrder);
    void setPanelSection(ui::INavigationSection* section);

    void loadInputParams(audio::AudioInputParams&& newParams);
    void loadOutputParams(audio::AudioOutputParams&& newParams);
    void loadSoloMuteState(project::IProjectAudioSettings::SoloMuteState&& newState);

    void subscribeOnAudioSignalChanges(audio::AudioSignalChanges&& audioSignalChanges);

    bool outputOnly() const;

    InputResourceItem* inputResourceItem() const;
    QList<OutputResourceItem*> outputResourceItemList() const;
    QList<AuxSendItem*> auxSendItemList() const;

public slots:
    void setTitle(QString title);

    void setLeftChannelPressure(float leftChannelPressure);
    void setRightChannelPressure(float rightChannelPressure);

    void setVolumeLevel(float volumeLevel);
    void setBalance(int balance);

    void setMutedManually(bool isMuted);
    void setSolo(bool solo);

signals:
    void titleChanged(QString title);

    void leftChannelPressureChanged(float leftChannelPressure);
    void rightChannelPressureChanged(float rightChannelPressure);

    void volumeLevelChanged(float volumeLevel);
    void balanceChanged(int balance);

    void mutedChanged();
    void soloChanged();

    void panelChanged(ui::NavigationPanel* panel);

    void inputParamsChanged(const audio::AudioInputParams& params);
    void outputParamsChanged(const audio::AudioOutputParams& params);
    void soloMuteStateChanged(const project::IProjectAudioSettings::SoloMuteState& state);

    void inputResourceItemChanged();
    void outputResourceItemListChanged();
    void auxSendItemListChanged();

protected:
    void setAudioChannelVolumePressure(const audio::audioch_t chNum, const float newValue);
    void resetAudioChannelsVolumePressure();

    void applyMuteToOutputParams(const bool isMuted);

    void loadOutputResourceItems(const audio::AudioFxChain& fxChain);
    void loadAuxSendItems(const audio::AuxSendsParams& auxSends);

    InputResourceItem* buildInputResourceItem();
    OutputResourceItem* buildOutputResourceItem(const audio::AudioFxParams& fxParams);
    AuxSendItem* buildAuxSendItem(size_t index);

    void removeRedundantEmptySlots();
    QList<audio::AudioFxChainOrder> emptySlotsToRemove() const;

    void ensureBlankOutputResourceSlot();
    audio::AudioFxChainOrder resolveNewBlankOutputResourceItemOrder() const;

    void openEditor(AbstractAudioResourceItem* item, const UriQuery& editorUri);
    void closeEditor(AbstractAudioResourceItem* item);

    Type m_type = Type::Unknown;

    audio::TrackId m_trackId = -1;
    engraving::InstrumentTrackId m_instrumentTrackId;

    audio::AudioInputParams m_inputParams;
    audio::AudioOutputParams m_outParams;
    project::IProjectAudioSettings::SoloMuteState m_soloMuteState;

    InputResourceItem* m_inputResourceItem = nullptr;
    QMap<audio::AudioFxChainOrder, OutputResourceItem*> m_outputResourceItems;
    QList<AuxSendItem*> m_auxSendItems;

    audio::AudioSignalChanges m_audioSignalChanges;

    QString m_title;
    bool m_outputOnly = false;

    float m_leftChannelPressure = 0.0;
    float m_rightChannelPressure = 0.0;

    ui::NavigationPanel* m_panel = nullptr;
};
}

#endif // MU_PLAYBACK_MIXERCHANNELITEM_H
