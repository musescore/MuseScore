#ifndef MU_AUDIO_SEQUENCETRACKS_H
#define MU_AUDIO_SEQUENCETRACKS_H

#include "async/asyncable.h"

#include "itracks.h"
#include "igettracksequence.h"

namespace mu::audio {
class TracksHandler : public ITracks, public async::Asyncable
{
public:
    explicit TracksHandler(IGetTrackSequence* getSequence);
    ~TracksHandler();

    async::Promise<TrackIdList> trackIdList(const TrackSequenceId sequenceId) const override;
    async::Promise<TrackId> addTrack(const TrackSequenceId sequenceId, const std::string& trackName, midi::MidiData&& inParams,
                                     AudioOutputParams&& outParams) override;
    async::Promise<TrackId> addTrack(const TrackSequenceId sequenceId, const std::string& trackName, io::path&& inParams,
                                     AudioOutputParams&& outParams) override;
    void removeTrack(const TrackSequenceId sequenceId, const TrackId trackId) override;
    void removeAllTracks(const TrackSequenceId sequenceId) override;

    async::Channel<TrackSequenceId, TrackId> trackAdded() const override;
    async::Channel<TrackSequenceId, TrackId> trackRemoved() const override;

private:
    ITrackSequencePtr sequence(const TrackSequenceId id) const;
    void ensureSubscriptions(const ITrackSequencePtr s) const;

    mutable async::Channel<TrackSequenceId, TrackId> m_trackAdded;
    mutable async::Channel<TrackSequenceId, TrackId> m_trackRemoved;

    IGetTrackSequence* m_getSequence = nullptr;
};
}

#endif // MU_AUDIO_SEQUENCETRACKS_H
