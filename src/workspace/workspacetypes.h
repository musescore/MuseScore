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
#ifndef MU_WORKSPACE_WORKSPACETYPES_H
#define MU_WORKSPACE_WORKSPACETYPES_H

#include <vector>
#include <map>

#include "val.h"

namespace mu::workspace {
enum class WorkspaceTag
{
    UiArrangement,
    Settings,
    Palettes,
    Toolbar
};

struct AbstractData
{
    virtual ~AbstractData() = default;

    WorkspaceTag tag;
    std::string name;
};

using AbstractDataPtr = std::shared_ptr<AbstractData>;
using AbstractDataPtrList = std::vector<AbstractDataPtr>;

//! NOTE Only data associations with framework.
//! Other data must be in the appropriate modules.

struct SettingsData : public AbstractData
{
    std::map<std::string /*key*/, Val> values;
};

using SettingsDataPtr = std::shared_ptr<SettingsData>;

struct ToolbarData : public AbstractData
{
    std::vector<std::string /*action*/> actions;
};

using ToolbarDataPtr = std::shared_ptr<ToolbarData>;

static constexpr std::string_view DEFAULT_WORKSPACE_NAME("Default");
}

#endif // MU_WORKSPACE_WORKSPACETYPES_H
