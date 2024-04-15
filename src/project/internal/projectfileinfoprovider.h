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
#ifndef MU_PROJECT_PROJECTFILEINFOPROVIDER_H
#define MU_PROJECT_PROJECTFILEINFOPROVIDER_H

#include "engraving/infrastructure/ifileinfoprovider.h"

#include "modularity/ioc.h"
#include "io/ifilesystem.h"

namespace mu::project {
class NotationProject;
class ProjectFileInfoProvider : public engraving::IFileInfoProvider
{
    INJECT(muse::io::IFileSystem, filesystem)
public:
    explicit ProjectFileInfoProvider(NotationProject* project);

    muse::io::path_t path() const override;
    muse::io::path_t fileName(bool includingExtension = true) const override;
    muse::io::path_t absoluteDirPath() const override;

    muse::String displayName() const override;

    muse::DateTime birthTime() const override;
    muse::DateTime lastModified() const override;

private:
    NotationProject* m_project = nullptr;
};
}

#endif // MU_PROJECT_PROJECTFILEINFOPROVIDER_H
