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

#include "onlinesoundsstatusmodel.h"

#include "audio/common/audioerrors.h"

#include "async/async.h"
#include "translation.h"

using namespace mu::notation;
using namespace mu::playback;
using namespace muse::audio;

static QString formatLimitReachedError(const muse::Ret& ret)
{
    if (ret.code() != (int)Err::OnlineSoundsLimitReached) {
        return QString();
    }

    const QString libName = QString::fromStdString(ret.data<std::string>("libraryName", std::string()));
    const QString date = QString::fromStdString(ret.data<std::string>("date", std::string()));

    if (libName.isEmpty() || date.isEmpty()) {
        return QString();
    }

    const QString text = muse::qtrc("playback", "You’ve reached your current render limit for %1. "
                                                "You will be able to process online sounds again after your quota resets on %2.")
                         .arg(libName)
                         .arg(date);
    return text;
}

OnlineSoundsStatusModel::OnlineSoundsStatusModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this)), m_ret(muse::make_ok())
{
}

void OnlineSoundsStatusModel::load()
{
    onOnlineSoundsChanged();
    playbackController()->onlineSoundsChanged().onNotify(this, [this]() {
        onOnlineSoundsChanged();
    });

    globalContext()->currentMasterNotationChanged().onNotify(this, [this]() {
        updateManualProcessingAllowed();
    });

    audioConfiguration()->autoProcessOnlineSoundsInBackgroundChanged().onReceive(this, [this](bool) {
        updateManualProcessingAllowed(false /*enableByDefault*/);
    });

    muse::Progress progress = playbackController()->onlineSoundsProcessingProgress();
    setStatus(progress.isStarted() ? Status::Processing : Status::Success);

    progress.started().onNotify(this, [this]() {
        setStatus(Status::Processing);
    });

    progress.finished().onReceive(this, [this](const muse::ProgressResult& result) {
        m_ret = result.ret;
        setStatus(result.ret ? Status::Success : Status::Error);

        if (result.ret && m_manualProcessingAllowed) {
            setManualProcessingAllowed(false);
        }
    });
}

void OnlineSoundsStatusModel::processOnlineSounds()
{
    if (m_manualProcessingAllowed) {
        dispatcher()->dispatch("process-online-sounds");
    }
}

bool OnlineSoundsStatusModel::hasOnlineSounds() const
{
    return !m_onlineTrackIdSet.empty();
}

bool OnlineSoundsStatusModel::manualProcessingAllowed() const
{
    return m_manualProcessingAllowed;
}

int OnlineSoundsStatusModel::status() const
{
    return static_cast<int>(m_status);
}

QString OnlineSoundsStatusModel::errorTitle() const
{
    if (!m_ret) {
        if (m_ret.code() == (int)Err::OnlineSoundsProcessingError) {
            return muse::qtrc("playback", "Some online sounds aren’t ready yet");
        } else if (m_ret.code() != (int)muse::Ret::Code::Cancel) {
            return muse::qtrc("playback", "Unable to process online sounds");
        }
    }

    return QString();
}

QString OnlineSoundsStatusModel::errorDescription() const
{
    if (!m_ret) {
        if (m_ret.code() == (int)Err::OnlineSoundsLimitReached) {
            return formatLimitReachedError(m_ret);
        } else if (m_ret.code() != (int)muse::Ret::Code::Cancel) {
            return muse::qtrc("playback", "Please check your connection, and make sure MuseHub is running and you are logged in.");
        }
    }

    return QString();
}

void OnlineSoundsStatusModel::onOnlineSoundsChanged()
{
    const std::map<TrackId, AudioResourceMeta>& onlineSounds = playbackController()->onlineSounds();
    const IPlaybackController::InstrumentTrackIdMap& instrumentTrackIdMap = playbackController()->instrumentTrackIdMap();

    const bool wasEmpty = m_onlineTrackIdSet.empty();
    m_onlineTrackIdSet.clear();
    m_onlineTrackIdSet.reserve(onlineSounds.size());

    for (const auto& pair : instrumentTrackIdMap) {
        if (muse::contains(onlineSounds, pair.second)) {
            m_onlineTrackIdSet.insert(pair.first);
        }
    }

    if (m_onlineTrackIdSet.empty() != wasEmpty) {
        emit hasOnlineSoundsChanged();
    }

    updateManualProcessingAllowed();
}

void OnlineSoundsStatusModel::updateManualProcessingAllowed(bool enableByDefault)
{
    if (m_onlineTrackIdSet.empty() || audioConfiguration()->autoProcessOnlineSoundsInBackground()) {
        m_tracksDataChanged.disconnect(this);
        setManualProcessingAllowed(false);
        return;
    }

    IMasterNotationPtr master = globalContext()->currentMasterNotation();
    if (!master) {
        m_tracksDataChanged.disconnect(this);
        setManualProcessingAllowed(false);
        return;
    }

    setManualProcessingAllowed(enableByDefault);

    m_tracksDataChanged = master->playback()->tracksDataChanged();
    m_tracksDataChanged.onReceive(this, [this](const InstrumentTrackIdSet& changedTrackIdSet) {
        for (const InstrumentTrackId& trackId : changedTrackIdSet) {
            if (muse::contains(m_onlineTrackIdSet, trackId)) {
                setManualProcessingAllowed(true);
                return;
            }
        }
    });
}

void OnlineSoundsStatusModel::setManualProcessingAllowed(bool allowed)
{
    if (m_manualProcessingAllowed == allowed) {
        return;
    }

    m_manualProcessingAllowed = allowed;
    emit manualProcessingAllowedChanged();

    if (allowed && m_shouldNotifyToursThatManualProcessingAllowed) {
        muse::async::Async::call(this, [=]() {
            tours()->onEvent(u"online_sounds_manual_processing_allowed");
        });

        m_shouldNotifyToursThatManualProcessingAllowed = false;
    }
}

void OnlineSoundsStatusModel::setStatus(Status status)
{
    if (m_status == status) {
        return;
    }

    m_status = status;
    emit statusChanged();
}
