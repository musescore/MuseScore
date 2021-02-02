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

using namespace mu::playback;
using namespace mu::midi;
using namespace mu::notation;
using namespace mu::async;
using namespace mu::audio;

void PlaybackController::init()
{
    dispatcher()->reg(this, "play", this, &PlaybackController::togglePlay);
    dispatcher()->reg(this, "rewind", this, &PlaybackController::rewindToStart);
    dispatcher()->reg(this, "loop", this, &PlaybackController::loopPlayback);
    dispatcher()->reg(this, "repeat", this, &PlaybackController::togglePlayRepeats);
    dispatcher()->reg(this, "pan", this, &PlaybackController::toggleAutomaticallyPan);
    dispatcher()->reg(this, "metronome", this, &PlaybackController::toggleMetronome);
    dispatcher()->reg(this, "midi-on", this, &PlaybackController::toggleMidiInput);
    dispatcher()->reg(this, "countin", this, &PlaybackController::toggleCountIn);
    dispatcher()->reg(this, "loop-in", this, &PlaybackController::setLoopInPosition);
    dispatcher()->reg(this, "loop-out", this, &PlaybackController::setLoopOutPosition);

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
                auto seconds = sequencer()->playbackPositionInSeconds();
                auto ticks = m_notation->playback()->secToTick(seconds);
                m_tickPlayed.send(ticks);
            }
        });
        break;

    case STEPPED:
        sequencer()->midiTickPlayed(MIDI_TRACK).onReceive(this, [this](midi::tick_t tick) {
            LOGI() << "midiTickPlayed tick: " << tick;
            m_tickPlayed.send(tick);
        });
        break;
    }
}

bool PlaybackController::isPlayAllowed() const
{
    return m_notation != nullptr;
}

Notification PlaybackController::isPlayAllowedChanged() const
{
    return m_isPlayAllowedChanged;
}

bool PlaybackController::isPlaying() const
{
    return sequencer()->status() == ISequencer::PLAYING;
}

bool PlaybackController::isPaused() const
{
    return sequencer()->status() == ISequencer::PAUSED;
}

Notification PlaybackController::isPlayingChanged() const
{
    return m_isPlayingChanged;
}

float PlaybackController::playbackPositionInSeconds() const
{
    return sequencer()->playbackPositionInSeconds();
}

Channel<uint32_t> PlaybackController::midiTickPlayed() const
{
    return m_tickPlayed;
}

void PlaybackController::playElementOnClick(const notation::Element* element)
{
    if (!configuration()->isPlayElementOnClick()) {
        return;
    }

    IF_ASSERT_FAILED(element) {
        return;
    }

    IF_ASSERT_FAILED(playback()) {
        return;
    }

    if (element->isHarmony() && !configuration()->isPlayHarmonyOnClick()) {
        return;
    }

    MidiData midiData = playback()->playElementMidiData(element);

    sequencer()->instantlyPlayMidi(midiData);
}

INotationPlaybackPtr PlaybackController::playback() const
{
    return m_notation ? m_notation->playback() : nullptr;
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
        pause();
    } else if (isPaused()) {
        resume();
    } else {
        play();
    }
}

void PlaybackController::play()
{
    IF_ASSERT_FAILED(playback()) {
        return;
    }

    auto stream = playback()->midiStream();
    sequencer()->setMIDITrack(MIDI_TRACK, stream);

    RetVal<int> tick = playback()->playPositionTick();
    if (!tick.ret) {
        LOGE() << "unable play, err: " << tick.ret.toString();
        return;
    }

    seek(tick.val);
    sequencer()->play();
}

void PlaybackController::rewindToStart()
{
    sequencer()->rewind();
}

void PlaybackController::seek(int tick)
{
    IF_ASSERT_FAILED(playback()) {
        return;
    }

    uint64_t milliseconds = playback()->tickToSec(tick) * 1000;
    sequencer()->seek(milliseconds);
}

void PlaybackController::pause()
{
    sequencer()->pause();
}

void PlaybackController::resume()
{
    sequencer()->play();
}

void PlaybackController::togglePlayRepeats()
{
    bool playRepeatsEnabled = configuration()->isPlayRepeatsEnabled();
    configuration()->setIsPlayRepeatsEnabled(!playRepeatsEnabled);
}

void PlaybackController::toggleAutomaticallyPan()
{
    bool panEnabled = configuration()->isAutomaticallyPanEnabled();
    configuration()->setIsAutomaticallyPanEnabled(!panEnabled);
}

void PlaybackController::toggleMetronome()
{
    bool metronomeEnabled = configuration()->isMetronomeEnabled();
    configuration()->setIsMetronomeEnabled(!metronomeEnabled);
}

void PlaybackController::toggleMidiInput()
{
    bool midiInputEnabled = configuration()->isMidiInputEnabled();
    configuration()->setIsMidiInputEnabled(!midiInputEnabled);
}

void PlaybackController::toggleCountIn()
{
    bool countInEnabled = configuration()->isCountInEnabled();
    configuration()->setIsCountInEnabled(!countInEnabled);
}

void PlaybackController::loopPlayback()
{
    NOT_SUPPORTED;
}

void PlaybackController::setLoopInPosition()
{
    NOT_IMPLEMENTED;
}

void PlaybackController::setLoopOutPosition()
{
    NOT_IMPLEMENTED;
}

bool PlaybackController::isActionEnabled(const std::string &actionCode) const
{
    QMap<std::string, bool> isEnabled {
        { "midi-on", configuration()->isMidiInputEnabled() },
        { "repeat", configuration()->isPlayRepeatsEnabled() },
        { "pan", configuration()->isAutomaticallyPanEnabled() },
        { "metronome", configuration()->isMetronomeEnabled() },
        { "countin", configuration()->isCountInEnabled() }
    };

    return isEnabled[actionCode];
}
