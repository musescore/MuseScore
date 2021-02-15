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
#include "workspacestub.h"

using namespace mu::workspace;

std::string WorkspaceStub::name() const
{
    return std::string();
}

std::string WorkspaceStub::title() const
{
    return std::string();
}

AbstractDataPtr WorkspaceStub::data(WorkspaceTag, const std::string&) const
{
    return std::make_shared<AbstractData>();
}

AbstractDataPtrList WorkspaceStub::dataList(WorkspaceTag) const
{
    return {};
}

void WorkspaceStub::addData(AbstractDataPtr)
{
}

mu::async::Channel<AbstractDataPtr> WorkspaceStub::dataChanged() const
{
    return mu::async::Channel<AbstractDataPtr>();
}

mu::Val WorkspaceStub::settingValue(const std::string&) const
{
    return mu::Val();
}

std::vector<std::string> WorkspaceStub::toolbarActions(const std::string&) const
{
    return {};
}
