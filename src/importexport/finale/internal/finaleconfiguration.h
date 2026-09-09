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

#include "../ifinaleconfiguration.h"
#include "async/asyncable.h"

namespace mu::iex::finale {
class FinaleConfiguration : public IFinaleConfiguration, public muse::async::Asyncable
{
public:
    void init();

    ImportPositionsType importPositionsType() const override;
    bool convertTextSymbols() const override;

    void setImportPositionsType(ImportPositionsType importPositionsType) override;
    void setConvertTextSymbols(bool convert) override;

    muse::async::Channel<IFinaleConfiguration::ImportPositionsType> importPositionsTypeChanged() const override;
    muse::async::Channel<bool> convertTextSymbolsChanged() const override;

private:
    muse::async::Channel<IFinaleConfiguration::ImportPositionsType> m_importPositionsTypeChanged;
    muse::async::Channel<bool> m_convertTextSymbolsChanged;
};
}
