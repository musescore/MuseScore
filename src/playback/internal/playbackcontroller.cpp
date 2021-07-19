/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
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
static const ActionCode STOP_CODE("stop");
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
    dispatcher()->reg(this, STOP_CODE, this, &PlaybackController::pause);
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

    m_needRewindBeforePlay = true;
}

int PlaybackController::currentTick() const
{
    return m_currentTick;
}

bool PlaybackController::isPlayAllowed() const
{
    return m_notation != nullptr && !m_notation->interaction()->isTextEditingStarted();
}

Notification PlaybackController::isPlayAllowedChanged() const
{
    return m_isPlayAllowedChanged;
}

bool PlaybackController::isPlaying() const
{
    return m_isPlaying;
}

bool PlaybackController::isPaused() const
{
    return !m_isPlaying;
}

bool PlaybackController::isLoopVisible() const
{
    return notationPlayback() ? notationPlayback()->loopBoundaries().val.visible : false;
}

bool PlaybackController::isPlaybackLooped() const
{
    return notationPlayback() ? !notationPlayback()->loopBoundaries().val.isNull() : false;
}

Notification PlaybackController::isPlayingChanged() const
{
    return m_isPlayingChanged;
}

void PlaybackController::reset()
{
    stop();
}

void PlaybackController::seek(const midi::tick_t tick)
{
    IF_ASSERT_FAILED(notationPlayback() && playback()) {
        return;
    }

    msecs_t milliseconds = secondsToMilliseconds(notationPlayback()->tickToSec(tick));
    playback()->player()->seek(m_currentSequenceId, std::move(milliseconds));
}

void PlaybackController::seek(const audio::msecs_t msecs)
{
    IF_ASSERT_FAILED(notationPlayback() && playback()) {
        return;
    }

    playback()->player()->seek(m_currentSequenceId, msecs);
}

Notification PlaybackController::playbackPositionChanged() const
{
    return m_playbackPositionChanged;
}

Channel<uint32_t> PlaybackController::midiTickPlayed() const
{
    return m_tickPlayed;
}

float PlaybackController::playbackPositionInSeconds() const
{
    return notationPlayback()->tickToSec(m_currentTick);
}

TrackSequenceId PlaybackController::currentTrackSequenceId() const
{
    return m_currentSequenceId;
}

Notification PlaybackController::currentTrackSequenceIdChanged() const
{
    return m_currentSequenceIdChanged;
}

void PlaybackController::playElement(const notation::Element* element)
{
    if (!configuration()->playNotesWhenEditing()) {
        return;
    }

    IF_ASSERT_FAILED(element) {
        return;
    }

    IF_ASSERT_FAILED(notationPlayback()) {
        return;
    }

    if (element->isHarmony() && !configuration()->playHarmonyWhenEditing()) {
        return;
    }

    notationPlayback()->playElementMidiData(element);
}

INotationPlaybackPtr PlaybackController::notationPlayback() const
{
    return m_notation ? m_notation->playback() : nullptr;
}

INotationSelectionPtr PlaybackController::selection() const
{
    return m_notation ? m_notation->interaction()->selection() : nullptr;
}

void PlaybackController::onNotationChanged()
{
    playback()->player()->stop(m_currentSequenceId);
    playback()->player()->playbackPositionMsecs().resetOnReceive(this);

    m_notation = globalContext()->currentNotation();
    if (!m_notation) {
        return;
    }

    io::path notationKey = m_notation->metaInfo().filePath;

    auto search = m_sequenceIdMap.find(notationKey.toStdString());

    if (search == m_sequenceIdMap.end()) {
        playback()->addSequence().onResolve(this, [this, notationKey](const TrackSequenceId& sequenceId) {
            m_sequenceIdMap.emplace(notationKey.toStdString(), sequenceId);
            setCurrentSequence(sequenceId);
        });
    } else {
        setCurrentSequence(search->second);
    }

    m_notation->playback()->loopBoundaries().ch.onReceive(this, [this](const LoopBoundaries& boundaries) {
        setLoop(boundaries);
    });

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
    IF_ASSERT_FAILED(notationPlayback() && playback()) {
        return;
    }

    if (m_needRewindBeforePlay) {
        seek(m_currentTick);
    } else {
        m_needRewindBeforePlay = true;
    }

    playback()->player()->play(m_currentSequenceId);

    m_isPlaying = true;
    m_isPlayingChanged.notify();
}

void PlaybackController::rewind(const ActionData& args)
{
    IF_ASSERT_FAILED(playback()) {
        return;
    }

    if (args.count() == 0) {
        playback()->player()->seek(m_currentSequenceId, 0);
        return;
    }

    msecs_t msec = args.arg<msecs_t>(0);
    playback()->player()->seek(m_currentSequenceId, msec);
    m_needRewindBeforePlay = false;
}

