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
#ifndef MU_MI_RESOURCELOCKGUARD_H
#define MU_MI_RESOURCELOCKGUARD_H

#include <string>
#include <memory>
#include "imultiinstancesprovider.h"

namespace mu::mi {
class ResourceLockGuard
{
public:
    ResourceLockGuard(std::shared_ptr<IMultiInstancesProvider> provider, const std::string& name)
        : m_provider(provider), m_name(name)
    {
        if (m_provider) {
            m_provider->lockResource(m_name);
        }
    }

    ~ResourceLockGuard()
    {
        if (m_provider) {
            m_provider->unlockResource(m_name);
        }
    }

private:
    std::shared_ptr<IMultiInstancesProvider> m_provider = nullptr;
    std::string m_name;
};
}

#endif // MU_MI_RESOURCELOCKGUARD_H
