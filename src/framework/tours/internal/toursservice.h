/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "itoursprovider.h"
#include "itoursconfiguration.h"

#include "../itoursservice.h"

namespace muse::tours {
class ToursService : public IToursService, public Injectable, public async::Asyncable
{
    Inject<IInteractive> interactive = { this };
    Inject<IToursProvider> toursProvider = { this };
    Inject<IToursConfiguration> toursConfiguration = { this };

public:
    ToursService(const muse::modularity::ContextPtr& ctx)
        : Injectable(ctx) {}

    void registerTour(const String& eventCode, const Tour& tour) override;

    void onEvent(const String& eventCode) override;

private:
    std::unordered_map<String /*eventCode*/, Tour> m_eventsMap;
};
}
