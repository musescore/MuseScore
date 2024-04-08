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
#include "autobotconfiguration.h"

#include <cstdlib>
#include <QDir>

using namespace muse;
using namespace muse::autobot;

io::paths_t AutobotConfiguration::scriptsDirPaths() const
{
    io::path_t p = io::path_t(std::getenv("MUSE_AUTOBOT_SCRIPTS_PATH"));
    if (!p.empty()) {
        return { p };
    }

    io::paths_t paths;
    paths.push_back(globalConfiguration()->appDataPath() + "/autobotscripts");
    paths.push_back(globalConfiguration()->userDataPath() + "/AutobotScripts");

    return paths;
}

io::paths_t AutobotConfiguration::testingFilesDirPaths() const
{
    io::path_t p = io::path_t(std::getenv("MUSE_AUTOBOT_FILES_PATH"));
    if (!p.empty()) {
        return { p };
    }

    io::paths_t paths;
    paths.push_back(globalConfiguration()->appDataPath() + "/autobotscripts/data");
    paths.push_back(globalConfiguration()->userDataPath() + "/AutobotTestingFiles");

    return paths;
}

io::path_t AutobotConfiguration::dataPath() const
{
    io::path_t p = io::path_t(std::getenv("MUSE_AUTOBOT_DATA_PATH"));
    if (!p.empty()) {
        return p;
    }

    p = globalConfiguration()->userDataPath() + "/AutobotData";
    return p;
}

io::path_t AutobotConfiguration::savingFilesPath() const
{
    return dataPath() + "/saving_files";
}

io::path_t AutobotConfiguration::reportsPath() const
{
    return dataPath() + "/reports";
}

io::path_t AutobotConfiguration::drawDataPath() const
{
    return dataPath() + "/draw_data";
}

io::path_t AutobotConfiguration::fileDrawDataPath(const io::path_t& filePath) const
{
    return drawDataPath() + "/" + io::completeBasename(filePath) + ".json";
}
