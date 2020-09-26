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
#ifndef MU_AUDIO_ISEQUENCER_H
#define MU_AUDIO_ISEQUENCER_H

#include <variant>

#include "async/channel.h"
#include "modularity/imoduleexport.h"
#include "iaudioplayer.h"
#include "imidiplayer.h"
#include "iaudiostream.h"
namespace mu::audio {
class ISequencer : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ISequencer)

public:
    virtual ~ISequencer() = default;

    using midi_track_t = std::shared_ptr<IMIDIPlayer>;
    using audio_track_t = std::shared_ptr<IAudioPlayer>;
    using track_t = std::shared_ptr<IPlayer>;
    using track_id = unsigned int;

    enum Status {
        STOPED,
        PLAYING
    };

    virtual Status status() const = 0;
    virtual async::Channel<Status> statusChanged() const = 0;
    virtual async::Channel<audio_track_t> audioTrackAdded() const = 0;

    virtual void initMIDITrack(track_id id) = 0;
    virtual void initAudioTrack(track_id id) = 0;
    virtual void setMIDITrack(track_id id, const std::shared_ptr<midi::MidiStream>& stream) = 0;
    virtual void setAudioTrack(track_id id, const std::shared_ptr<audio::IAudioStream>& stream) = 0;

    virtual bool play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual void seek(unsigned long sec) = 0;
    virtual void rewind() = 0;
    virtual void setLoop(unsigned int from, unsigned int to) = 0;
    virtual void unsetLoop() = 0;

    virtual async::Channel<mu::midi::tick_t> midiTickPlayed(track_id id) const = 0;
    virtual async::Notification positionChanged() const = 0;

    //!return position in seconds
    virtual float playbackPosition() const = 0;
    virtual midi_track_t instantlyPlayMidi(const midi::MidiData& data) = 0;
};

using ISequencerPtr = std::shared_ptr<ISequencer>;
}
#endif // MU_AUDIO_ISEQUENCER_H
