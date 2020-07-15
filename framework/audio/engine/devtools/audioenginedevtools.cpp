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

using namespace mu::audio::engine;
using namespace mu::audio::midi;

AudioEngineDevTools::AudioEngineDevTools(QObject* parent)
    : QObject(parent)
{
}

void AudioEngineDevTools::playSine()
{
    if (!m_sineStream) {
        m_sineStream = std::make_shared<SineStream>();
    }
    m_sineHandle = audioEngine()->play(m_sineStream);
}

void AudioEngineDevTools::stopSine()
{
    audioEngine()->stop(m_sineHandle);
}

void AudioEngineDevTools::playSourceMidi()
{
    if (!m_midiStream) {
        m_midiStream = std::make_shared<MidiStream>();
    }

    if (!m_midiData) {
        m_midiData = makeArpeggio();
        m_midiStream->init(audioEngine()->sampleRate());
        m_midiStream->loadMIDI(m_midiData);
    }

    m_midiHandel = audioEngine()->play(m_midiStream);
}

void AudioEngineDevTools::stopSourceMidi()
{
    audioEngine()->stop(m_midiHandel);
}

void AudioEngineDevTools::playPlayerMidi()
{
    if (!m_midiData) {
        m_midiData = makeArpeggio();
    }

    player()->setMidiData(m_midiData);
    player()->play();
}

void AudioEngineDevTools::stopPlayerMidi()
{
    player()->stop();
}

std::shared_ptr<MidiData> AudioEngineDevTools::makeArpeggio() const
{
    /* notes of the arpeggio */
    static std::vector<int> notes = { 60, 64, 67, 72, 76, 79, 84, 79, 76, 72, 67, 64 };
    static uint64_t duration = 4440;

    uint64_t note_duration = duration / notes.size();
    uint64_t note_time = 0;

    Channel ch;
    for (int n : notes) {
        ch.events.push_back(Event(note_time, ME_NOTEON, n, 100));
        note_time += note_duration;
        ch.events.push_back(Event(note_time, ME_NOTEOFF, n, 100));
    }

    Track t;
    t.channels.push_back(ch);

    std::shared_ptr<MidiData> data = std::make_shared<MidiData>();
    data->tracks.push_back(t);

    return data;
}
