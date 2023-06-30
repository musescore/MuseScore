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
static const ActionCode PLAY_CHORD_SYMBOLS_CODE("play-chord-symbols");
static const ActionCode PLAYBACK_SETUP("playback-setup");

static AudioOutputParams makeReverbOutputParams()
{
    AudioFxParams reverbParams;
    reverbParams.resourceMeta = makeReverbMeta();
    reverbParams.chainOrder = 0;
    reverbParams.active = true;

    AudioOutputParams result;
    result.fxChain.emplace(reverbParams.chainOrder, std::move(reverbParams));

    return result;
}

static std::string resolveAuxTrackTitle(aux_channel_idx_t index, const AudioOutputParams& params)
{
    if (params.fxChain.size() == 1) {
        const AudioResourceMeta& meta = params.fxChain.cbegin()->second.resourceMeta;
        if (meta.id == MUSE_REVERB_ID) {
            return mu::trc("playback", "Reverb");
        }

        return meta.id;
    }

    return mu::mtrc("playback", "Aux %1").arg(index + 1).toStdString();
}

void PlaybackController::init()
{
    dispatcher()->reg(this, PLAY_CODE, this, &PlaybackController::togglePlay);
    dispatcher()->reg(this, STOP_CODE, this, &PlaybackController::pause);
    dispatcher()->reg(this, REWIND_CODE, this, &PlaybackController::rewind);
    dispatcher()->reg(this, LOOP_CODE, this, &PlaybackController::toggleLoopPlayback);
    dispatcher()->reg(this, LOOP_IN_CODE, [this]() { addLoopBoundary(LoopBoundaryType::LoopIn); });
    dispatcher()->reg(this, LOOP_OUT_CODE, [this]() { addLoopBoundary(LoopBoundaryType::LoopOut); });
    dispatcher()->reg(this, REPEAT_CODE, this, &PlaybackController::togglePlayRepeats);
    dispatcher()->reg(this, PLAY_CHORD_SYMBOLS_CODE, this, &PlaybackController::togglePlayChordSymbols);
    dispatcher()->reg(this, PAN_CODE, this, &PlaybackController::toggleAutomaticallyPan);
    dispatcher()->reg(this, METRONOME_CODE, this, &PlaybackController::toggleMetronome);
    dispatcher()->reg(this, MIDI_ON_CODE, this, &PlaybackController::toggleMidiInput);
    dispatcher()->reg(this, COUNT_IN_CODE, this, &PlaybackController::toggleCountIn);
    dispatcher()->reg(this, PLAYBACK_SETUP, this, &PlaybackController::openPlaybackSetupDialog);

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });

    globalContext()->currentProjectChanged().onNotify(this, [this]() {
        if (m_currentSequenceId != -1) {
            resetCurrentSequence();
        }

        if (!globalContext()->currentProject()) {
            return;
        }

        m_loadingProgress.started.notify();

        playback()->addSequence().onResolve(this, [this](const TrackSequenceId& sequenceId) {
            setupNewCurrentSequence(sequenceId);
        });
    });

    m_totalPlayTimeChanged.onNotify(this, [this]() {
        updateCurrentTempo();
    });

    m_playbackPositionChanged.onNotify(this, [this]() {
        updateCurrentTempo();

        msecs_t endMsecs = playbackEndMsecs();
        const LoopBoundaries& loop = notationPlayback()->loopBoundaries();
        if (m_currentPlaybackTimeMsecs == endMsecs && m_currentPlaybackTimeMsecs != loop.loopOutTick) {
            stop();
        }
    });
}

void PlaybackController::updateCurrentTempo()
{
    if (!notationPlayback()) {
        return;
    }

    const Tempo& newTempo = notationPlayback()->tempo(m_currentTick);

    if (newTempo == m_currentTempo) {
        return;
    }

    m_currentTempo = newTempo;
    m_currentTempoChanged.notify();
}

