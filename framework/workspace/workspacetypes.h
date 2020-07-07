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

#include <string>
#include <vector>
#include <map>
#include "val.h"

namespace mu {
namespace workspace {
struct AbstractData
{
    virtual ~AbstractData() = default;
    std::string tag;
    std::string name;
};

//! NOTE Only data associations with framework.
//! Other data must be in the appropriate modules.

struct SettingsData : public AbstractData
{
    std::map<std::string /*key*/, Val> vals;
};

struct ToolbarData : public AbstractData
{
    std::vector<std::string /*action*/> actions;
};
}
}

#endif // MU_WORKSPACE_WORKSPACETYPES_H
