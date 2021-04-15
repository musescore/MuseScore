/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
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
