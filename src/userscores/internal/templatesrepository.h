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

#ifndef MU_USERSCORES_TEMPLATESREPOSITORY_H
#define MU_USERSCORES_TEMPLATESREPOSITORY_H

#include "modularity/ioc.h"

#include "itemplatesrepository.h"
#include "userscores/iuserscoresconfiguration.h"
#include "notation/imsczmetareader.h"
#include "system/ifilesystem.h"

namespace mu::userscores {
class TemplatesRepository : public ITemplatesRepository
{
    INJECT(userscores, IUserScoresConfiguration, configuration)
    INJECT(userscores, notation::IMsczMetaReader, msczReader)
    INJECT(userscores, system::IFileSystem, fileSystem)

public:
    RetVal<Templates> templates() const override;

private:
    Templates loadTemplates(const io::paths& filePaths) const;
    QString correctedTitle(const QString& title) const;
};
}

#endif // MU_USERSCORES_TEMPLATESREPOSITORY_H
