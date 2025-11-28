/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "appupdatescenariostub.h"

using namespace muse::update;

bool AppUpdateScenarioStub::needCheckForUpdate() const
{
    return false;
}

muse::async::Promise<muse::Ret> AppUpdateScenarioStub::checkForUpdate(bool)
{
    return muse::async::Promise<muse::Ret>([](auto /*resolve*/, auto reject) {
        return reject(int(muse::Ret::Code::UnknownError), "stub");
    });
}

bool AppUpdateScenarioStub::hasUpdate() const
{
    return false;
}

muse::async::Promise<muse::Ret> AppUpdateScenarioStub::showUpdate()
{
    return muse::async::Promise<muse::Ret>([](auto /*resolve*/, auto reject) {
        return reject(int(muse::Ret::Code::UnknownError), "stub");
    });
}
