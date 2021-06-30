#ifndef MU_AUDIO_ISEQUENCEPLAYER_H
#define MU_AUDIO_ISEQUENCEPLAYER_H

#include "ret.h"
#include "async/channel.h"

#include "audiotypes.h"

namespace mu::audio {
class ISequencePlayer
{
public:
    virtual ~ISequencePlayer() = default;

    virtual void play() = 0;
    virtual void seek(const msecs_t newPositionMsecs) = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;

    virtual Ret setLoop(const msecs_t fromMsec, const msecs_t toMsec) = 0;
    virtual void resetLoop() = 0;

    virtual async::Channel<msecs_t> playbackPositionMSecs() const = 0;
};
using ISequencePlayerPtr = std::shared_ptr<ISequencePlayer>;
}

#endif // MU_AUDIO_ISEQUENCEPLAYER_H
