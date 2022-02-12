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
#ifndef MU_PROJECT_PROJECTAUTOSAVER_H
#define MU_PROJECT_PROJECTAUTOSAVER_H

#include <QTimer>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "system/ifilesystem.h"
#include "iprojectconfiguration.h"

#include "../iprojectautosaver.h"

namespace mu::project {
class ProjectAutoSaver : public IProjectAutoSaver, public async::Asyncable
{
    INJECT(project, context::IGlobalContext, globalContext)
    INJECT(project, system::IFileSystem, fileSystem)
    INJECT(project, IProjectConfiguration, configuration)

public:
    ProjectAutoSaver() = default;

    void init();

    bool projectHasUnsavedChanges(const io::path& projectPath) const override;
    void removeProjectUnsavedChanges(const io::path& projectPath) override;

    io::path projectOriginalPath(const io::path& projectAutoSavePath) const override;
    io::path projectAutoSavePath(const io::path& projectPath) const override;

private:
    INotationProjectPtr currentProject() const;

    void update();

    void onTrySave();

    QTimer m_timer;
    io::path m_lastProjectPathNeedingAutosave;
};
}

#endif // MU_PROJECT_PROJECTAUTOSAVER_H
