/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "modularity/imoduleinterface.h"

namespace mu::iex::mnxio {
class IMnxConfiguration : MODULE_GLOBAL_EXPORT_INTERFACE
{
    INTERFACE_ID(IMnxConfiguration)

public:
    virtual ~IMnxConfiguration() = default;

    virtual int mnxIndentSpaces() const = 0;
    virtual void setMnxIndentSpaces(int value) = 0;

    virtual bool mnxExportBeams() const = 0;
    virtual void setMnxExportBeams(bool value) = 0;

    virtual bool mnxExportRestPositions() const = 0;
    virtual void setMnxExportRestPositions(bool value) = 0;
};
}
