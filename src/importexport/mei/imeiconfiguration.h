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
#ifndef MU_IMPORTEXPORT_IMEICONFIGURATION_H
#define MU_IMPORTEXPORT_IMEICONFIGURATION_H

#include "modularity/imodulesetup.h"
#include "io/path.h"

namespace mu::iex::mei {
class IMeiConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMeiConfiguration)

public:
    virtual ~IMeiConfiguration() = default;

    virtual bool meiImportLayout() const = 0;
    virtual void setMeiImportLayout(bool value) = 0;

    virtual bool meiExportLayout() const = 0;
    virtual void setMeiExportLayout(bool value) = 0;
};
}

#endif // MU_IMPORTEXPORT_IMEICONFIGURATION_H
