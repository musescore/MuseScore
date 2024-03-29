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
#ifndef MU_GLOBAL_CONFIG_H
#define MU_GLOBAL_CONFIG_H

#include "types/val.h"

namespace mu {
//! NOTE Some modules have configuration files, this is their model
struct Config {
    Config() = default;
    Config(const ValMap& v)
        : m_data(v) {}

    const Val& value(const std::string& key, const Val& def = Val()) const
    {
        auto it = m_data.find(key);
        if (it != m_data.end()) {
            return it->second;
        }
        return def;
    }

    void set(const std::string& key, const Val& val)
    {
        m_data[key] = val;
    }

private:
    ValMap m_data;
};
}

#endif // MU_GLOBAL_CONFIG_H
