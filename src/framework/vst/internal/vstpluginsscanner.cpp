/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

#include "vstpluginsscanner.h"

#include "vst/vsttypes.h"

#include "log.h"

using namespace muse;
using namespace muse::vst;

/**
 * @brief Scanning for plugins in the default VST locations, considering the current architecture (i386, x86_64, arm, etc.)
 * @see https://developer.steinberg.help/pages/viewpage.action?pageId=9798275
 **/
namespace muse::vst {
static std::set<io::path_t> pluginPathsFromDefaultLocation()
{
    std::set<io::path_t> result;

    try {
        PluginModule::PathList paths = PluginModule::getModulePaths();
        for (const std::string& path : paths) {
            result.insert(path);
        }
    } catch (...) {
        LOGE() << "Unable to get module paths";
    }

    return result;
}
}

io::paths_t VstPluginsScanner::scanPlugins() const
{
    TRACEFUNC;

    std::set<io::path_t> unique = pluginPathsFromDefaultLocation();
    std::set<io::path_t> custom = pluginPathsFromCustomLocations(configuration()->userVstDirectories());
    unique.insert(custom.begin(), custom.end());

    return { unique.begin(), unique.end() };
}

std::set<io::path_t> VstPluginsScanner::pluginPathsFromCustomLocations(const io::paths_t& customPaths) const
{
    std::set<io::path_t> result;

    for (const io::path_t& path : customPaths) {
        RetVal<io::paths_t> paths = fileSystem()->scanFiles(path, { VST3_PACKAGE_FILTER });
        if (!paths.ret) {
            LOGE() << paths.ret.toString();
            continue;
        }

        result.insert(paths.val.begin(), paths.val.end());
    }

    return result;
}
