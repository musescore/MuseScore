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
#ifndef MU_AUDIO_AUDIOPLAYER_H
#define MU_AUDIO_AUDIOPLAYER_H

#include "../iaudioplayer.h"

#include "modularity/ioc.h"
#include "../iaudioengine.h"

#include "async/asyncable.h"
#include "midisource.h"

namespace mu {
namespace audio {
class AudioPlayer : public IAudioPlayer, public async::Asyncable
{
    INJECT(audio, engine::IAudioEngine, audioEngine)

public:
    AudioPlayer();

    ValCh<PlayStatus> status() const override;

    // data
    void setMidiStream(const std::shared_ptr<midi::MidiStream>& stream) override;

    // Action
    bool play() override;
    void pause() override;
    void stop() override;
    void rewind() override;

    float playbackPosition() const override;
    void setPlaybackPosition(float sec) override;

    // General
    float generalVolume() const override;
    void setGeneralVolume(float v) override;
    float generalBalance() const override;
    void setGeneralBalance(float b) override;

private:

    struct Track {
        bool muted = false;
        float volume = 1.0f;
        float balance = 0.0f;
    };

    bool init();
    bool isInited() const;
    bool doPlay();
    void doPause();
    void doStop();

    float currentPlayPosition() const;

    bool hasTracks() const;

    float normalizedVolume(float volume) const;
    float normalizedBalance(float balance) const;

    void applyCurrentVolume();
    void applyCurrentBalance();

    void onPlayCallbackCalled();

    bool m_inited = false;
    ValCh<PlayStatus> m_status;

    std::shared_ptr<midi::MidiStream> m_midiStream;
    std::shared_ptr<engine::MidiSource> m_midiSource;
    engine::IAudioEngine::handle m_midiHandle = 0;

    float m_beginPlayPosition = 0.0f;

    float m_generalVolume = 1.0f;
    float m_generalBalance = 0.0f;

    std::map<int, std::shared_ptr<Track> > m_tracks;
};
}
}

#endif // MU_AUDIO_AUDIOPLAYER_H
