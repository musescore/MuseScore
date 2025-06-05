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

#include "vstconfiguration.h"

#include "settings.h"

using namespace muse;
using namespace muse::vst;

static const std::string module_name("vst");

static const Settings::Key USER_VST_PATHS = Settings::Key(module_name, "application/paths/myVSTs");
static const Settings::Key USED_VST_VIEW = Settings::Key(module_name, "application/vst/view");

void VstConfiguration::init()
{
    settings()->setDefaultValue(USED_VST_VIEW, Val("newview"));
    settings()->setDefaultValue(USER_VST_PATHS, Val(""));
    settings()->valueChanged(USER_VST_PATHS).onReceive(nullptr, [this](const Val&) {
        m_userVstDirsChanged.send(userVstDirectories());
    });
}

io::paths_t VstConfiguration::userVstDirectories() const
{
    std::string pathsStr = settings()->value(USER_VST_PATHS).toString();
    return io::pathsFromString(pathsStr);
}

void VstConfiguration::setUserVstDirectories(const io::paths_t& paths)
{
    settings()->setSharedValue(USER_VST_PATHS, Val(io::pathsToString(paths)));
}

async::Channel<io::paths_t> VstConfiguration::userVstDirectoriesChanged() const
{
    return m_userVstDirsChanged;
}

std::string VstConfiguration::usedVstView() const
{
    return settings()->value(USED_VST_VIEW).toString();
}

void VstConfiguration::setUsedVstView(const std::string& code)
{
    settings()->setSharedValue(USED_VST_VIEW, Val(code));
}
