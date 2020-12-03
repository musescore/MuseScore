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

#include "modularity/ioc.h"
#include "iaudioengine.h"
#include "iaudioplayer.h"
#include "context/iglobalcontext.h"
#include "internal/sinesource.h"
#include "internal/midisource.h"
#include "async/asyncable.h"

namespace mu {
namespace audio {
class AudioEngineDevTools : public QObject, public async::Asyncable
{
    Q_OBJECT
    INJECT(audio, IAudioEngine, audioEngine)
    INJECT(audio, IAudioPlayer, player)
    INJECT(audio, context::IGlobalContext, globalContext)

public:
    explicit AudioEngineDevTools(QObject* parent = nullptr);

    Q_INVOKABLE void playSine();
    Q_INVOKABLE void stopSine();

    Q_INVOKABLE void playSourceMidi();
    Q_INVOKABLE void stopSourceMidi();

    Q_INVOKABLE void playPlayerMidi();
    Q_INVOKABLE void stopPlayerMidi();

    Q_INVOKABLE void playNotation();
    Q_INVOKABLE void stopNotation();

private:

    void makeArpeggio();

    std::shared_ptr<SineSource> m_sineSource;
    IAudioEngine::handle m_sineHandle = 0;

    std::shared_ptr<midi::MidiStream> m_midiStream;

    std::shared_ptr<MidiSource> m_midiSource;
    IAudioEngine::handle m_midiHandel = 0;
};
}
}

#endif // MU_AUDIO_AUDIOENGINEDEVTOOLS_H
