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

#ifndef MU_PROJECT_TEMPLATESREPOSITORY_H
#define MU_PROJECT_TEMPLATESREPOSITORY_H

#include "modularity/ioc.h"

#include "itemplatesrepository.h"
#include "project/iprojectconfiguration.h"
#include "project/imscmetareader.h"
#include "io/ifilesystem.h"

namespace mu::project {
class TemplatesRepository : public ITemplatesRepository
{
    INJECT(IProjectConfiguration, configuration)
    INJECT(IMscMetaReader, mscReader)
    INJECT(io::IFileSystem, fileSystem)

public:
    RetVal<Templates> templates() const override;

private:
    Templates readTemplates(const io::path_t& dirPath) const;
    Templates readTemplates(const io::paths_t& files, const QString& category, const io::path_t& dirPath = io::path_t()) const;
};
}

#endif // MU_PROJECT_TEMPLATESREPOSITORY_H
