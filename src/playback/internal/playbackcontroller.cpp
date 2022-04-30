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

#include "playbacktypes.h"

#include "containers.h"
#include "defer.h"
#include "log.h"

using namespace mu::playback;
using namespace mu::midi;
using namespace mu::notation;
using namespace mu::async;
using namespace mu::audio;
using namespace mu::actions;
using namespace mu::engraving;

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

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });

    globalContext()->currentProjectChanged().onNotify(this, [this]() {
        if (m_currentSequenceId != -1) {
            resetCurrentSequence();
        }

        playback()->addSequence().onResolve(this, [this](const TrackSequenceId& sequenceId) {
            setupNewCurrentSequence(sequenceId);
        });
    });

    m_totalPlayTimeChanged.onNotify(this, [this]() {
        updateCurrentTempo();
    });

    m_playbackPositionChanged.onNotify(this, [this]() {
        updateCurrentTempo();
    });

    m_needRewindBeforePlay = true;
}

void PlaybackController::updateCurrentTempo()
{
    if (!notationPlayback()) {
        return;
    }

    const Tempo& newTempo = notationPlayback()->tempo(currentTick());

    if (newTempo == m_currentTempo) {
        return;
    }

    m_currentTempo = newTempo;
    m_currentTempoChanged.notify();
}

int PlaybackController::currentTick() const
{
    return m_currentTick;
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
    return m_currentPlaybackStatus == PlaybackStatus::Running;
}

bool PlaybackController::isPaused() const
{
    return m_currentPlaybackStatus == PlaybackStatus::Paused;
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

    playback()->player()->seek(m_currentSequenceId, tickToMsecs(tick));
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
    return notationPlayback() ? notationPlayback()->playedTickToSec(m_currentTick) : 0.0;
}

TrackSequenceId PlaybackController::currentTrackSequenceId() const
{
    return m_currentSequenceId;
}

Notification PlaybackController::currentTrackSequenceIdChanged() const
{
    return m_currentSequenceIdChanged;
}

mu::engraving::InstrumentTrackId PlaybackController::instrumentTrackIdForAudioTrackId(audio::TrackId theTrackId) const
{
    for (auto [instrumentTrackId, audioTrackId] : m_trackIdMap) {
        if (audioTrackId == theTrackId) {
            return instrumentTrackId;
        }
    }

    return {};
}

void PlaybackController::playElement(const notation::EngravingItem* element)
{
    IF_ASSERT_FAILED(element) {
        return;
    }

    IF_ASSERT_FAILED(notationPlayback()) {
        return;
    }

    RetVal<midi::tick_t> tick = notationPlayback()->playPositionTickByElement(element);
    if (!tick.ret) {
        return;
    }

    seek(tick.val);

    if (!configuration()->playNotesWhenEditing()) {
        return;
    }

    notationPlayback()->triggerEventsForItem(element);
}

INotationPlaybackPtr PlaybackController::notationPlayback() const
{
    return m_masterNotation ? m_masterNotation->playback() : nullptr;
}

INotationPartsPtr PlaybackController::masterNotationParts() const
{
    return m_masterNotation ? m_masterNotation->parts() : nullptr;
}

INotationSelectionPtr PlaybackController::selection() const
{
    return m_notation ? m_notation->interaction()->selection() : nullptr;
}

INotationSelectionRangePtr PlaybackController::selectionRange() const
{
    INotationSelectionPtr selection = this->selection();
    return selection ? selection->range() : nullptr;
}

INotationInteractionPtr PlaybackController::interaction() const
{
    return m_notation ? m_notation->interaction() : nullptr;
}

void PlaybackController::onNotationChanged()
{
    m_masterNotation = globalContext()->currentMasterNotation();
    m_notation = globalContext()->currentNotation();

    DEFER {
        m_isPlayAllowedChanged.notify();
        m_totalPlayTimeChanged.notify();
    };

    if (!m_notation || !m_masterNotation) {
        return;
    }

    INotationPartsPtr notationParts = m_notation->parts();

    updateMuteStates();

    NotifyList<const Part*> partList = notationParts->partList();

    partList.onItemAdded(this, [this](const Part*) {
        updateMuteStates();
    });

    partList.onItemChanged(this, [this](const Part*) {
        updateMuteStates();
    });

    notationPlayback()->loopBoundaries().ch.onReceive(this, [this](const LoopBoundaries& boundaries) {
        setLoop(boundaries);
    });

    m_notation->interaction()->selectionChanged().onNotify(this, [this]() {
        onSelectionChanged();
    });
}

