/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "engraving/dom/stafftext.h"
#include "engraving/dom/utils.h"
#include "engraving/dom/factory.h"

#include "audio/audioutils.h"
#include "audio/audioerrors.h"
#include "audio/devtools/inputlag.h"

#include "containers.h"
#include "defer.h"
#include "async/async.h"
#include "log.h"

using namespace muse;
using namespace muse::actions;
using namespace muse::async;
using namespace muse::audio;
using namespace muse::midi;
using namespace mu::engraving;
using namespace mu::notation;
using namespace mu::playback;

static const ActionCode PLAY_CODE("play");
static const ActionCode STOP_CODE("stop");
static const ActionCode PAUSE_AND_SELECT_CODE("pause-and-select");
static const ActionCode REWIND_CODE("rewind");
static const ActionCode LOOP_CODE("loop");
static const ActionCode LOOP_IN_CODE("loop-in");
static const ActionCode LOOP_OUT_CODE("loop-out");
static const ActionCode METRONOME_CODE("metronome");
static const ActionCode MIDI_ON_CODE("midi-on");
static const ActionCode INPUT_WRITTEN_PITCH("midi-input-written-pitch");
static const ActionCode INPUT_SOUNDING_PITCH("midi-input-sounding-pitch");
static const ActionCode COUNT_IN_CODE("countin");
static const ActionCode PAN_CODE("pan");
static const ActionCode REPEAT_CODE("repeat");
static const ActionCode PLAY_CHORD_SYMBOLS_CODE("play-chord-symbols");
static const ActionCode PLAYBACK_SETUP("playback-setup");
static const ActionCode TOGGLE_HEAR_PLAYBACK_WHEN_EDITING_CODE("toggle-hear-playback-when-editing");

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

static std::string resolveAuxTrackTitle(aux_channel_idx_t index, const AudioOutputParams& params, bool considerFx = true)
{
    if (considerFx && params.fxChain.size() == 1) {
        const AudioResourceMeta& meta = params.fxChain.cbegin()->second.resourceMeta;
        if (meta.id == MUSE_REVERB_ID) {
            return muse::trc("playback", "Reverb");
        }

        return meta.id;
    }

    return muse::mtrc("playback", "Aux %1").arg(index + 1).toStdString();
}

void PlaybackController::init()
{
    dispatcher()->reg(this, PLAY_CODE, this, &PlaybackController::togglePlay);
    dispatcher()->reg(this, STOP_CODE, [this]() { PlaybackController::pause(/*select*/ false); });
    dispatcher()->reg(this, PAUSE_AND_SELECT_CODE, [this]() { PlaybackController::pause(/*select*/ true); });
    dispatcher()->reg(this, REWIND_CODE, this, &PlaybackController::rewind);
    dispatcher()->reg(this, LOOP_CODE, this, &PlaybackController::toggleLoopPlayback);
    dispatcher()->reg(this, LOOP_IN_CODE, [this]() { addLoopBoundary(LoopBoundaryType::LoopIn); });
    dispatcher()->reg(this, LOOP_OUT_CODE, [this]() { addLoopBoundary(LoopBoundaryType::LoopOut); });
    dispatcher()->reg(this, REPEAT_CODE, this, &PlaybackController::togglePlayRepeats);
    dispatcher()->reg(this, PLAY_CHORD_SYMBOLS_CODE, this, &PlaybackController::togglePlayChordSymbols);
    dispatcher()->reg(this, PAN_CODE, this, &PlaybackController::toggleAutomaticallyPan);
    dispatcher()->reg(this, METRONOME_CODE, this, &PlaybackController::toggleMetronome);
    dispatcher()->reg(this, COUNT_IN_CODE, this, &PlaybackController::toggleCountIn);
    dispatcher()->reg(this, MIDI_ON_CODE, this, &PlaybackController::toggleMidiInput);
    dispatcher()->reg(this, INPUT_WRITTEN_PITCH, [this]() { PlaybackController::setMidiUseWrittenPitch(true); });
    dispatcher()->reg(this, INPUT_SOUNDING_PITCH, [this]() { PlaybackController::setMidiUseWrittenPitch(false); });
    dispatcher()->reg(this, PLAYBACK_SETUP, this, &PlaybackController::openPlaybackSetupDialog);
    dispatcher()->reg(this, TOGGLE_HEAR_PLAYBACK_WHEN_EDITING_CODE, this, &PlaybackController::toggleHearPlaybackWhenEditing);
    dispatcher()->reg(this, "playback-reload-cache", this, &PlaybackController::reloadPlaybackCache);

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

        m_loadingProgress.start();

        playback()->addSequence().onResolve(this, [this](const TrackSequenceId& sequenceId) {
            setupNewCurrentSequence(sequenceId);
        });
    });

    m_totalPlayTimeChanged.onNotify(this, [this]() {
        updateCurrentTempo();

        updateLoop();
    });

    notationConfiguration()->isMidiInputEnabledChanged().onNotify(this, [this]() {
        notifyActionCheckedChanged(MIDI_ON_CODE);
    });

    configuration()->playNotesWhenEditingChanged().onNotify(this, [this]() {
        notifyActionCheckedChanged(TOGGLE_HEAR_PLAYBACK_WHEN_EDITING_CODE);
    });

    listenAutoProcessOnlineSoundsInBackgroundChanged();

    m_measureInputLag = configuration()->shouldMeasureInputLag();
}

void PlaybackController::updateCurrentTempo()
{
    if (!notationPlayback()) {
        return;
    }

    const Tempo& newTempo = notationPlayback()->multipliedTempo(m_currentTick);

    if (newTempo == m_currentTempo) {
        return;
    }

    m_currentTempo = newTempo;
    m_currentTempoChanged.notify();
}

bool PlaybackController::isPlayAllowed() const
{
    return m_notation != nullptr && m_notation->hasVisibleParts() && isLoaded();
}

Notification PlaybackController::isPlayAllowedChanged() const
{
    return m_isPlayAllowedChanged;
}

