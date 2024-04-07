/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#ifndef MUSE_MPE_ARTICULATIONPROFILESREPOSITORYSTUB_H
#define MUSE_MPE_ARTICULATIONPROFILESREPOSITORYSTUB_H

#include "mpe/iarticulationprofilesrepository.h"

namespace muse::mpe {
class ArticulationProfilesRepositoryStub : public IArticulationProfilesRepository
{
public:
    ArticulationProfilesRepositoryStub() = default;

    ArticulationsProfilePtr createNew() const override;
    ArticulationsProfilePtr defaultProfile(const ArticulationFamily family) const override;
    ArticulationsProfilePtr loadProfile(const io::path_t& path) const override;
    void saveProfile(const io::path_t& path, const ArticulationsProfilePtr profilePtr) override;
    async::Channel<io::path_t> profileChanged() const override;
};
}

#endif // MUSE_MPE_ARTICULATIONPROFILESREPOSITORYSTUB_H
