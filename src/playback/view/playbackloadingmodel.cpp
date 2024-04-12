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
#include "playbackloadingmodel.h"

#include "log.h"

using namespace mu::playback;
using namespace muse;

PlaybackLoadingModel::PlaybackLoadingModel(QObject* parent)
    : QObject(parent)
{
}

void PlaybackLoadingModel::load()
{
    Progress progress = playbackController()->loadingProgress();

    progress.started.onNotify(this, [this]() {
        emit started();
    });

    progress.progressChanged.onReceive(this, [this](int64_t current, int64_t total, const std::string& title) {
        setCurrentProgress(current);
        setTotalProgress(total);
        setProgressTitle(QString::fromStdString(title));
    });

    progress.finished.onReceive(this, [this](const ProgressResult& res) {
        const Ret& ret = res.ret;

        if (!ret && !ret.text().empty()) {
            LOGE() << ret.toString();
        }

        emit finished();
    });
}

int PlaybackLoadingModel::currentProgress() const
{
    return m_currentProgress;
}

int PlaybackLoadingModel::totalProgress() const
{
    return m_totalProgress;
}

QString PlaybackLoadingModel::progressTitle() const
{
    return m_progressTitle;
}

void PlaybackLoadingModel::setCurrentProgress(int progress)
{
    if (m_currentProgress == progress) {
        return;
    }

    m_currentProgress = progress;
    emit currentProgressChanged();
}

void PlaybackLoadingModel::setTotalProgress(int progress)
{
    if (m_totalProgress == progress) {
        return;
    }

    m_totalProgress = progress;
    emit totalProgressChanged();
}

void PlaybackLoadingModel::setProgressTitle(const QString& title)
{
    if (m_progressTitle == title) {
        return;
    }

    m_progressTitle = title;
    emit progressTitleChanged();
}