bool PlaybackController::isPlayAllowed() const
{
    return m_notation != nullptr && m_masterNotation->hasParts() && isLoaded();
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

bool PlaybackController::isLoaded() const
{
    return m_loadingTrackCount == 0;
}

bool PlaybackController::isLoopVisible() const
{
    return notationPlayback() ? notationPlayback()->loopBoundaries().visible : false;
}

bool PlaybackController::isPlaybackLooped() const
{
    return notationPlayback() ? !notationPlayback()->loopBoundaries().isNull() : false;
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
    seek(tickToMsecs(tick));
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
    return secondsFromMilliseconds(m_currentPlaybackTimeMsecs);
}

TrackSequenceId PlaybackController::currentTrackSequenceId() const
{
    return m_currentSequenceId;
}

Notification PlaybackController::currentTrackSequenceIdChanged() const
{
    return m_currentSequenceIdChanged;
}

const IPlaybackController::InstrumentTrackIdMap& PlaybackController::instrumentTrackIdMap() const
{
    return m_instrumentTrackIdMap;
}

const IPlaybackController::AuxTrackIdMap& PlaybackController::auxTrackIdMap() const
{
    return m_auxTrackIdMap;
}

Channel<TrackId> PlaybackController::trackAdded() const
{
    return m_trackAdded;
}

Channel<TrackId> PlaybackController::trackRemoved() const
{
    return m_trackRemoved;
}

std::string PlaybackController::auxChannelName(aux_channel_idx_t index) const
{
    return resolveAuxTrackTitle(index, audioSettings()->auxOutputParams(index));
}

Channel<aux_channel_idx_t, std::string> PlaybackController::auxChannelNameChanged() const
{
    return m_auxChannelNameChanged;
}

void PlaybackController::playElements(const std::vector<const notation::EngravingItem*>& elements)
{
    IF_ASSERT_FAILED(notationPlayback()) {
        return;
    }

    if (!configuration()->playNotesWhenEditing()) {
        return;
    }

    std::vector<const notation::EngravingItem*> elementsForPlaying;

    for (const EngravingItem* element : elements) {
        IF_ASSERT_FAILED(element) {
            continue;
        }

        if (element->isChord() && !configuration()->playChordWhenEditing()) {
            continue;
        }

        if (element->isHarmony() && !configuration()->playHarmonyWhenEditing()) {
            continue;
        }

        elementsForPlaying.push_back(element);
    }

    notationPlayback()->triggerEventsForItems(elementsForPlaying);
}

void PlaybackController::playMetronome(int tick)
{
    notationPlayback()->triggerMetronome(tick);
}

void PlaybackController::seekElement(const notation::EngravingItem* element)
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
}

void PlaybackController::seekListSelection()
{
    const std::vector<EngravingItem*>& elements = selection()->elements();
    if (elements.empty()) {
        return;
    }

    seekElement(elements.back());
}

