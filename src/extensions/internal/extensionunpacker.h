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
#ifndef MU_EXTENSIONS_EXTENSIONUNPACKER_H
#define MU_EXTENSIONS_EXTENSIONUNPACKER_H

#include "modularity/ioc.h"
#include "retval.h"

#include "iextensionunpacker.h"
#include "framework/system/ifilesystem.h"

class MQZipReader;
class QVersionNumber;

namespace mu::extensions {
class ExtensionUnpacker : public IExtensionUnpacker
{
    INJECT(extensions, system::IFileSystem, fileSystem)

public:

    Ret unpack(const QString& source, const QString& destination) const override;

private:
    Ret isDirectoryWritable(const QString& directoryPath) const;
    Ret hasFreeSpace(const QString& directoryPath, qint64 neededSpace) const;

    RetVal2<QString, QVersionNumber> extensionMeta(const MQZipReader* zip) const;

    Ret checkActualVersion(const QString& destionation, const QString& extensionId, const QVersionNumber& version) const;
    Ret removePreviousVersion(const QString& path) const;
    Ret unzip(const MQZipReader* zip, const QString& destination) const;
};
}

#endif // MU_EXTENSIONS_EXTENSIONUNPACKER_H
