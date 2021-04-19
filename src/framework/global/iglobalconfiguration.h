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
#ifndef MU_FRAMEWORK_IGLOBALCONFIGURATION_H
#define MU_FRAMEWORK_IGLOBALCONFIGURATION_H

#include "modularity/imoduleexport.h"
#include "io/path.h"

namespace mu::framework {
class IGlobalConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IGlobalConfiguration)
public:

    virtual ~IGlobalConfiguration() = default;

    virtual io::path appDirPath() const = 0;
    virtual io::path sharePath() const = 0;
    virtual io::path dataPath() const = 0;
    virtual io::path logsPath() const = 0;
    virtual io::path backupPath() const = 0;

    virtual bool useFactorySettings() const = 0;
    virtual bool enableExperimental() const = 0;
};
}

#endif // MU_FRAMEWORK_IGLOBALCONFIGURATION_H
