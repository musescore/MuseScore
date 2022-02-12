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

#include "sessionsmanager.h"

#include "project/projecttypes.h"

using namespace mu::appshell;
using namespace mu::project;

void SessionsManager::init()
{
    update();

    globalContext()->currentProjectChanged().onNotify(this, [this]() {
        update();

        if (auto project = globalContext()->currentProject()) {
            project->saveLocationChanged().onNotify(this, [this]() {
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
    io::paths projects = configuration()->sessionProjectsPaths();
    if (projects.empty()) {
        return;
    }

    for (const io::path& path : projects) {
        auto saveLocation = SaveLocation::makeLocal(path);
        dispatcher()->dispatch("file-open", actions::ActionData::make_arg1<SaveLocation>(saveLocation));
    }
}

void SessionsManager::reset()
{
    configuration()->setSessionProjectsPaths({});
}

void SessionsManager::update()
{
    if (auto project = globalContext()->currentProject()) {
        SaveLocation newProjectSaveLocation = project->saveLocation();

        if (m_lastOpenedProjectLocation.has_value()) {
            if (m_lastOpenedProjectLocation.value() != newProjectSaveLocation) {
                removeProjectFromSession(m_lastOpenedProjectLocation.value());
                addProjectToSession(newProjectSaveLocation);
                m_lastOpenedProjectLocation = newProjectSaveLocation;
            }
        } else {
            addProjectToSession(newProjectSaveLocation);
            m_lastOpenedProjectLocation = newProjectSaveLocation;
        }
    } else {
        if (m_lastOpenedProjectLocation.has_value()) {
            removeProjectFromSession(m_lastOpenedProjectLocation.value());
            m_lastOpenedProjectLocation = std::nullopt;
        }
    }
}

void SessionsManager::removeProjectFromSession(const SaveLocation& location)
{
    if (!location.isLocal()) {
        NOT_SUPPORTED << "currently only locally saved files can be stored in the session.";
        return;
    }

    io::paths projects = configuration()->sessionProjectsPaths();
    if (projects.empty()) {
        return;
    }

    io::path projectPath = location.localInfo().path;
    projects.erase(std::remove(projects.begin(), projects.end(), projectPath), projects.end());
    configuration()->setSessionProjectsPaths(projects);
}

void SessionsManager::addProjectToSession(const SaveLocation& location)
{
    if (!location.isLocal()) {
        NOT_SUPPORTED << "currently only locally saved files can be stored in the session.";
        return;
    }

    io::paths projects = configuration()->sessionProjectsPaths();
    io::path projectPath = location.localInfo().path;

    if (std::find(projects.begin(), projects.end(), projectPath) != projects.end()) {
        return;
    }

    projects.push_back(projectPath);
    configuration()->setSessionProjectsPaths(projects);
}
