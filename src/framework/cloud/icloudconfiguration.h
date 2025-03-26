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
#pragma once

#include "io/path.h"
#include "network/networktypes.h"

#include "modularity/imoduleinterface.h"

namespace muse::cloud {
class ICloudConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ICloudConfiguration)

public:
    virtual ~ICloudConfiguration() = default;

    virtual std::string clientId() const = 0;

    virtual QByteArray uploadingLicense() const = 0;

    virtual io::path_t tokensFilePath(const std::string& cloudName) const = 0;

    virtual network::RequestHeaders headers() const = 0;
};
}
