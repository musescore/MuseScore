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

#include "modularity/ioc.h"
#include "iglobalconfiguration.h"

#include "imusesoundsconfiguration.h"

namespace mu::musesounds {
class MuseSoundsConfiguration : public IMuseSoundsConfiguration, public muse::Injectable
{
    Inject<muse::IGlobalConfiguration> globalConfiguration = { this };

public:
    MuseSoundsConfiguration(const muse::modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();

    muse::network::RequestHeaders headers() const override;

    muse::UriQuery soundsUri() const override;
    muse::UriQuery soundPageUri(const muse::String& soundCode) const override;

    bool needCheckForMuseSoundsUpdate() const override;

    QUrl checkForMuseSoundsUpdateUrl() const override;
    QUrl checkForMuseSamplerUpdateUrl() const override;

    QString getMuseSamplerVersionQuery() const override;

    std::string lastShownMuseSoundsReleaseVersion() const override;
    void setLastShownMuseSoundsReleaseVersion(const std::string& version) override;

    bool museSamplerCheckForUpdateTestMode() const override;

    bool museSamplerUpdateAvailable() const override;
    void setMuseSamplerUpdateAvailable(bool value) override;

private:
    bool getSoundsTestMode() const;
};
}