void PlaybackController::onSelectionChanged()
{
    static bool isRangeSelection = false;

    INotationSelectionPtr selection = this->selection();
    bool selectionTypeChanged = isRangeSelection && !selection->isRange();
    isRangeSelection = selection->isRange();

    if (!isRangeSelection) {
        if (selectionTypeChanged) {
            setLoop(notationPlayback()->loopBoundaries().val);
            updateMuteStates();
        }

        return;
    }

    msecs_t startMsecs = playbackStartMsecs();

    playback()->player()->resetLoop(m_currentSequenceId);
    playback()->player()->seek(m_currentSequenceId, startMsecs);

    updateMuteStates();
}

void PlaybackController::togglePlay(const actions::ActionData& args)
{
    if (!isPlayAllowed()) {
        LOGW() << "playback not allowed";
        return;
    }

    if (interaction()->isElementEditStarted()) {
        bool endEditElement = args.count() == 1 ? args.arg<bool>(0) : false;
        if (!endEditElement) {
            return;
        }

        interaction()->endEditElement();
    }

    if (isPlaying()) {
        pause();
    } else if (isPaused()) {
        msecs_t endMsecs = playbackEndMsecs();
        msecs_t currentTimeMsecs = tickToMsecs(m_currentTick);

        if (currentTimeMsecs == endMsecs) {
            msecs_t startMsecs = playbackStartMsecs();
            seek(startMsecs);
        }

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
        msecs_t startMsecs = playbackStartMsecs();
        seek(startMsecs);
    } else {
        m_needRewindBeforePlay = true;
    }

    playback()->player()->play(m_currentSequenceId);
    setCurrentPlaybackStatus(PlaybackStatus::Running);
}

void PlaybackController::rewind(const ActionData& args)
{
    IF_ASSERT_FAILED(playback()) {
        return;
    }

    msecs_t startMsecs = playbackStartMsecs();
    msecs_t endMsecs = playbackEndMsecs();
    msecs_t newPosition = !args.empty() ? args.arg<msecs_t>(0) : 0;
    newPosition = std::clamp(newPosition, startMsecs, endMsecs);

    seek(newPosition);
    m_needRewindBeforePlay = false;
}

void PlaybackController::pause()
{
    IF_ASSERT_FAILED(playback()) {
        return;
    }

    playback()->player()->pause(m_currentSequenceId);
    setCurrentPlaybackStatus(PlaybackStatus::Paused);
}

void PlaybackController::stop()
{
    IF_ASSERT_FAILED(playback()) {
        return;
    }

    playback()->player()->stop(m_currentSequenceId);
    setCurrentPlaybackStatus(PlaybackStatus::Stopped);
}

void PlaybackController::resume()
{
    IF_ASSERT_FAILED(playback()) {
        return;
    }

    playback()->player()->resume(m_currentSequenceId);
    setCurrentPlaybackStatus(PlaybackStatus::Running);
}

msecs_t PlaybackController::playbackStartMsecs() const
{
    if (!m_notation) {
        return 0;
    }

    if (selection()->isRange()) {
        return tickToMsecs(selectionRange()->startTick().ticks());
    }

    LoopBoundaries loop = notationPlayback()->loopBoundaries().val;
    if (loop.visible) {
        return tickToMsecs(loop.loopInTick);
    }

    return 0;
}

msecs_t PlaybackController::playbackEndMsecs() const
{
    return notationPlayback() ? notationPlayback()->totalPlayTime() : 0;
}

InstrumentTrackIdSet PlaybackController::instrumentTrackIdSetForRangePlayback() const
{
    std::vector<const Part*> selectedParts = selectionRange()->selectedParts();
    Fraction startTick = selectionRange()->startTick();
    int startTicks = startTick.ticks();

    InstrumentTrackIdSet result;

    for (const Part* part: selectedParts) {
        if (const Instrument* startInstrument = part->instrument(startTick)) {
            result.insert({ part->id(), startInstrument->id().toStdString() });
        }

        for (auto [tick, instrument] : part->instruments()) {
            if (tick > startTicks) {
                result.insert({ part->id(), instrument->id().toStdString() });
            }
        }
    }

    return result;
}

