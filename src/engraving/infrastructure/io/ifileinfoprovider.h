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
#include <QString>

namespace mu::engraving {
class IFileInfoProvider
{
public:
    virtual ~IFileInfoProvider() = default;

    /// Returns a file's absolute path, excluding the filename.
    virtual QString absoluteDirPath() const = 0;

    /// Returns a file's absolute path, including the filename.
    virtual QString absoluteFilePath() const = 0;

    virtual QString fileName() const = 0;

    /// Returns the filename, excluding the extension
    /// (that is, up to, but not including, the last dot character).
    virtual QString completeBaseName() const = 0;

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
