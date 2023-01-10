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
#ifndef MU_DIAGNOSTICS_IENGRAVINGDRAWPROVIDER_H
#define MU_DIAGNOSTICS_IENGRAVINGDRAWPROVIDER_H

#include "modularity/imoduleexport.h"
#include "global/types/ret.h"
#include "global/io/path.h"

namespace mu::diagnostics {
class IEngravingDrawProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IEngravingDrawProvider)
public:
    virtual ~IEngravingDrawProvider() = default;

    virtual Ret genDrawData(const io::path_t& scoresDir, const io::path_t& outDir) = 0;
};
}

#endif // MU_DIAGNOSTICS_IENGRAVINGDRAWPROVIDER_H
