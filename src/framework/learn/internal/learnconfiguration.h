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
#ifndef MUSE_LEARN_LEARNCONFIGURATION_H
#define MUSE_LEARN_LEARNCONFIGURATION_H

#include "modularity/ioc.h"
#include "iglobalconfiguration.h"

#include "global/types/config.h"

#include "ilearnconfiguration.h"

namespace muse::learn {
class LearnConfiguration : public ILearnConfiguration, public Injectable
{
    Inject<IGlobalConfiguration> globalConfiguration = { this };

public:
    LearnConfiguration(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();

    network::RequestHeaders headers() const override;

    QUrl startedPlaylistUrl() const override;
    QUrl advancedPlaylistUrl() const override;
    bool classesEnabled() const override;

private:
    bool isTestingMode() const;

    Config m_config;
};
}

#endif // MUSE_LEARN_LEARNCONFIGURATION_H
