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
#ifndef MU_EXTENSIONS_EXTENSIONSSERVICE_H
#define MU_EXTENSIONS_EXTENSIONSSERVICE_H

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "iextensionsservice.h"
#include "iextensionsconfiguration.h"
#include "iextensionunpacker.h"
#include "framework/system/ifilesystem.h"
#include "framework/network/inetworkmanagercreator.h"

namespace mu::extensions {
class ExtensionsService : public IExtensionsService, public async::Asyncable
{
    INJECT(extensions, IExtensionsConfiguration, configuration)
    INJECT(extensions, IExtensionUnpacker, extensionUnpacker)
    INJECT(extensions, system::IFileSystem, fileSystem)
    INJECT(extensions, network::INetworkManagerCreator, networkManagerCreator)

public:
    void refreshExtensions();

    ValCh<ExtensionsHash> extensions() const override;
    RetCh<ExtensionProgress> install(const QString& extensionCode) override;
    RetCh<ExtensionProgress> update(const QString& extensionCode) override;
    Ret uninstall(const QString& extensionCode) override;

    RetCh<Extension> extensionChanged() const override;

private:
    RetVal<ExtensionsHash> parseExtensionConfig(const QByteArray& json) const;
    bool isExtensionExists(const QString& extensionCode) const;

    RetVal<ExtensionsHash> correctExtensionsStates(ExtensionsHash& extensions) const;

    RetVal<QString> downloadExtension(const QString& extensionCode, async::Channel<ExtensionProgress>* progressChannel) const;
    Ret removeExtension(const QString& extensionCode) const;

    Extension::ExtensionTypes extensionTypes(const QString& extensionCode) const;

    void th_refreshExtensions();
    void th_install(const QString& extensionCode, async::Channel<ExtensionProgress>* progressChannel, async::Channel<Ret>* finishChannel);
    void th_update(const QString& extensionCode, async::Channel<ExtensionProgress>* progressChannel, async::Channel<Ret>* finishChannel);

    void closeOperation(const QString& extensionCode, async::Channel<ExtensionProgress>* progressChannel);

    enum OperationType
    {
        None,
        Install,
        Update
    };

    struct Operation
    {
        OperationType type = OperationType::None;
        async::Channel<ExtensionProgress>* progressChannel = nullptr;

        Operation() = default;
        Operation(const OperationType& type, async::Channel<ExtensionProgress>* progressChannel)
            : type(type), progressChannel(progressChannel) {}
    };

private:
    async::Channel<Extension> m_extensionChanged;

    mutable QHash<QString, Operation> m_operationsHash;
};
}

#endif // MU_EXTENSIONS_EXTENSIONSSERVICE_H
