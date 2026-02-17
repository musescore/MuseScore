/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "toursmodule.h"

#include "modularity/ioc.h"

#include "internal/toursservice.h"
#include "internal/toursconfiguration.h"
#include "internal/toursprovider.h"

using namespace muse::tours;
using namespace muse::modularity;

std::string ToursModule::moduleName() const
{
    return "tours";
}

void ToursModule::registerExports()
{
    m_service = std::make_shared<ToursService>(globalCtx());
    m_configuration = std::make_shared<ToursConfiguration>(globalCtx());
    m_provider = std::make_shared<ToursProvider>(globalCtx());

    globalIoc()->registerExport<IToursService>(moduleName(), m_service);
    globalIoc()->registerExport<IToursConfiguration>(moduleName(), m_configuration);
    globalIoc()->registerExport<IToursProvider>(moduleName(), m_provider);
}
