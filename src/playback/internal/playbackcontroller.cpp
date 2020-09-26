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
#include "playbackcontroller.h"

#include "log.h"

using namespace mu;
using namespace mu::playback;
using namespace mu::midi;

PlaybackController::PlaybackController()
    : m_cursorType(STEPPED)
{
}

void PlaybackController::init()
{
    dispatcher()->reg(this, "play", this, &PlaybackController::togglePlay);

    onNotationChanged();
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });

    sequencer()->statusChanged().onReceive(this, [this](const audio::ISequencer::Status&) {
        m_isPlayingChanged.notify();
    });
    sequencer()->initMIDITrack(MIDI_TRACK);

    switch (m_cursorType) {
    case SMOOTH:
        sequencer()->positionChanged().onNotify(this, [this]() {
            if (m_notation) {
                auto seconds = sequencer()->playbackPosition();
                auto ticks = m_notation->playback()->secToTick(seconds);
                m_tickPlayed.send(ticks);
            }
        });
        break;

    case STEPPED:
        sequencer()->midiTickPlayed(MIDI_TRACK).onReceive(this, [this](midi::tick_t tick) {
            m_tickPlayed.send(tick);
        });
        break;
    }
}

bool PlaybackController::isPlayAllowed() const
{
    return m_notation != nullptr;
}

async::Notification PlaybackController::isPlayAllowedChanged() const
{
    return m_isPlayAllowedChanged;
}

bool PlaybackController::isPlaying() const
{
    return sequencer()->status() == audio::ISequencer::PLAYING;
}

async::Notification PlaybackController::isPlayingChanged() const
{
    return m_isPlayingChanged;
}

float PlaybackController::playbackPosition() const
{
    return sequencer()->playbackPosition();
}

async::Channel<uint32_t> PlaybackController::midiTickPlayed() const
{
    return m_tickPlayed;
}

void PlaybackController::playElementOnClick(const notation::Element* e)
{
    if (!configuration()->isPlayElementOnClick()) {
        return;
    }

    IF_ASSERT_FAILED(e) {
        return;
    }

    IF_ASSERT_FAILED(m_notation) {
        return;
    }

    if (e->isHarmony() && !configuration()->isPlayHarmonyOnClick()) {
        return;
    }

    midi::MidiData midiData = m_notation->playback()->playElementMidiData(e);

    sequencer()->instantlyPlayMidi(midiData);
}

void PlaybackController::onNotationChanged()
{
    if (m_notation) {
        m_notation->playback()->playPositionTickChanged().resetOnReceive(this);
    }

    m_notation = globalContext()->currentNotation();
    if (m_notation) {
        m_notation->playback()->playPositionTickChanged().onReceive(this, [this](int tick) {
            seek(tick);
        });
    }

    m_isPlayAllowedChanged.notify();
}

void PlaybackController::togglePlay()
{
    if (!isPlayAllowed()) {
        LOGW() << "playback not allowed";
        return;
    }

    if (isPlaying()) {
        stop();
    } else {
        play();
    }
}

void PlaybackController::play()
{
    IF_ASSERT_FAILED(m_notation) {
        return;
    }

    auto stream = m_notation->playback()->midiStream();
    sequencer()->setMIDITrack(MIDI_TRACK, stream);

    RetVal<int> tick = m_notation->playback()->playPositionTick();
    if (!tick.ret) {
        LOGE() << "unable play, err: " << tick.ret.toString();
        return;
    }

    seek(tick.val);
    sequencer()->play();
}

void PlaybackController::seek(int tick)
{
    IF_ASSERT_FAILED(m_notation) {
        return;
    }

    unsigned long miliseconds = m_notation->playback()->tickToSec(tick) * 1000;
    sequencer()->seek(miliseconds);
}

void PlaybackController::stop()
{
    sequencer()->stop();
}
