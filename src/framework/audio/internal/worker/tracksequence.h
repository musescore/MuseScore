#ifndef MU_AUDIO_TRACKSEQUENCE_H
#define MU_AUDIO_TRACKSEQUENCE_H

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "imixer.h"
#include "imixerchannel.h"
#include "itracksequence.h"
#include "igettracks.h"
#include "iaudiosource.h"
#include "iclock.h"
#include "track.h"
#include "audiotypes.h"

namespace mu::audio {
class TrackSequence : public ITrackSequence, public IGetTracks, public async::Asyncable
{
    INJECT(audio, IMixer, mixer)

public:
    TrackSequence(const TrackSequenceId id);
    ~TrackSequence();

    // ITrackSequence
    TrackSequenceId id() const override;

    RetVal<TrackId> addTrack(const std::string& trackName, const midi::MidiData& midiData, const AudioOutputParams& outputParams) override;
    RetVal<TrackId> addTrack(const std::string& trackName, const io::path& filePath, const AudioOutputParams& outputParams) override;

    TrackIdList trackIdList() const override;

    Ret removeTrack(const TrackId id) override;

    async::Channel<TrackId> trackAdded() const override;
    async::Channel<TrackId> trackRemoved() const override;

    ISequencePlayerPtr player() const override;
    ISequenceIOPtr audioIO() const override;

    // IGetTracks
    TrackPtr track(const TrackId id) const override;
    TracksMap allTracks() const override;

private:
    TrackSequenceId m_id = -1;

    TracksMap m_tracks;

    ISequencePlayerPtr m_player = nullptr;
    ISequenceIOPtr m_audioIO = nullptr;

    IClockPtr m_clock = nullptr;

    async::Channel<TrackId> m_trackAdded;
    async::Channel<TrackId> m_trackRemoved;
};
}

#endif // MU_AUDIO_TRACKSEQUENCE_H
