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
#ifndef MUSE_EXTENSIONS_EXTPLUGINSLOADER_H
#define MUSE_EXTENSIONS_EXTPLUGINSLOADER_H

#include "../../extensionstypes.h"

namespace muse::extensions::legacy {
//! NOTE Searches and parses qml files of old plugins
class ExtPluginsLoader
{
public:
    ExtPluginsLoader() = default;

    std::map<std::string /*codeKey*/, Uri> loadCodekeyUriMap(const io::path_t& defPath, const io::path_t& extPath) const;

    ManifestList loadManifestList(const io::path_t& defPath, const io::path_t& extPath) const;

private:

    ManifestList manifestList(const io::path_t& rootPath) const;
    io::paths_t qmlsPaths(const io::path_t& rootPath) const;
    Manifest parseManifest(const io::path_t& rootPath, const io::path_t& path) const;
    void resolvePaths(Manifest& m, const io::path_t& rootDirPath) const;
};
}

#endif // MUSE_EXTENSIONS_EXTPLUGINSLOADER_H
