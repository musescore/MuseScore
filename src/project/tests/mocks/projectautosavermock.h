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

#include "project/iprojectautosaver.h"

namespace mu::project {
class ProjectAutoSaverMock : public IProjectAutoSaver
{
public:
    MOCK_METHOD(bool, projectHasUnsavedChanges, (const muse::io::path_t& projectPath), (const, override));
    MOCK_METHOD(void, removeProjectUnsavedChanges, (const muse::io::path_t& projectPath), (override));
    MOCK_METHOD(bool, isAutosaveOfNewlyCreatedProject, (const muse::io::path_t& projectPath), (const, override));
    MOCK_METHOD(muse::io::path_t, projectOriginalPath, (const muse::io::path_t& projectAutoSavePath), (const, override));
    MOCK_METHOD(muse::io::path_t, projectAutoSavePath, (const muse::io::path_t& projectPath), (const, override));
};
}
