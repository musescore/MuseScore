/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "projectsession.h"

#include "global/io/fileinfo.h"

#include "notation/imasternotation.h"
#include "notation/inotation.h"
#include "project/inotationproject.h"

using namespace mu::project;
using namespace mu::notation;
using namespace muse::async;

ProjectSession::ProjectSession()
{
}

void ProjectSession::setProject(const INotationProjectPtr& project)
{
    if (m_currentProject == project) {
        return;
    }

    //! NOTE Hold the previous project (and its notation) until all the receivers
    //! have disconnected from it in the notifications below
    INotationProjectPtr prevProject = m_currentProject;
    INotationPtr prevNotation = m_currentNotation;

    if (m_currentProject) {
        fsRestriction()->removeAllowedPathBase("project");
    }

    m_currentProject = project;

    if (m_currentProject) {
        fsRestriction()->addAllowedPathBase("project", muse::io::dirpath(m_currentProject->path()));

        m_currentProject->pathChanged().onNotify(this, [this]() {
            fsRestriction()->addAllowedPathBase("project", muse::io::dirpath(m_currentProject->path()));
        });
    } else {
        fsRestriction()->removeAllowedPathBase("project");
    }

    INotationPtr notation = project ? project->masterNotation()->notation() : nullptr;
    doSetCurrentNotation(notation);

    m_currentProjectChanged.notify();
    m_currentNotationChanged.notify();
}

INotationProjectPtr ProjectSession::project() const
{
    return m_currentProject;
}

Notification ProjectSession::projectChanged() const
{
    return m_currentProjectChanged;
}

IMasterNotationPtr ProjectSession::masterNotation() const
{
    return m_currentProject ? m_currentProject->masterNotation() : nullptr;
}

Notification ProjectSession::masterNotationChanged() const
{
    //! NOTE Same as project
    return m_currentProjectChanged;
}

void ProjectSession::setCurrentNotation(const INotationPtr& notation)
{
    if (m_currentNotation == notation) {
        return;
    }

    //! NOTE Hold the previous notation until all the receivers
    //! have disconnected from it in the notification below
    INotationPtr prevNotation = m_currentNotation;

    doSetCurrentNotation(notation);
    m_currentNotationChanged.notify();
}

INotationPtr ProjectSession::currentNotation() const
{
    return m_currentNotation;
}

Notification ProjectSession::currentNotationChanged() const
{
    return m_currentNotationChanged;
}

void ProjectSession::doSetCurrentNotation(const INotationPtr& notation)
{
    if (m_currentNotation == notation) {
        return;
    }

    m_currentNotation = notation;

    if (m_currentNotation) {
        m_currentNotation->setIsOpen(true);
    }
}
