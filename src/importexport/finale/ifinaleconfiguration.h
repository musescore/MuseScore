/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#pragma once

#include "modularity/imoduleinterface.h"
#include "async/channel.h"

namespace mu::iex::finale {
class IFinaleConfiguration : MODULE_GLOBAL_EXPORT_INTERFACE
{
    INTERFACE_ID(IFinaleConfiguration)

public:
    virtual ~IFinaleConfiguration() = default;

    enum class ImportPositionsType {
        None, AdjustmentsOnly, All
    };

    virtual ImportPositionsType importPositionsType() const = 0;
    virtual bool convertTextSymbols() const = 0;

    virtual void setImportPositionsType(ImportPositionsType importPositionsType) = 0;
    virtual void setConvertTextSymbols(bool convert) = 0;

    virtual muse::async::Channel<ImportPositionsType> importPositionsTypeChanged() const = 0;
    virtual muse::async::Channel<bool> convertTextSymbolsChanged() const = 0;
};
}
