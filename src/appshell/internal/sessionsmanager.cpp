/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "sessionsmanager.h"

using namespace mu::appshell;
using namespace muse;
using namespace muse::actions;

void SessionsManager::init()
{
    update();

    globalContext()->currentProjectChanged().onNotify(this, [this]() {
        update();

        if (auto project = globalContext()->currentProject()) {
            project->pathChanged().onNotify(this, [this]() {
                update();
            });
        }
    });
}

void SessionsManager::deinit()
{
    bool isServer = multiInstancesProvider()->isMainInstance();
    if (!isServer) {
        return;
    }

    if (configuration()->startupModeType() != StartupModeType::ContinueLastSession) {
        reset();
    }
}

bool SessionsManager::hasProjectsForRestore()
{
    return !configuration()->sessionProjectsPaths().empty();
}

void SessionsManager::restore()
{
    io::paths_t projects = configuration()->sessionProjectsPaths();
    if (projects.empty()) {
        return;
    }

    for (const muse::io::path_t& path : projects) {
        dispatcher()->dispatch("file-open", ActionData::make_arg1<QUrl>(path.toQUrl()));
    }
}

void SessionsManager::reset()
{
    configuration()->setSessionProjectsPaths({});
}

void SessionsManager::update()
{
    muse::io::path_t newProjectPath;

    if (auto project = globalContext()->currentProject()) {
        newProjectPath = project->isNewlyCreated() ? projectConfiguration()->newProjectTemporaryPath() : project->path();
    }

    if (newProjectPath == m_lastOpenedProjectPath) {
        return;
    }

    if (!m_lastOpenedProjectPath.empty()) {
        removeProjectFromSession(m_lastOpenedProjectPath);
    }

    if (!newProjectPath.empty()) {
        addProjectToSession(newProjectPath);
    }

    m_lastOpenedProjectPath = newProjectPath;
}

void SessionsManager::removeProjectFromSession(const muse::io::path_t& projectPath)
{
    io::paths_t projects = configuration()->sessionProjectsPaths();
    if (projects.empty()) {
        return;
    }

    projects.erase(std::remove(projects.begin(), projects.end(), projectPath), projects.end());
    configuration()->setSessionProjectsPaths(projects);
}

void SessionsManager::addProjectToSession(const muse::io::path_t& projectPath)
{
    io::paths_t projects = configuration()->sessionProjectsPaths();

    if (std::find(projects.begin(), projects.end(), projectPath) == projects.end()) {
        projects.push_back(projectPath);
    }

    configuration()->setSessionProjectsPaths(projects);
}
