/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "async/notifylist.h"
#include "containers.h"
#include "modularity/ioc.h"
#include "log.h"
#include "rcommand/commandtypes.h"
#include "types/ret.h"

#include "audio/common/audioutils.h"
#include "audio/devtools/inputlag.h"

#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/utils.h"

#include "notation/iexcerptnotation.h" // IWYU pragma: keep
#include "notation/imasternotation.h"
#include "notation/inotationinteraction.h"
#include "notation/inotationnoteinput.h" // IWYU pragma: keep
#include "notation/inotationparts.h"
#include "notation/inotationselection.h"

#include "project/inotationproject.h"

#include "../playbacktypes.h"
#include "../playbackcommands.h"
#include "onlinesoundscontroller.h"

using namespace muse;
using namespace muse::actions;
using namespace muse::async;
using namespace muse::audio;
using namespace muse::midi;
using namespace mu::engraving;
using namespace mu::notation;
using namespace mu::playback;
using namespace mu::project;

static AudioOutputParams makeReverbOutputParams()
{
    AudioFxParams reverbParams;
    reverbParams.resourceMeta = makeReverbMeta();
    reverbParams.categories.insert(AudioFxCategory::FxReverb);
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

PlaybackController::PlaybackController(const muse::modularity::ContextPtr& iocCtx)
    : muse::Contextable(iocCtx), m_onlineSoundsController(std::make_unique<OnlineSoundsController>(iocCtx))
{
}

PlaybackController::~PlaybackController() = default;

void PlaybackController::init()
{
    m_onlineSoundsController->regActions();

    auto d = commandsDispatcher();

    d->onRequest(this, PLAY_TOGGLE_COMMAND, [this]() { return togglePlay(); });
    d->onRequest(this, PLAY_COMMAND, [this]() { return play(); });
    d->onRequest(this, PLAY_SELECTION_COMMAND, [this]() { return playFromSelection(); });
    d->onRequest(this, PAUSE_COMMAND, [this]() { return pause(); });
    d->onRequest(this, PAUSE_AND_SELECT_COMMAND, [this]() { return pause(true); });
    d->onRequest(this, STOP_COMMAND, [this]() { return stop(); });
    d->onRequest(this, REWIND_COMMAND, [this](const rcommand::Request& request) { return rewind(request); });
    d->onRequest(this, LOOP_TOGGLE_COMMAND, [this]() { return toggleLoopPlayback(); });
    d->onRequest(this, LOOP_IN_COMMAND, [this]() { return addLoopBoundary(LoopBoundaryType::LoopIn); });
    d->onRequest(this, LOOP_OUT_COMMAND, [this]() { return addLoopBoundary(LoopBoundaryType::LoopOut); });
    d->onRequest(this, METRONOME_TOGGLE_COMMAND, [this]() { return toggleMetronome(); });
    d->onRequest(this, SHOW_PLAYBACK_SETUP_COMMAND, [this]() { return showPlaybackSetup(); });
    d->onRequest(this, MIDI_TOGGLE_COMMAND, [this]() { return toggleMidiInput(); });
    d->onRequest(this, MIDI_INPUT_WRITTEN_PITCH_COMMAND, [this]() { return setMidiUseWrittenPitch(true); });
    d->onRequest(this, MIDI_INPUT_SOUNDING_PITCH_COMMAND, [this]() { return setMidiUseWrittenPitch(false); });
    d->onRequest(this, REPEATS_TOGGLE_COMMAND, [this]() { return togglePlayRepeats(); });
    d->onRequest(this, CHORDSYMBOLS_TOGGLE_COMMAND, [this]() { return togglePlayChordSymbols(); });
    d->onRequest(this, HEAR_PLAYBACK_WHEN_EDITING_TOGGLE_COMMAND, [this]() { return toggleHearPlaybackWhenEditing(); });
    d->onRequest(this, PAN_TOGGLE_COMMAND, [this]() { return toggleAutomaticallyPan(); });
    d->onRequest(this, COUNTIN_TOGGLE_COMMAND, [this]() { return toggleCountIn(); });
    d->onRequest(this, RELOAD_PLAYBACK_CACHE_COMMAND, [this]() { return reloadPlaybackCache(); });

    // compat
    {
        static std::map<ActionCode, rcommand::Command> actionToCommand = {
            { "play", PLAY_TOGGLE_COMMAND },
            { "play-from-selection", PLAY_SELECTION_COMMAND },
            { "pause", PAUSE_COMMAND },
            { "pause-and-select", PAUSE_AND_SELECT_COMMAND },
            { "stop", STOP_COMMAND },
            { "rewind", REWIND_COMMAND },
            { "loop", LOOP_TOGGLE_COMMAND },
            { "loop-in", LOOP_IN_COMMAND },
            { "loop-out", LOOP_OUT_COMMAND },
            { "metronome", METRONOME_TOGGLE_COMMAND },
            { "playback-setup", SHOW_PLAYBACK_SETUP_COMMAND },
            { "midi-on", MIDI_TOGGLE_COMMAND },
            { "midi-input-written-pitch", MIDI_INPUT_WRITTEN_PITCH_COMMAND },
            { "midi-input-sounding-pitch", MIDI_INPUT_SOUNDING_PITCH_COMMAND },
            { "repeats", REPEATS_TOGGLE_COMMAND },
            { "play-chord-symbols", CHORDSYMBOLS_TOGGLE_COMMAND },
            { "toggle-hear-playback-when-editing", HEAR_PLAYBACK_WHEN_EDITING_TOGGLE_COMMAND },
            { "pan", PAN_TOGGLE_COMMAND },
            { "countin", COUNTIN_TOGGLE_COMMAND },
            { "reload-playback-cache", RELOAD_PLAYBACK_CACHE_COMMAND },
            { "clear-online-sounds-cache", CLEAR_ONLINESOUNDS_CACHE_COMMAND },
        };

        auto ad = dispatcher();
        for (const auto& [actionCode, command] : actionToCommand) {
            ad->reg(this, actionCode, [d, command]() { return d->dispatch(command); });
        }
    }

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });

    globalContext()->currentProjectChanged().onNotify(this, [this]() {
        if (m_isPlaybackInited) {
            resetPlayback();
        }

        if (!globalContext()->currentProject()) {
            return;
        }

        m_loadingProgress.start();

        playback()->init().onResolve(this, [this](const Ret& ret) {
            if (ret) {
                setupPlayback();
            }
        });
    });

    m_totalPlayTimeChanged.onNotify(this, [this]() {
        updateCurrentTempo();

        updateLoop();
    });

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
    if (!m_isPlaybackInited) {
        return false;
    }

    if (!m_notation) {
        return false;
    }

    if (!m_masterNotation) {
        return false;
    }

    const MasterScore* masterScore = m_masterNotation->masterScore();
    if (!masterScore) {
        return false;
    }

    if (!masterScore->firstMeasure()) {
        return false;
    }

    if (!m_notation->hasVisibleParts()) {
        return false;
    }

    if (!isLoaded()) {
        return false;
    }

    return true;
}

