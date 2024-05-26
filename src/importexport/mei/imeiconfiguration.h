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
#ifndef MU_IMPORTEXPORT_IMEICONFIGURATION_H
#define MU_IMPORTEXPORT_IMEICONFIGURATION_H

#include "modularity/imoduleinterface.h"

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

    virtual bool meiUseMuseScoreIds() const = 0;
    virtual void setMeiUseMuseScoreIds(bool value) = 0;
};
}

#endif // MU_IMPORTEXPORT_IMEICONFIGURATION_H
