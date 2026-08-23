/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <variant>

#include "io/path.h"
#include "log.h"

#include "projecttypes.h"

namespace mu::project {
enum class SaveLocationType
{
    Undefined,
    Local,
    Cloud
};

struct SaveLocation
{
    SaveLocationType type = SaveLocationType::Undefined;

    std::variant<muse::io::path_t, CloudProjectInfo> data;

    bool isLocal() const
    {
        return type == SaveLocationType::Local
               && std::holds_alternative<muse::io::path_t>(data);
    }

    bool isCloud() const
    {
        return type == SaveLocationType::Cloud
               && std::holds_alternative<CloudProjectInfo>(data);
    }

    bool isValid() const
    {
        return isLocal() || isCloud();
    }

    const muse::io::path_t& localPath() const
    {
        IF_ASSERT_FAILED(isLocal()) {
            static muse::io::path_t null;
            return null;
        }

        return std::get<muse::io::path_t>(data);
    }

    const CloudProjectInfo& cloudInfo() const
    {
        IF_ASSERT_FAILED(isCloud()) {
            static CloudProjectInfo null;
            return null;
        }

        return std::get<CloudProjectInfo>(data);
    }

    SaveLocation() = default;

    SaveLocation(SaveLocationType type, const std::variant<muse::io::path_t, CloudProjectInfo>& data = {})
        : type(type), data(data) {}

    SaveLocation(const muse::io::path_t& localPath)
        : type(SaveLocationType::Local), data(localPath) {}

    SaveLocation(const CloudProjectInfo& cloudInfo)
        : type(SaveLocationType::Cloud), data(cloudInfo) {}
};
}
