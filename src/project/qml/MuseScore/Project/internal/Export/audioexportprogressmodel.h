/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#pragma once

#include <QObject>
#include <QVariant>
#include <qqmlintegration.h>

#include "modularity/ioc.h"

#include "iexportprojectscenario.h"

namespace mu::project {
//! NOTE Data of the audio export progress dialog: the overall progress of the export in
//! progress, and the progress of each of its files, which are written at the same time
class AudioExportProgressModel : public QObject, public muse::Contextable
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QVariant overallProgress READ overallProgress NOTIFY loaded)
    Q_PROPERTY(QVariantList files READ files NOTIFY loaded)

public:
    explicit AudioExportProgressModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();

    QVariant overallProgress() const;
    QVariantList files() const;

signals:
    void loaded();

private:
    muse::ContextInject<IExportProjectScenario> exportProjectScenario = { this };

    QVariant m_overallProgress;
    QVariantList m_files;
};
}
