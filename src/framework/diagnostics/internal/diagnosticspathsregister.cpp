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
#include "diagnosticspathsregister.h"

using namespace muse::diagnostics;

void DiagnosticsPathsRegister::reg(const std::string& name, const muse::io::path_t& path)
{
    Item item;
    item.name = name;
    item.path = path;
    m_items.push_back(std::move(item));
}

const std::vector<IDiagnosticsPathsRegister::Item>& DiagnosticsPathsRegister::items() const
{
    return m_items;
}
