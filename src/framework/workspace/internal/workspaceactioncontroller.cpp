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
#include "workspaceactioncontroller.h"

#include "types/val.h"

#include "log.h"

using namespace muse::workspace;
using namespace muse::actions;

void WorkspaceActionController::init()
{
    dispatcher()->reg(this, "select-workspace", this, &WorkspaceActionController::selectWorkspace);
    dispatcher()->reg(this, "configure-workspaces", this, &WorkspaceActionController::openConfigureWorkspacesDialog);
    dispatcher()->reg(this, "create-workspace", this, &WorkspaceActionController::createNewWorkspace);
}

void WorkspaceActionController::selectWorkspace(const ActionData& args)
{
    std::string selectedWorkspace = !args.empty() ? args.arg<std::string>(0) : "";
    manager()->changeCurrentWorkspace(selectedWorkspace);
}

void WorkspaceActionController::openConfigureWorkspacesDialog()
{
    manager()->openConfigureWorkspacesDialog();
}

void muse::workspace::WorkspaceActionController::createNewWorkspace()
{
    manager()->createAndAppendNewWorkspace();
}
