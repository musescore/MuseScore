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

#include "modularity/ioc.h"
#include "internal/iexportprojectscenario.h"

namespace mu::project {
class ExportProgressModel : public QObject
{
    Q_OBJECT

    INJECT(project, IExportProjectScenario, exportScenario)

    Q_PROPERTY(qreal progress READ progress NOTIFY progressChanged)

public:
    explicit ExportProgressModel(QObject* parent = nullptr);

    qreal progress() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void cancel();

signals:
    void progressChanged();

private:
    void setProgress(qreal progress);

    qreal m_progress = 0.0;
};
}

#endif // MU_PROJECT_EXPORTPROGRESSMODEL_H
