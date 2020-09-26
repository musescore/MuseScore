//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_AUDIO_AUDIOENGINEDEVTOOLS_H
#define MU_AUDIO_AUDIOENGINEDEVTOOLS_H

#include <QObject>
#include <QTimer>
#include <QVariantList>

#include <optional>
#include "modularity/ioc.h"
#include "iaudioengine.h"
#include "imidiplayer.h"
#include "context/iglobalcontext.h"
#include "async/asyncable.h"
#include "global/iinteractive.h"
#include "iaudiostream.h"
#include "rpc/sequencer.h"

namespace mu::audio {
class AudioEngineDevTools : public QObject, public async::Asyncable
{
    Q_OBJECT
    INJECT(audio, IAudioEngine, audioEngine)
    //INJECT(audio, IAudioDriver, audioDriver)
    INJECT(audio, context::IGlobalContext, globalContext)
    INJECT(audio, audio::ISequencer, sequencer)
    INJECT(audio, framework::IInteractive, interactive)

    Q_PROPERTY(float time READ time NOTIFY timeChanged)
    Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)

public:
    explicit AudioEngineDevTools(QObject* parent = nullptr);

    Q_INVOKABLE void playSine();
    Q_INVOKABLE void stopSine();

    Q_INVOKABLE void setMuteSine(bool mute);
    Q_INVOKABLE void setMuteNoise(bool mute);

    Q_INVOKABLE void setLevelSine(float level);
    Q_INVOKABLE void setLevelNoise(float level);

    Q_INVOKABLE void setBalanceSine(float balance);
    Q_INVOKABLE void setBalanceNoise(float balance);

    Q_INVOKABLE void enableNoiseEq(bool enable);

    Q_INVOKABLE void playNoise();
    Q_INVOKABLE void stopNoise();

    Q_INVOKABLE void playSequencerMidi();
    Q_INVOKABLE void stopSequencerMidi();

    Q_INVOKABLE void playPlayerMidi();
    Q_INVOKABLE void stopPlayerMidi();

    Q_INVOKABLE void play();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void setLoop(unsigned int from, unsigned int to);
    Q_INVOKABLE void unsetLoop();

    Q_INVOKABLE void rpcPlay();
    Q_INVOKABLE void rpcStop();
    Q_INVOKABLE void rpcSetLoop(unsigned int from, unsigned int to);
    Q_INVOKABLE void rpcUnsetLoop();

    QVariantList devices() const;
    Q_INVOKABLE QString device() const;
    Q_INVOKABLE void selectDevice(QString name);

    Q_INVOKABLE void openAudio();
    Q_INVOKABLE void closeAudio();

    float time() const;

signals:
    void timeChanged();
    void devicesChanged();

private:
    void makeArpeggio();

    std::optional<unsigned int> m_sineChannelId, m_noiseChannel;
    std::shared_ptr<midi::MidiStream> m_midiStream = nullptr;
    std::shared_ptr<IAudioStream> m_audioStream = nullptr;
    std::weak_ptr<IMIDIPlayer> m_threadMIDIPlayer;
    RPCSequencer m_rpcSequencer;
};
}

#endif // MU_AUDIO_AUDIOENGINEDEVTOOLS_H
