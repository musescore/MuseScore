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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "extensioncontextresolver.h"

#include "extensions/extensionstypes.h"

using namespace mu::context;

void ExtensionContextResolver::init()
{
    globalContext()->currentProjectChanged().onNotify(this, [this]() {
        m_contextChanged.notify();
    });
}

bool ExtensionContextResolver::isContextAllowed(const std::string& context) const
{
    if (context == muse::extensions::PROJECT_OPENED_CONTEXT) {
        return globalContext()->currentProject() != nullptr;
    }

    return true;
}

muse::async::Notification ExtensionContextResolver::contextChanged() const
{
    return m_contextChanged;
}
