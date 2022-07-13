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

void ExportProgressModel::setProgress(int progress)
{
    if (m_progress == progress) {
        return;
    }

    m_progress = progress;
    emit progressChanged();
}

void ExportProgressModel::load()
{
    Progress progress = exportScenario()->progress();

    progress.started.onNotify(this, [this]() {
        setProgress(0);
    });

    progress.progressChanged.onReceive(this, [this](int64_t current, int64_t) {
        setProgress(current);
    });

    progress.finished.onReceive(this, [this](const Ret& ret) {
        if (!ret && !ret.text().empty()) {
            LOGE() << ret.toString();
        }

        emit exportFinished();
    });
}

void ExportProgressModel::cancel()
{
    exportScenario()->abort();
}
