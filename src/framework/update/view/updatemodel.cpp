/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "updatemodel.h"

#include "async/async.h"
#include "io/path.h"

#include "translation.h"
#include "log.h"

using namespace muse::update;

UpdateModel::UpdateModel(QObject* parent)
    : QObject(parent), Injectable(muse::iocCtxForQmlObject(this))
{
    setProgressTitle(muse::qtrc("update", "Updating MuseScore Studio"));
}

UpdateModel::~UpdateModel()
{
    service()->cancelUpdate();
}

void UpdateModel::load(const QString& mode)
{
    Progress progress = service()->updateProgress();

    progress.started().onNotify(this, [this]() {
        emit started();
    });

    progress.progressChanged().onReceive(this, [this](int64_t current, int64_t total, const std::string& title) {
        setCurrentProgress(current);
        setTotalProgress(total);
        setProgressTitle(QString::fromStdString(title));
    });

    progress.finished().onReceive(this, [](const ProgressResult& res) {
        const Ret& ret = res.ret;

        if (!ret && !ret.text().empty()) {
            LOGE() << ret.toString();
        }
    });

    async::Async::call(this, [this, mode]() {
        if (mode == "download") {
            RetVal<muse::io::path_t> downloadRetVal = service()->downloadRelease();
            emit finished(downloadRetVal.ret.code(), downloadRetVal.val.toQString());
        }
    });
}

int UpdateModel::currentProgress() const
{
    return m_currentProgress;
}

int UpdateModel::totalProgress() const
{
    return m_totalProgress;
}

QString UpdateModel::progressTitle() const
{
    return m_progressTitle;
}

void UpdateModel::setCurrentProgress(int progress)
{
    if (m_currentProgress == progress) {
        return;
    }

    m_currentProgress = progress;
    emit currentProgressChanged();
}

void UpdateModel::setTotalProgress(int progress)
{
    if (m_totalProgress == progress) {
        return;
    }

    m_totalProgress = progress;
    emit totalProgressChanged();
}

void UpdateModel::setProgressTitle(const QString& title)
{
    if (m_progressTitle == title) {
        return;
    }

    m_progressTitle = title;
    emit progressTitleChanged();
}
