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

#ifndef MU_VST_IVSTCONFIGURATION_H
#define MU_VST_IVSTCONFIGURATION_H

#include "modularity/imoduleexport.h"

#include "io/path.h"
#include "vsttypes.h"

namespace mu::vst {
class IVstConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IVstConfiguration)

public:
    virtual ~IVstConfiguration() = default;

    virtual mu::io::path customSearchPath() const = 0;
};
}

#endif // IVSTCONFIGURATION_H
