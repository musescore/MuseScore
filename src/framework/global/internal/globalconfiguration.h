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
#ifndef MU_FRAMEWORK_GLOBALCONFIGURATION_H
#define MU_FRAMEWORK_GLOBALCONFIGURATION_H

#include "../iglobalconfiguration.h"
#include "modularity/ioc.h"

namespace mu::framework {
class GlobalConfiguration : public IGlobalConfiguration
{
public:
    GlobalConfiguration() = default;

    io::path appBinPath() const override;
    io::path appDataPath() const override;
    io::path userDataPath() const override;
    io::path logsPath() const override;
    io::path backupPath() const override;

    bool useFactorySettings() const override;
    bool enableExperimental() const override;

private:
    QString getSharePath() const;

    mutable io::path m_sharePath;
    mutable io::path m_dataPath;
};
}

#endif // MU_FRAMEWORK_GLOBALCONFIGURATION_H
