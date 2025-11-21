/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "onlinesoundscontroller.h"

#include "audio/common/audioerrors.h"

#include "log.h"
#include "translation.h"

using namespace muse;
using namespace mu::playback;
using namespace muse::audio;

static Ret retFromProcessingStatus(const InputProcessingProgress::StatusInfo& status)
{
    Ret ret(status.errorCode, status.errorText);
    for (const auto& pair : status.data) {
        ret.setData(pair.first, pair.second);
    }

    return ret;
}

OnlineSoundsController::OnlineSoundsController()
    : m_onlineSoundsProcessingRet(make_ok())
{
}

void OnlineSoundsController::regActions()
{
    dispatcher()->reg(this, "process-online-sounds", this, &OnlineSoundsController::processOnlineSounds);
    dispatcher()->reg(this, "clear-online-sounds-cache", this, &OnlineSoundsController::clearOnlineSoundsCache);
}

void OnlineSoundsController::setCurrentSequence(TrackSequenceId seqId)
{
    m_currentSequenceId = seqId;
}

void OnlineSoundsController::resetCurrentSequence()
{
    const bool hadOnlineSounds = !m_onlineSounds.empty();
    m_currentSequenceId = -1;
    m_onlineSounds.clear();
    m_onlineSoundsBeingProcessed.clear();
    m_onlineLibrariesWithExceededLimit.clear();
    m_onlineSoundsProcessingRet = make_ok();

    if (hadOnlineSounds) {
        m_onlineSoundsChanged.notify();
    }
}

const std::map<TrackId, AudioResourceMeta>& OnlineSoundsController::onlineSounds() const
{
    return m_onlineSounds;
}

muse::async::Notification OnlineSoundsController::onlineSoundsChanged() const
{
    return m_onlineSoundsChanged;
}

muse::Progress OnlineSoundsController::onlineSoundsProcessingProgress() const
{
    return m_onlineSoundsProcessingProgress;
}

void OnlineSoundsController::addOnlineTrack(const TrackId trackId, const AudioResourceMeta& meta)
{
    auto it = m_onlineSounds.find(trackId);
    if (it != m_onlineSounds.end() && it->second == meta) {
        return;
    }

    m_onlineSounds.insert_or_assign(trackId, meta);
    listenProcessingProgress(trackId);
    m_onlineSoundsChanged.notify();
}

void OnlineSoundsController::removeOnlineTrack(const TrackId trackId)
{
    if (!muse::contains(m_onlineSounds, trackId)) {
        return;
    }

    muse::remove(m_onlineSounds, trackId);
    muse::remove(m_onlineSoundsBeingProcessed, trackId);

    if (m_onlineSoundsProcessingProgress.isStarted() && m_onlineSoundsBeingProcessed.empty()) {
        m_onlineSoundsProcessingProgress.finish(make_ret(Ret::Code::Cancel));
    }

    m_onlineSoundsChanged.notify();
}

void OnlineSoundsController::listenProcessingProgress(const TrackId trackId)
{
    playback()->inputProcessingProgress(m_currentSequenceId, trackId)
    .onResolve(this, [this, trackId](InputProcessingProgress inputProgress) {
        inputProgress.processedChannel.onReceive(this, [this, trackId]
                                                 (const InputProcessingProgress::StatusInfo& status,
                                                  const InputProcessingProgress::ChunkInfoList& /*chunks*/,
                                                  const InputProcessingProgress::ProgressInfo& progress)
        {
            switch (status.status) {
                case InputProcessingProgress::Undefined:
                    break;
                case InputProcessingProgress::Started: {
                    m_onlineSoundsBeingProcessed.insert(trackId);

                    if (!m_onlineSoundsProcessingProgress.isStarted()) {
                        m_onlineSoundsProcessingRet = make_ok();
                        m_onlineSoundsProcessingProgress.start();
                    }
                } break;
                case InputProcessingProgress::Processing: {
                    if (m_onlineSoundsBeingProcessed.size() == 1) {
                        m_onlineSoundsProcessingProgress.progress(progress.current, progress.total);
                    }
                } break;
                case InputProcessingProgress::Finished: {
                    muse::remove(m_onlineSoundsBeingProcessed, trackId);

                    if (status.errorCode != 0 && status.errorCode != (int)Ret::Code::Cancel) {
                        m_onlineSoundsProcessingRet = retFromProcessingStatus(status);
                        LOGE() << "Error during online sounds processing: " << status.errorText << ", track: " << trackId;

                        if (status.errorCode == (int)Err::OnlineSoundsLimitReached) {
                            showLimitReachedErrorIfNeed(status);
                        }
                    }

                    if (m_onlineSoundsBeingProcessed.empty()) {
                        m_onlineSoundsProcessingProgress.finish(m_onlineSoundsProcessingRet);
                    }
                } break;
            }
        });
    });
}

