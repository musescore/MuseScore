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

#include "translation.h"

using namespace mu::playback;

OnlineSoundsStatusModel::OnlineSoundsStatusModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void OnlineSoundsStatusModel::load()
{
    setHasOnlineSounds(!playbackController()->onlineSounds().empty());
    playbackController()->onlineSoundsChanged().onNotify(this, [this]() {
        setHasOnlineSounds(!playbackController()->onlineSounds().empty());
    });

    muse::Progress progress = playbackController()->onlineSoundsProcessingProgress();
    setStatus(progress.isStarted() ? Status::Processing : Status::Success);

    progress.started().onNotify(this, [this]() {
        setStatus(Status::Processing);
    });

    progress.finished().onReceive(this, [this](const muse::ProgressResult& result) {
        setStatus(result.ret.success() ? Status::Success : Status::Error);
    });

    setCanProcessOnlineSounds(!audioConfiguration()->autoProcessOnlineSoundsInBackground());
    audioConfiguration()->autoProcessOnlineSoundsInBackgroundChanged().onReceive(this, [this](bool value) {
        setCanProcessOnlineSounds(!value);
    });
}

void OnlineSoundsStatusModel::processOnlineSounds()
{
    if (m_canProcessOnlineSounds) {
        dispatcher()->dispatch("process-online-sounds");
    }
}

bool OnlineSoundsStatusModel::hasOnlineSounds() const
{
    return m_hasOnlineSounds;
}

bool OnlineSoundsStatusModel::canProcessOnlineSounds() const
{
    return m_canProcessOnlineSounds;
}

int OnlineSoundsStatusModel::status() const
{
    return static_cast<int>(m_status);
}

QString OnlineSoundsStatusModel::errorTitle() const
{
    if (m_status != Status::Error) {
        return QString();
    }

    return muse::qtrc("playback", "Some online sounds aren’t ready yet");
}

QString OnlineSoundsStatusModel::errorDescription() const
{
    if (m_status != Status::Error) {
        return QString();
    }

    return muse::qtrc("global", "Please check your internet connection or try again later.");
}

void OnlineSoundsStatusModel::setHasOnlineSounds(bool value)
{
    if (m_hasOnlineSounds == value) {
        return;
    }

    m_hasOnlineSounds = value;
    emit hasOnlineSoundsChanged();

    if (!m_hasOnlineSounds) {
        setStatus(Status::Success);
    }
}

void OnlineSoundsStatusModel::setCanProcessOnlineSounds(bool canRun)
{
    if (m_canProcessOnlineSounds == canRun) {
        return;
    }

    m_canProcessOnlineSounds = canRun;
    emit canProcessOnlineSoundsChanged();
}

void OnlineSoundsStatusModel::setStatus(Status status)
{
    if (m_status == status) {
        return;
    }

    m_status = status;
    emit statusChanged();
}
