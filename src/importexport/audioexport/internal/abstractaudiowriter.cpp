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

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QThread>

#include "global/containers.h"
#include "log.h"

#include "notation/inotation.h"

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

Ret AbstractAudioWriter::writeSync(INotationPtr notation, io::IODevice& dstDevice, const Options& options)
{
    IF_ASSERT_FAILED(unitTypeFromOptions(options) == UnitType::PER_PART) {
        return Ret(Ret::Code::NotSupported);
    }

    //! NOTE Blocking variant, for the callers that can't wait asynchronously
    bool isFinished = false;
    Ret result;

    doWriteAsync(notation, dstDevice, options, [&isFinished, &result](const Ret& ret) {
        result = ret;
        isFinished = true;
    });

    while (!isFinished) {
        application()->processEvents();
        QThread::yieldCurrentThread();
    }

    return result;
}

async::Promise<Ret> AbstractAudioWriter::write(INotationPtr notation, io::IODevice& dstDevice, const Options& options)
{
    IF_ASSERT_FAILED(unitTypeFromOptions(options) == UnitType::PER_PART) {
        return async::make_promise<Ret>([](auto resolve) {
            return resolve(Ret(Ret::Code::NotSupported));
        });
    }

    return async::make_promise<Ret>([this, notation, &dstDevice, options](auto resolve) {
        doWriteAsync(notation, dstDevice, options, [resolve](const Ret& ret) {
            (void)resolve(ret);
        });

        return async::Promise<Ret>::Result::unchecked();
    });
}

Ret AbstractAudioWriter::writeListSync(const INotationPtrList&, io::IODevice&, const Options& options)
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
    if (m_iocContext) {
        //! NOTE If the writing is still waiting for the audio system or the playback, it must not start later
        muse::ContextInject<playback::IPlaybackController> playbackController = { m_iocContext };
        startAudioController()->isAudioStartedChanged().disconnect(this);
        playbackController()->playbackInitedChanged().disconnect(this);
    }

    muse::ContextInject<muse::audio::IPlayback> playback = { m_iocContext };
    playback()->abortSavingAllSoundTracks();

    //! NOTE The engine settles the saving later, that must not touch what is going on then (e.g. the next write),
    //! so the saving is finished here
    ++m_writeId;

    const Ret ret = make_ret(Ret::Code::Cancel);
    if (m_isSaving) {
        finishSaving(ret);
    } else {
        finish(ret);
    }
}

muse::Progress* AbstractAudioWriter::progress()
{
    return &m_progress;
}

void AbstractAudioWriter::whenReady(const std::function<bool()>& isReady, const std::function<async::Channel<bool>()>& changed,
                                    const std::function<void()>& callback)
{
    if (isReady()) {
        callback();
        return;
    }

    //! NOTE The channel is not captured (only the way to get it), otherwise the callback would hold the channel that holds it,
    //! and it would never be released if the channel doesn't notify (e.g. the writing is aborted)
    changed().onReceive(this, [this, isReady, changed, callback](bool) {
        if (!isReady()) {
            return;
        }

        changed().disconnect(this);
        callback();
    }, Asyncable::Mode::SetReplace);
}

void AbstractAudioWriter::doWriteAsync(INotationPtr notation, io::IODevice& dstDevice, const Options& options,
                                       const Finished& onFinished)
{
    //! NOTE Temporary fix for the context injection
    m_iocContext = notation->iocContext();

    //! NOTE Stored before waiting, so that abort() can complete the writing that has not started yet
    m_onFinished = onFinished;
    const uint64_t writeId = ++m_writeId;

    muse::ContextInject<playback::IPlaybackController> playbackController = { m_iocContext };

    //! NOTE Waiting for the audio system to start if it is not already running,
    //! and then for the playback (tracks and duration), which loads asynchronously:
    //! rendering before it is ready yields 0 tracks and 0 duration ("No audio to export")
    auto start = [this, notation, &dstDevice, options, onFinished, writeId]() {
        if (writeId != m_writeId) {
            return;
        }

        muse::ContextInject<playback::IPlaybackController> controller = { m_iocContext };

        controller()->setNotation(notation);
        controller()->setIsExportingAudio(true);

        SoundTrackFormat actualFormat = soundTrackFormat();

        double leadingSilenceSec = muse::value(options, OptionKey::LEADING_SILENCE_SEC, Val(0.0)).toDouble();
        actualFormat.leadingSilenceDuration = std::isfinite(leadingSilenceSec) ? secs_t(leadingSilenceSec) : secs_t(0.0);

        double trailingSilenceSec = muse::value(options, OptionKey::TRAILING_SILENCE_SEC, Val(0.0)).toDouble();
        actualFormat.trailingSilenceDuration = std::isfinite(trailingSilenceSec) ? secs_t(trailingSilenceSec) : secs_t(0.0);

        const bool waitForCompletion = muse::value(options, OptionKey::WAIT_FOR_COMPLETION, Val(true)).toBool();
        if (!waitForCompletion) {
            m_onFinished = nullptr;
        }

        doWrite(dstDevice, actualFormat, writeId);

        if (!waitForCompletion) {
            onFinished(muse::make_ok());
        }
    };

    whenReady([this]() { return startAudioController()->isAudioStarted(); },
              [this]() { return startAudioController()->isAudioStartedChanged(); },
              [this, playbackController, start]() {
        whenReady([playbackController]() { return playbackController()->isPlaybackInited(); },
                  [playbackController]() { return playbackController()->playbackInitedChanged(); },
                  start);
    });
}

void AbstractAudioWriter::finish(const Ret& ret)
{
    Finished onFinished = std::move(m_onFinished);
    m_onFinished = nullptr;

    if (onFinished) {
        onFinished(ret);
    }
}

void AbstractAudioWriter::finishSaving(const Ret& ret)
{
    muse::ContextInject<playback::IPlaybackController> playbackController = { m_iocContext };
    playbackController()->setIsExportingAudio(false);
    playbackController()->setNotation(m_notationForRestore);

    muse::ContextInject<muse::audio::IPlayback> playback = { m_iocContext };
    playback()->saveSoundTrackProgressChanged().disconnect(this);

    m_isSaving = false;
    m_progress.finish(ret);

    finish(ret);
}

void AbstractAudioWriter::doWrite(io::IODevice& dstDevice, const SoundTrackFormat& format, uint64_t writeId)
{
    muse::ContextInject<muse::audio::IPlayback> playbackInj = { m_iocContext };

    const std::string processingOnlineSoundsMsg = trc("iex_audio", "Processing online sounds…");

    muse::ContextInject<project::IProjectSession> projectSession = { m_iocContext };
    m_notationForRestore = projectSession()->currentNotation();

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

    m_isSaving = true;
    m_progress.start();

    auto playback = playbackInj();

    playback->saveSoundTrackProgressChanged()
    .onReceive(this, [this, writeId, sendProgress](int64_t current, int64_t total, SaveSoundTrackStage stage) {
        if (writeId != m_writeId) {
            return;
        }

        sendProgress(current, total, stage);
    });

    playback->saveSoundTrack(std::move(format), dstDevice)
    .onResolve(this, [this, writeId](const bool /*result*/) {
        if (writeId != m_writeId) {
            return;
        }

        LOGI() << "Successfully saved sound track";

        finishSaving(muse::make_ok());
    })
    .onReject(this, [this, writeId](int errorCode, const std::string& msg) {
        if (writeId != m_writeId) {
            return;
        }

        finishSaving(make_ret(errorCode, msg));
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
