#ifndef MU_AUDIO_ITRACKSHANDLER_H
#define MU_AUDIO_ITRACKSHANDLER_H

#include <memory>

#include "async/promise.h"
#include "async/channel.h"
#include "midi/miditypes.h"
#include "io/path.h"

#include "audiotypes.h"

namespace mu::audio {
class ITracks
{
public:
    virtual ~ITracks() = default;

    virtual async::Promise<TrackIdList> trackIdList(const TrackSequenceId sequenceId) const = 0;
    virtual async::Promise<TrackId> addTrack(const TrackSequenceId sequenceId, const std::string& trackName, midi::MidiData&& inParams,
                                             AudioOutputParams&& outParams) = 0;
    virtual async::Promise<TrackId> addTrack(const TrackSequenceId sequenceId, const std::string& trackName, io::path&& inParams,
                                             AudioOutputParams&& outParams) = 0;
    virtual void removeTrack(const TrackSequenceId sequenceId, const TrackId trackId) = 0;
    virtual void removeAllTracks(const TrackSequenceId sequenceId) = 0;

    virtual async::Channel<TrackSequenceId, TrackId> trackAdded() const = 0;
    virtual async::Channel<TrackSequenceId, TrackId> trackRemoved() const = 0;
};

using ITracksPtr = std::shared_ptr<ITracks>;
}

#endif // MU_AUDIO_ITRACKSHANDLER_H
