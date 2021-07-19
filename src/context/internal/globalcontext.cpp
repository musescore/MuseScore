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
#include "globalcontext.h"

#include "notation/imasternotation.h"

using namespace mu::context;
using namespace mu::project;
using namespace mu::notation;
using namespace mu::async;

void GlobalContext::addNotationProject(const INotationProjectPtr& project)
{
    m_projects.push_back(project);
}

void GlobalContext::removeNotationProject(const INotationProjectPtr& project)
{
    m_projects.erase(std::remove(m_projects.begin(), m_projects.end(), project), m_projects.end());
}

const std::vector<INotationProjectPtr>& GlobalContext::notationProjects() const
{
    return m_projects;
}

bool GlobalContext::containsNotationProject(const io::path& path) const
{
    for (const auto& p : m_projects) {
        if (p->path() == path) {
            return true;
        }
    }
    return false;
}

void GlobalContext::setCurrentNotationProject(const INotationProjectPtr& project)
{
    if (m_currentNotationProject == project) {
        return;
    }

    m_currentNotationProject = project;

    INotationPtr notation = project ? project->masterNotation()->notation() : nullptr;
    doSetCurrentNotation(notation);

    m_currentNotationProjectChanged.notify();
    m_currentNotationChanged.notify();
}

INotationProjectPtr GlobalContext::currentNotationProject() const
{
    return m_currentNotationProject;
}

Notification GlobalContext::currentNotationProjectChanged() const
{
    return m_currentNotationProjectChanged;
}

IMasterNotationPtr GlobalContext::currentMasterNotation() const
{
    return m_currentNotationProject ? m_currentNotationProject->masterNotation() : nullptr;
}

Notification GlobalContext::currentMasterNotationChanged() const
{
    //! NOTE Same as project
    return m_currentNotationProjectChanged;
}

void GlobalContext::setCurrentNotation(const INotationPtr& notation)
{
    doSetCurrentNotation(notation);
    m_currentNotationChanged.notify();
}

INotationPtr GlobalContext::currentNotation() const
{
    return m_currentNotation;
}

Notification GlobalContext::currentNotationChanged() const
{
    return m_currentNotationChanged;
}

void GlobalContext::doSetCurrentNotation(const INotationPtr& notation)
{
    if (m_currentNotation == notation) {
        return;
    }

    m_currentNotation = notation;

    if (m_currentNotation) {
        m_currentNotation->setOpened(true);
    }
}
