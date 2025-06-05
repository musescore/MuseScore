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

#include "io/path.h"

#include "modularity/ioc.h"
#include "iglobalconfiguration.h"

#include "tourstypes.h"

#include "itoursconfiguration.h"

namespace muse::tours {
class ToursConfiguration : public IToursConfiguration, public Injectable
{
    Inject<IGlobalConfiguration> globalConfiguration = { this };

public:
    ToursConfiguration(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    String lastShownTourIdForEvent(const String& eventCode) const override;
    void setLastShownTourIdForEvent(const String& eventCode, const String& tourId) override;

    io::path_t toursFilePath() const override;

private:
    StringList lastShownTours() const;
};
}