void PlaybackController::pause()
{
    IF_ASSERT_FAILED(playback()) {
        return;
    }

    playback()->player()->pause(m_currentSequenceId);

    m_isPlaying = false;
    m_isPlayingChanged.notify();
}

void PlaybackController::stop()
{
    IF_ASSERT_FAILED(playback()) {
        return;
    }

    playback()->player()->stop(m_currentSequenceId);
    m_isPlaying = false;
    m_isPlayingChanged.notify();
}

void PlaybackController::resume()
{
    IF_ASSERT_FAILED(playback()) {
        return;
    }

    playback()->player()->resume(m_currentSequenceId);

    m_isPlaying = true;
    m_isPlayingChanged.notify();
}

void PlaybackController::togglePlayRepeats()
{
    bool playRepeatsEnabled = notationConfiguration()->isPlayRepeatsEnabled();
    notationConfiguration()->setIsPlayRepeatsEnabled(!playRepeatsEnabled);
    notifyActionCheckedChanged(REPEAT_CODE);
}

void PlaybackController::toggleAutomaticallyPan()
{
    bool panEnabled = notationConfiguration()->isAutomaticallyPanEnabled();
    notationConfiguration()->setIsAutomaticallyPanEnabled(!panEnabled);
    notifyActionCheckedChanged(PAN_CODE);
}

void PlaybackController::toggleMetronome()
{
    bool metronomeEnabled = notationConfiguration()->isMetronomeEnabled();
    notationConfiguration()->setIsMetronomeEnabled(!metronomeEnabled);
    notifyActionCheckedChanged(METRONOME_CODE);
}

void PlaybackController::toggleMidiInput()
{
    bool midiInputEnabled = notationConfiguration()->isMidiInputEnabled();
    notationConfiguration()->setIsMidiInputEnabled(!midiInputEnabled);
    notifyActionCheckedChanged(MIDI_ON_CODE);
}

void PlaybackController::toggleCountIn()
{
    bool countInEnabled = notationConfiguration()->isCountInEnabled();
    notationConfiguration()->setIsCountInEnabled(!countInEnabled);
    notifyActionCheckedChanged(COUNT_IN_CODE);
}

void PlaybackController::toggleLoopPlayback()
{
    if (isLoopVisible()) {
        hideLoop();
        return;
    }

    if (isPlaybackLooped() && !selection()->isRange()) {
        showLoop();
        return;
    }

    int loopInTick = INotationPlayback::FirstScoreTick;
    int loopOutTick = INotationPlayback::LastScoreTick;

    if (!selection()->isNone()) {
        loopInTick = selection()->range()->startTick().ticks();
        loopOutTick = selection()->range()->endTick().ticks();
    }

    addLoopBoundaryToTick(LoopBoundaryType::LoopIn, loopInTick);
    addLoopBoundaryToTick(LoopBoundaryType::LoopOut, loopOutTick);
}

void PlaybackController::addLoopBoundary(LoopBoundaryType type)
{
    if (isPlaying()) {
        addLoopBoundaryToTick(type, currentTick());
    } else {
        addLoopBoundaryToTick(type, INotationPlayback::SelectedNoteTick);
    }
}

void PlaybackController::addLoopBoundaryToTick(LoopBoundaryType type, int tick)
{
    if (notationPlayback()) {
        notationPlayback()->addLoopBoundary(type, tick);
        showLoop();
    }
}

void PlaybackController::setLoop(const LoopBoundaries& boundaries)
{
    IF_ASSERT_FAILED(playback()) {
        return;
    }

    if (!boundaries.visible) {
        hideLoop();
        return;
    }

    msecs_t fromMilliseconds = secondsToMilliseconds(notationPlayback()->tickToSec(boundaries.loopInTick));
    msecs_t toMilliseconds = secondsToMilliseconds(notationPlayback()->tickToSec(boundaries.loopOutTick));

    playback()->player()->setLoop(m_currentSequenceId, fromMilliseconds, toMilliseconds);
    showLoop();

    notifyActionCheckedChanged(LOOP_CODE);
}

void PlaybackController::showLoop()
{
    if (notationPlayback()) {
        notationPlayback()->setLoopBoundariesVisible(true);
    }
}

void PlaybackController::hideLoop()
{
    IF_ASSERT_FAILED(notationPlayback() && playback()) {
        return;
    }

    playback()->player()->resetLoop(m_currentSequenceId);
    notationPlayback()->setLoopBoundariesVisible(false);
    notifyActionCheckedChanged(LOOP_CODE);
}

