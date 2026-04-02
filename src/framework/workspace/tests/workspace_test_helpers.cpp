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

#include "workspace_test_helpers.h"

#include "global/configreader.h"

using namespace muse;
using namespace muse::workspace;

WorkspaceTestConfig& WorkspaceTestConfig::instance()
{
    static WorkspaceTestConfig s;
    return s;
}

void WorkspaceTestConfig::load(const io::path_t& configPath, const std::string& builtinWorkspacesDir)
{
    m_builtinWorkspacesDir = builtinWorkspacesDir;
    m_config = ConfigReader::read(configPath);

    m_defaultWorkspaceName = m_config.value("default_workspace_name").toString();

    ValList files = m_config.value("builtin_workspace_files").toList();
    m_builtinFiles.clear();
    for (const Val& v : files) {
        m_builtinFiles.push_back(v.toString());
    }
}

const Config& WorkspaceTestConfig::config() const
{
    return m_config;
}

const std::string& WorkspaceTestConfig::defaultWorkspaceName() const
{
    return m_defaultWorkspaceName;
}

const std::vector<std::string>& WorkspaceTestConfig::builtinFiles() const
{
    return m_builtinFiles;
}

const std::string& WorkspaceTestConfig::builtinWorkspacesDir() const
{
    return m_builtinWorkspacesDir;
}
