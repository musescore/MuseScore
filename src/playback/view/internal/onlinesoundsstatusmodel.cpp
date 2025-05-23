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

using namespace mu::playback;

OnlineSoundsStatusModel::OnlineSoundsStatusModel(QObject* parent)
    : QObject(parent)
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
}

bool OnlineSoundsStatusModel::hasOnlineSounds() const
{
    return m_hasOnlineSounds;
}

int OnlineSoundsStatusModel::status() const
{
    return static_cast<int>(m_status);
}

void OnlineSoundsStatusModel::setHasOnlineSounds(bool value)
{
    if (m_hasOnlineSounds == value) {
        return;
    }

    m_hasOnlineSounds = value;
    emit hasOnlineSoundsChanged();
}

void OnlineSoundsStatusModel::setStatus(Status status)
{
    if (m_status == status) {
        return;
    }

    m_status = status;
    emit statusChanged();
}
