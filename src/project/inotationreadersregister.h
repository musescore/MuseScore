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
#ifndef MU_PROJECT_INOTATIONREADERSREGISTER_H
#define MU_PROJECT_INOTATIONREADERSREGISTER_H

#include <string>
#include <vector>

#include "modularity/imoduleinterface.h"
#include "inotationreader.h"

namespace mu::project {
class INotationReadersRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(INotationReadersRegister)

public:
    virtual ~INotationReadersRegister() = default;

    //! NOTE In the future, we need to replace the suffix with an enumerator
    //! or a better structure describing the format.
    virtual void reg(const std::vector<std::string>& suffixes, INotationReaderPtr reader) = 0;
    virtual INotationReaderPtr reader(const std::string& suffix) = 0;
};
}

#endif // MU_PROJECT_INOTATIONREADERSREGISTER_H
