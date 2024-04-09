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
#ifndef MUSE_DIAGNOSTICS_IDIAGNOSTICSPATHSREGISTER_H
#define MUSE_DIAGNOSTICS_IDIAGNOSTICSPATHSREGISTER_H

#include "modularity/imoduleinterface.h"
#include "io/path.h"

namespace muse::diagnostics {
class IDiagnosticsPathsRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IDiagnosticsPathsRegister)
public:
    virtual ~IDiagnosticsPathsRegister() = default;

    struct Item
    {
        std::string name;
        muse::io::path_t path;
    };

    virtual void reg(const std::string& name, const muse::io::path_t& path) = 0;
    virtual const std::vector<Item>& items() const = 0;
};
}

#endif // MUSE_DIAGNOSTICS_IDIAGNOSTICSPATHSREGISTER_H
