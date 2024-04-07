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

#ifndef MUSE_MPE_ARTICULATIONPROFILESREPOSITORYMOCK_H
#define MUSE_MPE_ARTICULATIONPROFILESREPOSITORYMOCK_H

#include <gmock/gmock.h>

#include "mpe/iarticulationprofilesrepository.h"

namespace muse::mpe {
class ArticulationProfilesRepositoryMock : public IArticulationProfilesRepository
{
public:
    MOCK_METHOD(ArticulationsProfilePtr, createNew, (), (const, override));
    MOCK_METHOD(ArticulationsProfilePtr, defaultProfile, (const ArticulationFamily), (const, override));
    MOCK_METHOD(ArticulationsProfilePtr, loadProfile, (const io::path_t&), (const, override));
    MOCK_METHOD(void, saveProfile, (const io::path_t&, const ArticulationsProfilePtr), (override));
    MOCK_METHOD(async::Channel<io::path_t>, profileChanged, (), (const, override));
};
}

#endif // MUSE_MPE_ARTICULATIONPROFILESREPOSITORYMOCK_H
