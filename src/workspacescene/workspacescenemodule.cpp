/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "workspacescenemodule.h"

#include <QQmlEngine>

#include "framework/ui/iinteractiveuriregister.h"

#include "view/workspacelistmodel.h"
#include "view/newworkspacemodel.h"

using namespace mu::workspacescene;
using namespace muse;
using namespace muse::modularity;

static void workspacescene_init_qrc()
{
    Q_INIT_RESOURCE(workspacescene);
}

std::string WorkspaceSceneModule::moduleName() const
{
    return "workspacescene";
}

void WorkspaceSceneModule::resolveImports()
{
    auto ir = ioc()->resolve<muse::ui::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("muse://workspace/select"), "MuseScore/Workspace/WorkspacesDialog.qml");
        ir->registerQmlUri(Uri("muse://workspace/create"), "MuseScore/Workspace/NewWorkspaceDialog.qml");
    }
}

void WorkspaceSceneModule::registerResources()
{
    workspacescene_init_qrc();
}

void WorkspaceSceneModule::registerUiTypes()
{
    qmlRegisterType<WorkspaceListModel>("MuseScore.Workspace", 1, 0, "WorkspaceListModel");
    qmlRegisterType<NewWorkspaceModel>("MuseScore.Workspace", 1, 0, "NewWorkspaceModel");
}
