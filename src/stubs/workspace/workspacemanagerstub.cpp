//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "workspacemanagerstub.h"

using namespace mu::workspace;
using namespace mu;

RetValCh<IWorkspacePtr> WorkspaceManagerStub::currentWorkspace() const
{
    RetValCh<IWorkspacePtr> result;
    result.ret = make_ret(Ret::Code::NotSupported);
    return result;
}

RetVal<IWorkspacePtrList> WorkspaceManagerStub::workspaces() const
{
    RetVal<IWorkspacePtrList> result;
    result.ret = make_ret(Ret::Code::NotSupported);
    return result;
}

Ret WorkspaceManagerStub::setWorkspaces(const IWorkspacePtrList&)
{
    return make_ret(Ret::Code::NotSupported);
}
