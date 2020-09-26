//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "audioenginedevtools.h"
#include "framework/audio/internal/mixer.h"
#include "internal/noisesource.h"
#include "internal/sinesource.h"
#include "internal/equaliser.h"
#include "internal/audiostream.h"
#include "log.h"

using namespace mu::audio;
using namespace mu::midi;

AudioEngineDevTools::AudioEngineDevTools(QObject* parent)
    : QObject(parent)
{
    audioEngine()->driver()->availableOutputDevicesChanged().onNotify(this, [this]() {
        emit devicesChanged();
    });

    sequencer()->positionChanged().onNotify(this, [this]() {
        emit timeChanged();
    });
}

void AudioEngineDevTools::playSine()
{
    auto source = std::make_shared<SineSource>();

    if (!m_sineChannelId) {
        m_sineChannelId = audioEngine()->mixer()->addChannel(source);
    }
}

void AudioEngineDevTools::stopSine()
{
    audioEngine()->mixer()->removeChannel(m_sineChannelId.value_or(-1));
    m_sineChannelId.reset();
}

void AudioEngineDevTools::setMuteSine(bool mute)
{
    if (m_sineChannelId) {
        auto channel = audioEngine()->mixer()->channel(m_sineChannelId.value_or(-1));
        channel->setActive(!mute);
    }
}

void AudioEngineDevTools::setMuteNoise(bool mute)
{
    if (m_noiseChannel) {
        auto channel = audioEngine()->mixer()->channel(m_noiseChannel.value_or(-1));
        if (channel) {
            channel->setActive(!mute);
        }
    }
}

void AudioEngineDevTools::setLevelSine(float level)
{
    if (m_sineChannelId) {
        auto channel = audioEngine()->mixer()->channel(m_sineChannelId.value_or(-1));
        if (channel) {
            channel->setLevel(level);
        }
    }
}

void AudioEngineDevTools::setLevelNoise(float level)
{
    if (m_noiseChannel) {
        auto channel = audioEngine()->mixer()->channel(m_noiseChannel.value_or(-1));
        if (channel) {
            channel->setLevel(level);
        }
    }
}

void AudioEngineDevTools::setBalanceSine(float balance)
{
    if (m_sineChannelId) {
        auto channel = audioEngine()->mixer()->channel(m_sineChannelId.value_or(-1));
        if (channel) {
            channel->setBalance(balance);
        }
    }
}

void AudioEngineDevTools::setBalanceNoise(float balance)
{
    if (m_noiseChannel) {
        auto channel = audioEngine()->mixer()->channel(m_noiseChannel.value_or(-1));
        if (channel) {
            channel->setBalance(balance);
        }
    }
}

void AudioEngineDevTools::enableNoiseEq(bool enable)
{
    if (m_noiseChannel) {
        auto channel = audioEngine()->mixer()->channel(m_noiseChannel.value_or(-1));
        if (channel) {
            channel->insert(0)->setActive(enable);
        }
    }
}

void AudioEngineDevTools::playNoise()
{
    auto source = std::make_shared<NoiseSource>();
    source->setType(NoiseSource::Type::PINK);

    auto eq = std::make_shared<Equaliser>();
    eq->setFrequency(500);
    eq->setQ(3);
    eq->setGain(-12);
    eq->setActive(false);
    if (!m_noiseChannel) {
        m_noiseChannel = audioEngine()->mixer()->addChannel(source);
        auto channel = audioEngine()->mixer()->channel(m_noiseChannel.value_or(-1));
        channel->setInsert(0, eq);
    }
}

void AudioEngineDevTools::stopNoise()
{
    audioEngine()->mixer()->removeChannel(m_noiseChannel.value_or(-1));
    m_noiseChannel.reset();
}

void AudioEngineDevTools::playSequencerMidi()
{
    if (!m_midiStream) {
        makeArpeggio();
    }

    sequencer()->setMIDITrack(0, m_midiStream);
    sequencer()->play();
}

void AudioEngineDevTools::stopSequencerMidi()
{
    sequencer()->stop();
}

void AudioEngineDevTools::playPlayerMidi()
{
    makeArpeggio();
    m_threadMIDIPlayer = sequencer()->instantlyPlayMidi(m_midiStream->initData);
}

void AudioEngineDevTools::stopPlayerMidi()
{
    if (auto player = m_threadMIDIPlayer.lock()) {
        player->stop();
    }
}

