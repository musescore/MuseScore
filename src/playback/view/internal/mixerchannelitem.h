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

#ifndef MU_PLAYBACK_MIXERCHANNELITEM_H
#define MU_PLAYBACK_MIXERCHANNELITEM_H

#include <QObject>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "global/iinteractive.h"
#include "actions/iactionsdispatcher.h"
#include "context/iglobalcontext.h"
#include "iplaybackconfiguration.h"

#include "ui/view/navigationpanel.h"

#include "audio/audiotypes.h"
#include "inputresourceitem.h"
#include "outputresourceitem.h"
#include "auxsenditem.h"

namespace mu::playback {
class MixerChannelItem : public QObject, public muse::async::Asyncable
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
    Q_PROPERTY(bool solo READ solo WRITE setSolo NOTIFY soloChanged)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(bool forceMute READ forceMute NOTIFY forceMuteChanged)

    Q_PROPERTY(muse::ui::NavigationPanel * panel READ panel NOTIFY panelChanged)

    muse::Inject<muse::IInteractive> interactive;
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher;
    muse::Inject<context::IGlobalContext> context;
    muse::Inject<IPlaybackConfiguration> configuration;

public:
    enum class Type {
        Unknown,
        PrimaryInstrument,
        SecondaryInstrument,
        Metronome,
        Aux,
        Master,
    };
    Q_ENUM(Type)

    MixerChannelItem() = default;
    MixerChannelItem(QObject* parent, Type type, bool outputOnly = false, muse::audio::TrackId trackId = -1);

    ~MixerChannelItem() override;

    Type type() const;

    muse::audio::TrackId trackId() const;

    const engraving::InstrumentTrackId& instrumentTrackId() const;
    void setInstrumentTrackId(const engraving::InstrumentTrackId& instrumentTrackId);

    QString title() const;

    float leftChannelPressure() const;
    float rightChannelPressure() const;

    float volumeLevel() const;
    int balance() const;
    bool solo() const;
    bool muted() const;
    bool forceMute() const;

    muse::ui::NavigationPanel* panel() const;
    void setPanelOrder(int panelOrder);
    void setPanelSection(muse::ui::INavigationSection* section);

    void setOutputResourceItemCount(size_t count);

    void loadInputParams(const muse::audio::AudioInputParams& newParams);
    void loadOutputParams(const muse::audio::AudioOutputParams& newParams);
    void loadSoloMuteState(const notation::INotationSoloMuteState::SoloMuteState& newState);

    void subscribeOnAudioSignalChanges(muse::audio::AudioSignalChanges&& audioSignalChanges);

    bool outputOnly() const;

    const muse::audio::AudioInputParams& inputParams() const;
    const muse::audio::AudioOutputParams& outputParams() const;

    InputResourceItem* inputResourceItem() const;
    QList<OutputResourceItem*> outputResourceItemList() const;
    QList<AuxSendItem*> auxSendItemList() const;

    const QMap<muse::audio::aux_channel_idx_t, AuxSendItem*>& auxSendItems() const;

public slots:
    void setTitle(QString title);

    void setLeftChannelPressure(float leftChannelPressure);
    void setRightChannelPressure(float rightChannelPressure);

    void setVolumeLevel(float volumeLevel);
    void setBalance(int balance);
    void setSolo(bool solo);
    void setMuted(bool mute);

signals:
    void titleChanged(QString title);

    void leftChannelPressureChanged(float leftChannelPressure);
    void rightChannelPressureChanged(float rightChannelPressure);

    void volumeLevelChanged(float volumeLevel);
    void balanceChanged(int balance);
    void soloChanged();
    void mutedChanged();
    void forceMuteChanged();

    void panelChanged(muse::ui::NavigationPanel* panel);

    void inputParamsChanged(const muse::audio::AudioInputParams& params);
    void outputParamsChanged(const muse::audio::AudioOutputParams& params);
    void soloMuteStateChanged(const notation::INotationSoloMuteState::SoloMuteState& state);

    void inputResourceItemChanged();
    void outputResourceItemListChanged();
    void auxSendItemListChanged();

protected:
    notation::INotationPlaybackPtr notationPlayback() const;

    void setAudioChannelVolumePressure(const muse::audio::audioch_t chNum, const float newValue);
    void resetAudioChannelsVolumePressure();

    void applyMuteToOutputParams(const bool isMuted);

    void loadOutputResourceItems(const muse::audio::AudioFxChain& fxChain);
    void loadAuxSendItems(const muse::audio::AuxSendsParams& auxSends);

    InputResourceItem* buildInputResourceItem();
    OutputResourceItem* buildOutputResourceItem(const muse::audio::AudioFxParams& fxParams);
    AuxSendItem* buildAuxSendItem(muse::audio::aux_channel_idx_t index, const muse::audio::AuxSendParams& params);

    void addBlankSlots(size_t count);
    void removeBlankSlotsFromEnd(size_t count);

    muse::audio::AudioFxChainOrder resolveNewBlankOutputResourceItemOrder() const;

    void openEditor(AbstractAudioResourceItem* item, const muse::actions::ActionQuery& action);
    void closeEditor(AbstractAudioResourceItem* item);

    bool askAboutChangingSound();

    Type m_type = Type::Unknown;

    muse::audio::TrackId m_trackId = -1;
    engraving::InstrumentTrackId m_instrumentTrackId;

    muse::audio::AudioInputParams m_inputParams;
    muse::audio::AudioOutputParams m_outParams;

    InputResourceItem* m_inputResourceItem = nullptr;
    QMap<muse::audio::AudioFxChainOrder, OutputResourceItem*> m_outputResourceItems;
    QMap<muse::audio::aux_channel_idx_t, AuxSendItem*> m_auxSendItems;

    muse::audio::AudioSignalChanges m_audioSignalChanges;

    QString m_title;
    bool m_outputOnly = false;

    float m_leftChannelPressure = 0.0;
    float m_rightChannelPressure = 0.0;

    muse::ui::NavigationPanel* m_panel = nullptr;

    bool m_outputResourceItemsLoading = false;
};
}

#endif // MU_PLAYBACK_MIXERCHANNELITEM_H
