#ifndef MU_AUDIO_SEQUENCEIO_H
#define MU_AUDIO_SEQUENCEIO_H

#include "isequenceio.h"
#include "igettracks.h"
#include "audiotypes.h"

namespace mu::audio {
class SequenceIO : public ISequenceIO
{
public:
    explicit SequenceIO(IGetTracks* getTracks);

    RetVal<AudioInputParams> inputParams(const TrackId id) const;
    RetVal<AudioOutputParams> outputParams(const TrackId id) const;

    void setInputParams(const TrackId id, const AudioInputParams& params);
    void setOutputParams(const TrackId id, const AudioOutputParams& params);

    async::Channel<TrackId, AudioInputParams> inputParamsChanged() const;
    async::Channel<TrackId, AudioOutputParams> outputParamsChanged() const;

private:
    IGetTracks* m_getTracks = nullptr;

    async::Channel<TrackId, AudioInputParams> m_inputParamsChanged;
    async::Channel<TrackId, AudioOutputParams> m_outputParamsChanged;
};
}

#endif // MU_AUDIO_SEQUENCEIO_H
