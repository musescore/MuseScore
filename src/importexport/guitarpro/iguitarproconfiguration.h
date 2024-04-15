/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_IMPORTEXPORT_IGUITARPROCONFIGURATION_H
#define MU_IMPORTEXPORT_IGUITARPROCONFIGURATION_H

#include <optional>

#include "modularity/imoduleinterface.h"

namespace mu::iex::guitarpro {
class IGuitarProConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IGuitarProConfiguration)

public:
    virtual ~IGuitarProConfiguration() = default;

    virtual bool linkedTabStaffCreated() const = 0;
    virtual void setLinkedTabStaffCreated(std::optional<bool> created) = 0;

    virtual bool experimental() const = 0;
    virtual void setExperimental(std::optional<bool> experimental) = 0;
};
}

#endif // MU_IMPORTEXPORT_IGUITARPROCONFIGURATION_H
