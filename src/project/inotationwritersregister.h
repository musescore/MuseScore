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

#ifndef MU_PROJECT_INOTATIONWRITERSREGISTER_H
#define MU_PROJECT_INOTATIONWRITERSREGISTER_H

#include "modularity/imoduleinterface.h"
#include "inotationwriter.h"

namespace mu::project {
class INotationWritersRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(INotationWritersRegister)

public:
    virtual ~INotationWritersRegister() = default;

    virtual void reg(const std::vector<std::string>& suffixes, INotationWriterPtr writer) = 0;
    virtual INotationWriterPtr writer(const std::string& suffix) const = 0;
};
}

#endif // MU_PROJECT_INOTATIONWRITERSREGISTER_H
