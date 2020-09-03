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

#include "log.h"

using namespace mu::audio;
using namespace mu::midi;

AudioEngineDevTools::AudioEngineDevTools(QObject* parent)
    : QObject(parent)
{
}

void AudioEngineDevTools::playSine()
{
    if (!m_sineSource) {
        m_sineSource = std::make_shared<SineSource>();
    }
    m_sineHandle = audioEngine()->play(m_sineSource);
}

void AudioEngineDevTools::stopSine()
{
    audioEngine()->stop(m_sineHandle);
}

void AudioEngineDevTools::playSourceMidi()
{
    if (!m_midiSource) {
        m_midiSource = std::make_shared<MidiSource>();
    }

    if (!m_midiStream) {
        makeArpeggio();
        m_midiSource->loadMIDI(m_midiStream);
    }

    m_midiHandel = audioEngine()->play(m_midiSource);
}

void AudioEngineDevTools::stopSourceMidi()
{
    audioEngine()->stop(m_midiHandel);
}

void AudioEngineDevTools::playPlayerMidi()
{
    makeArpeggio();

    player()->setMidiStream(m_midiStream);
    player()->play();
}

void AudioEngineDevTools::stopPlayerMidi()
{
    player()->stop();
}

void AudioEngineDevTools::playNotation()
{
    auto notation = globalContext()->currentNotation();
    if (!notation) {
        LOGE() << "no notation";
        return;
    }

    auto stream = notation->playback()->midiStream();
    player()->setMidiStream(stream);
    player()->play();
}

void AudioEngineDevTools::stopNotation()
{
    player()->stop();
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

                         uint32_t note_duration = duration / notes.size();
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
