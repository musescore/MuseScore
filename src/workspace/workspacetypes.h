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
#ifndef MU_WORKSPACE_WORKSPACETYPES_H
#define MU_WORKSPACE_WORKSPACETYPES_H

#include <vector>
#include <map>

#include <QJsonValue>

#include "val.h"

namespace mu::workspace {
static constexpr std::string_view DEFAULT_WORKSPACE_NAME("Default");

struct Data
{
    QJsonValue data;
};

enum class WorkspaceTag
{
    Unknown,
    UiArrangement,
    Settings,
    Palettes,
    Toolbar
};
using WorkspaceTagList = std::vector<WorkspaceTag>;

inline bool containsTag(const WorkspaceTagList& list, const WorkspaceTag& tag)
{
    return std::find(list.cbegin(), list.cend(), tag) != list.cend();
}

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
}

#endif // MU_WORKSPACE_WORKSPACETYPES_H
