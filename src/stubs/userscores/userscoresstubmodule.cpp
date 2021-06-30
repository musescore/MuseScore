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
#include "userscoresstubmodule.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "filescorecontrollerstub.h"
#include "userscoresconfigurationstub.h"
#include "ui/iinteractiveuriregister.h"

using namespace mu::userscores;
using namespace mu::modularity;
using namespace mu::ui;

static void userscores_init_qrc()
{
    Q_INIT_RESOURCE(userscores);
}

std::string UserScoresStubModule::moduleName() const
{
    return "userscores_stub";
}

void UserScoresStubModule::registerExports()
{
    ioc()->registerExport<IFileScoreController>(moduleName(), new FileScoreControllerStub());
    ioc()->registerExport<IUserScoresConfiguration>(moduleName(), new UserScoresConfigurationStub());
}

void UserScoresStubModule::resolveImports()
{
    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://userscores/newscore"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/UserScores/NewScoreDialog.qml"));
    }
}

void UserScoresStubModule::registerResources()
{
    userscores_init_qrc();
}

void UserScoresStubModule::registerUiTypes()
{
    ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(userscores_QML_IMPORT);
}
