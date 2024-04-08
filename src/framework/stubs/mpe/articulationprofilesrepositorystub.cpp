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
#include "articulationprofilesrepositorystub.h"

using namespace muse;
using namespace muse::mpe;

ArticulationsProfilePtr ArticulationProfilesRepositoryStub::createNew() const
{
    return std::make_shared<ArticulationsProfile>();
}

ArticulationsProfilePtr ArticulationProfilesRepositoryStub::defaultProfile(const ArticulationFamily) const
{
    return std::make_shared<ArticulationsProfile>();
}

ArticulationsProfilePtr ArticulationProfilesRepositoryStub::loadProfile(const io::path_t&) const
{
    return std::make_shared<ArticulationsProfile>();
}

void ArticulationProfilesRepositoryStub::saveProfile(const io::path_t&, const ArticulationsProfilePtr)
{
}

async::Channel<io::path_t> ArticulationProfilesRepositoryStub::profileChanged() const
{
    static async::Channel<io::path_t> ch;
    return ch;
}
