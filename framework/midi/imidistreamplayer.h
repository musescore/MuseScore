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
#ifndef MU_MIDI_IMIDISTREAMPLAYER_H
#define MU_MIDI_IMIDISTREAMPLAYER_H

#include "modularity/imoduleexport.h"
#include "async/channel.h"
#include "midi/miditypes.h"

namespace mu {
namespace midi {
class IMidiStreamPlayer : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMidiStreamPlayer)

    enum PlayStatus {
        UNDEFINED = 0,
        STOPED,
        PLAYING,
        PAUSED
    };
public:
    virtual ~IMidiStreamPlayer() = default;

    virtual PlayStatus status() const = 0;
    virtual async::Channel<PlayStatus> statusChanged() const = 0;
    virtual async::Channel<uint32_t> midiTickPlayed() const = 0;

    // data
    virtual void setMidiStream(const std::shared_ptr<midi::MidiStream>& stream) = 0;

    // Action
    virtual bool play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual void rewind() = 0;

    virtual void playMidi(const midi::MidiData& data) = 0;

    virtual float playbackPosition() const = 0;      // sec
    virtual void setPlaybackPosition(float sec) = 0; // sec
};
}
}

#endif // MU_MIDI_IMIDISTREAMPLAYER_H
