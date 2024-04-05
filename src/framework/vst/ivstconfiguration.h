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

#ifndef MUSE_VST_IVSTCONFIGURATION_H
#define MUSE_VST_IVSTCONFIGURATION_H

#include "modularity/imoduleinterface.h"

#include "io/path.h"
#include "async/channel.h"

namespace muse::vst {
class IVstConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IVstConfiguration)

public:
    virtual ~IVstConfiguration() = default;

    virtual io::paths_t userVstDirectories() const = 0;
    virtual void setUserVstDirectories(const io::paths_t& paths) = 0;
    virtual async::Channel<io::paths_t> userVstDirectoriesChanged() const = 0;
};
}

#endif // MUSE_VST_IVSTCONFIGURATION_H
