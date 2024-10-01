/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#ifndef MUSE_UPDATE_MUSESOUNDSUPDATESERVICESTUB_H
#define MUSE_UPDATE_MUSESOUNDSUPDATESERVICESTUB_H

#include "update/imusesoundscheckupdateservice.h"

namespace muse::update {
class MuseSoundsCheckUpdateServiceStub : public IMuseSoundsCheckUpdateService
{
public:
    Ret needCheckForUpdate() const override;

    RetVal<ReleaseInfo> checkForUpdate() override;
    RetVal<ReleaseInfo> lastCheckResult() override;

    Progress updateProgress() override;
};
}

#endif // MUSE_UPDATE_MUSESOUNDSUPDATESERVICESTUB_H
