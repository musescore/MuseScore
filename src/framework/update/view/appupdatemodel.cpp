/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

#include "appupdatemodel.h"

#include "translation.h"
#include "log.h"

using namespace muse::update;

AppUpdateModel::AppUpdateModel(QObject* parent)
    : QObject(parent), Injectable(muse::iocCtxForQmlObject(this))
{
    setProgressTitle(muse::qtrc("update", "Updating MuseScore Studio"));
}

AppUpdateModel::~AppUpdateModel()
{
    if (m_progress.isStarted()) {
        m_progress.cancel();
    }
}

void AppUpdateModel::load(const QString& mode)
{
    if (mode != "download") {
        return;
    }

    RetVal<Progress> progress = service()->downloadRelease();
    if (!progress.ret) {
        LOGE() << progress.ret.toString();
        emit finished(progress.ret.code(), QString());
        return;
    }

    m_progress = progress.val;
    emit started();

    const RetVal<ReleaseInfo>& info = service()->lastCheckResult();

    //: Means that the download is currently in progress.
    //: %1 will be replaced by the version number of the version that is being downloaded.
    setProgressTitle(muse::qtrc("update", "Downloading MuseScore Studio %1")
                     .arg(QString::fromStdString(info.val.version)));

    m_progress.progressChanged().onReceive(this, [this](int64_t current, int64_t total, const std::string&) {
        setCurrentProgress(current);
        setTotalProgress(total);
    });

    m_progress.finished().onReceive(this, [this](const ProgressResult& res) {
        if (!res.ret) {
            LOGE() << res.ret.toString();
        }

        emit finished(res.ret.code(), res.val.toQString());
    });
}

int AppUpdateModel::currentProgress() const
{
    return m_currentProgress;
}

int AppUpdateModel::totalProgress() const
{
    return m_totalProgress;
}

QString AppUpdateModel::progressTitle() const
{
    return m_progressTitle;
}

void AppUpdateModel::setCurrentProgress(int progress)
{
    if (m_currentProgress == progress) {
        return;
    }

    m_currentProgress = progress;
    emit currentProgressChanged();
}

void AppUpdateModel::setTotalProgress(int progress)
{
    if (m_totalProgress == progress) {
        return;
    }

    m_totalProgress = progress;
    emit totalProgressChanged();
}

void AppUpdateModel::setProgressTitle(const QString& title)
{
    if (m_progressTitle == title) {
        return;
    }

    m_progressTitle = title;
    emit progressTitleChanged();
}
