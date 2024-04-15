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
#ifndef MU_ENGRAVING_IFILEINFOPROVIDER_H
#define MU_ENGRAVING_IFILEINFOPROVIDER_H

#include <memory>

#include "types/datetime.h"
#include "io/path.h"

namespace mu::engraving {
class IFileInfoProvider
{
public:
    virtual ~IFileInfoProvider() = default;

    virtual muse::io::path_t path() const = 0; //! Absolute path
    virtual muse::io::path_t fileName(bool includingExtension = true) const = 0;
    virtual muse::io::path_t absoluteDirPath() const = 0; //! Absolute path of the containing folder

    virtual muse::String displayName() const = 0;

    virtual muse::DateTime birthTime() const = 0;
    virtual muse::DateTime lastModified() const = 0;
};

using IFileInfoProviderPtr = std::shared_ptr<IFileInfoProvider>;
}

#endif // MU_ENGRAVING_IFILEINFOPROVIDER_H
