#include "sequenceplayer.h"

#include "log.h"

#include "internal/audiosanitizer.h"

using namespace mu;
using namespace mu::audio;
using namespace mu::async;

SequencePlayer::SequencePlayer(IGetTracks* getTracks, IClockPtr clock)
    : m_getTracks(getTracks), m_clock(clock)
{
    m_clock->seekOccurred().onNotify(this, [this]() {
        for (auto& pair : tracks()) {
            pair.second->audioSource->seek(m_clock->currentTime());
        }
    });
}

void SequencePlayer::play()
{
    ONLY_AUDIO_WORKER_THREAD;

    m_clock->start();

    for (auto& pair : tracks()) {
        pair.second->audioSource->setIsActive(true);
    }
}

void SequencePlayer::seek(const msecs_t newPositionMsecs)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_clock->seek(newPositionMsecs);

    for (auto& pair : tracks()) {
        pair.second->audioSource->seek(newPositionMsecs);
    }
}

void SequencePlayer::stop()
{
    ONLY_AUDIO_WORKER_THREAD;

    m_clock->stop();

    for (auto& pair : tracks()) {
        pair.second->audioSource->setIsActive(false);
    }
}

void SequencePlayer::pause()
{
    ONLY_AUDIO_WORKER_THREAD;

    m_clock->pause();

    for (auto& pair : tracks()) {
        pair.second->audioSource->setIsActive(false);
    }
}

Ret SequencePlayer::setLoop(const msecs_t fromMsec, const msecs_t toMsec)
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_clock->setTimeLoop(fromMsec, toMsec);
}

void SequencePlayer::resetLoop()
{
    ONLY_AUDIO_WORKER_THREAD;

    m_clock->resetTimeLoop();
}

Channel<msecs_t> SequencePlayer::playbackPositionMSecs() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_clock->timeChanged();
}

TracksMap SequencePlayer::tracks() const
{
    IF_ASSERT_FAILED(m_getTracks) {
        return {};
    }

    return m_getTracks->allTracks();
}
