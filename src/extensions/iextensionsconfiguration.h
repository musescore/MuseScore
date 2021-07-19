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
#ifndef MU_EXTENSIONS_IEXTENSIONSCONFIGURATION_H
#define MU_EXTENSIONS_IEXTENSIONSCONFIGURATION_H

#include "modularity/imoduleexport.h"
#include "retval.h"
#include "io/path.h"
#include "extensionstypes.h"

namespace mu::extensions {
class IExtensionsConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IExtensionsConfiguration)

public:
    virtual ~IExtensionsConfiguration() = default;

    virtual QUrl extensionsUpdateUrl() const = 0;
    virtual QUrl extensionFileServerUrl(const QString& extensionCode) const = 0;

    virtual bool needCheckForUpdate() const = 0;
    virtual void setNeedCheckForUpdate(bool needCheck) = 0;

    virtual ValCh<ExtensionsHash> extensions() const = 0;
    virtual Ret setExtensions(const ExtensionsHash& extensions) const = 0;

    virtual io::path extensionPath(const QString& extensionCode) const = 0;
    virtual io::path extensionArchivePath(const QString& extensionCode) const = 0;

    virtual io::paths extensionWorkspaceFiles(const QString& extensionCode) const = 0;
    virtual io::paths workspacesPaths() const = 0;

    virtual io::paths extensionInstrumentFiles(const QString& extensionCode) const = 0;
    virtual io::paths instrumentsPaths() const = 0;

    virtual io::paths templatesPaths() const = 0;

    virtual io::path userExtensionsPath() const = 0;
    virtual void setUserExtensionsPath(const io::path& path) = 0;
    virtual async::Channel<io::path> userExtensionsPathChanged() const = 0;
};
}

#endif // MU_EXTENSIONS_IEXTENSIONSCONFIGURATION_H
