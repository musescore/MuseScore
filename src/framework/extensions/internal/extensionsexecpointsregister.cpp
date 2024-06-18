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
#include "extensionsexecpointsregister.h"

#include "containers.h"

#include "log.h"

using namespace muse::extensions;

void ExtensionsExecPointsRegister::reg(const std::string& module, const ExecPoint& p)
{
    //! NOTE For statistics in the future
    UNUSED(module);

    auto it = m_points.find(p.name);
    IF_ASSERT_FAILED(it == m_points.end()) {
        LOGE() << "already registred point with name: " << p.name;
        return;
    }

    m_points[p.name] = p;
}

ExecPoint ExtensionsExecPointsRegister::point(const std::string& name) const
{
    auto it = m_points.find(name);
    if (it != m_points.end()) {
        return it->second;
    }
    return ExecPoint();
}

std::vector<ExecPoint> ExtensionsExecPointsRegister::allPoints() const
{
    return muse::values(m_points);
}
