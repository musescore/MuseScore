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

#include "cloudsregister.h"

#include "containers.h"

using namespace mu::cloud;

void CloudsRegister::reg(const QString& cloudCode, IAuthorizationServicePtr cloud)
{
    m_clouds.insert({ cloudCode, cloud });
}

IAuthorizationServicePtr CloudsRegister::cloud(const QString& cloudCode) const
{
    auto it = m_clouds.find(cloudCode);
    if (it != m_clouds.end()) {
        return it->second;
    }

    return nullptr;
}

std::vector<IAuthorizationServicePtr> CloudsRegister::clouds() const
{
    return values(m_clouds);
}
