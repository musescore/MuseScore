#include "tracksequence.h"

#include "log.h"

#include "internal/audiosanitizer.h"
#include "internal/audiothread.h"
#include "clock.h"
#include "midiaudiosource.h"
#include "sequenceplayer.h"
#include "sequenceio.h"
#include "audioerrors.h"

using namespace mu;
using namespace mu::async;
using namespace mu::audio;
using namespace mu::midi;

TrackSequence::TrackSequence(const TrackSequenceId id)
    : m_id(id)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_clock = std::make_shared<Clock>();
    m_player = std::make_shared<SequencePlayer>(this, m_clock);
    m_audioIO = std::make_shared<SequenceIO>(this);

    mixer()->addClock(m_clock);
}

TrackSequence::~TrackSequence()
{
    mixer()->removeClock(m_clock);

    for (const auto& pair : m_tracks) {
        if (pair.second && pair.second->mixerChannel) {
            mixer()->removeChannel(pair.second->mixerChannel->id());
        }
    }
}

TrackSequenceId TrackSequence::id() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_id;
}

RetVal<TrackId> TrackSequence::addTrack(const std::string& trackName, const midi::MidiData& midiData, const AudioOutputParams& outputParams)
{
    ONLY_AUDIO_WORKER_THREAD;

    RetVal<TrackId> result;

    if (!midiData.mapping.isValid()) {
        result.ret = make_ret(Err::InvalidMidiMapping);
        result.val = -1;
        return result;
    }

    TrackId newId = m_tracks.size();

    MidiTrack track;
    track.id = newId;
    track.name = trackName;
    track.setInputParams(midiData);
    track.setOutputParams(outputParams);
    track.audioSource = std::make_shared<MidiAudioSource>(midiData, track.inputParamsChanged);
    track.mixerChannel = mixer()->addChannel(track.audioSource, outputParams, track.outputParamsChanged).val;

    m_tracks.emplace(newId, std::make_shared<MidiTrack>(std::move(track)));

    m_trackAdded.send(newId);

    return result.make_ok(newId);
}

RetVal<TrackId> TrackSequence::addTrack(const std::string& trackName, const io::path& filePath, const AudioOutputParams& outputParams)
{
    ONLY_AUDIO_WORKER_THREAD;

    NOT_IMPLEMENTED;

    RetVal<TrackId> result;

    if (filePath.empty()) {
        result.ret = make_ret(Err::InvalidAudioFilePath);
        result.val = -1;
        return result;
    }

    TrackId newId = m_tracks.size();

    AudioTrack track;
    track.id = newId;
    track.name = trackName;
    track.setInputParams(filePath);
    track.setOutputParams(outputParams);
    //TODO create AudioSource and MixerChannel

    m_tracks.emplace(newId, std::make_shared<AudioTrack>(std::move(track)));

    m_trackAdded.send(newId);

    return result.make_ok(newId);
}

TrackIdList TrackSequence::trackIdList() const
{
    ONLY_AUDIO_WORKER_THREAD;

    TrackIdList result(m_tracks.size());

    for (const auto& pair : m_tracks) {
        result.push_back(pair.first);
    }

    return result;
}

Ret TrackSequence::removeTrack(const TrackId id)
{
    ONLY_AUDIO_WORKER_THREAD;

    auto search = m_tracks.find(id);

    if (search != m_tracks.end() && search->second) {
        mixer()->removeChannel(search->second->mixerChannel->id());
        m_tracks.erase(id);
        m_trackRemoved.send(id);
        return true;
    }

    return make_ret(Err::InvalidTrackId);
}

Channel<TrackId> TrackSequence::trackAdded() const
{
    return m_trackAdded;
}

Channel<TrackId> TrackSequence::trackRemoved() const
{
    return m_trackRemoved;
}

ISequencePlayerPtr TrackSequence::player() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_player;
}

ISequenceIOPtr TrackSequence::audioIO() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_audioIO;
}

TrackPtr TrackSequence::track(const TrackId id) const
{
    ONLY_AUDIO_WORKER_THREAD;

    auto search = m_tracks.find(id);

    if (search != m_tracks.end()) {
        return search->second;
    }

    return nullptr;
}

TracksMap TrackSequence::allTracks() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_tracks;
}
