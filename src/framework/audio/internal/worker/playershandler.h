#ifndef MU_AUDIO_PLAYERSHANDLER_H
#define MU_AUDIO_PLAYERSHANDLER_H

#include "async/asyncable.h"

#include "iplayers.h"
#include "igettracksequence.h"

namespace mu::audio {
class PlayersHandler : public IPlayers, public async::Asyncable
{
public:
    explicit PlayersHandler(IGetTrackSequence* getSequence);
    ~PlayersHandler();

    void play(const TrackSequenceId sequenceId) override;
    void seek(const TrackSequenceId sequenceId, const msecs_t newPositionMsecs) override;
    void stop(const TrackSequenceId sequenceId) override;
    void pause(const TrackSequenceId sequenceId) override;

    async::Promise<bool> setLoop(const TrackSequenceId sequenceId, const msecs_t fromMsec, const msecs_t toMsec) override;
    void resetLoop(const TrackSequenceId sequenceId) override;

    async::Channel<TrackSequenceId, msecs_t> playbackPositionMsecs() const override;

private:
    ITrackSequencePtr sequence(const TrackSequenceId id) const;
    void ensureSubscriptions(const ITrackSequencePtr s) const;

    IGetTrackSequence* m_getSequence = nullptr;

    mutable async::Channel<TrackSequenceId, msecs_t> m_playbackPositionMsecsChanged;
};
}

#endif // MU_AUDIO_PLAYERSHANDLER_H
