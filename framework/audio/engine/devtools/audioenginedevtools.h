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

#include "modularity/ioc.h"
#include "audio/engine/iaudioengine.h"
#include "audio/engine/iaudioplayer.h"
#include "sinesource.h"
#include "midisource.h"

namespace mu {
namespace audio {
namespace engine {
class AudioEngineDevTools : public QObject
{
    Q_OBJECT
    INJECT(audio, IAudioEngine, audioEngine)
    INJECT(audio, IAudioPlayer, player)

public:
    explicit AudioEngineDevTools(QObject* parent = nullptr);

    Q_INVOKABLE void playSine();
    Q_INVOKABLE void stopSine();

    Q_INVOKABLE void playSourceMidi();
    Q_INVOKABLE void stopSourceMidi();

    Q_INVOKABLE void playPlayerMidi();
    Q_INVOKABLE void stopPlayerMidi();

private:

    std::shared_ptr<midi::MidiData> makeArpeggio() const;

    std::shared_ptr<SineSource> m_sineSource;
    IAudioEngine::handle m_sineHandle = 0;

    std::shared_ptr<midi::MidiData> m_midiData;
    std::shared_ptr<MidiSource> m_midiSource;
    IAudioEngine::handle m_midiHandel = 0;
};
}
}
}

#endif // MU_AUDIO_AUDIOENGINEDEVTOOLS_H
