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
#ifndef MUSE_LEARN_ILEARNCONFIGURATION_H
#define MUSE_LEARN_ILEARNCONFIGURATION_H

#include "modularity/imoduleinterface.h"
#include "network/networktypes.h"

namespace muse::learn {
class ILearnConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ILearnConfiguration)

public:
    virtual ~ILearnConfiguration() = default;

    virtual network::RequestHeaders headers() const = 0;

    virtual QUrl startedPlaylistUrl() const = 0;
    virtual QUrl advancedPlaylistUrl() const = 0;
    virtual bool classesEnabled() const = 0;
};
}

#endif // MUSE_LEARN_ILEARNCONFIGURATION_H
