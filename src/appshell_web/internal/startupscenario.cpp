/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "startupscenario.h"

#include "log.h"

using namespace muse;
using namespace mu::appshell;

void StartupScenario::setStartupType(const std::optional<std::string>& /*type*/)
{
    NOT_IMPLEMENTED;
}

bool StartupScenario::isStartWithNewFileAsSecondaryInstance() const
{
    return false;
}

const mu::project::ProjectFile& StartupScenario::startupScoreFile() const
{
    static mu::project::ProjectFile file;
    return file;
}

void StartupScenario::setStartupScoreFile(const std::optional<project::ProjectFile>& /*file*/)
{
    NOT_IMPLEMENTED;
}

void StartupScenario::runOnSplashScreen()
{
}

void StartupScenario::runAfterSplashScreen()
{
    interactive()->open("musescore://notation").onResolve(this, [this](const Val&) {
        m_startupCompleted = true;
    });
}

bool StartupScenario::startupCompleted() const
{
    return m_startupCompleted;
}
