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

#include "actions/iactionsdispatcher.h"
#include "global/iinteractive.h"
#include "modularity/ioc.h"
#include "project/iimportfiletoscorescenario.h"

namespace mu::project {
class ImportFileToScoreDevToolsModel : public QObject, public muse::async::Asyncable, public muse::Contextable
{
    Q_OBJECT
    QML_ELEMENT

    muse::ContextInject<IImportFileToScoreScenario> importFileToScoreScenario = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<muse::actions::IActionsDispatcher> dispatcher = { this };

public:
    explicit ImportFileToScoreDevToolsModel(QObject* parent = nullptr);

    Q_INVOKABLE void init();
    Q_INVOKABLE void selectAndImportFiles();
};
}
