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

#include "vstconfigurationstub.h"

using namespace muse;
using namespace muse::vst;

io::paths_t VstConfigurationStub::userVstDirectories() const
{
    return {};
}

void VstConfigurationStub::setUserVstDirectories(const io::paths_t&)
{
}

async::Channel<io::paths_t> VstConfigurationStub::userVstDirectoriesChanged() const
{
    static async::Channel<io::paths_t> stub;
    return stub;
}

std::string VstConfigurationStub::usedVstView() const
{
    return std::string();
}

void VstConfigurationStub::setUsedVstView(const std::string&)
{
}