void PlaybackController::seekRangeSelection()
{
    if (!selection()->isRange()) {
        return;
    }

    midi::tick_t startTick = selectionRange()->startTick().ticks();

    RetVal<midi::tick_t> tick = notationPlayback()->playPositionTickByRawTick(startTick);
    if (!tick.ret) {
        return;
    }

    seek(tick.val);
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

uint64_t PlaybackController::notationPlaybackKey() const
{
    return reinterpret_cast<uint64_t>(notationPlayback().get());
}

void PlaybackController::onNotationChanged()
{
    m_masterNotation = globalContext()->currentMasterNotation();
    setNotation(globalContext()->currentNotation());

    DEFER {
        m_isPlayAllowedChanged.notify();
        m_totalPlayTimeChanged.notify();
    };

    if (!m_masterNotation) {
        return;
    }

    m_masterNotation->hasPartsChanged().onNotify(this, [this]() {
        m_isPlayAllowedChanged.notify();
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
            updateLoop();
            updateMuteStates();
        }

        if (!isPlaybackLooped()) {
            seekListSelection();
        }

        return;
    }

    playback()->player()->resetLoop(m_currentSequenceId);

    seekRangeSelection();
    updateMuteStates();
}

void PlaybackController::togglePlay()
{
    if (!isPlayAllowed()) {
        LOGW() << "playback not allowed";
        return;
    }

    interaction()->endEditElement();

    if (isPlaying()) {
        pause();
    } else if (isPaused()) {
        msecs_t endMsecs = playbackEndMsecs();

        if (m_currentPlaybackTimeMsecs == endMsecs) {
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
    IF_ASSERT_FAILED(playback()) {
        return;
    }

    if (isPlaybackLooped()) {
        msecs_t startMsecs = playbackStartMsecs();
        seek(startMsecs);
    }

    playback()->player()->play(m_currentSequenceId);
    setCurrentPlaybackStatus(PlaybackStatus::Running);
}

void PlaybackController::rewind(const ActionData& args)
{
    msecs_t startMsecs = playbackStartMsecs();
    msecs_t endMsecs = playbackEndMsecs();
    msecs_t newPosition = !args.empty() ? args.arg<msecs_t>(0) : 0;
    newPosition = std::clamp(newPosition, startMsecs, endMsecs);

    seek(newPosition);
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

    const LoopBoundaries& loop = notationPlayback()->loopBoundaries();
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

        if (part->hasChordSymbol()) {
            result.insert(notationPlayback()->chordSymbolsTrackId(part->id()));
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

void PlaybackController::togglePlayChordSymbols()
{
    bool playChordSymbolsEnabled = notationConfiguration()->isPlayChordSymbolsEnabled();
    notationConfiguration()->setIsPlayChordSymbolsEnabled(!playChordSymbolsEnabled);
    notifyActionCheckedChanged(PLAY_CHORD_SYMBOLS_CODE);

    for (auto it = m_instrumentTrackIdMap.cbegin(); it != m_instrumentTrackIdMap.cend(); ++it) {
        if (notationPlayback()->isChordSymbolsTrack(it->first)) {
            setTrackActivity(it->first, !playChordSymbolsEnabled);
        }
    }
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

void PlaybackController::openPlaybackSetupDialog()
{
    interactive()->open("musescore://playback/soundprofilesdialog");
}

void PlaybackController::addLoopBoundary(LoopBoundaryType type)
{
    if (isPlaying()) {
        addLoopBoundaryToTick(type, m_currentTick);
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

void PlaybackController::updateLoop()
{
    IF_ASSERT_FAILED(notationPlayback() && playback()) {
        return;
    }

    const LoopBoundaries& boundaries = notationPlayback()->loopBoundaries();

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

    playback()->tracks()->clearSources();
    playback()->tracks()->inputParamsChanged().resetOnReceive(this);

    playback()->audioOutput()->clearAllFx();
    playback()->audioOutput()->outputParamsChanged().resetOnReceive(this);
    playback()->audioOutput()->masterOutputParamsChanged().resetOnReceive(this);
    playback()->audioOutput()->clearMasterOutputParams();

    setCurrentPlaybackTime(0);
    setCurrentPlaybackStatus(PlaybackStatus::Stopped);

    playback()->removeSequence(m_currentSequenceId);

    m_instrumentTrackIdMap.clear();
    m_auxTrackIdMap.clear();

    m_currentSequenceId = -1;
    m_currentSequenceIdChanged.notify();
}

void PlaybackController::setCurrentPlaybackTime(msecs_t msecs)
{
    if (m_currentPlaybackTimeMsecs == msecs) {
        return;
    }

    m_currentPlaybackTimeMsecs = msecs;
    m_currentTick = notationPlayback()->secToTick(secondsFromMilliseconds(msecs));

    m_playbackPositionChanged.notify();
}

void PlaybackController::addTrack(const InstrumentTrackId& instrumentTrackId, const TrackAddFinished& onFinished)
{
    if (notationPlayback()->metronomeTrackId() == instrumentTrackId) {
        doAddTrack(instrumentTrackId, trc("playback", "Metronome"), onFinished);
        return;
    }

    const Part* part = masterNotationParts()->part(instrumentTrackId.partId);
    if (!part) {
        return;
    }

    if (notationPlayback()->isChordSymbolsTrack(instrumentTrackId)) {
        const std::string trackName = trc("playback", "Chords") + "." + part->partName().toStdString();
        doAddTrack(instrumentTrackId, trackName, onFinished);
        return;
    }

    const std::string primaryInstrId = part->instrument()->id().toStdString();

    const std::string trackName = (instrumentTrackId.instrumentId == primaryInstrId)
                                  ? part->partName().toStdString()
                                  : "(" + part->instrumentById(instrumentTrackId.instrumentId)->trackName().toStdString() + ")";

    doAddTrack(instrumentTrackId, trackName, onFinished);
}

void PlaybackController::doAddTrack(const InstrumentTrackId& instrumentTrackId, const std::string& title,
                                    const TrackAddFinished& onFinished)
{
    IF_ASSERT_FAILED(notationPlayback() && playback()) {
        return;
    }

    if (!instrumentTrackId.isValid()) {
        return;
    }

    mpe::PlaybackData playbackData = notationPlayback()->trackPlaybackData(instrumentTrackId);
    if (!playbackData.isValid()) {
        return;
    }

    AudioInputParams inParams = audioSettings()->trackInputParams(instrumentTrackId);
    AudioOutputParams outParams = trackOutputParams(instrumentTrackId);

    bool isMetronome = notationPlayback()->metronomeTrackId() == instrumentTrackId;

    if (!inParams.isValid()) {
        if (isMetronome) {
            const SoundProfile& profile = profilesRepo()->profile(configuration()->basicSoundProfileName());
            inParams = { profile.findResource(playbackData.setupData), {} };
        } else {
            const SoundProfile& profile = profilesRepo()->profile(audioSettings()->activeSoundProfile());
            inParams = { profile.findResource(playbackData.setupData), {} };
        }
    }

    if (!isMetronome && outParams.auxSends.empty()) {
        const String& instrumentSoundId = inParams.resourceMeta.attributeVal(PLAYBACK_SETUP_DATA_ATTRIBUTE);
        AudioSourceType sourceType = inParams.isValid() ? inParams.type() : AudioSourceType::Fluid;

        for (aux_channel_idx_t idx = 0; idx < AUX_CHANNEL_NUM; ++idx) {
            gain_t signalAmount = configuration()->defaultAuxSendValue(idx, sourceType, instrumentSoundId);
            outParams.auxSends.emplace_back(AuxSendParams { signalAmount, true });
        }
    }

    uint64_t playbackKey = notationPlaybackKey();

    playback()->tracks()->addTrack(m_currentSequenceId, title, std::move(playbackData), { std::move(inParams), std::move(outParams) })
    .onResolve(this, [this, instrumentTrackId, playbackKey, onFinished](const TrackId trackId, const AudioParams& appliedParams) {
        //! NOTE It may be that while we were adding a track, the notation was already closed (or opened another)
        //! This situation can be if the notation was opened and immediately closed.
        if (notationPlaybackKey() != playbackKey) {
            return;
        }

        m_instrumentTrackIdMap.insert({ instrumentTrackId, trackId });

        audioSettings()->setTrackInputParams(instrumentTrackId, appliedParams.in);
        audioSettings()->setTrackOutputParams(instrumentTrackId, appliedParams.out);

        updateMuteStates();

        onFinished();

        m_trackAdded.send(trackId);
    })
    .onReject(this, [instrumentTrackId, onFinished](int code, const std::string& msg) {
        LOGE() << "can't add a new track, code: [" << code << "] " << msg;

        onFinished();
    });

    m_loadingTrackCount++;
}

void PlaybackController::addAuxTrack(aux_channel_idx_t index, const TrackAddFinished& onFinished)
{
    IF_ASSERT_FAILED(notationPlayback() && playback()) {
        return;
    }

    AudioOutputParams outParams;

    if (audioSettings()->containsAuxOutputParams(index)) {
        outParams = audioSettings()->auxOutputParams(index);
    } else if (index == REVERB_CHANNEL_IDX) {
        outParams = makeReverbOutputParams();
    }

    std::string title = resolveAuxTrackTitle(index, outParams);
    uint64_t playbackKey = notationPlaybackKey();

    playback()->tracks()->addAuxTrack(m_currentSequenceId, title, outParams)
    .onResolve(this, [this, playbackKey, index, onFinished](const TrackId trackId, const AudioOutputParams& appliedParams) {
        //! NOTE It may be that while we were adding a track, the notation was already closed (or opened another)
        //! This situation can be if the notation was opened and immediately closed.
        if (notationPlaybackKey() != playbackKey) {
            return;
        }

        m_auxTrackIdMap.insert({ index, trackId });

        audioSettings()->setAuxOutputParams(index, appliedParams);

        updateMuteStates();
        onFinished();

        m_trackAdded.send(trackId);
    })
    .onReject(this, [onFinished](int code, const std::string& msg) {
        LOGE() << "can't add a new aux track, code: [" << code << "] " << msg;

        onFinished();
    });

    m_loadingTrackCount++;
}

void PlaybackController::setTrackActivity(const engraving::InstrumentTrackId& instrumentTrackId, const bool isActive)
{
    IF_ASSERT_FAILED(audioSettings() && playback()) {
        return;
    }

    AudioOutputParams outParams = audioSettings()->trackOutputParams(instrumentTrackId);

    outParams.muted = !isActive;

    audio::TrackId trackId = m_instrumentTrackIdMap[instrumentTrackId];
    playback()->audioOutput()->setOutputParams(m_currentSequenceId, trackId, std::move(outParams));
}

AudioOutputParams PlaybackController::trackOutputParams(const InstrumentTrackId& instrumentTrackId) const
{
    IF_ASSERT_FAILED(audioSettings() && notationConfiguration() && notationPlayback()) {
        return {};
    }

    AudioOutputParams result = audioSettings()->trackOutputParams(instrumentTrackId);

    if (instrumentTrackId == notationPlayback()->metronomeTrackId()) {
        result.muted = !notationConfiguration()->isMetronomeEnabled();
        return result;
    }

    if (notationPlayback()->isChordSymbolsTrack(instrumentTrackId)) {
        result.muted = !notationConfiguration()->isPlayChordSymbolsEnabled();
    }

    return result;
}

InstrumentTrackIdSet PlaybackController::availableInstrumentTracks() const
{
    InstrumentTrackIdSet result;

    for (const auto& pair : m_instrumentTrackIdMap) {
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

    auto search = m_instrumentTrackIdMap.find(instrumentTrackId);

    if (search == m_instrumentTrackIdMap.end()) {
        return;
    }

    playback()->tracks()->removeTrack(m_currentSequenceId, search->second);
    audioSettings()->removeTrackParams(instrumentTrackId);

    m_trackRemoved.send(search->second);
    m_instrumentTrackIdMap.erase(instrumentTrackId);
}

void PlaybackController::setupNewCurrentSequence(const TrackSequenceId sequenceId)
{
    playback()->tracks()->removeAllTracks(m_currentSequenceId);

    m_currentSequenceId = sequenceId;

    if (!notationPlayback()) {
        return;
    }

    audio::AudioOutputParams masterOutputParams = audioSettings()->masterAudioOutputParams();
    playback()->audioOutput()->setMasterOutputParams(masterOutputParams);

    subscribeOnAudioParamsChanges();
    setupSequenceTracks();
    setupSequencePlayer();

    m_currentSequenceIdChanged.notify();
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

        auto search = std::find_if(m_instrumentTrackIdMap.begin(), m_instrumentTrackIdMap.end(), [trackId](const auto& pair) {
            return pair.second == trackId;
        });

        if (search != m_instrumentTrackIdMap.end()) {
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

        auto instrumentIt = std::find_if(m_instrumentTrackIdMap.begin(), m_instrumentTrackIdMap.end(), [trackId](const auto& pair) {
            return pair.second == trackId;
        });

        if (instrumentIt != m_instrumentTrackIdMap.end()) {
            audioSettings()->setTrackOutputParams(instrumentIt->first, params);
            return;
        }

        auto auxIt = std::find_if(m_auxTrackIdMap.begin(), m_auxTrackIdMap.end(), [trackId](const auto& pair) {
            return pair.second == trackId;
        });

        if (auxIt != m_auxTrackIdMap.end()) {
            aux_channel_idx_t auxIdx = auxIt->first;
            std::string oldName = resolveAuxTrackTitle(auxIdx, audioSettings()->auxOutputParams(auxIdx));
            std::string newName = resolveAuxTrackTitle(auxIdx, params);

            audioSettings()->setAuxOutputParams(auxIdx, params);

            if (oldName != newName) {
                m_auxChannelNameChanged.send(auxIdx, newName);
            }
        }
    });
}

void PlaybackController::setupSequenceTracks()
{
    m_instrumentTrackIdMap.clear();

    if (!masterNotationParts()) {
        return;
    }

    m_loadingTrackCount = 0;

    InstrumentTrackIdSet trackIdSet = notationPlayback()->existingTrackIdSet();
    size_t trackCount = trackIdSet.size() + AUX_CHANNEL_NUM;
    std::string title = trc("playback", "Loading audio samples");

    auto onAddFinished = [this, trackCount, title]() {
        m_loadingTrackCount--;

        size_t current = trackCount - m_loadingTrackCount;
        m_loadingProgress.progressChanged.send(current, trackCount, title);

        if (m_loadingTrackCount == 0) {
            m_loadingProgress.finished.send(make_ok());
            m_isPlayAllowedChanged.notify();
        }
    };

    for (const InstrumentTrackId& trackId : trackIdSet) {
        addTrack(trackId, onAddFinished);
    }

    for (aux_channel_idx_t idx = 0; idx < AUX_CHANNEL_NUM; ++idx) {
        addAuxTrack(idx, onAddFinished);
    }

    m_loadingProgress.progressChanged.send(0, trackCount, title);

    notationPlayback()->trackAdded().onReceive(this, [this, onAddFinished](const InstrumentTrackId& instrumentTrackId) {
        addTrack(instrumentTrackId, onAddFinished);
    });

    notationPlayback()->trackRemoved().onReceive(this, [this](const InstrumentTrackId& instrumentTrackId) {
        removeTrack(instrumentTrackId);
    });

    NotifyList<const Part*> partList = masterNotationParts()->partList();
    partList.onItemChanged(this, [this, onAddFinished](const Part* part) {
        for (const InstrumentTrackId& trackId : part->instrumentTrackIdSet()) {
            auto search = m_instrumentTrackIdMap.find(trackId);
            if (search == m_instrumentTrackIdMap.cend()) {
                removeNonExistingTracks();
                addTrack(trackId, onAddFinished);
            }
        }

        updateMuteStates();
    });

    audioSettings()->trackSoloMuteStateChanged().onReceive(
        this, [this](const InstrumentTrackId&, const project::IProjectAudioSettings::SoloMuteState&) {
        updateMuteStates();
    });

    m_isPlayAllowedChanged.notify();
}

void PlaybackController::setupSequencePlayer()
{
    playback()->player()->playbackPositionMsecs().onReceive(
        this, [this](const TrackSequenceId id, const audio::msecs_t& msecs) {
        if (m_currentSequenceId != id) {
            return;
        }

        if (!isPlaying()) {
            return;
        }

        setCurrentPlaybackTime(msecs);
        m_tickPlayed.send(m_currentTick);
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
    if (!audioSettings() || !playback()) {
        return;
    }

    InstrumentTrackIdSet existingTrackIdSet = notationPlayback()->existingTrackIdSet();
    bool hasSolo = false;

    for (const InstrumentTrackId& instrumentTrackId : existingTrackIdSet) {
        if (audioSettings()->trackSoloMuteState(instrumentTrackId).solo) {
            hasSolo = true;
            break;
        }
    }

    INotationPartsPtr notationParts = m_notation->parts();
    InstrumentTrackIdSet allowedInstrumentTrackIdSet = instrumentTrackIdSetForRangePlayback();
    bool isRangePlaybackMode = !m_isExportingAudio && selection()->isRange() && !allowedInstrumentTrackIdSet.empty();

    for (const InstrumentTrackId& instrumentTrackId : existingTrackIdSet) {
        if (!mu::contains(m_instrumentTrackIdMap, instrumentTrackId)) {
            continue;
        }

        if (instrumentTrackId == notationPlayback()->metronomeTrackId()) {
            continue;
        }

        const Part* part = notationParts->part(instrumentTrackId.partId);
        bool isPartVisible = part && part->show();

        auto soloMuteState = audioSettings()->trackSoloMuteState(instrumentTrackId);

        bool shouldBeMuted = soloMuteState.mute
                             || (hasSolo && !soloMuteState.solo)
                             || (!isPartVisible);

        if (notationPlayback()->isChordSymbolsTrack(instrumentTrackId) && !shouldBeMuted) {
            shouldBeMuted = !notationConfiguration()->isPlayChordSymbolsEnabled();
        }

        if (isRangePlaybackMode && !shouldBeMuted) {
            shouldBeMuted = !mu::contains(allowedInstrumentTrackIdSet, instrumentTrackId);
        }

        AudioOutputParams params = trackOutputParams(instrumentTrackId);
        params.muted = shouldBeMuted;

        audio::TrackId trackId = m_instrumentTrackIdMap.at(instrumentTrackId);
        playback()->audioOutput()->setOutputParams(m_currentSequenceId, trackId, std::move(params));
    }
}

bool PlaybackController::actionChecked(const ActionCode& actionCode) const
{
    QMap<std::string, bool> isChecked {
        { LOOP_CODE, isLoopVisible() },
        { MIDI_ON_CODE, notationConfiguration()->isMidiInputEnabled() },
        { REPEAT_CODE, notationConfiguration()->isPlayRepeatsEnabled() },
        { PLAY_CHORD_SYMBOLS_CODE, notationConfiguration()->isPlayChordSymbolsEnabled() },
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
    return notationPlayback() ? notationPlayback()->beat(m_currentTick) : MeasureBeat();
}

msecs_t PlaybackController::beatToMilliseconds(int measureIndex, int beatIndex) const
{
    if (!notationPlayback()) {
        return 0;
    }

    int tick = notationPlayback()->beatToTick(measureIndex, beatIndex);
    return tickToMsecs(tick);
}

double PlaybackController::tempoMultiplier() const
{
    INotationPlaybackPtr playback = notationPlayback();
    return playback ? playback->tempoMultiplier() : 1.0;
}

void PlaybackController::setTempoMultiplier(double multiplier)
{
    INotationPlaybackPtr playback = notationPlayback();
    if (!playback) {
        return;
    }

    tick_t tick = m_currentTick;
    bool playing = isPlaying();

    if (playing) {
        pause();
    }

    playback->setTempoMultiplier(multiplier);
    seek(tick);
    updateLoop();

    if (playing) {
        resume();
    }
}

mu::framework::Progress PlaybackController::loadingProgress() const
{
    return m_loadingProgress;
}

void PlaybackController::applyProfile(const SoundProfileName& profileName)
{
    project::IProjectAudioSettingsPtr audioSettingsPtr = audioSettings();

    IF_ASSERT_FAILED(audioSettingsPtr) {
        return;
    }

    const SoundProfile& profile = profilesRepo()->profile(profileName);
    if (!profile.isValid()) {
        return;
    }

    const InstrumentTrackId& metronomeTrackId = notationPlayback()->metronomeTrackId();

    for (const auto& pair : m_instrumentTrackIdMap) {
        if (pair.first == metronomeTrackId) {
            continue;
        }

        const mpe::PlaybackData& playbackData = notationPlayback()->trackPlaybackData(pair.first);

        AudioInputParams newInputParams { profile.findResource(playbackData.setupData), {} };

        playback()->tracks()->setInputParams(m_currentSequenceId, pair.second, std::move(newInputParams));
    }

    audioSettingsPtr->setActiveSoundProfile(profileName);
}

void PlaybackController::setNotation(notation::INotationPtr notation)
{
    if (m_notation == notation) {
        return;
    }

    m_notation = notation;

    if (!m_notation) {
        return;
    }

    updateMuteStates();

    INotationPartsPtr notationParts = m_notation->parts();
    NotifyList<const Part*> partList = notationParts->partList();

    partList.onItemAdded(this, [this](const Part*) {
        updateMuteStates();
    });

    partList.onItemChanged(this, [this](const Part*) {
        updateMuteStates();
    });

    notationPlayback()->loopBoundariesChanged().onNotify(this, [this]() {
        updateLoop();
    });

    m_notation->interaction()->selectionChanged().onNotify(this, [this]() {
        onSelectionChanged();
    });

    m_notation->interaction()->textEditingEnded().onReceive(this, [this](engraving::TextBase* text) {
        if (text->isHarmony()) {
            playElements({ text });
        }
    });
}

void PlaybackController::setIsExportingAudio(bool exporting)
{
    m_isExportingAudio = exporting;
    updateMuteStates();
}

bool PlaybackController::canReceiveAction(const actions::ActionCode&) const
{
    return m_masterNotation != nullptr && m_masterNotation->hasParts();
}

msecs_t PlaybackController::tickToMsecs(int tick) const
{
    float sec = notationPlayback()->playedTickToSec(tick);
    return secondsToMilliseconds(sec);
}
