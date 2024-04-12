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
#include "workspacescenestubmodule.h"

#include "modularity/ioc.h"

#include "framework/ui/iinteractiveuriregister.h"

using namespace mu::workspacescene;
using namespace muse;
using namespace muse::modularity;
using namespace muse::ui;

static void workspacescene_init_qrc()
{
    Q_INIT_RESOURCE(workspacescene);
}

std::string WorkspaceSceneModule::moduleName() const
{
    return "workspacescene_stub";
}

void WorkspaceSceneModule::resolveImports()
{
    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("muse://workspace/select"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/Workspace/WorkspacesDialog.qml"));

        ir->registerUri(Uri("muse://workspace/create"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/Workspace/NewWorkspaceDialog.qml"));
    }
}

void WorkspaceSceneModule::registerResources()
{
    workspacescene_init_qrc();
}
