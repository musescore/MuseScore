/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#pragma once

#include <string>
#include <vector>

#include "global/types/config.h"
#include "io/path.h"

namespace muse::workspace {
class WorkspaceTestConfig
{
public:
    static WorkspaceTestConfig& instance();

    WorkspaceTestConfig(const WorkspaceTestConfig&) = delete;
    WorkspaceTestConfig& operator=(const WorkspaceTestConfig&) = delete;
    WorkspaceTestConfig(WorkspaceTestConfig&&) = delete;
    WorkspaceTestConfig& operator=(WorkspaceTestConfig&&) = delete;

    void load(const io::path_t& configPath, const std::string& builtinWorkspacesDir);

    const Config& config() const;
    const std::string& defaultWorkspaceName() const;
    const std::vector<std::string>& builtinFiles() const;
    const std::string& builtinWorkspacesDir() const;

private:
    WorkspaceTestConfig() = default;

    Config m_config;
    std::string m_defaultWorkspaceName;
    std::vector<std::string> m_builtinFiles;
    std::string m_builtinWorkspacesDir;
};
}
