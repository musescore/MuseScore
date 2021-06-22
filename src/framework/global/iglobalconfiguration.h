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

    //! NOTE Each platform has its own paths and rules
    //! More details about paths: https://github.com/musescore/MuseScore/wiki/Resources

    //! NOTE The path to the dir with the executable file (probably readonly, probably private for a user)
    //! Like: programs/MuseScore/bin
    virtual io::path appBinPath() const = 0;

    //! NOTE The path to the dir with the app data files (probably readonly, probably private for a user)
    //! Like: programs/MuseScore/share
    virtual io::path appDataPath() const = 0;

    //! NOTE The path to the dir with the app configure files (must be writable, probably private for a user)
    //! Like: user/config/MuseScore
    virtual io::path appConfigPath() const = 0;

    //! NOTE The path to the dir with the app user data files (must be writable, probably private for a user)
    //! Like: user/appdata/MuseScore
    virtual io::path userAppDataPath() const = 0;

    //! NOTE The path to the dir with the user backup files (must be writable, probably private for a user)
    //! Like: user/appdata/MuseScore/backups
    virtual io::path userBackupPath() const = 0;

    //! NOTE The path to the dir with the user data files (must be writable, probably public for a user)
    //! Like: user/documents/MuseScore
    virtual io::path userDataPath() const = 0;

    virtual bool useFactorySettings() const = 0;
    virtual bool enableExperimental() const = 0;
};
}

#endif // MU_FRAMEWORK_IGLOBALCONFIGURATION_H
