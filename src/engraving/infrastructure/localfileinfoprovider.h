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
#ifndef MU_ENGRAVING_LocalFILEINFOPROVIDER_H
#define MU_ENGRAVING_LocalFILEINFOPROVIDER_H

#include "ifileinfoprovider.h"

#include "modularity/ioc.h"
#include "io/ifilesystem.h"

namespace mu::engraving {
class LocalFileInfoProvider : public IFileInfoProvider
{
    muse::GlobalInject<muse::io::IFileSystem> fileSystem;

public:
    explicit LocalFileInfoProvider(const muse::io::path_t& filePath);

    muse::io::path_t path() const override;
    muse::io::path_t fileName(bool includingExtension = true) const override;
    muse::io::path_t absoluteDirPath() const override;

    muse::String displayName() const override;

    muse::DateTime birthTime() const override;
    muse::DateTime lastModified() const override;

private:
    muse::io::path_t m_path;
};
}

#endif // MU_ENGRAVING_LocalFILEINFOPROVIDER_H
