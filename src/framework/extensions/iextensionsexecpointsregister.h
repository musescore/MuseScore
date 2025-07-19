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
#pragma once

#include <vector>

#include "modularity/imoduleinterface.h"

#include "extensionstypes.h"

namespace muse::extensions {
class IExtensionsExecPointsRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IExtensionsExecPointsRegister)
public:

    virtual ~IExtensionsExecPointsRegister() = default;

    virtual void reg(const std::string& module, const ExecPoint& p) = 0;
    virtual ExecPoint point(const std::string& name) const = 0;
    virtual std::vector<ExecPoint> allPoints() const = 0;
};
}
