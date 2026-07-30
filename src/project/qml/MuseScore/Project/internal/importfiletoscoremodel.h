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
#include <qqmlintegration.h>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "iinteractive.h"

#include "project/iprojectconfiguration.h"
#include "project/iimportfiletoscorescenario.h"

namespace mu::project {
class ImportFileToScoreModel : public QObject, public muse::async::Asyncable, public muse::Contextable
{
    Q_OBJECT

    Q_PROPERTY(QString guidelinesLinkText READ guidelinesLinkText CONSTANT)

    QML_ELEMENT

    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<IImportFileToScoreScenario> importFileToScoreScenario = { this };
    muse::GlobalInject<IProjectConfiguration> configuration;

public:
    explicit ImportFileToScoreModel(QObject* parent = nullptr);

    QString guidelinesLinkText() const;

    Q_INVOKABLE QStringList selectFiles();
    Q_INVOKABLE void validateFiles(const QStringList& pathsOrUrls);

signals:
    void validationFinished(int type, QVariantList paths);
};
}
