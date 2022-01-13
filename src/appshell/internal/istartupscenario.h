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
#ifndef MU_APPSHELL_ISTARTUPSCENARIO_H
#define MU_APPSHELL_ISTARTUPSCENARIO_H

#include "modularity/ioc.h"
#include "io/path.h"
#include "uri.h"

namespace mu::appshell {
class IStartupScenario : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IStartupScenario)

public:
    virtual ~IStartupScenario() = default;

    virtual void setModeType(const QString& modeType) = 0;
    virtual void setStartupScorePath(const io::path& path) = 0;

    virtual void run() = 0;
    virtual bool startupCompleted() const = 0;
};
}

#endif // MU_APPSHELL_ISTARTUPSCENARIO_H
