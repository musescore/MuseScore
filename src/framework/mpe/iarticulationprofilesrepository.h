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

#ifndef MUSE_MPE_IARTICULATIONPROFILESREPOSITORY_H
#define MUSE_MPE_IARTICULATIONPROFILESREPOSITORY_H

#include "modularity/imoduleinterface.h"
#include "async/channel.h"
#include "io/path.h"

#include "mpetypes.h"

namespace muse::mpe {
class IArticulationProfilesRepository : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IArticulationProfilesRepository)

public:
    virtual ~IArticulationProfilesRepository() = default;

    virtual ArticulationsProfilePtr createNew() const = 0;
    virtual ArticulationsProfilePtr defaultProfile(const ArticulationFamily family) const = 0;
    virtual ArticulationsProfilePtr loadProfile(const io::path_t& path) const = 0;
    virtual void saveProfile(const io::path_t& path, const ArticulationsProfilePtr profilePtr) = 0;
    virtual async::Channel<io::path_t> profileChanged() const = 0;
};
}

#endif // MUSE_MPE_IARTICULATIONPROFILESREPOSITORY_H
