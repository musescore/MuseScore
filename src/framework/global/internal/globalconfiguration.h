//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_FRAMEWORK_GLOBALCONFIGURATION_H
#define MU_FRAMEWORK_GLOBALCONFIGURATION_H

#include "../iglobalconfiguration.h"

namespace mu {
namespace framework {
class GlobalConfiguration : public IGlobalConfiguration
{
public:
    GlobalConfiguration() = default;

    io::path appDirPath() const override;
    io::path sharePath() const override;
    io::path dataPath() const override;
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
}
#endif // MU_FRAMEWORK_GLOBALCONFIGURATION_H
