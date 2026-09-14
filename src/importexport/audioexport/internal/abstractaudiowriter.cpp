/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "abstractaudiowriter.h"

#include <limits>
#include <optional>

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QThread>

#include "global/containers.h"
#include "log.h"

#include "notation/imasternotation.h"
#include "notation/inotation.h"
#include "notation/inotationplayback.h"

using namespace muse;
using namespace muse::audio;
using namespace mu::iex::audioexport;
using namespace mu::project;
using namespace mu::notation;

std::vector<INotationWriter::UnitType> AbstractAudioWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PART };
}

bool AbstractAudioWriter::supportsUnitType(UnitType unitType) const
{
    std::vector<UnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

Ret AbstractAudioWriter::write(INotationPtr, io::IODevice&, const Options& options)
{
    IF_ASSERT_FAILED(unitTypeFromOptions(options) != UnitType::MULTI_PART) {
        return Ret(Ret::Code::NotSupported);
    }

    if (supportsUnitType(muse::value(options, OptionKey::UNIT_TYPE, Val(UnitType::PER_PAGE)).toEnum<UnitType>())) {
        NOT_IMPLEMENTED;
        return Ret(Ret::Code::NotImplemented);
    }

    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}

Ret AbstractAudioWriter::writeList(const INotationPtrList&, io::IODevice&, const Options& options)
{
    IF_ASSERT_FAILED(unitTypeFromOptions(options) == UnitType::MULTI_PART) {
        return Ret(Ret::Code::NotSupported);
    }

    if (supportsUnitType(muse::value(options, OptionKey::UNIT_TYPE, Val(UnitType::PER_PAGE)).toEnum<UnitType>())) {
        NOT_IMPLEMENTED;
        return Ret(Ret::Code::NotImplemented);
    }

    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}

void AbstractAudioWriter::abort()
{
    muse::ContextInject<muse::audio::IPlayback> playback = { m_iocContext };
    playback()->abortSavingAllSoundTracks();
    m_writeRet = make_ret(Ret::Code::Cancel);
    m_isCompleted = true;
}

muse::Progress* AbstractAudioWriter::progress()
{
    return &m_progress;
}

Ret AbstractAudioWriter::doWriteAndWait(INotationPtr notation,
                                        io::IODevice& dstDevice,
                                        const SoundTrackFormat& format,
                                        const Options& options)
{
    //! NOTE Temporary fix for the context injection
    m_iocContext = notation->iocContext();

    muse::ContextInject<playback::IPlaybackController> playbackController = { m_iocContext };

    //! NOTE Waiting for the audio system to start if it is not already running
    while (!startAudioController()->isAudioStarted()) {
        application()->processEvents();
        QThread::yieldCurrentThread();
    }

    //! NOTE Playback (tracks and duration) loads asynchronously; rendering before it is
    //! ready yields 0 tracks and 0 duration ("No audio to export"). Wait for it first.
    while (!playbackController()->isPlaybackInited()) {
        application()->processEvents();
        QThread::yieldCurrentThread();
    }

    m_isCompleted = false;
    m_writeRet = muse::Ret();

    playbackController()->setNotation(notation);

    SoundTrackFormat actualFormat = format;

    double leadingSilenceSec = muse::value(options, OptionKey::LEADING_SILENCE_SEC, Val(0.0)).toDouble();
    actualFormat.leadingSilenceDuration = std::isfinite(leadingSilenceSec)
                                          ? static_cast<msecs_t>(leadingSilenceSec) : msecs_t(0);

    double trailingSilenceSec = muse::value(options, OptionKey::TRAILING_SILENCE_SEC, Val(0.0)).toDouble();
    actualFormat.trailingSilenceDuration = std::isfinite(trailingSilenceSec)
                                           ? static_cast<msecs_t>(trailingSilenceSec) : msecs_t(0);

    SoundTrackSaveOptions saveOptions;
    std::optional<bool> selectionMetronomeEnabled;
    const auto startTickIt = options.find(OptionKey::AUDIO_EXPORT_START_TICK);
    const auto endTickIt = options.find(OptionKey::AUDIO_EXPORT_END_TICK);

    if ((startTickIt == options.end()) != (endTickIt == options.end())) {
        return make_ret(Ret::Code::BadArgs, std::string("audio export range requires both start and end ticks"));
    }

    if (startTickIt != options.end()) {
        const int64_t startTickValue = startTickIt->second.toInt64();
        const int64_t endTickValue = endTickIt->second.toInt64();
        if (startTickValue < 0 || endTickValue <= startTickValue
            || endTickValue > std::numeric_limits<midi::tick_t>::max()) {
            return make_ret(Ret::Code::BadArgs, std::string("invalid audio export tick range"));
        }
        const notation::INotationPlaybackPtr notationPlayback = notation->masterNotation()->playback();
        if (!notationPlayback) {
            return make_ret(Ret::Code::InternalError, std::string("notation playback is unavailable"));
        }

        const auto tempoPercentageIt = options.find(OptionKey::AUDIO_EXPORT_TEMPO_PERCENT);
        if (tempoPercentageIt != options.end()) {
            const double tempoPercentage = tempoPercentageIt->second.toDouble();
            if (!std::isfinite(tempoPercentage) || tempoPercentage < 10.0 || tempoPercentage > 300.0) {
                return make_ret(Ret::Code::BadArgs, std::string("invalid audio export tempo percentage"));
            }

            m_tempoMultiplierForRestore = playbackController()->tempoMultiplier();
            m_shouldRestoreTempoMultiplier = true;
            playbackController()->setTempoMultiplier(tempoPercentage / 100.0);
        }

        const RetVal<midi::tick_t> startPlayedTick
            = notationPlayback->playPositionTickByRawTick(static_cast<midi::tick_t>(startTickValue));
        const RetVal<midi::tick_t> endPlayedTick
            = notationPlayback->playPositionTickByRawTick(static_cast<midi::tick_t>(endTickValue));
        if (!startPlayedTick.ret || !endPlayedTick.ret) {
            if (m_shouldRestoreTempoMultiplier) {
                playbackController()->setTempoMultiplier(m_tempoMultiplierForRestore);
                m_shouldRestoreTempoMultiplier = false;
            }
            return make_ret(Ret::Code::BadData, std::string("could not map the selected score range to playback time"));
        }

        const secs_t selectionStartTime = notationPlayback->playedTickToSec(startPlayedTick.val);
        const secs_t selectionEndTime = notationPlayback->playedTickToSec(endPlayedTick.val);
        const double fadeInSeconds = muse::value(options, OptionKey::AUDIO_EXPORT_FADE_IN_SEC, Val(0.0)).toDouble();
        const double fadeOutSeconds = muse::value(options, OptionKey::AUDIO_EXPORT_FADE_OUT_SEC, Val(0.0)).toDouble();
        if (!std::isfinite(fadeInSeconds) || fadeInSeconds < 0.0 || fadeInSeconds > 30.0
            || !std::isfinite(fadeOutSeconds) || fadeOutSeconds < 0.0 || fadeOutSeconds > 30.0) {
            if (m_shouldRestoreTempoMultiplier) {
                playbackController()->setTempoMultiplier(m_tempoMultiplierForRestore);
                m_shouldRestoreTempoMultiplier = false;
            }
            return make_ret(Ret::Code::BadArgs, std::string("invalid selection export fade duration"));
        }

        const secs_t totalPlayTime = notationPlayback->totalPlayTime();
        saveOptions.hasTimeRange = true;
        saveOptions.startTime = std::max(secs_t(0.0), selectionStartTime - secs_t(fadeInSeconds));
        saveOptions.endTime = std::min(totalPlayTime, selectionEndTime + secs_t(fadeOutSeconds));
        saveOptions.fadeInDuration = selectionStartTime - saveOptions.startTime;
        saveOptions.fadeOutDuration = saveOptions.endTime - selectionEndTime;
        if (!saveOptions.isValid()) {
            if (m_shouldRestoreTempoMultiplier) {
                playbackController()->setTempoMultiplier(m_tempoMultiplierForRestore);
                m_shouldRestoreTempoMultiplier = false;
            }
            return make_ret(Ret::Code::BadData, std::string("selected score range has no playable duration"));
        }
    }

    const auto metronomeEnabledIt = options.find(OptionKey::AUDIO_EXPORT_METRONOME_ENABLED);
    if (metronomeEnabledIt != options.end()) {
        if (!saveOptions.hasTimeRange) {
            if (m_shouldRestoreTempoMultiplier) {
                playbackController()->setTempoMultiplier(m_tempoMultiplierForRestore);
                m_shouldRestoreTempoMultiplier = false;
            }
            return make_ret(Ret::Code::BadArgs, std::string("selection export metronome requires a time range"));
        }
        selectionMetronomeEnabled = metronomeEnabledIt->second.toBool();
    }

    const auto partVolumesIt = options.find(OptionKey::AUDIO_EXPORT_PART_VOLUMES);
    if (partVolumesIt != options.end()) {
        playback::IPlaybackController::PartVolumeMap partVolumes;
        bool validPartVolumes = saveOptions.hasTimeRange;

        for (const Val& partVolumeValue : partVolumesIt->second.toList()) {
            const ValMap partVolume = partVolumeValue.toMap();
            const auto partIdIt = partVolume.find("partId");
            const auto volumeIt = partVolume.find("volume");
            if (partIdIt == partVolume.end() || volumeIt == partVolume.end()) {
                validPartVolumes = false;
                break;
            }

            const muse::ID partId(partIdIt->second.toString());
            const int volume = volumeIt->second.toInt();
            if (!partId.isValid() || volume < 0 || volume > 200) {
                validPartVolumes = false;
                break;
            }

            partVolumes.insert_or_assign(partId, volume);
        }

        if (!validPartVolumes || partVolumes.empty()) {
            if (m_shouldRestoreTempoMultiplier) {
                playbackController()->setTempoMultiplier(m_tempoMultiplierForRestore);
                m_shouldRestoreTempoMultiplier = false;
            }
            return make_ret(Ret::Code::BadArgs, std::string("invalid selection export part volumes"));
        }

        playbackController()->setSelectionExportTrackVolumes(partVolumes);
    } else {
        playbackController()->setIsExportingAudio(true);
    }

    if (selectionMetronomeEnabled.has_value()) {
        playbackController()->setSelectionExportMetronomeEnabled(*selectionMetronomeEnabled);
    }

    doWrite(dstDevice, actualFormat, saveOptions);

    const bool waitForCompletion = muse::value(options, OptionKey::WAIT_FOR_COMPLETION, Val(true)).toBool();
    if (waitForCompletion) {
        while (!m_isCompleted) {
            application()->processEvents();
            QThread::yieldCurrentThread();
        }
    }

    return m_writeRet;
}

void AbstractAudioWriter::doWrite(io::IODevice& dstDevice, const SoundTrackFormat& format,
                                  const SoundTrackSaveOptions& saveOptions)
{
    muse::ContextInject<muse::audio::IPlayback> playbackInj = { m_iocContext };

    const std::string processingOnlineSoundsMsg = trc("iex_audio", "Processing online sounds…");

    muse::ContextInject<context::IGlobalContext> globalContext = { m_iocContext };
    m_notationForRestore = globalContext()->currentNotation();

    auto restorePlaybackState = [this]() {
        muse::ContextInject<playback::IPlaybackController> playbackController = { m_iocContext };
        if (m_shouldRestoreTempoMultiplier) {
            playbackController()->setTempoMultiplier(m_tempoMultiplierForRestore);
            m_shouldRestoreTempoMultiplier = false;
        }
        playbackController()->setIsExportingAudio(false);
        playbackController()->setNotation(m_notationForRestore);
    };

    auto sendProgress = [this, processingOnlineSoundsMsg](int64_t current, int64_t total, SaveSoundTrackStage stage) {
        switch (stage) {
        case SaveSoundTrackStage::ProcessingOnlineSounds:
            m_progress.progress(current, total, processingOnlineSoundsMsg);
            break;
        case SaveSoundTrackStage::WritingSoundTrack:
        case SaveSoundTrackStage::Unknown:
            m_progress.progress(current, total);
            break;
        }
    };

    m_progress.start();

    auto playback = playbackInj();

    playback->saveSoundTrackProgressChanged()
    .onReceive(this, [sendProgress](int64_t current, int64_t total, SaveSoundTrackStage stage) {
        sendProgress(current, total, stage);
    });

    playback->saveSoundTrack(std::move(format), dstDevice, saveOptions)
    .onResolve(this, [this, playback, restorePlaybackState](const bool /*result*/) {
        LOGI() << "Successfully saved sound track";

        restorePlaybackState();

        m_writeRet = muse::make_ok();
        m_isCompleted = true;
        m_progress.finish(muse::make_ok());
        playback->saveSoundTrackProgressChanged().disconnect(this);
    })
    .onReject(this, [this, playback, restorePlaybackState](int errorCode, const std::string& msg) {
        restorePlaybackState();

        m_writeRet = Ret(errorCode, msg);
        m_isCompleted = true;
        m_progress.finish(make_ret(errorCode, msg));
        playback->saveSoundTrackProgressChanged().disconnect(this);
    });
}

INotationWriter::UnitType AbstractAudioWriter::unitTypeFromOptions(const Options& options) const
{
    std::vector<UnitType> supported = supportedUnitTypes();
    IF_ASSERT_FAILED(!supported.empty()) {
        return UnitType::PER_PART;
    }

    UnitType defaultUnitType = supported.front();
    UnitType unitType = muse::value(options, OptionKey::UNIT_TYPE, Val(defaultUnitType)).toEnum<UnitType>();
    if (!supportsUnitType(unitType)) {
        return defaultUnitType;
    }

    return unitType;
}
