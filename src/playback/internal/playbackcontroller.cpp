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
#include "types/ret.h"

#include "audio/common/audioutils.h"
#include "audio/devtools/inputlag.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/utils.h"
#include "engraving/editing/noteinput.h"

#include "notation/iexcerptnotation.h" // IWYU pragma: keep
#include "notation/imasternotation.h"
#include "notation/inotationautomation.h"
#include "notation/inotationelements.h"
#include "notation/inotationinteraction.h"
#include "notation/inotationnoteinput.h" // IWYU pragma: keep
#include "notation/inotationparts.h"
#include "notation/inotationselection.h"

#include "project/inotationproject.h"

#include "../playbacktypes.h"
#include "onlinesoundscontroller.h"
#include "midirecordingcontroller.h"

using namespace muse;
using namespace muse::async;
using namespace muse::audio;
using namespace muse::midi;
using namespace mu::engraving;
using namespace mu::notation;
using namespace mu::playback;
using namespace mu::project;

static const muse::Uri NOTATION_REVIEW_PAGE_URI("musescore://notation/review");

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
    : muse::Contextable(iocCtx), m_onlineSoundsController(std::make_unique<OnlineSoundsController>(iocCtx)),
    m_midiRecordingController(std::make_unique<MidiRecordingController>(iocCtx))
{
}

PlaybackController::~PlaybackController() = default;

void PlaybackController::init()
{
    m_onlineSoundsController->regActions();

    m_midiRecordingController->init();
    m_midiRecordingController->setPlayNoteCallback([this](int pitch, int velocity) {
        playMidiRecordingNote(pitch, velocity);
    });
    m_midiRecordingController->lastRecordingChanged().onNotify(this, [this]() {
        m_lastMidiRecordingChanged.notify();
    });

    projectSession()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();

        setupPlaybackIfNeed();
    });

    projectSession()->projectChanged().onNotify(this, [this]() {
        m_midiRecordingController->clearLastRecording();

        if (m_isPlaybackInited) {
            resetPlayback();
        }

        m_needSetupPlayback = projectSession()->project() != nullptr;

        if (m_needSetupPlayback) {
            m_loadingProgress.start();
        }
    });

    playback()->init().onResolve(this, [this](const Ret& ret) {
        if (!ret) {
            LOGE() << "failed to init the audio context: " << ret.toString();
            return;
        }

        setupPlaybackIfNeed();
    });

    m_totalPlayTimeChanged.onNotify(this, [this]() {
        updateCurrentTempo();

        updateLoop();

        // Tempo/repeat/duration changes shift what secs_t a given tick maps to,
        // which invalidates any Volume/Pan automation envelope already sent to the engine
        resendAutomationEnvelopes();
    });

    m_measureInputLag = configuration()->shouldMeasureInputLag();
}

void PlaybackController::setupPlaybackIfNeed()
{
    if (!m_needSetupPlayback || !playback()->isInited()) {
        return;
    }

    m_needSetupPlayback = false;

    setupPlayback();
}

