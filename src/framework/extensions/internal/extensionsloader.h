/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#ifndef MU_EXTENSIONS_EXTENSIONSLOADER_H
#define MU_EXTENSIONS_EXTENSIONSLOADER_H

#include "global/io/path.h"

#include "../extensionstypes.h"

namespace mu::extensions {
class ExtensionsLoader
{
public:
    ExtensionsLoader() = default;

    ManifestList loadManifesList(const io::path_t& defPath, const io::path_t& extPath) const;

private:
    ManifestList manifesList(const io::path_t& rootPath) const;
    mu::io::paths_t manifestPaths(const io::path_t& rootPath) const;
    Manifest parseManifest(const io::path_t& path) const;
    void resolvePaths(Manifest& m, const io::path_t& rootDirPath) const;
};
}

#endif // MU_EXTENSIONS_EXTENSIONSLOADER_H
