#ifndef MU_AUDIO_AUDIOIOHANDLER_H
#define MU_AUDIO_AUDIOIOHANDLER_H

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "iaudioio.h"
#include "imixer.h"
#include "igettracksequence.h"

namespace mu::audio {
class AudioIOHandler : public IAudioIO, public async::Asyncable
{
    INJECT(audio, IMixer, mixer)

public:
    explicit AudioIOHandler(IGetTrackSequence* getSequence);

    async::Promise<AudioInputParams> inputParams(const TrackSequenceId sequenceId, const TrackId trackId) const override;
    void setInputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioInputParams& params) override;
    async::Channel<TrackSequenceId, TrackId, AudioInputParams> inputParamsChanged() const override;

    async::Promise<AudioOutputParams> outputParams(const TrackSequenceId sequenceId, const TrackId trackId) const override;
    void setOutputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioOutputParams& params) override;
    async::Channel<TrackSequenceId, TrackId, AudioOutputParams> outputParamsChanged() const override;

    async::Promise<AudioOutputParams> globalOutputParams() const override;
    void setGlobalOutputParams(const AudioOutputParams& params) override;
    async::Channel<AudioOutputParams> globalOutputParamsChanged() const override;

    async::Channel<audioch_t, float> masterSignalAmplitudeChanged() const override;
    async::Channel<audioch_t, volume_dbfs_t> masterVolumePressureChanged() const override;

private:
    ITrackSequencePtr sequence(const TrackSequenceId id) const;
    void ensureSubscriptions(const ITrackSequencePtr s) const;

    IGetTrackSequence* m_getSequence = nullptr;

    mutable async::Channel<TrackSequenceId, TrackId, AudioInputParams> m_inputParamsChanged;
    mutable async::Channel<TrackSequenceId, TrackId, AudioOutputParams> m_outputParamsChanged;
};
}

#endif // MU_AUDIO_AUDIOIOHANDLER_H
