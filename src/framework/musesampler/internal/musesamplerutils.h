/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#ifndef MU_MUSESAMPLER_UTILS_H
#define MU_MUSESAMPLER_UTILS_H

#include <string>
#include <optional>

namespace mu::musesampler {

// Format of stored ID:
// CATEGORY\NAME\UNIQUE ID

inline static std::optional<std::string> getMuseInstrumentCategoryFromId(const std::string& id)
{
    if (auto pos = id.find_first_of('\\'); pos != std::string::npos) 
        return id.substr(0, pos);
    return std::nullopt;
}

inline static std::optional<std::string> getMuseInstrumentNameFromId(const std::string& id)
{
    auto start = id.find_first_of('\\');
    auto end = id.find_last_of('\\');
    if (start == std::string::npos || end == std::string::npos || start >= end)
        return std::nullopt;

    return id.substr(start + 1, end - start - 1);
}

inline static std::optional<int> getMuseInstrumentUniqueIdFromId(const std::string& id)
{
    auto pos = id.find_last_of('\\');
    try {
        return std::stoi(id.substr(pos + 1));
    } catch(...) {
        return std::nullopt;
    }
}

inline static std::string buildMuseInstrumentId(const std::string& category, const std::string& name, int unique_id)
{
    return category + '\\' + name + '\\' + std::to_string(unique_id);
}

} // namespace mu::musesampler

#endif // MU_MUSESAMPLER_UTILS_H