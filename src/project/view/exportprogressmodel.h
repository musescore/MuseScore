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
#ifndef MU_PROJECT_EXPORTPROGRESSMODEL_H
#define MU_PROJECT_EXPORTPROGRESSMODEL_H

#include <QObject>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "internal/iexportprojectscenario.h"

namespace mu::project {
class ExportProgressModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(project, IExportProjectScenario, exportScenario)

    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(int totalProgress READ totalProgress NOTIFY totalProgressChanged)

public:
    explicit ExportProgressModel(QObject* parent = nullptr);

    int progress() const;
    int totalProgress() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void cancel();

signals:
    void progressChanged();
    void totalProgressChanged();
    void exportFinished();

private:
    void setProgress(int progress);
    void setTotalProgress(int progress);

    int m_progress = 0;
    int m_totalProgress = INT_MAX;
};
}

#endif // MU_PROJECT_EXPORTPROGRESSMODEL_H