bool PlaybackController::isPlaying() const
{
    if (!currentPlayer()) {
        return false;
    }
    return currentPlayer()->playbackStatus() == PlaybackStatus::Running;
}

bool PlaybackController::isPaused() const
{
    if (!currentPlayer()) {
        return false;
    }
    return currentPlayer()->playbackStatus() == PlaybackStatus::Paused;
}

bool PlaybackController::isLoaded() const
{
    return m_loadingTrackCount == 0;
}

bool PlaybackController::isLoopEnabled() const
{
    if (!notationPlayback()) {
        return false;
    }
    return loopBoundariesSet() && notationPlayback()->loopBoundaries().enabled;
}

bool PlaybackController::loopBoundariesSet() const
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

void PlaybackController::seekRawTick(const midi::tick_t tick)
{
    if (m_currentTick == tick) {
        return;
    }

    RetVal<midi::tick_t> playedTick = notationPlayback()->playPositionTickByRawTick(tick);
    if (!playedTick.ret) {
        return;
    }

    seek(playedTickToSecs(playedTick.val));
}

void PlaybackController::seek(const audio::secs_t secs)
{
    IF_ASSERT_FAILED(currentPlayer()) {
        return;
    }

    currentPlayer()->seek(secs);
}

muse::async::Channel<secs_t, tick_t> PlaybackController::currentPlaybackPositionChanged() const
{
    return m_currentPlaybackPositionChanged;
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

Promise<SoundPresetList> PlaybackController::availableSoundPresets(const InstrumentTrackId& instrumentTrackId) const
{
    auto it = m_instrumentTrackIdMap.find(instrumentTrackId);
    if (it == m_instrumentTrackIdMap.end()) {
        return Promise<SoundPresetList>([](auto, auto reject) {
            return reject(static_cast<int>(Ret::Code::UnknownError), "invalid instrumentTrackId");
        });
    }

    const AudioInputParams& params = audioSettings()->trackInputParams(instrumentTrackId);
    return playback()->availableSoundPresets(params.resourceMeta);
}

mu::notation::INotationSoloMuteState::SoloMuteState PlaybackController::trackSoloMuteState(const InstrumentTrackId& trackId) const
{
    return m_notation->soloMuteState()->trackSoloMuteState(trackId);
}

void PlaybackController::setTrackSoloMuteState(const InstrumentTrackId& trackId,
                                               const notation::INotationSoloMuteState::SoloMuteState& state)
{
    if (trackId == notationPlayback()->metronomeTrackId()) {
        if (state.mute != notationConfiguration()->isMetronomeEnabled()) {
            return;
        }

        toggleMetronome();
        return;
    }

    m_notation->soloMuteState()->setTrackSoloMuteState(trackId, state);
}

void PlaybackController::playElements(const std::vector<const notation::EngravingItem*>& elements, bool isMidi)
{
    IF_ASSERT_FAILED(notationPlayback()) {
        return;
    }

    if ((!configuration()->playNotesWhenEditing()) || (isMidi && !configuration()->playNotesOnMidiInput())) {
        return;
    }

    if (m_measureInputLag) {
        START_INPUT_LAG_TIMER;
    }

    std::vector<const notation::EngravingItem*> elementsForPlaying;

    bool playChordWhenEditing = configuration()->playChordWhenEditing();
    bool playHarmonyWhenEditing = configuration()->playHarmonyWhenEditing();

    for (const EngravingItem* element : elements) {
        IF_ASSERT_FAILED(element) {
            continue;
        }

        if (element->isChord() && !playChordWhenEditing) {
            continue;
        }

        if (element->isHarmony() && !playHarmonyWhenEditing) {
            continue;
        }

        elementsForPlaying.push_back(element);
    }

    notationPlayback()->triggerEventsForItems(elementsForPlaying);
}

void PlaybackController::playNotes(const NoteValList& notes, const staff_idx_t staffIdx, const Segment* segment)
{
    Segment* seg = const_cast<Segment*>(segment);
    Chord* chord = engraving::Factory::createChord(seg);
    chord->setParent(seg);

    std::vector<const EngravingItem*> elements;

    for (const NoteVal& nval : notes) {
        Note* note = engraving::Factory::createNote(chord);
        note->setParent(chord);
        note->setStaffIdx(staffIdx);
        note->setNval(nval);
        elements.push_back(note);
    }

    playElements(elements);

    delete chord;
    DeleteAll(elements);
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

    seek(playedTickToSecs(tick.val));
}

void PlaybackController::seekBeat(int measureIndex, int beatIndex)
{
    secs_t targetSecs = beatToSecs(measureIndex, beatIndex);
    seek(targetSecs);
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

    seekRawTick(startTick);
}

void PlaybackController::onAudioResourceChanged(const TrackId trackId, const InstrumentTrackId& instrumentTrackId,
                                                const AudioResourceMeta& oldMeta, const AudioResourceMeta& newMeta)
{
    INotationPlaybackPtr notationPlayback = this->notationPlayback();
    if (!notationPlayback) {
        return;
    }

    if (oldMeta.type == newMeta.type && oldMeta.id == newMeta.id) {
        return;
    }

    if (shouldLoadDrumset(instrumentTrackId, oldMeta, newMeta)) {
        m_drumsetLoader.loadDrumset(m_notation, instrumentTrackId, newMeta);
    }

    notationPlayback->removeSoundFlags({ instrumentTrackId });

    if (audio::isOnlineAudioResource(newMeta)) {
        addToOnlineSounds(trackId);
        tours()->onEvent(u"online_sounds_added");
    } else if (audio::isOnlineAudioResource(oldMeta)) {
        removeFromOnlineSounds(trackId);
    }
}

bool PlaybackController::shouldLoadDrumset(const engraving::InstrumentTrackId& instrumentTrackId,
                                           const AudioResourceMeta& oldMeta, const AudioResourceMeta& newMeta) const
{
    if (oldMeta.type == newMeta.type && oldMeta.id == newMeta.id) {
        return false;
    }

    const Part* part = masterNotationParts()->part(instrumentTrackId.partId);
    const Instrument* instrument = part ? part->instrumentById(instrumentTrackId.instrumentId) : nullptr;
    if (!instrument || !instrument->useDrumset()) {
        return false;
    }

    return oldMeta.type == AudioResourceType::MuseSamplerSoundPack || newMeta.type == AudioResourceType::MuseSamplerSoundPack;
}

void PlaybackController::addSoundFlagsIfNeed(const std::vector<EngravingItem*>& selection)
{
    if (selection.empty()) {
        return;
    }

    std::vector<StaffText*> staffTextList;

    for (EngravingItem* item : selection) {
        if (!item || !item->isStaffText()) {
            continue;
        }

        InstrumentTrackId trackId = mu::engraving::makeInstrumentTrackId(item);
        const AudioInputParams& params = audioSettings()->trackInputParams(trackId);

        bool supportsSoundFlags = params.type() == AudioSourceType::MuseSampler;
        if (supportsSoundFlags) {
            staffTextList.push_back(toStaffText(item));
        }
    }

    if (!staffTextList.empty()) {
        notationPlayback()->addSoundFlags(staffTextList);
    }
}

muse::audio::IPlayerPtr PlaybackController::currentPlayer() const
{
    return m_player;
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
    if (globalContext()->currentMasterNotation() != m_masterNotation) {
        m_masterNotation = globalContext()->currentMasterNotation();
        notifyActionCheckedChanged(LOOP_CODE);
    }

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

void PlaybackController::onPartChanged(const Part* part)
{
    if (!m_notation->hasVisibleParts()) {
        pause();
    }
    m_isPlayAllowedChanged.notify();

    if (!configuration()->muteHiddenInstruments()) {
        return;
    }

    for (const InstrumentTrackId& instrumentTrackId : part->instrumentTrackIdList()) {
        auto soloMuteState = trackSoloMuteState(instrumentTrackId);
        soloMuteState.mute = !part->show();
        setTrackSoloMuteState(instrumentTrackId, soloMuteState);
    }

    if (part->hasChordSymbol()) {
        InstrumentTrackId chordSymbolsTrackId = notationPlayback()->chordSymbolsTrackId(part->id());
        auto chordsSoloMuteState = trackSoloMuteState(chordSymbolsTrackId);
        chordsSoloMuteState.mute = !part->show();
        setTrackSoloMuteState(chordSymbolsTrackId, chordsSoloMuteState);
    }
}

void PlaybackController::onSelectionChanged()
{
    const INotationSelectionPtr selection = this->selection();
    if (!selection || !m_player) {
        return;
    }

    bool selectionTypeChanged = m_isRangeSelection && !selection->isRange();
    m_isRangeSelection = selection->isRange();

    if (!m_isRangeSelection) {
        if (selectionTypeChanged) {
            updateLoop();
            updateSoloMuteStates();
        }

        if (!isLoopEnabled()) {
            seekListSelection();
        }

        addSoundFlagsIfNeed(selection->elements());

        return;
    }

    m_player->resetLoop();

    seekRangeSelection();
    updateSoloMuteStates();
}

void PlaybackController::togglePlay()
{
    if (!isPlayAllowed()) {
        LOGW() << "playback not allowed";
        return;
    }

    if (shouldShowOnlineSoundsConnectionWarning()) {
        showOnlineSoundsConnectionWarning();
        return;
    }

    interaction()->endEditElement();

    if (isPlaying()) {
        pause();
    } else if (isPaused()) {
        if (currentPlayer()) {
            secs_t pos = currentPlayer()->playbackPosition();
            secs_t endSecs = playbackEndSecs();
            if (pos == endSecs) {
                secs_t startSecs = playbackStartSecs();
                seek(startSecs);
            }

            resume();
        }
    } else {
        play();
    }
}

void PlaybackController::play()
{
    IF_ASSERT_FAILED(currentPlayer()) {
        return;
    }
    if (isLoopEnabled()) {
        secs_t startSecs = playbackStartSecs();
        seek(startSecs);
    }

    secs_t delay = 0.;
    if (notationConfiguration()->isCountInEnabled()) {
        if (INotationPlaybackPtr notationPlay = notationPlayback()) {
            secs_t countInDuration = 0.;
            notationPlay->triggerCountIn(m_currentTick, countInDuration);
            delay = playbackDelay(countInDuration);
        }
    }
    currentPlayer()->play(delay);
}

void PlaybackController::rewind(const ActionData& args)
{
    secs_t startSecs = playbackStartSecs();
    secs_t endSecs = playbackEndSecs();
    secs_t newPosition = !args.empty() ? args.arg<secs_t>(0) : secs_t{ 0 };
    newPosition = std::clamp(newPosition, startSecs, endSecs);

    seek(newPosition);
}

void PlaybackController::pause(bool select)
{
    IF_ASSERT_FAILED(currentPlayer()) {
        return;
    }

    if (isPaused()) {
        return;
    }

    currentPlayer()->pause();

    if (select) {
        selectAtRawTick(m_currentTick);
    }
}

void PlaybackController::stop()
{
    IF_ASSERT_FAILED(currentPlayer()) {
        return;
    }

    currentPlayer()->stop();
}

void PlaybackController::resume()
{
    IF_ASSERT_FAILED(currentPlayer()) {
        return;
    }

    secs_t delay = 0.;
    if (notationConfiguration()->isCountInEnabled()) {
        if (INotationPlaybackPtr notationPlay = notationPlayback()) {
            secs_t countInDuration = 0.;
            notationPlay->triggerCountIn(m_currentTick, countInDuration);
            delay = playbackDelay(countInDuration);
        }
    }

    currentPlayer()->resume(delay);
}

void PlaybackController::selectAtRawTick(const tick_t& rawTick)
{
    if (!m_notation) {
        return;
    }

    const RetVal<tick_t> playPositionTick = notationPlayback()->playPositionTickByRawTick(rawTick);
    if (!playPositionTick.ret) {
        return;
    }

    const Fraction playPositionFrac = Fraction::fromTicks(playPositionTick.val);
    interaction()->findAndSelectChordRest(playPositionFrac);
}

secs_t PlaybackController::playbackStartSecs() const
{
    if (!m_notation) {
        return 0;
    }

    const LoopBoundaries& loop = notationPlayback()->loopBoundaries();
    if (loop.enabled) {
        // Convert from raw ticks (visual tick != playback tick due to repeats etc)
        RetVal<tick_t> startTick = notationPlayback()->playPositionTickByRawTick(loop.loopInTick);
        if (!startTick.ret) {
            return 0;
        }
        return playedTickToSecs(startTick.val);
    }

    return 0;
}

secs_t PlaybackController::playbackEndSecs() const
{
    return notationPlayback() ? notationPlayback()->totalPlayTime() : secs_t { 0 };
}

secs_t PlaybackController::playbackDelay(const secs_t countInDuration) const
{
    if (!notationConfiguration()->isMetronomeEnabled()) {
        return countInDuration;
    }

    const MeasureBeat beat = currentBeat();
    const secs_t closestMainBeatPosition = beatToSecs(beat.measureIndex, std::ceil(beat.beat));
    const secs_t playbackPosition = currentPlayer()->playbackPosition();
    const secs_t delay = countInDuration - (closestMainBeatPosition - playbackPosition);
    IF_ASSERT_FAILED(!delay.is_negative()) {
        return 0.;
    }

    return delay;
}

InstrumentTrackIdSet PlaybackController::instrumentTrackIdSetForRangePlayback() const
{
    std::vector<const Part*> selectedParts = selectionRange()->selectedParts();
    Fraction startTick = selectionRange()->startTick();
    int startTicks = startTick.ticks();

    InstrumentTrackIdSet result;

    for (const Part* part : selectedParts) {
        if (const Instrument* startInstrument = part->instrument(startTick)) {
            result.insert({ part->id(), startInstrument->id() });
        }

        for (auto [tick, instrument] : part->instruments()) {
            if (tick > startTicks) {
                result.insert({ part->id(), instrument->id() });
            }
        }

        if (part->hasChordSymbol()) {
            result.insert(notationPlayback()->chordSymbolsTrackId(part->id()));
        }
    }

    return result;
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
    bool countInEnabled = notationConfiguration()->isCountInEnabled();

    notationConfiguration()->setIsMetronomeEnabled(!metronomeEnabled);
    notifyActionCheckedChanged(METRONOME_CODE);

    setTrackActivity(notationPlayback()->metronomeTrackId(), !metronomeEnabled || countInEnabled);
}

void PlaybackController::toggleCountIn()
{
    bool metronomeEnabled = notationConfiguration()->isMetronomeEnabled();
    bool countInEnabled = notationConfiguration()->isCountInEnabled();

    notationConfiguration()->setIsCountInEnabled(!countInEnabled);
    notifyActionCheckedChanged(COUNT_IN_CODE);

    setTrackActivity(notationPlayback()->metronomeTrackId(), metronomeEnabled || !countInEnabled);
}

void PlaybackController::toggleMidiInput()
{
    bool wasMidiInputEnabled = notationConfiguration()->isMidiInputEnabled();
    notationConfiguration()->setIsMidiInputEnabled(!wasMidiInputEnabled);
}

void PlaybackController::setMidiUseWrittenPitch(bool useWrittenPitch)
{
    notationConfiguration()->setMidiUseWrittenPitch(useWrittenPitch);
    notifyActionCheckedChanged(INPUT_WRITTEN_PITCH);
    notifyActionCheckedChanged(INPUT_SOUNDING_PITCH);
}

void PlaybackController::toggleLoopPlayback()
{
    if (isLoopEnabled()) {
        disableLoop();
        return;
    }

    if (loopBoundariesSet() && !selection()->isRange()) {
        enableLoop();
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

void PlaybackController::toggleHearPlaybackWhenEditing()
{
    bool wasPlayNotesWhenEditing = configuration()->playNotesWhenEditing();
    configuration()->setPlayNotesWhenEditing(!wasPlayNotesWhenEditing);
}

void PlaybackController::reloadPlaybackCache()
{
    INotationPlaybackPtr nPlayback = notationPlayback();
    if (nPlayback) {
        nPlayback->reload();
    }
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
        enableLoop();
    }
}

void PlaybackController::updateLoop()
{
    if (!notationPlayback() || !currentPlayer()) {
        return;
    }

    const LoopBoundaries& boundaries = notationPlayback()->loopBoundaries();

    if (!boundaries.enabled) {
        disableLoop();
        return;
    }

    // Convert from raw ticks (visual tick != playback tick due to repeats etc)
    RetVal<tick_t> playbackTickFrom = notationPlayback()->playPositionTickByRawTick(boundaries.loopInTick);
    RetVal<tick_t> playbackTickTo = notationPlayback()->playPositionTickByRawTick(boundaries.loopOutTick);
    if (!playbackTickFrom.ret || !playbackTickTo.ret) {
        return;
    }

    secs_t fromSecs = playedTickToSecs(playbackTickFrom.val);
    secs_t toSecs = playedTickToSecs(playbackTickTo.val);
    currentPlayer()->setLoop(secsToMilisecs(fromSecs), secsToMilisecs(toSecs));

    enableLoop();

    notifyActionCheckedChanged(LOOP_CODE);
}

void PlaybackController::enableLoop()
{
    if (notationPlayback()) {
        notationPlayback()->setLoopBoundariesEnabled(true);
    }
}

void PlaybackController::disableLoop()
{
    IF_ASSERT_FAILED(notationPlayback() && currentPlayer()) {
        return;
    }

    currentPlayer()->resetLoop();
    notationPlayback()->setLoopBoundariesEnabled(false);

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
    if (currentPlayer()) {
        currentPlayer()->playbackPositionChanged().resetOnReceive(this);
        currentPlayer()->playbackStatusChanged().resetOnReceive(this);
    }

    playback()->clearSources();
    playback()->inputParamsChanged().resetOnReceive(this);

    playback()->clearAllFx();
    playback()->outputParamsChanged().resetOnReceive(this);
    playback()->masterOutputParamsChanged().resetOnReceive(this);
    playback()->clearMasterOutputParams();

    m_currentTick = 0;

    playback()->removeSequence(m_currentSequenceId);

    m_instrumentTrackIdMap.clear();
    m_auxTrackIdMap.clear();

    m_isRangeSelection = false;

    m_currentSequenceId = -1;
    m_currentSequenceIdChanged.notify();

    m_player = nullptr;
    globalContext()->setCurrentPlayer(nullptr);

    const bool hadOnlineSounds = !m_onlineSounds.empty();
    m_onlineSounds.clear();
    m_onlineSoundsBeingProcessed.clear();
    m_onlineSoundsProcessingErrorCode = 0;

    if (hadOnlineSounds) {
        m_onlineSoundsChanged.notify();
    }
}

void PlaybackController::addTrack(const InstrumentTrackId& instrumentTrackId, const TrackAddFinished& onFinished)
{
    if (notationPlayback()->metronomeTrackId() == instrumentTrackId) {
        doAddTrack(instrumentTrackId, muse::trc("playback", "Metronome"), onFinished);
        return;
    }

    const Part* part = masterNotationParts()->part(instrumentTrackId.partId);
    if (!part) {
        return;
    }

    if (notationPlayback()->isChordSymbolsTrack(instrumentTrackId)) {
        const std::string trackName = muse::trc("playback", "Chords") + "." + part->partName().toStdString();
        doAddTrack(instrumentTrackId, trackName, onFinished);
        return;
    }

    const muse::String primaryInstrId = part->instrument()->id();
    if (instrumentTrackId.instrumentId == primaryInstrId) {
        const std::string trackName = part->partName().toStdString();
        doAddTrack(instrumentTrackId, trackName, onFinished);
        return;
    }

    const Instrument* instrument = part->instrumentById(instrumentTrackId.instrumentId);
    if (instrument != nullptr) {
        std::string trackName = "(" + instrument->trackName().toStdString() + ")";
        doAddTrack(instrumentTrackId, trackName, onFinished);
    }
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
    AudioResourceMeta originMeta = inParams.resourceMeta;

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
        const muse::String& instrumentSoundId = inParams.resourceMeta.attributeVal(PLAYBACK_SETUP_DATA_ATTRIBUTE);
        AudioSourceType sourceType = inParams.isValid() ? inParams.type() : AudioSourceType::Fluid;

        for (aux_channel_idx_t idx = 0; idx < AUX_CHANNEL_NUM; ++idx) {
            gain_t signalAmount = configuration()->defaultAuxSendValue(idx, sourceType, instrumentSoundId);
            outParams.auxSends.emplace_back(AuxSendParams { signalAmount, true });
        }
    }

    uint64_t playbackKey = notationPlaybackKey();

    playback()->addTrack(m_currentSequenceId, title, std::move(playbackData), { std::move(inParams), std::move(outParams) })
    .onResolve(this, [this, instrumentTrackId, playbackKey, onFinished, originMeta](const TrackId trackId,
                                                                                    const AudioParams& appliedParams) {
        //! NOTE It may be that while we were adding a track, the notation was already closed (or opened another)
        //! This situation can be if the notation was opened and immediately closed.
        if (notationPlaybackKey() != playbackKey) {
            return;
        }

        m_instrumentTrackIdMap.insert({ instrumentTrackId, trackId });

        const bool trackNewlyAdded = !audioSettings()->trackHasExistingOutputParams(instrumentTrackId);
        audioSettings()->setTrackInputParams(instrumentTrackId, appliedParams.in);
        audioSettings()->setTrackOutputParams(instrumentTrackId, appliedParams.out);

        updateSoloMuteStates();

        onFinished();

        m_trackAdded.send(trackId);

        if (trackNewlyAdded) {
            onTrackNewlyAdded(instrumentTrackId);
        }

        if (shouldLoadDrumset(instrumentTrackId, originMeta, appliedParams.in.resourceMeta)) {
            m_drumsetLoader.loadDrumset(m_notation, instrumentTrackId, appliedParams.in.resourceMeta);
        }

        if (muse::audio::isOnlineAudioResource(appliedParams.in.resourceMeta)) {
            addToOnlineSounds(trackId);
        }
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

    std::string title = resolveAuxTrackTitle(index, outParams, false);
    uint64_t playbackKey = notationPlaybackKey();

    playback()->addAuxTrack(m_currentSequenceId, title, outParams)
    .onResolve(this, [this, playbackKey, index, onFinished](const TrackId trackId, const AudioOutputParams& appliedParams) {
        //! NOTE It may be that while we were adding a track, the notation was already closed (or opened another)
        //! This situation can be if the notation was opened and immediately closed.
        if (notationPlaybackKey() != playbackKey) {
            return;
        }

        m_auxTrackIdMap.insert({ index, trackId });

        audioSettings()->setAuxOutputParams(index, appliedParams);

        updateSoloMuteStates();
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
    playback()->setOutputParams(m_currentSequenceId, trackId, std::move(outParams));
}

AudioOutputParams PlaybackController::trackOutputParams(const InstrumentTrackId& instrumentTrackId) const
{
    IF_ASSERT_FAILED(audioSettings() && notationConfiguration() && notationPlayback()) {
        return {};
    }

    AudioOutputParams result = audioSettings()->trackOutputParams(instrumentTrackId);

    if (instrumentTrackId == notationPlayback()->metronomeTrackId()) {
        result.muted = !notationConfiguration()->isMetronomeEnabled() && !notationConfiguration()->isCountInEnabled();
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

    updateSoloMuteStates();
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

    playback()->removeTrack(m_currentSequenceId, search->second);
    audioSettings()->removeTrackParams(instrumentTrackId);

    m_masterNotation->notation()->soloMuteState()->removeTrackSoloMuteState(instrumentTrackId);
    for (const IExcerptNotationPtr& excerpt : m_masterNotation->excerpts()) {
        if (const INotationPtr& notation = excerpt->notation()) {
            notation->soloMuteState()->removeTrackSoloMuteState(instrumentTrackId);
        }
    }

    m_trackRemoved.send(search->second);
    m_instrumentTrackIdMap.erase(instrumentTrackId);

    removeFromOnlineSounds(search->second);
}

void PlaybackController::onTrackNewlyAdded(const InstrumentTrackId& instrumentTrackId)
{
    for (const IExcerptNotationPtr& excerpt : m_masterNotation->excerpts()) {
        if (const INotationPtr& notation = excerpt->notation()) {
            if (notation == m_notation || notation->soloMuteState()->trackSoloMuteStateExists(instrumentTrackId)) {
                continue;
            }
            const INotationSoloMuteState::SoloMuteState soloMuteState = { /*mute*/ true, /*solo*/ false };
            notation->soloMuteState()->setTrackSoloMuteState(instrumentTrackId, soloMuteState);
        }
    }
}

void PlaybackController::addToOnlineSounds(const TrackId trackId)
{
    if (muse::contains(m_onlineSounds, trackId)) {
        return;
    }

    m_onlineSounds.insert(trackId);
    listenOnlineSoundsProcessingProgress(trackId);
    m_onlineSoundsChanged.notify();
}

void PlaybackController::removeFromOnlineSounds(const TrackId trackId)
{
    if (!muse::contains(m_onlineSounds, trackId)) {
        return;
    }

    muse::remove(m_onlineSounds, trackId);
    muse::remove(m_onlineSoundsBeingProcessed, trackId);

    if (m_onlineSoundsProcessingProgress.isStarted() && m_onlineSoundsBeingProcessed.empty()) {
        m_onlineSoundsProcessingProgress.finish(Ret(m_onlineSoundsProcessingErrorCode));
    }

    m_onlineSoundsChanged.notify();
}

void PlaybackController::listenOnlineSoundsProcessingProgress(const TrackId trackId)
{
    playback()->inputProcessingProgress(m_currentSequenceId, trackId)
    .onResolve(this, [this, trackId](muse::audio::InputProcessingProgress inputProgress) {
        inputProgress.progress.started().onNotify(this, [this, trackId]() {
            m_onlineSoundsBeingProcessed.insert(trackId);

            if (!m_onlineSoundsProcessingProgress.isStarted()) {
                m_onlineSoundsProcessingErrorCode = 0;
                m_onlineSoundsProcessingProgress.start();
            }
        });

        inputProgress.progress.progressChanged().onReceive(this, [this](int64_t current, int64_t total, const std::string& msg) {
            if (m_onlineSoundsBeingProcessed.size() == 1) {
                m_onlineSoundsProcessingProgress.progress(current, total, msg);
            }
        });

        inputProgress.progress.finished().onReceive(this, [this, trackId](const muse::ProgressResult& res) {
            muse::remove(m_onlineSoundsBeingProcessed, trackId);

            if (m_onlineSoundsProcessingErrorCode == 0 && res.ret.code() != static_cast<int>(Ret::Code::Cancel)) {
                m_onlineSoundsProcessingErrorCode = res.ret.code();
            }

            if (m_onlineSoundsBeingProcessed.empty()) {
                m_onlineSoundsProcessingProgress.finish(Ret(m_onlineSoundsProcessingErrorCode));
            }
        });
    });
}

void PlaybackController::listenAutoProcessOnlineSoundsInBackgroundChanged()
{
    audioConfiguration()->autoProcessOnlineSoundsInBackgroundChanged().onReceive(this, [this](bool value) {
        if (value) {
            return;
        }

        const Uri preferencesUri("muse://preferences");
        const String toursEventCode(u"online_sounds_auto_process_disabled");

        if (!interactive()->isOpened(preferencesUri).val) {
            tours()->onEvent(toursEventCode);
            return;
        }

        async::Channel<Uri> currentUriChanged = interactive()->currentUri().ch;
        currentUriChanged.onReceive(this, [=](const Uri&) {
            if (!audioConfiguration()->autoProcessOnlineSoundsInBackground()
                && !interactive()->isOpened(preferencesUri).val) {
                async::Async::call(this, [=]() {
                    tours()->onEvent(toursEventCode);
                });

                async::Channel<Uri> mut = currentUriChanged;
                mut.resetOnReceive(this);
            }
        });
    });
}

bool PlaybackController::shouldShowOnlineSoundsConnectionWarning() const
{
    if (m_onlineSoundsProcessingErrorCode == static_cast<int>(muse::audio::Err::OnlineSoundsNetworkError)) {
        return configuration()->needToShowOnlineSoundsConnectionWarning() && !isPlaying();
    }

    return false;
}

void PlaybackController::showOnlineSoundsConnectionWarning()
{
    const std::string text = muse::trc("playback", "This may be due to a poor internet connection or server issue. Your score will still play, "
                                                   "but some sounds may be missing. Please check your internet connection or try again later.");

    auto promise = interactive()->warning(muse::trc("playback", "Some online sounds aren’t ready yet"), text,
                                          { IInteractive::Button::Ok }, IInteractive::Button::Ok,
                                          IInteractive::Option::WithIcon | IInteractive::Option::WithDontShowAgainCheckBox);

    m_onlineSoundsProcessingErrorCode = 0;

    promise.onResolve(this, [this](const IInteractive::Result& res) {
        if (!res.showAgain()) {
            configuration()->setNeedToShowOnlineSoundsConnectionWarning(false);
        }

        togglePlay();
    });
}

void PlaybackController::setupNewCurrentSequence(const TrackSequenceId sequenceId)
{
    playback()->removeAllTracks(m_currentSequenceId);

    m_currentSequenceId = sequenceId;
    m_player = playback()->player(sequenceId);
    globalContext()->setCurrentPlayer(m_player);

    if (!notationPlayback()) {
        return;
    }

    const audio::AudioOutputParams& masterOutputParams = audioSettings()->masterAudioOutputParams();
    playback()->setMasterOutputParams(masterOutputParams);

    subscribeOnAudioParamsChanges();
    setupSequenceTracks();
    setupSequencePlayer();

    m_currentSequenceIdChanged.notify();
}

void PlaybackController::subscribeOnAudioParamsChanges()
{
    playback()->masterOutputParamsChanged().onReceive(this, [this](const audio::AudioOutputParams& params) {
        audioSettings()->setMasterAudioOutputParams(params);
    });

    playback()->inputParamsChanged().onReceive(this,
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
            const AudioResourceMeta& oldMeta = audioSettings()->trackInputParams(search->first).resourceMeta;
            onAudioResourceChanged(trackId, search->first, oldMeta, params.resourceMeta);

            audioSettings()->setTrackInputParams(search->first, params);
        }
    });

    playback()->outputParamsChanged().onReceive(this,
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
    std::string title = muse::trc("playback", "Loading audio samples");

    auto onAddFinished = [this, trackCount, title]() {
        m_loadingTrackCount--;

        size_t current = trackCount - m_loadingTrackCount;
        m_loadingProgress.progress(current, trackCount, title);

        if (m_loadingTrackCount == 0) {
            m_loadingProgress.finish(muse::make_ok());
            m_isPlayAllowedChanged.notify();
        }
    };

    for (const InstrumentTrackId& trackId : trackIdSet) {
        addTrack(trackId, onAddFinished);
    }

    for (aux_channel_idx_t idx = 0; idx < AUX_CHANNEL_NUM; ++idx) {
        addAuxTrack(idx, onAddFinished);
    }

    m_loadingProgress.progress(0, trackCount, title);

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

        updateSoloMuteStates();
    });

    audioSettings()->auxSoloMuteStateChanged().onReceive(
        this, [this](aux_channel_idx_t, const notation::INotationSoloMuteState::SoloMuteState&) {
        updateSoloMuteStates();
    });

    m_isPlayAllowedChanged.notify();
}

void PlaybackController::setupSequencePlayer()
{
    currentPlayer()->playbackPositionChanged().onReceive(this, [this](const audio::secs_t pos) {
        m_currentTick = notationPlayback()->secToTick(pos);
        m_currentPlaybackPositionChanged.send(pos, m_currentTick);

        updateCurrentTempo();

        secs_t endSecs = playbackEndSecs();
        if (pos + milisecsToSecs(1) >= endSecs) {
            stop();
        }
    });

    currentPlayer()->playbackStatusChanged().onReceive(this, [this](PlaybackStatus) {
        m_isPlayingChanged.notify();
    });

    currentPlayer()->setDuration(secsToMilisecs(notationPlayback()->totalPlayTime()));

    notationPlayback()->totalPlayTimeChanged().onReceive(this, [this](const audio::secs_t totalPlaybackTime) {
        currentPlayer()->setDuration(secsToMilisecs(totalPlaybackTime));
        m_totalPlayTimeChanged.notify();
    });
}

void PlaybackController::updateSoloMuteStates()
{
    if (!audioSettings() || !playback() || !m_notation) {
        return;
    }

    TRACEFUNC;

    InstrumentTrackIdSet existingTrackIdSet = notationPlayback()->existingTrackIdSet();
    bool hasSolo = false;

    for (const InstrumentTrackId& instrumentTrackId : existingTrackIdSet) {
        if (instrumentTrackId == notationPlayback()->metronomeTrackId()) {
            continue;
        }
        if (m_notation->soloMuteState()->trackSoloMuteState(instrumentTrackId).solo) {
            hasSolo = true;
            break;
        }
    }

    InstrumentTrackIdSet allowedInstrumentTrackIdSet = instrumentTrackIdSetForRangePlayback();
    bool isRangePlaybackMode = !m_isExportingAudio && selection()->isRange() && !allowedInstrumentTrackIdSet.empty();

    for (const InstrumentTrackId& instrumentTrackId : existingTrackIdSet) {
        if (!muse::contains(m_instrumentTrackIdMap, instrumentTrackId)) {
            continue;
        }

        if (instrumentTrackId == notationPlayback()->metronomeTrackId()) {
            continue;
        }

        // 1. Recall the solo-mute state for this notation
        auto soloMuteState = m_notation->soloMuteState()->trackSoloMuteState(instrumentTrackId);

        // 2. Evaluate "force mute" (disabling the mute button)
        bool shouldForceMute = hasSolo && !soloMuteState.solo;
        if (notationPlayback()->isChordSymbolsTrack(instrumentTrackId) && !shouldForceMute) {
            shouldForceMute = !notationConfiguration()->isPlayChordSymbolsEnabled();
        }

        if (isRangePlaybackMode && !shouldForceMute) {
            shouldForceMute = !muse::contains(allowedInstrumentTrackIdSet, instrumentTrackId);
        }

        // 3. Update params for playback / mixer
        AudioOutputParams params = trackOutputParams(instrumentTrackId);
        params.solo = soloMuteState.solo;
        params.muted = soloMuteState.mute || shouldForceMute;
        params.forceMute = shouldForceMute;

        audio::TrackId trackId = m_instrumentTrackIdMap.at(instrumentTrackId);
        playback()->setOutputParams(m_currentSequenceId, trackId, std::move(params));
    }

    updateAuxMuteStates();
}

void PlaybackController::updateAuxMuteStates()
{
    for (const auto& pair : m_auxTrackIdMap) {
        auto soloMuteState = audioSettings()->auxSoloMuteState(pair.first);

        AudioOutputParams params = audioSettings()->auxOutputParams(pair.first);
        if (params.muted == soloMuteState.mute) {
            continue;
        }

        params.muted = soloMuteState.mute;
        playback()->setOutputParams(m_currentSequenceId, pair.second, std::move(params));
    }
}

bool PlaybackController::actionChecked(const ActionCode& actionCode) const
{
    QMap<std::string, bool> isChecked {
        { LOOP_CODE, isLoopEnabled() },
        { MIDI_ON_CODE, notationConfiguration()->isMidiInputEnabled() },
        { INPUT_WRITTEN_PITCH, notationConfiguration()->midiUseWrittenPitch().val },
        { INPUT_SOUNDING_PITCH, !notationConfiguration()->midiUseWrittenPitch().val },
        { REPEAT_CODE, notationConfiguration()->isPlayRepeatsEnabled() },
        { PLAY_CHORD_SYMBOLS_CODE, notationConfiguration()->isPlayChordSymbolsEnabled() },
        { PAN_CODE, notationConfiguration()->isAutomaticallyPanEnabled() },
        { METRONOME_CODE, notationConfiguration()->isMetronomeEnabled() },
        { COUNT_IN_CODE, notationConfiguration()->isCountInEnabled() },
        { TOGGLE_HEAR_PLAYBACK_WHEN_EDITING_CODE, configuration()->playNotesWhenEditing() }
    };

    return isChecked[actionCode];
}

Channel<ActionCode> PlaybackController::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}

QTime PlaybackController::totalPlayTime() const
{
    if (!notationPlayback()) {
        return ZERO_TIME;
    }

    return timeFromSeconds(notationPlayback()->totalPlayTime());
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

secs_t PlaybackController::beatToSecs(int measureIndex, int beatIndex) const
{
    if (!notationPlayback()) {
        return 0;
    }

    muse::midi::tick_t rawTick = notationPlayback()->beatToRawTick(measureIndex, beatIndex);
    muse::midi::tick_t playedTick = notationPlayback()->playPositionTickByRawTick(rawTick).val;

    return playedTickToSecs(playedTick);
}

double PlaybackController::tempoMultiplier() const
{
    return notationPlayback() ? notationPlayback()->tempoMultiplier() : 1.0;
}

void PlaybackController::setTempoMultiplier(double multiplier)
{
    if (!notationPlayback()) {
        return;
    }

    tick_t tick = m_currentTick;
    bool playing = isPlaying();

    if (playing) {
        pause();
    }

    notationPlayback()->setTempoMultiplier(multiplier);
    seekRawTick(tick);
    updateLoop();

    if (playing) {
        resume();
    }
}

muse::Progress PlaybackController::loadingProgress() const
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

    notationPlayback()->removeSoundFlags(notationPlayback()->existingTrackIdSet());

    const InstrumentTrackId& metronomeTrackId = notationPlayback()->metronomeTrackId();

    for (const auto& pair : m_instrumentTrackIdMap) {
        if (pair.first == metronomeTrackId) {
            continue;
        }

        const mpe::PlaybackData& playbackData = notationPlayback()->trackPlaybackData(pair.first);
        AudioInputParams newInputParams { profile.findResource(playbackData.setupData), {} };

        playback()->setInputParams(m_currentSequenceId, pair.second, std::move(newInputParams));
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

    if (!m_notation->hasVisibleParts()) {
        pause();
    }
    m_isPlayAllowedChanged.notify();

    // All invisible tracks should be muted in newly opened notations (initNotationSoloMuteState)
    // Once the mute state has been edited, this "custom state" will be recalled from then onwards
    bool emptyMuteStates = true;
    InstrumentTrackIdSet existingTrackIdSet = notationPlayback()->existingTrackIdSet();
    for (const InstrumentTrackId& instrumentTrackId : existingTrackIdSet) {
        if (m_notation->soloMuteState()->trackSoloMuteStateExists(instrumentTrackId)) {
            emptyMuteStates = false;
            break;
        }
    }

    if (emptyMuteStates) {
        m_masterNotation->initNotationSoloMuteState(notation);
    }

    updateSoloMuteStates();

    INotationPartsPtr notationParts = m_notation->parts();
    NotifyList<const Part*> partList = notationParts->partList();

    partList.onItemAdded(this, [this](const Part* part) {
        onPartChanged(part);
    });

    partList.onItemChanged(this, [this](const Part* part) {
        onPartChanged(part);
    });

    notationPlayback()->loopBoundariesChanged().onNotify(this, [this]() {
        updateLoop();
    });

    m_notation->interaction()->selectionChanged().onNotify(this, [this]() {
        onSelectionChanged();
    });

    m_notation->interaction()->textEditingEnded().onReceive(this, [this](engraving::TextBase* text) {
        if (text && text->isHarmony()) {
            playElements({ text });
        }
    });

    m_notation->soloMuteState()->trackSoloMuteStateChanged().onReceive(
        this, [this](const InstrumentTrackId&, const notation::INotationSoloMuteState::SoloMuteState&) {
        updateSoloMuteStates();
    });
}

void PlaybackController::setIsExportingAudio(bool exporting)
{
    if (m_isExportingAudio == exporting) {
        return;
    }

    m_isExportingAudio = exporting;
    updateSoloMuteStates();

    if (!m_onlineSounds.empty() && !audioConfiguration()->autoProcessOnlineSoundsInBackground()) {
        dispatcher()->dispatch("process-online-sounds");
    }
}

bool PlaybackController::canReceiveAction(const ActionCode&) const
{
    return m_masterNotation != nullptr && m_masterNotation->hasParts();
}

const std::set<TrackId>& PlaybackController::onlineSounds() const
{
    return m_onlineSounds;
}

muse::async::Notification PlaybackController::onlineSoundsChanged() const
{
    return m_onlineSoundsChanged;
}

muse::Progress PlaybackController::onlineSoundsProcessingProgress() const
{
    return m_onlineSoundsProcessingProgress;
}

muse::audio::secs_t PlaybackController::playedTickToSecs(int tick) const
{
    return secs_t(notationPlayback()->playedTickToSec(tick));
}
