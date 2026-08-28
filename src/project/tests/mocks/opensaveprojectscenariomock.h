/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#pragma once

#include <gmock/gmock.h>

#include "project/internal/iopensaveprojectscenario.h"

namespace mu::project {
class OpenSaveProjectScenarioMock : public IOpenSaveProjectScenario
{
public:
    MOCK_METHOD(muse::RetVal<SaveLocation>, askSaveLocation,
                (INotationProjectPtr project, SaveMode mode, SaveLocationType preselectedType), (const, override));

    MOCK_METHOD(muse::RetVal<muse::io::path_t>, askLocalPath, (INotationProjectPtr project, SaveMode mode), (const, override));
    MOCK_METHOD(muse::RetVal<CloudProjectInfo>, askCloudLocation, (INotationProjectPtr project, SaveMode mode), (const, override));
    MOCK_METHOD(muse::RetVal<CloudProjectInfo>, askPublishLocation, (INotationProjectPtr project), (const, override));
    MOCK_METHOD(muse::RetVal<CloudAudioInfo>, askShareAudioLocation, (INotationProjectPtr project), (const, override));

    MOCK_METHOD(bool, warnBeforeSavingToExistingPubliclyVisibleCloudProject, (), (const, override));

    MOCK_METHOD(void, showCloudOpenError, (const muse::Ret& ret), (const, override));
    MOCK_METHOD(muse::Ret, showCloudSaveError,
                (const muse::Ret& ret, const CloudProjectInfo& info, bool publishMode, bool alreadyAttempted), (const, override));
    MOCK_METHOD(muse::Ret, showAudioCloudShareError, (const muse::Ret& ret), (const, override));
};
}
