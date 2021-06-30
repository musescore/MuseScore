#ifndef MU_AUDIO_IPLAYER_H
#define MU_AUDIO_IPLAYER_H

#include <memory>

#include "async/promise.h"
#include "async/channel.h"

#include "audiotypes.h"

namespace mu::audio {
class IPlayers
{
public:
    virtual ~IPlayers() = default;

    virtual void play(const TrackSequenceId sequenceId) = 0;
    virtual void seek(const TrackSequenceId sequenceId, const msecs_t newPositionMsecs) = 0;
    virtual void stop(const TrackSequenceId sequenceId) = 0;
    virtual void pause(const TrackSequenceId sequenceId) = 0;

    virtual async::Promise<bool> setLoop(const TrackSequenceId sequenceId, const msecs_t fromMsec, const msecs_t toMsec) = 0;
    virtual void resetLoop(const TrackSequenceId sequenceId) = 0;

    virtual async::Channel<TrackSequenceId, msecs_t> playbackPositionMsecs() const = 0;
};

using IPlayersPtr = std::shared_ptr<IPlayers>;
}

#endif // MU_AUDIO_IPLAYER_H
