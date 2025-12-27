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
#include "finaleconfiguration.h"

#include "settings.h"
#include "translation.h"

using namespace mu;
using namespace muse;
using namespace mu::iex::finale;

static const std::string module_name("iex_finale");

static const Settings::Key FINALE_IMPORT_POSITIONS_KEY(module_name, "import/finale/importPositions");

void FinaleConfiguration::init()
{
    settings()->setDefaultValue(FINALE_IMPORT_POSITIONS_KEY, Val(ImportPositionsType::AdjustmentsOnly));

    settings()->valueChanged(FINALE_IMPORT_POSITIONS_KEY).onReceive(this, [this](const Val& val) {
        m_importPositionsTypeChanged.send(val.toEnum<ImportPositionsType>());
    });
}

IFinaleConfiguration::ImportPositionsType FinaleConfiguration::importPositionsType() const
{
    return settings()->value(FINALE_IMPORT_POSITIONS_KEY).toEnum<IFinaleConfiguration::ImportPositionsType>();
}

void FinaleConfiguration::setImportPositionsType(IFinaleConfiguration::ImportPositionsType importPositionsType)
{
    settings()->setSharedValue(FINALE_IMPORT_POSITIONS_KEY, Val(importPositionsType));
}

async::Channel<IFinaleConfiguration::ImportPositionsType> FinaleConfiguration::importPositionsTypeChanged() const
{
    return m_importPositionsTypeChanged;
}