void PlaybackController::setCurrentPlaybackStatus(PlaybackStatus status)
{
    if (m_currentPlaybackStatus == status) {
        return;
    }

    m_currentPlaybackStatus = status;
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

    setTrackActivity(notationPlayback()->metronomeTrackId(), !metronomeEnabled);
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

    int loopInTick = 0;
    int loopOutTick = 0;

    if (!selection()->isNone()) {
        loopInTick = selectionRange()->startTick().ticks();
        loopOutTick = selectionRange()->endTick().ticks();
    }

    if (loopInTick <= 0) {
        loopInTick = INotationPlayback::FirstScoreTick;
    }

    if (loopOutTick <= 0) {
        loopOutTick = INotationPlayback::LastScoreTick;
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

    msecs_t fromMsesc = tickToMsecs(boundaries.loopInTick);
    msecs_t toMsecs = tickToMsecs(boundaries.loopOutTick);
    playback()->player()->setLoop(m_currentSequenceId, fromMsesc, toMsecs);

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

mu::project::IProjectAudioSettingsPtr PlaybackController::audioSettings() const
{
    IF_ASSERT_FAILED(globalContext()->currentProject()) {
        return nullptr;
    }

    return globalContext()->currentProject()->audioSettings();
}

void PlaybackController::resetCurrentSequence()
{
    playback()->player()->playbackPositionMsecs().resetOnReceive(this);
    playback()->player()->playbackStatusChanged().resetOnReceive(this);

    playback()->tracks()->inputParamsChanged().resetOnReceive(this);
    playback()->audioOutput()->outputParamsChanged().resetOnReceive(this);
    playback()->audioOutput()->masterOutputParamsChanged().resetOnReceive(this);

    setCurrentTick(0);
    setCurrentPlaybackStatus(PlaybackStatus::Stopped);

    playback()->removeSequence(m_currentSequenceId);
    m_currentSequenceId = -1;
}

void PlaybackController::setCurrentTick(const tick_t tick)
{
    if (m_currentTick == tick) {
        return;
    }

    m_currentTick = tick;
    m_playbackPositionChanged.notify();
}

void PlaybackController::addTrack(const InstrumentTrackId& instrumentTrackId, const std::string& title)
{
    IF_ASSERT_FAILED(notationPlayback() && playback()) {
        return;
    }

    if (!instrumentTrackId.isValid()) {
        return;
    }

    AudioInputParams inParams = audioSettings()->trackInputParams(instrumentTrackId);
    AudioOutputParams outParams = trackOutputParams(instrumentTrackId);
    mpe::PlaybackData playbackData = notationPlayback()->trackPlaybackData(instrumentTrackId);

    if (!playbackData.isValid()) {
        return;
    }

    uint64_t notationPlaybackKey = reinterpret_cast<uint64_t>(notationPlayback().get());

    playback()->tracks()->addTrack(m_currentSequenceId, title, std::move(playbackData), { std::move(inParams), std::move(outParams) })
    .onResolve(this, [this, instrumentTrackId, notationPlaybackKey](const TrackId trackId, const AudioParams& appliedParams) {
        //! NOTE It may be that while we were adding a track, the notation was already closed (or opened another)
        //! This situation can be if the notation was opened and immediately closed.
        quint64 currentNotationPlaybackKey = reinterpret_cast<uint64_t>(notationPlayback().get());
        if (currentNotationPlaybackKey != notationPlaybackKey) {
            return;
        }

        m_trackIdMap.insert({ instrumentTrackId, trackId });

        audioSettings()->setTrackInputParams(instrumentTrackId, appliedParams.in);
        audioSettings()->setTrackOutputParams(instrumentTrackId, appliedParams.out);

        updateMuteStates();
    })
    .onReject(this, [](int code, const std::string& msg) {
        LOGE() << "can't add a new track, code: [" << code << "] " << msg;
    });
}

void PlaybackController::setTrackActivity(const engraving::InstrumentTrackId& instrumentTrackId, const bool isActive)
{
    IF_ASSERT_FAILED(audioSettings() && playback()) {
        return;
    }

    AudioOutputParams outParams = audioSettings()->trackOutputParams(instrumentTrackId);

    outParams.muted = !isActive;

    audio::TrackId trackId = m_trackIdMap[instrumentTrackId];
    playback()->audioOutput()->setOutputParams(m_currentSequenceId, trackId, std::move(outParams));
}

AudioOutputParams PlaybackController::trackOutputParams(const engraving::InstrumentTrackId& instrumentTrackId) const
{
    IF_ASSERT_FAILED(audioSettings() && notationConfiguration() && notationPlayback()) {
        return {};
    }

    AudioOutputParams result = audioSettings()->trackOutputParams(instrumentTrackId);

    if (instrumentTrackId == notationPlayback()->metronomeTrackId()) {
        result.muted = !notationConfiguration()->isMetronomeEnabled();
    }

    return result;
}

InstrumentTrackIdSet PlaybackController::availableInstrumentTracks() const
{
    InstrumentTrackIdSet result;

    for (const auto& pair : m_trackIdMap) {
        result.insert(pair.first);
    }

    return result;
}

void PlaybackController::removeNonExistingTracks()
{
    for (const InstrumentTrackId& instrumentTrackId : availableInstrumentTracks()) {
        if (instrumentTrackId == notationPlayback()->metronomeTrackId()) {
            continue;
        }

        if (!masterNotationParts()->partExists(instrumentTrackId.partId)) {
            removeTrack(instrumentTrackId);
            continue;
        }

        const Part* part = masterNotationParts()->part(instrumentTrackId.partId);
        const InstrumentTrackIdSet& idSet = part->instrumentTrackIdSet();

        if (idSet.find(instrumentTrackId) == idSet.cend()) {
            removeTrack(instrumentTrackId);
        }
    }

    updateMuteStates();
}

void PlaybackController::removeTrack(const InstrumentTrackId& instrumentTrackId)
{
    IF_ASSERT_FAILED(notationPlayback() && playback()) {
        return;
    }

    auto search = m_trackIdMap.find(instrumentTrackId);

    if (search == m_trackIdMap.end()) {
        return;
    }

    playback()->tracks()->removeTrack(m_currentSequenceId, search->second);
    audioSettings()->removeTrackParams(instrumentTrackId);

    m_trackIdMap.erase(instrumentTrackId);
}

void PlaybackController::setupNewCurrentSequence(const TrackSequenceId sequenceId)
{
    playback()->tracks()->removeAllTracks(m_currentSequenceId);

    m_currentSequenceId = sequenceId;
    m_currentSequenceIdChanged.notify();

    if (!notationPlayback()) {
        return;
    }

    audio::AudioOutputParams masterOutputParams = audioSettings()->masterAudioOutputParams();
    playback()->audioOutput()->setMasterOutputParams(masterOutputParams);

    subscribeOnAudioParamsChanges();
    setupSequenceTracks();
    setupSequencePlayer();
}

void PlaybackController::subscribeOnAudioParamsChanges()
{
    playback()->audioOutput()->masterOutputParamsChanged().onReceive(this, [this](const audio::AudioOutputParams& params) {
        audioSettings()->setMasterAudioOutputParams(params);
    });

    playback()->tracks()->inputParamsChanged().onReceive(this,
                                                         [this](const TrackSequenceId sequenceId,
                                                                const TrackId trackId,
                                                                const AudioInputParams& params) {
        if (sequenceId != m_currentSequenceId) {
            return;
        }

        auto search = std::find_if(m_trackIdMap.begin(), m_trackIdMap.end(), [trackId](const auto& pair) {
            return pair.second == trackId;
        });

        if (search != m_trackIdMap.end()) {
            audioSettings()->setTrackInputParams(search->first, params);
        }
    });

    playback()->audioOutput()->outputParamsChanged().onReceive(this,
                                                               [this](const TrackSequenceId sequenceId,
                                                                      const TrackId trackId,
                                                                      const AudioOutputParams& params) {
        if (sequenceId != m_currentSequenceId) {
            return;
        }

        auto search = std::find_if(m_trackIdMap.begin(), m_trackIdMap.end(), [trackId](const auto& pair) {
            return pair.second == trackId;
        });

        if (search != m_trackIdMap.end()) {
            audioSettings()->setTrackOutputParams(search->first, params);
        }
    });
}

void PlaybackController::setupSequenceTracks()
{
    m_trackIdMap.clear();

    if (!masterNotationParts()) {
        return;
    }

    NotifyList<const Part*> partList = masterNotationParts()->partList();

    for (const Part* part : partList) {
        for (const InstrumentTrackId& trackId : part->instrumentTrackIdSet()) {
            addTrack(trackId, part->partName().toStdString());
        }
    }

    addTrack(notationPlayback()->metronomeTrackId(), qtrc("playback", "Metronome").toStdString());

    notationPlayback()->trackAdded().onReceive(this, [this](const InstrumentTrackId& instrumentTrackId) {
        const Part* part = masterNotationParts()->part(instrumentTrackId.partId);

        if (!part) {
            return;
        }

        addTrack(instrumentTrackId, part->partName().toStdString());
    });

    notationPlayback()->trackRemoved().onReceive(this, [this](const InstrumentTrackId& instrumentTrackId) {
        removeTrack(instrumentTrackId);
    });

    partList.onItemChanged(this, [this](const Part* part) {
        for (const InstrumentTrackId& trackId : part->instrumentTrackIdSet()) {
            auto search = m_trackIdMap.find(trackId);
            if (search == m_trackIdMap.cend()) {
                removeNonExistingTracks();
                addTrack(trackId, part->partName().toStdString());
            }
        }

        updateMuteStates();
    });

    audioSettings()->soloMuteStateChanged().onReceive(this,
                                                      [this](const InstrumentTrackId&, const project::IProjectAudioSettings::SoloMuteState&) {
        updateMuteStates();
    });
}

void PlaybackController::setupSequencePlayer()
{
    PlaybackCursorType cursorType = configuration()->cursorType();

    playback()->player()->playbackPositionMsecs().onReceive(this, [this, cursor = std::move(cursorType)]
                                                            (const TrackSequenceId id, const audio::msecs_t& msecs) {
        if (m_currentSequenceId != id) {
            return;
        }

        tick_t tick = notationPlayback()->secToTick(msecs / 1000.);

        setCurrentTick(tick);
        m_tickPlayed.send(std::move(tick));
    });

    playback()->player()->setDuration(m_currentSequenceId, notationPlayback()->totalPlayTime());

    notationPlayback()->totalPlayTimeChanged().onReceive(this, [this](const audio::msecs_t totalPlaybackTime) {
        playback()->player()->setDuration(m_currentSequenceId, totalPlaybackTime);
        m_totalPlayTimeChanged.notify();
    });

    playback()->player()->playbackStatusChanged().onReceive(this, [this](const TrackSequenceId id, const PlaybackStatus status) {
        if (m_currentSequenceId == id) {
            setCurrentPlaybackStatus(status);
        }
    });
}

void PlaybackController::updateMuteStates()
{
    if (!masterNotationParts() || !audioSettings() || !playback()) {
        return;
    }

    NotifyList<const Part*> masterPartList = masterNotationParts()->partList();
    bool hasSolo = false;

    for (const Part* masterPart : masterPartList) {
        for (const InstrumentTrackId& instrumentTrackId : masterPart->instrumentTrackIdSet()) {
            if (audioSettings()->soloMuteState(instrumentTrackId).solo) {
                hasSolo = true;
                break;
            }
        }
    }

    INotationPartsPtr notationParts = m_notation->parts();

    InstrumentTrackIdSet allowedInstrumentTrackIdSet = instrumentTrackIdSetForRangePlayback();
    bool isRangePlaybackMode = selection()->isRange() && !allowedInstrumentTrackIdSet.empty();

    for (const Part* masterPart : masterPartList) {
        const Part* part = notationParts->part(masterPart->id());
        bool isPartVisible = part && part->show();

        for (const InstrumentTrackId& instrumentTrackId : masterPart->instrumentTrackIdSet()) {
            if (!mu::contains(m_trackIdMap, instrumentTrackId)) {
                continue;
            }

            auto soloMuteState = audioSettings()->soloMuteState(instrumentTrackId);

            bool shouldBeMuted = soloMuteState.mute
                                 || (hasSolo && !soloMuteState.solo)
                                 || (!isPartVisible);

            if (isRangePlaybackMode && !shouldBeMuted) {
                shouldBeMuted = !mu::contains(allowedInstrumentTrackIdSet, instrumentTrackId);
            }

            AudioOutputParams params = trackOutputParams(instrumentTrackId);
            params.muted = shouldBeMuted;

            audio::TrackId trackId = m_trackIdMap.at(instrumentTrackId);
            playback()->audioOutput()->setOutputParams(m_currentSequenceId, trackId, std::move(params));
        }
    }
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
    QTime result = ZERO_TIME;

    if (!notationPlayback()) {
        return result;
    }

    return result.addMSecs(notationPlayback()->totalPlayTime());
}

Notification PlaybackController::totalPlayTimeChanged() const
{
    return m_totalPlayTimeChanged;
}

Tempo PlaybackController::currentTempo() const
{
    return m_currentTempo;
}

Notification PlaybackController::currentTempoChanged() const
{
    return m_currentTempoChanged;
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
    return tickToMsecs(tick);
}

msecs_t PlaybackController::tickToMsecs(int tick) const
{
    float sec = notationPlayback()->playedTickToSec(tick);
    return secondsToMilliseconds(sec);
}
