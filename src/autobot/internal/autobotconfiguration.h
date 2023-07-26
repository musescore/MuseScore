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
#ifndef MU_AUTOBOT_AUTOBOTCONFIGURATION_H
#define MU_AUTOBOT_AUTOBOTCONFIGURATION_H

#include "../iautobotconfiguration.h"

#include "modularity/ioc.h"
#include "iglobalconfiguration.h"

namespace mu::autobot {
class AutobotConfiguration : public IAutobotConfiguration
{
    INJECT(framework::IGlobalConfiguration, globalConfiguration)

public:
    AutobotConfiguration() = default;

    io::paths_t scriptsDirPaths() const override;
    io::paths_t testingFilesDirPaths() const override;

    io::path_t dataPath() const override;
    io::path_t savingFilesPath() const override;
    io::path_t reportsPath() const override;
    io::path_t drawDataPath() const override;
    io::path_t fileDrawDataPath(const io::path_t& filePath) const override;
};
}

#endif // MU_AUTOBOT_AUTOBOTCONFIGURATION_H
