/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

#include "global/types/string.h"
#include "global/types/uri.h"
#include "network/networktypes.h"

#include "modularity/imoduleinterface.h"

namespace mu::musesounds {
class IMuseSoundsConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMuseSoundsConfiguration)

public:
    virtual ~IMuseSoundsConfiguration() = default;

    virtual muse::network::RequestHeaders headers() const = 0;

    virtual muse::UriQuery soundsUri() const = 0;
    virtual muse::UriQuery soundPageUri(const muse::String& soundCode) const = 0;

    virtual bool needCheckForMuseSoundsUpdate() const = 0;

    virtual QUrl checkForMuseSoundsUpdateUrl() const = 0;
    virtual QUrl checkForMuseSamplerUpdateUrl() const = 0;

    virtual QString getMuseSamplerVersionQuery() const = 0;

    virtual std::string lastShownMuseSoundsReleaseVersion() const = 0;
    virtual void setLastShownMuseSoundsReleaseVersion(const std::string& version) = 0;

    virtual bool museSoundsCheckForUpdateTestMode() const = 0;
    virtual bool museSamplerCheckForUpdateTestMode() const = 0;

    virtual bool museSamplerUpdateAvailable() const = 0;
    virtual void setMuseSamplerUpdateAvailable(bool value) = 0;
};
}
