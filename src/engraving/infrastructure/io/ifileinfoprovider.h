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
#ifndef MU_ENGRAVING_IFILEINFOPROVIDER_H
#define MU_ENGRAVING_IFILEINFOPROVIDER_H

#include <memory>

#include <QDateTime>

#include "io/path.h"

namespace mu::engraving {
class IFileInfoProvider
{
public:
    virtual ~IFileInfoProvider() = default;

    virtual io::path path() const = 0; //! Absolute path
    virtual io::path fileName(bool includingExtension = true) const = 0;
    virtual io::path absoluteDirPath() const = 0; //! Absolute path of the containing folder

    virtual QDateTime birthTime() const = 0;
    virtual QDateTime lastModified() const = 0;
};

using IFileInfoProviderPtr = std::shared_ptr<IFileInfoProvider>;
}

//! NOTE compat
namespace Ms {
using IFileInfoProviderPtr = mu::engraving::IFileInfoProviderPtr;
}

#endif // MU_ENGRAVING_IFILEINFOPROVIDER_H