Channel<bool> PlaybackController::isPlayAllowedChanged() const
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

Channel<bool> PlaybackController::isPlayingChanged() const
{
    return m_isPlayingChanged;
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
    return notationPlayback() && notationPlayback()->isLoopEnabled();
}

Channel<bool> PlaybackController::loopEnabledChanged() const
{
    return m_loopEnabledChanged;
}

bool PlaybackController::loopBoundariesSet() const
{
    return notationPlayback() && !notationPlayback()->loopBoundaries().isNull();
}

void PlaybackController::seekRawTick(const midi::tick_t tick, const bool flushSound)
{
    if (m_currentTick == tick) {
        return;
    }

    RetVal<midi::tick_t> playedTick = notationPlayback()->playPositionTickByRawTick(tick);
    if (!playedTick.ret) {
        return;
    }

    seek(playedTickToSecs(playedTick.val), flushSound);
}

void PlaybackController::seek(const audio::secs_t secs, const bool flushSound)
{
    IF_ASSERT_FAILED(currentPlayer()) {
        return;
    }

    currentPlayer()->seek(secs, flushSound);
}

bool PlaybackController::isPlaybackInited() const
{
    return m_isPlaybackInited;
}

muse::async::Channel<bool> PlaybackController::playbackInitedChanged() const
{
    return m_playbackInited;
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

const PlaybackController::SoloMuteState& PlaybackController::trackSoloMuteState(const InstrumentTrackId& trackId) const
{
    return m_notation->soloMuteState()->trackSoloMuteState(trackId);
}

void PlaybackController::setTrackSoloMuteState(const InstrumentTrackId& trackId, const SoloMuteState& state)
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

void PlaybackController::playElements(const std::vector<const notation::EngravingItem*>& elements, const PlayParams& params, bool isMidi)
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
    elementsForPlaying.reserve(elements.size());

    bool playChordWhenEditing = configuration()->playChordWhenEditing();
    bool playHarmonyWhenEditing = configuration()->playHarmonyWhenEditing();

    for (const EngravingItem* element : elements) {
        IF_ASSERT_FAILED(element) {
            continue;
        }

        if (!element->isPlayable()) {
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

    const mpe::duration_t duration = params.duration.has_value() ? params.duration.value()
                                     : notationConfiguration()->notePlayDurationMilliseconds() * 1000;

    notationPlayback()->triggerEventsForItems(elementsForPlaying, duration, params.flushSound);
}

void PlaybackController::playNotes(const NoteValList& notes, staff_idx_t staffIdx, const Segment* segment, const PlayParams& params)
{
    Segment* seg = const_cast<Segment*>(segment);
    Chord* chord = engraving::Factory::createChord(seg);
    chord->setParent(seg);

    std::vector<const EngravingItem*> elements;
    elements.reserve(notes.size());

    for (const NoteVal& nval : notes) {
        Note* note = engraving::Factory::createNote(chord);
        note->setParent(chord);
        note->setStaffIdx(staffIdx);
        note->setNval(nval);
        elements.push_back(note);
    }

    playElements(elements, params);

    delete chord;
    DeleteAll(elements);
}

void PlaybackController::playMetronome(int tick)
{
    notationPlayback()->triggerMetronome(tick);
}

void PlaybackController::triggerControllers(const muse::mpe::ControllerChangeEventList& list, staff_idx_t staffIdx, int tick)
{
    notationPlayback()->triggerControllers(list, staffIdx, tick);
}

void PlaybackController::seekElement(const notation::EngravingItem* element, bool flushSound)
{
    IF_ASSERT_FAILED(notationPlayback()) {
        return;
    }

    if (!element) {
        return;
    }

    RetVal<midi::tick_t> tick = notationPlayback()->playPositionTickByElement(element);
    if (!tick.ret) {
        return;
    }

    seek(playedTickToSecs(tick.val), flushSound);
}

void PlaybackController::seekBeat(int measureIndex, int beatIndex, bool flushSound)
{
    secs_t targetSecs = beatToSecs(measureIndex, beatIndex);
    seek(targetSecs, flushSound);
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
        m_onlineSoundsController->addOnlineTrack(trackId, newMeta);
        tours()->onEvent(u"online_sounds_added");
        notationPlayback->setSendEventsOnScoreChange(instrumentTrackId, true);
    } else if (audio::isOnlineAudioResource(oldMeta)) {
        m_onlineSoundsController->removeOnlineTrack(trackId);
        notationPlayback->setSendEventsOnScoreChange(instrumentTrackId, false);
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

    return isResourceType(oldMeta, AudioResourceType::MuseSamplerSoundPack)
           || isResourceType(newMeta, AudioResourceType::MuseSamplerSoundPack);
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
    setNotation(globalContext()->currentNotation());
}

void PlaybackController::onPartChanged(const Part* part)
{
    if (!m_notation->hasVisibleParts()) {
        doPause();
    }
    m_isPlayAllowedChanged.send(isPlayAllowed());

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

        addSoundFlagsIfNeed(selection->elements());
        return;
    }

    m_player->resetLoop();

    seekRangeSelection();
    updateSoloMuteStates();
}

muse::Ret PlaybackController::togglePlay()
{
    if (isPlaying()) {
        return pause();
    }

    if (isPaused()) {
        doResume();
        return make_ok();
    }

    return play();
}

muse::Ret PlaybackController::play(bool showErrors)
{
    if (!isPlayAllowed()) {
        LOGW() << "playback not allowed";
        return make_ret(Ret::Code::NotSupported);
    }

    IF_ASSERT_FAILED(currentPlayer()) {
        return make_ret(Ret::Code::InternalError);
    }

    if (showErrors && m_onlineSoundsController->shouldShowOnlineSoundsProcessingError(isPlaying())) {
        m_onlineSoundsController->showOnlineSoundsProcessingError([this]() { play(false /*showErrors*/); });
        return make_ret(Ret::Code::NotSupported);
    }

    interaction()->endEditElement();
    interaction()->noteInput()->endNoteInput();

    if (isPaused()) {
        notationPlayback()->sendEventsForChangedTracks();

        secs_t pos = currentPlayer()->playbackPosition();
        secs_t endSecs = totalPlayTime();
        if (pos == endSecs) {
            secs_t startSecs = playbackStartSecs();
            seek(startSecs);
        }

        doResume();
    } else {
        notationPlayback()->sendEventsForChangedTracks();

        doPlay();
    }

    return make_ok();
}

muse::Ret PlaybackController::pause(bool select)
{
    if (isPlaying()) {
        doPause(select);
    }

    return make_ok();
}

muse::Ret PlaybackController::stop()
{
    doStop();
    return make_ok();
}

muse::rcommand::Response PlaybackController::rewind(const muse::rcommand::Request& request)
{
    if (!isPlayAllowed()) {
        LOGW() << "playback not allowed";
        return muse::rcommand::make_response(request, make_ret(Ret::Code::NotSupported));
    }

    double secs = request.query.param("position", Val(0.0)).toDouble();
    doRewind(secs_t(secs));
    return muse::rcommand::make_response(request, make_ok());
}

muse::Ret PlaybackController::playFromSelection(bool showErrors)
{
    if (!isPlayAllowed()) {
        LOGW() << "playback not allowed";
        return make_ret(Ret::Code::NotSupported);
    }

    if (showErrors && m_onlineSoundsController->shouldShowOnlineSoundsProcessingError(isPlaying())) {
        m_onlineSoundsController->showOnlineSoundsProcessingError([this]() { playFromSelection(false /*showErrors*/); });
        return make_ret(Ret::Code::NotSupported);
    }

    int startTick = 0;

    if (!selection()->isNone()) {
        startTick = INT_MAX;
        for (const EngravingItem* item : selection()->elements()) {
            startTick = std::min(startTick, item->tick().ticks());
        }
    } else {
        // Selection is none - fall back to last element hit...
        if (const EngravingItem* lastElementHit = selection()->lastElementHit()) {
            startTick = lastElementHit->tick().ticks();
        }
    }

    const LoopBoundaries& loop = notationPlayback()->loopBoundaries();
    if (loop.enabled) {
        if (startTick < loop.loopInTick.ticks() || startTick > loop.loopOutTick.ticks()) {
            startTick = loop.loopInTick.ticks();
        }
    }

    const RetVal<midi::tick_t> retval = notationPlayback()->playPositionTickByRawTick(startTick);
    if (!retval.ret) {
        return retval.ret;
    }

    seek(playedTickToSecs(retval.val));

    if (isPaused()) {
        doResume();
    } else if (!isPlaying()) {
        doPlay();
    }

    return make_ok();
}

void PlaybackController::doPlay()
{
    IF_ASSERT_FAILED(currentPlayer()) {
        return;
    }

    if (isLoopEnabled()) {
        secs_t startSecs = playbackStartSecs();
        seek(startSecs);
    }

    currentPlayer()->prepareToPlay().onResolve(this, [this](const Ret& ret) {
        if (!currentPlayer()) {
            return;
        }

        if (!ret) {
            LOGE() << ret.toString();
        }

        secs_t delay = 0.;
        if (notationConfiguration()->isCountInEnabled()) {
            notationPlayback()->triggerCountIn(m_currentTick, delay);
        }

        currentPlayer()->play(delay);
    });
}

void PlaybackController::doRewind(secs_t newPosition)
{
    secs_t startSecs = playbackStartSecs();
    secs_t endSecs = totalPlayTime();
    newPosition = std::clamp(newPosition, startSecs, endSecs);

    seek(newPosition);
}

void PlaybackController::doPause(bool select)
{
    IF_ASSERT_FAILED(currentPlayer()) {
        return;
    }

    if (isPaused()) {
        return;
    }

    currentPlayer()->pause();

    if (select && m_notation) {
        const Fraction playPositionFrac = Fraction::fromTicks(m_currentTick);
        interaction()->findAndSelectChordRest(playPositionFrac);
    }
}

void PlaybackController::doStop()
{
    IF_ASSERT_FAILED(currentPlayer()) {
        return;
    }

    currentPlayer()->stop();
}

void PlaybackController::doResume()
{
    IF_ASSERT_FAILED(currentPlayer()) {
        return;
    }

    currentPlayer()->prepareToPlay().onResolve(this, [this](const Ret& ret) {
        if (!currentPlayer()) {
            return;
        }

        if (!ret) {
            LOGE() << ret.toString();
        }

        secs_t delay = 0.;
        if (notationConfiguration()->isCountInEnabled()) {
            notationPlayback()->triggerCountIn(m_currentTick, delay);
        }

        currentPlayer()->resume(delay);
    });
}

void PlaybackController::onPlaybackStatusChanged()
{
    if (!notationPlayback()) {
        return;
    }

    bool playing = isPlaying();
    const auto& onlineSounds = m_onlineSoundsController->onlineSounds();

    for (const auto& pair : m_instrumentTrackIdMap) {
        bool shouldSendOnScoreChange = playing || muse::contains(onlineSounds, pair.second);
        notationPlayback()->setSendEventsOnScoreChange(pair.first, shouldSendOnScoreChange);
    }
}

secs_t PlaybackController::playbackStartSecs() const
{
    if (!m_notation) {
        return 0;
    }

    const LoopBoundaries& loop = notationPlayback()->loopBoundaries();
    if (loop.enabled) {
        // Convert from raw ticks (visual tick != playback tick due to repeats etc)
        RetVal<tick_t> startTick = notationPlayback()->playPositionTickByRawTick(loop.loopInTick.ticks());
        if (!startTick.ret) {
            return 0;
        }
        return playedTickToSecs(startTick.val);
    }

    return 0;
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

muse::Ret PlaybackController::togglePlayRepeats()
{
    bool playRepeatsEnabled = notationConfiguration()->isPlayRepeatsEnabled();
    notationConfiguration()->setIsPlayRepeatsEnabled(!playRepeatsEnabled);
    return make_ok();
}

muse::Ret PlaybackController::togglePlayChordSymbols()
{
    bool playChordSymbolsEnabled = notationConfiguration()->isPlayChordSymbolsEnabled();
    notationConfiguration()->setIsPlayChordSymbolsEnabled(!playChordSymbolsEnabled);

    for (auto it = m_instrumentTrackIdMap.cbegin(); it != m_instrumentTrackIdMap.cend(); ++it) {
        if (notationPlayback()->isChordSymbolsTrack(it->first)) {
            setTrackActivity(it->first, !playChordSymbolsEnabled);
        }
    }
    return make_ok();
}

muse::Ret PlaybackController::toggleAutomaticallyPan()
{
    bool panEnabled = notationConfiguration()->isAutomaticallyPanEnabled();
    notationConfiguration()->setIsAutomaticallyPanEnabled(!panEnabled);
    return make_ok();
}

muse::Ret PlaybackController::toggleMetronome()
{
    bool metronomeEnabled = notationConfiguration()->isMetronomeEnabled();
    bool countInEnabled = notationConfiguration()->isCountInEnabled();

    notationConfiguration()->setIsMetronomeEnabled(!metronomeEnabled);

    setTrackActivity(notationPlayback()->metronomeTrackId(), !metronomeEnabled || countInEnabled);

    return make_ok();
}

muse::Ret PlaybackController::toggleCountIn()
{
    bool metronomeEnabled = notationConfiguration()->isMetronomeEnabled();
    bool countInEnabled = notationConfiguration()->isCountInEnabled();

    notationConfiguration()->setIsCountInEnabled(!countInEnabled);

    setTrackActivity(notationPlayback()->metronomeTrackId(), metronomeEnabled || !countInEnabled);

    return make_ok();
}

muse::Ret PlaybackController::toggleMidiInput()
{
    bool wasMidiInputEnabled = notationConfiguration()->isMidiInputEnabled();
    notationConfiguration()->setIsMidiInputEnabled(!wasMidiInputEnabled);
    return make_ok();
}

muse::Ret PlaybackController::setMidiUseWrittenPitch(bool useWrittenPitch)
{
    notationConfiguration()->setMidiUseWrittenPitch(useWrittenPitch);
    return make_ok();
}

muse::Ret PlaybackController::toggleLoopPlayback()
{
    if (isLoopEnabled()) {
        disableLoop();
        return make_ok();
    }

    if (loopBoundariesSet() && !selection()->isRange()) {
        enableLoop();
        return make_ok();
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

    return make_ok();
}

muse::Ret PlaybackController::toggleHearPlaybackWhenEditing()
{
    bool wasPlayNotesWhenEditing = configuration()->playNotesWhenEditing();
    configuration()->setPlayNotesWhenEditing(!wasPlayNotesWhenEditing);
    return make_ok();
}

muse::Ret PlaybackController::reloadPlaybackCache()
{
    INotationPlaybackPtr nPlayback = notationPlayback();
    if (nPlayback) {
        nPlayback->reload();
    }

    return make_ok();
}

muse::Ret PlaybackController::showPlaybackSetup()
{
    interactive()->open("musescore://playback/soundprofiles");
    return make_ok();
}

muse::Ret PlaybackController::addLoopBoundary(LoopBoundaryType type)
{
    if (isPlaying()) {
        addLoopBoundaryToTick(type, m_currentTick);
    } else {
        addLoopBoundaryToTick(type, INotationPlayback::SelectedNoteTick);
    }

    return make_ok();
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
    RetVal<tick_t> playbackTickFrom = notationPlayback()->playPositionTickByRawTick(boundaries.loopInTick.ticks());
    RetVal<tick_t> playbackTickTo = notationPlayback()->playPositionTickByRawTick(boundaries.loopOutTick.ticks());
    if (!playbackTickFrom.ret || !playbackTickTo.ret) {
        return;
    }

    secs_t fromSecs = playedTickToSecs(playbackTickFrom.val);
    secs_t toSecs = playedTickToSecs(playbackTickTo.val);
    currentPlayer()->setLoop(fromSecs, toSecs);

    enableLoop();
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
}

void PlaybackController::notifyActionCheckedChanged(const ActionCode& actionCode)
{
    m_actionCheckedChanged.send(actionCode);
}

mu::project::IProjectAudioSettingsPtr PlaybackController::audioSettings() const
{
    INotationProjectPtr project = globalContext()->currentProject();
    IF_ASSERT_FAILED(project) {
        return nullptr;
    }

    return project->audioSettings();
}

mu::project::IProjectVideoSettingsPtr PlaybackController::videoSettings() const
{
    if (!globalContext()->currentProject()) {
        return nullptr;
    }

    return globalContext()->currentProject()->videoSettings();
}

void PlaybackController::updateMasterControlParams()
{
    if (!globalContext()->currentProject() || !playback()) {
        return;
    }

    IProjectAudioSettingsPtr audioSettingsPtr = audioSettings();
    IF_ASSERT_FAILED(audioSettingsPtr) {
        return;
    }

    AudioOutputParams params = audioSettingsPtr->masterAudioOutputParams();
    IProjectVideoSettingsPtr videoSettingsPtr = videoSettings();
    if (videoSettingsPtr && videoSettingsPtr->attachment().isValid() && videoSettingsPtr->attachment().solo) {
        params.muted = true;
    }

    playback()->setMasterControlParams(params.control());
}

void PlaybackController::resetPlayback()
{
    if (currentPlayer()) {
        currentPlayer()->playbackPositionChanged().disconnect(this);
        currentPlayer()->playbackStatusChanged().disconnect(this);
    }

    playback()->clearSources();
    playback()->sourceParamsChanged().disconnect(this);
    playback()->fxChainParamsChanged().disconnect(this);
    playback()->clearAllFx();
    playback()->masterFxChainParamsChanged().disconnect(this);
    playback()->clearMasterOutputParams();

    m_seqAsyncReceiver.async_disconnectAll();

    m_currentTick = 0;

    playback()->deinit();

    m_instrumentTrackIdMap.clear();
    m_auxTrackIdMap.clear();

    m_isRangeSelection = false;

    m_isPlaybackInited = false;
    m_playbackInited.send(m_isPlaybackInited);

    m_player = nullptr;
    globalContext()->setCurrentPlayer(nullptr);

    m_onlineSoundsController->reset();
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
    AudioOutputParams originParams = trackOutputParams(instrumentTrackId);
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

    if (!isMetronome && originParams.auxSends.empty()) {
        const muse::String& instrumentSoundId = inParams.resourceMeta.attributeVal(PLAYBACK_SETUP_DATA_ATTRIBUTE);
        AudioSourceType sourceType = inParams.isValid() ? inParams.type() : AudioSourceType::Fluid;

        for (aux_channel_idx_t idx = 0; idx < AUX_CHANNEL_NUM; ++idx) {
            gain_t signalAmount = configuration()->defaultAuxSendValue(idx, sourceType, instrumentSoundId);
            originParams.auxSends.emplace_back(AuxSendParams { signalAmount, true });
        }
    }

    uint64_t playbackKey = notationPlaybackKey();

    TrackParams trackParams;
    trackParams.source = inParams;
    trackParams.fxChain = originParams.fxChain;
    trackParams.auxSends = originParams.auxSends;
    trackParams.control = originParams.control();

    playback()->addTrack(title, std::move(playbackData), trackParams)
    .onResolve(this, [this, title, instrumentTrackId, playbackKey, onFinished, originMeta, originParams](const TrackId trackId,
                                                                                                         const TrackParams& appliedParams) {
        //! NOTE It may be that while we were adding a track, the notation was already closed (or opened another)
        //! This situation can be if the notation was opened and immediately closed.
        if (notationPlaybackKey() != playbackKey) {
            return;
        }

        m_instrumentTrackIdMap.insert({ instrumentTrackId, trackId });

        const bool trackNewlyAdded = !audioSettings()->trackHasExistingOutputParams(instrumentTrackId);

        auto appliedOutParams = originParams;
        appliedOutParams.fxChain = appliedParams.fxChain;

        audioSettings()->setTrackInputParams(instrumentTrackId, appliedParams.source);
        audioSettings()->setTrackOutputParams(instrumentTrackId, appliedOutParams);

        updateSoloMuteStates();

        onFinished();

        m_trackAdded.send(trackId);

        if (trackNewlyAdded) {
            onTrackNewlyAdded(instrumentTrackId);
        }

        if (shouldLoadDrumset(instrumentTrackId, originMeta, appliedParams.source.resourceMeta)) {
            m_drumsetLoader.loadDrumset(m_notation, instrumentTrackId, appliedParams.source.resourceMeta);
        }

        if (muse::audio::isOnlineAudioResource(appliedParams.source.resourceMeta)) {
            m_onlineSoundsController->addOnlineTrack(trackId, appliedParams.source.resourceMeta);

            if (notationPlayback()) {
                notationPlayback()->setSendEventsOnScoreChange(instrumentTrackId, true);
            }
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

    AudioOutputParams originParams;

    if (audioSettings()->containsAuxOutputParams(index)) {
        originParams = audioSettings()->auxOutputParams(index);
    } else if (index == REVERB_CHANNEL_IDX) {
        originParams = makeReverbOutputParams();
    }

    TrackParams trackParams;
    trackParams.source = {};
    trackParams.fxChain = originParams.fxChain;
    trackParams.auxSends = originParams.auxSends;
    trackParams.control = originParams.control();

    std::string title = resolveAuxTrackTitle(index, originParams, false);
    uint64_t playbackKey = notationPlaybackKey();

    playback()->addAuxTrack(title, trackParams)
    .onResolve(this, [this, playbackKey, index, onFinished, originParams](const TrackId trackId, const TrackParams& appliedParams) {
        //! NOTE It may be that while we were adding a track, the notation was already closed (or opened another)
        //! This situation can be if the notation was opened and immediately closed.
        if (notationPlaybackKey() != playbackKey) {
            return;
        }

        m_auxTrackIdMap.insert({ index, trackId });

        auto appliedOutParams = originParams;
        appliedOutParams.fxChain = appliedParams.fxChain;

        audioSettings()->setAuxOutputParams(index, appliedOutParams);

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
    ControlParams control = outParams.control();

    control.muted = !isActive;

    audio::TrackId trackId = m_instrumentTrackIdMap[instrumentTrackId];
    playback()->setControlParams(trackId, control);
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

void PlaybackController::removeTrack(const InstrumentTrackId& instrumentTrackId)
{
    IF_ASSERT_FAILED(notationPlayback() && playback()) {
        return;
    }

    auto search = m_instrumentTrackIdMap.find(instrumentTrackId);

    if (search == m_instrumentTrackIdMap.end()) {
        return;
    }

    playback()->removeTrack(search->second);
    audioSettings()->removeTrackParams(instrumentTrackId);

    m_masterNotation->notation()->soloMuteState()->removeTrackSoloMuteState(instrumentTrackId);
    for (const IExcerptNotationPtr& excerpt : m_masterNotation->excerpts()) {
        if (const INotationPtr& notation = excerpt->notation()) {
            notation->soloMuteState()->removeTrackSoloMuteState(instrumentTrackId);
        }
    }

    m_onlineSoundsController->removeOnlineTrack(search->second);

    m_trackRemoved.send(search->second);
    m_instrumentTrackIdMap.erase(instrumentTrackId);
}

void PlaybackController::onTrackNewlyAdded(const InstrumentTrackId& instrumentTrackId)
{
    for (const IExcerptNotationPtr& excerpt : m_masterNotation->excerpts()) {
        if (const INotationPtr& notation = excerpt->notation()) {
            if (notation == m_notation || notation->soloMuteState()->trackSoloMuteStateExists(instrumentTrackId)) {
                continue;
            }

            const Part* part = notation->parts()->part(instrumentTrackId.partId);
            const bool shouldMute = !part || !part->show();

            const INotationSoloMuteState::SoloMuteState soloMuteState = { shouldMute, /*solo*/ false };
            notation->soloMuteState()->setTrackSoloMuteState(instrumentTrackId, soloMuteState);
        }
    }
}

void PlaybackController::setupPlayback()
{
    playback()->removeAllTracks();

    m_player = playback()->player();
    globalContext()->setCurrentPlayer(m_player);

    if (!notationPlayback()) {
        return;
    }

    const AudioOutputParams& masterOutputParams = audioSettings()->masterAudioOutputParams();
    playback()->setMasterFxChainParams(masterOutputParams.fxChain);
    playback()->setMasterAuxSendsParams(masterOutputParams.auxSends);
    updateMasterControlParams();

    subscribeOnAudioParamsChanges();
    setupTracks();
    setupPlayer();

    m_isPlaybackInited = true;
    m_playbackInited.send(m_isPlaybackInited);
}

void PlaybackController::subscribeOnAudioParamsChanges()
{
    playback()->masterFxChainParamsChanged().onReceive(this, [this](const AudioFxChain& params) {
        AudioOutputParams outParams = audioSettings()->masterAudioOutputParams();
        outParams.fxChain = params;
        audioSettings()->setMasterAudioOutputParams(outParams);
    });

    playback()->sourceParamsChanged().onReceive(this, [this](const TrackId trackId, const AudioInputParams& params) {
        auto search = std::find_if(m_instrumentTrackIdMap.begin(), m_instrumentTrackIdMap.end(), [trackId](const auto& pair) {
            return pair.second == trackId;
        });

        if (search != m_instrumentTrackIdMap.end()) {
            const AudioResourceMeta& oldMeta = audioSettings()->trackInputParams(search->first).resourceMeta;
            onAudioResourceChanged(trackId, search->first, oldMeta, params.resourceMeta);

            audioSettings()->setTrackInputParams(search->first, params);
        }
    });

    playback()->fxChainParamsChanged().onReceive(this, [this](const TrackId trackId, const AudioFxChain& params) {
        auto instrumentIt = std::find_if(m_instrumentTrackIdMap.begin(), m_instrumentTrackIdMap.end(), [trackId](const auto& pair) {
            return pair.second == trackId;
        });

        if (instrumentIt != m_instrumentTrackIdMap.end()) {
            AudioOutputParams outParams = audioSettings()->trackOutputParams(instrumentIt->first);
            outParams.fxChain = params;
            audioSettings()->setTrackOutputParams(instrumentIt->first, outParams);
            return;
        }

        auto auxIt = std::find_if(m_auxTrackIdMap.begin(), m_auxTrackIdMap.end(), [trackId](const auto& pair) {
            return pair.second == trackId;
        });

        if (auxIt != m_auxTrackIdMap.end()) {
            aux_channel_idx_t auxIdx = auxIt->first;
            std::string oldName = resolveAuxTrackTitle(auxIdx, audioSettings()->auxOutputParams(auxIdx));
            AudioOutputParams outParams = audioSettings()->auxOutputParams(auxIdx);
            outParams.fxChain = params;
            std::string newName = resolveAuxTrackTitle(auxIdx, outParams);

            audioSettings()->setAuxOutputParams(auxIdx, outParams);

            if (oldName != newName) {
                m_auxChannelNameChanged.send(auxIdx, newName);
            }
        }
    });
}

void PlaybackController::setupTracks()
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
            m_isPlayAllowedChanged.send(isPlayAllowed());
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

    //! HACK - ideally we would use "this" (PlaybackController) instead of m_seqAsyncReceiver for the following
    //! subscription, but we've already subscribed to onItemChanged for a different reason in setNotation...
    partList.onItemChanged(&m_seqAsyncReceiver, [this](const Part*) {
        updateSoloMuteStates();
    });

    audioSettings()->auxSoloMuteStateChanged().onReceive(
        this, [this](aux_channel_idx_t, const notation::INotationSoloMuteState::SoloMuteState&) {
        updateSoloMuteStates();
    });

    if (videoSettings()) {
        videoSettings()->settingsChanged().onNotify(this, [this]() {
            updateMasterControlParams();
        }, Asyncable::Mode::SetReplace);
    }

    m_isPlayAllowedChanged.send(isPlayAllowed());
}

void PlaybackController::setupPlayer()
{
    currentPlayer()->playbackPositionChanged().onReceive(this, [this](const audio::secs_t pos) {
        m_currentTick = notationPlayback()->secToTick(pos);

        updateCurrentTempo();

        secs_t endSecs = totalPlayTime();
        if (pos + muse::msecs_to_secs(1) >= endSecs) {
            doStop();
        }
    });

    currentPlayer()->playbackStatusChanged().onReceive(this, [this](PlaybackStatus) {
        onPlaybackStatusChanged();
    });

    currentPlayer()->setDuration(notationPlayback()->totalPlayTime());

    notationPlayback()->totalPlayTimeChanged().onReceive(this, [this](const audio::secs_t totalPlaybackTime) {
        currentPlayer()->setDuration(totalPlaybackTime);
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
        const auto& soloMuteState = m_notation->soloMuteState()->trackSoloMuteState(instrumentTrackId);

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
        playback()->setControlParams(trackId, params.control());
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
        playback()->setControlParams(pair.second, params.control());
    }
}

bool PlaybackController::actionChecked(const ActionCode&) const
{
    return false;
}

Channel<ActionCode> PlaybackController::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}

secs_t PlaybackController::totalPlayTime() const
{
    auto np = notationPlayback();
    return np ? np->totalPlayTime() : secs_t { 0.0 };
}

Notification PlaybackController::totalPlayTimeChanged() const
{
    return m_totalPlayTimeChanged;
}

const Tempo& PlaybackController::currentTempo() const
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
        doPause();
    }

    notationPlayback()->setTempoMultiplier(multiplier);
    seekRawTick(tick);
    updateLoop();

    if (playing) {
        doResume();
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
        AudioSourceParams newSourceParams { profile.findResource(playbackData.setupData), {} };

        playback()->setSourceParams(pair.second, newSourceParams);
    }

    audioSettingsPtr->setActiveSoundProfile(profileName);
}

void PlaybackController::setNotation(notation::INotationPtr notation)
{
    if (m_notation == notation) {
        return;
    }

    if (m_notation) {
        INotationPartsPtr notationParts = m_notation->parts();
        NotifyList<const Part*> partList = notationParts->partList();
        partList.disconnect(this);

        m_notation->interaction()->selectionChanged().disconnect(this);
        m_notation->interaction()->textEditingEnded().disconnect(this);
        m_notation->soloMuteState()->trackSoloMuteStateChanged().disconnect(this);
    }

    m_notation = notation;

    m_isPlayAllowedChanged.send(isPlayAllowed());

    if (!m_notation) {
        setMasterNotation(nullptr);
        return;
    }

    setMasterNotation(m_notation->masterNotation());

    if (!m_notation->hasVisibleParts()) {
        doPause();
    }

    updateSoloMuteStates();

    NotifyList<const Part*> partList = m_notation->parts()->partList();

    partList.onItemAdded(this, [this](const Part* part) {
        onPartChanged(part);
    });

    partList.onItemChanged(this, [this](const Part* part) {
        onPartChanged(part);
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

void PlaybackController::setMasterNotation(notation::IMasterNotationPtr masterNotation)
{
    if (m_masterNotation == masterNotation) {
        return;
    }

    if (m_masterNotation) {
        m_masterNotation->hasPartsChanged().disconnect(this);
        m_masterNotation->playback()->loopBoundariesChanged().disconnect(this);
        m_masterNotation->playback()->loopEnabledChanged().disconnect(this);
    }

    m_masterNotation = masterNotation;

    m_totalPlayTimeChanged.notify();

    if (!m_masterNotation) {
        return;
    }

    m_masterNotation->hasPartsChanged().onNotify(this, [this]() {
        m_isPlayAllowedChanged.send(isPlayAllowed());
    });

    m_masterNotation->playback()->loopBoundariesChanged().onNotify(this, [this]() {
        updateLoop();
    });

    m_masterNotation->playback()->loopEnabledChanged().onReceive(this, [this](bool value) {
        m_loopEnabledChanged.send(value);
    });
}

void PlaybackController::setIsExportingAudio(bool exporting)
{
    if (m_isExportingAudio == exporting) {
        return;
    }

    m_isExportingAudio = exporting;
    updateSoloMuteStates();

    if (exporting && notationPlayback()) {
        notationPlayback()->sendEventsForChangedTracks();
    }
}

bool PlaybackController::canReceiveAction(const ActionCode&) const
{
    if (!m_masterNotation || !m_masterNotation->hasParts()) {
        return false;
    }

    return true;
}

const std::map<TrackId, AudioResourceMeta>& PlaybackController::onlineSounds() const
{
    return m_onlineSoundsController->onlineSounds();
}

muse::async::Notification PlaybackController::onlineSoundsChanged() const
{
    return m_onlineSoundsController->onlineSoundsChanged();
}

muse::Progress PlaybackController::onlineSoundsProcessingProgress() const
{
    return m_onlineSoundsController->onlineSoundsProcessingProgress();
}

muse::audio::secs_t PlaybackController::playedTickToSecs(int tick) const
{
    return secs_t(notationPlayback()->playedTickToSec(tick));
}