void PlaybackController::deinit()
{
    m_needSetupPlayback = false;

    if (m_isPlaybackInited) {
        resetPlayback();
    }

    if (playback()->isInited()) {
        playback()->deinit();
    }
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

muse::audio::secs_t PlaybackController::playbackPosition() const
{
    if (!currentPlayer()) {
        return 0;
    }
    return currentPlayer()->playbackPosition();
}

muse::async::Channel<muse::audio::secs_t> PlaybackController::playbackPositionChanged() const
{
    return m_playbackPositionChanged;
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

ControlParams PlaybackController::currentTrackControlParams(const InstrumentTrackId& trackId) const
{
    IF_ASSERT_FAILED(audioSettings()) {
        return {};
    }

    const AudioOutputParams& outParams = audioSettings()->trackOutputParams(trackId);
    ControlParams control = outParams.control();
    control.muted = resolveTrackMuted(trackId, outParams.muted);

    const auto it = m_automationEnvelopesCache.find(trackId);
    if (it != m_automationEnvelopesCache.end()) {
        applyEnvelopes(it->second, control);
    }

    return control;
}

ControlParams PlaybackController::currentMasterControlParams() const
{
    IF_ASSERT_FAILED(audioSettings()) {
        return {};
    }

    ControlParams control = audioSettings()->masterAudioOutputParams().control();
    applyEnvelopes(m_masterAutomationEnvelopes, control);
    return control;
}

void PlaybackController::playElements(const std::vector<const notation::EngravingItem*>& elements, const PlayParams& params, bool isMidi)
{
    IF_ASSERT_FAILED(notationPlayback()) {
        return;
    }

    if ((!configuration()->playNotesWhenEditing()) || (isMidi && !configuration()->playNotesOnMidiInput())) {
        return;
    }

    if (isMidiRecording()) {
        //! NOTE During a MIDI recording the keys are played by playMidiRecordingNote, with the instrument of the recorded staff
        if (isMidi) {
            return;
        }
    } else if (isPlaying()) {
        //! NOTE The audio engine plays these events also during playback (see AbstractEventSequencer::movePlaybackForward),
        //! but this was not the case before: keep the previous behaviour outside of MIDI recording
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

    std::vector<const EngravingItem*> elements;
    elements.reserve(notes.size());

    for (const NoteVal& nval : notes) {
        Note* note = engraving::Factory::createNote(chord);
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
    if (isPlaying() && !isMidiRecording()) {
        return; // see playElements
    }

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
    cancelMidiRecording();
    setNotation(projectSession()->currentNotation());
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

    //! NOTE A MIDI recording always starts a new playback, with its own count-in: the playback was stopped
    //! by startMidiRecording, even if the stopped status may not be reported yet
    if (isPaused() && !isMidiRecording()) {
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

muse::Ret PlaybackController::rewind(muse::secs_t secs)
{
    if (!isPlayAllowed()) {
        LOGW() << "playback not allowed";
        return make_ret(Ret::Code::NotSupported);
    }

    doRewind(secs);
    return make_ok();
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

    if (isLoopEnabled() && !isMidiRecording()) {
        secs_t startSecs = playbackStartSecs();
        seek(startSecs);
    }

    const uint64_t requestId = ++m_playRequestId;

    currentPlayer()->prepareToPlay().onResolve(this, [this, requestId](const Ret& ret) {
        if (!currentPlayer()) {
            return;
        }

        if (requestId != m_playRequestId) {
            LOGI() << "the playback was stopped, or requested again, during its preparation";
            return;
        }

        if (!ret) {
            LOGE() << ret.toString();
        }

        secs_t delay = 0.;
        if (isMidiRecording()) {
            //! NOTE The count-in of the MIDI recording has its own setting (0, 1 or 2 measures)
            const int countInMeasures = notationConfiguration()->midiRecordingCountInMeasures();
            if (countInMeasures > 0) {
                notationPlayback()->triggerCountIn(m_currentTick, delay, countInMeasures);
            }

            const auto countInDuration = std::chrono::duration_cast<MidiRecordingController::Clock::duration>(
                std::chrono::duration<double>(static_cast<double>(delay)));
            m_midiRecordingController->setMusicStartTime(MidiRecordingController::Clock::now() + countInDuration);
        } else if (notationConfiguration()->isCountInEnabled()) {
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

    ++m_playRequestId; // a pending play request must not start the playback after the stop
    currentPlayer()->stop();
}

void PlaybackController::doResume()
{
    IF_ASSERT_FAILED(currentPlayer()) {
        return;
    }

    const uint64_t requestId = ++m_playRequestId;

    currentPlayer()->prepareToPlay().onResolve(this, [this, requestId](const Ret& ret) {
        if (!currentPlayer()) {
            return;
        }

        if (requestId != m_playRequestId) {
            LOGI() << "the playback was stopped, or requested again, during its preparation";
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
    const bool playing = isPlaying();
    m_isPlayingChanged.send(playing);

    if (!notationPlayback()) {
        return;
    }

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

    //! NOTE During a MIDI recording the click is always on: the setting is applied when the recording ends
    if (!isMidiRecording()) {
        setTrackActivity(notationPlayback()->metronomeTrackId(), !metronomeEnabled || countInEnabled);
    }

    return make_ok();
}

muse::Ret PlaybackController::toggleCountIn()
{
    bool metronomeEnabled = notationConfiguration()->isMetronomeEnabled();
    bool countInEnabled = notationConfiguration()->isCountInEnabled();

    notationConfiguration()->setIsCountInEnabled(!countInEnabled);

    if (!isMidiRecording()) {
        setTrackActivity(notationPlayback()->metronomeTrackId(), metronomeEnabled || !countInEnabled);
    }

    return make_ok();
}

bool PlaybackController::isMidiRecording() const
{
    return m_midiRecordingController && m_midiRecordingController->isRecording();
}

muse::async::Notification PlaybackController::isMidiRecordingChanged() const
{
    return m_isMidiRecordingChanged;
}

muse::Ret PlaybackController::toggleMidiRecording()
{
    if (isMidiRecording()) {
        if (!m_midiRecordingPlaybackStarted) {
            //! NOTE The playback has not started yet (e.g. still preparing, or it could not start):
            //! nothing was recorded, so the recording is cancelled instead of waiting for the end of a playback
            //! that may never come
            cancelMidiRecording();
            doStop();
            return make_ok();
        }

        // The recorded notes are written when the playback stops (see onMidiRecordingPlaybackStatusChanged)
        doStop();
        return make_ok();
    }

    startMidiRecording();
    return make_ok();
}

void PlaybackController::startMidiRecording(bool showErrors)
{
    if (!isPlayAllowed() || !currentPlayer() || !m_notation) {
        LOGW() << "playback not allowed";
        return;
    }

    if (!notationConfiguration()->isMidiInputEnabled()) {
        interactive()->info(muse::trc("playback", "MIDI input is disabled"),
                            muse::trc("playback", "Enable MIDI input in the playback settings to record from a MIDI keyboard."));
        return;
    }

    if (!m_midiRecordingController->canWriteRecording()) {
        interactive()->error(muse::trc("playback", "MIDI recording is not available"),
                             muse::trc("playback", "This build does not include the MIDI import module, "
                                                   "which is needed to write the recorded notes."));
        return;
    }

    if (isLoopEnabled()) {
        interactive()->info(muse::trc("playback", "Loop playback is enabled"),
                            muse::trc("playback", "Disable loop playback to record from a MIDI keyboard."));
        return;
    }

    // The recording starts at the beginning of the selected measure, on the selected staff
    staff_idx_t staffIdx = muse::nidx;
    Fraction selectionTick;

    if (selection()->isRange()) {
        staffIdx = selectionRange()->startStaffIndex();
        selectionTick = selectionRange()->startTick();
    } else if (const EngravingItem* element = selection()->element()) {
        staffIdx = element->staffIdx();
        selectionTick = element->tick();
    }

    const Score* score = m_notation->elements()->msScore();
    const Measure* measure = (score && staffIdx != muse::nidx) ? score->tick2measure(selectionTick) : nullptr;
    const Staff* staff = measure ? score->staff(staffIdx) : nullptr;

    if (!measure || !staff) {
        interactive()->info(muse::trc("playback", "No measure selected"),
                            muse::trc("playback", "Select the measure where the recording should start, then press Record."));
        return;
    }

    if (staff->isDrumStaff(measure->tick())) {
        interactive()->info(muse::trc("playback", "Percussion staff selected"),
                            muse::trc("playback", "MIDI recording on percussion staves is not supported yet."));
        return;
    }

    // Two staves: the lower notes go to the staff below, which must belong to the same instrument
    staff_idx_t lowerStaffIdx = muse::nidx;
    if (notationConfiguration()->midiRecordingTwoStaves()) {
        const Staff* lowerStaff = staffIdx + 1 < score->nstaves() ? score->staff(staffIdx + 1) : nullptr;
        if (!lowerStaff || lowerStaff->part() != staff->part() || !lowerStaff->isPitchedStaff(measure->tick())) {
            interactive()->info(muse::trc("playback", "No staff below for the split"),
                                muse::trc("playback", "Recording on two staves needs a staff of the same instrument below the selected one "
                                                      "(e.g. the left hand of a piano). Select the upper staff, "
                                                      "or choose “One staff” in the Recording menu."));
            return;
        }

        lowerStaffIdx = staffIdx + 1;
    }

    const RetVal<tick_t> startPlayedTick = notationPlayback()->playPositionTickByRawTick(measure->tick().ticks());
    if (!startPlayedTick.ret) {
        return;
    }

    //! NOTE If the user accepts to continue while the online sounds are processing, the recording is started
    //! (not an ordinary playback): the selection is read again at that moment
    if (showErrors && m_onlineSoundsController->shouldShowOnlineSoundsProcessingError(isPlaying())) {
        m_onlineSoundsController->showOnlineSoundsProcessingError([this]() { startMidiRecording(false /*showErrors*/); });
        return;
    }

    if (isPlaying() || isPaused()) {
        doStop();
    }

    // Avoid that the notes played on the keyboard start note input on the selection:
    // during the recording, the keyboard only plays the sounds
    INotationInteractionPtr notationInteraction = interaction();
    if (notationInteraction->noteInput()->isNoteInputMode()) {
        notationInteraction->noteInput()->endNoteInput();
    }
    notationInteraction->clearSelection();

    MidiRecordingController::Session session;
    session.notation = m_notation;
    session.notationPlayback = notationPlayback();
    session.staffIdx = staffIdx;
    session.startTick = measure->tick();
    session.startPlayedTick = startPlayedTick.val;
    session.startSecs = playedTickToSecs(startPlayedTick.val);
    session.lowerStaffIdx = lowerStaffIdx;
    session.splitPitch = notationConfiguration()->midiRecordingSplitPitch();

    const secs_t estimatedLatency = MidiRecordingController::estimateOutputLatency(audioConfiguration()->driverBufferSize(),
                                                                                    audioConfiguration()->sampleRate());
    const secs_t userCompensation = secs_t(notationConfiguration()->midiRecordingLatencyCompensationMilliseconds() / 1000.0);
    session.outputLatency = estimatedLatency + userCompensation;

    LOGI() << "MIDI recording: estimated output latency " << static_cast<double>(estimatedLatency) * 1000.0
           << " ms, user compensation " << static_cast<double>(userCompensation) * 1000.0 << " ms";

    seek(session.startSecs);
    m_currentTick = measure->tick().ticks(); // used by the count-in

    m_midiRecordingPlaybackStarted = false;
    m_midiRecordingController->start(session);
    forceMetronomeForMidiRecording();
    m_isMidiRecordingChanged.notify();

    const Ret ret = play(false /*showErrors: already checked above*/);
    if (!ret) {
        LOGE() << "MIDI recording: unable to start the playback: " << ret.toString();
        cancelMidiRecording();
    }
}

void PlaybackController::onMidiRecordingPlaybackStatusChanged()
{
    if (!isMidiRecording()) {
        return;
    }

    if (isPlaying()) {
        m_midiRecordingPlaybackStarted = true;
        return;
    }

    if (!m_midiRecordingPlaybackStarted) {
        return; // e.g. the playback that was stopped before starting the recording
    }

    // The playback was stopped or paused (Stop, Space, Record, end of the score): end of the recording
    m_midiRecordingPlaybackStarted = false;
    restoreMetronomeAfterMidiRecording();

    if (isPaused()) {
        doStop();
    }

    m_midiRecordingController->finishAndWrite();
    m_isMidiRecordingChanged.notify();
}

void PlaybackController::cancelMidiRecording()
{
    if (!isMidiRecording()) {
        return;
    }

    m_midiRecordingController->cancel();
    m_midiRecordingPlaybackStarted = false;
    restoreMetronomeAfterMidiRecording();
    m_isMidiRecordingChanged.notify();
}

void PlaybackController::forceMetronomeForMidiRecording()
{
    //! NOTE Only the audio track is unmuted: the metronome setting of the user is not changed
    setTrackActivity(notationPlayback()->metronomeTrackId(), true);
}

void PlaybackController::restoreMetronomeAfterMidiRecording()
{
    if (!notationPlayback()) {
        return;
    }

    const bool active = notationConfiguration()->isMetronomeEnabled() || notationConfiguration()->isCountInEnabled();
    setTrackActivity(notationPlayback()->metronomeTrackId(), active);
}

void PlaybackController::playMidiRecordingNote(int pitch, int velocity)
{
    const bool noteOn = velocity > 0;

    auto it = m_midiRecordingSoundingNotes.find(pitch);
    if (it != m_midiRecordingSoundingNotes.end()) {
        Note* note = it->second;
        Chord* chord = note->chord();
        m_midiRecordingSoundingNotes.erase(it);

        if (notationPlayback()) {
            note->setUserVelocity(0);
            notationPlayback()->triggerEventsForItems({ note }, 0 /*note off*/, false /*flushSound*/);
        }

        delete note;
        delete chord;
    }

    if (!noteOn || !notationPlayback()) {
        return;
    }

    const MidiRecordingController::Session& session = m_midiRecordingController->session();
    Score* score = session.notation ? session.notation->elements()->msScore() : nullptr;
    const Measure* measure = score ? score->tick2measure(session.startTick) : nullptr;
    Segment* segment = measure ? measure->first(SegmentType::ChordRest) : nullptr;
    if (!segment) {
        return;
    }

    // A temporary note on the recorded staff, so that it is played with the instrument of that staff
    const staff_idx_t staffIdx = session.staffForPitch(pitch);
    const track_idx_t track = staffIdx * VOICES;

    Chord* chord = Factory::createChord(segment);
    chord->setTrack(track);

    Note* note = Factory::createNote(chord);
    note->setTrack(track);
    note->setNval(NoteInput::noteVal(score, pitch, staffIdx, false /*MIDI pitches are sounding pitches*/));
    note->setUserVelocity(velocity);

    notationPlayback()->triggerEventsForItems({ note }, muse::mpe::INFINITE_DURATION, false /*flushSound*/);

    m_midiRecordingSoundingNotes[pitch] = note;
}

const MidiRecordingPreviewNoteList& PlaybackController::midiRecordingPreviewNotes() const
{
    return m_midiRecordingController->previewNotes();
}

muse::async::Notification PlaybackController::midiRecordingPreviewNotesChanged() const
{
    return m_midiRecordingController->previewNotesChanged();
}

bool PlaybackController::canRequantizeLastMidiRecording() const
{
    return !isMidiRecording() && m_midiRecordingController->hasLastRecording(m_notation);
}

muse::async::Notification PlaybackController::lastMidiRecordingChanged() const
{
    return m_lastMidiRecordingChanged;
}

muse::Ret PlaybackController::requantizeLastMidiRecording()
{
    if (!canRequantizeLastMidiRecording()) {
        interactive()->info(muse::trc("playback", "No MIDI recording to re-quantize"),
                            muse::trc("playback", "Record from a MIDI keyboard first: the last recording of this score can then be "
                                                  "written again with the current recording settings."));
        return make_ret(Ret::Code::NotSupported);
    }

    if (isPlaying() || isPaused()) {
        doStop();
    }

    m_midiRecordingController->requantizeLastRecording();
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

mu::project::IProjectAudioSettingsPtr PlaybackController::audioSettings() const
{
    INotationProjectPtr project = projectSession()->project();
    IF_ASSERT_FAILED(project) {
        return nullptr;
    }

    return project->audioSettings();
}

void PlaybackController::resetPlayback()
{
    auto player = m_player;

    m_player = nullptr;
    m_currentTick = 0;

    if (player) {
        player->playbackPositionChanged().disconnect(this);
        player->playbackStatusChanged().disconnect(this);

        player->stop();
        player->seek(0);
        m_playbackPositionChanged.send(0);
        m_isPlayingChanged.send(false);
    }

    playback()->clearSources();
    playback()->sourceParamsChanged().disconnect(this);
    playback()->controlParamsChanged().disconnect(this);
    playback()->fxChainParamsChanged().disconnect(this);
    playback()->auxSendsParamsChanged().disconnect(this);
    playback()->clearAllFx();
    playback()->masterControlParamsChanged().disconnect(this);
    playback()->masterFxChainParamsChanged().disconnect(this);
    playback()->masterAuxSendsParamsChanged().disconnect(this);
    playback()->clearMasterOutputParams();

    m_seqAsyncReceiver.async_disconnectAll();

    m_instrumentTrackIdMap.clear();
    m_auxTrackIdMap.clear();
    m_automationEnvelopesCache.clear();
    m_masterAutomationEnvelopes = {};

    m_isRangeSelection = false;

    m_isPlaybackInited = false;
    m_playbackInited.send(m_isPlaybackInited);

    m_onlineSoundsController->reset();
}

void PlaybackController::addTrack(const InstrumentTrackId& instrumentTrackId, bool projectHadNoAudioSettings,
                                  const TrackAddFinished& onFinished)
{
    const std::optional<std::string> trackName = notationPlayback()->trackName(instrumentTrackId);
    if (trackName) {
        doAddTrack(instrumentTrackId, *trackName, projectHadNoAudioSettings, onFinished);
    }
}

void PlaybackController::doAddTrack(const InstrumentTrackId& instrumentTrackId, const std::string& title,
                                    bool projectHadNoAudioSettings, const TrackAddFinished& onFinished)
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
    trackParams.control = trackControlParams(instrumentTrackId, originParams);

    struct {
        uint64_t playbackKey;
        InstrumentTrackId instrumentTrackId;
        bool projectHadNoAudioSettings;
        AudioOutputParams originParams;
        TrackAddFinished onFinished;
        AudioResourceMeta originMeta;
    } tctx = { playbackKey, instrumentTrackId, projectHadNoAudioSettings, originParams, onFinished,  originMeta };

    playback()->addTrack(title, std::move(playbackData), trackParams)
    .onResolve(this, [this, tctx](const TrackId trackId, const TrackParams& appliedParams) {
        //! NOTE It may be that while we were adding a track, the notation was already closed (or opened another)
        //! This situation can be if the notation was opened and immediately closed.
        if (notationPlaybackKey() != tctx.playbackKey) {
            return;
        }

        // context variables
        const InstrumentTrackId instrumentTrackId = tctx.instrumentTrackId;
        const bool projectHadNoAudioSettings = tctx.projectHadNoAudioSettings;
        const AudioOutputParams& originParams = tctx.originParams;
        const TrackAddFinished onFinished = tctx.onFinished;
        const AudioResourceMeta& originMeta = tctx.originMeta;

        m_instrumentTrackIdMap.insert({ instrumentTrackId, trackId });

        const bool trackNewlyAdded = projectHadNoAudioSettings
                                     || !audioSettings()->trackHasExistingOutputParams(instrumentTrackId);

        auto appliedOutParams = originParams;
        appliedOutParams.fxChain = appliedParams.fxChain;

        audioSettings()->setTrackInputParams(instrumentTrackId, appliedParams.source, !projectHadNoAudioSettings);
        audioSettings()->setTrackOutputParams(instrumentTrackId, appliedOutParams, !projectHadNoAudioSettings);

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

void PlaybackController::addAuxTrack(aux_channel_idx_t index, bool projectHadNoAudioSettings, const TrackAddFinished& onFinished)
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

    struct {
        uint64_t playbackKey;
        aux_channel_idx_t index;
        bool projectHadNoAudioSettings;
        AudioOutputParams originParams;
        TrackAddFinished onFinished;
    } tctx = { playbackKey, index, projectHadNoAudioSettings, originParams, onFinished };

    playback()->addAuxTrack(title, trackParams)
    .onResolve(this, [this, tctx](const TrackId trackId, const TrackParams& appliedParams) {
        //! NOTE It may be that while we were adding a track, the notation was already closed (or opened another)
        //! This situation can be if the notation was opened and immediately closed.
        if (notationPlaybackKey() != tctx.playbackKey) {
            return;
        }

        // context variables
        const aux_channel_idx_t index = tctx.index;
        const bool projectHadNoAudioSettings = tctx.projectHadNoAudioSettings;
        const AudioOutputParams& originParams = tctx.originParams;
        const TrackAddFinished onFinished = tctx.onFinished;

        m_auxTrackIdMap.insert({ index, trackId });

        auto appliedOutParams = originParams;
        appliedOutParams.fxChain = appliedParams.fxChain;

        audioSettings()->setAuxOutputParams(index, appliedOutParams, !projectHadNoAudioSettings);

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
    ControlParams control = trackControlParams(instrumentTrackId, outParams, /*rebuildVolume*/ false, /*rebuildPan*/ false);

    control.muted = !isActive;

    audio::TrackId trackId = m_instrumentTrackIdMap[instrumentTrackId];
    playback()->setControlParams(trackId, control);
}

AudioOutputParams PlaybackController::trackOutputParams(const InstrumentTrackId& instrumentTrackId) const
{
    IF_ASSERT_FAILED(audioSettings()) {
        return {};
    }

    AudioOutputParams result = audioSettings()->trackOutputParams(instrumentTrackId);
    result.muted = resolveTrackMuted(instrumentTrackId, result.muted);
    return result;
}

bool PlaybackController::resolveTrackMuted(const InstrumentTrackId& instrumentTrackId, bool savedMuted) const
{
    IF_ASSERT_FAILED(notationPlayback() && notationConfiguration()) {
        return savedMuted;
    }

    if (instrumentTrackId == notationPlayback()->metronomeTrackId()) {
        return !notationConfiguration()->isMetronomeEnabled() && !notationConfiguration()->isCountInEnabled();
    }

    if (notationPlayback()->isChordSymbolsTrack(instrumentTrackId)) {
        return !notationConfiguration()->isPlayChordSymbolsEnabled();
    }

    return savedMuted;
}

ControlParams PlaybackController::trackControlParams(const InstrumentTrackId& instrumentTrackId,
                                                     const AudioOutputParams& outParams,
                                                     bool rebuildVolume, bool rebuildPan)
{
    // Fixed values always come from the mixer settings; only envelopes are cached
    ControlParams control = outParams.control();

    const bool rebuild = rebuildVolume || rebuildPan;
    const INotationAutomationPtr automation = m_masterNotation ? m_masterNotation->automation() : nullptr;
    const AutomationDataConstPtr automationData = automation ? automation->automationData() : nullptr;

    if (rebuild && (!automationData || !instrumentTrackId.isValid())) {
        m_automationEnvelopesCache.erase(instrumentTrackId);
        return control;
    }

    const auto it = rebuild ? m_automationEnvelopesCache.try_emplace(instrumentTrackId).first
                    : m_automationEnvelopesCache.find(instrumentTrackId);
    if (it == m_automationEnvelopesCache.end()) {
        return control;
    }

    AutomationEnvelopes& envelopes = it->second;
    if (rebuildVolume) {
        envelopes.volume = buildAutomationEnvelope(AutomationCurveKey::instrument(AutomationType::Volume, instrumentTrackId));
    }
    if (rebuildPan) {
        envelopes.pan = buildAutomationEnvelope(AutomationCurveKey::instrument(AutomationType::Pan, instrumentTrackId));
    }

    if (!envelopes.volume && !envelopes.pan) {
        m_automationEnvelopesCache.erase(it);
        return control;
    }

    applyEnvelopes(envelopes, control);
    return control;
}

void PlaybackController::onAutomationDataChanged(const AutomationChanges& changes)
{
    if (changes.isFullReset) {
        resendMasterControlParams();
        resendTrackControlParams();
        return;
    }

    InstrumentTrackIdSet volumeTrackIds;
    InstrumentTrackIdSet panTrackIds;
    bool rebuildMasterVolume = false;
    bool rebuildMasterPan = false;

    // Envelopes are in seconds, so a tempo change moves them even if the total length doesn't change
    if (muse::contains(changes.affectedKeys, TEMPO_KEY)) {
        rebuildMasterVolume = m_masterAutomationEnvelopes.volume.has_value();
        rebuildMasterPan = m_masterAutomationEnvelopes.pan.has_value();
        collectEnvelopeTrackIds(volumeTrackIds, panTrackIds);
    }

    for (const AutomationCurveKey& key : changes.affectedKeys) {
        switch (key.type) {
        case AutomationType::Volume:
            if (key.isGlobal()) {
                rebuildMasterVolume = true;
            } else if (const std::optional<InstrumentTrackId> trackId = key.trackId()) {
                volumeTrackIds.insert(*trackId);
            }
            break;
        case AutomationType::Pan:
            if (key.isGlobal()) {
                rebuildMasterPan = true;
            } else if (const std::optional<InstrumentTrackId> trackId = key.trackId()) {
                panTrackIds.insert(*trackId);
            }
            break;
        default:
            break;
        }
    }

    if (rebuildMasterVolume || rebuildMasterPan) {
        resendMasterControlParams(rebuildMasterVolume, rebuildMasterPan);
    }

    if (!volumeTrackIds.empty() || !panTrackIds.empty()) {
        resendTrackControlParams(volumeTrackIds, panTrackIds);
    }
}

void PlaybackController::resendAutomationEnvelopes()
{
    const bool masterVolume = m_masterAutomationEnvelopes.volume.has_value();
    const bool masterPan = m_masterAutomationEnvelopes.pan.has_value();
    if (masterVolume || masterPan) {
        resendMasterControlParams(masterVolume, masterPan);
    }

    InstrumentTrackIdSet volumeTrackIds;
    InstrumentTrackIdSet panTrackIds;
    collectEnvelopeTrackIds(volumeTrackIds, panTrackIds);

    if (volumeTrackIds.empty() && panTrackIds.empty()) {
        return;
    }

    resendTrackControlParams(volumeTrackIds, panTrackIds);
}

void PlaybackController::collectEnvelopeTrackIds(InstrumentTrackIdSet& volumeTrackIds, InstrumentTrackIdSet& panTrackIds) const
{
    for (const auto& [trackId, envelopes] : m_automationEnvelopesCache) {
        if (envelopes.volume) {
            volumeTrackIds.insert(trackId);
        }
        if (envelopes.pan) {
            panTrackIds.insert(trackId);
        }
    }
}

void PlaybackController::resendTrackControlParams(std::optional<InstrumentTrackIdSet> volumeTrackIds,
                                                  std::optional<InstrumentTrackIdSet> panTrackIds)
{
    if (!playback()) {
        return;
    }

    for (const auto& pair : m_instrumentTrackIdMap) {
        const bool rebuildVolume = !volumeTrackIds || muse::contains(*volumeTrackIds, pair.first);
        const bool rebuildPan = !panTrackIds || muse::contains(*panTrackIds, pair.first);

        if (!rebuildVolume && !rebuildPan) {
            continue;
        }

        const AudioOutputParams outParams = trackOutputParams(pair.first);
        playback()->setControlParams(pair.second, trackControlParams(pair.first, outParams, rebuildVolume, rebuildPan));
    }
}

void PlaybackController::resendMasterControlParams(bool rebuildVolume, bool rebuildPan)
{
    if (!playback() || !audioSettings()) {
        return;
    }

    if (rebuildVolume) {
        m_masterAutomationEnvelopes.volume = buildAutomationEnvelope(AutomationCurveKey::global(AutomationType::Volume));
    }
    if (rebuildPan) {
        m_masterAutomationEnvelopes.pan = buildAutomationEnvelope(AutomationCurveKey::global(AutomationType::Pan));
    }

    playback()->setMasterControlParams(currentMasterControlParams());
}

std::optional<AutomationEnvelope> PlaybackController::buildAutomationEnvelope(const AutomationCurveKey& key) const
{
    const INotationAutomationPtr automation = m_masterNotation ? m_masterNotation->automation() : nullptr;
    const AutomationDataConstPtr automationData = automation ? automation->automationData() : nullptr;
    if (!automationData) {
        return std::nullopt;
    }

    const AutomationCurve& curve = automationData->curve(key);
    if (curve.empty()) {
        return std::nullopt;
    }

    AutomationEnvelope envelope;
    for (const auto& [tick, point] : curve) {
        envelope.insert({ playedTickToSecs(tick), point.value });
    }

    return envelope;
}

void PlaybackController::applyEnvelopes(const AutomationEnvelopes& envelopes, ControlParams& control)
{
    if (envelopes.volume) {
        control.volume = *envelopes.volume;
    }
    if (envelopes.pan) {
        control.balance = *envelopes.pan;
    }
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
    m_automationEnvelopesCache.erase(instrumentTrackId);
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

    if (!notationPlayback()) {
        return;
    }

    const AudioOutputParams& masterOutputParams = audioSettings()->masterAudioOutputParams();
    playback()->setMasterFxChainParams(masterOutputParams.fxChain);
    playback()->setMasterAuxSendsParams(masterOutputParams.auxSends);
    resendMasterControlParams();

    subscribeOnAudioParamsChanges();
    setupTracks();
    setupPlayer();

    m_isPlaybackInited = true;
    m_playbackInited.send(m_isPlaybackInited);
}

void PlaybackController::subscribeOnAudioParamsChanges()
{
    playback()->masterControlParamsChanged().onReceive(this, [this](const ControlParams& params) {
        AudioOutputParams outParams = audioSettings()->masterAudioOutputParams();
        outParams.setControl(params);
        audioSettings()->setMasterAudioOutputParams(outParams);
    });

    playback()->masterFxChainParamsChanged().onReceive(this, [this](const AudioFxChain& params) {
        AudioOutputParams outParams = audioSettings()->masterAudioOutputParams();
        outParams.fxChain = params;
        audioSettings()->setMasterAudioOutputParams(outParams);
    });

    playback()->masterAuxSendsParamsChanged().onReceive(this, [this](const AuxSendsParams& params) {
        AudioOutputParams outParams = audioSettings()->masterAudioOutputParams();
        outParams.auxSends = params;
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

    playback()->controlParamsChanged().onReceive(this, [this](const TrackId trackId, const ControlParams& params) {
        auto instrumentIt = std::find_if(m_instrumentTrackIdMap.begin(), m_instrumentTrackIdMap.end(), [trackId](const auto& pair) {
            return pair.second == trackId;
        });

        if (instrumentIt != m_instrumentTrackIdMap.end()) {
            AudioOutputParams outParams = audioSettings()->trackOutputParams(instrumentIt->first);
            outParams.setControl(params);
            audioSettings()->setTrackOutputParams(instrumentIt->first, outParams);
            return;
        }

        auto auxIt = std::find_if(m_auxTrackIdMap.begin(), m_auxTrackIdMap.end(), [trackId](const auto& pair) {
            return pair.second == trackId;
        });

        if (auxIt != m_auxTrackIdMap.end()) {
            aux_channel_idx_t auxIdx = auxIt->first;
            AudioOutputParams outParams = audioSettings()->auxOutputParams(auxIdx);
            outParams.setControl(params);
            audioSettings()->setAuxOutputParams(auxIdx, outParams);
        }
    });

    playback()->auxSendsParamsChanged().onReceive(this, [this](const TrackId trackId, const AuxSendsParams& params) {
        auto instrumentIt = std::find_if(m_instrumentTrackIdMap.begin(), m_instrumentTrackIdMap.end(), [trackId](const auto& pair) {
            return pair.second == trackId;
        });

        if (instrumentIt != m_instrumentTrackIdMap.end()) {
            AudioOutputParams outParams = audioSettings()->trackOutputParams(instrumentIt->first);
            outParams.auxSends = params;
            audioSettings()->setTrackOutputParams(instrumentIt->first, outParams);
            return;
        }

        auto auxIt = std::find_if(m_auxTrackIdMap.begin(), m_auxTrackIdMap.end(), [trackId](const auto& pair) {
            return pair.second == trackId;
        });

        if (auxIt != m_auxTrackIdMap.end()) {
            aux_channel_idx_t auxIdx = auxIt->first;
            AudioOutputParams outParams = audioSettings()->auxOutputParams(auxIdx);
            outParams.auxSends = params;
            audioSettings()->setAuxOutputParams(auxIdx, outParams);
        }
    });
}

void PlaybackController::setupTracks()
{
    m_instrumentTrackIdMap.clear();

    if (!masterNotationParts()) {
        return;
    }

    const bool projectHadNoAudioSettings = !audioSettings()->hasAnyAudioSettings();

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
        addTrack(trackId, projectHadNoAudioSettings, onAddFinished);
    }

    for (aux_channel_idx_t idx = 0; idx < AUX_CHANNEL_NUM; ++idx) {
        addAuxTrack(idx, projectHadNoAudioSettings, onAddFinished);
    }

    m_loadingProgress.progress(0, trackCount, title);

    notationPlayback()->trackAdded().onReceive(this, [this, onAddFinished](const InstrumentTrackId& instrumentTrackId) {
        addTrack(instrumentTrackId, false /*projectHadNoAudioSettings*/, onAddFinished);
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

    m_isPlayAllowedChanged.send(isPlayAllowed());
}

void PlaybackController::setupPlayer()
{
    currentPlayer()->playbackPositionChanged().onReceive(this, [this](const audio::secs_t pos) {
        m_playbackPositionChanged.send(pos);

        m_currentTick = notationPlayback()->secToTick(pos);

        if (isMidiRecording()) {
            m_midiRecordingController->onPlaybackPositionChanged(pos);
        }

        updateCurrentTempo();

        secs_t endSecs = totalPlayTime();
        if (pos + muse::msecs_to_secs(1) >= endSecs) {
            doStop();
        }
    });

    currentPlayer()->playbackStatusChanged().onReceive(this, [this](PlaybackStatus) {
        onPlaybackStatusChanged();
        onMidiRecordingPlaybackStatusChanged();
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
        playback()->setControlParams(trackId, trackControlParams(instrumentTrackId, params, /*rebuildVolume*/ false, /*rebuildPan*/ false));
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

        if (const AutomationDataConstPtr automationData = m_masterNotation->automation()->automationData()) {
            automationData->changed().disconnect(this);
        }
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

    if (const AutomationDataConstPtr automationData = m_masterNotation->automation()->automationData()) {
        automationData->changed().onReceive(this, [this](const AutomationChanges& changes) {
            onAutomationDataChanged(changes);
        });
    }
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
