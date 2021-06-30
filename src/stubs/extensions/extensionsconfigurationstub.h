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
#ifndef MU_EXTENSIONS_EXTENSIONSCONFIGURATIONSTUB_H
#define MU_EXTENSIONS_EXTENSIONSCONFIGURATIONSTUB_H

#include "extensions/iextensionsconfiguration.h"

namespace mu::extensions {
class ExtensionsConfigurationStub : public IExtensionsConfiguration
{
public:
    QUrl extensionsUpdateUrl() const;
    QUrl extensionFileServerUrl(const QString&) const;

    bool needCheckForUpdate() const override;
    void setNeedCheckForUpdate(bool needCheck) override;

    ValCh<ExtensionsHash> extensions() const;
    Ret setExtensions(const ExtensionsHash& extensions) const override;

    io::path extensionPath(const QString& extensionCode) const override;
    io::path extensionArchivePath(const QString& extensionCode) const override;

    io::paths extensionWorkspaceFiles(const QString& extensionCode) const override;
    io::paths workspacesPaths() const override;

    io::paths extensionInstrumentFiles(const QString& extensionCode) const override;
    io::paths instrumentsPaths() const override;

    io::paths templatesPaths() const override;

    ValCh<io::path> extensionsPath() const override;
    void setExtensionsPath(const io::path& path) override;
};
}

#endif // MU_EXTENSIONS_IEXTENSIONSCONFIGURATION_H
