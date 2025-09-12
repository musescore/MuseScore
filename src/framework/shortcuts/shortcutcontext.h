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
#ifndef MUSE_SHORTCUTS_SHORTCUTCONTEXT_H
#define MUSE_SHORTCUTS_SHORTCUTCONTEXT_H

#include <string>

#include "modularity/imoduleinterface.h"

namespace muse::shortcuts {
//! NOTE Only general shortcut contexts are declared here,
//! which do not depend on the specifics of the application.
//! Application-specific UI contexts are declared in the `context/shortcutcontext.h` file

static const std::string CTX_ANY("any");
static const std::string CTX_PROJECT_OPENED("project-opened");
static const std::string CTX_PROJECT_FOCUSED("project-focused");

//! NOTE special context for navigation shortcuts because the project has its own navigation system
static const std::string CTX_NOT_PROJECT_FOCUSED("not-project-focused");

class IShortcutContextPriority : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IShortcutContextPriority);

public:
    virtual ~IShortcutContextPriority() = default;

    virtual bool hasLowerPriorityThan(const std::string& ctx1, const std::string& ctx2) const = 0;
};
}

#endif // MUSE_SHORTCUTS_SHORTCUTCONTEXT_H
