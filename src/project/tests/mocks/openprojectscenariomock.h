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

#include "project/internal/iopenprojectscenario.h"

namespace mu::project {
class OpenProjectScenarioMock : public IOpenProjectScenario
{
public:
    MOCK_METHOD(muse::Ret, openProject, (const ProjectFile& file), (override));
    MOCK_METHOD(muse::Ret, openProject, (const muse::io::path_t& path, const QString& displayNameOverride), (override));
    MOCK_METHOD(muse::Ret, openProject, (const muse::rcommand::Params& params), (override));
    MOCK_METHOD(void, revertToLastSaved, (), (override));
    MOCK_METHOD(muse::Ret, finishOpening, (), (override));
    MOCK_METHOD(const ProjectBeingDownloaded&, projectBeingDownloaded, (), (const, override));
    MOCK_METHOD(muse::async::Notification, projectBeingDownloadedChanged, (), (const, override));
    MOCK_METHOD(bool, isBusy, (IProjectCommandsController::BusyStatus status), (const, override));
    MOCK_METHOD(muse::async::Notification, busyChanged, (), (const, override));
    MOCK_METHOD(void, showCloudOpenError, (const muse::Ret& ret), (const, override));
};
}
