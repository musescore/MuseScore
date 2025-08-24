/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
#include "musesoundscheckupdatescenariostub.h"

using namespace mu::musesounds;

bool MuseSoundsCheckUpdateScenarioStub::needCheckForUpdate() const
{
    return false;
}

muse::async::Promise<muse::Ret> MuseSoundsCheckUpdateScenarioStub::checkForUpdate(bool)
{
    return muse::async::make_promise<Ret>([this](auto, auto) {
        return muse::async::Promise<muse::Ret>::dummy_result();
    });
}

bool MuseSoundsCheckUpdateScenarioStub::hasUpdate() const
{
    return false;
}

muse::Ret MuseSoundsCheckUpdateScenarioStub::showUpdate()
{
    return muse::make_ok();
}
