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

#include "playbacktypes.h"

using namespace mu::playback;
using namespace mu::midi;
using namespace mu::notation;
using namespace mu::async;
using namespace mu::audio;
using namespace mu::actions;

static const ActionCode METRONOME_CODE("metronome");
static const ActionCode MIDI_ON_CODE("midi-on");
static const ActionCode COUNT_IN_CODE("countin");
static const ActionCode PAN_CODE("pan");
static const ActionCode REPEAT_CODE("repeat");

uint64_t secondsToMilliseconds(float seconds)
{
    return seconds * 1000;
}

void PlaybackController::init()
{
    dispatcher()->reg(this, "play", this, &PlaybackController::togglePlay);
    dispatcher()->reg(this, "rewind", this, &PlaybackController::rewind);
    dispatcher()->reg(this, "loop", this, &PlaybackController::loopPlayback);
    dispatcher()->reg(this, "loop-in", this, &PlaybackController::setLoopInPosition);
    dispatcher()->reg(this, "loop-out", this, &PlaybackController::setLoopOutPosition);

    dispatcher()->reg(this, REPEAT_CODE, this, &PlaybackController::togglePlayRepeats);
    dispatcher()->reg(this, PAN_CODE, this, &PlaybackController::toggleAutomaticallyPan);
    dispatcher()->reg(this, METRONOME_CODE, this, &PlaybackController::toggleMetronome);
    dispatcher()->reg(this, MIDI_ON_CODE, this, &PlaybackController::toggleMidiInput);
    dispatcher()->reg(this, COUNT_IN_CODE, this, &PlaybackController::toggleCountIn);

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
                float seconds = sequencer()->playbackPositionInSeconds();
                int ticks = m_notation->playback()->secToTick(seconds);
                m_tickPlayed.set(ticks);
            }
        });
        break;

    case STEPPED:
        sequencer()->midiTickPlayed(MIDI_TRACK).onReceive(this, [this](midi::tick_t tick) {
            m_tickPlayed.set(tick);
        });
        break;
    }

    m_needRewindBeforePlay = true;
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
    return m_tickPlayed.ch;
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

    if (m_needRewindBeforePlay) {
        seek(tick.val);
    } else {
        m_needRewindBeforePlay = true;
    }

    sequencer()->play();
}

void PlaybackController::rewind(const ActionData& args)
{
    if (args.count() == 0) {
        sequencer()->rewind();
        return;
    }

    uint64_t msec = args.arg<uint64_t>(0);
    sequencer()->seek(msec);
    m_needRewindBeforePlay = false;
}

void PlaybackController::seek(int tick)
{
    IF_ASSERT_FAILED(playback()) {
        return;
    }

    uint64_t milliseconds = secondsToMilliseconds(playback()->tickToSec(tick));
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
    notifyActionEnabledChanged(REPEAT_CODE);
}

void PlaybackController::toggleAutomaticallyPan()
{
    bool panEnabled = configuration()->isAutomaticallyPanEnabled();
    configuration()->setIsAutomaticallyPanEnabled(!panEnabled);
    notifyActionEnabledChanged(PAN_CODE);
}

void PlaybackController::toggleMetronome()
{
    bool metronomeEnabled = configuration()->isMetronomeEnabled();
    configuration()->setIsMetronomeEnabled(!metronomeEnabled);
    notifyActionEnabledChanged(METRONOME_CODE);
}

void PlaybackController::toggleMidiInput()
{
    bool midiInputEnabled = configuration()->isMidiInputEnabled();
    configuration()->setIsMidiInputEnabled(!midiInputEnabled);
    notifyActionEnabledChanged(MIDI_ON_CODE);
}

void PlaybackController::toggleCountIn()
{
    bool countInEnabled = configuration()->isCountInEnabled();
    configuration()->setIsCountInEnabled(!countInEnabled);
    notifyActionEnabledChanged(COUNT_IN_CODE);
}

void PlaybackController::loopPlayback()
{
    NOT_IMPLEMENTED;
}

void PlaybackController::setLoopInPosition()
{
    NOT_IMPLEMENTED;
}

void PlaybackController::setLoopOutPosition()
{
    NOT_IMPLEMENTED;
}

void PlaybackController::notifyActionEnabledChanged(const ActionCode& actionCode)
{
    m_actionEnabledChanged.send(actionCode);
}

bool PlaybackController::isActionEnabled(const ActionCode& actionCode) const
{
    QMap<std::string, bool> isEnabled {
        { MIDI_ON_CODE, configuration()->isMidiInputEnabled() },
        { REPEAT_CODE, configuration()->isPlayRepeatsEnabled() },
        { PAN_CODE, configuration()->isAutomaticallyPanEnabled() },
        { METRONOME_CODE, configuration()->isMetronomeEnabled() },
        { COUNT_IN_CODE, configuration()->isCountInEnabled() }
    };

    return isEnabled[actionCode];
}

Channel<ActionCode> PlaybackController::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

QTime PlaybackController::totalPlayTime() const
{
    return playback() ? playback()->totalPlayTime() : QTime(0, 0, 0, 0);
}

Tempo PlaybackController::currentTempo() const
{
    return playback() ? playback()->tempo(m_tickPlayed.val) : Tempo();
}

MeasureBeat PlaybackController::currentMeasureBeat() const
{
    return playback() ? playback()->measureBeat(m_tickPlayed.val) : MeasureBeat();
}

uint64_t PlaybackController::measureBeatToMilliseconds(const MeasureBeat& measureBeat) const
{
    if (!playback()) {
        return 0;
    }

    int tick = playback()->measureBeatToTick(measureBeat);
    uint64_t milliseconds = secondsToMilliseconds(playback()->tickToSec(tick));

    return milliseconds;
}
