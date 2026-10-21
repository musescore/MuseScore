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
#pragma once

#include "../iprojectsession.h"

#include "global/async/asyncable.h"

#include "global/modularity/ioc.h"
#include "global/api/ifilesystemapirestriction.h"

namespace mu::project {
class ProjectSession : public IProjectSession, public muse::async::Asyncable
{
    muse::GlobalInject<muse::api::IFileSystemApiRestriction> fsRestriction;
public:

    ProjectSession();

    void setProject(const project::INotationProjectPtr& project) override;
    project::INotationProjectPtr project() const override;
    muse::async::Notification projectChanged() const override;

    notation::IMasterNotationPtr masterNotation() const override;
    muse::async::Notification masterNotationChanged() const override;

    void setCurrentNotation(const notation::INotationPtr& notation) override;
    notation::INotationPtr currentNotation() const override;
    muse::async::Notification currentNotationChanged() const override;

private:
    void doSetCurrentNotation(const notation::INotationPtr& notation);

    project::INotationProjectPtr m_currentProject;
    muse::async::Notification m_currentProjectChanged;

    notation::INotationPtr m_currentNotation;
    muse::async::Notification m_currentNotationChanged;
};
}
