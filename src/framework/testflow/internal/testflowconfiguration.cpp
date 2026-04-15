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
#include "testflowconfiguration.h"

#include <cstdlib>
#include <QDir>

using namespace muse;
using namespace muse::testflow;

io::paths_t TestflowConfiguration::scriptsDirPaths() const
{
    io::path_t p = io::path_t(std::getenv("MUSE_TESTFLOW_SCRIPTS_PATH"));
    if (!p.empty()) {
        return { p };
    }

    io::paths_t paths;
    paths.push_back(globalConfiguration()->appDataPath() + "/testflowscripts");
    paths.push_back(globalConfiguration()->userDataPath() + "/TestflowScripts");

    return paths;
}

io::paths_t TestflowConfiguration::testingFilesDirPaths() const
{
    io::path_t p = io::path_t(std::getenv("MUSE_TESTFLOW_FILES_PATH"));
    if (!p.empty()) {
        return { p };
    }

    io::paths_t paths;
    paths.push_back(globalConfiguration()->appDataPath() + "/testflowscripts/data");
    paths.push_back(globalConfiguration()->userDataPath() + "/TestflowTestingFiles");

    return paths;
}

io::path_t TestflowConfiguration::dataPath() const
{
    io::path_t p = io::path_t(std::getenv("MUSE_TESTFLOW_DATA_PATH"));
    if (!p.empty()) {
        return p;
    }

    p = globalConfiguration()->userDataPath() + "/TestflowData";
    return p;
}

io::path_t TestflowConfiguration::savingFilesPath() const
{
    return dataPath() + "/saving_files";
}

io::path_t TestflowConfiguration::reportsPath() const
{
    return dataPath() + "/reports";
}

io::path_t TestflowConfiguration::drawDataPath() const
{
    return dataPath() + "/draw_data";
}

io::path_t TestflowConfiguration::fileDrawDataPath(const io::path_t& filePath) const
{
    return drawDataPath() + "/" + io::completeBasename(filePath) + ".json";
}
