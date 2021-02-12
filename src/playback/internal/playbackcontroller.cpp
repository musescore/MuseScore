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

static const ActionCode PLAY_CODE("play");
static const ActionCode REWIND_CODE("rewind");
static const ActionCode LOOP_CODE("loop");
static const ActionCode LOOP_IN_CODE("loop-in");
static const ActionCode LOOP_OUT_CODE("loop-out");
static const ActionCode METRONOME_CODE("metronome");
static const ActionCode MIDI_ON_CODE("midi-on");
static const ActionCode COUNT_IN_CODE("countin");
static const ActionCode PAN_CODE("pan");
static const ActionCode REPEAT_CODE("repeat");

void PlaybackController::init()
{
    dispatcher()->reg(this, PLAY_CODE, this, &PlaybackController::togglePlay);
    dispatcher()->reg(this, REWIND_CODE, this, &PlaybackController::rewind);
    dispatcher()->reg(this, LOOP_CODE, this, &PlaybackController::toggleLoopPlayback);
    dispatcher()->reg(this, LOOP_IN_CODE, [this]() { addLoopBoundary(LoopBoundaryType::LoopIn); });
    dispatcher()->reg(this, LOOP_OUT_CODE, [this]() { addLoopBoundary(LoopBoundaryType::LoopOut); });
    dispatcher()->reg(this, REPEAT_CODE, this, &PlaybackController::togglePlayRepeats);
    dispatcher()->reg(this, PAN_CODE, this, &PlaybackController::toggleAutomaticallyPan);
    dispatcher()->reg(this, METRONOME_CODE, this, &PlaybackController::toggleMetronome);
    dispatcher()->reg(this, MIDI_ON_CODE, this, &PlaybackController::toggleMidiInput);
    dispatcher()->reg(this, COUNT_IN_CODE, this, &PlaybackController::toggleCountIn);

    onNotationChanged();
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });

    sequencer()->statusChanged().onReceive(this, [this](const ISequencer::Status&) {
        m_isPlayingChanged.notify();
    });

    sequencer()->initMIDITrack(MIDI_TRACK);

    sequencer()->positionChanged().onNotify(this, [this]() {
        if (configuration()->cursorType() == PlaybackCursorType::SMOOTH) {
            float seconds = sequencer()->playbackPositionInSeconds();
            int tick = m_notation->playback()->secToTick(seconds);
            m_tickPlayed.set(tick);
        }

        m_playbackPositionChanged.notify();
    });

    if (configuration()->cursorType() == PlaybackCursorType::STEPPED) {
        sequencer()->midiTickPlayed(MIDI_TRACK).onReceive(this, [this](midi::tick_t tick) {
            m_tickPlayed.set(tick);
        });
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

Notification PlaybackController::playbackPositionChanged() const
{
    return m_playbackPositionChanged;
}

Channel<uint32_t> PlaybackController::midiTickPlayed() const
{
    return m_tickPlayed.ch;
}

float PlaybackController::playbackPositionInSeconds() const
{
    return sequencer()->playbackPositionInSeconds();
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

INotationSelectionPtr PlaybackController::selection() const
{
    return m_notation ? m_notation->interaction()->selection() : nullptr;
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

        m_notation->playback()->loopBoundariesChanged().onReceive(this, [this](const LoopBoundaries& boundary) {
            setLoop(boundary);
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
    bool playRepeatsEnabled = notationConfiguration()->isPlayRepeatsEnabled();
    notationConfiguration()->setIsPlayRepeatsEnabled(!playRepeatsEnabled);
    notifyActionEnabledChanged(REPEAT_CODE);
}

void PlaybackController::toggleAutomaticallyPan()
{
    bool panEnabled = notationConfiguration()->isAutomaticallyPanEnabled();
    notationConfiguration()->setIsAutomaticallyPanEnabled(!panEnabled);
    notifyActionEnabledChanged(PAN_CODE);
}

void PlaybackController::toggleMetronome()
{
    bool metronomeEnabled = notationConfiguration()->isMetronomeEnabled();
    notationConfiguration()->setIsMetronomeEnabled(!metronomeEnabled);
    notifyActionEnabledChanged(METRONOME_CODE);
}

void PlaybackController::toggleMidiInput()
{
    bool midiInputEnabled = notationConfiguration()->isMidiInputEnabled();
    notationConfiguration()->setIsMidiInputEnabled(!midiInputEnabled);
    notifyActionEnabledChanged(MIDI_ON_CODE);
}

void PlaybackController::toggleCountIn()
{
    bool countInEnabled = notationConfiguration()->isCountInEnabled();
    notationConfiguration()->setIsCountInEnabled(!countInEnabled);
    notifyActionEnabledChanged(COUNT_IN_CODE);
}

void PlaybackController::toggleLoopPlayback()
{
    if (m_isPlaybackLooped) {
        unsetLoop();
        return;
    }

    int loopInTick = INotationPlayback::FirstScoreTick;
    int loopOutTick = INotationPlayback::LastScoreTick;

    if (!selection()->isNone()) {
        loopInTick = selection()->range()->startTick().ticks();
        loopOutTick = selection()->range()->endTick().ticks();
    }

    playback()->addLoopBoundary(LoopBoundaryType::LoopIn, loopInTick);
    playback()->addLoopBoundary(LoopBoundaryType::LoopOut, loopOutTick);

    m_isPlaybackLooped = true;
}

void PlaybackController::addLoopBoundary(LoopBoundaryType type)
{
    if (!playback()) {
        return;
    }

    if (isPlaying()) {
        playback()->addLoopBoundary(type, m_tickPlayed.val);
    } else {
        playback()->addLoopBoundary(type, INotationPlayback::SelectedNoteTick);
    }
}

void PlaybackController::setLoop(const LoopBoundaries& boundary)
{
    if (boundary.isNull()) {
        unsetLoop();
        return;
    }

    uint64_t fromMilliseconds = secondsToMilliseconds(playback()->tickToSec(boundary.loopInTick));
    uint64_t toMilliseconds = secondsToMilliseconds(playback()->tickToSec(boundary.loopOutTick));

    sequencer()->setLoop(fromMilliseconds, toMilliseconds);

    m_isPlaybackLooped = true;
    notifyActionEnabledChanged(LOOP_CODE);
}

void PlaybackController::unsetLoop()
{
    if (playback() && m_isPlaybackLooped) {
        m_isPlaybackLooped = false;
        sequencer()->unsetLoop();
        playback()->removeLoopBoundaries();
        notifyActionEnabledChanged(LOOP_CODE);
    }
}

void PlaybackController::notifyActionEnabledChanged(const ActionCode& actionCode)
{
    m_actionEnabledChanged.send(actionCode);
}

bool PlaybackController::isActionEnabled(const ActionCode& actionCode) const
{
    QMap<std::string, bool> isEnabled {
        { LOOP_CODE, m_isPlaybackLooped },
        { MIDI_ON_CODE, notationConfiguration()->isMidiInputEnabled() },
        { REPEAT_CODE, notationConfiguration()->isPlayRepeatsEnabled() },
        { PAN_CODE, notationConfiguration()->isAutomaticallyPanEnabled() },
        { METRONOME_CODE, notationConfiguration()->isMetronomeEnabled() },
        { COUNT_IN_CODE, notationConfiguration()->isCountInEnabled() }
    };

    return isEnabled[actionCode];
}

Channel<ActionCode> PlaybackController::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

QTime PlaybackController::totalPlayTime() const
{
    return playback() ? playback()->totalPlayTime() : ZERO_TIME;
}

Tempo PlaybackController::currentTempo() const
{
    return playback() ? playback()->tempo(m_tickPlayed.val) : Tempo();
}

MeasureBeat PlaybackController::currentBeat() const
{
    return playback() ? playback()->beat(m_tickPlayed.val) : MeasureBeat();
}

uint64_t PlaybackController::beatToMilliseconds(int measureIndex, int beatIndex) const
{
    if (!playback()) {
        return 0;
    }

    int tick = playback()->beatToTick(measureIndex, beatIndex);
    uint64_t milliseconds = secondsToMilliseconds(playback()->tickToSec(tick));

    return milliseconds;
}
