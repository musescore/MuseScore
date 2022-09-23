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

#include "exportprogressmodel.h"

#include <QTimer>

#include "log.h"

using namespace mu::project;
using namespace mu::framework;

ExportProgressModel::ExportProgressModel(QObject* parent)
    : QObject(parent)
{
}

int ExportProgressModel::progress() const
{
    return m_progress;
}

int ExportProgressModel::totalProgress() const
{
    return m_totalProgress;
}

void ExportProgressModel::setProgress(int progress)
{
    if (m_progress == progress) {
        return;
    }

    m_progress = progress;
    emit progressChanged();
}

void ExportProgressModel::setTotalProgress(int progress)
{
    if (m_totalProgress == progress) {
        return;
    }

    m_totalProgress = progress;
    emit totalProgressChanged();
}

void ExportProgressModel::load()
{
    Progress progress = exportScenario()->progress();

    progress.started.onNotify(this, [this]() {
        setProgress(0);
    });

    progress.progressChanged.onReceive(this, [this](int64_t current, int64_t total, const std::string&) {
        setProgress(current);
        setTotalProgress(total);
    });

    progress.finished.onReceive(this, [this](const ProgressResult& res) {
        const Ret& ret = res.ret;

        if (!ret && !ret.text().empty()) {
            LOGE() << ret.toString();
        }

        QTimer::singleShot(100, this, [this]() {
            emit exportFinished();
        });
    });
}

void ExportProgressModel::cancel()
{
    exportScenario()->abort();
}
