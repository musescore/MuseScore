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
#ifndef MU_PROJECT_PROJECTFILEINFOPROVIDER_H
#define MU_PROJECT_PROJECTFILEINFOPROVIDER_H

#include "engraving/infrastructure/ifileinfoprovider.h"

#include "modularity/ioc.h"
#include "io/ifilesystem.h"

namespace mu::project {
class NotationProject;
class ProjectFileInfoProvider : public engraving::IFileInfoProvider
{
    INJECT(io::IFileSystem, filesystem)
public:
    explicit ProjectFileInfoProvider(NotationProject* project);

    io::path_t path() const override;
    io::path_t fileName(bool includingExtension = true) const override;
    io::path_t absoluteDirPath() const override;

    String displayName() const override;

    DateTime birthTime() const override;
    DateTime lastModified() const override;

private:
    NotationProject* m_project = nullptr;
};
}

#endif // MU_PROJECT_PROJECTFILEINFOPROVIDER_H
