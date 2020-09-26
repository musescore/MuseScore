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
#ifndef MU_MIDI_MIDISTREAMPLAYER_H
#define MU_MIDI_MIDISTREAMPLAYER_H

#include "imidistreamplayer.h"
#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "isequencer.h"
#include "retval.h"

namespace mu {
namespace midi {
class MidiStreamPlayer : public IMidiStreamPlayer, public async::Asyncable
{
    INJECT(audio, midi::ISequencer, sequencer)

public:
    MidiStreamPlayer();

    PlayStatus status() const override;
    async::Channel<PlayStatus> statusChanged() const override;
    async::Channel<uint32_t> midiTickPlayed() const override;

    // data
    void setMidiStream(const std::shared_ptr<midi::MidiStream>& stream) override;

    // Action
    bool play() override;
    void pause() override;
    void stop() override;
    void rewind() override;

    void playMidi(const midi::MidiData& data) override;

    float playbackPosition() const override;
    void setPlaybackPosition(float sec) override;

private:

    bool init();
    bool isInited() const;
    bool doPlay();
    void doPause();
    void doStop();
    void onStop();

    float currentPlayPosition() const;

    bool hasTracks() const;

    bool m_inited = false;
    ValCh<PlayStatus> m_status;

    std::shared_ptr<midi::MidiStream> m_midiStream;
    float m_beginPlayPosition = 0.0f;
    uint32_t m_lastMidiPlayTick = 0;

    async::Channel<uint32_t> m_midiTickPlayed;
    std::shared_ptr<midi::MidiStream> m_stream;
};
}
}
#endif // MU_MIDI_MIDISTREAMPLAYER_H
