#include "sequenceio.h"

#include "log.h"

#include "internal/audiosanitizer.h"
#include "audioerrors.h"

using namespace mu;
using namespace mu::audio;
using namespace mu::async;

SequenceIO::SequenceIO(IGetTracks* getTracks)
    : m_getTracks(getTracks)
{
}

RetVal<AudioInputParams> SequenceIO::inputParams(const TrackId id) const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_getTracks) {
        return {};
    }

    RetVal<AudioInputParams> result;

    TrackPtr track = m_getTracks->track(id);

    if (track) {
        return result.make_ok(track->inputParams());
    }

    return make_ret(Err::InvalidTrackId);
}

RetVal<AudioOutputParams> SequenceIO::outputParams(const TrackId id) const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_getTracks) {
        return {};
    }

    RetVal<AudioOutputParams> result;

    TrackPtr track = m_getTracks->track(id);

    if (track) {
        return result.make_ok(track->outputParams());
    }

    return make_ret(Err::InvalidTrackId);
}

void SequenceIO::setInputParams(const TrackId id, const AudioInputParams& params)
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_getTracks) {
        return;
    }

    TrackPtr track = m_getTracks->track(id);

    if (track && track->setInputParams(params)) {
        m_inputParamsChanged.send(id, params);
    }
}

void SequenceIO::setOutputParams(const TrackId id, const AudioOutputParams& params)
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_getTracks) {
        return;
    }

    TrackPtr track = m_getTracks->track(id);

    if (track && track->setOutputParams(params)) {
        m_outputParamsChanged.send(id, params);
    }
}

Channel<TrackId, AudioInputParams> SequenceIO::inputParamsChanged() const
{
    return m_inputParamsChanged;
}

Channel<TrackId, AudioOutputParams> SequenceIO::outputParamsChanged() const
{
    return m_outputParamsChanged;
}
