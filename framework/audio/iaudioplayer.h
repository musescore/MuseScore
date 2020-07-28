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
#ifndef MU_AUDIO_IAUDIOPLAYER_H
#define MU_AUDIO_IAUDIOPLAYER_H

#include "modularity/imoduleexport.h"

#include "async/channel.h"
#include "audiotypes.h"
#include "midi/miditypes.h"

//! NOTE This is the main public playback control interface for consumers,
//! so namespace is just mu::audio

namespace mu {
namespace audio {
class IAudioPlayer : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAudioPlayer)

public:
    virtual ~IAudioPlayer() = default;

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

    // General
    virtual float generalVolume() const = 0;    // 0.0 to 1.0.
    virtual void setGeneralVolume(float v) = 0; // 0.0 to 1.0.
    virtual float generalBalance() const = 0;
    virtual void setGeneralBalance(float b) = 0; // -1.0 only left, 0.0 center, 1.0 only right
};
}
}

#endif // MU_AUDIO_IAUDIOPLAYER_H
