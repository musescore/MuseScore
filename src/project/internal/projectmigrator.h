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
#ifndef MU_PROJECT_PROJECTMIGRATOR_H
#define MU_PROJECT_PROJECTMIGRATOR_H

#include "iprojectmigrator.h"

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "iprojectconfiguration.h"

namespace mu::project {
class ProjectMigrator : public IProjectMigrator
{
    INJECT(IProjectConfiguration, configuration)
    INJECT(muse::IInteractive, interactive)
public:
    ProjectMigrator() = default;

    muse::Ret migrateEngravingProjectIfNeed(engraving::EngravingProjectPtr project) override;

private:

    muse::Ret askAboutMigration(MigrationOptions& out, const QString& appVersion, MigrationType migrationType);

    muse::Ret migrateProject(engraving::EngravingProjectPtr project, const MigrationOptions& opt);

    bool applyLelandStyle(mu::engraving::MasterScore* score);
    bool applyEdwinStyle(mu::engraving::MasterScore* score);
    bool resetAllElementsPositions(mu::engraving::MasterScore* score);
    bool resetCrossBeams(mu::engraving::MasterScore* score);
    void resetStyleSettings(mu::engraving::MasterScore* score);

    bool m_resetStyleSettings{ false };
};
}

#endif // MU_PROJECT_PROJECTMIGRATOR_H
