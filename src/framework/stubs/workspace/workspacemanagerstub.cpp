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
#include "workspacemanagerstub.h"
#include "workspacestub.h"

using namespace muse::workspace;
using namespace muse;

IWorkspacePtr WorkspaceManagerStub::defaultWorkspace() const
{
    static IWorkspacePtr p = std::make_shared<WorkspaceStub>();
    return p;
}

IWorkspacePtr WorkspaceManagerStub::currentWorkspace() const
{
    static IWorkspacePtr p = std::make_shared<WorkspaceStub>();
    return p;
}

void WorkspaceManagerStub::prepareCurrentWorkspaceForChange()
{
}

async::Notification WorkspaceManagerStub::currentWorkspaceAboutToBeChanged() const
{
    return async::Notification();
}

async::Notification WorkspaceManagerStub::currentWorkspaceChanged() const
{
    return async::Notification();
}

IWorkspacePtrList WorkspaceManagerStub::workspaces() const
{
    return { defaultWorkspace() };
}

Ret WorkspaceManagerStub::setWorkspaces(const IWorkspacePtrList&)
{
    return make_ret(Ret::Code::NotImplemented);
}

async::Notification WorkspaceManagerStub::workspacesListChanged() const
{
    return async::Notification();
}

IWorkspacePtr WorkspaceManagerStub::cloneWorkspace(const IWorkspacePtr&, const std::string&)const
{
    return std::make_shared<WorkspaceStub>();
}
