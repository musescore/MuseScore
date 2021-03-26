//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#include "autobotconfiguration.h"

#include <cstdlib>

using namespace mu::autobot;

mu::io::path AutobotConfiguration::dataPath() const
{
    return io::path(std::getenv("MU_AUTOBOT_DATA_PATH"));
}

mu::io::path AutobotConfiguration::filesPath() const
{
    return io::path(std::getenv("MU_AUTOBOT_FILES_PATH"));
}

mu::io::path AutobotConfiguration::drawDataPath() const
{
    return dataPath() + "/draw_data";
}

mu::io::path AutobotConfiguration::fileDrawDataPath(const io::path& filePath) const
{
    return drawDataPath() + "/" + io::basename(filePath) + ".json";
}

mu::io::path AutobotConfiguration::reportsPath() const
{
    return dataPath() + "/reports";
}