void AudioEngineDevTools::play()
{
    auto notation = globalContext()->currentNotation();
    if (notation) {
        auto stream = notation->playback()->midiStream();
        sequencer()->setMIDITrack(0, stream);
    }
    sequencer()->rewind();
    sequencer()->play();
}

void AudioEngineDevTools::stop()
{
    sequencer()->stop();
}

void AudioEngineDevTools::setLoop(unsigned int from, unsigned int to)
{
    sequencer()->setLoop(from, to);
}

void AudioEngineDevTools::unsetLoop()
{
    sequencer()->unsetLoop();
}

void AudioEngineDevTools::rpcPlay()
{
    auto notation = globalContext()->currentNotation();
    if (notation) {
        auto stream = notation->playback()->midiStream();
        sequencer()->setMIDITrack(0, stream);
    }

    m_rpcSequencer.rewind();
    m_rpcSequencer.play();
}

void AudioEngineDevTools::rpcStop()
{
    m_rpcSequencer.stop();
}

void AudioEngineDevTools::rpcSetLoop(unsigned int from, unsigned int to)
{
    m_rpcSequencer.setLoop(from, to);
}

void AudioEngineDevTools::rpcUnsetLoop()
{
    m_rpcSequencer.unsetLoop();
}

void AudioEngineDevTools::openAudio()
{
    auto path = interactive()->selectOpeningFile("audio file", "", "Audio files (*.wav *mp3 *ogg)");
    if (!m_audioStream) {
        m_audioStream = std::make_shared<AudioStream>();
    }

    if (!path.empty()) {
        if (m_audioStream->loadFile(path)) {
            sequencer()->setAudioTrack(1, m_audioStream);
        }
    }
}

void AudioEngineDevTools::closeAudio()
{
    sequencer()->setAudioTrack(1, nullptr);
}

float AudioEngineDevTools::time() const
{
    return sequencer()->playbackPosition();
}

QVariantList AudioEngineDevTools::devices() const
{
    QVariantList list;
    auto devices = audioEngine()->driver()->availableOutputDevices();
    for (auto&& device : devices) {
        list.push_back(QString::fromStdString(device));
    }
    return list;
}

QString AudioEngineDevTools::device() const
{
    return QString::fromStdString(audioEngine()->driver()->outputDevice());
}

void AudioEngineDevTools::selectDevice(QString name)
{
    audioEngine()->driver()->selectOutputDevice(name.toStdString());
}

void AudioEngineDevTools::makeArpeggio()
{
    if (m_midiStream) {
        return;
    }

    m_midiStream = std::make_shared<midi::MidiStream>();

    Track t;
    t.num = 0;
    t.channels.push_back(0);
    m_midiStream->initData.tracks.push_back(t);

    midi::Event e(Event::Opcode::ProgramChange);
    m_midiStream->initData.initEvents.push_back(e);

    auto makeChunk = [](Chunk& chunk, uint32_t tick, int pitch) {
        UNUSED(pitch);
        /* notes of the arpeggio */
        static std::vector<int> notes = { 60, 64, 67, 72, 76, 79, 84, 79, 76, 72, 67, 64 };
        static uint32_t duration = 4440;

        chunk.beginTick = tick;
        chunk.endTick = chunk.beginTick + duration;

        uint32_t note_duration = static_cast<uint32_t>(duration / notes.size());
        uint32_t note_time = tick + (tick > 0 ? note_duration : 0);

        for (int n : notes) {
            auto noteOn = Event(Event::Opcode::NoteOn);
            noteOn.setNote(n);
            noteOn.setVelocityFraction(0.8f);
            chunk.events.insert({ note_time, noteOn });
            note_time += note_duration;
            auto noteOff = noteOn;
            noteOff.setOpcode(Event::Opcode::NoteOff);
            chunk.events.insert({ note_time, noteOff });
        }
    };

    Chunk chunk;
    makeChunk(chunk, 0, 0);
    m_midiStream->lastTick = chunk.endTick;
    m_midiStream->initData.chunks.insert({ chunk.beginTick, std::move(chunk) });

    m_midiStream->isStreamingAllowed = true;
    m_midiStream->request.onReceive(this, [this, makeChunk](tick_t tick) {
        static int pitch = -11;
        ++pitch;
        if (pitch > 11) {
            pitch = -10;
        }

        if (tick > 20000) {
            m_midiStream->stream.close();
            return;
        }

        Chunk chunk;
        makeChunk(chunk, tick, pitch);

        m_midiStream->stream.send(chunk);
    });
}
