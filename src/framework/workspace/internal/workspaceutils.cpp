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
#include "workspaceutils.h"

using namespace muse::workspace;

bool WorkspaceUtils::workspaceLessThan(const IWorkspacePtr& workspace1, const IWorkspacePtr& workspace2)
{
    bool isWorkspace1Builtin = workspace1->isBuiltin();
    bool isWorkspace2Builtin = workspace2->isBuiltin();
    if (isWorkspace1Builtin != isWorkspace2Builtin) {
        return isWorkspace1Builtin;
    }

    if (isWorkspace1Builtin && isWorkspace2Builtin) {
        return false;
    }

    return workspace1->name() < workspace2->name();
}
