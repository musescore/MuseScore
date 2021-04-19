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
#ifndef MU_EXTENSIONS_IEXTENSIONSSERVICE_H
#define MU_EXTENSIONS_IEXTENSIONSSERVICE_H

#include "modularity/imoduleexport.h"
#include "retval.h"

#include "extensionstypes.h"

namespace mu::extensions {
class IExtensionsService : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IExtensionsService)

public:
    virtual ~IExtensionsService() = default;

    virtual ValCh<ExtensionsHash> extensions() const = 0;
    virtual RetCh<ExtensionProgress> install(const QString& extensionCode) = 0;
    virtual RetCh<ExtensionProgress> update(const QString& extensionCode) = 0;
    virtual Ret uninstall(const QString& extensionCode) = 0;

    virtual RetCh<Extension> extensionChanged() const = 0;
};
}

#endif // MU_EXTENSIONS_IEXTENSIONSSERVICE_H