void PlaybackController::notifyActionCheckedChanged(const ActionCode& actionCode)
{
    m_actionCheckedChanged.send(actionCode);
}

void PlaybackController::setCurrentTick(const tick_t tick)
{
    m_currentTick = tick;
    m_playbackPositionChanged.notify();
}

void PlaybackController::addTrack(const notation::INotationPlayback::InstrumentTrackId& id)
{
    IF_ASSERT_FAILED(notationPlayback() && playback()) {
        return;
    }

    playback()->tracks()->addTrack(m_currentSequenceId, id, notationPlayback()->instrumentMidiData(id), AudioOutputParams())
    .onResolve(this, [this, id](const TrackId trackId) {
        m_trackIdMap.insert({ id, trackId });
    })
    .onReject(this, [](int code, const std::string& msg) {
        LOGE() << "can't add a new track, code: [" << code << "] " << msg;
    });
}

void PlaybackController::removeTrack(const notation::INotationPlayback::InstrumentTrackId& id)
{
    IF_ASSERT_FAILED(notationPlayback() && playback()) {
        return;
    }

    auto search = m_trackIdMap.find(id);

    if (search == m_trackIdMap.end()) {
        return;
    }

    playback()->tracks()->removeTrack(m_currentSequenceId, search->second);
}

void PlaybackController::setCurrentSequence(const TrackSequenceId sequenceId)
{
    PlaybackCursorType cursorType = configuration()->cursorType();

    m_currentSequenceId = sequenceId;
    m_currentSequenceIdChanged.notify();

    m_trackIdMap.clear();
    playback()->tracks()->removeAllTracks(m_currentSequenceId);

    if (!notationPlayback()) {
        return;
    }

    for (const INotationPlayback::InstrumentTrackId& id : notationPlayback()->instrumentTrackIdList()) {
        addTrack(id);
    }

    notationPlayback()->instrumentTrackAdded().onReceive(this, [this](INotationPlayback::InstrumentTrackId&& id) {
        addTrack(id);
    });

    notationPlayback()->instrumentTrackRemoved().onReceive(this, [this](INotationPlayback::InstrumentTrackId&& id) {
        removeTrack(id);
    });

    playback()->player()->playbackPositionMsecs().onReceive(this, [this, cursor = std::move(cursorType)]
                                                                (const TrackSequenceId id, const audio::msecs_t& msecs) {
        if (m_currentSequenceId != id) {
            return;
        }

        tick_t tick = notationPlayback()->secToTick(msecs / 1000.);

        if (cursor == PlaybackCursorType::STEPPED) {
            MeasureBeat newBeat = notationPlayback()->beat(tick);

            if (currentBeat().beatIndex == newBeat.beatIndex) {
                setCurrentTick(tick);
                return;
            }
        }

        setCurrentTick(tick);
        m_tickPlayed.send(std::move(tick));
    });

    playback()->player()->playbackStatusChanged().onReceive(this, [this](const TrackSequenceId id, const PlaybackStatus status) {
        if (m_currentSequenceId != id) {
            return;
        }

        m_currentPlaybackStatus = status;
    });
}

bool PlaybackController::actionChecked(const ActionCode& actionCode) const
{
    QMap<std::string, bool> isChecked {
        { LOOP_CODE, isLoopVisible() },
        { MIDI_ON_CODE, notationConfiguration()->isMidiInputEnabled() },
        { REPEAT_CODE, notationConfiguration()->isPlayRepeatsEnabled() },
        { PAN_CODE, notationConfiguration()->isAutomaticallyPanEnabled() },
        { METRONOME_CODE, notationConfiguration()->isMetronomeEnabled() },
        { COUNT_IN_CODE, notationConfiguration()->isCountInEnabled() }
    };

    return isChecked[actionCode];
}

Channel<ActionCode> PlaybackController::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}

QTime PlaybackController::totalPlayTime() const
{
    return notationPlayback() ? notationPlayback()->totalPlayTime() : ZERO_TIME;
}

Tempo PlaybackController::currentTempo() const
{
    return notationPlayback() ? notationPlayback()->tempo(currentTick()) : Tempo();
}

MeasureBeat PlaybackController::currentBeat() const
{
    return notationPlayback() ? notationPlayback()->beat(currentTick()) : MeasureBeat();
}

msecs_t PlaybackController::beatToMilliseconds(int measureIndex, int beatIndex) const
{
    if (!notationPlayback()) {
        return 0;
    }

    int tick = notationPlayback()->beatToTick(measureIndex, beatIndex);
    msecs_t milliseconds = secondsToMilliseconds(notationPlayback()->tickToSec(tick));

    return milliseconds;
}