bool OnlineSoundsController::shouldShowOnlineSoundsProcessingError(bool isPlaying) const
{
    if (m_onlineSounds.empty() || !configuration()->shouldShowOnlineSoundsProcessingError() || isPlaying) {
        return false;
    }

    if (!m_onlineSoundsProcessingRet) {
        const int code = m_onlineSoundsProcessingRet.code();
        return code != (int)Ret::Code::Cancel && code != (int)Err::OnlineSoundsLimitReached;
    }

    return false;
}

void OnlineSoundsController::showOnlineSoundsProcessingError(const std::function<void()>& onShown)
{
    const std::string text = muse::qtrc("playback", "This may be due to a poor internet connection or server issue. "
                                                    "Your score will still play, but some sounds may be missing. "
                                                    "Please check your connection, and make sure MuseHub is running and you are logged in. "
                                                    "<a href=\"%1\">Learn more here</a>.")
                             .arg(configuration()->onlineSoundsHandbookUrl()).toStdString();

    auto promise = interactive()->warning(muse::trc("playback", "Some online sounds aren’t ready yet"), text,
                                          { IInteractive::Button::Ok }, IInteractive::Button::Ok,
                                          IInteractive::Option::WithIcon | IInteractive::Option::WithDontShowAgainCheckBox);

    promise.onResolve(this, [this, onShown](const IInteractive::Result& res) {
        if (!res.showAgain()) {
            configuration()->setShouldShowOnlineSoundsProcessingError(false);
        }

        if (onShown) {
            onShown();
        }
    });
}

void OnlineSoundsController::showLimitReachedErrorIfNeed(const InputProcessingProgress::StatusInfo& status)
{
    IF_ASSERT_FAILED(status.errorCode == (int)audio::Err::OnlineSoundsLimitReached) {
        return;
    }

    const String libName = String::fromStdString(muse::value(status.data, "libraryName"));
    const String date = String::fromStdString(muse::value(status.data, "date"));
    const String url = String::fromStdString(muse::value(status.data, "url"));

    IF_ASSERT_FAILED(!libName.empty() && !date.empty() && !url.empty()) {
        return;
    }

    if (muse::contains(m_onlineLibrariesWithExceededLimit, libName)) {
        return;
    }
    m_onlineLibrariesWithExceededLimit.insert(libName);

    const std::string text = muse::qtrc("playback", "You’ve reached your current render limit for %1. "
                                                    "You will be able to process online sounds again after your quota resets on %2. "
                                                    "More info: <a href=\"%3\">%3</a>.")
                             .arg(libName)
                             .arg(date)
                             .arg(url).toStdString();

    interactive()->warning(muse::trc("playback", "Unable to process online sounds"), text);
}

void OnlineSoundsController::processOnlineSounds()
{
    IF_ASSERT_FAILED(playback()) {
        return;
    }

    for (const auto& pair : m_onlineSounds) {
        playback()->processInput(m_currentSequenceId, pair.first);
    }
}

void OnlineSoundsController::clearOnlineSoundsCache()
{
    auto promise = interactive()->warning(
        muse::trc("playback", "Are you sure you want to clear online sounds cache?"),
        muse::trc("playback", "This will delete online sounds data stored on your computer for this score. "
                              "Online sounds processing will try to restart immediately."),
        { IInteractive::Button::Ok, IInteractive::Button::Cancel },
        IInteractive::Button::Ok);

    promise.onResolve(this, [this](const IInteractive::Result& res) {
        if (!res.isButton(IInteractive::Button::Ok)) {
            return;
        }

        IF_ASSERT_FAILED(playback()) {
            return;
        }

        for (const auto& pair : m_onlineSounds) {
            playback()->clearCache(m_currentSequenceId, pair.first);
        }
    });
}
